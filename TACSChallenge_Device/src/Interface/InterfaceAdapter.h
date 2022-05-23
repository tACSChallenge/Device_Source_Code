#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <Arduino.h>
#include "TextInterface/TextInterface.h"
#include "TextInterface/TextReader.h"
#include "TextInterface/TextWriter.h"
#include "BinaryInterface/BinaryInterface.h"
#include "BinaryInterface/BinaryReader.h"
#include "BinaryInterface/BinaryWriter.h"
#include "Reader.h"
#include "Writer.h"
#include "CommandCodes.h"
#include "InputCallback.h"
#include "InterfaceTypes.h"

class MessageAdapter;

/**********************************************************************/
// INTERFACE CLASS

#define INTERFACE_INPUT_TYPE_UNDEF 0
#define INTERFACE_INPUT_TYPE_BINARY 1
#define INTERFACE_INPUT_TYPE_TEXT 2

#define INTERFACE_OUTPUT_TYPE_UNDEF 0
#define INTERFACE_OUTPUT_TYPE_BINARY 1
#define INTERFACE_OUTPUT_TYPE_TEXT 2

class InputEchoCallback : public InputCallback{
protected:
public:
  InputEchoCallback() : InputCallback(){
  }
  InputEchoCallback(Writer* w): InputCallback(w){
  }
  virtual void operator() (const uint8_t* buffer, size_t size){
    if(w_!=NULL){
      w_->writeCC(CC_ECHO);
      w_->writeSTRING((char*)buffer, size);
      w_->send();
    }
  }
};

class InputPrintEchoCallback : public InputCallback{
protected:
  TextWriter USBSerialWriter_;
public:
  InputPrintEchoCallback() : InputCallback(){
  }
  InputPrintEchoCallback(Print* p): InputCallback(), USBSerialWriter_(p){
    w_ = &USBSerialWriter_;
  }
  virtual void setWriter(Writer* w){
    // do nothing
  }
  virtual void operator() (const uint8_t* buffer, size_t size){
    if(w_!=NULL){
      w_->writeCC(CC_ECHO);
      w_->writeSTRING((char*)buffer, size);
      w_->send();
    }
  }
};

class InterfaceAdapter: public TextInterface, public BinaryInterface {
private:
  uint8_t inputType_;
  uint8_t outputType_;
  InputCallback* inputCallback_;
public:
  InterfaceAdapter(uint8_t inputType, uint8_t outputType, InputCallback* inputCallback) : TextInterface(), BinaryInterface(), inputType_(inputType), outputType_(outputType), inputCallback_(inputCallback){
  }
  void setInputType(uint8_t inputType){
    inputType_ = inputType;
  }
  uint8_t getInputType(){
    return inputType_;
  }
  void setOutputType(uint8_t outputType){
    outputType_ = outputType;
  }
  uint8_t getOutputType(){
    return outputType_;
  }
  void begin(Stream *s);
  void addTextHandler(uint8_t cmd_code, std::function<void(TextReader* r)> handler);
  void addBinaryHandler(uint8_t cmd_code, std::function<void(BinaryReader* r)> handler);
  void addHandler(uint8_t cmd_code, std::function<void(Reader *r)> handler);
//  void addTxt(uint8_t cmd_code, std::function<void(TextReader &r)> handler);
//  void addBin(uint8_t cmd_code, std::function<void(BinaryReader &r)> handler);
//  void remove(BinaryMessage* msg);
//  void remove(uint8_t bin_code);
  boolean handleMessages();
//  void sendMessage(BinaryMessage* msg);
  Writer* getWriter();
  void forwardTo(InterfaceAdapter &destITF, Reader *r, INTERFACE_TYPE types[], size_t typeslen);
  void text2binary(TextReader &tr, BinaryWriter &bw, INTERFACE_TYPE types[], size_t typeslen);
  void binary2text(BinaryReader &br, TextWriter &tw, INTERFACE_TYPE types[], size_t typeslen);
};


#endif // _INTERFACE_H_
