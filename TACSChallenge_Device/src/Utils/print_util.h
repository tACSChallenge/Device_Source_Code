#ifndef _PRINT_UTIL_H_
#define _PRINT_UTIL_H_

#include <Arduino.h>

void printHexArray(Stream &stream, uint8_t* array, size_t size);

#endif // _PRINT_UTIL_H_