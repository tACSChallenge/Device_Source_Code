#ifndef _TEXT_INTERFACE_H_
#define _TEXT_INTERFACE_H_

#include <Arduino.h>
#include <functional>

#include "Interface/InputCallback.h"
#include "Interface/ReadStringUntil.h"
#include "TextReader.h"
#include "TextWriter.h"

class TextMessage;

#define MAX_TXT_RECV_BUF_LEN 256

class TextInterface {
private:
  std::function<void(TextReader *r)> handlers[CC_MAX_SIZE];
  unsigned long readlineTimeout_ms = 200; // 2/10 of a second
  Stream* stream_ = NULL;
  ReadStringUntil<MAX_TXT_RECV_BUF_LEN> rsu;
  std::function<void(TextReader *r)> find(TextReader *r);
protected:
  TextReader txtReader_;
  TextWriter txtWriter_;
public:
  TextInterface(){}
  virtual ~TextInterface(){}
  void begin(Stream* s);
  void add(uint8_t cmd_code, std::function<void(TextReader *r)> handler);
  void remove(uint8_t cmd_code);
  boolean handleMessages(InputCallback* callback);
  TextReader* getTxtReader();
  TextWriter* getTxtWriter();
//  void sendMessage(TextMessage* msg);
};


#endif // _TEXT_INTERFACE_H_
