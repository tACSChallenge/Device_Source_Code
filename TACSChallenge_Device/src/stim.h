#ifndef _STIM_H_
#define _STIM_H_

#include <Arduino.h>
#include "Interface/Writer.h"
#include "Interface/InterfaceAdapter.h"
#include "state.h"
#include "return_codes.h"
#include "device_log.h"
#include "board_config.h"

static const char *STIM_TAG = "stim";	// Logging prefix for this module

extern void writeLED(int lednum, float brightness);
extern void writeBNC(float volts);
extern void sendCommandDone(uint8_t status_val, COMM_CODE_T comm_code );

enum CommandType {COMMAND_TYPE_NONE=0, COMMAND_TYPE_LOG, COMMAND_TYPE_STIM, COMMAND_TYPE_WAIT, COMMAND_TYPE_TRIGGER};

struct CommandStim {
  uint16_t led_bright[6];
  uint32_t dur; // milliseconds
  float getBright(uint8_t i){
    return (float)led_bright[i]/(float)ANALOG_WRITE_MAX;
  }
  void setBright(uint8_t i, float bright){
    led_bright[i] = (uint16_t) (ANALOG_WRITE_MAX*bright);
  }
};

struct CommandWait {
  uint32_t dur; //milliseconds
};

struct CommandTrigger {
  float on_volts; 
  float off_volts;
  uint32_t dur; //milliseconds
};

struct CommandRecord {
  CommandType type;
  size_t buf_idx;
  size_t buf_len;
};

enum CommandMode {MODE_STOPPED, MODE_STARTED};

#define COMMAND_REC_MAX_SIZE 1100
#define COMMAND_BUF_MAX_SIZE (COMMAND_REC_MAX_SIZE*sizeof(CommandStim))

enum activeCommandState {ACTIVE_COMMAND_MODE_STANDBY, ACTIVE_COMMAND_MODE_RUNNING, ACTIVE_COMMAND_MODE_DONE};

class ActiveCommand {
public:
  activeCommandState state_;
  CommandRecord rec_;
  unsigned long start_ms_;
  ActiveCommand():
        state_(ACTIVE_COMMAND_MODE_DONE),
        start_ms_(0){
  }
  ActiveCommand(CommandRecord rec):
        state_(ACTIVE_COMMAND_MODE_STANDBY),
        rec_(rec),
        start_ms_(0)
  {  }
  void start(unsigned long currms){
    start_ms_ = currms;
  }
};

class Script{
private:
  State *state_;
  InterfaceAdapter *itf_;
  CommandMode mode = MODE_STOPPED;
  size_t command_rec_exec_idx;
  size_t command_buf_exec_idx;
  size_t command_rec_fill_idx;
  size_t command_buf_fill_idx;
  CommandRecord command_records[COMMAND_REC_MAX_SIZE];
  char command_buf[COMMAND_BUF_MAX_SIZE];

  ActiveCommand act_cmd;
public:

  Script(State *state, InterfaceAdapter* itf) : 
    state_(state), itf_(itf), 
    mode(MODE_STOPPED),
    command_rec_exec_idx(0),
    command_buf_exec_idx(0),
    command_rec_fill_idx(0),
    command_buf_fill_idx(0)
  {
    memset(command_records,0,sizeof(command_records));
    memset(command_buf,0,sizeof(command_buf));
  }

  bool addLogCommand(char* buf, size_t buf_len){
    DLOGI(STIM_TAG,"Adding LOG Command");
    return addCommand(COMMAND_TYPE_LOG, buf, buf_len);
  }

  bool addStimCommand(CommandStim *stim){
    DLOGI(STIM_TAG,"Adding Stim Command: %f %f %f %f %f %f %d", 
      stim->getBright(0), stim->getBright(1), stim->getBright(2), 
      stim->getBright(3), stim->getBright(4), stim->getBright(5), 
      stim->dur);
    return addCommand(COMMAND_TYPE_STIM, (char*) stim, sizeof(CommandStim));
  }

  bool addWaitCommand(CommandWait *wait){
    DLOGI(STIM_TAG,"Adding Wait Command: %d", wait->dur);
    return addCommand(COMMAND_TYPE_WAIT, (char*) wait, sizeof(CommandWait));
  }

