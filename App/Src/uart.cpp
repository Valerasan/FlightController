#include "uart.h"

UartBase* UartBase::_instances[MAX_UARTS] = {nullptr};
uint8_t UartBase::_uartNum = 0;

void UartBase::init() {
    //TODO: add regoster
    _instances[_uartNum++] = this;
    startReceiveIT();
}


HAL_StatusTypeDef UartBase::sendMessage(const uint8_t *pData, uint16_t Size) {
    if(!_TxReady) {
        return HAL_BUSY;
    }
    return HAL_UART_Transmit_IT(_huart, pData, Size);
}

bool UartBase::startReceiveIT()
{
    return HAL_UART_Receive_IT(_huart, &_rxByte, 1) == HAL_OK;
}


void UartBase::localRxHandle() {
    parseByte(_rxByte);
    startReceiveIT();
}

void UartBase::localTxHandle() {
    _TxReady = 1;
}


void UartBase::handleRxCallback(UART_HandleTypeDef *huart) {
    for (uint8_t i = 0; i < MAX_UARTS; i++) {
        if(_instances[i] != nullptr && _instances[i]->_huart == huart){
            _instances[i]->localRxHandle();
            break;
        }
    }
}

void UartBase::handleTxCallback(UART_HandleTypeDef *huart) {
    for (uint8_t i = 0; i < MAX_UARTS; i++) {
        if(_instances[i] != nullptr && _instances[i]->_huart == huart){
            _instances[i]->localTxHandle();
            break;
        }
    }
}


extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    UartBase::handleRxCallback(huart);
}


extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    UartBase::handleTxCallback(huart);
}