/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project 
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

#include "common/rect.h"

enum {					/** Camera modes */
	CM_NORMAL = 1,
	CM_FOLLOW_ACTOR = 2,
	CM_PANNING = 3
};

struct CameraData {		/** Camera state data */
	ScummVM::Point _cur;
	ScummVM::Point _dest;
	ScummVM::Point _accel;
	ScummVM::Point _last;
	int _leftTrigger, _rightTrigger;
	byte _follows, _mode;
	bool _movingToActor;
};

struct VirtScreen {		/** Virtual screen areas */
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

struct ColorCycle {		/** Palette cycles */
	uint16 delay;
	uint16 counter;
	uint16 flags;
	byte start;
	byte end;
};

struct BlastObject {		/** BlastObjects to draw */
	uint16 number;
	int16 posX, posY;
	uint16 width, height;
	uint16 scaleX, scaleY;
	uint16 image;
	uint16 mode;
};

/** Bomp graphics data, used as parameter to Scumm::drawBomp. */
struct BompDrawData {
	byte *out;
	int outwidth, outheight;
	int x, y;
	byte scale_x, scale_y;
	const byte *dataptr;
	int srcwidth, srcheight;
	uint16 shadowMode;

	int32 scaleRight, scaleBottom;
	byte *scalingXPtr, *scalingYPtr;
	byte *maskPtr;
	
	BompDrawData() { memset(this, 0, sizeof(*this)); }
};

class Gdi {
	friend class Scumm;	// Mostly for the code in saveload.cpp ...
public:
	Scumm *_vm;

	int _numZBuffer;
	int _imgBufOffs[8];
	int32 _numStrips;
	ScummVM::Rect _mask;

protected:
	byte _palette_mod;
	byte _decomp_shr, _decomp_mask;
	byte _transparentColor;
	uint32 _vertStripNextInc;

	bool _zbufferDisabled;

	/* Bitmap decompressors */
	bool decompressBitmap(byte *bgbak_ptr, const byte *src, int numLinesToProcess);
	void decodeStripEGA(byte *dst, const byte *src, int height);
	void unkDecodeA(byte *dst, const byte *src, int height);
	void unkDecodeA_trans(byte *dst, const byte *src, int height);
	void unkDecodeB(byte *dst, const byte *src, int height);
	void unkDecodeB_trans(byte *dst, const byte *src, int height);
	void unkDecodeC(byte *dst, const byte *src, int height);
	void unkDecodeC_trans(byte *dst, const byte *src, int height);

	void unkDecode7(byte *dst, const byte *src, int height);
	void unkDecode8(byte *dst, const byte *src, int height);
	void unkDecode9(byte *dst, const byte *src, int height);
	void unkDecode10(byte *dst, const byte *src, int height);
	void unkDecode11(byte *dst, const byte *src, int height);

	void draw8ColWithMasking(byte *dst, const byte *src, int height, byte *mask);
	void draw8Col(byte *dst, const byte *src, int height);
	void clear8ColWithMasking(byte *dst, int height, byte *mask);
	void clear8Col(byte *dst, int height);
	void decompressMaskImgOr(byte *dst, const byte *src, int height);
	void decompressMaskImg(byte *dst, const byte *src, int height);

	void drawStripToScreen(VirtScreen *vs, int x, int w, int t, int b);
	void updateDirtyScreen(VirtScreen *vs);

public:
	void drawBitmap(const byte *ptr, VirtScreen *vs, int x, int y, const int width, const int height,
	                int stripnr, int numstrip, byte flag);
	void clearCharsetMask();

	void disableZBuffer() { _zbufferDisabled = true; }
	void enableZBuffer() { _zbufferDisabled = false; }

	void resetBackground(int top, int bottom, int strip);

	enum DrawBitmapFlags {
		dbAllowMaskOr = 1,
		dbDrawMaskOnAll = 2,
		dbClear = 4
	};
};


// If you wan to try buggy hacked smooth scrolling support in The Dig, enable
// the following preprocessor flag by uncommenting it.
//
// Note: This is purely experimental, NOT WORKING COMPLETLY and very buggy.
// Please do not make reports about problems with it - this is only in CVS
// to get it fixed and so that really interested parties can experiment it.
// It is NOT FIT FOR GENERAL USAGE! You have been warned.
//
// Doing this correctly will be quite some more complicated. Basically, with smooth
// scrolling, the virtual screen strips don't match the display screen strips.
// Hence we either have to draw partial strips - but that'd be rather cumbersome.
// Or the much simple (and IMHO more elegant) solution is to simply use a screen pitch
// that is 8 pixel wider than the real screen width, and always draw one strip more than 
// needed to the backbuf. This will still require quite some code to be changed but
// should otherwise be relatively easy to understand, and using VirtScreen::pitch
// will actually clean up the code.
//
// #define V7_SMOOTH_SCROLLING_HACK


#endif
