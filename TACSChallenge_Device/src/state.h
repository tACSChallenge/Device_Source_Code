#ifndef _STATE_H_
#define _STATE_H_



enum BNC_MODE {BNC_MODE_INPUT=0, BNC_MODE_OUTPUT=1};

class State{
public:
  volatile BNC_MODE bnc_mode;
  volatile float bnc_out_volts;
  volatile float led0_bright;
  volatile float led1_bright;
  volatile float led2_bright;
  volatile float led3_bright;
  volatile float led4_bright;
  volatile float led5_bright;
  State(): bnc_mode(BNC_MODE_INPUT), bnc_out_volts(0),
    led0_bright(0),led1_bright(0),led2_bright(0),led3_bright(0),led4_bright(0),led5_bright(0){
  }
};



#endif //_STATE_H_