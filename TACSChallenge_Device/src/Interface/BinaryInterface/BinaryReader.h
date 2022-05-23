#ifndef _BINARY_PARSER_H_
#define _BINARY_PARSER_H_

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

#include "BitCopy.h"

#include "ErrorHandling/ErrVal.h"
#include "ErrorHandling/ErrCodes.h"
#include "Interface/Reader.h"
#include "Utils/print_util.h"
#include "Utils/endian_util.h"

/**********************************************************************/
// PARSE BINARY COMMAND STRUCT

class BinaryReader : public Reader {
private:
  uint8_t* buffer_ = NULL;
  size_t bitSize_ = 0;
  size_t bitIndex_ = 0;
  ENDIAN endian_;
public:
  BinaryReader(){
    endian_ = getEndian();
  }
  BinaryReader(const uint8_t* buffer, const size_t byteSize): buffer_((uint8_t*)buffer), bitSize_(8*byteSize), bitIndex_(0){
    endian_ = getEndian();
  }
  virtual ~BinaryReader(){}
  void reset(){
    bitIndex_ = 0;
  }
  void init(const uint8_t* buffer, const size_t byteSize) {
    buffer_ = (uint8_t*) buffer;
    bitSize_ = 8*byteSize;
    bitIndex_ = 0;
  }
  template <typename T>
  ErrVal<T> read(const size_t numBits){
    T val;
    if ( read((uint8_t*)&val,sizeof(T),numBits) ){
      // printHexArray(Serial, (uint8_t*)&val, ceil(numBits/8));
      // Serial.println();
      // int32_t example = 4;
      // printHexArray(Serial, (uint8_t*)&example, ceil(numBits/8));
      // Serial.println();
      // Serial.print("val: ");
      // Serial.println((int)val);
      return value<T>(val);
    }else{
      return error(ERROR_INDEX_OUT_OF_BOUNDS);
    }
  }
  boolean read(uint8_t* buf, const size_t bufBytes, const size_t numBits){
    return read(buf, bufBytes, numBits, endian_);
  }
  boolean read(uint8_t* buf, const size_t bufBytes, const size_t numBits, ENDIAN endian){
    if(bitIndex_+numBits <= bitSize_){
      if(numBits==0){
        // do nothing
      }else{
        if(bitIndex_%8==0 && numBits%8==0){
          // if the bit boundaries are respected, use memcpy
          memcpy(buf,buffer_+bitIndex_/8,numBits/8); // dest, source, byte_size
        }else{
          copyBitsLittleEndian(buf, buffer_, numBits, 0, bitIndex_); // bufBytes, srcNumBytes
        }
        bitIndex_ += numBits;
        switch(endian){
          case ENDIAN_LITTLE:
            // data is sent big endian, but the processor is little endian,
            // do nothing
            break;
          case ENDIAN_BIG:
            // data is sent big endian, and the processor is big endian,
            // need to reverse data order
            //reverse(buf,bufBytes);
            break;
          default:
            // quit, because we don't know what the endian is.
            return false;
        }
      }
      return true;
    }else{
      return false;
    }
  }
  ErrVal<COMM_CODE_T> readCC(){
     return readUINT8();
  }
  ErrVal<unsigned int> readUINT(size_t bitSize=DEFAULT_UINT_BITS){
    return read<unsigned int>(bitSize);
  }
  ErrVal<uint8_t> readUINT8(){
    return read<uint8_t>(1*8);
  }
  ErrVal<uint16_t> readUINT16(){
    return read<uint16_t>(2*8);
  }
  ErrVal<uint32_t> readUINT32(){
    return read<uint32_t>(4*8);
  }
  ErrVal<uint64_t> readUINT64(){
    return read<uint64_t>(8*8);
  }
  ErrVal<int> readINT(size_t bitSize=DEFAULT_INT_BITS){
    auto result = read<unsigned int>(bitSize);
    if(result.isError()){
      return error(result.getError());
    }else{
      unsigned int ui = result.getValue();
      if( ((ui >> (bitSize-1)) & 1U) == 0 ){
        // positive number
        return value<int>((int) ui);
      }else{
        // negative number, convert from 2s compliment
        ui = (~ui & BITMASKU8(bitSize)) + 1;
        return value<int>(-(int)ui);
      }
    }
  }
  ErrVal<int8_t> readINT8(){
    return read<int8_t>(1*8);
  }
  ErrVal<int16_t> readINT16(){
    return read<int16_t>(2*8);
  }
  ErrVal<int32_t> readINT32(){
    return read<int32_t>(4*8);
  }
  ErrVal<int64_t> readINT64(){
    return read<int64_t>(8*8);
  }
  ErrVal<float> readFLOAT(){
    return read<float>(4*8);
  }
  ErrVal<double> readDOUBLE(){
    return read<double>(8*8);
  }
  ErrVal<uint8_t> readSTRING(char* xbuf, int xsize){
    auto result = readUINT8();
    if(result.isError()){
      return error(ERROR_INDEX_OUT_OF_BOUNDS);
    }
    uint8_t str_size = result.getValue();
    if ( str_size < xsize) {
      if ( read((uint8_t*)xbuf, str_size, str_size*8) ){
        xbuf[str_size] = '\0';
        return value<uint8_t>(str_size);
      }else{
        bitIndex_ -= 8; // undo reading the bits from the call to "readUINT8".
        return error(ERROR_INDEX_OUT_OF_BOUNDS);
      }
    }else{
      if ( read((uint8_t*)xbuf, xsize-1, (xsize-1)*8) ){
        xbuf[xsize-1] = '\0';
        return value<uint8_t>(xsize-1);
      }else{
        bitIndex_ -= 8; // undo reading the bits from the call to "readUINT8".
        return error(ERROR_INDEX_OUT_OF_BOUNDS);
      }
    }
  }
  ErrVal<boolean> readBOOL(const boolean oneBit=DEFAULT_BOOLEAN_ONE_BIT){
    uint8_t numval = 0;
    if(oneBit){
      auto result = readUINT(1);
      if(result.isError()){
        return error(result.getError());
      }
      numval = result.getValue();
    }else{
      auto result = readUINT8();
      if(result.isError()){
        return error(result.getError());
      }
      numval = result.getValue();
    }
    return numval==0 ? value(false) : value(true);
  }


  uint8_t* getBuffer(){
    return buffer_;
  }
  size_t getBitIndex(){
    return bitIndex_;
  }
  size_t getBitSize(){
    return bitSize_;
  }
  size_t getIndex(){
    return ceil(bitIndex_/8);
  }
  size_t getSize(){
    return ceil(bitSize_/8);
  }
};


#endif // _BINARY_PARSER_H_
