#ifndef _TEXT_PARSER_H_
#define _TEXT_PARSER_H_

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>


/**********************************************************************/
// READ LINE FUNCTION

// A timeout is indiciated by a negative number returned.
int readLine(Stream &S, char *buffer, int len, unsigned long timeout_ms);
int readLine(Stream &S, char *buffer, int len);

/**********************************************************************/
// CONVENIENCE FUNCTIONS

void printSubstring(char *string, int start_index, int end_index);
boolean isEqual(char *string1, int start_index, int end_index, char *string2);

/**********************************************************************/
// PARSE TEXT COMMAND STRUCT

class TextParser {
public:
  char* buffer_;
  size_t curr_index_;
  size_t size_;
  TextParser(): curr_index_(0), size_(0){
  }
  void populate(const char* buffer, const size_t size) {
	  buffer_ = (char*) buffer;
	  buffer_[size] = '\0';
    size_ = size;
    curr_index_ = 0;
  }
  bool nextText(unsigned int &start_index, unsigned int &end_index) {
    if (curr_index_ < size_) {
      start_index = curr_index_;
      for (end_index = start_index ; ; end_index++) {
        if (buffer_[end_index] == ',' || buffer_[end_index] == '\0') {
          curr_index_ = end_index + 1;
          break;
        }
      }
      return start_index != end_index;
    } else {
      start_index = curr_index_;
      end_index = curr_index_;
      return false;
    }
  }
  void nextText() {
    unsigned int start_index = 0;
    unsigned int end_index = 0;
    nextText(start_index, end_index);
    printSubstring(buffer_, start_index, end_index);
  }
  int nextInt() {
    unsigned int start_index;
    unsigned int end_index;
    nextText(start_index, end_index);
    return atoi(buffer_ + start_index);
  }
  float nextFloat() {
    unsigned int start_index;
    unsigned int end_index;
    nextText(start_index, end_index);
    return atof(buffer_ + start_index);
  }
  boolean nextBoolean() {
    unsigned int start_index;
    unsigned int end_index;
    nextText(start_index, end_index);
    char c = buffer_[start_index];
    switch (c) {
      case 't':
      case 'T':
      case '1':
      case '*':
        return true;
      case 'f':
      case 'F':
      case '0':
      case '_':
      default:
        return false;
    }
    return false;
  }
};

#endif // _TEXT_PARSER_H_