  bool addTriggerCommand(CommandTrigger *trig){
    DLOGI(STIM_TAG,"Adding Trigger Command: %f %d", trig->on_volts, trig->dur);
    return addCommand(COMMAND_TYPE_TRIGGER, (char*) trig, sizeof(CommandTrigger));
  }

  void clear(){
    DLOGI(STIM_TAG,"Clear in-memory commands");
    command_rec_fill_idx = 0;
    command_buf_fill_idx = 0;
    command_rec_exec_idx = 0;
    command_buf_exec_idx = 0;
  }

  void start(){
    DLOGI(STIM_TAG,"Start in-memory command execution");
    mode = MODE_STARTED;
    // getCommand(act_cmd.rec_);
  }

  void stop(){
    DLOGI(STIM_TAG,"Stop in-memory command execution");
    if(mode == MODE_STARTED){
      mode = MODE_STOPPED;
      command_rec_exec_idx = 0;
      command_buf_exec_idx = 0;
      // send confirmation that the command finished
      sendCommandDone(1,CC_START_STIM); // aborted
    }
  }

  void pause(){
    DLOGI(STIM_TAG,"Paused in-memory command execution");
    mode = MODE_STOPPED;
  }

  void handleLog(unsigned long currms) {
    char* log = command_buf + act_cmd.rec_.buf_idx;
    size_t log_len = act_cmd.rec_.buf_len;
    switch(act_cmd.state_){
      case ACTIVE_COMMAND_MODE_STANDBY:
        DLOGI(STIM_TAG,"Handling Log Command");
        {
          // Report Log 
          noInterrupts();
          Writer *w = itf_->getWriter();
          w->writeUINT8(CC_LOG);
          w->writeSTRING(log,log_len);
          w->send();
          interrupts();
          // Transition to next state
          act_cmd.state_ = ACTIVE_COMMAND_MODE_DONE;
        }
        break;
      default:
        break;
    }
  }

  void handleStim(unsigned long currms) {
    CommandStim* stim = (CommandStim*) (command_buf + act_cmd.rec_.buf_idx);
    switch(act_cmd.state_){
      case ACTIVE_COMMAND_MODE_STANDBY:
        DLOGI(STIM_TAG,"Handling Stim Command: %f %f %f %f %f %f %d", 
          stim->getBright(0), stim->getBright(1), stim->getBright(2), 
          stim->getBright(3), stim->getBright(4), stim->getBright(5), 
          stim->dur);
        // Control LEDs
        writeLED(0, stim->getBright(0));
        writeLED(1, stim->getBright(1));
        writeLED(2, stim->getBright(2));
        writeLED(3, stim->getBright(3));
        writeLED(4, stim->getBright(4));
        writeLED(5, stim->getBright(5));
        // Record start time
        act_cmd.start(currms);
        // Transition to RUNNING
        act_cmd.state_ = ACTIVE_COMMAND_MODE_RUNNING;
        break;
      case ACTIVE_COMMAND_MODE_RUNNING:
        if(currms - act_cmd.start_ms_ >= stim->dur){
          // Transition to DONE
          act_cmd.state_ = ACTIVE_COMMAND_MODE_DONE;
        }
        break;
      default:
        break;
    }
  }

  void handleWait(unsigned long currms) {
    CommandWait* wait = (CommandWait*) (command_buf + act_cmd.rec_.buf_idx);
    switch(act_cmd.state_){
      case ACTIVE_COMMAND_MODE_STANDBY:
        DLOGI(STIM_TAG,"Handling Wait Command: %d", wait->dur);
        // Record start time
        act_cmd.start(currms);
        // Transition to RUNNING
        act_cmd.state_ = ACTIVE_COMMAND_MODE_RUNNING;
        break;
      case ACTIVE_COMMAND_MODE_RUNNING:
        if(currms - act_cmd.start_ms_ >= wait->dur){
          // Transition to DONE
          act_cmd.state_ = ACTIVE_COMMAND_MODE_DONE;
        }
        break;
      default:
        break;
    }
  }

