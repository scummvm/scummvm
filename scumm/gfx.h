/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project 
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

#include "graphics/surface.h"

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
 * Each of these virtual screens has a fixed number or id (see also
 * \ref VirtScreenNumber).
 */
struct VirtScreen : Graphics::Surface {
	/**
	 * The unique id of this screen (corresponds to its position in the
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
	
	/**
	 * Horizontal scroll offset, tells how far the screen is scrolled to the
	 * right. Only used for the main screen. After all, verbs and the
	 * conversation text box don't have to scroll.
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
	uint16 tdirty[80 + 1];

	/**
	 * Array containing for each visible strip of this virtual screen the
	 * coordinate at which the dirty region of that strip end.
	 * 'b' stands for 'bottom' - the bottom coordinate of the dirty region.
	 * This together with tdirty is used to do efficient redrawing of
	 * the screen.
	 */
	uint16 bdirty[80 + 1];

	/**
	 * Convenience method to set the whole tdirty and bdirty arrays to one
	 * specific value each. This is mostly used to mark every as dirty in
	 * a single step, like so:
	 *   vs->setDirtyRange(0, vs->height);
	 * or to mark everything as clean, like so:
	 *   vs->setDirtyRange(0, 0);
	 */
	void setDirtyRange(int top, int bottom) {
		for (int i = 0; i < 80 + 1; i++) {
			tdirty[i] = top;
			bdirty[i] = bottom;
		}
	}
	
	byte *getPixels(int x, int y) const {
		return (byte *)pixels + xstart + y * pitch + x;
	}

	byte *getBackPixels(int x, int y) const {
		return (byte *)backBuf + xstart + y * pitch + x;
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

/** Bomp graphics data, used as parameter to ScummEngine::drawBomp. */
struct BompDrawData {
	Graphics::Surface dst;

	int x, y;
	byte scale_x, scale_y;
	const byte *dataptr;
	int srcwidth, srcheight;
	uint16 shadowMode;

	byte *maskPtr;
	
	BompDrawData() { memset(this, 0, sizeof(*this)); }
};

struct StripTable;

#define CHARSET_MASK_TRANSPARENCY	253

class Gdi {
	ScummEngine *_vm;

public:
	int _numZBuffer;
	int _imgBufOffs[8];
	int32 _numStrips;
	
	Gdi(ScummEngine *vm);
	~Gdi();

protected:
	byte *_roomPalette;
	byte _transparentColor;
	byte _decomp_shr, _decomp_mask;
	uint32 _vertStripNextInc;

	bool _zbufferDisabled;

	/** Flag which is true when an object is being rendered, false otherwise. */
	bool _objectMode;

	/** Render settings which are specific to the C64 graphic decoders. */
	struct {
		byte colors[4];
		byte charMap[2048], objectMap[2048], picMap[4096], colorMap[4096];
		byte maskMap[4096], maskChar[4096];
	} _C64;

	struct {
		byte nametable[16][64], nametableObj[16][64];
		byte attributes[64], attributesObj[64];
		byte masktable[16][8], masktableObj[16][8];
		int  objX;
		bool hasmask;
	} _NES;

	/** For V2 games, we cache offsets into the room graphics, to speed up things. */
	StripTable *_roomStrips;

	/* Bitmap decompressors */
	bool decompressBitmap(byte *dst, int dstPitch, const byte *src, int numLinesToProcess);

	void drawStripEGA(byte *dst, int dstPitch, const byte *src, int height) const;
	void drawStripC64Object(byte *dst, int dstPitch, int stripnr, int width, int height);
	void drawStripC64Background(byte *dst, int dstPitch, int stripnr, int height);
	void drawStripNES(byte *dst, byte *mask, int dstPitch, int stripnr, int top, int height);

	void drawStripComplex(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const;
	void drawStripBasicH(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const;
	void drawStripBasicV(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const;

	void unkDecode7(byte *dst, int dstPitch, const byte *src, int height) const;
	void unkDecode8(byte *dst, int dstPitch, const byte *src, int height) const;
	void unkDecode9(byte *dst, int dstPitch, const byte *src, int height) const;
	void unkDecode10(byte *dst, int dstPitch, const byte *src, int height) const;
	void unkDecode11(byte *dst, int dstPitch, const byte *src, int height) const;
	void drawStrip3DO(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const;

	void drawStripHE(byte *dst, int dstPitch, const byte *src, int width, int height, const bool transpCheck) const;

	/* Mask decompressors */
	void drawStripC64Mask(byte *dst, int stripnr, int width, int height) const;
	void drawStripNESMask(byte *dst, int stripnr, int top, int height) const;
	void decompressTMSK(byte *dst, const byte *tmsk, const byte *src, int height) const;
	void decompressMaskImgOr(byte *dst, const byte *src, int height) const;
	void decompressMaskImg(byte *dst, const byte *src, int height) const;

	/* Misc */
	void decodeC64Gfx(const byte *src, byte *dst, int size) const;

	int getZPlanes(const byte *smap_ptr, const byte *zplane_list[9], bool bmapImage) const;

	StripTable *generateStripTable(const byte *src, int width, int height, StripTable *table) const;
	void drawBitmapV2Helper(const byte *ptr, VirtScreen *vs, int x, int y, const int width, const int height, 
	                int stripnr, int numstrip);

public:
	void init();
	void roomChanged(byte *roomptr, uint32 IM00_offs, byte transparentColor);

	void drawBitmap(const byte *ptr, VirtScreen *vs, int x, int y, const int width, const int height,
	                int stripnr, int numstrip, byte flag);

	void decodeNESGfx(const byte *room);
	void decodeNESObject(const byte *ptr, int xpos, int ypos, int width, int height);

	void drawBMAPBg(const byte *ptr, VirtScreen *vs);
	void drawBMAPObject(const byte *ptr, VirtScreen *vs, int obj, int x, int y, int w, int h);
	
	void copyVirtScreenBuffers(Common::Rect rect, int dirtybit = 0);

	byte *getMaskBuffer(int x, int y, int z);
	void disableZBuffer() { _zbufferDisabled = true; }
	void enableZBuffer() { _zbufferDisabled = false; }

	void resetBackground(int top, int bottom, int strip);

	enum DrawBitmapFlags {
		dbAllowMaskOr   = 1 << 0,
		dbDrawMaskOnAll = 1 << 1,
		dbObjectMode    = 2 << 2
	};
};


} // End of namespace Scumm

#endif
