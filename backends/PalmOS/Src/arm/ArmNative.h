#ifndef _ARMNATIVE_H_
#define _ARMNATIVE_H_

#ifdef WIN32
	#include "testing/SimNative.h"
	#include "testing/oscalls.h"
#endif

// functions
typedef unsigned long (*PnoProc)(void *userData68KP);

#define DECLARE(x)	unsigned long x(void *userData68KP);

typedef struct {
	UInt32 func;
	void *dst;
	void *src;
	
} DataOSysWideType , *DataOSysWidePtr;

typedef struct {
	UInt32 func;
	void *dst;
	const void *buf;
	UInt32 pitch, _offScreenPitch;
	UInt32 w, h;
} DataOSysCopyRectType, *DataOSysCopyRectPtr;

DECLARE(OSystem_PALMOS_update_screen__wide_portrait)
DECLARE(OSystem_PALMOS_update_screen__wide_landscape)
DECLARE(OSystem_PALMOS_copy_rect)

// rsrc
#define ARMCODE_1	1000

// function indexes
enum {
	kOSysWidePortrait = 0,
	kOSysWideLandscape,
	kOSysCopyRect
};

#endif