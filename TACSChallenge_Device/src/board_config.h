#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

// I am using Pin A0 as the analog input for recording the TACS waveform from the instrumentation amplifier.
// I am using pins 3,4,5,6,9,10 as the PWM output to drive the six LED's (each LED is powered directly via the teensy; the brightness is controlled using PWM).
// I am using pin 0 for the trigger in/out that connects to the BNC socket.

#define PIN_TACS_ANALOG_IN A0

#define PIN_LED_0 10
#define PIN_LED_1 3
#define PIN_LED_2 4
#define PIN_LED_3 5
#define PIN_LED_4 6
#define PIN_LED_5 9

#define PIN_BNC_DIGITAL 2
#define PIN_BNC_ANALOG A14

#define PIN_LEFT_BUTTON 8
#define PIN_RIGHT_BUTTON 7

#define ANALOG_READ_RESOLUTION 13
#define ANALOG_WRITE_RESOLUTION 13

#define ANALOG_READ_REF_VOLT 3.3
#define ANALOG_WRITE_REF_VOLT 3.3

#define ANALOG_READ_MAX (1 << ANALOG_READ_RESOLUTION)
#define ANALOG_WRITE_MAX (1 << ANALOG_WRITE_RESOLUTION)

#define VARIABLE_MAX_NUM 20
#define VARIABLE_MAX_NAME_STR_LENGTH 256
#define VARIABLE_MAX_VALUE_STR_LENGTH 256

#endif //_BOARD_CONFIG_H_