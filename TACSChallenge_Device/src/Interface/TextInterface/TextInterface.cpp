#include <Arduino.h>
#include <string.h>
#include <functional>

#include "TextInterface.h"
#include "TextWriter.h"
#include "Interface/Reader.h"

char txtRecvBuf[MAX_TXT_RECV_BUF_LEN];

void TextInterface::begin(Stream* s){
  // setup serial communication
  stream_ = s;
  // setup writer
  txtWriter_.init(s);
}

void TextInterface::add(uint8_t cmd_code, std::function<void(TextReader *r)> handler){
  handlers[cmd_code] = handler;
}

void TextInterface::remove(uint8_t cmd_code){
  handlers[cmd_code] = NULL;
}

//TextMessage* TextInterface::find(TextReader &r){
//  unsigned int starti;
//  unsigned int endi;
//  r.readSTRING(starti, endi);
//  for(int i=0; i<txtMsgsCount_; i++){
//    TextMessage* msg = txtMsgs_[i];
//    if(isEqual(r.buffer_, starti, endi, msg->txt_name_)){
//      return msg;
//    }
//  }
//  return NULL;
//}

boolean TextInterface::handleMessages(InputCallback* callback) {
  // Parse Inputs From User
  while (stream_->available() > 0) {

    int bytesRead = rsu.readLine(stream_, txtRecvBuf, MAX_TXT_RECV_BUF_LEN, readlineTimeout_ms, true);

    if (bytesRead <= 0) {
      return false;
    }
    txtReader_.init((uint8_t*)txtRecvBuf, bytesRead);
    auto cmd_code = txtReader_.readCC();

    // a hack to make sure EEG streaming data is NOT echoed back.
    if(cmd_code.isError() || (cmd_code.isValue())){
      if(callback!=NULL){
        (*callback)((uint8_t*)txtRecvBuf,bytesRead);
      }
    }

    if (cmd_code.isValue()){
      std::function<void(TextReader *r)> handler = handlers[cmd_code.getValue(CC_NONE)];
      if(handler != NULL){
        handler(&txtReader_);
      }
    }
  }
  return true;
}

TextReader* TextInterface::getTxtReader(){
  return &txtReader_;
}

TextWriter* TextInterface::getTxtWriter(){
  return &txtWriter_;
}
