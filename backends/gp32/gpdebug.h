//////////////////////////////////////////////////////////////////////////////
// debug.h                                                                  //
//////////////////////////////////////////////////////////////////////////////
#ifndef _GP_DEBUG_H
#define _GP_DEBUG_H

/*
	Debug library
	Note: include debug.h after stdio.h and conio.h!!!
*/

//////////////////////////////////////////////////////////////////////////////
// Includes                                                                 //
//////////////////////////////////////////////////////////////////////////////
#include "gpregs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// Defines                                                                  //
//////////////////////////////////////////////////////////////////////////////
// public
#define dprintf			__printf	//ph0x ..redefine rest?
#undef	getchar
#define getchar			__getchar
#undef	putchar
#define putchar(c)		__putchar(c)
#define gets(s)			__gets(s)
#define puts(s)			__puts(s)	// fixme?
#define kbhit()			__kbhit()	// uncomment?

// function -> constructor & class instance
//#define INIT(fn) class fn##_Init { public: fn##_Init() { fn(); } } fn##_init
//#define INIT1(fn,param1) class fn##_Init { public: fn##_Init() { fn(param1); } } fn##_init

//////////////////////////////////////////////////////////////////////////////
// Prototypes                                                               //
//////////////////////////////////////////////////////////////////////////////
extern void InitDebug(void);		// auto-initialized
extern int __kbhit(void);
extern void __putchar(int c);
extern int __getchar(void);		// non-blocking
extern int __printf(char *fmt, ...);
extern int __puts(const char *s);
extern char * __gets(char *s);

#endif // _DEBUG_H
