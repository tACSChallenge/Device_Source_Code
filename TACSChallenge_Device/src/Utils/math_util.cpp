#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "math_util.h"

/**********************************************/
// BASIC MATH UTILITIES


// Linearly scales a number x, from the range [in_min, in_max] to the range [out_min, out_max].
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Converts seconds to microseconds.
unsigned long scaleSecondsToMicroseconds(float sec){
  if (sec<0 || sec>4294.967f){
    return MAX_UNSIGNED_LONG;
  }else{
    return (sec * 1000000.0f) + 0.5f;
  }
}

// Converts microseconds to seconds.
unsigned long scaleSecondsToMilliseconds(float sec){
  if (sec<0 || sec>4294.967f){
    return MAX_UNSIGNED_LONG;
  }else{
    return (sec * 1000.0f) + 0.5f;
  }
}

// Wraps a radian angle to lie between 0 and 2*pi.
float wrapTo2Pi(float angle) {
  angle = fmod(angle, TWO_PI);
  return angle < 0 ? angle + TWO_PI : angle;
}

/**********************************************/
// MAGNITUDE APPROXIMATION FUNCTIONS

// Helper function for fast magnitude computation.
int mult_shft( int a, int b)  
{
  return (( a  *  b )  >> 12);      
}

// Approximation of sqrt(r*r+i*i)
int fastMag(int r, int i)
{  
  const int16_t  alpha =  3881, beta = 1608;   
  int tmp_M;
  int abs_R = abs(r);
  int abs_I = abs(i);
  if (abs_R > abs_I) tmp_M = mult_shft( alpha, abs_R) + mult_shft( beta, abs_I);
  else               tmp_M = mult_shft( alpha, abs_I) + mult_shft( beta, abs_R);
  return tmp_M;
}

// Approximation of sqrt(r*r+i*i) for the an array h_r of element r,
// and an array h_i of element i, starting at array index si and ending
// at array index ei.
double fastMag(int *h_r, int *h_i, int si, int ei)
{
  const int16_t alpha = 3881, beta = 1608;
  double avgmag = 0;
  for ( int  i = si; i < ei; i++) { //Fastest Magnitude Calculation, w/o slow sqrt.
    int abs_R = abs(h_r[i]);
    int abs_I = abs(h_i[i]);
    int tmp_M;
    if (abs_R > abs_I) tmp_M = mult_shft( alpha, abs_R) + mult_shft( beta, abs_I);
    else               tmp_M = mult_shft( alpha, abs_I) + mult_shft( beta, abs_R);
    avgmag += tmp_M;
  }
  return avgmag / (ei - si);
}

/**********************************************/
// ATAN2 APPROXIMATION FUNCTIONS

// Two approximations of the atan2 function that 
// are faster to compute than its standard C 
// implementation, but offer different balances 
// between speed and accuracy. 
// These functions are used by ECHT to compute 
// the instantaneous phase given a complex number 
// (i.e. "atan2_approximation2(ci,cr)", 
// where 'ci' is the imaginary component and 
// 'cr' is the real component of an imaginary 
// number.).
// 
// Approximations Copied From:
// https://gist.github.com/volkansalma/2972237
// http://pubs.opengroup.org/onlinepubs/
//        009695399/functions/atan2.html
// Volkan SALMA
/**********************************************/


float atan2_approximation1(float y, float x)
{
  const float ONEQTR_PI = M_PI / 4.0;
  const float THRQTR_PI = 3.0 * M_PI / 4.0;
  float r, angle;
  // kludge to prevent 0/0 condition
  float abs_y = fabs(y) + 1e-10f;      
  if ( x < 0.0f )
  {
    r = (x + abs_y) / (abs_y - x);
    angle = THRQTR_PI;
  }
  else
  {
    r = (x - abs_y) / (x + abs_y);
    angle = ONEQTR_PI;
  }
  angle += (0.1963f * r * r - 0.9817f) * r;
  if ( y < 0.0f )
    return( -angle );     // negate if in quad III or IV
  else
    return( angle );
}

float atan2_approximation2( float y, float x )
{
  if ( x == 0.0f )
  {
    if ( y > 0.0f ) return PIBY2_FLOAT;
    if ( y == 0.0f ) return 0.0f;
    return -PIBY2_FLOAT;
  }
  float atan;
  float z = y/x;
  if ( fabs( z ) < 1.0f )
  {
    atan = z/(1.0f + 0.28f*z*z);
    if ( x < 0.0f )
    {
      if ( y < 0.0f ) return atan - PI_FLOAT;
      return atan + PI_FLOAT;
    }
  }
  else
  {
    atan = PIBY2_FLOAT - z/(z*z + 0.28f);
    if ( y < 0.0f ) return atan - PI_FLOAT;
  }
  return atan;
}
