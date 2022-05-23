/*
 * ErrVal.h
 *
 *  Created on: Dec 28, 2018
 *      Author: david
 */

#ifndef SRC_ERRVAL_H_
#define SRC_ERRVAL_H_

#include <Arduino.h>
#include "ErrCodes.h"

typedef uint8_t error_t;

/*************************************************/
// USING MONAD

struct Err {
  error_t error_;
};

Err error(error_t const& error) ;
Err error(error_t&& error);

template<class V>
struct Val{
  V value_;
};

template<class V>
constexpr Val<V> value(V const& value) {
  return {value};
}
template<class V>
Val<V> value(V&& value) {
  return { std::move(value) };
}

template<class V>
class ErrVal {
private:
  union {
    error_t error_;
    V value_;
  };
  bool const isValue_ = 0;
public:
  constexpr ErrVal( Val<V> const& value )
    : value_(value.value_)
    , isValue_(1)
  {}
  ErrVal( Val<V> && value )
    : value_(std::move(value.value_))
    , isValue_(1)
  {}
  constexpr ErrVal( Err const& error )
    : error_(error.error_)
    , isValue_(0)
  {}
  ErrVal( Err && error )
    : error_(std::move(error.error_))
    , isValue_(0)
  {}
  ~ErrVal() {
    if (isValue_) {
      value_.~V();
    }
  }
  inline boolean isValue(){
    return isValue_;
  }
  inline boolean isError(){
    return !isValue_;
  }
  inline V getValue(V defaultValue=0){
    return isValue_ ? value_ : defaultValue;
  }
  inline error_t getError(){
    return isValue_ ? ERROR_NONE : error_;
  }
};


template<class T1, class T2>
constexpr ErrVal<T1> convert(ErrVal<T2> val){
  if(val.isError()){
    return error(val.getError());
  }else{
    return value((T1)val.getValue());
  }
}


#endif /* SRC_ERRVAL_H_ */
