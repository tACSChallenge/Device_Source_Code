#ifndef _VARIABLES_H_
#define _VARIABLES_H_

#include <Arduino.h>
#include "Regexp.h"
#include "Interface/TextInterface/TextReader.h"
#include "device_log.h"

// #if 0
static const char *VAR_TAG = "var";	// Logging prefix for this module
// #endif

size_t parse_str_name(char* name, size_t name_len, char* dst_name, size_t dst_name_len);

template <int MAX_NAME_STR_LENGTH, int MAX_VALUE_STR_LENGTH>
class Variable{
private:
    char str_name_[ MAX_NAME_STR_LENGTH ];
    char str_value_[ MAX_VALUE_STR_LENGTH ];
    size_t var_name_len_;
    size_t var_value_len_;
    enum type {TYPE_NONE, TYPE_UNDEFINED, TYPE_LL, TYPE_LD} type_;
    union{
        long long ll_val_;
        long double ld_val_;
    };

    /*
    size_t parseValueString(char* value, size_t value_len, char* dst_value, size_t dst_value_len){
        MatchState ms;
        ms.Target (value, value_len);

        // Execute regular expression 
        // char* exp = (char*) "%s*([+-]?[%d]+%.?[%d]*)%s*";
        // char* exp = "%s*[%-%+]?[%d]*%.?[%d]+([eE][%-%+]?[%d]+)?%s*";
        char* exp = (char*) "%s*([%-%+]?[%d]*%.?[%d]+)([eE][%-%+]?[%d]+)%s*";
        char result = ms.Match(exp);

        if ( result == REGEXP_MATCHED ) {
            size_t len = 0;
            if(ms.level >= 1){
                size_t idx = 0;
                len = ms.capture[idx].len;
                if ( len + 1 < dst_value_len ) {
                    ms.GetCapture(dst_value, idx);
                }
            }
            if(ms.level >= 2){
                size_t idx = 1;
                len += ms.capture[idx].len;
                if ( len + 1 < dst_value_len ) {
                    ms.GetCapture(dst_value, idx);
                }
            }
            return len;
        }
        return 0;
    }
    */

public:
    Variable() {
        // clear saved values
        clear();
    }

    void clear(){
        memset(str_name_,0,sizeof(str_name_));
        memset(str_value_,0,sizeof(str_value_));
        var_name_len_ = 0;
        var_value_len_ = 0;
        type_ = TYPE_NONE;
        ll_val_ = 0;
        ld_val_ = 0;
    }

    bool setName(char* name, uint8_t name_len){
        if( name_len+1 >= MAX_NAME_STR_LENGTH ){
            return false;
        }

        // Parse the name string and copy into the field "str_name_"
        size_t var_name_len_temp = parse_str_name(name, name_len, str_name_, MAX_NAME_STR_LENGTH);

        // If no match to a variable name, return false.
        if ( var_name_len_temp <= 0 ) {
            return false;
        }

        // save the name length 
        var_name_len_ = var_name_len_temp;
        return true;
    }

    char* getName(){
        return str_name_;
    }

    size_t getNameLength(){
        return var_name_len_;
    }

    bool setValue(char* value, uint8_t value_len){
        if( value_len+1 >= MAX_VALUE_STR_LENGTH ){
            return false;
        }

        // check if the value contains symbols common to floating point
        MatchState ms;
        ms.Target (value, value_len);
        const char* exp = "[%.eE]";
        char result = ms.Match(exp);

        // parse the long double or long long value
        char* pStart = value;
        char* pEnd = NULL;
        if ( result == REGEXP_MATCHED ){
            long double ld_val = strtold(pStart, &pEnd);
            if( pStart != pEnd ){
                // save string
                memcpy(str_value_, value, value_len);
                str_value_[value_len] = '\0';
                var_value_len_ = value_len;
                // save value
                ld_val_ = ld_val;
                type_ = TYPE_LD;
                return true;
            }
        }else{
            long long ll_val = strtoll(pStart, &pEnd, 0);
            if( pStart != pEnd ){
                // save string
                memcpy(str_value_, value, value_len);
                str_value_[value_len] = '\0';
                var_value_len_ = value_len;
                // save value
                ll_val_ = ll_val;
                type_ = TYPE_LL;
                return true;
            }
        }
        return false;
    }

    char* getValue(){
        return str_value_;
    }

#if 0
    bool setLDValue(long double ld_val){
        ld_val_ = ld_val;
        type_ = TYPE_LD;
        return true;
    }
#endif

#if 0
    bool setLLValue(long double ll_val){
        ll_val_ = ll_val;
        type_ = TYPE_LL;
        return true;
    }
#endif

    bool isLLValue(){
        return type_==TYPE_LL;
    }

    long long getLLValue() {
        switch(type_){
            case TYPE_LL: return ll_val_;
            case TYPE_LD: return (long long) ld_val_;
            case TYPE_NONE:
            case TYPE_UNDEFINED: 
            default: return 0;
        }
    }

    bool isLDValue(){
        return type_==TYPE_LD;
    }

    long double getLDValue(){
        switch(type_){
            case TYPE_LL: return (long double) ll_val_;
            case TYPE_LD: return ld_val_;
            case TYPE_NONE:
            case TYPE_UNDEFINED:
            default: return 0;
        }
    }

};

