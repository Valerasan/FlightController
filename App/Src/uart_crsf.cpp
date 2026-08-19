#include "uart_crsf.h"

#include <cstring>
#include <cmath>
#include "flight_state.h"

namespace {
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
}

void UartCrsf::parseByte(uint8_t byte) {
    switch (_state) {
        case CrsfParseState::WaitSync:
            if (byte == CRSF_SYNC_BYTE) {
                _state = CrsfParseState::WaitLen;
            }
            break;
        case CrsfParseState::WaitLen:
            if (byte >= 2 && byte <= sizeof(_frame)) {
                _frameLen = byte;
                _frameIdx = 0;
                _state = CrsfParseState::WaitData;
            } else {
                _state = CrsfParseState::WaitSync;
            }
            break;
        case CrsfParseState::WaitData:
            _frame[_frameIdx++] = byte;
            _frameLen--;
            if (_frameLen == 0) {
                uint8_t crc = crsf_crc8(_frame, _frameIdx - 1);
                if (crc == _frame[_frameIdx - 1]) {
                    memcpy(_readyFrame, _frame, _frameIdx);
                    _readyLen = _frameIdx;
                    _frameReady = true;
                }
                _state = CrsfParseState::WaitSync;
            }
            break;
    }
}


void UartCrsf::sendAttitude() {
    AttitudeSample attitude = _flightState.attitude();
    float pitch_rad = attitude.pitchDeg  * (M_PI / 180.0f);
    int16_t pitch_i16 = (int16_t)(pitch_rad * 10000.0f);  
    float roll_rad = attitude.rollDeg * (M_PI / 180.0f);
    int16_t roll_i16  = (int16_t)(roll_rad  * 10000.0f);
    int16_t yaw_i16   = 0;

    uint8_t data[6];
    data[0] = (uint8_t)((pitch_i16 >> 8) & 0xFF);
    data[1] = (uint8_t)pitch_i16 & 0xFF;

    data[2] = (uint8_t)((roll_i16 >> 8) & 0xFF);
    data[3] = (uint8_t)roll_i16 & 0xFF;

    data[4] = (uint8_t)((yaw_i16 >> 8) & 0xFF);
    data[5] = (uint8_t)yaw_i16 & 0xFF;
    uint16_t len = makeCRSFMessage(_txFrame, data, sizeof(data));

    sendMessage(_txFrame, len);
    
}

uint16_t UartCrsf::makeCRSFMessage(uint8_t *outFrame, const uint8_t *pData, uint16_t Size) {
    // TODO: refactor
    uint8_t index = 0;
    outFrame[index++] = CRSF_SYNC_BYTE;
    
    outFrame[index++] = Size+2;
    //TODO: extract in variable (frame type )
    outFrame[index++] = 0x1E; // Attitude
    
    for(uint8_t i = 0; i < Size; i++) {
        outFrame[index++] = pData[i];
    }

    outFrame[index++] = crsf_crc8(&outFrame[2], Size + 1);

    return index;
}
