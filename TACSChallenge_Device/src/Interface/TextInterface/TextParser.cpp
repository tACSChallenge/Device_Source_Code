
#include "TextParser.h"

/**********************************************************************/
// READ LINE FUNCTION

// A timeout is indiciated by a negative number returned.
int readLine(Stream &S, char *buffer, int len, unsigned long timeout_ms)
{
  int pos = 0;
  int readch;
  unsigned long readLineStart = millis();
  while (pos < len - 1) {
    if (S.available() > 0 && (readch = S.read()) > 0) {
      switch (readch) {
        case '\n': // stop on new-lines
          goto exit_read_line;
        case '\r': // stop on carraige returns
          goto exit_read_line;
        default:
          buffer[pos++] = readch;
      }
    }
    if(millis() - readLineStart >= timeout_ms){
      buffer[pos] = 0;
      return -pos;
    }
  }
exit_read_line:
  buffer[pos] = 0;
  return pos;
}

int readLine(Stream &S, char *buffer, int len)
{
  int pos = 0;
  int readch;
  unsigned long readLineStart = millis();
  while (pos < len - 1) {
    if (S.available() > 0 && (readch = S.read()) > 0) {
      switch (readch) {
        case '\n': // stop on new-lines
          goto exit_read_line;
        case '\r': // stop on carraige returns
          goto exit_read_line;
        default:
          buffer[pos++] = readch;
      }
    }
  }
exit_read_line:
  buffer[pos] = 0;
  return pos;
}

/**********************************************************************/
// CONVENIENCE FUNCTIONS

void printSubstring(char *string, int start_index, int end_index) {
  for (int i = start_index; i < end_index; i++) {
    Serial.print(string[i]);
  }
}

boolean isEqual(char *string1, int start_index, int end_index, char *string2) {
  int j = 0;
  for (int i = start_index; i < end_index; i++, j++) {
    if (string1[i] != string2[j] || string2[j] == '\0') {
      return false;
    }
  }
  return string2[j] == '\0';
}
