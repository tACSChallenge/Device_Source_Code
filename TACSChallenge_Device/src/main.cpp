#include <Arduino.h>
#include "board_config.h"
#include "Interface/Writer.h"
#include "Interface/CommandCodes.h"
#include "Interface/InterfaceAdapter.h"
#include "Interface/TextInterface/TextWriter.h"
#include "Interface/BinaryInterface/BinaryWriter.h"
#include "Utils/CircularBuffer.h"
#include "state.h"
#include "staircase.h"
#include "stim.h"
#include "return_codes.h"
#include "device_log.h"
#include "variables.h"
#include "print_types.h"

static const char *MAIN_TAG = "main";	// Logging prefix for this module

Variables<VARIABLE_MAX_NUM,VARIABLE_MAX_NAME_STR_LENGTH,VARIABLE_MAX_VALUE_STR_LENGTH> vars;

IntervalTimer timer;

class InputPrintEchoCallback2 : public InputCallback{
private:
  Writer* w_;
public:
  InputPrintEchoCallback2() : InputCallback(){
  }
  virtual void setWriter(Writer* w){
    w_ = w;
  }
  virtual void operator() (const uint8_t* buffer, size_t size){
    if(w_!=NULL){
      w_->writeCC(RC_CMD_ECHO);
      w_->writeSTRING((char*)buffer, size);
      w_->send();
    }
  }
};
InputPrintEchoCallback2 printEchoCallback;
InterfaceAdapter itf(INTERFACE_INPUT_TYPE_TEXT, INTERFACE_OUTPUT_TYPE_BINARY, &printEchoCallback);

State state;
Script script(&state, &itf);
Staircase stair(&state, &itf, &vars);

void writeLED(int lednum, float brightness){
  int bi = brightness * ANALOG_WRITE_MAX;
  switch(lednum){
    case 0: analogWrite(PIN_LED_0, bi); noInterrupts(); state.led0_bright=brightness; interrupts(); break;
    case 1: analogWrite(PIN_LED_1, bi); noInterrupts(); state.led1_bright=brightness; interrupts(); break;
    case 2: analogWrite(PIN_LED_2, bi); noInterrupts(); state.led2_bright=brightness; interrupts(); break;
    case 3: analogWrite(PIN_LED_3, bi); noInterrupts(); state.led3_bright=brightness; interrupts(); break;
    case 4: analogWrite(PIN_LED_4, bi); noInterrupts(); state.led4_bright=brightness; interrupts(); break;
    case 5: analogWrite(PIN_LED_5, bi); noInterrupts(); state.led5_bright=brightness; interrupts(); break;
  }
}

void writeBNC(float volts){
  noInterrupts();
  if(state.bnc_mode == BNC_MODE_OUTPUT){
    state.bnc_out_volts = constrain(volts,0,ANALOG_WRITE_REF_VOLT);
    int val = volts / float(ANALOG_WRITE_REF_VOLT) * float(ANALOG_WRITE_MAX);
    analogWrite(PIN_BNC_ANALOG, val);
  }
  interrupts();
}

float readBNC(){
  noInterrupts();
  if(state.bnc_mode == BNC_MODE_INPUT){
    int val = analogRead(PIN_BNC_ANALOG);
    float volts = float(val)/float(ANALOG_READ_MAX) * float(ANALOG_READ_REF_VOLT);
    return volts;
  }else{
    return 0;
  }
  interrupts();
}

void writeBNCMode(BNC_MODE mode){
  noInterrupts();
  state.bnc_mode = mode;
  switch(mode){
    case BNC_MODE_INPUT: state.bnc_out_volts=0; break;
    case BNC_MODE_OUTPUT: break;
  }
  interrupts();
}

float readTACS(){
  int val = analogRead(PIN_TACS_ANALOG_IN);
  float volt = float(val)/float(ANALOG_READ_MAX) * float(ANALOG_READ_REF_VOLT);
  return volt;
}

bool readLeftButton(){
  return !digitalRead(PIN_LEFT_BUTTON);
}

bool readRightButton(){
  return !digitalRead(PIN_RIGHT_BUTTON);
}

void sendCommandDone(uint8_t status_val, COMM_CODE_T comm_code ){
    Writer *w = itf.getWriter();
    w->writeUINT8(RC_CMD_DONE);
    w->writeUINT8(status_val); // aborted
    w->writeUINT8(comm_code); // aborted
    w->send();
}

