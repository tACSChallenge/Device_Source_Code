/*
 * BitCopy.cpp
 *
 *  Created on: Jan 2, 2019
 *      Author: david
 */

#include "BitCopy.h"


/**
 * Copies 'numBits' from an offset of 'srcTotBitOff' in the 'src' buffer
 * to an offset of 'dstTotBitOff' in the 'dst' buffer. The offsets are
 * counted from the LSB. Each buffer is an array of bytes, with index 0
 * being the LSB and the highest index being the MSB.
 *
 * @param dst: Destination buffer
 * @param src: Source buffer
 * @param numBits: Number of bits to copy from src to dst
 * @params dstTotBitOff: Offset in destination buffer
 * @params srcTotBitOff: Offset in the source buffer
 */
void copyBitsLittleEndian(uint8_t* dst, const uint8_t* src,
        size_t numBits,
        size_t dstTotBitOff, size_t srcTotBitOff) {
  while(numBits>0){
    uint8_t dstBitOff = dstTotBitOff%8U;
    uint8_t srcBitOff = srcTotBitOff%8U;
    size_t dstByteOff = dstTotBitOff/8U;
    size_t srcByteOff = srcTotBitOff/8U;
    uint8_t numBits2Copy = min(numBits,min(8U-dstBitOff,8U-srcBitOff));
    // copy bits
    copyBitsHelper(dst+dstByteOff,src+srcByteOff,numBits2Copy,dstBitOff,srcBitOff);
    // increment
    dstTotBitOff += numBits2Copy;
    srcTotBitOff += numBits2Copy;
    numBits -= numBits2Copy;
  }
}

/**
 * Copies 'numBits' from an offset of 'srcTotBitOff' in the 'src' buffer
 * to an offset of 'dstTotBitOff' in the 'dst' buffer. The offsets are
 * counted from the LSB. Each buffer is an array of bytes, with index 0
 * being the MSB and the highest index being the LSB.
 */
void copyBitsBigEndian(uint8_t* dst, const uint8_t* src,
        size_t numBits,
        size_t dstTotBitOff, size_t srcTotBitOff,
        size_t dstNumBytes, size_t srcNumBytes) {
//  Serial.println("copyBitsBigEndian");
//  Serial.print("numBits: "); Serial.println(numBits);
//  Serial.print("dstTotBitOff: "); Serial.println(dstTotBitOff);
//  Serial.print("srcTotBitOff: "); Serial.println(srcTotBitOff);
//  Serial.print("dstNumBytes: "); Serial.println(dstNumBytes);
//  Serial.print("srcNumBytes: "); Serial.println(srcNumBytes);

  while(numBits>0){
    uint8_t dstBitOff = dstTotBitOff%8U;
    uint8_t srcBitOff = srcTotBitOff%8U;
    size_t dstByteOff = dstNumBytes-1-dstTotBitOff/8U;
    size_t srcByteOff = srcNumBytes-1-srcTotBitOff/8U;
    uint8_t numBits2Copy = min(numBits,min(8U-dstBitOff,8U-srcBitOff));
    // copy bits
    copyBitsHelper(dst+dstByteOff,src+srcByteOff,numBits2Copy,dstBitOff,srcBitOff);
    // increment
    dstTotBitOff += numBits2Copy;
    srcTotBitOff += numBits2Copy;
    numBits -= numBits2Copy;
  }
}

/**
 * Helper function for "copyBits".
 * Copies 'numBits' from the first byte in 'src' to the first byte in 'dst'.
 * The bit offsets are given from the LSB in each byte.
 * The caller must ensure that "dstBitOff + numBits <= 8" and "srcBitOff + numBits <= 8"
 *
 * @param dst: Pointer to destination byte
 * @param src: Pointer to source byte
 * @param numBits : The number of bits to copy
 * @param dstBitOff: Number of low bits to select from src
 * @param srcBitOff: Offset from msb in dst
 */
void copyBitsHelper(uint8_t *dst, const uint8_t *src,
    uint8_t numBits, uint8_t dstBitOff, uint8_t srcBitOff) {
//  Serial.println("copyBitsHelper");
//  Serial.print("numBits: "); Serial.println(numBits);
//  Serial.print("dstBitOff: "); Serial.println(dstBitOff);
//  Serial.print("srcBitOff: "); Serial.println(srcBitOff);

  *dst &= ~BITMASKU8_RANGE(dstBitOff,numBits);
  uint8_t copiedBits;
  if(dstBitOff >= srcBitOff){
    copiedBits = (*src) << (dstBitOff-srcBitOff);
  }else{
    copiedBits = (*src) >> (srcBitOff-dstBitOff);
  }
  copiedBits &= BITMASKU8_RANGE(dstBitOff,numBits);
  *dst |= copiedBits;
}


void printBits(uint8_t myByte){
 for(uint8_t mask = 0x80; mask; mask >>= 1){
   if(mask & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}

void printBytes(uint8_t* myBytes, size_t size, boolean littleEndian){
  for(size_t s=0; s<size; s++){
    if(littleEndian){
      printBits(myBytes[size-s-1]);
    }else{
      printBits(myBytes[s]);
    }
    Serial.print(" ");
  }
}

// void reverse(uint8_t* myBytes, size_t size)
// {
//     size_t start=0;
//     size_t end = size;
//     while (start < end)
//     {
//       uint8_t temp = myBytes[start];
//       myBytes[start] = myBytes[end];
//       myBytes[end] = temp;
//       start++;
//       end--;
//     }
// }

void reverse(uint8_t *start, int size) {
    unsigned char *lo = start;
    unsigned char *hi = start + size - 1;
    unsigned char swap;
    while (lo < hi) {
        swap = *lo;
        *lo++ = *hi;
        *hi-- = swap;
    }
}

boolean isEqual(uint8_t* bytes1, uint8_t* bytes2, size_t size){
  for(size_t i=0; i<size; i++){
    if(bytes1[i]!=bytes2[i]){
      return false;
    }
  }
  return true;
}




