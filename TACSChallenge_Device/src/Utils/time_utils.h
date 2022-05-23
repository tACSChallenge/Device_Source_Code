#ifndef _TIME_UTILS_H_
#define _TIME_UTILS_H_

#include "Arduino.h"

// Delays the processor by NanoSeconds.
// This code is an adaptation of Paul Stoffregen's implementation of 
// "delayMicroseconds" from the Arduino Teensy 3 "core_pins.h" code.
static inline void delayNanoseconds(uint32_t) __attribute__((always_inline, unused));
static inline void delayNanoseconds(uint32_t nsec)
{
#if F_CPU == 240000000
  uint32_t n = nsec * 80;
#elif F_CPU == 216000000
  uint32_t n = nsec * 72;
#elif F_CPU == 192000000
  uint32_t n = nsec * 64;
#elif F_CPU == 180000000
  uint32_t n = nsec * 60;
#elif F_CPU == 168000000
  uint32_t n = nsec * 56;
#elif F_CPU == 144000000
  uint32_t n = nsec * 48;
#elif F_CPU == 120000000
  uint32_t n = nsec * 40;
#elif F_CPU == 96000000
  uint32_t n = nsec << 5;
#elif F_CPU == 72000000
  uint32_t n = nsec * 24;
#elif F_CPU == 48000000
  uint32_t n = nsec << 4;
#elif F_CPU == 24000000
  uint32_t n = nsec << 3;
#elif F_CPU == 16000000
  uint32_t n = nsec << 2;
#elif F_CPU == 8000000
  uint32_t n = nsec << 1;
#elif F_CPU == 4000000
  uint32_t n = nsec;
#elif F_CPU == 2000000
  uint32_t n = nsec >> 1;
#endif
  n = n >> 10; // divide by 1024 (which is close to dividing by 1000)
    // changed because a delay of 1 micro Sec @ 2MHz will be 0
  if (n == 0) return;
  __asm__ volatile(
    "L_%=delayNanoseconds:"   "\n\t"
#if F_CPU < 24000000
    "nop"         "\n\t"
#endif
#ifdef KINETISL
    "sub    %0, #1"       "\n\t"
#else
    "subs   %0, #1"       "\n\t"
#endif
    "bne    L_%=delayNanoseconds"   "\n"
    : "+r" (n) :
  );
}



#endif //_TIME_UTILS_H_