bool parseArgument(TextReader* r, long double &val) {
  char arg_str[256];
  auto arg = r->readSTRING(arg_str,256);
  if(arg.isError()){
    return false;
  }
  uint8_t arg_len = arg.getValue();
  // see if we can find the variable
  size_t found_idx;
  if ( vars.findVariable(arg_str, arg_len, &found_idx) ){
    // variable found, return its value.
    auto* var = vars.getVariable(found_idx);
    val = (long double) var->getLDValue();
    return true;
  }
  // parse the read text as a floating point number
  char* pStart = arg_str;
  char* pEnd = NULL;
  long double ld_val = strtold(pStart, &pEnd);
  if(pStart!=pEnd){
    val = (long double) ld_val;
    return true;
  }
  return false;
}

bool parseArgument(TextReader* r, long long &val) {
  char arg_str[256];
  auto arg = r->readSTRING(arg_str,256);
  if(arg.isError()){
    return false;
  }
  uint8_t arg_len = arg.getValue();
  // see if we can find the variable
  size_t found_idx;
  if ( vars.findVariable(arg_str, arg_len, &found_idx) ){
    // variable found, return its value.
    auto* var = vars.getVariable(found_idx);
    val = (long long) var->getLLValue();
    return true;
  }
  // parse the read text as a floating point number
  char* pStart = arg_str;
  char* pEnd = NULL;
  long long ll_val = strtoll(pStart, &pEnd, 0);
  if(pStart!=pEnd){
    val = (long long) ll_val;
    return true;
  }
  return false;
}


