#ifndef _ARMNATIVE_H_
#define _ARMNATIVE_H_

#include "PNOLoader.h"

//#define DISABLE_ARM
//#define DEBUG_ARM

#ifndef __PALM_OS__

typedef UInt8 byte;
typedef UInt8 uint8;
typedef Int32 int32;
typedef UInt16 uint16;
typedef unsigned int uint;

#endif

// rsrc
enum {
	RSC_WIDELANDSCAPE = 1,
	RSC_WIDEPORTRAIT,
	RSC_COPYRECT,
	RSC_COSTUMEPROC3,
	RSC_DRAWSTRIP,
	RSC_BLIT
};

enum {
	PNO_COPYRECT = 0,
	PNO_WIDE,
	PNO_COSTUMEPROC3,
	PNO_DRAWSTRIP,
	PNO_BLIT,
	PNO_COUNT
};

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
} WideType;

typedef struct {
	void *dst;
	const void *buf;
	UInt32 pitch, _offScreenPitch;
	UInt32 w, h;
} CopyRectangleType;

typedef struct {
	const byte *scaletable;
	byte mask, shr;
	byte repcolor;
	byte replen;
	int scaleXstep;
	int x, y;
	int scaleXindex, scaleYindex;
	int skip_width;
	byte *destptr;
	const byte *mask_ptr;
	int imgbufoffs;
} V1CodecType;

typedef struct {
	const byte *scaletable;
	int32 mask, shr;
	int32 repcolor;
	int32 replen;
	int32 scaleXstep;
	int32 x, y;
	int32 scaleXindex, scaleYindex;
	int skip_width;
	byte *destptr;
	const byte *mask_ptr;
} V1Type;

typedef struct {
	void *v1; // struct v1 *
	const byte *revBitMask;	
	
	const byte *_srcptr;
	int _height;

	byte _scaleIndexX;		// to be redefined, return it from the function
	byte _scaleIndexY;
	byte _scaleX;
	byte _scaleY;
	int32 _numStrips;

	int _out_pitch;
	int _out_w;
	int _out_h;

	byte *_shadow_table;
	byte *_palette;
	byte _shadow_mode;
} CostumeProc3Type;

typedef struct {
	int width;
	int height;
	
	const byte *src;
	byte *dst;
	const byte *text;
	
	int _vm_screenWidth;
	uint16 vs_pitch;
	uint16 _textSurface_pitch;
} DrawStripType;

// Queen
typedef struct {
	uint8 *dstBuf;
	const uint8 *srcBuf;
	uint16 dstPitch;
	uint16 srcPitch;
	uint16 w;
	uint16 h;
	byte xflip;
	byte masked;
} BlitType;

// calls
MemPtr	_PceInit(DmResID resID);
UInt32	_PceCall(void *armP, void *userDataP);
void	_PceFree(void *armP);

MemPtr	_PnoInit(DmResID resID, PnoDescriptor *pnoP);
UInt32	_PnoCall(PnoDescriptor *pnoP, void *userDataP);
void	_PnoFree(PnoDescriptor *pnoP, MemPtr armP);

#endif