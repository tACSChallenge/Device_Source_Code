/*
 * BitCopy.h
 *
 *  Created on: Jan 2, 2019
 *      Author: david
 */

#ifndef _BITCOPY_H_
#define _BITCOPY_H_

#include <Arduino.h>

#define BITMASKU8(size) ((1U << (size)) - 1)
#define BITMASKU8_RANGE(start,size) (BITMASKU8(size) << (start))

void copyBitsHelper(uint8_t *dst, const uint8_t *src,
    uint8_t numBits, uint8_t dstBitOff, uint8_t srcBitOff);
void copyBitsLittleEndian(uint8_t* dst, const uint8_t* src,
        size_t numBits, size_t dstTotBitOff, size_t srcTotBitOff);
void copyBitsBigEndian(uint8_t* dst, const uint8_t* src,
        size_t numBits,
        size_t dstTotBitOff, size_t srcTotBitOff,
        size_t dstNumBytes, size_t srcNumBytes);

void printBits(uint8_t myByte);
void printBytes(uint8_t* myBytes, size_t size, boolean bigEndian = true);

void reverse(uint8_t *start, int size);
boolean isEqual(uint8_t* bytes1, uint8_t* bytes2, size_t size);


#endif // _BITCOPY_H_
