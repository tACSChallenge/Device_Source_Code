#ifndef _TEXT_PARSER_H_
#define _TEXT_PARSER_H_

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

#include "Interface/CommandCodes.h"
#include "Interface/Reader.h"
#include "ErrorHandling/ErrVal.h"
#include "ErrorHandling/ErrCodes.h"

/**********************************************************************/
// READ LINE FUNCTION

// A timeout is indiciated by a negative number returned.
int readLine(Stream &S, char *buffer, int len, unsigned long timeout_ms);
int readLine(Stream &S, char *buffer, int len);

/**********************************************************************/
// CONVENIENCE FUNCTIONS

void printSubstring(char *string, int start_index, int end_index);
boolean isEqual(char *string1, int start_index, int end_index, char *string2);

/**********************************************************************/
// PARSE TEXT COMMAND STRUCT

class TextReader : public Reader{
private:
  char* buffer_ = NULL; // this buffer MUST be NULL terminated
  size_t size_ = 0;     // the size of the buffer does NOT count NULL terminator
  size_t index_ = 0;
public:
  TextReader(){
  }
  TextReader(char* buffer, const size_t size): buffer_(buffer), size_(size), index_(0){
  }
  virtual ~TextReader(){}
  void reset(){
    index_ = 0;
  }
  void init(const uint8_t* buffer, const size_t size) {
	  buffer_ = (char*) buffer;
	  buffer_[size] = '\0'; // force NULL termination
    size_ = size;
    index_ = 0;
  }
  ErrVal<COMM_CODE_T> readCC(){
    char xbuf[COMM_CODE_MAX_CHAR];
    readSTRING(xbuf, COMM_CODE_MAX_CHAR);
    CommandCodes cmd_code = str2enum(xbuf);
    if(cmd_code==CC_NONE){
      return error(ERROR_CMD_CODE_NOT_FOUND);
    }else{
      return value<COMM_CODE_T>(cmd_code);
    }
  }
  ErrVal<unsigned int> readUINT(size_t bitSize=DEFAULT_UINT_BITS){return convert<unsigned int,int>(readINT(0));}
  ErrVal<uint8_t> readUINT8(){return  convert<uint8_t,int>(readINT(0));}
  ErrVal<uint16_t> readUINT16(){return convert<uint16_t,int>(readINT(0));}
  ErrVal<uint32_t> readUINT32(){return convert<uint32_t,int>(readINT(0));}
  ErrVal<uint64_t> readUINT64(){return convert<uint64_t,int>(readINT(0));}
  ErrVal<int> readINT(size_t bitSize=DEFAULT_INT_BITS) {
    unsigned int start_index;
    unsigned int end_index;
    if ( readSTRING(start_index, end_index) ){
      int val = atoi(buffer_ + start_index);
      return value<int>(val);
    }else{
      return error(ERROR_INDEX_OUT_OF_BOUNDS);
    }
  }
  ErrVal<int8_t> readINT8(){return convert<int8_t,int>(readINT(0));}
  ErrVal<int16_t> readINT16(){return convert<int16_t,int>(readINT(0));}
  ErrVal<int32_t> readINT32(){return convert<int32_t,int>(readINT(0));}
  ErrVal<int64_t> readINT64(){return convert<int64_t,int>(readINT(0));}
  ErrVal<float> readFLOAT() {
    unsigned int start_index;
    unsigned int end_index;
    if(readSTRING(start_index, end_index)){
      float val = atof(buffer_ + start_index);
      return value<float>(val);
    }else{
      return error(ERROR_INDEX_OUT_OF_BOUNDS);
    }
  }
  ErrVal<double> readDOUBLE() {return convert<double,float>(readFLOAT());}
  /*
   * Reads a string from the input buffer.
   * Given:
   *  buffer_ = "hi,david" (with null terminal)
   *  index_ = 3
   * The returned string will be:
   *  start_index = 3
   *  end_index = 8
   */
  bool readSTRING(unsigned int &start_index, unsigned int &end_index) {
    if (index_ >= size_)
      return false;
    start_index = index_;
    end_index = index_;
    while(true){
      if(index_ >= size_){
        end_index = size_;
        break;
      }
      if (buffer_[index_] == ',' || buffer_[index_] == '\0') {
        end_index = index_;
        index_++;
        break;
      }
      index_++;
    }
    return true;
  }
  /*
   * Reads a string from the input buffer.
   * Given:
   *  buffer_ = "hi,david" (with null terminal char)
   *  index_ = 3
   * The returned string will be:
   *  str = "david" (with null terminal char)
   *  size = 6
   */
  ErrVal<uint8_t> readSTRING(char* xbuf, int xsize){
    unsigned int start_index = 0;
    unsigned int end_index = 0;
    if ( !readSTRING(start_index, end_index) ){
      return error(ERROR_INDEX_OUT_OF_BOUNDS);
    }
    int size_read = end_index-start_index; // does not include NULL term char
    if(size_read < xsize){
      // the external buffer is large enough
      memcpy(xbuf,buffer_+start_index,size_read);
      xbuf[size_read] = '\0';
      return value((uint8_t)size_read);
    }else{
      // the external buffer is not large enough
      memcpy(xbuf,buffer_+start_index,xsize-1);
      xbuf[xsize-1] = '\0';
      return value((uint8_t)(xsize-1));
    }
  }


  ErrVal<boolean> readBOOL(const boolean oneBit=DEFAULT_BOOLEAN_ONE_BIT) {
    unsigned int start_index;
    unsigned int end_index;
    readSTRING(start_index, end_index);
    char c = buffer_[start_index];
    switch (c) {
      case 't':
      case 'T':
      case '1':
      case '*':
        return value(true);
      case 'f':
      case 'F':
      case '0':
      case '_':
      default:
        return value(false);
    }
    return value(false);
  }

  uint8_t* getBuffer(){
    return (uint8_t*) buffer_;
  }
  size_t getIndex(){
    return index_;
  }
  size_t getSize(){
    return size_;
  }


};

#endif // _TEXT_PARSER_H_
