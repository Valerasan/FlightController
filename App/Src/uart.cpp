#include "uart.h"

UartBase* UartBase::_instances[MAX_UARTS] = {nullptr};
uint8_t UartBase::_uartNum = 0;

void UartBase::init() {
    //TODO: add regoster
    _instances[_uartNum++] = this;
    startReceiveIT();
}


bool UartBase::startReceiveIT()
{
    return HAL_UART_Receive_IT(_huart, &_rxByte, 1) == HAL_OK;
}


void UartBase::localHandle() {
    _rxBuffer[_rxBufferNum] = _rxByte;
    _rxBufferNum = (_rxBufferNum + 1) % 32;
    HAL_UART_Receive_IT(_huart, &_rxByte, 1);
}

void UartBase::handleRxCallback(UART_HandleTypeDef *huart) {
    LOG("recive");
    for (uint8_t i = 0; i < MAX_UARTS; i++) {
        if(_instances[i] != nullptr && _instances[i]->_huart == huart){
            _instances[i]->localHandle();
            break;
        }
    }
}


extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    UartBase::handleRxCallback(huart);
}