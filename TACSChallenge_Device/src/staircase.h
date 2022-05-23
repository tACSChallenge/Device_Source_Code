#ifndef _STAIRCASE_H_
#define _STAIRCASE_H_

#include <Arduino.h>
#include "Utils/CircularBuffer.h"
#include "Interface/InterfaceAdapter.h"
#include "board_config.h"
#include "return_codes.h"
#include "device_log.h"
#include "state.h"
#include "variables.h"


extern void writeLED(int lednum, float brightness);
extern bool readLeftButton();
extern bool readRightButton();
extern void send_log();
extern void sendCommandDone(uint8_t status_val, COMM_CODE_T comm_code );

static const char *STAIR_TAG = "stair"; // Logging prefix for this module

enum staircase_mode
{
    STAIR_MODE_STANDBY,
    STAIR_MODE_RUNNING
};

class Staircase
{
private:
    State *state_;
    InterfaceAdapter *itf_;
    Variables<VARIABLE_MAX_NUM,VARIABLE_MAX_NAME_STR_LENGTH,VARIABLE_MAX_VALUE_STR_LENGTH> *vars_;

    staircase_mode mode_;

    uint32_t numTrials_;
    uint32_t targetDur_;
    uint32_t isiMin_;
    uint32_t isiMax_;
    uint32_t numMiss_;
    uint32_t numHit_;

    float baseBrightness_;
    float step_;
    float stepSize_;
    uint32_t numHitCount;
    uint32_t numMissCount;
    
    // changed 05.04.22 Florian Kasten
    
    // uint8_t previousResp;
    uint8_t previousStep;
    uint8_t currentStep;
    uint8_t countSameSteps;

    // end of changes


    uint8_t response;

    uint8_t record_hit;
    float record_targetBrightness;

    unsigned long start_ms_;

    unsigned long handleDelay_start_ms_;

    size_t cb_cap = 10;
    CircularBuffer<float,10> cb;

    boolean handleDelay(unsigned long delayms){
        handleDelay_start_ms_ = millis();
        while(millis() - handleDelay_start_ms_ < delayms ){
            if ( systemTasks() ){
                return false;
            }
        }
        return false;
    }

    boolean systemTasks(){
        if (mode_ != STAIR_MODE_RUNNING) {
            return true;
        } else {
            itf_->handleMessages();
            send_log();
            return false;
        }
    }

public:
    Staircase(State *state, InterfaceAdapter *itf, 
              Variables<VARIABLE_MAX_NUM,VARIABLE_MAX_NAME_STR_LENGTH,VARIABLE_MAX_VALUE_STR_LENGTH> *vars) : 
              state_(state), itf_(itf), vars_(vars), mode_(STAIR_MODE_STANDBY)
    {
        mode_ = STAIR_MODE_STANDBY;
    }

    void start(float baseBrightness, float initStep, float stepSize, uint32_t numTrials, uint32_t targetDur, uint32_t isiMin, uint32_t isiMax, uint32_t numMiss, uint32_t numHit)
    {
        // local - from passed in values.
        numTrials_ = numTrials;
        targetDur_ = targetDur;
        isiMin_ = isiMin;
        isiMax_ = isiMax;
        numMiss_ = numMiss;
        numHit_ = numHit;

        // local
        baseBrightness_ = baseBrightness;
        step_ = initStep;
        stepSize_ = stepSize;
        numHitCount = 0;
        numMissCount = 0;
        
	// changed 05.04.22 Florian Kasten:
	// previousResp = 1;
	previousStep = 99;
	currentStep = 99;
	countSameSteps = 0;

	// end of changes

        // set the mode
        mode_ = STAIR_MODE_RUNNING;
    }

    void stop()
    {
        if(mode_ == STAIR_MODE_RUNNING){
            mode_ = STAIR_MODE_STANDBY;
            // command done
            sendCommandDone(1,CC_START_STAIR); // aborted
        }
    }

