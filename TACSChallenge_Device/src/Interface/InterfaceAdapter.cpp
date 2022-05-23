
#include "InterfaceAdapter.h"
#include "InterfaceTypes.h"
#include "BinaryInterface/BinaryReader.h"

/**********************************************************************/
// INTERFACE CLASS

void InterfaceAdapter::begin(Stream *s){
  // setup serial communication
  BinaryInterface::begin(s);
  TextInterface::begin(s);
}


void InterfaceAdapter::addTextHandler(uint8_t cmd_code, std::function<void(TextReader *r)> handler){
//  BinaryInterface::add(cmd_code, handler);
  TextInterface::add(cmd_code, handler);
}

void InterfaceAdapter::addBinaryHandler(uint8_t cmd_code, std::function<void(BinaryReader *r)> handler){
  BinaryInterface::add(cmd_code, handler);
//  TextInterface::add(cmd_code, handler);
}

void InterfaceAdapter::addHandler(uint8_t cmd_code, std::function<void(Reader *r)> handler){
  BinaryInterface::add(cmd_code, handler);
  TextInterface::add(cmd_code, handler);
}

boolean InterfaceAdapter::handleMessages() {
  if(inputCallback_ != NULL){
    inputCallback_->setWriter(getWriter());
  }
  switch(inputType_){
  case INTERFACE_INPUT_TYPE_BINARY:
    return BinaryInterface::handleMessages( inputCallback_ );
  case INTERFACE_INPUT_TYPE_TEXT:
    return TextInterface::handleMessages( inputCallback_ );
  default:
    return false;
  }
}

Writer* InterfaceAdapter::getWriter(){
  switch(outputType_){
  case INTERFACE_OUTPUT_TYPE_BINARY:
    return &(BinaryInterface::binWriter_);
  case INTERFACE_OUTPUT_TYPE_TEXT:
    return &(TextInterface::txtWriter_);
  default:
    return NULL;
  }
}

void InterfaceAdapter::forwardTo(InterfaceAdapter &destITF, Reader *r, INTERFACE_TYPE types[], size_t typeslen){
  if(this->inputType_ == INTERFACE_INPUT_TYPE_BINARY){
    BinaryReader *br = ((BinaryReader*)r);
    if(destITF.outputType_ == INTERFACE_OUTPUT_TYPE_BINARY){
      // translate from binary to binary
      BinaryWriter* bw = (BinaryWriter*) destITF.getWriter();
      bw->write(br->getBuffer(), br->getIndex());
      bw->send();
    }else if(destITF.outputType_ == INTERFACE_OUTPUT_TYPE_TEXT){
      // translate from binary to text
      TextWriter* tw = (TextWriter*) destITF.getWriter();
      binary2text(*br,*tw,types,typeslen);
    }
  }else if(this->inputType_ == INTERFACE_INPUT_TYPE_TEXT){
    TextReader* tr = ((TextReader*)r);
    if(destITF.outputType_ == INTERFACE_OUTPUT_TYPE_BINARY){
      // translate from text to binary
      BinaryWriter* bw = (BinaryWriter*)destITF.getWriter();
      text2binary(*tr,*bw,types,typeslen);
    }else if(destITF.outputType_ == INTERFACE_OUTPUT_TYPE_TEXT){
      // translate from text to text
      TextWriter* tw = (TextWriter*) destITF.getWriter();
      tw->writeSTRING((char*)(tr->getBuffer()));
      tw->send();
    }
  }
}

void InterfaceAdapter::text2binary(TextReader &tr, BinaryWriter &bw, INTERFACE_TYPE types[], size_t typeslen){
  for(int typei = 0; typei<typeslen; typei++){
    INTERFACE_TYPE type = types[typei];
    switch(type){
    case TYPE_UINT8: bw.writeUINT8(tr.readUINT8().getValue()); break;
    case TYPE_UINT16: bw.writeUINT16(tr.readUINT16().getValue()); break;
    case TYPE_UINT32: bw.writeUINT32(tr.readUINT32().getValue()); break;
    case TYPE_UINT64: bw.writeUINT64(tr.readUINT64().getValue()); break;
    case TYPE_INT8: bw.writeUINT8(tr.readUINT8().getValue()); break;
    case TYPE_INT16: bw.writeUINT16(tr.readUINT16().getValue()); break;
    case TYPE_INT32: bw.writeUINT32(tr.readUINT32().getValue()); break;
    case TYPE_INT64: bw.writeUINT64(tr.readUINT64().getValue()); break;
    case TYPE_FLOAT: bw.writeFLOAT(tr.readFLOAT().getValue()); break;
    case TYPE_DOUBLE: bw.writeDOUBLE(tr.readDOUBLE().getValue()); break;
    case TYPE_STRING: {
      char buf[256];
      auto size = tr.readSTRING(buf,256);
      bw.writeSTRING(buf, size.getValue());
      break;
    }
    case TYPE_BOOL:{
      bw.writeBOOL(tr.readBOOL().getValue(),true);
      break;
    }
    }
  }
}

void InterfaceAdapter::binary2text(BinaryReader &br, TextWriter &tw, INTERFACE_TYPE types[], size_t typeslen){
  for(int typei = 0; typei<typeslen; typei++){
    INTERFACE_TYPE type = types[typei];
    switch(type){
    case TYPE_UINT8: tw.writeUINT8(br.readUINT8().getValue()); break;
    case TYPE_UINT16: tw.writeUINT16(br.readUINT16().getValue()); break;
    case TYPE_UINT32: tw.writeUINT32(br.readUINT32().getValue()); break;
    case TYPE_UINT64: tw.writeUINT64(br.readUINT64().getValue()); break;
    case TYPE_INT8: tw.writeUINT8(br.readUINT8().getValue()); break;
    case TYPE_INT16: tw.writeUINT16(br.readUINT16().getValue()); break;
    case TYPE_INT32: tw.writeUINT32(br.readUINT32().getValue()); break;
    case TYPE_INT64: tw.writeUINT64(br.readUINT64().getValue()); break;
    case TYPE_FLOAT: tw.writeFLOAT(br.readFLOAT().getValue()); break;
    case TYPE_DOUBLE: tw.writeDOUBLE(br.readDOUBLE().getValue()); break;
    case TYPE_STRING: {
      char buf[256];
      auto size = br.readSTRING(buf,256);
      tw.writeSTRING(buf, size.getValue());
      break;
    }
    case TYPE_BOOL:{
      tw.writeBOOL(br.readBOOL().getValue(),true);
      break;
    }
    }
  }
}


