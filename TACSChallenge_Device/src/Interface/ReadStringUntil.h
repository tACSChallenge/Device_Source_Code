/*
 * ReadStringUntil.h
 *
 *  Created on: Mar 8, 2019
 *      Author: David Wang
 */

#ifndef _READSTRINGUNTIL_H_
#define _READSTRINGUNTIL_H_

#include <Arduino.h>

template<int MAX_BUF_CAP>
class ReadStringUntil{
private:
  char buf_[MAX_BUF_CAP];
  size_t index_;

public:
  ReadStringUntil(): index_(0){
  }

  void reset(){
    index_ = 0;
  }

  // A timeout is indicated by 0 being returned
  // addNULLTerm == true, means a null terminator is added, but is NOT counted in the number of chars read.
  // addNULLTerm == false, means no null terminator is added.
  int readLine(Stream* s, char *xbuf, int xlen, unsigned long timeout_ms, bool addNULLTerm)
  {
    if(s == NULL || xlen==0) return 0;

    unsigned long readLineStart = millis();
    while (s->available() > 0){
        char c = s->read();
//        Serial.print("|");
//        Serial.print(c);
//        Serial.println("|");
        switch (c) {
          case '\n': // stop on new-lines
            goto exit_read_line;
          case '\r': // stop on carraige returns
            goto exit_read_line;
          default:
            if ((index_ + 1) < MAX_BUF_CAP){
              buf_[index_++] = c;
            }else{
              // Error, buffer overflow
            }
        }
        if(millis() - readLineStart >= timeout_ms){
          return 0;
        }
    }
    return 0;
  exit_read_line:
    // save off the index, so we can reset it to 0
    // note: at this point, "index_" represents the number of chars read,
    //       ignoring the ending '\n' or '\r' characters
    int temp_index = index_;
    index_ = 0;
    // check for the size of the ext provided buffer
    if(temp_index >= xlen){
      // the number of chars read is greater or equal to the buffer length.
      memcpy(xbuf,buf_,xlen);
      if(addNULLTerm){
        xbuf[xlen-1] = 0;
        return xlen - 1;
      }else{
        return xlen;
      }
    }else{
      // the number of chars read is less than the buffer length
      memcpy(xbuf,buf_,temp_index);
      if(addNULLTerm){
        xbuf[temp_index] = 0;
      }
      return temp_index;
    }
  }

};


#endif /* _READSTRINGUNTIL_H_ */
