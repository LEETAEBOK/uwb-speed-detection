#include <DW1000Jang.hpp>
#include <DW1000JangUtils.hpp>
#include <DW1000JangTime.hpp>
#include <DW1000JangConstants.hpp>
#include <DW1000JangRanging.hpp>
#include <DW1000JangRTLS.hpp>

const uint8_t PIN_RST = 7;
const uint8_t PIN_IRQ = 2;
const uint8_t PIN_SS  = 10;

const byte TARGET_TAG_ADDR = 0x04;
const double MAX_VALID_RANGE = 2.0;
const double DISTANCE_THRESHOLD = 1.05;
char car_number[9] = {0};

const double TUNNEL_LENGTH = 3.0;  // 단위: meter
const double SPEED_LIMIT = 1.5;    // 단위: m/s

bool awaitingTimeData = false;

device_configuration_t DEFAULT_CONFIG = {
    false, true, true, true, false,
    SFDMode::STANDARD_SFD,
    Channel::CHANNEL_4,
    DataRate::RATE_850KBPS,
    PulseFrequency::FREQ_16MHZ,
    PreambleLength::LEN_256,
    PreambleCode::CODE_3
};

frame_filtering_configuration_t ANCHOR_FRAME_FILTER_CONFIG = {
    false, true, true, false, false, false, false, false
};

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("### Exit Anchor Ready ###"));

    DW1000Jang::initialize(PIN_SS, PIN_IRQ, PIN_RST);
    DW1000Jang::applyConfiguration(DEFAULT_CONFIG);
    DW1000Jang::enableFrameFiltering(ANCHOR_FRAME_FILTER_CONFIG);

    DW1000Jang::setDeviceAddress(6);  // 출구 Anchor 주소
    DW1000Jang::setNetworkId(10);
    DW1000Jang::setAntennaDelay(16436);
    DW1000Jang::setPreambleDetectionTimeout(64);
    DW1000Jang::setSfdDetectionTimeout(273);
    DW1000Jang::setReceiveFrameWaitTimeoutPeriod(8000);
}

void loop() {
    if (!awaitingTimeData) {
        // TWR 거리 측정 루틴
        if (!DW1000JangRTLS::receiveFrame()) return;

        size_t poll_len = DW1000Jang::getReceivedDataLength();
        byte poll_data[poll_len];
        DW1000Jang::getReceivedData(poll_data, poll_len);

        if (poll_len > 9 && poll_data[9] == RANGING_TAG_POLL && poll_data[7] == TARGET_TAG_ADDR) {
            uint64_t timePollReceived = DW1000Jang::getReceiveTimestamp();
            DW1000JangRTLS::transmitResponseToPoll(&poll_data[7]);
            DW1000JangRTLS::waitForTransmission();

            delayMicroseconds(1500);
            if (!DW1000JangRTLS::receiveFrame()) return;

            size_t rfinal_len = DW1000Jang::getReceivedDataLength();
            byte rfinal_data[rfinal_len];
            DW1000Jang::getReceivedData(rfinal_data, rfinal_len);

            if (rfinal_len > 18 && rfinal_data[9] == RANGING_TAG_FINAL_RESPONSE_EMBEDDED) {
                uint64_t timeFinalMessageReceive = DW1000Jang::getReceiveTimestamp();

                double range = DW1000JangRanging::computeRangeAsymmetric(
                    DW1000JangUtils::bytesAsValue(rfinal_data + 10, LENGTH_TIMESTAMP),
                    timePollReceived,
                    DW1000Jang::getTransmitTimestamp(),
                    DW1000JangUtils::bytesAsValue(rfinal_data + 14, LENGTH_TIMESTAMP),
                    DW1000JangUtils::bytesAsValue(rfinal_data + 18, LENGTH_TIMESTAMP),
                    timeFinalMessageReceive
                );

                range = DW1000JangRanging::correctRange(range);
                if (range <= 0) range = 0.000001;

                if (range <= MAX_VALID_RANGE) {
                    DW1000JangRTLS::transmitRangingConfirm_v1(&rfinal_data[7], range);
                    DW1000JangRTLS::waitForTransmission();

                    Serial.print("▶ 거리: ");
                    Serial.print(range);
                    Serial.println(" m (응답 전송)");

                    if (range <= DISTANCE_THRESHOLD) {
                        Serial.println("🏁 출구 도착 확인 → 이동 시간 수신 대기");
                        awaitingTimeData = true;
                    }
                } else {
                    Serial.print("❌ 거리 초과: ");
                    Serial.print(range);
                    Serial.println(" m");
                }
            }
        }
    } else {
        // 이동 시간(duration) 수신 루틴
        DW1000Jang::startReceive();
        while (!DW1000Jang::isReceiveDone()) { yield(); }
        DW1000Jang::clearReceiveStatus();

        byte raw[32];
        DW1000Jang::getReceivedData(raw, sizeof(raw));

        // 디버깅용 전체 출력
//        Serial.print("📦 수신 바이트: ");
//        for (int i = 0; i < 15; i++) {
//            Serial.print(raw[i], HEX);
//            Serial.print(" ");
//        }
        Serial.println();

        // duration (4바이트) 복원
        
        unsigned long duration = 0;  // 먼저 선언 (블록 밖에서)

        if (raw[0] == 0x01 && raw[1] == 0x88) {
          duration = ((unsigned long)raw[11] << 24) |
                     ((unsigned long)raw[12] << 16) |
                     ((unsigned long)raw[13] << 8)  |
                     raw[14];

          memcpy(car_number, &raw[15], 8);  // 앞 8바이트만 복원


         Serial.print("📩 수신된 이동 시간: ");
         Serial.print(duration);
         Serial.println(" ms");

          // 속도 계산
          double seconds = duration / 1000.0;
          double speed = TUNNEL_LENGTH / seconds;

         Serial.print("⏱ 소요 시간: ");
          Serial.print(seconds, 3);
         Serial.println(" s");

          Serial.print("🚗 평균 속도: ");
          Serial.print(speed, 3);
         Serial.println(" m/s");

         Serial.print("📩 수신된 차량 번호: ");
          Serial.println(car_number);

          if (speed > SPEED_LIMIT) {
            Serial.println("🚨 과속 감지!");
           } 
          else
           {
            Serial.println("✅ 정상 주행");
           }
        }


      

        awaitingTimeData = false; // 다시 거리 측정으로 복귀
    }
}
