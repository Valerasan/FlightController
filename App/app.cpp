#include "app.h"
#include "stm32f4xx_hal.h"


constexpr uint32_t kBlinkPeriodMs = 150;


extern "C" void app_init(void)
{
    _LED_Blue_On;
}

extern "C" void app_loop(void)
{
    _LED_Blue_Toggle;
    HAL_Delay(kBlinkPeriodMs);
}
