#ifndef _DEVICE_LOG_H_
#define _DEVICE_LOG_H_

#include <Arduino.h>
#include "return_codes.h"

#define MAX_DEVICE_LOG_CHAR_LENGTH 200

enum DLOG {DLOG_NONE=0, DLOG_ERROR, DLOG_WARN, DLOG_INFO, DLOG_DEBUG, DLOG_VERBOSE};

extern uint8_t g_runtime_dlog_level;

void set_dlog_level(uint8_t dlog);
void dlog(uint8_t rc_code, char const *format, ...);

#define DLOG_FORMAT(format)  "(%lu)%s: " format 

#define DLOG_LEVEL(level, tag, format, ...) do {                     \
        if      (level==DLOG_ERROR )     { dlog(RC_ERROR  , DLOG_FORMAT(format), millis(), tag, ##__VA_ARGS__); } \
        else if (level==DLOG_WARN )      { dlog(RC_WARN   , DLOG_FORMAT(format), millis(), tag, ##__VA_ARGS__); } \
        else if (level==DLOG_DEBUG )     { dlog(RC_DEBUG  , DLOG_FORMAT(format), millis(), tag, ##__VA_ARGS__); } \
        else if (level==DLOG_VERBOSE )   { dlog(RC_VERBOSE, DLOG_FORMAT(format), millis(), tag, ##__VA_ARGS__); } \
        else                             { dlog(RC_INFO   , DLOG_FORMAT(format), millis(), tag, ##__VA_ARGS__); } \
    } while(0)

#define DLOG_LEVEL_RUNTIME(level, tag, format, ...) do {               \
        if ( g_runtime_dlog_level >= level ) DLOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
    } while(0)

#define DLOGE( tag, format, ... ) DLOG_LEVEL_RUNTIME(DLOG_ERROR,   tag, format, ##__VA_ARGS__)
#define DLOGW( tag, format, ... ) DLOG_LEVEL_RUNTIME(DLOG_WARN,    tag, format, ##__VA_ARGS__)
#define DLOGI( tag, format, ... ) DLOG_LEVEL_RUNTIME(DLOG_INFO,    tag, format, ##__VA_ARGS__)
#define DLOGD( tag, format, ... ) DLOG_LEVEL_RUNTIME(DLOG_DEBUG,   tag, format, ##__VA_ARGS__)
#define DLOGV( tag, format, ... ) DLOG_LEVEL_RUNTIME(DLOG_VERBOSE, tag, format, ##__VA_ARGS__)



#endif