    void run()
    {
        if (mode_ == STAIR_MODE_STANDBY)
        {
            return;
        }

        // initialize diode
        writeLED(0, baseBrightness_ / 255.0f); 
        writeLED(1, baseBrightness_ / 255.0f);
        writeLED(2, baseBrightness_ / 255.0f);
        writeLED(3, baseBrightness_ / 255.0f);
        writeLED(4, baseBrightness_ / 255.0f);
        writeLED(5, baseBrightness_ / 255.0f);
        // wait a moment before start
        if( handleDelay(4000) ) return;

        // trial loop
        unsigned long t = millis(); //get initial timestamp

        //run trials for staircase
        for (uint32_t i = 0; i < numTrials_; i++)
        {
            DLOGI( STAIR_TAG, "Running staircase trial %d", i+1 );

            unsigned long t2 = millis();
            record_targetBrightness = step_/255.0f;
            // set random ISI
            if ( handleDelay(random(isiMin_ - (t2 - t), isiMax_ - (t2 - t))) ) return;
            // select the LED that will be dimmed
            int32_t led_selected = random(1,6);

            // present target
            writeLED(led_selected, (baseBrightness_ + step_) / 255.0f);
            if ( handleDelay(targetDur_) ) return;

            writeLED(led_selected, (baseBrightness_) / 255.0f); //return to base brightness
            t = millis();
            bool buttonInput = false;
            uint32_t Hit = 0;
            while (millis() - t <= 800)
            {
                if (systemTasks())  return;
                //buttonInput = digitalRead(buttonPin);
                buttonInput = readLeftButton() || readRightButton();
                if (buttonInput == true)
                {
                    Hit = 1;
                    break;
                }
            }

            if (Hit == 1) {
                numHitCount += 1;
            } else {
                numMissCount += 1;
            }

            float tempStep = 0;
            if (numHitCount == numHit_) {
                numHitCount = 0;
                numMissCount = 0;
                
		// changed 05.04.22 Florian Kasten
		currentStep = 1;
		// check if this was a reversal and decrease step size
		if (i>1 && previousStep != currentStep){
			countSameSteps = 0;
			stepSize_ = stepSize_ / 2.0f;
		} else {
			countSameSteps = countSameSteps + 1;
			if (countSameSteps > 2){
				stepSize_ = stepSize_ * 2.0f;
			}
			
		}
		// end of change
		
                tempStep = step_ - stepSize_;
                if ( ((baseBrightness_ + tempStep) <= 255.0f) &&
                    ((baseBrightness_ + tempStep) >= 0.0f  ))
                {
                    step_ = tempStep;
                }
                
            } else if (numMissCount == numMiss_) {
                numHitCount = 0;
                numMissCount = 0;
		
		// changed 05.04.22 Florian Kasten
		currentStep = 0;
		// check if this was a reversal and decrease step size
		if (i>1 && previousStep != currentStep){
			countSameSteps = 0;
			stepSize_ = stepSize_ / 2.0f;
		} else {
			countSameSteps = countSameSteps + 1;
			if (countSameSteps > 2){
				stepSize_ = stepSize_ * 2.0f;
			}
			
		}
		// end of change		


                tempStep = step_ + stepSize_;
                if ( ((baseBrightness_ + tempStep) <= 255.0f) &&
                    ((baseBrightness_ + tempStep) >= 0.0f  ))
                {
                    step_ = tempStep;
                }
            }
	    
            // changed 05.04.22 Florian Kasten
            // decrease stepSize after reversal
            // if (i > 5 && previousResp != Hit)
            // {
                // if (stepSize_ > 1)
                // {
            //        stepSize_ = stepSize_ / 2.0f;
                // }
            // }
		

            // previousResp = Hit;
	    previousStep = currentStep;
	    
  	    // end of changes

            record_hit = Hit;

            Writer *w = itf_->getWriter();
            w->writeUINT8(RC_STAIR_RESP);
            w->writeFLOAT(record_targetBrightness);
            w->writeUINT8(record_hit);
            w->send();

            // save off the value
            cb.add(record_targetBrightness);
        } // end for numTrials_
        
        mode_ = STAIR_MODE_STANDBY;

        // turn all LEDs off.
        // initialize diode
        writeLED(0, 0); 
        writeLED(1, 0);
        writeLED(2, 0);
        writeLED(3, 0);
        writeLED(4, 0);
        writeLED(5, 0);

        // compute the average of the last 10 brightnesses
        float avg_targetBrightness = 0;
        size_t cb_size = cb.getSize();
        for(size_t i=0; i<cb_size; i++){
            float targetBrightness = cb.remove(0); 
            avg_targetBrightness += targetBrightness;
        }
        avg_targetBrightness /= cb_size;

        // save off the variables
        vars_->addVariable((char*)"a", 1, (baseBrightness_) / 255.0f);
        vars_->addVariable((char*)"b", 1, baseBrightness_/255.0f + avg_targetBrightness );

        // command done
        sendCommandDone(0,CC_START_STAIR); // success
    } // end run()

};

#endif //_STAIRCASE_H_