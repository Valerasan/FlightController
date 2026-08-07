#ifndef DEFINE__H
#define DEFINE__H

#include "main.h"


#define LOG(...)  do { printf(__VA_ARGS__); printf("\n"); } while(0)

#define _LED_Blue_On        HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, GPIO_PIN_SET);
#define _LED_Blue_Off       HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, GPIO_PIN_RESET);
#define _LED_Blue_Toggle    HAL_GPIO_TogglePin(LED_Blue_GPIO_Port, LED_Blue_Pin);



#endif // DEFINE__H