template <int MAX_NUM_VARS, int MAX_NAME_STR_LENGTH, int MAX_VALUE_STR_LENGTH>
class Variables {
private:
    Variable<MAX_NAME_STR_LENGTH, MAX_VALUE_STR_LENGTH> vars_[MAX_NUM_VARS];
    size_t var_count_;
    // temporary buffers
    char read_var_scratch_[MAX_NAME_STR_LENGTH];
    char parsed_var_scratch_[MAX_NAME_STR_LENGTH];
public:
    Variables(){
        clear();
    }

    void clear() {
        var_count_ = 0;
        // keep the two variables used by staircase
        addVariable((char*)"a",1,(char*)"0",1);
        addVariable((char*)"b",1,(char*)"0",1);
    }
    
    Variable<MAX_NAME_STR_LENGTH, MAX_VALUE_STR_LENGTH>* getVariable(uint8_t idx){
        if ( idx < var_count_ ) {
            return &(vars_[idx]);
        } else {
            return NULL;
        }
    }

    /*
     * Returns:
     * 0 = success
     * 1 = parse error
     * 2 = max num vars 
     */
    uint8_t addVariable(TextReader *r){
        // Create index to the variable we are finding by name
        size_t found_var_idx = -1;
        // read the variable name, save name in SCRATCH
        auto read_var_name_len = r->readSTRING(read_var_scratch_, MAX_NAME_STR_LENGTH);
        if(read_var_name_len.isError()){
            return 1; // parse error
        }
        // find the variable by name
        bool found_var = findVariable(read_var_scratch_, read_var_name_len.getValue(), &found_var_idx);
        if ( found_var ) {
            // read the variable value, save value in SCRATCH
            auto read_var_value_len = r->readSTRING(read_var_scratch_, MAX_VALUE_STR_LENGTH);
            if(read_var_value_len.isError()){
                return 1; // parse error
            }
            // variable found, set it's value
            vars_[found_var_idx].setValue(read_var_scratch_, read_var_value_len.getValue());
        } else {
            if( var_count_+1 >= MAX_NUM_VARS ){
                return 2; // max num vars
            }
            // set variable name
            vars_[var_count_].setName(read_var_scratch_, read_var_name_len.getValue());
            // read the variable value, save value in SCRATCH
            auto read_var_value_len = r->readSTRING(read_var_scratch_, MAX_VALUE_STR_LENGTH);
            if(read_var_value_len.isError()){
                return 1; // parse error
            }
            // set variable value
            vars_[var_count_].setValue(read_var_scratch_, read_var_value_len.getValue());
            // increment the var_count_
            var_count_++;
        }
        return 0; // success
    }

    uint8_t addVariable(char* name, size_t name_len, float value ){
        char value_buf[256];
        uint8_t value_len = sprintf(value_buf, "%f", value);
        return addVariable(name, name_len, value_buf, value_len);
    }

    uint8_t addVariable(char* name, size_t name_len, char* value, size_t value_len ){
        // Create index to the variable we are finding by name
        size_t found_var_idx = -1;
        // Find the variable by name
        bool found_var = findVariable(name, name_len, &found_var_idx);
        if ( found_var ) {
            // variable found, set it's value
            vars_[found_var_idx].setValue(value, value_len);
        } else {
            if( var_count_+1 >= MAX_NUM_VARS ){
                return 2; // max num vars
            }
            // set variable name
            vars_[var_count_].setName(name, name_len);
            // set variable value
            vars_[var_count_].setValue(value, value_len);
            // increment the var_count_
            var_count_++;
        }
        return 0; // success
    }

    Variable<MAX_NAME_STR_LENGTH, MAX_VALUE_STR_LENGTH>* getVariable(TextReader* r) {
        size_t found_idx;
        if ( findVariable(r, &found_idx) ){
            return &(vars_[found_idx]);
        } else {
            return NULL;
        }
    }

    bool removeVariable(TextReader *r){
        // Create index to the variable we are finding by name
        size_t found_var_idx = -1;
        // find the variable by name
        if( !findVariable(r, &found_var_idx) ){
            return false;
        }
        // remove the variable
        vars_[found_var_idx].clear();
        // decrement the var_count_
        var_count_--;
        // copy upper variables down
        for(size_t idx=found_var_idx; idx<var_count_; idx++){
            vars_[idx] = vars_[idx+1];
        }
        return true;
    }

    bool findVariable(TextReader* r, size_t* found_idx){
        // parse the variable name
        auto read_var_name_len = r->readSTRING(read_var_scratch_, MAX_NAME_STR_LENGTH);
        if(read_var_name_len.isError()){
            return false;
        }
        return findVariable(read_var_scratch_, read_var_name_len.getValue(), found_idx);
    }

    bool findVariable(char* read_var_name, size_t read_var_name_len, size_t* found_idx){
        // Create pointer to the variable we are removing;
        Variable<MAX_NAME_STR_LENGTH,MAX_VALUE_STR_LENGTH>* found_var = NULL;
        // parse out the clean variable name
        size_t parsed_var_name_len = parse_str_name(read_var_name, read_var_name_len, parsed_var_scratch_, MAX_NAME_STR_LENGTH);
        if(parsed_var_name_len <= 0){
            return false;
        }
        // find the variable we wish to remove
        for(size_t idx=0; idx<var_count_; idx++){
            found_var = &(vars_[idx]);
            if ( strncmp(found_var->getName(),parsed_var_scratch_,parsed_var_name_len) == 0 ){
                *found_idx = idx;
                return true;
            }
        }
        return false;
    }

};



#endif // _VARIABLES_H_