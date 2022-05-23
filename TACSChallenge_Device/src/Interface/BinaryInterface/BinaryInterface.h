#ifndef _BINARY_INTERFACE_H_
#define _BINARY_INTERFACE_H_

#include <Arduino.h>
#include <functional>

#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "Interface/Reader.h"
#include "Interface/InputCallback.h"
#include "Interface/PacketSerial_Mod/src/PacketSerial_Mod.h"


class BinaryMessage;

#define BINARY_WRITE_BUFFER_SIZE 256

class BinaryInterface: public PacketHandlerClass {
private:
  PacketSerial pSerial;
  std::function<void(BinaryReader *r)> handlers[256];
  uint8_t binWriterBuffer[BINARY_WRITE_BUFFER_SIZE];
  InputCallback* handleMessageCallback_;
  std::function<void(BinaryReader *r)> find(BinaryReader &reader);
  void onPacketFunction(const uint8_t* buffer, size_t size);
protected:
  BinaryReader binReader_;
  BinaryWriter binWriter_;
public:
  BinaryInterface() {}
  virtual ~BinaryInterface() {}
  void begin(Stream* s);
  void add(uint8_t cmd_code, std::function<void(BinaryReader *r)> handler);
  void remove(uint8_t cmd_code);
  boolean handleMessages(InputCallback* callback);
  BinaryReader* getBinReader();
  BinaryWriter* getBinWriter();
};



#endif // _BINARY_INTERFACE_H_
