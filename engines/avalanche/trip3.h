/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */
#ifndef __trip3_h__
#define __trip3_h__


#include "gyro.h"


 void loadtrip();

 void boundscheck(integer& x,integer& y, byte xm,byte ym);

 void budge(byte who, shortint xx,shortint yy, byte frame); /* Moving & animation controller */

 void tripkey(char dir);

 void trippancy();

 void enter(byte what_handle, integer xx,integer yy,integer lx,integer ly, byte mmx,byte mmy,byte st);

#endif
