#include <DW1000Jang.hpp>
#include <DW1000JangUtils.hpp>
#include <DW1000JangTime.hpp>
#include <DW1000JangConstants.hpp>
#include <DW1000JangRanging.hpp>
#include <DW1000JangRTLS.hpp>

const uint8_t PIN_RST = 7;
const uint8_t PIN_IRQ = 2;
const uint8_t PIN_SS  = 10;

const double MAX_VALID_RANGE = 2.0;
const double ENTRY_THRESHOLD = 1.05;
const double EXIT_THRESHOLD  = 1.05;

const byte ENTRY_ANCHOR[2] = {0x05, 0x00};
const byte EXIT_ANCHOR[2]  = {0x06, 0x00};
const char* car_number = "12가4568";

byte SEQ_NUMBER = 1;

unsigned long entryTime = 0;
unsigned long exitTime  = 0;
bool hasEntered = false;
bool hasExited  = false;

double est_distance = 0;
double est_error = 1;
const double process_noise = 0.01;
const double measurement_noise = 0.1;

device_configuration_t DEFAULT_CONFIG = {
    false, true, true, true, false,
    SFDMode::STANDARD_SFD,
    Channel::CHANNEL_4,
    DataRate::RATE_850KBPS,
    PulseFrequency::FREQ_16MHZ,
    PreambleLength::LEN_256,
    PreambleCode::CODE_3
};

frame_filtering_configuration_t TAG_FRAME_FILTER_CONFIG = {
    false, true, true, false, false, false, false, false
};

double applyKalmanFilter(double measured) {
    est_error += process_noise;
    double kalman_gain = est_error / (est_error + measurement_noise);
    est_distance = est_distance + kalman_gain * (measured - est_distance);
    est_error = (1 - kalman_gain) * est_error;
    return est_distance;
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("### TAG Ready ###"));

    DW1000Jang::initialize(PIN_SS, PIN_IRQ, PIN_RST);
    DW1000Jang::applyConfiguration(DEFAULT_CONFIG);
    DW1000Jang::enableFrameFiltering(TAG_FRAME_FILTER_CONFIG);

    DW1000Jang::setDeviceAddress(4);
    DW1000Jang::setNetworkId(10);
    DW1000Jang::setAntennaDelay(16436);
    DW1000Jang::setPreambleDetectionTimeout(64);
    DW1000Jang::setSfdDetectionTimeout(273);
    DW1000Jang::setReceiveFrameWaitTimeoutPeriod(5000);
}

void loop() {
    if (!hasEntered) {
        checkAndRecordEntry();
    } else if (!hasExited) {
        checkAndRecordExitAndSendDuration();
    }
}

void checkAndRecordEntry() {
    DW1000JangRTLS::transmitPoll(ENTRY_ANCHOR);
    if (!DW1000JangRTLS::waitForNextRangingStep()) return;

    size_t cont_len = DW1000Jang::getReceivedDataLength();
    byte cont_recv[cont_len];
    DW1000Jang::getReceivedData(cont_recv, cont_len);

    if (cont_len > 10 && cont_recv[9] == ACTIVITY_CONTROL && cont_recv[10] == RANGING_CONTINUE) {
        DW1000JangRTLS::transmitFinalMessage(
            &cont_recv[7],
            1500,
            DW1000Jang::getTransmitTimestamp(),
            DW1000Jang::getReceiveTimestamp()
        );
        DW1000JangRTLS::waitForTransmission();

        if (!DW1000JangRTLS::receiveFrame()) return;

        size_t act_len = DW1000Jang::getReceivedDataLength();
        byte act_recv[act_len];
        DW1000Jang::getReceivedData(act_recv, act_len);

        if (act_len > 12 && act_recv[9] == ACTIVITY_CONTROL && act_recv[10] == RANGING_CONFIRM) {
            double raw_distance = static_cast<double>(DW1000JangUtils::bytesAsValue(&act_recv[13], 2)) / 1000.0;
            if (raw_distance <= 0 || raw_distance > MAX_VALID_RANGE) return;

            double filtered = applyKalmanFilter(raw_distance);
            Serial.print("✔ Entry 거리: ");
            Serial.print(filtered);
            Serial.println(" m");

            if (filtered <= ENTRY_THRESHOLD) {
                entryTime = millis();
                hasEntered = true;
                Serial.print("🚗 입구 도착! entryTime(ms): ");
                Serial.println(entryTime);
            }
        }
    }
}

