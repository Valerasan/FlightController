#include "app.h"
#include "log.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "uart_crsf.h"
#include "imu_lsm6ds3.h"
#include "attitude_estimator.h"

constexpr uint32_t kBlinkPeriodMs = 150;
extern UART_HandleTypeDef huart1;
extern SPI_HandleTypeDef hspi1;
UartCrsf _uartCrsf(&huart1);
ImuLsm6ds3 _imu(&hspi1, IMU_CS_GPIO_Port, IMU_CS_Pin);
AttitudeEstimator _attitude;

constexpr uint8_t kCrsfFrameRcChannelsPacked = 0x16;
constexpr uint8_t kCrsfChannelCount = 8;

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


static volatile int16_t s_lastAccel[3];
static volatile int16_t s_lastGyro[3];
static volatile uint32_t s_intCount = 0;   // diagnostic: how many times the ISR actually fired

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin != INT1_Pin) {
        return;
    }
    s_intCount++;

    static uint32_t lastTickMs = 0;
    const uint32_t nowMs = HAL_GetTick();
    const float dtSeconds = (lastTickMs == 0) ? 0.0f : (nowMs - lastTickMs) / 1000.0f;
    lastTickMs = nowMs;

    int16_t accel[3];
    int16_t gyro[3];
    if (_imu.readRaw(accel, gyro)) {
        _attitude.update(accel, gyro, dtSeconds);
        s_lastAccel[0] = accel[0]; s_lastAccel[1] = accel[1]; s_lastAccel[2] = accel[2];
        s_lastGyro[0] = gyro[0]; s_lastGyro[1] = gyro[1]; s_lastGyro[2] = gyro[2];
    }
}

extern "C" void app_init(void)
{
    _LED_Blue_On;
    _uartCrsf.init();

    if (!_imu.init()) {
        LOG("lsm6ds3 init failed (bad WHO_AM_I / SPI wiring?)");
    }

    HAL_Delay(5000);
    uint8_t ctrl1Xl = 0, ctrl2G = 0, int1Ctrl = 0;
    if (_imu.readDebugRegs(ctrl1Xl, ctrl2G, int1Ctrl)) {
        LOG("ctrl1_xl=0x%02X ctrl2_g=0x%02X int1_ctrl=0x%02X", ctrl1Xl, ctrl2G, int1Ctrl);
    } else {
        LOG("readDebugRegs SPI failed");
    }
}

extern "C" void app_loop(void)
{
    static uint32_t lastLogMs = 0;
    const uint32_t nowMs = HAL_GetTick();
    if (nowMs - lastLogMs >= 100) {
        lastLogMs = nowMs;
        const float roll = _attitude.rollDeg();
        const float pitch = _attitude.pitchDeg();
        LOG("R:%.1f,P:%.1f,AL:%.1f,AR:%.1f", roll, pitch, roll, -roll);
    }

    if (_uartCrsf.frameReady()) {
        if (_uartCrsf.frameType() == kCrsfFrameRcChannelsPacked) {
            uint16_t channels[kCrsfChannelCount];
            crsf_parse_channels(_uartCrsf.framePayload(), channels);

            uint16_t us[kCrsfChannelCount];
            for (uint8_t i = 0; i < kCrsfChannelCount; i++) {
                us[i] = crsf_to_pwm_us(channels[i]);
            }

            // LOG("ch1=%u ch2=%u ch3=%u ch4=%u ch5=%u ch6=%u ch7=%u ch8=%u",
            //     us[0], us[1], us[2], us[3], us[4], us[5], us[6], us[7]);
        }
        _uartCrsf.consumeFrame();
    }




    _LED_Blue_Toggle;
    HAL_Delay(kBlinkPeriodMs);
}
