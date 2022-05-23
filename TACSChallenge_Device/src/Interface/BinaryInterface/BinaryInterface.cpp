#include <Arduino.h>
#include "BinaryInterface.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "Interface/CommandCodes.h"
#include "ErrorHandling/ErrVal.h"

void BinaryInterface::begin(Stream* s){
  // setup serial communication
  pSerial.setPacketHandler(this);
  pSerial.begin(s);
  // setup writer
  binWriter_.init(binWriterBuffer, BINARY_WRITE_BUFFER_SIZE, &pSerial);
}

void BinaryInterface::add(uint8_t cmd_code, std::function<void(BinaryReader *r)> handler){
  handlers[cmd_code] = handler;
}

void BinaryInterface::remove(uint8_t cmd_code){
  handlers[cmd_code] = NULL;
}

std::function<void(BinaryReader *r)> BinaryInterface::find(BinaryReader &reader){
  boolean error = false;
  uint8_t cmd_code = reader.readUINT8().getValue(); // CC_NONE
  if(!error && cmd_code < CC_MAX_SIZE){
    return handlers[cmd_code];
  }
  return NULL;
}

boolean BinaryInterface::handleMessages(InputCallback* callback) {
  handleMessageCallback_ = callback;
  // Parse Inputs From User
  pSerial.update();
  return true;
}

void BinaryInterface::onPacketFunction(const uint8_t* buffer, size_t size){
  if(handleMessageCallback_!=NULL){
    (*handleMessageCallback_)(buffer,size);
  }

  binReader_.init(buffer, size);

  auto handler = find(binReader_);

  if (handler!=NULL){
    handler(&binReader_);
  }
}

BinaryReader* BinaryInterface::getBinReader(){
  return &binReader_;
}

BinaryWriter* BinaryInterface::getBinWriter(){
  return &binWriter_;
}

