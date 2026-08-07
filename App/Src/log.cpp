#include "log.h"

#include <cstdarg>
#include <cstdio>
#include <cstdint>

#include "usbd_cdc_if.h"

namespace {
constexpr size_t kLogBufferSize = 128;
}

extern "C" void log_printf(const char *fmt, ...)
{
    char buffer[kLogBufferSize];

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len <= 0)
    {
        return;
    }
    if (static_cast<size_t>(len) >= sizeof(buffer))
    {
        len = sizeof(buffer) - 1;
    }

    CDC_Transmit_FS(reinterpret_cast<uint8_t *>(buffer), static_cast<uint16_t>(len));
}
