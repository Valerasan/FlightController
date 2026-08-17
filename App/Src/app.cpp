#include "app.h"
#include "log.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "uart_crsf.h"
#include "imu_lsm6ds3.h"
#include "attitude_estimator.h"
#include "flight_state.h"

constexpr uint32_t kBlinkPeriodMs = 150;
extern UART_HandleTypeDef huart1;
extern SPI_HandleTypeDef hspi1;
UartCrsf _uartCrsf(&huart1);
ImuLsm6ds3 _imu(&hspi1, IMU_CS_GPIO_Port, IMU_CS_Pin);
AttitudeEstimator _attitude;

constexpr uint8_t kCrsfFrameRcChannelsPacked = 0x16;
constexpr uint8_t kCrsfChannelCount = 8;
static volatile float s_pendingDtSeconds = 0.0f;


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


static void onImuSampleReady(const int16_t accel[3], const int16_t gyro[3])
{
    _attitude.update(accel, gyro, s_pendingDtSeconds);
    _flightState.setAttitude(_attitude.rollDeg(), _attitude.pitchDeg());
    _flightState.setImuSample(accel, gyro);
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin != INT1_Pin) {
        return;
    }
    _flightState.incrementImuIntCount();

    static uint32_t lastTickMs = 0;
    const uint32_t nowMs = HAL_GetTick();
    s_pendingDtSeconds = (lastTickMs == 0) ? 0.0f : (nowMs - lastTickMs) / 1000.0f;
    lastTickMs = nowMs;

    _imu.startReadRawDma();
}



extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    _imu.onDmaComplete();
}

extern "C" void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    _imu.onDmaError();
}


extern "C" void app_init(void)
{
    _LED_Blue_On;
    _uartCrsf.init();
    _imu.setSampleReadyCallback(&onImuSampleReady);

    if (!_imu.init()) {
        LOG("lsm6ds3 init failed, who=0x%02X (bad WHO_AM_I / SPI wiring?)", _imu.whoAmI());
    } else {
        LOG("lsm6ds3 init ok, who=0x%02X", _imu.whoAmI());
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
    const AttitudeSample att = _flightState.attitude();
    uint16_t channelsUs[FlightState::kChannelCount];
    _flightState.channelsUs(channelsUs);
    LOG("R:%.1f,P:%.1f,AL:%.1f,AR:%.1f,C1:%u,C2:%u,C3:%u,C4:%u,C5:%u,C6:%u,C7:%u,C8:%u",
        att.rollDeg, att.pitchDeg, att.rollDeg, -att.rollDeg,
        channelsUs[0], channelsUs[1], channelsUs[2], channelsUs[3],
        channelsUs[4], channelsUs[5], channelsUs[6], channelsUs[7]);

    if (_uartCrsf.frameReady()) {
        if (_uartCrsf.frameType() == kCrsfFrameRcChannelsPacked) {
            uint16_t channels[kCrsfChannelCount];
            crsf_parse_channels(_uartCrsf.framePayload(), channels);

            uint16_t decodedUs[kCrsfChannelCount];
            for (uint8_t i = 0; i < kCrsfChannelCount; i++) {
                decodedUs[i] = crsf_to_pwm_us(channels[i]);
            }
            _flightState.setChannelsUs(decodedUs);
        }
        _uartCrsf.consumeFrame();
    }

    _LED_Blue_Toggle;
    HAL_Delay(kBlinkPeriodMs);
}
