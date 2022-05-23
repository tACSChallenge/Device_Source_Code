/*
 * TextWriter.h
 *
 *  Created on: Jan 6, 2019
 *      Author: David Wang
 */

#ifndef _TEXTWRITER_H_
#define _TEXTWRITER_H_

#include <Arduino.h>

#include "ErrorHandling/ErrVal.h"
#include "Interface/Reader.h"
#include "Interface/Writer.h"

class TextWriter : public Writer {
private:
  Print* printer_ = NULL;
public:
  TextWriter(){
  }
  TextWriter(Print* printer){
    init(printer);
  }
  virtual ~TextWriter(){}
  void init(Print* printer){
    printer_ = printer;
  }
  void reset(){
    // does nothing
  }
  template <class T>
  boolean write(T val){
    printer_->print(val); printer_->print(",");
    return true;
  }
  boolean writeCC(COMM_CODE_T cc){return writeSTRING((char*)enum2str(cc));}
  boolean writeUINT(const unsigned int val, const size_t bitSize=DEFAULT_UINT_BITS){ return write(val); }
  boolean writeUINT8(const uint8_t val){ return write(val); }
  boolean writeUINT16(const uint16_t val){ return write(val); }
  boolean writeUINT32(const uint32_t val){ return write(val); }
  boolean writeUINT64(const uint64_t val){ return write((unsigned long)val); }
  boolean writeINT(const int val, const size_t bitSize=DEFAULT_INT_BITS){ return write(val); }
  boolean writeINT8(const int8_t val){ return write(val); }
  boolean writeINT16(const int16_t val){ return write(val); }
  boolean writeINT32(const int32_t val){ return write(val); }
  boolean writeINT64(const int64_t val){ return write((long)val); }
  boolean writeFLOAT(const float val){ printer_->print(val,6); printer_->print(","); return true; }
  boolean writeDOUBLE(const double val){ printer_->print(val,6); printer_->print(","); return true; }
  boolean writeSTRING(char* str){
    printer_->print(str);
    printer_->print(",");
    return true;
  }
  boolean writeSTRING(char* str, const size_t str_size){
    printer_->write(str,str_size);
    printer_->print(",");
    return true;
  }
  boolean writeBOOL(const boolean val, const boolean oneBit=DEFAULT_BOOLEAN_ONE_BIT){ return write(val); }
  size_t getSize(){ return 0; }
  size_t getCapacity(){ return 0; }
  boolean send() { printer_->println(); return true; }
};


#endif // _TEXTWRITER_H_
