#ifndef _ARMNATIVE_H_
#define _ARMNATIVE_H_

#include "PNOLoader.h"

#ifdef WIN32
	#include "testing/SimNative.h"
	#include "testing/oscalls.h"
#endif

//#define DISABLE_ARM
//#define DEBUG_ARM

// rsrc
enum {
	ARM_PA1SND		= 1,
	ARM_STREAMSND,
	ARM_OWIDELS,
	ARM_OWIDEPT,
	ARM_OCOPYRECT
};

#define TRUE	1
#define FALSE	0

// types
typedef struct {
	void *srcP;
	void *dstP;
	UInt32 length;
} ARMPa1SndType, *ARMPa1SndPtr;

typedef struct {
	void *proc;
	void *param;

	void	*handle;	// sound handle
	UInt32	size;		// buffer size
	UInt32	slot;
	UInt32 	active,		// is the sound handler active
			set,		// is the buffer filled
			wait;		// do we need to wait for sound completion
	void	*dataP,		// main buffer
			*tmpP;		// tmp buffer (convertion)
} SoundDataType;

typedef struct {
	void *dst;
	void *src;
} OSysWideType , *OSysWidePtr;

typedef struct {
	void *dst;
	const void *buf;
	UInt32 pitch, _offScreenPitch;
	UInt32 w, h;
} OSysCopyType, *OSysCopyPtr;

// calls
MemPtr	_PceInit(DmResID resID);
UInt32	_PceCall(void *armP, void *userDataP);
void	_PceFree(void *armP);

MemPtr	_PnoInit(DmResID resID, PnoDescriptor *pnoP);
UInt32	_PnoCall(PnoDescriptor *pnoP, void *userDataP);
void	_PnoFree(PnoDescriptor *pnoP, MemPtr armP);

#endif