  void handleTrigger(unsigned long currms) {
    CommandTrigger* trig = (CommandTrigger*) (command_buf + act_cmd.rec_.buf_idx);
    switch(act_cmd.state_){
      case ACTIVE_COMMAND_MODE_STANDBY:
        DLOGI(STIM_TAG,"Handling Trigger Command: %f %d", trig->on_volts, trig->dur);
        if(trig->dur == 0){
          // Save off the previous BNC trigger value
          trig->off_volts = trig->on_volts;
          // Turn ON BNC trigger
          writeBNC(trig->on_volts);
          // Leave the trigger on
          act_cmd.state_ = ACTIVE_COMMAND_MODE_DONE;
        }else{
          // Save off the previous BNC trigger value
          noInterrupts();
          trig->off_volts = state_->bnc_out_volts;
          interrupts();
          // Turn ON BNC trigger
          writeBNC(trig->on_volts);
          // Record start time
          act_cmd.start(currms);
          // Transition to RUNNING
          act_cmd.state_ = ACTIVE_COMMAND_MODE_RUNNING;
        }
        break;
      case ACTIVE_COMMAND_MODE_RUNNING:
        if(currms - act_cmd.start_ms_ >= trig->dur){
          // Turn OFF BNC trigger
          writeBNC(trig->off_volts);
          // Transition to DONE
          act_cmd.state_ = ACTIVE_COMMAND_MODE_DONE;
        }
        break;
      default:
        break;
    }
  }

  void run(){
    unsigned long currms = millis();
    if(mode == MODE_STOPPED){
      return;
    }
    // check if the previous command is done
    if( act_cmd.state_ == ACTIVE_COMMAND_MODE_DONE ){
      // get the next command
      if ( getCommand(act_cmd.rec_) ){
        act_cmd.state_ = ACTIVE_COMMAND_MODE_STANDBY;
      }else{
        act_cmd.state_ = ACTIVE_COMMAND_MODE_DONE;
        writeLED(0, 0);
        writeLED(1, 0);
        writeLED(2, 0);
        writeLED(3, 0);
        writeLED(4, 0);
        writeLED(5, 0);
        writeBNC(0);
        mode=MODE_STOPPED;
        // send confirmation that the command finished
        sendCommandDone(0,CC_START_STIM); // success
        return;
      }
    } 
    // execute command
    switch ( act_cmd.rec_.type ) {
      case COMMAND_TYPE_LOG:
        handleLog(currms);
        break;
      case COMMAND_TYPE_STIM:
        handleStim(currms);
        break;
      case COMMAND_TYPE_WAIT:
        handleWait(currms);
        break;
      case COMMAND_TYPE_TRIGGER:
        handleTrigger(currms);
        break;
      default:
        break;
    }
  }


private:

  bool addCommand(CommandType type, char* buf, size_t buf_len) 
  {
    CommandRecord rec = {.type=type, .buf_idx=command_buf_fill_idx, .buf_len=buf_len};
    DLOGD(STIM_TAG,"Adding command, type: %d, buf_idx: %d, buf_len %d", rec.type, rec.buf_idx, rec.buf_len);
    size_t new_command_rec_fill_idx = command_rec_fill_idx + 1;
    size_t new_command_buf_fill_idx = rec.buf_idx + rec.buf_len;
    if(new_command_rec_fill_idx >= COMMAND_REC_MAX_SIZE || 
       new_command_buf_fill_idx >= COMMAND_BUF_MAX_SIZE){
      DLOGE(STIM_TAG,"Failed to add command");
      return false;
    }
    command_records[command_rec_fill_idx] = rec;
    memcpy(command_buf + rec.buf_idx, buf, rec.buf_len);
    command_rec_fill_idx = new_command_rec_fill_idx;
    command_buf_fill_idx = new_command_buf_fill_idx;
    return true;
  }

  bool getCommand(CommandRecord &rec)
  {
    DLOGI(STIM_TAG,"Getting command");
    if(command_rec_exec_idx >= command_rec_fill_idx){
      DLOGI(STIM_TAG,"No more commands available");
      return false;
    }
    rec = command_records[command_rec_exec_idx];
    command_rec_exec_idx++;
    return true;
  }
};


#endif // _STIM_H_