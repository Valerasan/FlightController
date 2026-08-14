#ifndef UART_CRSF_H
#define UART_CRSF_H

#include "uart.h"

#define CRSF_SYNC_BYTE    0xC8
#define CRSF_MAX_PAYLOAD  60

enum class CrsfParseState : uint8_t
{
    WaitSync,
    WaitLen,
    WaitData
};

class UartCrsf : public UartBase {
public:

    UartCrsf(UART_HandleTypeDef *huart) : UartBase(huart) {};

    bool frameReady() const { return _frameReady; }
    uint8_t frameType() const { return _readyFrame[0]; }
    const uint8_t* framePayload() const { return &_readyFrame[1]; }
    uint8_t framePayloadLen() const { return _readyLen - 2; }
    void consumeFrame() { _frameReady = false; }

private:
    void parseByte(uint8_t byte) override;

    CrsfParseState _state = CrsfParseState::WaitSync;

    uint8_t _frame[64];
    uint8_t _frameLen = 0;
    uint8_t _frameIdx = 0;

    volatile bool _frameReady = false;
    uint8_t _readyFrame[64];
    uint8_t _readyLen = 0;
};


#endif // UART_CRSF_H
