// [터널 입구 코드]

#include <DW1000Jang.hpp>
#include <DW1000JangUtils.hpp>
#include <DW1000JangTime.hpp>
#include <DW1000JangConstants.hpp>
#include <DW1000JangRanging.hpp>
#include <DW1000JangRTLS.hpp>

const uint8_t PIN_RST = 7;
const uint8_t PIN_IRQ = 2;
const uint8_t PIN_SS  = 10;

const byte TARGET_TAG_ADDR = 0x04; // Tag 주소 (고정)

// 거리 제한
const double MAX_VALID_RANGE = 2.0;

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
    false, false, true, false, false, false, false, false
};

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("### DW1000Jang Entry Anchor ###"));

    DW1000Jang::initialize(PIN_SS, PIN_IRQ, PIN_RST);
    DW1000Jang::applyConfiguration(DEFAULT_CONFIG);
    DW1000Jang::enableFrameFiltering(ANCHOR_FRAME_FILTER_CONFIG);

    DW1000Jang::setDeviceAddress(5);  // Entry Anchor
    DW1000Jang::setNetworkId(10);
    DW1000Jang::setAntennaDelay(16436);
    DW1000Jang::setPreambleDetectionTimeout(64);
    DW1000Jang::setSfdDetectionTimeout(273);
    DW1000Jang::setReceiveFrameWaitTimeoutPeriod(8000);

    Serial.println(F("Entry Anchor Ready."));
}

void loop() {
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
            } else {
                Serial.print("❌ 거리 초과 (");
                Serial.print(range);
                Serial.println(" m) - 무시됨");
            }
        }
    }
}