void checkAndRecordExitAndSendDuration() {
    DW1000JangRTLS::transmitPoll(EXIT_ANCHOR);
    if (!DW1000JangRTLS::waitForNextRangingStep()) return;

    size_t cont_len = DW1000Jang::getReceivedDataLength();
    byte cont_recv[cont_len];
    DW1000Jang::getReceivedData(cont_recv, cont_len);

    if (cont_len > 10 && cont_recv[9] == ACTIVITY_CONTROL && cont_recv[10] == RANGING_CONTINUE) {
        DW1000JangRTLS::transmitFinalMessage(
            &cont_recv[7],
            1500,
            DW1000Jang::getTransmitTimestamp(),
            DW1000Jang::getReceiveTimestamp()
        );
        DW1000JangRTLS::waitForTransmission();

        if (!DW1000JangRTLS::receiveFrame()) return;

        size_t act_len = DW1000Jang::getReceivedDataLength();
        byte act_recv[act_len];
        DW1000Jang::getReceivedData(act_recv, act_len);

        if (act_len > 12 && act_recv[9] == ACTIVITY_CONTROL && act_recv[10] == RANGING_CONFIRM) {
            double raw_distance = static_cast<double>(DW1000JangUtils::bytesAsValue(&act_recv[13], 2)) / 1000.0;
            if (raw_distance <= 0 || raw_distance > MAX_VALID_RANGE) return;

            double filtered = applyKalmanFilter(raw_distance);
            Serial.print("✔ Exit 거리: ");
            Serial.print(filtered);
            Serial.println(" m");

            if (filtered <= EXIT_THRESHOLD) {
                exitTime = millis();
                hasExited = true;
                unsigned long duration = exitTime - entryTime;

                Serial.print("🏁 출구 도착! exitTime(ms): ");
                Serial.println(exitTime);
                Serial.print("🚗 입구 entryTime(ms): ");
                Serial.println(entryTime);
                Serial.print("시간 차이 duration(ms): ");
                Serial.println(duration);

                sendDuration(duration);
            }
        }
    }
}

void sendDuration(unsigned long duration) {
    Serial.print("📤 이동 시간 전송: ");
    Serial.println(duration);

    byte msg[23] = {
      0x01,       // Frame Type: DATA
      0x88,       // Frame Control: Short Src/Dest
      SEQ_NUMBER++,

      0x0A, 0x00, // Network ID (송신자 기준)
      0x06, 0x00, // 수신자 주소 (출구 Anchor = 6)

      0x0A, 0x00, // Network ID (송신자 기준)
      0x04, 0x00, // 송신자 주소 (TAG = 4)

      (duration >> 24) & 0xFF,
      (duration >> 16) & 0xFF,
      (duration >> 8) & 0xFF,
      duration & 0xFF
    };
  

    // duration 값을 Big Endian으로 삽입
    msg[11] = (duration >> 24) & 0xFF;
    msg[12] = (duration >> 16) & 0xFF;
    msg[13] = (duration >> 8)  & 0xFF;
    msg[14] = duration & 0xFF;

    memcpy(&msg[15], car_number, 8);

    DW1000Jang::setTransmitData(msg, sizeof(msg));
    DW1000Jang::startTransmit();
    while (!DW1000Jang::isTransmitDone()) { yield(); }
    DW1000Jang::clearTransmitStatus();
}
