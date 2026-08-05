#include "app.h"
#include "stm32f4xx_hal.h"

namespace {
constexpr uint16_t kLedPin = GPIO_PIN_13;
GPIO_TypeDef *const kLedPort = GPIOC;
constexpr uint32_t kBlinkPeriodMs = 150;
}

extern "C" void app_init(void)
{
    HAL_GPIO_WritePin(kLedPort, kLedPin, GPIO_PIN_SET);
}

extern "C" void app_loop(void)
{
    HAL_GPIO_TogglePin(kLedPort, kLedPin);
    HAL_Delay(kBlinkPeriodMs);
}
