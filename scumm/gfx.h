/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef GFX_H
#define GFX_H

enum {					/* Camera modes */
	CM_NORMAL = 1,
	CM_FOLLOW_ACTOR = 2,
	CM_PANNING = 3
};

struct CameraData {		/* Camera state data */
	ScummPoint _cur;
	ScummPoint _dest;
	ScummPoint _accel;
	ScummPoint _last;
	int _leftTrigger, _rightTrigger;
	byte _follows, _mode;
	bool _movingToActor;
};

struct VirtScreen {		/* Virtual screen areas */
	int number;
	uint16 topline;
	uint16 width, height;
	int size;
	byte alloctwobuffers;
	bool scrollable;
	uint16 xstart;
	uint16 tdirty[80];
	uint16 bdirty[80];
	byte *screenPtr;
	byte *backBuf;
};

struct MouseCursor {	/* Mouse cursor */
	int8 hotspot_x, hotspot_y;
	byte colors[4];
	byte data[32];
};

struct ColorCycle {		/* Palette cycles */
	uint16 delay;
	uint16 counter;
	uint16 flags;
	byte start;
	byte end;
};

struct BlastObject {		/* BlastObjects to draw */
	uint16 number;
	int16 posX, posY;
	uint16 width, height;
	uint16 scaleX, scaleY;
	uint16 image;
	uint16 mode;
};

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct BompHeader {			/* Bomp header */
	union {
		struct {
			uint16 unk;
			uint16 width, height;
		} GCC_PACK old;

		struct {
			uint32 width, height;
		} GCC_PACK v8;
	} GCC_PACK;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

struct BompDrawData {		/* Bomp graphics data */
	byte *out;
	int outwidth, outheight;
	int x, y;
	byte scale_x, scale_y;
	byte *dataptr;
	int srcwidth, srcheight;
	uint16 shadowMode;
};

class Gdi {
	friend class Scumm;	// Mostly for the code in saveload.cpp ...
public:
	Scumm *_vm;

	int _numZBuffer;
	int _imgBufOffs[8];
	byte _disable_zbuffer;
	int32 _numStrips;
	int16 _mask_top, _mask_bottom, _mask_right, _mask_left;

protected:
	byte *_readPtr;

	bool _useOrDecompress;
	byte _cursorActive;

	byte *_mask_ptr;
	byte *_mask_ptr_dest;

	byte _palette_mod;
	byte _decomp_shr, _decomp_mask;
	byte _transparentColor;
	uint32 _vertStripNextInc;

	/* Bitmap decompressors */
	bool decompressBitmap(byte *bgbak_ptr, byte *smap_ptr, int numLinesToProcess);
	void unkDecodeA(byte *dst, byte *src, int height);
	void unkDecodeA_trans(byte *dst, byte *src, int height);
	void unkDecodeB(byte *dst, byte *src, int height);
	void unkDecodeB_trans(byte *dst, byte *src, int height);
	void unkDecodeC(byte *dst, byte *src, int height);
	void unkDecodeC_trans(byte *dst, byte *src, int height);

	void unkDecode7(byte *dst, byte *src, int height);
	void unkDecode8(byte *dst, byte *src, int height);
	void unkDecode9(byte *dst, byte *src, int height);
	void unkDecode10(byte *dst, byte *src, int height);
	void unkDecode11(byte *dst, byte *src, int height);

	void draw8ColWithMasking(byte *dst, byte *src, int height, byte *mask);
	void draw8Col(byte *dst, byte *src, int height);
	void clear8ColWithMasking(byte *dst, int height, byte *mask);
	void clear8Col(byte *dst, int height);
	void decompressMaskImgOr(byte *dst, byte *src, int height);
	void decompressMaskImg(byte *dst, byte *src, int height);

	void drawStripToScreen(VirtScreen *vs, int x, int w, int t, int b);
	void updateDirtyScreen(VirtScreen *vs);

public:
	void drawBitmap(byte *ptr, VirtScreen *vs, int x, int y, const int h, int stripnr, int numstrip, byte flag);
	void clearUpperMask();

	void disableZBuffer() { _disable_zbuffer++; }
	void enableZBuffer() { _disable_zbuffer--; }

	void resetBackground(int top, int bottom, int strip);

	enum DrawBitmapFlags {
		dbAllowMaskOr = 1,
		dbDrawMaskOnAll = 2,
		dbClear = 4
	};
};

#endif
