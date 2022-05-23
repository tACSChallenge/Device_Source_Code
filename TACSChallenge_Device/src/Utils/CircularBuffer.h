#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#include <Arduino.h>

/**********************************************/
// Implementation of Circular Buffer (aka Ring Buffer)
// Based on description found online at:
// https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc

template<typename T, int CAPACITY>
class CircularBuffer {
  private:
    T window_[CAPACITY]; // the window array
    size_t cap_; // the size of the window array
    size_t add_index_; // the index into the window array that will be ADDED next. (head)
    size_t rem_index_; // the index into the window array that will be REMOVED next. (tail)
    boolean full_;
    void advancePointer(){
      if(full_){
        rem_index_ = (rem_index_ + 1) % cap_;
      }
      add_index_ = (add_index_+1) % cap_;
      full_ = (add_index_ == rem_index_);
    }
    void retreatPointer(){
      full_ = false;
      rem_index_ = (rem_index_+1) % cap_;
    }
  public:
    CircularBuffer(){
      reset();
    }
    ~CircularBuffer() {
    }
    inline void reset() {
      cap_ = CAPACITY;
      add_index_ = 0;
      rem_index_ = 0;
      full_ = false;
    }
    inline boolean add(T val, boolean overwrite = true) {
      // returns true if the new value is added.
      if(overwrite || !full()){
        window_[add_index_] = val;
        advancePointer();
        return true;
      }
      return false;
    }
    inline T peek(T defaultVal) {
      if(empty()){
        return defaultVal;
      }else{
        return window_[rem_index_];
      }
    }
    inline T remove(T defaultVal) {
      if(empty()){
        return defaultVal;
      }else{
        T val = window_[rem_index_];
        retreatPointer();
        return val;
      }
    }
    inline bool empty() {
      return rem_index_ == add_index_ && !full_;
    }

    boolean full(){
      return full_;
    }
    void debugPrintWindow(Stream &s) {
      for (int i = 0; i < cap_; i++) {
        s.print(window_[i]);
        s.print(',');
      }
      s.print('[');
      s.print(cap_);
      s.print(',');
      s.print(add_index_);
      s.print(',');
      s.print(rem_index_);
      s.print(']');
      s.println();
    }
    size_t getCapacity(){
      return cap_;
    }
    size_t getSize(){
      if(!full()){
        if(add_index_ >= rem_index_){
          return add_index_ - rem_index_;
        }else{
          return cap_ + add_index_ - rem_index_;
        }
      }else{
        return cap_;
      }
    }
};


#endif  /* _CIRCULAR_BUFFER_H_ */

