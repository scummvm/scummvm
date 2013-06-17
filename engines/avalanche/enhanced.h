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
