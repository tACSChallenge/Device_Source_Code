#include "Utils/print_util.h"

void printHexArray(Stream &stream, uint8_t* array, size_t size){
    for(int i=0; i<size; i++){
        stream.print(array[i],HEX);
        stream.print(" ");
    }
}

void printlnHexArray(Stream &stream, uint8_t* array, size_t size){
    printHexArray(stream,array,size);
    Serial.println();
}