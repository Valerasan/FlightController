#include "uart.h"

#include <cstring>

UartBase* UartBase::_instances[MAX_UARTS] = {nullptr};
uint8_t UartBase::_uartNum = 0;

uint8_t crsf_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? uint8_t((crc << 1) ^ 0xD5) : uint8_t(crc << 1);
        }
    }
    return crc;
}

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
    parseByte(_rxByte);
    HAL_UART_Receive_IT(_huart, &_rxByte, 1);
}

 void UartBase::parseByte(uint8_t byte) {
    switch (_state) {
        case ParseState::WaitSync: 
            if(byte == CRSF_SYNC_BYTE) {
                 _state = ParseState::WaitLen;
            }
            break;
        case ParseState::WaitLen:
            if (byte >= 2 && byte <= sizeof(_frame)) {
                _frameLen = byte;
                _frameIdx = 0;
                _state = ParseState::WaitData;
            } else {
                _state = ParseState::WaitSync;  
            }
            break;
        case ParseState::WaitData:
            _frame[_frameIdx++] = byte;
            _frameLen--;
            if (_frameLen == 0) {
                uint8_t crc = crsf_crc8(_frame, _frameIdx - 1);
                if (crc == _frame[_frameIdx - 1]) {
                    memcpy(_readyFrame, _frame, _frameIdx);
                    _readyLen = _frameIdx;
                    _frameReady = true;
                }
                _state = ParseState::WaitSync;
            }
            break;

    }
 }

void UartBase::handleRxCallback(UART_HandleTypeDef *huart) {
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