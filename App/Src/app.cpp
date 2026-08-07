#include "app.h"
#include "log.h"
#include "stm32f4xx_hal.h"
#include "uart.h"

constexpr uint32_t kBlinkPeriodMs = 150;
extern UART_HandleTypeDef huart1;
UartBase _uartCrsf(&huart1);

extern "C" void app_init(void)
{
    _LED_Blue_On;
    _uartCrsf.init();
}

extern "C" void app_loop(void)
{
    _LED_Blue_Toggle;
    HAL_Delay(kBlinkPeriodMs);
}
