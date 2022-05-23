/*
 * HandleMessageCallback.h
 *
 *  Created on: Mar 7, 2019
 *      Author: David Wang
 */

#ifndef _INPUTCALLBACK_H_
#define _INPUTCALLBACK_H_

#include <Arduino.h>
#include "Writer.h"


class InputCallback{
protected:
  Writer* w_;
public:
  InputCallback(){
  }
  InputCallback(Writer* w): w_(w){
  }
  virtual void setWriter(Writer* w){
    w_ = w;
  }
  virtual void operator() (const uint8_t* buffer, size_t size) = 0;
};


#endif /* _INPUTCALLBACK_H_ */
