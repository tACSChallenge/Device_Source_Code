/*
 * Handshake.h
 *
 *  Created on: Feb 21, 2019
 *      Author: David Wang
 */

#ifndef SRC_INTERFACE_HANDSHAKE_H_
#define SRC_INTERFACE_HANDSHAKE_H_

#include <Arduino.h>

#include "InterfaceAdapter.h"

class HandshakeRequest{
private:
  InterfaceAdapter* itf_;
public:
  HandshakeRequest(InterfaceAdapter* itf): itf_(itf){
    itf->addHandler(CC_SHAKE_REQ,[this](Reader *r){this->read(*r);});
  }
  void read(Reader &r){
    respond(r);
  }
  void respond(Reader &r){
    uint32_t code = r.readUINT32().getValue(0);
    Writer* w = itf_->getWriter();
    w->writeCC(CC_SHAKE_RSP);
    w->writeUINT32(code);
    w->send();
    //Serial.println(code);
  }
};

#define WAIT_FOR_SHAKE_MS 1000;

class Handshake{
private:
  HandshakeRequest shkreq;
  InterfaceAdapter* itf_;
  uint32_t tx_code_;
  boolean shake_complete_;

public:
  Handshake(InterfaceAdapter* itf):
    shkreq(itf), itf_(itf), tx_code_(0){
    itf->addHandler(CC_SHAKE_RSP,[this](Reader *r){this->read(*r);});
  }

  void read(Reader &r){
    respond(r);
  }

  void respond(Reader &r){
    uint32_t rx_code = r.readUINT32().getValue(0);
    if(rx_code==tx_code_){
      shake_complete_ = true;
    }
  }

  boolean shake(int num_shakes, unsigned long shake_delayms){
    shake_complete_ = false;
    tx_code_ = random(0L,2147483647L);
    //tx_code_ = 0x0F0F0F0F;
    unsigned long currms = millis();
    unsigned long prevms = currms;
    int shake_count = 0;
    while( shake_count<num_shakes && !shake_complete_ ){
      // send shake
      currms = millis();
      if(shake_count==0 || (currms-prevms >= shake_delayms)) {
        shake_count++;
        prevms = currms;
        // send the shake
        Writer* w = itf_->getWriter();
        w->writeCC(CC_SHAKE_REQ);
        w->writeUINT32(tx_code_);
        w->send();
        //Serial.println(tx_code_);
      }
      // wait for response
      itf_->handleMessages();
    }
    return shake_complete_;
  }

};




#endif /* SRC_INTERFACE_HANDSHAKE_H_ */
