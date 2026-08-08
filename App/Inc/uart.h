#ifndef UART_H
#define UART_H

#include "define.h"

#define MAX_UARTS  4
#define CRSF_SYNC_BYTE    0xC8
#define CRSF_MAX_PAYLOAD  60 


enum class ParseState : uint8_t 
{   
    WaitSync, 
    WaitLen, 
    WaitData 
};



class UartBase {
public:

    UartBase(UART_HandleTypeDef *huart) : _huart(huart) {};

    void init();
    void localHandle();
    void parseByte(uint8_t byte);
    static void handleRxCallback(UART_HandleTypeDef *huart);
    
    bool frameReady() const { return _frameReady; }
    uint8_t frameType() const { return _readyFrame[0]; }
    const uint8_t* framePayload() const { return &_readyFrame[1]; }
    uint8_t framePayloadLen() const { return _readyLen - 2; }
    void consumeFrame() { _frameReady = false; }

protected:
    UART_HandleTypeDef* _huart;
    

private:
    
    bool startReceiveIT();


    ParseState _state = ParseState::WaitSync;

    uint8_t _rxByte;

    uint8_t _rxBufferNum = 0;
    //uint8_t _rxBuffer[64];
    uint8_t _frame[64];      
    uint8_t _frameLen = 0;
    uint8_t _frameIdx = 0;

   
    volatile bool _frameReady = false;
    uint8_t _readyFrame[64];
    uint8_t _readyLen = 0;
    static uint8_t _uartNum;
    static UartBase* _instances[MAX_UARTS];
};


#endif // UART_H