/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */
#ifndef __enhanced_h__
#define __enhanced_h__



#ifdef __enhanced_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN byte shiftstate;/*ABSOLUTE $40:$17;*/
EXTERN boolean atbios;
EXTERN char inchar,extd;
#undef EXTERN
#define EXTERN extern


void readkeye();

boolean keypressede();

#endif
