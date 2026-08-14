#ifndef DEFINE__H
#define DEFINE__H

#include "main.h"
#include "log.h"

/* Plain printf() goes nowhere here - the build links with --specs=nosys.specs,
 * so newlib's _write() syscall is an empty stub. Route through log_printf()
 * (App/log.cpp), which writes to the USB CDC port via CDC_Transmit_FS
 * instead. Requires fmt to be a string literal (adjacent-literal concat). */
#define LOG(fmt, ...)  log_printf(fmt "\n", ##__VA_ARGS__)

#define _LED_Blue_On        HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, GPIO_PIN_SET);
#define _LED_Blue_Off       HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, GPIO_PIN_RESET);
#define _LED_Blue_Toggle    HAL_GPIO_TogglePin(LED_Blue_GPIO_Port, LED_Blue_Pin);



#endif // DEFINE__H