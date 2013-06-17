#ifndef __basher_h__
#define __basher_h__


#include "gyro.h"
/*#include "Tommys.h"*/


#ifdef RECORD

#ifdef __basher_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN word count;
#undef EXTERN
#define EXTERN extern

#endif

 void plottext();

 void keyboard_link();

 void cursor_on();

 void get_demorec();

 boolean demo_ready();

 void cursor_off();

 void filename_edit();
 void normal_edit();
#ifdef RECORD
  void record_one();
#endif

#endif
