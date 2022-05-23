#ifndef _STREAM_ELEMENT_H_
#define _STREAM_ELEMENT_H_

#include <Arduino.h>

class StreamElement {
public:
  char* txt_name_;
  uint8_t txt_code_;
  TextMessage(TextInterface** itfs, size_t itfs_size, const char* txt_name):
    TextMessage(txt_name) {
    // add command to all sets
    for(size_t i=0; i<itfs_size; i++){
      addToInterface(itfs[i]);
    }
  }
  TextMessage(TextInterface* itf, const char* txt_name):
    TextMessage(txt_name) {
    // add command to set
    addToInterface(itf);
  }
  TextMessage(const char* txt_name):
    txt_name_((char*)txt_name){
  // txt_code_ will be populated by "addToInterface".
  }
  virtual ~TextMessage(){ }
  void addToInterface(TextInterface* itf){
    itf->add(this);
  }
  virtual void parse(TextParser &p){ }
  virtual void write(Stream &w){ }
};



#endif // _STREAM_ELEMENT_H_