void initCommands(){

  itf.addHandler(CC_DEVICE_LOG_LEVEL,[](Reader *r){
    auto loglevel = r->readUINT8();
    if(loglevel.isError()){
      sendCommandDone(EC_PARSE,CC_DEVICE_LOG_LEVEL);
      return;
    }
    set_dlog_level(loglevel.getValue());
    // command done
    sendCommandDone(EC_NONE,CC_DEVICE_LOG_LEVEL);
  });

  itf.addHandler(CC_WRITE_LED,[](Reader *r){
    auto lednum = r->readUINT8();
    auto brightness = r->readFLOAT();
    if(lednum.isError() || brightness.isError()){
      sendCommandDone(EC_PARSE,CC_WRITE_LED);
      return;
    }
    writeLED(lednum.getValue(),brightness.getValue());
    // command done
    sendCommandDone(EC_NONE,CC_WRITE_LED);
  });

  itf.addHandler(CC_WRITE_BNC,[](Reader *r){
    auto volts = r->readFLOAT();
    if(volts.isError()){
      sendCommandDone(EC_PARSE,CC_WRITE_BNC);
      return;
    }
    writeBNC(volts.getValue());
    // command done
    sendCommandDone(EC_NONE,CC_WRITE_BNC);
  });

  itf.addHandler(CC_WRITE_BNC_MODE,[](Reader *r){
    auto mode = r->readUINT8();
    if(mode.isError()){
      sendCommandDone(EC_PARSE,CC_WRITE_BNC_MODE);
      return;
    }
    writeBNCMode((BNC_MODE)mode.getValue());
    // command done
    sendCommandDone(EC_NONE,CC_WRITE_BNC_MODE);
  });

  // clear
  itf.addHandler(CC_CLEAR_STIM,[](Reader *r){
    script.clear();
    // command done
    sendCommandDone(EC_NONE,CC_CLEAR_STIM);
  });

  // log
  itf.addHandler(CC_LOG,[](Reader *r){
    // Note: All that matters is this command IS echoed back
    // command done
    sendCommandDone(EC_NONE,CC_LOG);
  });

/*
  // stim
  itf.addHandler(CC_STIM,[](Reader *r){
    auto led0 = r->readFLOAT();
    auto led1 = r->readFLOAT();
    auto led2 = r->readFLOAT();
    auto led3 = r->readFLOAT();
    auto led4 = r->readFLOAT();
    auto led5 = r->readFLOAT();
    auto dur = r->readUINT32();
    if(led0.isError() || led1.isError() || led2.isError() || led3.isError() || led4.isError() || led5.isError() || dur.isError()){
      sendCommandDone(EC_PARSE,CC_STIM);
      return;
    }
    // create the command structure
    CommandStim cmd;
    cmd.setBright(0,led0.getValue());
    cmd.setBright(1,led1.getValue());
    cmd.setBright(2,led2.getValue());
    cmd.setBright(3,led3.getValue());
    cmd.setBright(4,led4.getValue());
    cmd.setBright(5,led5.getValue());
    cmd.dur = dur.getValue();
    // add command to script
    script.addStimCommand(&cmd);
    // command done
    sendCommandDone(EC_NONE,CC_STIM);
  });
*/

  // stim
  itf.addHandler(CC_STIM,[](Reader *r){
    // Make sure input type is TEXT only.
    if(itf.getInputType() != INTERFACE_INPUT_TYPE_TEXT){
      sendCommandDone(EC_PARSE,CC_STIM);
      return;
    }
    // Cast the Reader to a TextReader
    TextReader *tr = (TextReader*) r;
    // Parse LED values & duration
    long double led0 = 0;
    long double led1 = 0;
    long double led2 = 0;
    long double led3 = 0;
    long double led4 = 0;
    long double led5 = 0;
    long long dur = 0;
    if ( !parseArgument(tr, led0) || !parseArgument(tr, led1) || !parseArgument(tr, led2) ||
         !parseArgument(tr, led3) || !parseArgument(tr, led4) || !parseArgument(tr, led5) ||
         !parseArgument(tr, dur)){
           sendCommandDone(EC_PARSE,CC_STIM);
           return;
    }
    // Create the command structure
    CommandStim cmd;
    cmd.setBright(0,(float)led0);
    cmd.setBright(1,(float)led1);
    cmd.setBright(2,(float)led2);
    cmd.setBright(3,(float)led3);
    cmd.setBright(4,(float)led4);
    cmd.setBright(5,(float)led5);
    cmd.dur = (uint32_t) dur;
    // add command to script
    script.addStimCommand(&cmd);
    // command done
    sendCommandDone(EC_NONE,CC_STIM);
  });

  // wait
  itf.addHandler(CC_WAIT,[](Reader *r){
    // Make sure input type is TEXT only.
    if(itf.getInputType() != INTERFACE_INPUT_TYPE_TEXT){
      sendCommandDone(EC_PARSE,CC_WAIT);
      return;
    }
    // Cast the Reader to a TextReader
    TextReader *tr = (TextReader*) r;
    // Parse duration
    long long dur = 0;
    if ( !parseArgument(tr, dur)){
      sendCommandDone(EC_PARSE,CC_WAIT);
      return;
    }
    // create the command structure
    CommandWait cmd;
    cmd.dur = (uint32_t) dur;
    // add command to script
    script.addWaitCommand(&cmd);
    // command done
    sendCommandDone(EC_NONE,CC_WAIT);
  });

  itf.addHandler(CC_TRIG,[](Reader *r){
    // Make sure input type is TEXT only.
    if(itf.getInputType() != INTERFACE_INPUT_TYPE_TEXT){
      sendCommandDone(EC_PARSE,CC_TRIG);
      return;
    }
    // Cast the Reader to a TextReader
    TextReader *tr = (TextReader*) r;
    // Parse duration
    long double on_volts = 0;
    long long dur = 0;
    if ( !parseArgument(tr, on_volts) || !parseArgument(tr, dur)){
      sendCommandDone(EC_PARSE,CC_TRIG);
      return;
    }
    // create the command structure
    CommandTrigger cmd;
    cmd.on_volts = (float) on_volts;
    cmd.dur = (uint32_t) dur;
    // add command to script
    script.addTriggerCommand(&cmd);
    // command done
    sendCommandDone(EC_NONE,CC_TRIG);
  });

  // start
  itf.addHandler(CC_START_STIM,[](Reader *r){
    script.start();
  });

  // stop
  itf.addHandler(CC_STOP_STIM,[](Reader *r){
    script.stop();
    // command done
    sendCommandDone(EC_NONE,CC_STOP_STIM);
  });

  // pause
  itf.addHandler(CC_PAUSE_STIM,[](Reader *r){
    script.pause();
    // command done
    sendCommandDone(EC_NONE,CC_PAUSE_STIM);
  });

  itf.addHandler(CC_START_STAIR,[](Reader *r){
    auto baseBright = r->readFLOAT();
    auto targetBright = r->readFLOAT();
    auto stepSize  = r->readFLOAT();
    auto numTrials = r->readUINT32();
    auto targetDur = r->readUINT32();
    auto isiMin = r->readUINT32();
    auto isiMax = r->readUINT32();
    auto numUp = r->readUINT32();
    auto numDown = r->readUINT32();
    if(baseBright.isError() || targetBright.isError() || stepSize.isError()  ||
       numTrials.isError() || targetDur.isError() || isiMin.isError() || 
       isiMax.isError() || numUp.isError() || numDown.isError()){
      sendCommandDone(EC_PARSE,CC_START_STAIR);
      return;
    }
    // add stairs
    stair.start(baseBright.getValue()*255.0f, targetBright.getValue()*255.0f, stepSize.getValue()*255.0f,
                numTrials.getValue(), targetDur.getValue(), isiMin.getValue(), 
                isiMax.getValue(), numUp.getValue(), numDown.getValue() );
  });

  itf.addHandler(CC_STOP_STAIR,[](Reader *r){
    stair.stop();
    // command done
    sendCommandDone(EC_NONE,CC_STOP_STAIR);
  });

  itf.addHandler(CC_SET_VAR,[](Reader *r){
    if(itf.getInputType() == INTERFACE_INPUT_TYPE_TEXT){
      uint8_t status = vars.addVariable((TextReader*)r);
      switch(status){
        case 0:
          // command done
          sendCommandDone(EC_NONE,CC_SET_VAR);
          break;
        case 1:
          // command done
          sendCommandDone(EC_PARSE,CC_SET_VAR);
          break;
        case 2:
          // command done
          sendCommandDone(EC_MAX_VARS,CC_SET_VAR);
          break;
      }
    }else{
      // command parse error
      sendCommandDone(EC_PARSE,CC_SET_VAR);
    }
  });

  itf.addHandler(CC_GET_VAR,[](Reader *r){
    if(itf.getInputType() == INTERFACE_INPUT_TYPE_TEXT){
      auto var = vars.getVariable((TextReader*)r);
      if ( var != NULL ){
        // send the variable value
        Writer *w = itf.getWriter();
        w->writeUINT8(RC_VAR_VALUE);
        w->writeSTRING(var->getName());
        w->writeSTRING(var->getValue());
        w->send();
        
        // command done
        sendCommandDone(EC_NONE,CC_GET_VAR);
        return;
      }else{
        // command variable does not exist
        sendCommandDone(EC_VAR_DNE,CC_GET_VAR);
        return;
      }
    }
    // command parse error
    sendCommandDone(EC_PARSE,CC_GET_VAR);
  });

  itf.addHandler(CC_DELETE_VAR,[](Reader *r){
    if(itf.getInputType() == INTERFACE_INPUT_TYPE_TEXT && 
       vars.removeVariable((TextReader*)r) ){
      // command done
      sendCommandDone(EC_NONE,CC_DELETE_VAR);
    }else{
      // command parse error
      sendCommandDone(EC_PARSE,CC_DELETE_VAR);
    }
  });

  itf.addHandler(CC_DELETE_VARS,[](Reader *r){
    vars.clear();
    // command done
    sendCommandDone(EC_NONE,CC_DELETE_VARS);
  });
}

