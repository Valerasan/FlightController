#ifndef UART_H
#define UART_H

#include "define.h"

#define MAX_UARTS  4

class UartBase {
public:

    UartBase(UART_HandleTypeDef *huart) : _huart(huart) {};
    virtual ~UartBase() = default;

    void init();
    static void handleRxCallback(UART_HandleTypeDef *huart);

protected:
    UART_HandleTypeDef* _huart;
    uint8_t _rxByte;

    bool startReceiveIT();
    virtual void parseByte(uint8_t byte) = 0;

private:
    void localHandle();

    static uint8_t _uartNum;
    static UartBase* _instances[MAX_UARTS];
};


#endif // UART_H
