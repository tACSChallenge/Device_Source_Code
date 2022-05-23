#ifndef _BINARY_PARSER_H_
#define _BINARY_PARSER_H_

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

#include "ErrorHandling/ErrVal.h"
#include "BitCopy.h"

/**********************************************************************/
// PARSE BINARY COMMAND STRUCT

class BinaryParser {
public:
  uint8_t* buffer_;
  size_t bitIndex_;
  size_t bitSize_;
  BinaryParser(): bitIndex_(0), bitSize_(0){
  }
  void populate(const uint8_t* buffer, const size_t byteSize) {
    buffer_ = (uint8_t*) buffer;
    bitSize_ = 8*byteSize;
    bitIndex_ = 0;
  }
  template <typename T>
  ErrVal<T> next(const size_t numBits){
    T val;
    if(bitIndex_+numBits < bitSize_){
      if(bitIndex_%8==0 && numBits%8==0){
        // if the bit boundaries are respected, use memcpy
        memcpy(&val,buffer_+bitIndex_/8,numBits/8); // dest, source, byte_size
      }else{
        copyBitsBigEndian((uint8_t*)&val, buffer_, numBits, 0, bitIndex_, sizeof(T), bitSize_/8);
      }
      bitIndex_ += numBits;
      return value<T>(val);
    }else{
      return error(ERROR_INDEX_OUT_OF_BOUNDS);
    }
  }
  ErrVal<unsigned int> nextUINT(size_t bitSize){
    return next<unsigned int>(bitSize);
  }
  ErrVal<uint8_t> nextUINT8(){
    return next<uint8_t>(1*8);
  }
  ErrVal<uint16_t> nextUINT16(){
    return next<uint16_t>(2*8);
  }
  ErrVal<uint32_t> nextUINT32(){
    return next<uint32_t>(4*8);
  }
  ErrVal<uint64_t> nextUINT64(){
    return next<uint64_t>(8*8);
  }
  ErrVal<int> nextINT(size_t bitSize){
    auto result = next<unsigned int>(bitSize);
    if(result.isError()){
      return error(result.getError());
    }else{
      unsigned int ui = result.getValue();
      if( (ui >> (bitSize-1)) & 1U == 0 ){
        // positive number
        return value<int>((int) ui);
      }else{
        // negative number, convert from 2s compliment
        ui = (~ui & BITMASKU8(bitSize)) + 1;
        return value<int>(-(int)ui);
      }
    }
  }
  ErrVal<int8_t> nextINT8(){
    return next<int8_t>(1*8);
  }
  ErrVal<int16_t> nextINT16(){
    return next<int16_t>(2*8);
  }
  ErrVal<int32_t> nextINT32(){
    return next<int32_t>(4*8);
  }
  ErrVal<int64_t> nextINT64(){
    return next<int64_t>(8*8);
  }
  ErrVal<boolean> nextBOOLEAN(size_t bitSize){
    return next<boolean>(bitSize);
  }
  ErrVal<float> nextFLOAT(){
    return next<float>(4*8);
  }
  ErrVal<double> nextDOUBLE(){
    return next<double>(8*8);
  }
  ErrVal<uint8_t> nextSTRING(char* str){
    auto result = nextUINT8();
    if(result.isError()){
      return error(ERROR_INDEX_OUT_OF_BOUNDS);
    }
    uint8_t str_size = result.getValue();
    if(bitIndex_+str_size*8 < bitSize_){
      if(bitIndex_%8==0){
        // if the bit boundaries are respected, use memcpy
        memcpy(&str,buffer_+bitIndex_/8,str_size); // dest, source, byte_size
      }else{
        copyBitsLittleEndian((uint8_t*)&str, buffer_, str_size*8, 0, bitIndex_);
      }
      str[str_size] = '\0';
      bitIndex_ += str_size*8;
      return value<uint8_t>(str_size);
    }else{
      return error(ERROR_INDEX_OUT_OF_BOUNDS);
    }
  }
};


#endif // _BINARY_PARSER_H_
