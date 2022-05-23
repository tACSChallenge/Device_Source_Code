/*
 * Reader.h
 *
 *  Created on: Jan 6, 2019
 *      Author: David Wang
 */

#ifndef _READER_H_
#define _READER_H_

#include <Arduino.h>
#include "Interface/InterfaceTypes.h"
#include "Interface/CommandCodes.h"
#include "ErrorHandling/ErrVal.h"

class Reader {
public:
  virtual ~Reader(){}
  virtual void reset() = 0;
  virtual ErrVal<COMM_CODE_T> readCC() = 0;
  virtual ErrVal<unsigned int> readUINT(size_t bitSize=DEFAULT_UINT_BITS) = 0;
  virtual ErrVal<uint8_t> readUINT8() = 0;
  virtual ErrVal<uint16_t> readUINT16() = 0;
  virtual ErrVal<uint32_t> readUINT32() = 0;
  virtual ErrVal<uint64_t> readUINT64() = 0;
  virtual ErrVal<int> readINT(size_t bitSize=DEFAULT_INT_BITS) = 0;
  virtual ErrVal<int8_t> readINT8() = 0;
  virtual ErrVal<int16_t> readINT16() = 0;
  virtual ErrVal<int32_t> readINT32() = 0;
  virtual ErrVal<int64_t> readINT64() = 0;
  virtual ErrVal<float> readFLOAT() = 0;
  virtual ErrVal<double> readDOUBLE() = 0;
  virtual ErrVal<uint8_t> readSTRING(char* xbuf, int xsize) = 0;
  virtual ErrVal<boolean> readBOOL(const boolean oneBit=DEFAULT_BOOLEAN_ONE_BIT) = 0;
  virtual uint8_t* getBuffer() = 0;
  virtual size_t getIndex() = 0;
  virtual size_t getSize() = 0;
};


#endif // _READER_H_
