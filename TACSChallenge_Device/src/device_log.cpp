
#include <Arduino.h>
#include "return_codes.h"
#include "Interface/InterfaceAdapter.h"
#include "device_log.h"


extern InterfaceAdapter itf;
uint8_t g_runtime_dlog_level = DLOG_INFO;


void set_dlog_level(uint8_t dlog)
{
    g_runtime_dlog_level = dlog;
}

void dlog(uint8_t rc_code, char const *format, ...)
{
    noInterrupts();
    
    // format the data with a temporary buffer
    char buffer[MAX_DEVICE_LOG_CHAR_LENGTH];
    va_list arg;
    va_start(arg, format);
    size_t buffer_size = vsnprintf ( buffer, MAX_DEVICE_LOG_CHAR_LENGTH, format, arg );
    va_end(arg);
    // Send data
    Writer *w = itf.getWriter();
    w->writeUINT8(rc_code);
    w->writeSTRING(buffer, min(MAX_DEVICE_LOG_CHAR_LENGTH, buffer_size));
    w->send();
    
    interrupts();
}
