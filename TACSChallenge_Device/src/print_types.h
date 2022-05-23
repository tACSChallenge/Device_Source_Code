
#ifndef _PRINT_TYPES_H_
#define _PRINT_TYPES_H_

#include <Arduino.h>

void print_type_sizes(){
  Serial.print("char: ");    Serial.println(sizeof(char));
  Serial.print("byte: ");    Serial.println(sizeof(byte));
  Serial.print("uint8_t: ");  Serial.println(sizeof(uint8_t));
  Serial.print("int8_t: ");   Serial.println(sizeof(int8_t));
  Serial.print("uint16_t: "); Serial.println(sizeof(uint16_t));
  Serial.print("int16_t: ");  Serial.println(sizeof(int16_t));
  Serial.print("uint32_t: "); Serial.println(sizeof(uint32_t));
  Serial.print("int32_t: ");  Serial.println(sizeof(int32_t));
  Serial.print("int: ");                Serial.println(sizeof(int));
  Serial.print("long int: ");           Serial.println(sizeof(long int));
  Serial.print("unsigned long: ");      Serial.println(sizeof(unsigned long));
  Serial.print("long long: ");          Serial.println(sizeof(long long));
  Serial.print("unsigned long int: ");  Serial.println(sizeof(unsigned long int));
  Serial.print("unsigned long long int: ");  Serial.println(sizeof(unsigned long long int));

  Serial.print("float: ");        Serial.println(sizeof(float));
  Serial.print("double: ");       Serial.println(sizeof(double));
  Serial.print("long double: ");  Serial.println(sizeof(long double));
}

#endif  //_PRINT_TYPES_H_