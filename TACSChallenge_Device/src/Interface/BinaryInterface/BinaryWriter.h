#ifndef _BINARY_WRITER_H_
#define _BINARY_WRITER_H_

#include <Arduino.h>
#include "BitCopy.h"
#include "Interface/Reader.h"
#include "Interface/Writer.h"
#include "Interface/PacketSerial_Mod/src/PacketSerial_Mod.h"

class BinaryWriter : public Writer{
private:
  uint8_t* buffer_ = NULL;
  size_t bitCap_ = 0;
  size_t bitIndex_ = 0;
  PacketSerial* pSerial_ = NULL;
public:
  BinaryWriter(){
  }
  BinaryWriter(uint8_t* buffer, const size_t byteSize, PacketSerial* pSerial){
    init(buffer,byteSize,pSerial);
  }
  virtual ~BinaryWriter(){}
  void reset(){
    bitIndex_ = 0;
  }
  void init(uint8_t* buffer, const size_t byteSize, PacketSerial* pSerial) {
    buffer_ = buffer;
    bitCap_ = 8*byteSize;
    bitIndex_ = 0;
    pSerial_ = pSerial;
  }
  template <typename T>
  boolean write(const T val, const size_t numBits){
    if ( write((uint8_t*)&val,sizeof(T),numBits) ){
      return true;
    }else{
      return false;
    }
  }
  boolean write(uint8_t* buf, const size_t bufBytes, const size_t numBits){
    if(bitIndex_+numBits <= bitCap_){
      if(numBits==0){
        // do nothing
      }else if(bitIndex_%8==0 && numBits%8==0){
        // if the bit boundaries are respected, use memcpy
        memcpy(buffer_+bitIndex_/8,buf,numBits/8); // dest, source, byte_size
      }else{
        copyBitsLittleEndian(buffer_, buf, numBits, bitIndex_, 0); // , dstNumBytes, bufBytes
      }
      bitIndex_ += numBits;
      return true;
    }else{
      return false;
    }
  }
  boolean writeCC(COMM_CODE_T cc){
    return writeUINT8(cc);
  }
  boolean writeUINT(const unsigned int val, const size_t bitSize){
    return write<unsigned int>(val, bitSize);
  }
  boolean writeUINT8(const uint8_t val){
    return write(val,1*8);
  }
  boolean writeUINT16(const uint16_t val){
    return write(val,2*8);
  }
  boolean writeUINT32(const uint32_t val){
    return write(val,4*8);
  }
  boolean writeUINT64(const uint64_t val){
    return write(val,8*8);
  }
  boolean writeINT(const int val, const size_t bitSize){
    // Assuming "val" is stored as 2's compliment,
    // the write operation will truncate the sign extension.
    return write<unsigned int>((unsigned int)val, bitSize);
  }
  boolean writeINT8(const int8_t val){
    return write(val,1*8);
  }
  boolean writeINT16(const int16_t val){
    return write(val,2*8);
  }
  boolean writeINT32(const int32_t val){
    return write(val,4*8);
  }
  boolean writeINT64(const int64_t val){
    return write(val,8*8);
  }
  boolean writeFLOAT(const float val){
    return write(val,4*8);
  }
  boolean writeDOUBLE(const double val){
    return write(val,8*8);
  }
  boolean writeSTRING(char* str){
    return writeSTRING(str,strlen(str));
  }
  boolean writeSTRING(char* str, const size_t str_size){
    size_t bitIndexBackup = bitIndex_;
    if ((bitIndex_+8*(str_size+1)<=bitCap_) &&
        writeUINT8(str_size) &&
        write((uint8_t*) str, str_size, str_size*8)){
      return true;
    }else{
      bitIndex_ = bitIndexBackup;
      return false;
    }
  }
  boolean writeBOOL(const boolean val, const boolean oneBit=true){
    uint8_t numval = val ? 1 : 0;
    if(oneBit){
      return write<unsigned int>(numval, 1);
    }else{
      return writeUINT8(numval);
    }
  }

  size_t getBitSize(){
    return bitIndex_;
  }
  size_t getBitCapacity(){
    return bitCap_;
  }
  size_t getSize(){
    return ceil(bitIndex_/8.0);
  }
  size_t getCapacity(){
    return bitCap_/8;
  }
  boolean send(){
    pSerial_->send(buffer_,getSize());
    // reset the buffer
    bitIndex_ = 0;
    return true;
  }
};


#endif // _BINARY_WRITER_H_
