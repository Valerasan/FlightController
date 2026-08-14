#include "uart_crsf.h"

#include <cstring>

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
