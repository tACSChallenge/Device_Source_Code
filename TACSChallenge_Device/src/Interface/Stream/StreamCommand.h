#ifndef _STREAM_COMMAND_H_
#define _STREAM_COMMAND_H_

#include <Arduino.h>
#include "Interface/Writer.h"
#include "Interface/TextInterface/TextReader.h"
#include "StreamCodes.h"


#include <functional>

class StreamCommandArray;

class StreamCommandWriter {
public:
  virtual ~StreamCommandWriter(){}
  virtual void write(Writer &w) = 0;
};

struct StreamCommand {
  uint16_t id_;
  char* cmd_;
  std::function<void(Writer &w)> write_;
  StreamCommand(): id_(SC_UNDEF), cmd_(NULL), write_(NULL){}
  void clear(){
    id_ = SC_UNDEF;
    cmd_ = NULL;
  }
};

class StreamCommandArray {
public:
  StreamCommand* strCmdArr_;
  uint16_t size_;
  uint16_t cap_;
  StreamCommandArray(StreamCommand* strCmdArr, uint16_t cap): strCmdArr_(strCmdArr), size_(0), cap_(cap) {
    memset(strCmdArr,0,sizeof(StreamCommand*)*cap_);
  }
  boolean set(uint16_t id, char* cmd, std::function<void(Writer &w)> write) {
    if(id>=0 && id<cap_){
      if(strCmdArr_[id].id_==SC_UNDEF){
        size_++;
      }
      strCmdArr_[id].id_ = id;
      strCmdArr_[id].cmd_ = cmd;
      strCmdArr_[id].write_ = write;
      return true;
    }
    return false;
  }
  boolean remove(uint16_t id){
    strCmdArr_[id].clear();
    return true;
  }
  inline StreamCommand* get(uint16_t id) {
    if(id>=0 && id<cap_ && strCmdArr_[id].id_!=SC_UNDEF){
      return &strCmdArr_[id];
    }
    return NULL;
  }
  inline StreamCommand* get(char* buf, size_t starti, size_t endi) {
    for (int sci = 0; sci < cap_; sci++) {
      StreamCommand* strCmd = strCmdArr_ + sci;
      if (strCmd->id_!=SC_UNDEF && isEqual(buf, starti, endi, strCmd->cmd_)) {
        return strCmd;
      }
    }
    return NULL;
  }
  inline boolean write(uint16_t id, Writer &w) {
    if(id>=0 && id<cap_ && strCmdArr_[id].id_!=SC_UNDEF){
      strCmdArr_[id].write_(w);
      return true;
    }
    return false;
  }
  inline uint16_t getSize() {
    return size_;
  }
};



#endif //_STREAM_COMMAND_H_
