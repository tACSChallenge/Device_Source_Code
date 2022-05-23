#ifndef _SIMULATOR_INTERFACE_H_
#define _SIMULATOR_INTERFACE_H_

#include "Interface/PacketSerial_Mod/src/PacketSerial_Mod.h"

union byteInt {
   byte b[4]; 
   int i;
   byteInt(int i_):i(i_){};
};

union byteFloat {
   byte b[4];
   float f;
   byteFloat(float f_):f(f_){};
};

class DataHandlerClass{
public:
  virtual void onDataFunction(float freq, float amp, float phase) = 0;
};

class StimulatorInterface : PacketHandlerClass{
public:
  typedef void (*DataHandlerFunction)(float freq, float amp, float phase);

  StimulatorInterface():
    freq_bF(0),amp_bF(0),phase_bF(0)
  {
  }
  
  void begin(HardwareSerial *s, int baud){
    // setup serial communication
    s->begin(baud);
    pSerial.setPacketHandler(this);
    pSerial.begin(s);
  }

  void sendToStimulator(float freq, float amp, float phase){
    // send data to the stimulator in binary 
    //(float) = 4 bytes
    // convert the frequency
    freq_bF.f = freq;
    // convert the amplitude
    amp_bF.f = amp;
    // convert the phase
    phase_bF.f = phase;
    // send the data
    uint8_t buf[] = {freq_bF.b[0],freq_bF.b[1],freq_bF.b[2],freq_bF.b[3],
                     amp_bF.b[0],amp_bF.b[1],amp_bF.b[2],amp_bF.b[3],
                     phase_bF.b[0],phase_bF.b[1],phase_bF.b[2],phase_bF.b[3]};
    pSerial.send(buf,3*4);
  }

  void setDataHandler(DataHandlerFunction onDataFunction){
    onDataFunction_ = onDataFunction;
  }

  void setDataHandler(DataHandlerClass &onDataClass){
    onDataClass_ = &onDataClass;
  }  

  void getData(float &freq, float &amp, float &phase){
    freq = freq_bF.f;
    amp = amp_bF.f;
    phase = phase_bF.f;
  }

  void update(){
    pSerial.update();
  }

  void handlePacket(const uint8_t* buffer, size_t size){
    if(size != 4*4) return;
    freq_bF.b[0] = buffer[0];
    freq_bF.b[1] = buffer[1];
    freq_bF.b[2] = buffer[2];
    freq_bF.b[3] = buffer[3];
    amp_bF.b[0] = buffer[4];
    amp_bF.b[1] = buffer[5];
    amp_bF.b[2] = buffer[6];
    amp_bF.b[3] = buffer[7];
    phase_bF.b[0] = buffer[8];
    phase_bF.b[1] = buffer[9];
    phase_bF.b[2] = buffer[10];
    phase_bF.b[3] = buffer[11];
    if(onDataFunction_!=NULL){
      onDataFunction_(freq_bF.f,amp_bF.f,phase_bF.f);
    }
    if(onDataClass_!=NULL){
      onDataClass_->onDataFunction(freq_bF.f,amp_bF.f,phase_bF.f);
    }
  }
  
private:
  PacketSerial pSerial;
  byteFloat freq_bF;
  byteFloat amp_bF;
  byteFloat phase_bF;
  DataHandlerFunction onDataFunction_;
  DataHandlerClass *onDataClass_;
  
  void onPacketFunction(const uint8_t* buffer, size_t size){
    handlePacket(buffer,size);
  }
  
};










#endif /*_SIMULATOR_INTERFACE_H_*/
