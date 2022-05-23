#ifndef _MATH_UTIL_H_
#define _MATH_UTIL_H_

/**********************************************/
// CONSTANTS

#define MAX_UNSIGNED_LONG 4294967295UL
#define PI_FLOAT     3.14159265f
#define PIBY2_FLOAT  1.5707963f
#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923 
#endif

/**********************************************/
// BASIC MATH UTILITIES

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
unsigned long scaleSecondsToMicroseconds(float sec);
unsigned long scaleSecondsToMilliseconds(float sec);
float wrapTo2Pi(float angle);

/**********************************************/
// MAGNITUDE APPROXIMATION FUNCTIONS

int mult_shft( int a, int b);
int fastMag(int r, int i);
double fastMag(int *h_r, int *h_i, int si, int ei);

/**********************************************/
// ATAN2 APPROXIMATION FUNCTIONS

float atan2_approximation1(float y, float x);
float atan2_approximation2( float y, float x );


#endif //_MATH_UTIL_H_
