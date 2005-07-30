#ifndef _ARMNATIVE_H_
#define _ARMNATIVE_H_

#include "arm/pnodefs.h"

#ifndef __PALM_OS__

typedef UInt8 byte;
typedef UInt8 uint8;
typedef Int32 int32;
typedef UInt32 uint32;
typedef Int16 int16;
typedef UInt16 uint16;
typedef unsigned int uint;

#endif

typedef struct {
	UInt32 funcID;
	void *dataP;
} PnoType;

typedef UInt32 (PnoProc)(void *);

enum {
	COMMON_COPYRECT = 0,
	COMMON_WPORTRAIT,
	COMMON_WLANDSCAPE,
//	COMMON_SNDBUFFER
};

enum {
	QUEEN_BLIT = 0
};

enum {
	SCUMM_DRAWSTRIP = 0,
	SCUMM_PROC3
};

enum {
	SWORD1_SCREENDRAW = 0,
	SWORD1_DRAWSPRITE,
	SWORD1_FASTSHRINK,
	SWORD1_RENDERPARALLAX,
	SWORD1_DECOMPTONY,
	SWORD1_DECOMPRLE7,
	SWORD1_DECOMPRLE0
};

// types
typedef struct {
	void *srcP;
	void *dstP;
	UInt32 length;
} ARMPa1SndType, *ARMPa1SndPtr;

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

// Sword1
typedef struct {
	uint8 *data;
	uint32 *lineIndexes;
	uint8 *_screenBuf;
	uint16 _scrnSizeX;
	uint16 scrnScrlX;
	uint16 scrnScrlY;
	uint16 paraScrlX;
	uint16 paraScrlY;
	uint16 scrnWidth;
	uint16 scrnHeight;
} ParallaxType;

typedef struct {
	uint8 *sprData;
	uint8 *dest;
	uint16 sprHeight;
	uint16 sprWidth;
	uint16 sprPitch;
	uint16 _scrnSizeX;
} DrawSpriteType;

typedef struct {
	uint8 *src;
	uint8 *dest;
	uint16 _scrnSizeX;
	uint16 _scrnSizeY;
} DrawType;

typedef struct {
	uint8 *src;
	uint8 *dest;
	uint32 width;
	uint32 height;
	uint32 scale;
} FastShrinkType;

typedef struct {
	uint8 *src;
	uint32 compSize;
	uint8 *dest;
} CompressType;

typedef struct {
	int32 samples;
	int32 len;
	int16 *buffer;
	const byte *_ptr;
	int32 is16Bit;
	int32 isUnsigned;
	int32 isLE;
} ReadBufferType;

// Warning : all the struct MUST be 4byte align and even
// from one member to another

#endif