struct DataLogElement{
  unsigned long currus;
  bool left_button;
  bool right_button;
  float tacs_volts;
  float bnc_in_volts;
};

DataLogElement defaultDataLogElement = {.currus = 0,};

#define LOG_BUFFER_CAPACITY 100
CircularBuffer<DataLogElement, LOG_BUFFER_CAPACITY> data_log_buffer;

void log (){
  DataLogElement element;
  element.currus = micros();
  element.left_button = readLeftButton();
  element.right_button = readRightButton();
  element.tacs_volts = readTACS();
  element.bnc_in_volts = readBNC();

  data_log_buffer.add(element);
}

void send_log(){
  while(true){
    DataLogElement element;
    // DLOGV("","size: %d", data_log_buffer.getSize()  );
    noInterrupts();
    if(data_log_buffer.getSize() > 0){
      element = data_log_buffer.remove(defaultDataLogElement);
    }else{
      interrupts();
      return;
    }
    interrupts();

    Writer *w = itf.getWriter();
    w->writeUINT8(RC_DATA);
    w->writeUINT64(element.currus);
    w->writeFLOAT(element.tacs_volts);
    w->writeUINT8(state.bnc_mode); 
    w->writeFLOAT(element.bnc_in_volts);
    w->writeFLOAT(state.bnc_out_volts);
    w->writeBOOL(element.left_button ,false);
    w->writeBOOL(element.right_button,false);
    w->writeFLOAT(state.led0_bright);
    w->writeFLOAT(state.led1_bright);
    w->writeFLOAT(state.led2_bright);
    w->writeFLOAT(state.led3_bright);
    w->writeFLOAT(state.led4_bright);
    w->writeFLOAT(state.led5_bright);
    w->send();
  }
}

void setup() {
  Serial.begin(921600); //115200

  // setup interface
  itf.begin(&Serial);
  initCommands();

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(PIN_LED_0, OUTPUT);
  pinMode(PIN_LED_1, OUTPUT);
  pinMode(PIN_LED_2, OUTPUT);
  pinMode(PIN_LED_3, OUTPUT);
  pinMode(PIN_LED_4, OUTPUT);
  pinMode(PIN_LED_5, OUTPUT);

  // tri-state the BNC digital pin
  pinMode(PIN_BNC_DIGITAL, INPUT);
  writeBNCMode(BNC_MODE_OUTPUT);

  pinMode(PIN_LEFT_BUTTON,INPUT_PULLUP);
  pinMode(PIN_RIGHT_BUTTON,INPUT_PULLUP);

  analogReadResolution(ANALOG_READ_RESOLUTION);
  analogWriteResolution(ANALOG_WRITE_RESOLUTION);

  // seed the random value
  randomSeed(analogRead(PIN_TACS_ANALOG_IN));

  timer.begin(log,1000);
}

void loop() {

  itf.handleMessages();

  send_log();

  script.run();

  stair.run();

}



