#ifndef _RUN_EVERY_H_
#define _RUN_EVERY_H_

// COPIED FROM:
// https://forum.arduino.cc/index.php?topic=124974.0
// ORIGINALLY POSTED BY: "WizenedEE"

#ifdef __cplusplus
template <bool _Flag, typename _True, typename _False> struct If{ typedef _True Result; };
template <typename _True, typename _False> struct If<false, _True, _False>{ typedef _False Result; };

template< uint64_t _Number >
class BestFitUInt{
protected:
public: // Remove for final version
   static const uint32_t max_32 =  ((~(uint32_t)0) - 100);
   static const uint16_t max_16 =  ((~(uint16_t)0) - 100);
   static const uint8_t  max_8  =  ((~(uint8_t )0) - 100);

   // uint64_t should be an error.. Perhaps use this to link in a file with
   //  #error explaining better?
   typedef typename If< (_Number <= max_32), uint32_t, uint64_t >::Result tA;
   typedef typename If< (_Number <= max_16), uint16_t, tA >::Result tB;
public:
   typedef typename If< (_Number <= max_8 ), uint8_t , tB >::Result Result;
};

#define RETYPE(n) BestFitUInt< n >::Result

#else // cplusplus
#define RETYPE(n) uint32_t
#endif //cplusplus

#define runEvery(t) for (static  RETYPE(t) _lasttime;\
                        (RETYPE(t))((RETYPE(t))millis() - _lasttime) >= (t);\
                        _lasttime += (t))

#define DEBUG(x) do {Serial.print(#x ": "); Serial.print(x); Serial.println(/*'\t'*/); } while (0)

#define LOOKATTYPE(n) do {typename BestFitUInt< n >::Result var; Serial.print("Number: "); Serial.print(n); Serial.print("\tType: "); Serial.println(mytypeof(var)); } while(0)

template <class T> const char* mytypeof(T&) { return "unknown";}
template <> const char* mytypeof(uint8_t &) { return "uint8_t ";}
template <> const char* mytypeof(uint16_t&) { return "uint16_t";}
template <> const char* mytypeof(uint32_t&) { return "uint32_t";}
template <> const char* mytypeof(uint64_t&) { return "uint64_t";}

/*
void setup() {
   Serial.begin(115200);

   DEBUG(BestFitUInt<5>::max_32);
   DEBUG(BestFitUInt<5>::max_16);
   DEBUG(BestFitUInt<5>::max_8);
   
   Serial.println();

   LOOKATTYPE(126);
   LOOKATTYPE(127);
   LOOKATTYPE(128);

   Serial.println();

   LOOKATTYPE(154);
   LOOKATTYPE(155);
   LOOKATTYPE(156);

   Serial.println();

   LOOKATTYPE(65434);
   LOOKATTYPE(65435);
   LOOKATTYPE(65436);

   Serial.println();

   LOOKATTYPE(65534);
   LOOKATTYPE(65535);
   LOOKATTYPE(65536);

   Serial.println();

   LOOKATTYPE(4294967194);
   LOOKATTYPE(4294967195);
   LOOKATTYPE(4294967196);

   Serial.println();

   LOOKATTYPE(4294967294);
   LOOKATTYPE(4294967295);
   //LOOKATTYPE(4294967296); // Serial.print can't handle 64 bit

}

void loop() {

}
*/

#endif //_RUN_EVERY_H_