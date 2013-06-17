/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */
#ifndef __sezunit_h__
#define __sezunit_h__


/*#include "Gyro.h"*/


struct sezheader {
             array<1,2,char> initials;     /* should be "TT" */
             word gamecode;
             word revision; /* as 3- or 4-digit code (eg v1.00 = 100) */
             longint chains; /* number of scroll chains */
             longint size; /* total size of all chains */
};



#ifdef __sezunit_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN array<0,1999,char> chain; /* This chain */
EXTERN word chainsize; /* Length of "chain" */
EXTERN byte sezerror; /* Error code */
EXTERN sezheader sezhead;
#undef EXTERN
#define EXTERN extern


     /* Error codes for "sezerror" */
const integer sezok = 0;
const integer sezgunkyfile = 1;
const integer sezhacked = 2;


void sez_setup();

void getchain(longint number);


#endif
