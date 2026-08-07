#ifndef UART_H
#define UART_H

#include "define.h"

#define MAX_UARTS  4

class UartBase {
public:

    UartBase(UART_HandleTypeDef *huart) : _huart(huart) {};

    void init();
    void localHandle();

    static void handleRxCallback(UART_HandleTypeDef *huart);
    

protected:
    UART_HandleTypeDef* _huart;
    

private:
    
    bool startReceiveIT();

    uint8_t _rxByte;

    uint8_t _rxBufferNum = 0;
    uint8_t _rxBuffer[32];
    static uint8_t _uartNum;
    static UartBase* _instances[MAX_UARTS];
};


#endif // UART_H