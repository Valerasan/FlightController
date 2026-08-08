#include "app.h"
#include "log.h"
#include "stm32f4xx_hal.h"
#include "uart.h"

constexpr uint32_t kBlinkPeriodMs = 150;
extern UART_HandleTypeDef huart1;
UartBase _uartCrsf(&huart1);

constexpr uint8_t kCrsfFrameRcChannelsPacked = 0x16;
constexpr uint8_t kCrsfChannelCount = 16;

static void crsf_parse_channels(const uint8_t *payload, uint16_t channels[kCrsfChannelCount])
{
    uint32_t bitBuffer = 0;
    uint8_t bitsInBuffer = 0;
    uint8_t byteIdx = 0;

    for (uint8_t ch = 0; ch < kCrsfChannelCount; ch++) {
        while (bitsInBuffer < 11) {
            bitBuffer |= static_cast<uint32_t>(payload[byteIdx++]) << bitsInBuffer;
            bitsInBuffer += 8;
        }
        channels[ch] = bitBuffer & 0x7FF;   
        bitBuffer >>= 11;
        bitsInBuffer -= 11;
    }
}


constexpr uint16_t crsf_to_pwm_us(uint16_t raw)
{
    return static_cast<uint16_t>((static_cast<int32_t>(raw) - 992) * 5 / 8 + 1500);
}


extern "C" void app_init(void)
{
    _LED_Blue_On;
    _uartCrsf.init();
}

extern "C" void app_loop(void)
{
    if (_uartCrsf.frameReady()) {
        if (_uartCrsf.frameType() == kCrsfFrameRcChannelsPacked) {
            uint16_t channels[kCrsfChannelCount];
            crsf_parse_channels(_uartCrsf.framePayload(), channels);

            uint16_t us[kCrsfChannelCount];
            for (uint8_t i = 0; i < kCrsfChannelCount; i++) {
                us[i] = crsf_to_pwm_us(channels[i]);
            }

            LOG("ch1=%u ch2=%u ch3=%u ch4=%u ch5=%u ch6=%u ch7=%u ch8=%u",
                us[0], us[1], us[2], us[3], us[4], us[5], us[6], us[7]);
        }
        _uartCrsf.consumeFrame();
    }


    _LED_Blue_Toggle;
    HAL_Delay(kBlinkPeriodMs);
}
