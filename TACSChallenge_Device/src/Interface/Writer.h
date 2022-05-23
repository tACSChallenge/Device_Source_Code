/*
 * Writer.h
 *
 *  Created on: Jan 6, 2019
 *      Author: David Wang
 */

#ifndef _WRITER_H_
#define _WRITER_H_

#include <Arduino.h>
#include "Interface/InterfaceTypes.h"
#include "Interface/CommandCodes.h"

class Writer {
public:
  virtual ~Writer(){}
  virtual void reset() = 0;
  virtual boolean writeCC(COMM_CODE_T cc) = 0;
  virtual boolean writeUINT(const unsigned int val, const size_t bitSize=DEFAULT_UINT_BITS) = 0;
  virtual boolean writeUINT8(const uint8_t val) = 0;
  virtual boolean writeUINT16(const uint16_t val) = 0;
  virtual boolean writeUINT32(const uint32_t val) = 0;
  virtual boolean writeUINT64(const uint64_t val) = 0;
  virtual boolean writeINT(const int val, const size_t bitSize=DEFAULT_INT_BITS) = 0;
  virtual boolean writeINT8(const int8_t val) = 0;
  virtual boolean writeINT16(const int16_t val) = 0;
  virtual boolean writeINT32(const int32_t val) = 0;
  virtual boolean writeINT64(const int64_t val) = 0;
  virtual boolean writeFLOAT(const float val) = 0;
  virtual boolean writeDOUBLE(const double val) = 0;
  virtual boolean writeSTRING(char* str) = 0;
  virtual boolean writeSTRING(char* str, const size_t str_size) = 0;
  virtual boolean writeBOOL(const boolean val, const boolean oneBit=DEFAULT_BOOLEAN_ONE_BIT) = 0;
  virtual size_t getSize() = 0;
  virtual size_t getCapacity() = 0;
  virtual boolean send();
};


#endif // _WRITER_H_
