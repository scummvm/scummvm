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

/**
 * In all Scumm games, one to four virtual screen (or 'windows') together make
 * up the content of the actual screen. Thinking of virtual screens as fixed
 * size, fixed location windows might help understanding them. Typical, in all
 * scumm games there is either one single virtual screen covering the entire
 * real screen (mostly in all newer games, e.g. Sam & Max, and all V7+ games).
 * The classic setup consists of three virtual screens: one at the top of the
 * screen, where all conversation texts are printed; then the main one (which
 * I like calling 'the stage', since all the actors are doing their stuff
 * there), and finally the lower part of the real screen is taken up by the
 * verb area.
 * Finally, in V5 games and some V6 games, it's almost the same as in the
 * original games, except that there is no separate conversation area.
 *
 * If you now wonder what the last screen is/was good for: I am not 100% sure,
 * but it appears that it was used by the original engine to display stuff
 * like the pause message, or questions ("Do you really want to restart?").
 * It seems that it is not used at all by ScummVM, so we probably could just
 * get rid of it and save a couple kilobytes of RAM.
 *
 * Each of these virtual screens has a fixed number or id (see also the
 * VirtScreenNumber enum).
 */
struct VirtScreen {
	/**
	 * The unique id of this screen (correponds to its position in the
	 * ScummEngine:virtscr array).
	 */
	VirtScreenNumber number;
	
	/**
	 * Vertical position of the virtual screen. Tells how much the virtual
	 * screen is shifted along the y axis relative to the real screen.
	 * If you wonder why there is no horizontal position: there is none,
	 * because all virtual screens are always exactly as wide as the
	 * real screen. This might change in the future to allow smooth
	 * horizontal scrolling in V7-V8 games.
	 */
	uint16 topline;
	
	/** Width of the virtual screen (currently always identical to _screenWidth). */
	uint16 width;

	/** Height of the virtual screen. */
	uint16 height;

	/**
	 * Flag indicating that this virtual screen allows (horizontal) scrolling.
	 * This is always only true for the main screen (stage)!  After all, verbs
	 * and the conversation text box don't have to scroll.
	 * @todo Get rid of this, there is only one place where it is used,
	 *       and there it is trivial to remove the usage.
	 */
	bool scrollable;
	
	/**
	 * Horizontal scroll offset, tells how far the screen is scrolled to the
	 * right. Only used for the main screen.
	 */
	uint16 xstart;

	/**
	 * Flag indicating  which tells whether this screen has a back buffer or
	 * not. This is yet another feature which is only used by the main screen.
	 * Strictly spoken one could remove this variable and replace checks
	 * on it with checks on backBuf. But since some code needs to temporarily
	 * disable the backBuf (so it can abuse drawBitmap; see drawVerbBitmap()
	 * and useIm01Cursor()), we keep it (at least for now).
	 */
	bool hasTwoBuffers;
	
	/**
	 * Pointer to the screen's data buffer. This is where the content of
	 * the screen is stored. Just as one would expect :-).
	 */
	byte *screenPtr;
	
	/**
	 * Pointer to the screen's back buffer, if it has one (see also
	 * the hasTwoBuffers member).
	 * The backBuf is used by drawBitmap to store the background graphics of
	 * the active room. This eases redrawing: whenever a portion of the screen
	 * has to be redrawn, first a copy from the backBuf content to screenPtr is
	 * performed. Then, any objects/actors in that area are redrawn atop that.
	 */
	byte *backBuf;

	/**
	 * Array containing for each visible strip of this virtual screen the
	 * coordinate at which the dirty region of that strip starts.
	 * 't' stands for 'top' - the top coordinate of the dirty region.
	 * This together with bdirty is used to do efficient redrawing of
	 * the screen.
	 */
	uint16 tdirty[80];

	/**
	 * Array containing for each visible strip of this virtual screen the
	 * coordinate at which the dirty region of that strip end.
	 * 'b' stands for 'bottom' - the bottom coordinate of the dirty region.
	 * This together with tdirty is used to do efficient redrawing of
	 * the screen.
	 */
	uint16 bdirty[80];

	/**
	 * Convenience method to set the whole tdirty and bdirty arrays to one
	 * specific value each. This is mostly used to mark every as dirty in
	 * a single step, like so:
	 *   vs->setDirtyRange(0, vs->height);
	 * or to mark everything as clean, like so:
	 *   vs->setDirtyRange(0, 0);
	 */
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
