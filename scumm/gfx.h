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

namespace Scumm {

class ScummEngine;

/** Camera modes */
enum {
	kNormalCameraMode = 1,
	kFollowActorCameraMode = 2,
	kPanningCameraMode = 3
};

/** Camera state data */
struct CameraData {
	Common::Point _cur;
	Common::Point _dest;
	Common::Point _accel;
	Common::Point _last;
	int _leftTrigger, _rightTrigger;
	byte _follows, _mode;
	bool _movingToActor;
};

/** Virtual screen identifiers */
enum VirtScreenNumber {
	kMainVirtScreen = 0,	// The 'stage'
	kTextVirtScreen = 1,	// In V1-V3 games: the area where text is printed
	kVerbVirtScreen = 2,	// The verb area
	kUnkVirtScreen = 3		// ?? Not sure what this one is good for...
};

/** Virtual screen areas */
struct VirtScreen {
	VirtScreenNumber number;
	uint16 topline;
	uint16 width, height;
	byte alloctwobuffers;
	bool scrollable;
	uint16 xstart;
	uint16 tdirty[80];
	uint16 bdirty[80];
	byte *screenPtr;
	byte *backBuf;

	void setDirtyRange(int top, int bottom) {
		for (int i = 0; i < 80; i++) {
			tdirty[i] = top;
			bdirty[i] = bottom;
		}
	}
};

/** Palette cycles */
struct ColorCycle {
	uint16 delay;
	uint16 counter;
	uint16 flags;
	byte start;
	byte end;
};

/** BlastObjects to draw */
struct BlastObject {
	uint16 number;
	int16 posX, posY;
	uint16 width, height;
	uint16 scaleX, scaleY;
	uint16 image;
	uint16 mode;
};

/** Bomp graphics data, used as parameter to ScummEngine::drawBomp. */
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

struct StripTable;

class Gdi {
	friend class ScummEngine;	// Mostly for the code in saveload.cpp ...
	ScummEngine *_vm;

public:
	int _numZBuffer;
	int _imgBufOffs[8];
	int32 _numStrips;
	Common::Rect _mask;
	byte _C64Colors[4];
	
	Gdi(ScummEngine *vm);

protected:
	byte *_roomPalette;
	byte _decomp_shr, _decomp_mask;
	byte _transparentColor;
	uint32 _vertStripNextInc;

	bool _zbufferDisabled;

	byte _C64CharMap[2048], _C64ObjectMap[2048], _C64PicMap[4096], _C64ColorMap[4096];
	byte _C64MaskMap[4096], _C64MaskChar[4096];
	bool _C64ObjectMode;

	/* Bitmap decompressors */
	bool decompressBitmap(byte *bgbak_ptr, const byte *src, int numLinesToProcess);
	void decodeStripEGA(byte *dst, const byte *src, int height);
	void decodeC64Gfx(const byte *src, byte *dst, int size);
	void drawStripC64Object(byte *dst, int stripnr, int width, int height);
	void drawStripC64Background(byte *dst, int stripnr, int height);
	void drawStripC64Mask(byte *dst, int stripnr, int width, int height);
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
	
	byte *getMaskBuffer(int x, int y, int z = 0);

public:
	void drawBitmap(const byte *ptr, VirtScreen *vs, int x, int y, const int width, const int height,
	                int stripnr, int numstrip, byte flag, StripTable *table = 0);
	StripTable *generateStripTable(const byte *src, int width, int height, StripTable *table);
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


// If you want to try buggy hacked smooth scrolling support in The Dig, enable
// the following preprocessor flag by uncommenting it.
//
// Note: This is purely experimental, NOT WORKING COMPLETLY and very buggy.
// Please do not make reports about problems with it - this is only in CVS
// to get it fixed and so that really interested parties can experiment it.
// It is NOT FIT FOR GENERAL USAGE! You have been warned.
//
// Doing this correctly will be complicated. Basically, with smooth scrolling,
// the virtual screen strips don't match the display screen strips. Hence we
// either have to draw partial strips (but that'd be rather cumbersome). Or the
// alternative (and IMHO more elegant) solution is to simply use a screen pitch
// that is 8 pixel wider than the real screen width, and always draw one strip
// more than needed to the backbuf. This will still require quite some code to
// be changed but should otherwise be relatively easy to understand, and using
// VirtScreen::pitch will actually clean up the code.
//
// #define V7_SMOOTH_SCROLLING_HACK


} // End of namespace Scumm

#endif
