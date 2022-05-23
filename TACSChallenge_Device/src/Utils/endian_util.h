/*
 Code originally copied from:
 https://forum.arduino.cc/index.php?topic=254143.0
 on: October 24th, 2019

 Original Author: "Coding Badly"

 Modified by: David Wang
*/



#ifndef _ENDIAN_UTIL_H_
#define _ENDIAN_UTIL_H_

#include <Arduino.h>

#include <limits.h>
#include <stdint.h>

#if CHAR_BIT != 8
#error "unsupported char size"
#endif

enum
{
    O32_LITTLE_ENDIAN = 0x03020100ul,
    O32_BIG_ENDIAN = 0x00010203ul,
    O32_PDP_ENDIAN = 0x01000302ul
};

enum ENDIAN{
    ENDIAN_LITTLE,
    ENDIAN_BIG,
    ENDIAN_PDP,
    ENDIAN_OTHER
};

ENDIAN getEndian();



#endif // _ENDIAN_UTIL_H_