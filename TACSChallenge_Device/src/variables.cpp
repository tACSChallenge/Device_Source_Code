
#include "variables.h"

size_t parse_str_name(char *name, size_t name_len, char *dst_name, size_t dst_name_len)
{
    MatchState ms;
    ms.Target(name, name_len);

    // Execute regular expression
    char *exp = (char *)"%s*([%a_][%w_]*)%s*";
    char result = ms.Match(exp);

    if (result == REGEXP_MATCHED && ms.level == 1) {
        size_t idx = 0;
        size_t len = ms.capture[idx].len;
        if (len + 1 < dst_name_len) {
            ms.GetCapture(dst_name, idx);
            return len;
        }
    }
    return 0;
}

#if 0
void test_parse_var() {
  Variable<256,256> v;
  
  char* var_name = (char*) "  _123_a__bc \t ";

  Serial.print("input var name length:");
  Serial.println(strlen(var_name));

  v.setName(var_name, strlen(var_name));

  Serial.print("output var name length:");
  Serial.print(v.getNameLength());
  Serial.print(",");
  Serial.print(v.getName());
  Serial.println();

  char* var_value = (char*) " 10237E2";

  v.setValue(var_value, strlen(var_value));

  Serial.print("input var value length:");
  Serial.println(strlen(var_value));

  Serial.print("output var value length:");
  Serial.print((double)v.getLDValue(),10);
  Serial.print(",");
  Serial.print((long)v.getLLValue());
  Serial.println();

  Serial.println();
}
#endif