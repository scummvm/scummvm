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

#include "stdafx.h"
#include "common/system.h"
#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/resource.h"
#include "scumm/usage_bits.h"
#include "scumm/wiz_he.h"

#if defined(__PALM_OS__)
#include "arm/native.h"
#include "arm/macros.h"
#endif

namespace Scumm {

static void blit(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h);
static void fill(byte *dst, int dstPitch, byte color, int w, int h);

static void copy8Col(byte *dst, int dstPitch, const byte *src, int height);
static void clear8Col(byte *dst, int dstPitch, int height);


struct StripTable {
	int offsets[160];
	int run[160];
	int color[160];
	int zoffsets[120];	// FIXME: Why only 120 here?
	int zrun[120];		// FIXME: Why only 120 here?
};

enum {
	kScrolltime = 500,  // ms scrolling is supposed to take
	kPictureDelay = 20
};

#define NUM_SHAKE_POSITIONS 8
static const int8 shake_positions[NUM_SHAKE_POSITIONS] = {
	0, 1 * 2, 2 * 2, 1 * 2, 0 * 2, 2 * 2, 3 * 2, 1 * 2
};

/**
 * The following structs define four basic fades/transitions used by
 * transitionEffect(), each looking differently to the user.
 * Note that the stripTables contain strip numbers, and they assume
 * that the screen has 40 vertical strips (i.e. 320 pixel), and 25 horizontal
 * strips (i.e. 200 pixel). There is a hack in transitionEffect that
 * makes it work correctly in games which have a different screen height
 * (for example, 240 pixel), but nothing is done regarding the width, so this
 * code won't work correctly in COMI. Also, the number of iteration depends
 * on min(vertStrips, horizStrips}. So the 13 is derived from 25/2, rounded up.
 * And the 25 = min(25,40). Hence for Zak256 instead of 13 and 25, the values
 * 15 and 30 should be used, and for COMI probably 30 and 60.
 */
struct TransitionEffect {
	byte numOfIterations;
	int8 deltaTable[16];	// four times l / t / r / b
	byte stripTable[16];	// ditto
};

#ifdef __PALM_OS__
static const TransitionEffect *transitionEffects;
#else
static const TransitionEffect transitionEffects[6] = {
	// Iris effect (looks like an opening/closing camera iris)
	{
		13,		// Number of iterations
		{
			1,  1, -1,  1,
		   -1,  1, -1, -1,
			1, -1, -1, -1,
			1,  1,  1, -1
		},
		{
			0,  0, 39,  0,
		   39,  0, 39, 24,
			0, 24, 39, 24,
			0,  0,  0, 24
		}
	},
	
	// Box wipe (a box expands from the upper-left corner to the lower-right corner)
	{
		25,		// Number of iterations
		{
			0,  1,  2,  1,
			2,  0,  2,  1,
			2,  0,  2,  1,
			0,  0,  0,  0
		},
		{
			0,  0,  0,  0,
			0,  0,  0,  0,
			1,  0,  1,  0,
		  255,  0,  0,  0
		}
	},
	
	// Box wipe (a box expands from the lower-right corner to the upper-left corner)
	{
		25,		// Number of iterations
		{
		   -2, -1,  0, -1,
		   -2, -1, -2,  0,
		   -2, -1, -2,  0,
			0,  0,  0,  0
		},
		{
		   39, 24, 39, 24,
		   39, 24, 39, 24,
		   38, 24, 38, 24,
		  255,  0,  0,  0
		}
	},
	
	// Inverse box wipe
	{
		25,		// Number of iterations
		{
			0, -1, -2, -1,
		   -2,  0, -2, -1,
		   -2,  0, -2, -1,
		    0,  0,  0,  0
		},
		{
			0, 24, 39, 24,
		   39,  0, 39, 24,
		   38,  0, 38, 24,
		  255,  0,  0,  0
		}
	},

	// Inverse iris effect, specially tailored for V1/V2 games
	{
		9,		// Number of iterations
		{
			-1, -1,  1, -1,
			-1,  1,  1,  1,
			-1, -1, -1,  1,
			 1, -1,  1,  1
		},
		{
			 7, 7, 32, 7,
			 7, 8, 32, 8,
			 7, 8,  7, 8,
			32, 7, 32, 8
		}
	},

	// Horizontal wipe (a box expands from left to right side). For MM NES
	{
		16,		// Number of iterations
		{
			  2,  0,  2,  0,
			  2,  0,  2,  0,
			  0,  0,  0,  0,
			  0,  0,  0,  0
		},
		{
			  0, 0,  0,  15,
			  1, 0,  1,  15,
			255, 0,  0,  0,
			255, 0,  0,  0
		}
	}
	
};
#endif


Gdi::Gdi(ScummEngine *vm) {
	memset(this, 0, sizeof(*this));
	_vm = vm;
	_roomPalette = vm->_roomPalette;
	_roomStrips = 0;
	if ((vm->_platform == Common::kPlatformAmiga) && (vm->_version >= 4))
		_roomPalette += 16;
}

Gdi::~Gdi() {
	free(_roomStrips);
}

void Gdi::init() {
	_numStrips = _vm->_screenWidth / 8;

	// Increase the number of screen strips by one; needed for smooth scrolling
	if (_vm->_version >= 7) {
		// We now have mostly working smooth scrolling code in place for V7+ games
		// (i.e. The Dig, Full Throttle and COMI). It seems to work very well so far.
		// One area which still may need some work are the AKOS codecs (except for
		// codec 1, which I already updated): their masking code may need adjustments,
		// similar to the treatment codec 1 received.
		//
		// To understand how we achieve smooth scrolling, first note that with it, the
		// virtual screen strips don't match the display screen strips anymore. To
		// overcome that problem, we simply use a screen pitch that is 8 pixel wider
		// than the actual screen width, and always draw one strip more than needed to
		// the backbuf (of course we have to treat the right border seperately). This
		_numStrips += 1;
	}
}

void Gdi::roomChanged(byte *roomptr, uint32 IM00_offs, byte transparentColor) {
	if (_vm->_version == 1) {
		if (_vm->_platform == Common::kPlatformNES) {
			decodeNESGfx(roomptr);
		} else {
			for (int i = 0; i < 4; i++){
				_C64.colors[i] = roomptr[6 + i];
			}
			decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 10), _C64.charMap, 2048);
			decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 12), _C64.picMap, roomptr[4] * roomptr[5]);
			decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 14), _C64.colorMap, roomptr[4] * roomptr[5]);
			decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 16), _C64.maskMap, roomptr[4] * roomptr[5]);
			decodeC64Gfx(roomptr + READ_LE_UINT16(roomptr + 18) + 2, _C64.maskChar, READ_LE_UINT16(roomptr + READ_LE_UINT16(roomptr + 18)));
			_objectMode = true;
		}
	} else if (_vm->_version == 2) {
		_roomStrips = generateStripTable(roomptr + IM00_offs, _vm->_roomWidth, _vm->_roomHeight, _roomStrips);
	}
	
	_transparentColor = transparentColor;
}


#pragma mark -
#pragma mark --- Virtual Screens ---
#pragma mark -


void ScummEngine::initScreens(int b, int h) {
	int i;
	int adj = 0;

	for (i = 0; i < 3; i++) {
		res.nukeResource(rtBuffer, i + 1);
		res.nukeResource(rtBuffer, i + 5);
	}

	if (!getResourceAddress(rtBuffer, 4)) {
		// Since the size of screen 3 is fixed, there is no need to reallocate
		// it if its size changed.
		// Not sure what it is good for, though. I think it may have been used
		// in pre-V7 for the games messages (like 'Pause', Yes/No dialogs,
		// version display, etc.). I don't know about V7, maybe the same is the
		// case there. If so, we could probably just remove it completely.
		if (_version >= 7) {
			initVirtScreen(kUnkVirtScreen, (_screenHeight / 2) - 10, _screenWidth, 13, false, false);
		} else {
			initVirtScreen(kUnkVirtScreen, 80, _screenWidth, 13, false, false);
		}
	}

	if ((_platform == Common::kPlatformNES) && (h != _screenHeight)) {
		// This is a hack to shift the whole screen downwards to match the original.
		// Otherwise we would have to do lots of coordinate adjustments all over
		// the code.
		adj = 16;
		initVirtScreen(kUnkVirtScreen, 0, _screenWidth, adj, false, false);
	}

	initVirtScreen(kMainVirtScreen, b + adj, _screenWidth, h - b, true, true);
	initVirtScreen(kTextVirtScreen, adj, _screenWidth, b, false, false);
	initVirtScreen(kVerbVirtScreen, h + adj, _screenWidth, _screenHeight - h - adj, false, false);
	_screenB = b;
	_screenH = h;
	
	gdi.init();
}

void ScummEngine::initVirtScreen(VirtScreenNumber slot, int top, int width, int height, bool twobufs,
													 bool scrollable) {
	VirtScreen *vs = &virtscr[slot];
	int size;

	assert(height >= 0);
	assert(slot >= 0 && slot < 4);

	if (_version >= 7) {
		if (slot == kMainVirtScreen && (_roomHeight != 0))
			height = _roomHeight;
	}

	vs->number = slot;
	vs->w = width;
	vs->topline = top;
	vs->h = height;
	vs->hasTwoBuffers = twobufs;
	vs->xstart = 0;
	vs->backBuf = NULL;
	vs->bytesPerPixel = 1;
	vs->pitch = width;

	if (_version >= 7) {
		// Increase the pitch by one; needed to accomodate the extra
		// screen strip which we use to implement smooth scrolling.
		// See Gdi::init()
		vs->pitch += 8;
	}

	size = vs->pitch * vs->h;
	if (scrollable) {
		// Allow enough spaces so that rooms can be up to 4 resp. 8 screens
		// wide. To achieve (horizontal!) scrolling, we use a neat trick:
		// only the offset into the screen buffer (xstart) is changed. That way
		// very little of the screen has to be redrawn, and we have a very low
		// memory overhead (namely for every pixel we want to scroll, we need
		// one additional byte in the buffer).
		if (_version >= 7) {
			size += vs->pitch * 8;
		} else {
			size += vs->pitch * 4;
		}
	}

	res.createResource(rtBuffer, slot + 1, size);
	vs->pixels = getResourceAddress(rtBuffer, slot + 1);
	memset(vs->pixels, 0, size);	// reset background

	if (twobufs) {
		vs->backBuf = res.createResource(rtBuffer, slot + 5, size);
	}

	if (slot != 3) {
		vs->setDirtyRange(0, height);
	}
}

VirtScreen *ScummEngine::findVirtScreen(int y) {
	VirtScreen *vs = virtscr;
	int i;

	for (i = 0; i < 3; i++, vs++) {
		if (y >= vs->topline && y < vs->topline + vs->h) {
			return vs;
		}
	}
	return NULL;
}

void ScummEngine::markRectAsDirty(VirtScreenNumber virt, int left, int right, int top, int bottom, int dirtybit) {
	VirtScreen *vs = &virtscr[virt];
	int lp, rp;

	if (left > right || top > bottom)
		return;
	if (top > vs->h || bottom < 0)
		return;

	if (top < 0)
		top = 0;
	if (bottom > vs->h)
		bottom = vs->h;

	if (virt == kMainVirtScreen && dirtybit) {

		lp = left / 8 + _screenStartStrip;
		if (lp < 0)
			lp = 0;

		rp = (right + vs->xstart) / 8;
		if (_version >= 7) {
			if (rp > 409)
				rp = 409;
		} else {
			if (rp >= 200)
				rp = 200;
		}
		for (; lp <= rp; lp++)
			setGfxUsageBit(lp, dirtybit);
	}

	// The following code used to be in the separate method setVirtscreenDirty
	lp = left / 8;
	rp = right / 8;

	if ((lp >= gdi._numStrips) || (rp < 0))
		return;
	if (lp < 0)
		lp = 0;
	if (rp >= gdi._numStrips)
		rp = gdi._numStrips - 1;

	while (lp <= rp) {
		if (top < vs->tdirty[lp])
			vs->tdirty[lp] = top;
		if (bottom > vs->bdirty[lp])
			vs->bdirty[lp] = bottom;
		lp++;
	}
}

/**
 * Update all dirty screen areas. This method blits all of the internal engine
 * graphics to the actual display, as needed. In addition, the 'shaking'
 * code in the backend is controlled from here.
 */
void ScummEngine::drawDirtyScreenParts() {
	// Update verbs
	updateDirtyScreen(kVerbVirtScreen);
	
	// Update the conversation area (at the top of the screen)
	updateDirtyScreen(kTextVirtScreen);

	// Update game area ("stage")
	if (camera._last.x != camera._cur.x || (_features & GF_NEW_CAMERA && (camera._cur.y != camera._last.y))) {
		// Camera moved: redraw everything
		VirtScreen *vs = &virtscr[kMainVirtScreen];
		drawStripToScreen(vs, 0, vs->w, 0, vs->h);
		vs->setDirtyRange(vs->h, 0);
	} else {
		updateDirtyScreen(kMainVirtScreen);
	}

	// Handle shaking
	if (_shakeEnabled) {
		_shakeFrame = (_shakeFrame + 1) % NUM_SHAKE_POSITIONS;
		_system->setShakePos(shake_positions[_shakeFrame]);
	} else if (!_shakeEnabled &&_shakeFrame != 0) {
		_shakeFrame = 0;
		_system->setShakePos(0);
	}
}

void ScummEngine_v6::drawDirtyScreenParts() {
	// For the Full Throttle credits to work properly, the blast
	// texts have to be drawn before the blast objects. Unless
	// someone can think of a better way to achieve this effect.

	if (_version >= 7 && VAR(VAR_BLAST_ABOVE_TEXT) == 1) {
		drawBlastTexts();
		drawBlastObjects();
	} else {
		drawBlastObjects();
		drawBlastTexts();
	}
	if (_version == 8)
		processUpperActors();

	// Call the original method.
	ScummEngine::drawDirtyScreenParts();

	// Remove all blasted objects/text again.
	removeBlastTexts();
	removeBlastObjects();
}

/**
 * Blit the dirty data from the given VirtScreen to the display. If the camera moved,
 * a full blit is done, otherwise only the visible dirty areas are updated.
 */
void ScummEngine::updateDirtyScreen(VirtScreenNumber slot) {
	VirtScreen *vs = &virtscr[slot];

	// Do nothing for unused virtual screens
	if (vs->h == 0)
		return;

	int i;
	int w = 8;
	int start = 0;

	for (i = 0; i < gdi._numStrips; i++) {
		if (vs->bdirty[i]) {
			const int top = vs->tdirty[i];
			const int bottom = vs->bdirty[i];
			vs->tdirty[i] = vs->h;
			vs->bdirty[i] = 0;
			if (i != (gdi._numStrips - 1) && vs->bdirty[i + 1] == bottom && vs->tdirty[i + 1] == top) {
				// Simple optimizations: if two or more neighbouring strips
				// form one bigger rectangle, coalesce them.
				w += 8;
				continue;
			}
			drawStripToScreen(vs, start * 8, w, top, bottom);
			w = 8;
		}
		start = i + 1;
	}
}

/**
 * Blit the specified rectangle from the given virtual screen to the display.
 * Note: t and b are in *virtual screen* coordinates, while x is relative to
 * the *real screen*. This is due to the way tdirty/vdirty work: they are
 * arrays which map 'strips' (sections of the real screen) to dirty areas as
 * specified by top/bottom coordinate in the virtual screen.
 */
void ScummEngine::drawStripToScreen(VirtScreen *vs, int x, int width, int top, int bottom) {

	if (bottom <= top)
		return;

	if (top >= vs->h)
		return;

	assert(top >= 0 && bottom <= vs->h);	// Paranoia checks
	assert(x >= 0 && width <= vs->pitch);
	assert(_charset->_textSurface.pixels);
	assert(_compositeBuf);
	
	if (width > vs->w - x)
		width = vs->w - x;

	// Clip to the visible part of the scene
	if (top < _screenTop)
		top = _screenTop;
	if (bottom > _screenTop + _screenHeight)
		bottom = _screenTop + _screenHeight;

	// Convert the vertical coordinates to real screen coords
	int y = vs->topline + top - _screenTop;
	int height = bottom - top;
	
	// Compute screen etc. buffer pointers
	const byte *src = vs->getPixels(x, top);
	byte *dst = _compositeBuf + x + y * _screenWidth;
	const byte *text = (byte *)_charset->_textSurface.pixels + x + y * _charset->_textSurface.pitch;

#ifdef __PALM_OS__
	ARM_START(DrawStripType)
		ARM_INIT(SCUMM_DRAWSTRIP)
		ARM_ADDM(width)
		ARM_ADDM(height)
		ARM_ADDM(src)
		ARM_ADDM(dst)
		ARM_ADDM(text)
		ARM_ADDV(_vm_screenWidth, _screenWidth)
		ARM_ADDV(vs_pitch, vs->pitch)
		ARM_ADDV(_charset->_textSurface_pitch, _charset->_textSurface.pitch)
		ARM_CALL(ARM_ENGINE, PNO_DATA())
	ARM_CONTINUE()
#endif
	// Compose the text over the game graphics
	for (int h = 0; h < height; ++h) {
		for (int w = 0; w < width; ++w) {
			if (text[w] == CHARSET_MASK_TRANSPARENCY)
				dst[w] = src[w];
			else
				dst[w] = text[w];
		}
		src += vs->pitch;
		dst += _screenWidth;
		text += _charset->_textSurface.pitch;
	}

	if (_renderMode == Common::kRenderCGA)
		ditherCGA(_compositeBuf + x + y * _screenWidth, _screenWidth, x, y, width, height);

	if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG) {
		ditherHerc(_compositeBuf + x + y * _screenWidth, _herculesBuf, _screenWidth, &x, &y, &width, &height);
		// center image on the screen
		_system->copyRectToScreen(_herculesBuf + x + y * Common::kHercW, 
			Common::kHercW, x + (Common::kHercW - _screenWidth * 2) / 2, y, width, height);
	} else {
		// Finally blit the whole thing to the screen
		int x1 = x;

		// HACK: This is dirty hack which renders narrow NES rooms centered
		// NES can address negative number strips and that poses problem for
		// our code. So instead adding zillions of fixes and potentially break
		// other games we shift it right on rendering stage
		if ((_platform == Common::kPlatformNES) && (((_NESStartStrip > 0) && (vs->number == kMainVirtScreen)) || (vs->number == kTextVirtScreen))) {
			x += 16;
		}

		_system->copyRectToScreen(_compositeBuf + x1 + y * _screenWidth, _screenWidth, x, y, width, height);
	}
}

// CGA
// indy3 loom maniac monkey1 zak
//
// Herc (720x350)
// maniac monkey1 zak
//
// EGA
// monkey2 loom maniac monkey1 atlantis indy3 zak loomcd

// CGA dithers 4x4 square with direct substitutes
// Odd lines have colors swapped, so there will be checkered patterns.
// But apparently there is a mistake for 10th color.
void ScummEngine::ditherCGA(byte *dst, int dstPitch, int x, int y, int width, int height) const {
	byte *ptr;
	int idx1, idx2;
	static const byte cgaDither[2][2][16] = {
		{{0, 1, 0, 1, 2, 2, 0, 0, 3, 1, 3, 1, 3, 2, 1, 3},
		 {0, 0, 1, 1, 0, 2, 2, 3, 0, 3, 1, 1, 3, 3, 1, 3}},
		{{0, 0, 1, 1, 0, 2, 2, 3, 0, 3, 1, 1, 3, 3, 1, 3},
		 {0, 1, 0, 1, 2, 2, 0, 0, 3, 1, 1, 1, 3, 2, 1, 3}}};

	for (int y1 = 0; y1 < height; y1++) {
		ptr = dst + y1 * dstPitch;

		idx1 = (y + y1) % 2;

		if (_version == 2)
			idx1 = 0;

		for (int x1 = 0; x1 < width; x1++) {
			idx2 = (x + x1) % 2;
			*ptr++ = cgaDither[idx1][idx2][*ptr & 0xF];
		}
	}
}

// Hercules dithering. It uses same dithering tables but output is 1bpp and
// it stretches in this way:
//         aaaa0
// aa      aaaa1
// bb      bbbb0      Here 0 and 1 mean dithering table row number
// cc -->  bbbb1
// dd      cccc0
//         cccc1
//         dddd0
void ScummEngine::ditherHerc(byte *src, byte *hercbuf, int srcPitch, int *x, int *y, int *width, int *height) const {
	byte *srcptr, *dstptr;
	int xo = *x, yo = *y, widtho = *width, heighto = *height;
	int idx1, idx2, dsty = 0, y1;
	static const byte cgaDither[2][2][16] = {
		{{0, 1, 0, 1, 2, 2, 0, 0, 3, 1, 3, 1, 3, 2, 1, 3},
		 {0, 0, 1, 1, 0, 2, 2, 3, 0, 3, 1, 1, 3, 3, 1, 3}},
		{{0, 0, 1, 1, 0, 2, 2, 3, 0, 3, 1, 1, 3, 3, 1, 3},
		 {0, 1, 0, 1, 2, 2, 0, 0, 3, 1, 1, 1, 3, 2, 1, 3}}};

	// calculate dsty
	for (y1 = 0; y1 < yo; y1++) {
		dsty += 2;
		if (y1 % 4 == 3)
			dsty--;
	}
	*y = dsty;
	*x *= 2;
	*width *= 2;
	*height = 0;

	for (y1 = 0; y1 < heighto;) {
		srcptr = src + y1 * srcPitch;
		dstptr = hercbuf + dsty * Common::kHercW + xo * 2;

		assert(dstptr < hercbuf + Common::kHercW * Common::kHercH + widtho * 2);

		idx1 = (dsty % 7) % 2;
		for (int x1 = 0; x1 < widtho; x1++) {
			idx2 = (xo + x1) % 2;
			*dstptr++ = cgaDither[idx1][idx2][*srcptr & 0xF] >> 1;
			*dstptr++ = cgaDither[idx1][idx2][*srcptr & 0xF] & 0x1;
			srcptr++;
		}
		if (idx1 || dsty % 7 == 6)
			y1++;
		dsty++;
		(*height)++;
	}
}


#pragma mark -
#pragma mark --- Background buffers & charset mask ---
#pragma mark -


void ScummEngine::initBGBuffers(int height) {
	const byte *ptr;
	int size, itemsize, i;
	byte *room;

	if (_version >= 7) {
		// Resize main virtual screen in V7 games. This is necessary
		// because in V7, rooms may be higher than one screen, so we have
		// to accomodate for that.
		initVirtScreen(kMainVirtScreen, virtscr[0].topline, _screenWidth, height, 1, 1);
	}

	if (_heversion >= 70)
		room = getResourceAddress(rtRoomImage, _roomResource);
	else
		room = getResourceAddress(rtRoom, _roomResource);

	if (_version <= 3) {
		gdi._numZBuffer = 2;
	} else if (_features & GF_SMALL_HEADER) {
		int off;
		ptr = findResourceData(MKID('SMAP'), room);
		gdi._numZBuffer = 0;

		if (_features & GF_16COLOR)
			off = READ_LE_UINT16(ptr);
		else
			off = READ_LE_UINT32(ptr);

		while (off && gdi._numZBuffer < 4) {
			gdi._numZBuffer++;
			ptr += off;
			off = READ_LE_UINT16(ptr);
		}
	} else if (_version == 8) {
		// in V8 there is no RMIH and num z buffers is in RMHD
		ptr = findResource(MKID('RMHD'), room);
		gdi._numZBuffer = READ_LE_UINT32(ptr + 24) + 1;
	} else if (_heversion >= 70) {
		ptr = findResource(MKID('RMIH'), room);
		gdi._numZBuffer = READ_LE_UINT16(ptr + 8) + 1;
	} else {
		ptr = findResource(MKID('RMIH'), findResource(MKID('RMIM'), room));
		gdi._numZBuffer = READ_LE_UINT16(ptr + 8) + 1;
	}
	assert(gdi._numZBuffer >= 1 && gdi._numZBuffer <= 8);

	if (_version >= 7)
		itemsize = (_roomHeight + 10) * gdi._numStrips;
	else
		itemsize = (_roomHeight + 4) * gdi._numStrips;


	size = itemsize * gdi._numZBuffer;
	memset(res.createResource(rtBuffer, 9, size), 0, size);

	for (i = 0; i < (int)ARRAYSIZE(gdi._imgBufOffs); i++) {
		if (i < gdi._numZBuffer)
			gdi._imgBufOffs[i] = i * itemsize;
		else
			gdi._imgBufOffs[i] = (gdi._numZBuffer - 1) * itemsize;
	}
}

/**
 * Redraw background as needed, i.e. the left/right sides if scrolling took place etc.
 * Note that this only updated the virtual screen, not the actual display.
 */
void ScummEngine::redrawBGAreas() {
	int i;
	int diff;
	int val = 0;

	if (!(_features & GF_NEW_CAMERA))
		if (camera._cur.x != camera._last.x && _charset->_hasMask && (_version > 3 && _gameId != GID_PASS))
			stopTalk();

	// Redraw parts of the background which are marked as dirty.
	if (!_fullRedraw && _bgNeedsRedraw) {
		for (i = 0; i != gdi._numStrips; i++) {
			if (testGfxUsageBit(_screenStartStrip + i, USAGE_BIT_DIRTY)) {
				redrawBGStrip(i, 1);
			}
		}
	}

	if (_features & GF_NEW_CAMERA) {
		diff = camera._cur.x / 8 - camera._last.x / 8;
		if (_fullRedraw == 0 && diff == 1) {
			val = 2;
			redrawBGStrip(gdi._numStrips - 1, 1);
		} else if (_fullRedraw == 0 && diff == -1) {
			val = 1;
			redrawBGStrip(0, 1);
		} else if (_fullRedraw != 0 || diff != 0) {
			_bgNeedsRedraw = false;
			redrawBGStrip(0, gdi._numStrips);
		}
	} else {
		if (_fullRedraw == 0 && camera._cur.x - camera._last.x == 8) {
			val = 2;
			redrawBGStrip(gdi._numStrips - 1, 1);
		} else if (_fullRedraw == 0 && camera._cur.x - camera._last.x == -8) {
			val = 1;
			redrawBGStrip(0, 1);
		} else if (_fullRedraw != 0 || camera._cur.x != camera._last.x) {
			_bgNeedsRedraw = false;
			_flashlight.isDrawn = false;
			redrawBGStrip(0, gdi._numStrips);
		}
	}

	drawRoomObjects(val);
	_bgNeedsRedraw = false;
}

void ScummEngine_v71he::redrawBGAreas() {
	if (camera._cur.x != camera._last.x && _charset->_hasMask)
		stopTalk();

	byte *room = getResourceAddress(rtRoomImage, _roomResource) + _IM00_offs;
	if (_fullRedraw) {
		_bgNeedsRedraw = false;
		gdi.drawBMAPBg(room, &virtscr[0]);
	}

	drawRoomObjects(0);
	_bgNeedsRedraw = false;
}

void ScummEngine_v72he::redrawBGAreas() {
	ScummEngine_v71he::redrawBGAreas();
	flushWizBuffer();
}

void ScummEngine::redrawBGStrip(int start, int num) {
	byte *room;

	int s = _screenStartStrip + start;

	for (int i = 0; i < num; i++)
		setGfxUsageBit(s + i, USAGE_BIT_DIRTY);

	if (_heversion >= 70)
		room = getResourceAddress(rtRoomImage, _roomResource);
	else
		room = getResourceAddress(rtRoom, _roomResource);

	gdi.drawBitmap(room + _IM00_offs, &virtscr[0], s, 0, _roomWidth, virtscr[0].h, s, num, 0);
}

void ScummEngine::restoreBG(Common::Rect rect, byte backColor) {
	VirtScreen *vs;
	byte *screenBuf;

	if (rect.top < 0)
		rect.top = 0;
	if (rect.left >= rect.right || rect.top >= rect.bottom)
		return;

	if ((vs = findVirtScreen(rect.top)) == NULL)
		return;

	if (rect.left > vs->w)
		return;

	// Convert 'rect' to local (virtual screen) coordinates
	rect.top -= vs->topline;
	rect.bottom -= vs->topline;

	rect.clip(vs->w, vs->h);

	markRectAsDirty(vs->number, rect, USAGE_BIT_RESTORED);

	screenBuf = vs->getPixels(rect.left, rect.top);

	const int height = rect.height();
	const int width = rect.width();
	
	if (!height)
		return;

	if (vs->hasTwoBuffers && _currentRoom != 0 && isLightOn()) {
		blit(screenBuf, vs->pitch, vs->getBackPixels(rect.left, rect.top), vs->pitch, width, height);
		if (vs->number == kMainVirtScreen && _charset->_hasMask) {
			byte *mask = (byte *)_charset->_textSurface.pixels + _charset->_textSurface.pitch * (rect.top - _screenTop) + rect.left;
			fill(mask, _charset->_textSurface.pitch, CHARSET_MASK_TRANSPARENCY, width, height);
		}
	} else {
		fill(screenBuf, vs->pitch, backColor, width, height);
	}
}

void CharsetRenderer::restoreCharsetBg() {
	_nextLeft = _vm->_string[0].xpos;
	_nextTop = _vm->_string[0].ypos + _vm->_screenTop;

	if (_hasMask) {
		_hasMask = false;
		_str.left = -1;
		_left = -1;

		// Restore background on the whole text area. This code is based on
		// restoreBG(), but was changed to only restore those parts which are
		// currently covered by the charset mask.

		VirtScreen *vs = &_vm->virtscr[_textScreenID];
		if (!vs->h)
			return;

		_vm->markRectAsDirty(vs->number, Common::Rect(vs->w, vs->h), USAGE_BIT_RESTORED);
	
		byte *screenBuf = vs->getPixels(0, 0);

		if (vs->hasTwoBuffers && _vm->_currentRoom != 0 && _vm->isLightOn()) {
			if (vs->number != kMainVirtScreen) {
				// Restore from back buffer
				const byte *backBuf = vs->getBackPixels(0, 0);
				blit(screenBuf, vs->pitch, backBuf, vs->pitch, vs->w, vs->h);
			}
		} else {
			// Clear area
			memset(screenBuf, 0, vs->h * vs->pitch);
		}

		if (vs->hasTwoBuffers) {
			// Clean out the charset mask
			clearTextSurface();
		}
	}
}

void CharsetRenderer::clearCharsetMask() {
	memset(_vm->getResourceAddress(rtBuffer, 9), 0, _vm->gdi._imgBufOffs[1]);
}

void CharsetRenderer::clearTextSurface() {
	memset(_textSurface.pixels, CHARSET_MASK_TRANSPARENCY, _textSurface.pitch * _textSurface.h);
}

byte *ScummEngine::getMaskBuffer(int x, int y, int z) {
	return gdi.getMaskBuffer((x + virtscr[0].xstart) / 8, y, z);
}

byte *Gdi::getMaskBuffer(int x, int y, int z) {
	return _vm->getResourceAddress(rtBuffer, 9)
			+ x + y * _numStrips + _imgBufOffs[z];
}


#pragma mark -
#pragma mark --- Misc ---
#pragma mark -

static void blit(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h) {
	assert(w > 0);
	assert(h > 0);
	assert(src != NULL);
	assert(dst != NULL);
	
	if (w == srcPitch && w == dstPitch) {
		memcpy(dst, src, w*h);
	} else {
		do {
			memcpy(dst, src, w);
			dst += dstPitch;
			src += srcPitch;
		} while (--h);
	}
}

static void fill(byte *dst, int dstPitch, byte color, int w, int h) {
	assert(h > 0);
	assert(dst != NULL);
	
	if (w == dstPitch) {
		memset(dst, color, w*h);
	} else {
		do {
			memset(dst, color, w);
			dst += dstPitch;
		} while (--h);
	}
}

static void copy8Col(byte *dst, int dstPitch, const byte *src, int height) {
	do {
#if defined(SCUMM_NEED_ALIGNMENT)
		memcpy(dst, src, 8);
#else
		((uint32 *)dst)[0] = ((const uint32 *)src)[0];
		((uint32 *)dst)[1] = ((const uint32 *)src)[1];
#endif
		dst += dstPitch;
		src += dstPitch;
	} while (--height);
}

static void clear8Col(byte *dst, int dstPitch, int height) {
	do {
#if defined(SCUMM_NEED_ALIGNMENT)
		memset(dst, 0, 8);
#else
		((uint32 *)dst)[0] = 0;
		((uint32 *)dst)[1] = 0;
#endif
		dst += dstPitch;
	} while (--height);
}

void ScummEngine::drawBox(int x, int y, int x2, int y2, int color) {
	int width, height;
	VirtScreen *vs;
	byte *backbuff, *bgbuff;

	if ((vs = findVirtScreen(y)) == NULL)
		return;

	if (x > x2)
		SWAP(x, x2);

	if (y > y2)
		SWAP(y, y2);

	x2++;
	y2++;

	// Adjust for the topline of the VirtScreen
	y -= vs->topline;
	y2 -= vs->topline;
	
	// Clip the coordinates
	if (x < 0)
		x = 0;
	else if (x >= vs->w)
		return;

	if (x2 < 0)
		return;
	else if (x2 > vs->w)
		x2 = vs->w;

	if (y < 0)
		y = 0;
	else if (y > vs->h)
		return;

	if (y2 < 0)
		return;
	else if (y2 > vs->h)
		y2 = vs->h;
	
	width = x2 - x;
	height = y2 - y;

	// This will happen in the Sam & Max intro - see bug #1039162 - where
	// it would trigger an assertion in blit().

	if (width <= 0 || height <= 0)
		return;

	markRectAsDirty(vs->number, x, x2, y, y2);

	backbuff = vs->getPixels(x, y);

	if (color == -1) {
		if (vs->number != kMainVirtScreen)
			error("can only copy bg to main window");
		bgbuff = vs->getBackPixels(x, y);
		blit(backbuff, vs->pitch, bgbuff, vs->pitch, width, height);
		if (_charset->_hasMask) {
			byte *mask = (byte *)_charset->_textSurface.pixels + _charset->_textSurface.pitch * (y - _screenTop) + x;
			fill(mask, _charset->_textSurface.pitch, CHARSET_MASK_TRANSPARENCY, width, height);
		}
	} else {
		fill(backbuff, vs->pitch, color, width, height);
	}
}

void ScummEngine::drawFlashlight() {
	int i, j, x, y;
	VirtScreen *vs = &virtscr[kMainVirtScreen];

	// Remove the flash light first if it was previously drawn
	if (_flashlight.isDrawn) {
		markRectAsDirty(kMainVirtScreen, _flashlight.x, _flashlight.x + _flashlight.w,
										_flashlight.y, _flashlight.y + _flashlight.h, USAGE_BIT_DIRTY);
		
		if (_flashlight.buffer) {
			fill(_flashlight.buffer, vs->pitch, 0, _flashlight.w, _flashlight.h);
		}
		_flashlight.isDrawn = false;
	}

	if (_flashlight.xStrips == 0 || _flashlight.yStrips == 0)
		return;

	// Calculate the area of the flashlight
	if (_gameId == GID_ZAK256 || _version <= 2) {
		x = _mouse.x + vs->xstart;
		y = _mouse.y - vs->topline;
	} else {
		Actor *a = derefActor(VAR(VAR_EGO), "drawFlashlight");
		x = a->_pos.x;
		y = a->_pos.y;
	}
	_flashlight.w = _flashlight.xStrips * 8;
	_flashlight.h = _flashlight.yStrips * 8;
	_flashlight.x = x - _flashlight.w / 2 - _screenStartStrip * 8;
	_flashlight.y = y - _flashlight.h / 2;

	if (_gameId == GID_LOOM || _gameId == GID_LOOM256)
		_flashlight.y -= 12;

	// Clip the flashlight at the borders
	if (_flashlight.x < 0)
		_flashlight.x = 0;
	else if (_flashlight.x + _flashlight.w > gdi._numStrips * 8)
		_flashlight.x = gdi._numStrips * 8 - _flashlight.w;
	if (_flashlight.y < 0)
		_flashlight.y = 0;
	else if (_flashlight.y + _flashlight.h> vs->h)
		_flashlight.y = vs->h - _flashlight.h;

	// Redraw any actors "under" the flashlight
	for (i = _flashlight.x / 8; i < (_flashlight.x + _flashlight.w) / 8; i++) {
		assert(0 <= i && i < gdi._numStrips);
		setGfxUsageBit(_screenStartStrip + i, USAGE_BIT_DIRTY);
		vs->tdirty[i] = 0;
		vs->bdirty[i] = vs->h;
	}

	byte *bgbak;
	_flashlight.buffer = vs->getPixels(_flashlight.x, _flashlight.y);
	bgbak = vs->getBackPixels(_flashlight.x, _flashlight.y);

	blit(_flashlight.buffer, vs->pitch, bgbak, vs->pitch, _flashlight.w, _flashlight.h);

	// Round the corners. To do so, we simply hard-code a set of nicely
	// rounded corners.
	static const int corner_data[] = { 8, 6, 4, 3, 2, 2, 1, 1 };
	int minrow = 0;
	int maxcol = _flashlight.w - 1;
	int maxrow = (_flashlight.h - 1) * vs->pitch;

	for (i = 0; i < 8; i++, minrow += vs->pitch, maxrow -= vs->pitch) {
		int d = corner_data[i];

		for (j = 0; j < d; j++) {
			_flashlight.buffer[minrow + j] = 0;
			_flashlight.buffer[minrow + maxcol - j] = 0;
			_flashlight.buffer[maxrow + j] = 0;
			_flashlight.buffer[maxrow + maxcol - j] = 0;
		}
	}
	
	_flashlight.isDrawn = true;
}

bool ScummEngine::isLightOn() const {
	return (VAR_CURRENT_LIGHTS == 0xFF) || (VAR(VAR_CURRENT_LIGHTS) & LIGHTMODE_screen);
}

void ScummEngine::setShake(int mode) {
	if (_shakeEnabled != (mode != 0))
		_fullRedraw = true;

	_shakeEnabled = mode != 0;
	_shakeFrame = 0;
	_system->setShakePos(0);
}

#pragma mark -
#pragma mark --- Image drawing ---
#pragma mark -


void Gdi::drawBitmapV2Helper(const byte *ptr, VirtScreen *vs, int x, int y, const int width, const int height, int stripnr, int numstrip) {
	StripTable *table = (_objectMode ? 0 : _roomStrips);
	const int left = (stripnr * 8);
	const int right = left + (numstrip * 8);
	byte *dst;
	byte *mask_ptr;
	const byte *src;
	byte color, data = 0;
	int run;
	bool dither = false;
	byte dither_table[128];
	byte *ptr_dither_table;
	int theX, theY, maxX;

	memset(dither_table, 0, sizeof(dither_table));

	if (vs->hasTwoBuffers)
		dst = vs->backBuf + y * vs->pitch + x * 8;
	else
		dst = (byte *)vs->pixels + y * vs->pitch + x * 8;

	mask_ptr = getMaskBuffer(x, y, 1);


	if (table) {
		run = table->run[stripnr];
		color = table->color[stripnr];
		src = ptr + table->offsets[stripnr];
		theX = left;
		maxX = right;
	} else {
		run = 1;
		color = 0;
		src = ptr;
		theX = 0;
		maxX = width;
	}
	
	// Decode and draw the image data.
	assert(height <= 128);
	for (; theX < maxX; theX++) {
		ptr_dither_table = dither_table;
		for (theY = 0; theY < height; theY++) {
			if (--run == 0) {
				data = *src++;
				if (data & 0x80) {
					run = data & 0x7f;
					dither = true;
				} else {
					run = data >> 4;
					dither = false;
				}
				color = _roomPalette[data & 0x0f];
				if (run == 0) {
					run = *src++;
				}
			}
			if (!dither) {
				*ptr_dither_table = color;
			}
			if (left <= theX && theX < right) {
				*dst = *ptr_dither_table++;
				dst += vs->pitch;
			}
		}
		if (left <= theX && theX < right) {
			dst -= _vertStripNextInc;
		}
	}


	// Draw mask (zplane) data
	theY = 0;

	if (table) {
		src = ptr + table->zoffsets[stripnr];
		run = table->zrun[stripnr];
		theX = left;
	} else {
		run = *src++;
		theX = 0;
	}
	while (theX < right) {
		const byte runFlag = run & 0x80;
		if (runFlag) {
			run &= 0x7f;
			data = *src++;
		}
		do {
			if (!runFlag)
				data = *src++;
			
			if (left <= theX) {
				*mask_ptr = data;
				mask_ptr += _numStrips;
			}
			theY++;
			if (theY >= height) {
				if (left <= theX) {
					mask_ptr -= _numStrips * height - 1;
				}
				theY = 0;
				theX += 8;
				if (theX >= right)
					break;
			}
		} while (--run);
		run = *src++;
	}
}

int Gdi::getZPlanes(const byte *ptr, const byte *zplane_list[9], bool bmapImage) const {
	int numzbuf;
	int i;

	if ((_vm->_features & GF_SMALL_HEADER) || _vm->_version == 8)
		zplane_list[0] = ptr;
	else if (bmapImage)
		zplane_list[0] = _vm->findResource(MKID('BMAP'), ptr);
	else
		zplane_list[0] = _vm->findResource(MKID('SMAP'), ptr);

	if (_zbufferDisabled)
		numzbuf = 0;
	else if (_numZBuffer <= 1 || (_vm->_version <= 2))
		numzbuf = _numZBuffer;
	else {
		numzbuf = _numZBuffer;
		assert(numzbuf <= 9);
		
		if (_vm->_features & GF_SMALL_HEADER) {
			if (_vm->_features & GF_16COLOR)
				zplane_list[1] = ptr + READ_LE_UINT16(ptr);
			else {
				zplane_list[1] = ptr + READ_LE_UINT32(ptr);
				if (_vm->_features & GF_OLD256) {
					if (0 == READ_LE_UINT32(zplane_list[1]))
						zplane_list[1] = 0;
				}
			}
			for (i = 2; i < numzbuf; i++) {
				zplane_list[i] = zplane_list[i-1] + READ_LE_UINT16(zplane_list[i-1]);
			}
		} else if (_vm->_version == 8) {
			// Find the OFFS chunk of the ZPLN chunk
			const byte *zplnOffsChunkStart = ptr + 24 + READ_BE_UINT32(ptr + 12);
			
			// Each ZPLN contains a WRAP chunk, which has (as always) an OFFS subchunk pointing
			// at ZSTR chunks. These once more contain a WRAP chunk which contains nothing but
			// an OFFS chunk. The content of this OFFS chunk contains the offsets to the
			// Z-planes.
			// We do not directly make use of this, but rather hard code offsets (like we do
			// for all other Scumm-versions, too). Clearly this is a bit hackish, but works
			// well enough, and there is no reason to assume that there are any cases where it
			// might fail. Still, doing this properly would have the advantage of catching
			// invalid/damaged data files, and allow us to exit gracefully instead of segfaulting.
			for (i = 1; i < numzbuf; i++) {
				zplane_list[i] = zplnOffsChunkStart + READ_LE_UINT32(zplnOffsChunkStart + 4 + i*4) + 16;
			}
		} else {
			const uint32 zplane_tags[] = {
				MKID('ZP00'),
				MKID('ZP01'),
				MKID('ZP02'),
				MKID('ZP03'),
				MKID('ZP04')
			};
			
			for (i = 1; i < numzbuf; i++) {
				zplane_list[i] = _vm->findResource(zplane_tags[i], ptr);
			}
		}
	}
	
	return numzbuf;
}

/**
 * Draw a bitmap onto a virtual screen. This is main drawing method for room backgrounds
 * and objects, used throughout all SCUMM versions.
 */
void Gdi::drawBitmap(const byte *ptr, VirtScreen *vs, int x, int y, const int width, const int height,
					int stripnr, int numstrip, byte flag) {
	assert(ptr);
	assert(height > 0);
	byte *dstPtr;
	const byte *smap_ptr;
	const byte *z_plane_ptr;
	byte *mask_ptr;

	int i;
	const byte *zplane_list[9];

	int bottom;
	int numzbuf;
	int sx;
	bool useOrDecompress = false;

	// Check whether lights are turned on or not
	const bool lightsOn = _vm->isLightOn();

	_objectMode = (flag & dbObjectMode) == dbObjectMode;
	
	if (_objectMode && _vm->_version == 1) {
		if (_vm->_platform == Common::kPlatformNES) {
			// TODO: Maybe call decodeNESObject here?
		} else {
			decodeC64Gfx(ptr, _C64.objectMap, (width / 8) * (height / 8) * 3);
		}
	}

	CHECK_HEAP;
	if (_vm->_features & GF_SMALL_HEADER) {
		smap_ptr = ptr;
	} else if (_vm->_version == 8) {
		// Skip to the BSTR->WRAP->OFFS chunk
		smap_ptr = ptr + 24;
	} else
		smap_ptr = _vm->findResource(MKID('SMAP'), ptr);

	assert(smap_ptr);

	numzbuf = getZPlanes(ptr, zplane_list, false);
	
	bottom = y + height;
	if (bottom > vs->h) {
		warning("Gdi::drawBitmap, strip drawn to %d below window bottom %d", bottom, vs->h);
	}

	_vertStripNextInc = height * vs->pitch - 1;

	//
	// Since V3, all graphics data was encoded in strips, which is very efficient
	// for redrawing only parts of the screen. However, V2 is different: here
	// the whole graphics are encoded as one big chunk. That makes it rather
	// dificult to draw only parts of a room/object. We handle the V2 graphics
	// differently from all other (newer) graphic formats for this reason.
	//
	if (_vm->_version == 2)
		drawBitmapV2Helper(ptr, vs, x, y, width, height, stripnr, numstrip);

	sx = x - vs->xstart / 8;
	if (sx < 0) {
		numstrip -= -sx;
		x += -sx;
		stripnr += -sx;
		sx = 0;
	}

	//if (_vm->_NESStartStrip > 0)
	//	stripnr -= _vm->_NESStartStrip;

	while (numstrip > 0 && sx < _numStrips && x * 8 < MAX(_vm->_roomWidth, (int) vs->w)) {
		CHECK_HEAP;

		if (y < vs->tdirty[sx])
			vs->tdirty[sx] = y;

		if (bottom > vs->bdirty[sx])
			vs->bdirty[sx] = bottom;

		// In the case of a double buffered virtual screen, we draw to
		// the backbuffer, otherwise to the primary surface memory.
		if (vs->hasTwoBuffers)
			dstPtr = vs->backBuf + y * vs->pitch + x * 8;
		else
			dstPtr = (byte *)vs->pixels + y * vs->pitch + x * 8;

		if (_vm->_version == 1) {
			if (_vm->_platform == Common::kPlatformNES) {
				mask_ptr = getMaskBuffer(x, y, 0);
				drawStripNES(dstPtr, mask_ptr, vs->pitch, stripnr, y, height);
			}
			else if (_objectMode)
				drawStripC64Object(dstPtr, vs->pitch, stripnr, width, height);
			else
				drawStripC64Background(dstPtr, vs->pitch, stripnr, height);
		} else if (_vm->_version == 2) {
			// Do nothing here for V2 games - drawing was already handled.
		} else {
			// Do some input verification and make sure the strip/strip offset
			// are actually valid. Normally, this should never be a problem,
			// but if e.g. a savegame gets corrupted, we can easily get into
			// trouble here. See also bug #795214.
			int offset = -1, smapLen;
			if (_vm->_features & GF_16COLOR) {
				smapLen = READ_LE_UINT16(smap_ptr);
				if (stripnr * 2 + 2 < smapLen)
					offset = READ_LE_UINT16(smap_ptr + stripnr * 2 + 2);
			} else if (_vm->_features & GF_SMALL_HEADER) {
				smapLen = READ_LE_UINT32(smap_ptr);
				if (stripnr * 4 + 4 < smapLen)
					offset = READ_LE_UINT32(smap_ptr + stripnr * 4 + 4);
			} else {
				smapLen = READ_BE_UINT32(smap_ptr);
				if (stripnr * 4 + 8 < smapLen)
					offset = READ_LE_UINT32(smap_ptr + stripnr * 4 + 8);
			}
			if (offset < 0 || offset >= smapLen) {
				warning("drawBitmap: Trying to draw a non-existant strip");
				return;
			}
			useOrDecompress = decompressBitmap(dstPtr, vs->pitch, smap_ptr + offset, height);
		}

		CHECK_HEAP;
		if (vs->hasTwoBuffers) {
			byte *frontBuf = (byte *)vs->pixels + y * vs->pitch + x * 8;
			if (lightsOn)
				copy8Col(frontBuf, vs->pitch, dstPtr, height);
			else
				clear8Col(frontBuf, vs->pitch, height);
		}
		CHECK_HEAP;

		// COMI and HE games only uses flag value
		if (_vm->_version == 8 || _vm->_heversion >= 60)
			useOrDecompress = true;

		if (_vm->_version == 1) {
			mask_ptr = getMaskBuffer(x, y, 1);
			if (_vm->_platform == Common::kPlatformNES) {
				drawStripNESMask(mask_ptr, stripnr, height);
			} else {
				drawStripC64Mask(mask_ptr, stripnr, width, height);
			}
		} else if (_vm->_version == 2) {
			// Do nothing here for V2 games - zplane was already handled.
		} else if (flag & dbDrawMaskOnAll) {
			// Sam & Max uses dbDrawMaskOnAll for things like the inventory
			// box and the speech icons. While these objects only have one
			// mask, it should be applied to all the Z-planes in the room,
			// i.e. they should mask every actor.
			//
			// This flag used to be called dbDrawMaskOnBoth, and all it
			// would do was to mask Z-plane 0. (Z-plane 1 would also be
			// masked, because what is now the else-clause used to be run
			// always.) While this seems to be the only way there is to
			// mask Z-plane 0, this wasn't good enough since actors in
			// Z-planes >= 2 would not be masked.
			//
			// The flag is also used by The Dig and Full Throttle, but I
			// don't know what for. At the time of writing, these games
			// are still too unstable for me to investigate.

			if (_vm->_version == 8)
				z_plane_ptr = zplane_list[1] + READ_LE_UINT32(zplane_list[1] + stripnr * 4 + 8);
			else
				z_plane_ptr = zplane_list[1] + READ_LE_UINT16(zplane_list[1] + stripnr * 2 + 8);
			for (i = 0; i < numzbuf; i++) {
				mask_ptr = getMaskBuffer(x, y, i);
				if (useOrDecompress && (flag & dbAllowMaskOr))
					decompressMaskImgOr(mask_ptr, z_plane_ptr, height);
				else
					decompressMaskImg(mask_ptr, z_plane_ptr, height);
			}
		} else {
			for (i = 1; i < numzbuf; i++) {
				uint32 offs;

				if (!zplane_list[i])
					continue;

				if (_vm->_features & GF_OLD_BUNDLE)
					offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2);
				else if (_vm->_features & GF_OLD256)
					offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2 + 4);
				else if (_vm->_features & GF_SMALL_HEADER)
					offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2 + 2);
				else if (_vm->_version == 8)
					offs = READ_LE_UINT32(zplane_list[i] + stripnr * 4 + 8);
				else
					offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2 + 8);

				mask_ptr = getMaskBuffer(x, y, i);

				if (offs) {
					z_plane_ptr = zplane_list[i] + offs;

					if (useOrDecompress && (flag & dbAllowMaskOr)) {
						decompressMaskImgOr(mask_ptr, z_plane_ptr, height);
					} else {
						decompressMaskImg(mask_ptr, z_plane_ptr, height);
					}

				} else {
					if (!(useOrDecompress && (flag & dbAllowMaskOr)))
						for (int h = 0; h < height; h++)
							mask_ptr[h * _numStrips] = 0;
					// FIXME: needs better abstraction
				}
			}
		}
		
		numstrip--;
		x++;
		sx++;
		stripnr++;
	}
}

/**
 * Draw a bitmap onto a virtual screen. This is main drawing method for room backgrounds
 * used throughout in 7.2+ HE versions.
 *
 * @note This function essentially is a stripped down & special cased version of
 * the generic Gdi::drawBitmap() method.
 */
void Gdi::drawBMAPBg(const byte *ptr, VirtScreen *vs) {
	const byte *z_plane_ptr;
	byte *mask_ptr;
	const byte *zplane_list[9];

	const byte *bmap_ptr = _vm->findResourceData(MKID('BMAP'), ptr);
	assert(bmap_ptr);

	byte code = *bmap_ptr++;
	int scrX = _vm->_screenStartStrip * 8;
	byte *dst = (byte *)_vm->virtscr[0].backBuf + scrX;

	// The following few lines more or less duplicate decompressBitmap(), only
	// for an area spanning multiple strips. In particular, the codecs 13 & 14
	// in decompressBitmap call drawStripHE()
	_decomp_shr = code % 10;
	_decomp_mask = 0xFF >> (8 - _decomp_shr);
	code /= 10;
		
	switch (code) {
	case 13:	
		drawStripHE(dst, vs->pitch, bmap_ptr, vs->w, vs->h, false);
		break;
	case 14:
		drawStripHE(dst, vs->pitch, bmap_ptr, vs->w, vs->h, true);
		break;
	case 15:
		fill(dst, vs->pitch, *bmap_ptr, vs->w, vs->h);
		break;
	default:
		// Alternayive russian freddi3 uses badly formatted bitmaps
		warning("Gdi::drawBMAPBg: default case %d", code);
	}

	copyVirtScreenBuffers(Common::Rect(vs->w, vs->h));

	int numzbuf = getZPlanes(ptr, zplane_list, true);
	if (numzbuf <= 1)
		return;

	uint32 offs;
	for (int stripnr = 0; stripnr < _numStrips; stripnr++)
		for (int i = 1; i < numzbuf; i++) {
			if (!zplane_list[i])
				continue;

			offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2 + 8);
			mask_ptr = getMaskBuffer(stripnr, 0, i);

			if (offs) {
				z_plane_ptr = zplane_list[i] + offs;
				decompressMaskImg(mask_ptr, z_plane_ptr, vs->h);
			}
		}
}

void Gdi::drawBMAPObject(const byte *ptr, VirtScreen *vs, int obj, int x, int y, int w, int h) {
	const byte *bmap_ptr = _vm->findResourceData(MKID('BMAP'), ptr);
	assert(bmap_ptr);

	byte code = *bmap_ptr++;
	int scrX = _vm->_screenStartStrip * 8;

	if (code == 8 || code == 9) {
		Common::Rect rScreen(0, 0, vs->w, vs->h);
		byte *dst = (byte *)_vm->virtscr[0].backBuf + scrX;
		Wiz::copyWizImage(dst, bmap_ptr, vs->w, vs->h, x - scrX, y, w, h, &rScreen);
	}

	Common::Rect rect1(x, y, x + w, y + h);
	Common::Rect rect2(scrX, 0, vs->w + scrX, vs->h);

	if (rect1.intersects(rect2)) {
		rect1.clip(rect2);
		rect1.left -= rect2.left;
		rect1.right -= rect2.left;
		rect1.top -= rect2.top;
		rect1.bottom -= rect2.top;
		
		copyVirtScreenBuffers(rect1);
	}
}

void Gdi::copyVirtScreenBuffers(Common::Rect rect, int dirtybit) {
	byte *src, *dst;
	VirtScreen *vs = &_vm->virtscr[0];

	debug(1,"copyVirtScreenBuffers: Left %d Right %d Top %d Bottom %d", rect.left, rect.right, rect.top, rect.bottom);

	if (rect.top > vs->h || rect.bottom < 0)
		return;

	if (rect.left > vs->w || rect.right < 0)
		return;

	rect.left = MAX(0, (int)rect.left);
	rect.left = MIN((int)rect.left, (int)vs->w - 1);

	rect.right = MAX(0, (int)rect.right);
	rect.right = MIN((int)rect.right, (int)vs->w);

	rect.top = MAX(0, (int)rect.top);
	rect.top = MIN((int)rect.top, (int)vs->h - 1);

	rect.bottom = MAX(0, (int)rect.bottom);
	rect.bottom = MIN((int)rect.bottom, (int)vs->h);

	const int rw = rect.width();
	const int rh = rect.height();
	
	if (rw == 0 || rh == 0)
		return;

	src = _vm->virtscr[0].getBackPixels(rect.left, rect.top);
	dst = _vm->virtscr[0].getPixels(rect.left, rect.top);
	
	assert(rw <= _vm->_screenWidth && rw > 0);
	assert(rh <= _vm->_screenHeight && rh > 0);
	blit(dst, _vm->virtscr[0].pitch, src, _vm->virtscr[0].pitch, rw, rh);
	_vm->markRectAsDirty(kMainVirtScreen, rect, dirtybit);
}

/**
 * Reset the background behind an actor or blast object.
 */
void Gdi::resetBackground(int top, int bottom, int strip) {
	VirtScreen *vs = &_vm->virtscr[0];
	byte *backbuff_ptr, *bgbak_ptr;
	int numLinesToProcess;
	
	if (bottom >= vs->h)
		bottom = vs->h;

	if (top >= bottom)
		return;

	assert(0 <= strip && strip < _numStrips);

	if (top < vs->tdirty[strip])
		vs->tdirty[strip] = top;

	if (bottom > vs->bdirty[strip])
		vs->bdirty[strip] = bottom;

	bgbak_ptr = (byte *)vs->backBuf + top * vs->pitch + (strip + vs->xstart/8) * 8;
	backbuff_ptr = (byte *)vs->pixels + top * vs->pitch + (strip + vs->xstart/8) * 8;

	numLinesToProcess = bottom - top;
	if (numLinesToProcess) {
		if (_vm->isLightOn()) {
			copy8Col(backbuff_ptr, vs->pitch, bgbak_ptr, numLinesToProcess);
		} else {
			clear8Col(backbuff_ptr, vs->pitch, numLinesToProcess);
		}
	}
}

bool Gdi::decompressBitmap(byte *dst, int dstPitch, const byte *src, int numLinesToProcess) {
	assert(numLinesToProcess);
	
	if (_vm->_features & GF_16COLOR) {
		drawStripEGA(dst, dstPitch, src, numLinesToProcess);
		return false;
	}

	byte code = *src++;
	bool useOrDecompress = false;
	
	if (code <= 10) {
		switch (code) {
		case 1:
			unkDecode7(dst, dstPitch, src, numLinesToProcess);
			break;
	
		case 2:
			unkDecode8(dst, dstPitch, src, numLinesToProcess);       /* Ender - Zak256/Indy256 */
			break;
	
		case 3:
			unkDecode9(dst, dstPitch, src, numLinesToProcess);       /* Ender - Zak256/Indy256 */
			break;
	
		case 4:
			unkDecode10(dst, dstPitch, src, numLinesToProcess);      /* Ender - Zak256/Indy256 */
			break;
	
		case 7:
			unkDecode11(dst, dstPitch, src, numLinesToProcess);      /* Ender - Zak256/Indy256 */
			break;
	
		case 8:
			// Used in 3DO versions of HE games
			useOrDecompress = true;
			drawStrip3DO(dst, dstPitch, src, numLinesToProcess, true);
			break;
	
		case 9:
			drawStrip3DO(dst, dstPitch, src, numLinesToProcess, false);
			break;
	
		case 10:
			// Used in Amiga version of Monkey Island 1
			drawStripEGA(dst, dstPitch, src, numLinesToProcess);
			break;

		default:
			error("Gdi::decompressBitmap: default case %d", code);
		}
	} else {
		_decomp_shr = code % 10;
		_decomp_mask = 0xFF >> (8 - _decomp_shr);
		code /= 10;
		
		switch (code) {
		case 1:
			drawStripBasicV(dst, dstPitch, src, numLinesToProcess, false);
			break;
	
		case 2:
			drawStripBasicH(dst, dstPitch, src, numLinesToProcess, false);
			break;
	
		case 3:
			useOrDecompress = true;
			drawStripBasicV(dst, dstPitch, src, numLinesToProcess, true);
			break;
	
		case 4:
			useOrDecompress = true;
			drawStripBasicH(dst, dstPitch, src, numLinesToProcess, true);
			break;
	
		case 6:
		case 10:
			drawStripComplex(dst, dstPitch, src, numLinesToProcess, false);
			break;
	
		case 8:
		case 12:
			useOrDecompress = true;
			drawStripComplex(dst, dstPitch, src, numLinesToProcess, true);
			break;
	
		case 13:
			drawStripHE(dst, dstPitch, src, 8, numLinesToProcess, false);
			break;
	
		case 14:
			useOrDecompress = true;
			drawStripHE(dst, dstPitch, src, 8, numLinesToProcess, true);
			break;
	
		default:
			error("Gdi::decompressBitmap: default case %d", code);
		}
	}
	
	return useOrDecompress;
}

void Gdi::decompressMaskImg(byte *dst, const byte *src, int height) const {
	byte b, c;

	while (height) {
		b = *src++;

		if (b & 0x80) {
			b &= 0x7F;
			c = *src++;

			do {
				*dst = c;
				dst += _numStrips;
				--height;
			} while (--b && height);
		} else {
			do {
				*dst = *src++;
				dst += _numStrips;
				--height;
			} while (--b && height);
		}
	}
}

void Gdi::decompressMaskImgOr(byte *dst, const byte *src, int height) const {
	byte b, c;

	while (height) {
		b = *src++;
		
		if (b & 0x80) {
			b &= 0x7F;
			c = *src++;

			do {
				*dst |= c;
				dst += _numStrips;
				--height;
			} while (--b && height);
		} else {
			do {
				*dst |= *src++;
				dst += _numStrips;
				--height;
			} while (--b && height);
		}
	}
}

void decodeNESTileData(const byte *src, byte *dest) {
	int len = READ_LE_UINT16(src);	src += 2;
	const byte *end = src + len;
	src++;	// skip number-of-tiles byte, assume it is correct
	while (src < end) {
		byte data = *src++;
		for (int j = 0; j < (data & 0x7F); j++)
			*dest++ = (data & 0x80) ? (*src++) : (*src);
		if (!(data & 0x80))
			src++;
	}
}

void ScummEngine::decodeNESBaseTiles() {
	byte *basetiles = getResourceAddress(rtCostume, 37);
	_NESBaseTiles = basetiles[2];
	decodeNESTileData(basetiles, _NESPatTable[1]);
}

static const int v1MMNEScostTables[2][6] = {
     /* desc lens offs data  gfx  pal */
	{ 25,  27,  29,  31,  33,  35},
	{ 26,  28,  30,  32,  34,  36}
};
void ScummEngine::NES_loadCostumeSet(int n) {
	int i;
	_NESCostumeSet = n;

	_NEScostdesc = getResourceAddress(rtCostume, v1MMNEScostTables[n][0]) + 2;
	_NEScostlens = getResourceAddress(rtCostume, v1MMNEScostTables[n][1]) + 2;
	_NEScostoffs = getResourceAddress(rtCostume, v1MMNEScostTables[n][2]) + 2;
	_NEScostdata = getResourceAddress(rtCostume, v1MMNEScostTables[n][3]) + 2;
	decodeNESTileData(getResourceAddress(rtCostume, v1MMNEScostTables[n][4]), _NESPatTable[0]);
	byte *palette = getResourceAddress(rtCostume, v1MMNEScostTables[n][5]) + 2;
	for (i = 0; i < 16; i++) {
		byte c = *palette++;
		if (c == 0x1D)	// HACK - switch around colors 0x00 and 0x1D
			c = 0;		// so we don't need a zillion extra checks
		else if (c == 0)// for determining the proper background color
			c = 0x1D;
		_NESPalette[1][i] = c;
	}

}

void Gdi::decodeNESGfx(const byte *room) {
	const byte *gdata = room + READ_LE_UINT16(room + 0x0A);
	int tileset = *gdata++;
	int width = READ_LE_UINT16(room + 0x04);
	// int height = READ_LE_UINT16(room + 0x06);
	int i, j, n;

	// We have narrow room. so expand it
	if (width < 32) {
		_vm->_NESStartStrip = (32 - width) >> 1;
	} else {
		_vm->_NESStartStrip = 0;
	}

	decodeNESTileData(_vm->getResourceAddress(rtCostume, 37 + tileset), _vm->_NESPatTable[1] + _vm->_NESBaseTiles * 16);
	for (i = 0; i < 16; i++) {
		byte c = *gdata++;
		if (c == 0x0D)
			c = 0x1D;

		if (c == 0x1D)	 // HACK - switch around colors 0x00 and 0x1D
			c = 0;		 // so we don't need a zillion extra checks
		else if (c == 0) // for determining the proper background color
			c = 0x1D;

		_vm->_NESPalette[0][i] = c;
	}
	for (i = 0; i < 16; i++) {
		_NES.nametable[i][0] = _NES.nametable[i][1] = 0;
		n = 0;
		while (n < width) {
			byte data = *gdata++;
			for (j = 0; j < (data & 0x7F); j++)
				_NES.nametable[i][2 + n++] = (data & 0x80) ? (*gdata++) : (*gdata);
			if (!(data & 0x80))
				gdata++;
		}
		_NES.nametable[i][width+2] = _NES.nametable[i][width+3] = 0;
	}

	const byte *adata = room + READ_LE_UINT16(room + 0x0C);
	for (n = 0; n < 64;) {
		byte data = *adata++;
		for (j = 0; j < (data & 0x7F); j++)
			_NES.attributes[n++] = (data & 0x80) ? (*adata++) : (*adata);
		if (!(n & 7) && (width == 0x1C))
			n += 8;
		if (!(data & 0x80))
			adata++;
	}

	const byte *mdata = room + READ_LE_UINT16(room + 0x0E);
	int mask = *mdata++;
	if (mask == 0) {
		_NES.hasmask = false;
		return;
	}
	_NES.hasmask = true;
	if (mask != 1)
		debug(0,"NES room %i has irregular mask count %i!",_vm->_currentRoom,mask);
	int mwidth = *mdata++;
	for (i = 0; i < 16; i++) {
		n = 0;
		while (n < mwidth) {
			byte data = *mdata++;
			for (j = 0; j < (data & 0x7F); j++)
				_NES.masktable[i][n++] = (data & 0x80) ? (*mdata++) : (*mdata);
			if (!(data & 0x80))
				mdata++;
		}
	}
}

void Gdi::decodeNESObject(const byte *ptr, int xpos, int ypos, int width, int height) {
	int x, y;

	_NES.objX = xpos;

	// decode tile update data
	memcpy(_NES.nametableObj,_NES.nametable,16*64);
	ypos /= 8;
	height /= 8;
	for (y = ypos; y < ypos + height; y++) {
		x = xpos;
		while (x < xpos + width) {
			byte len = *ptr++;
			for (int i = 0; i < (len & 0x7F); i++)
				_NES.nametableObj[y][2 + x++] = (len & 0x80) ? (*ptr++) : (*ptr);
			if (!(len & 0x80))
				ptr++;
		}
	}

	int ax, ay;
	// decode attribute update data
	memcpy(_NES.attributesObj, _NES.attributes,64);
	y = height / 2;
	ay = ypos;
	while (y) {
		ax = xpos + 2;
		x = 0;
		int adata = 0;
		while (x < (width >> 1)) {
			if (!(x & 3))
				adata = *ptr++;
			byte *dest = &_NES.attributesObj[((ay << 2) & 0x30) | ((ax >> 2) & 0xF)];

			int aand = 3;
			int aor = adata & 3;
			if (ay & 0x02) {
				aand <<= 4;
				aor <<= 4;
			}
			if (ax & 0x02) {
				aand <<= 2;
				aor <<= 2;
			}
			*dest = ((~aand) & *dest) | aor;

			adata >>= 2;
			ax += 2;
			x++;
		}
		ay += 2;
		y--;
	}

	// decode mask update data
	if (!_NES.hasmask)
		return;
	memcpy(_NES.masktableObj, _NES.masktable,16*8);
	int mx, mwidth;
	int lmask, rmask;
	mx = *ptr++;
	mwidth = *ptr++;
	lmask = *ptr++;
	rmask = *ptr++;

	y = 0;
	do {
		byte *dest = &_NES.masktableObj[y + ypos][mx];
		*dest++ = (*dest & lmask) | *ptr++;
		for (x = 1; x < mwidth; x++) {
			if (x + 1 == mwidth)
				*dest++ = (*dest & rmask) | *ptr++;
			else
				*dest++ = *ptr++;
		}
		y++;
	} while (y < height);
}

void Gdi::drawStripNES(byte *dst, byte *mask, int dstPitch, int stripnr, int top, int height) {
	top /= 8;
	height /= 8;
	int x = stripnr + 2;	// NES version has a 2 tile gap on each edge

	if (_objectMode)
		x += _NES.objX; // for objects, need to start at the left edge of the object, not the screen
	if (x > 63) {
		debug(0,"NES tried to render invalid strip %i",stripnr);
		return;
	}
	for (int y = top; y < top + height; y++) {
		int palette = ((_objectMode ? _NES.attributesObj : _NES.attributes)[((y << 2) & 0x30) | ((x >> 2) & 0xF)] >> (((y & 2) << 1) | (x & 2))) & 0x3;
		int tile = (_objectMode ? _NES.nametableObj : _NES.nametable)[y][x];

		for (int i = 0; i < 8; i++) {
			byte c0 = _vm->_NESPatTable[1][tile * 16 + i];
			byte c1 = _vm->_NESPatTable[1][tile * 16 + i + 8];
			for (int j = 0; j < 8; j++)
				dst[j] = _vm->_NESPalette[0][((c0 >> (7 - j)) & 1) | (((c1 >> (7 - j)) & 1) << 1) | (palette << 2)];
			dst += dstPitch;
			*mask = c0 | c1;
			mask += _numStrips;
		}
	}
}

void Gdi::drawStripNESMask(byte *dst, int stripnr, int height) const {
	if (!_NES.hasmask)
		return;
	height /= 8;
	int x = stripnr + 2;

	if (_objectMode)
		x += _NES.objX; // for objects, need to start at the left edge of the object, not the screen
	if (x > 63) {
		debug(0,"NES tried to mask invalid strip %i",stripnr);
		return;
	}
	for (int y = 0; y < height; y++) {
		// the ? 0xFF : 0x00 here might be backwards - '1' bits indicate that sprites can get hidden
		byte c = (((_objectMode ? _NES.masktableObj : _NES.masktable)[y][x >> 3] >> (x & 7)) & 1) ? 0xFF : 0x00;
		for (int i = 0; i < 8; i++) {
			*dst = c;
			dst += _numStrips;
		}
	}
}

void Gdi::drawStripC64Background(byte *dst, int dstPitch, int stripnr, int height) {
	int charIdx;
	height /= 8;
	for (int y = 0; y < height; y++) {
		_C64.colors[3] = (_C64.colorMap[y + stripnr * height] & 7);
		// Check for room color change in V1 zak
		if (_roomPalette[0] == 255) {
			_C64.colors[2] = _roomPalette[2];
			_C64.colors[1] = _roomPalette[1];
		}

		charIdx = _C64.picMap[y + stripnr * height] * 8;
		for (int i = 0; i < 8; i++) {
			byte c = _C64.charMap[charIdx + i];
			dst[0] = dst[1] = _C64.colors[(c >> 6) & 3];
			dst[2] = dst[3] = _C64.colors[(c >> 4) & 3];
			dst[4] = dst[5] = _C64.colors[(c >> 2) & 3];
			dst[6] = dst[7] = _C64.colors[(c >> 0) & 3];
			dst += dstPitch;
		}
	}
}

void Gdi::drawStripC64Object(byte *dst, int dstPitch, int stripnr, int width, int height) {
	int charIdx;
	height /= 8;
	width /= 8;
	for (int y = 0; y < height; y++) {
		_C64.colors[3] = (_C64.objectMap[(y + height) * width + stripnr] & 7);
		charIdx = _C64.objectMap[y * width + stripnr] * 8;
		for (int i = 0; i < 8; i++) {
			byte c = _C64.charMap[charIdx + i];
			dst[0] = dst[1] = _C64.colors[(c >> 6) & 3];
			dst[2] = dst[3] = _C64.colors[(c >> 4) & 3];
			dst[4] = dst[5] = _C64.colors[(c >> 2) & 3];
			dst[6] = dst[7] = _C64.colors[(c >> 0) & 3];
			dst += dstPitch;
		}
	}
}

void Gdi::drawStripC64Mask(byte *dst, int stripnr, int width, int height) const {
	int maskIdx;
	height /= 8;
	width /= 8;
	for (int y = 0; y < height; y++) {
		if (_objectMode)
			maskIdx = _C64.objectMap[(y + 2 * height) * width + stripnr] * 8;
		else
			maskIdx = _C64.maskMap[y + stripnr * height] * 8;
		for (int i = 0; i < 8; i++) {
			byte c = _C64.maskChar[maskIdx + i];

			// V1/C64 masks are inverted compared to what ScummVM expects
			*dst = c ^ 0xFF;
			dst += _numStrips;
		}
	}
}

void Gdi::decodeC64Gfx(const byte *src, byte *dst, int size) const {
	int x, z;
	byte color, run, common[4];

	for (z = 0; z < 4; z++) {
		common[z] = *src++;
	}

	x = 0;
	while (x < size) {
		run = *src++;
		if (run & 0x80) {
			color = common[(run >> 5) & 3];
			run &= 0x1F;
			for (z = 0; z <= run; z++) {
				dst[x++] = color;
			}
		} else if (run & 0x40) {
			run &= 0x3F;
			color = *src++;
			for (z = 0; z <= run; z++) {
				dst[x++] = color;
			}
		} else {
			for (z = 0; z <= run; z++) {
				dst[x++] = *src++;
			}
		}
	}
}

/**
 * Create and fill a table with offsets to the graphic and mask strips in the
 * given V2 EGA bitmap.
 * @param src		the V2 EGA bitmap
 * @param width		the width of the bitmap
 * @param height	the height of the bitmap
 * @param table		the strip table to fill
 * @return filled strip table
 */
StripTable *Gdi::generateStripTable(const byte *src, int width, int height, StripTable *table) const {

	// If no strip table was given to use, allocate a new one
	if (table == 0)
		table = (StripTable *)calloc(1, sizeof(StripTable));

	const byte *bitmapStart = src;
	byte color = 0, data = 0;
	int x, y, length = 0;
	byte run = 1;

	// Decode the graphics strips, and memorize the run/color values
	// as well as the byte offset.
	for (x = 0 ; x < width; x++) {

		if ((x % 8) == 0) {
			assert(x / 8 < 160);
			table->run[x / 8] = run;
			table->color[x / 8] = color;
			table->offsets[x / 8] = src - bitmapStart;
		}

		for (y = 0; y < height; y++) {
			if (--run == 0) {
				data = *src++;
				if (data & 0x80) {
					run = data & 0x7f;
				} else {
					run = data >> 4;
				}
				if (run == 0) {
					run = *src++;
				}
				color = data & 0x0f;
			}
		}
	}

	// The mask data follows immediately after the graphics.
	x = 0;
	y = height;
	width /= 8;
	
	for (;;) {
		length = *src++;
		const byte runFlag = length & 0x80;
		if (runFlag) {
			length &= 0x7f;
			data = *src++;
		}
		do {
			if (!runFlag)
				data = *src++;
			if (y == height) {
				assert(x < 120);
				table->zoffsets[x] = src - bitmapStart - 1;
				table->zrun[x] = length | runFlag;
			}
			if (--y == 0) {
				if (--width == 0)
					return table;
				x++;
				y = height;
			}
		} while (--length);
	}

	return table;
}

void Gdi::drawStripEGA(byte *dst, int dstPitch, const byte *src, int height) const {
	byte color = 0;
	int run = 0, x = 0, y = 0, z;

	while (x < 8) {
		color = *src++;
		
		if (color & 0x80) {
			run = color & 0x3f;

			if (color & 0x40) {
				color = *src++;

				if (run == 0) {
					run = *src++;
				}
				for (z = 0; z < run; z++) {
					*(dst + y * dstPitch + x) = (z & 1) ? _roomPalette[color & 0xf] : _roomPalette[color >> 4];

					y++;
					if (y >= height) {
						y = 0;
						x++;
					}
				}
			} else {
				if (run == 0) {
					run = *src++;
				}

				for (z = 0; z < run; z++) {
					*(dst + y * dstPitch + x) = *(dst + y * dstPitch + x - 1);

					y++;
					if (y >= height) {
						y = 0;
						x++;
					}
				}
			}
		} else {
			run = color >> 4;
			if (run == 0) {
				run = *src++;
			}
			
			for (z = 0; z < run; z++) {
				*(dst + y * dstPitch + x) = _roomPalette[color & 0xf];

				y++;
				if (y >= height) {
					y = 0;
					x++;
				}
			}
		}
	}
}

#define READ_BIT (shift--, dataBit = data & 1, data >>= 1, dataBit)
#define FILL_BITS(n) do {            \
		if (shift < n) {             \
			data |= *src++ << shift; \
			shift += 8;              \
		}                            \
	} while (0)

// NOTE: drawStripHE is actually very similar to drawStripComplex
void Gdi::drawStripHE(byte *dst, int dstPitch, const byte *src, int width, int height, const bool transpCheck) const {
	static const int delta_color[] = { -4, -3, -2, -1, 1, 2, 3, 4 };
	uint32 dataBit, data;
	byte color;
	int shift;
	
	color = *src++;
	data = READ_LE_UINT24(src);
	src += 3;
	shift = 24;
	
	int x = width;
	while (1) {
		if (!transpCheck || color != _transparentColor)
			*dst = _roomPalette[color];
		dst++;
		--x;
		if (x == 0) {
			x = width;
			dst += dstPitch - width;
			--height;
			if (height == 0)
				return;
		}
		FILL_BITS(1);
		if (READ_BIT) {
			FILL_BITS(1);
			if (READ_BIT) {
				FILL_BITS(3);
				color += delta_color[data & 7];
				shift -= 3;
				data >>= 3;
			} else {
				FILL_BITS(_decomp_shr);
				color = data & _decomp_mask;
				shift -= _decomp_shr;
				data >>= _decomp_shr;
			}
		}
	}
}

#undef READ_BIT
#undef FILL_BITS


void Gdi::drawStrip3DO(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const {
	if (height == 0)
		return;

	int decSize = height * 8;
	int curSize = 0;

	do {
		uint8 data = *src++;
		uint8 rle = data & 1;
		int len = (data >> 1) + 1;

		len = MIN(decSize, len);
		decSize -= len;

		if (!rle) {
			for (; len > 0; len--, src++, dst++) {
				if (!transpCheck || *src != _transparentColor)
					*dst = _roomPalette[*src];
				curSize++;
				if (!(curSize & 7))
					dst += dstPitch - 8; // Next row
			}
		} else {
			byte color = *src++;
			for (; len > 0; len--, dst++) {
				if (!transpCheck || color != _transparentColor)
					*dst = _roomPalette[color];
				curSize++;
				if (!(curSize & 7))
					dst += dstPitch - 8; // Next row
			}
		}
	} while (decSize > 0);
}


#define READ_BIT (cl--, bit = bits & 1, bits >>= 1, bit)
#define FILL_BITS do {              \
		if (cl <= 8) {              \
			bits |= (*src++ << cl); \
			cl += 8;                \
		}                           \
	} while (0)

void Gdi::drawStripComplex(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const {
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	byte incm, reps;

	do {
		int x = 8;
		do {
			FILL_BITS;
			if (!transpCheck || color != _transparentColor)
				*dst = _roomPalette[color];
			dst++;

		againPos:
			if (!READ_BIT) {
			} else if (!READ_BIT) {
				FILL_BITS;
				color = bits & _decomp_mask;
				bits >>= _decomp_shr;
				cl -= _decomp_shr;
			} else {
				incm = (bits & 7) - 4;
				cl -= 3;
				bits >>= 3;
				if (incm) {
					color += incm;
				} else {
					FILL_BITS;
					reps = bits & 0xFF;
					do {
						if (!--x) {
							x = 8;
							dst += dstPitch - 8;
							if (!--height)
								return;
						}
						if (!transpCheck || color != _transparentColor)
							*dst = _roomPalette[color];
						dst++;
					} while (--reps);
					bits >>= 8;
					bits |= (*src++) << (cl - 8);
					goto againPos;
				}
			}
		} while (--x);
		dst += dstPitch - 8;
	} while (--height);
}

void Gdi::drawStripBasicH(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const {
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	int8 inc = -1;

	do {
		int x = 8;
		do {
			FILL_BITS;
			if (!transpCheck || color != _transparentColor)
				*dst = _roomPalette[color];
			dst++;
			if (!READ_BIT) {
			} else if (!READ_BIT) {
				FILL_BITS;
				color = bits & _decomp_mask;
				bits >>= _decomp_shr;
				cl -= _decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--x);
		dst += dstPitch - 8;
	} while (--height);
}

void Gdi::drawStripBasicV(byte *dst, int dstPitch, const byte *src, int height, const bool transpCheck) const {
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	int8 inc = -1;

	int x = 8;
	do {
		int h = height;
		do {
			FILL_BITS;
			if (!transpCheck || color != _transparentColor)
				*dst = _roomPalette[color];
			dst += dstPitch;
			if (!READ_BIT) {
			} else if (!READ_BIT) {
				FILL_BITS;
				color = bits & _decomp_mask;
				bits >>= _decomp_shr;
				cl -= _decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--h);
		dst -= _vertStripNextInc;
	} while (--x);
}

#undef READ_BIT
#undef FILL_BITS

/* Ender - Zak256/Indy256 decoders */
#define READ_BIT_256                       \
		do {                               \
			if ((mask <<= 1) == 256) {     \
				buffer = *src++;           \
				mask = 1;                  \
			}                              \
			bits = ((buffer & mask) != 0); \
		} while (0)

#define READ_N_BITS(n, c)                  \
		do {                               \
			c = 0;                         \
			for (int b = 0; b < n; b++) {  \
				READ_BIT_256;              \
				c += (bits << b);          \
			}                              \
		} while (0)

#define NEXT_ROW                           \
		do {                               \
			dst += dstPitch;               \
			if (--h == 0) {                \
				if (!--x)                  \
					return;                \
				dst -= _vertStripNextInc;  \
				h = height;                \
			}                              \
		} while (0)

void Gdi::unkDecode7(byte *dst, int dstPitch, const byte *src, int height) const {

	if (_vm->_features & GF_OLD256) {
		uint h = height;
		int x = 8;
		for (;;) {
			*dst = *src++;
			NEXT_ROW;
		}
	} else {
		do {
			memcpy(dst, src, 8);
			dst += dstPitch;
			src += 8;
		} while (--height);
	}
}

void Gdi::unkDecode8(byte *dst, int dstPitch, const byte *src, int height) const {
	uint h = height;

	int x = 8;
	for (;;) {
		uint run = (*src++) + 1;
		byte color = *src++;

		do {
			*dst = _roomPalette[color];
			NEXT_ROW;
		} while (--run);
	}
}

void Gdi::unkDecode9(byte *dst, int dstPitch, const byte *src, int height) const {
	byte c, bits, color, run;
	int i;
	uint buffer = 0, mask = 128;
	int h = height;
	i = run = 0;

	int x = 8;
	for (;;) {
		READ_N_BITS(4, c);

		switch (c >> 2) {
		case 0:
			READ_N_BITS(4, color);
			for (i = 0; i < ((c & 3) + 2); i++) {
				*dst = _roomPalette[run * 16 + color];
				NEXT_ROW;
			}
			break;

		case 1:
			for (i = 0; i < ((c & 3) + 1); i++) {
				READ_N_BITS(4, color);
				*dst = _roomPalette[run * 16 + color];
				NEXT_ROW;
			}
			break;

		case 2:
			READ_N_BITS(4, run);
			break;
		}
	}
}

void Gdi::unkDecode10(byte *dst, int dstPitch, const byte *src, int height) const {
	int i;
	byte local_palette[256], numcolors = *src++;
	uint h = height;

	for (i = 0; i < numcolors; i++)
		local_palette[i] = *src++;

	int x = 8;

	for (;;) {
		byte color = *src++;
		if (color < numcolors) {
			*dst = _roomPalette[local_palette[color]];
			NEXT_ROW;
		} else {
			uint run = color - numcolors + 1;
			color = *src++;
			do {
				*dst = _roomPalette[color];
				NEXT_ROW;
			} while (--run);
		}
	}
}


void Gdi::unkDecode11(byte *dst, int dstPitch, const byte *src, int height) const {
	int bits, i;
	uint buffer = 0, mask = 128;
	byte inc = 1, color = *src++;

	int x = 8;
	do {
		int h = height;
		do {
			*dst = _roomPalette[color];
			dst += dstPitch;
			for (i = 0; i < 3; i++) {
				READ_BIT_256;
				if (!bits)
					break;
			}
			switch (i) {
			case 1:
				inc = -inc;
				color -= inc;
				break;

			case 2:
				color -= inc;
				break;

			case 3:
				inc = 1;
				READ_N_BITS(8, color);
				break;
			}
		} while (--h);
		dst -= _vertStripNextInc;
	} while (--x);
}

#undef NEXT_ROW
#undef READ_BIT_256


#pragma mark -
#pragma mark --- Transition effects ---
#pragma mark -

void ScummEngine::fadeIn(int effect) {
	updatePalette();

	switch (effect) {
	case 0:
		// seems to do nothing
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		// Some of the transition effects won't work properly unless
		// the screen is marked as clean first. At first I thought I
		// could safely do this every time fadeIn() was called, but
		// that broke the FOA intro. Probably other things as well.
		//
		// Hopefully it's safe to do it at this point, at least.
		virtscr[0].setDirtyRange(0, 0);
 		transitionEffect(effect - 1);
		break;
	case 128:
		unkScreenEffect6();
		break;
	case 129:
		break;
	case 130:
	case 131:
	case 132:
	case 133:
		scrollEffect(133 - effect);
		break;
	case 134:
		dissolveEffect(1, 1);
		break;
	case 135:
		unkScreenEffect5(1);
		break;
	default:
		warning("Unknown screen effect, %d", effect);
	}
	_screenEffectFlag = true;
}

void ScummEngine::fadeOut(int effect) {
	VirtScreen *vs = &virtscr[0];

	vs->setDirtyRange(0, 0);
	if (!(_features & GF_NEW_CAMERA))
		camera._last.x = camera._cur.x;

	if (_switchRoomEffect >= 130 && _switchRoomEffect <= 133) {
		// We're going to use scrollEffect(), so we'll need a copy of
		// the current VirtScreen zero.

		free(_scrollBuffer);
		_scrollBuffer = (byte *) malloc(vs->h * vs->pitch);
		memcpy(_scrollBuffer, vs->getPixels(0, 0), vs->h * vs->pitch);
	}


	if (_screenEffectFlag && effect != 0) {
	
		// Fill screen 0 with black
		memset(vs->getPixels(0, 0), 0, vs->pitch * vs->h);
	
		// Fade to black with the specified effect, if any.
		switch (effect) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			transitionEffect(effect - 1);
			break;
		case 128:
			unkScreenEffect6();
			break;
		case 129:
			// Just blit screen 0 to the display (i.e. display will be black)
			vs->setDirtyRange(0, vs->h);
			updateDirtyScreen(kMainVirtScreen);
			break;
		case 134:
			dissolveEffect(1, 1);
			break;
		case 135:
			unkScreenEffect5(1);
			break;
		default:
			warning("fadeOut: default case %d", effect);
		}
	}

	// Update the palette at the end (once we faded to black) to avoid
	// some nasty effects when the palette is changed
	updatePalette();

	_screenEffectFlag = false;
}

/**
 * Perform a transition effect. There are four different effects possible:
 * 0: Iris effect
 * 1: Box wipe (a black box expands from the upper-left corner to the lower-right corner)
 * 2: Box wipe (a black box expands from the lower-right corner to the upper-left corner)
 * 3: Inverse box wipe
 * All effects operate on 8x8 blocks of the screen. These blocks are updated
 * in a certain order; the exact order determines how the effect appears to the user.
 * @param a		the transition effect to perform
 */
void ScummEngine::transitionEffect(int a) {
	int delta[16];								// Offset applied during each iteration
	int tab_2[16];
	int i, j;
	int bottom;
	int l, t, r, b;
	const int height = MIN((int)virtscr[0].h, _screenHeight);

	for (i = 0; i < 16; i++) {
		delta[i] = transitionEffects[a].deltaTable[i];
		j = transitionEffects[a].stripTable[i];
		if (j == 24)
			j = height / 8 - 1;
		tab_2[i] = j;
	}

	bottom = height / 8;
	for (j = 0; j < transitionEffects[a].numOfIterations; j++) {
		for (i = 0; i < 4; i++) {
			l = tab_2[i * 4];
			t = tab_2[i * 4 + 1];
			r = tab_2[i * 4 + 2];
			b = tab_2[i * 4 + 3];
			if (t == b) {
				while (l <= r) {
					if (l >= 0 && l < gdi._numStrips && t < bottom) {
						virtscr[0].tdirty[l] = _screenTop + t * 8;
						virtscr[0].bdirty[l] = _screenTop + (b + 1) * 8;
					}
					l++;
				}
			} else {
				if (l < 0 || l >= gdi._numStrips || b <= t)
					continue;
				if (b > bottom)
					b = bottom;
				if (t < 0)
					t = 0;
 				virtscr[0].tdirty[l] = _screenTop + t * 8;
				virtscr[0].bdirty[l] = _screenTop + (b + 1) * 8;
			}
			updateDirtyScreen(kMainVirtScreen);
		}

		for (i = 0; i < 16; i++)
			tab_2[i] += delta[i];

		// Draw the current state to the screen and wait half a sec so the user
		// can watch the effect taking place.
		_system->updateScreen();
		waitForTimer(30);
	}
}

/**
 * Update width*height areas of the screen, in random order, until the whole
 * screen has been updated. For instance:
 * 
 * dissolveEffect(1, 1) produces a pixel-by-pixel dissolve
 * dissolveEffect(8, 8) produces a square-by-square dissolve
 * dissolveEffect(virtsrc[0].width, 1) produces a line-by-line dissolve
 */
void ScummEngine::dissolveEffect(int width, int height) {
#ifdef __PALM_OS__
	// Remove this dissolve effect for now on PalmOS since it is a bit
	// too slow using 68k emulation
	if (width == 1 && height == 1) {
		waitForTimer(30);
		return;
	}
#endif

	VirtScreen *vs = &virtscr[0];
	int *offsets;
	int blits_before_refresh, blits;
	int x, y;
	int w, h;
	int i;

	// There's probably some less memory-hungry way of doing this. But
	// since we're only dealing with relatively small images, it shouldn't
	// be too bad.

	w = vs->w / width;
	h = vs->h / height;

	// When used correctly, vs->width % width and vs->height % height
	// should both be zero, but just to be safe...

	if (vs->w % width)
		w++;

	if (vs->h % height)
		h++;

	offsets = (int *) malloc(w * h * sizeof(int));
	if (offsets == NULL) {
		warning("dissolveEffect: out of memory");
		return;
	}

	// Create a permutation of offsets into the frame buffer

	if (width == 1 && height == 1) {
		// Optimized case for pixel-by-pixel dissolve

		for (i = 0; i < vs->w * vs->h; i++)
			offsets[i] = i;

		for (i = 1; i < w * h; i++) {
			int j;

			j = _rnd.getRandomNumber(i - 1);
			offsets[i] = offsets[j];
			offsets[j] = i;
		}
	} else {
		int *offsets2;

		for (i = 0, x = 0; x < vs->w; x += width)
			for (y = 0; y < vs->h; y += height)
				offsets[i++] = y * vs->pitch + x;

		offsets2 = (int *) malloc(w * h * sizeof(int));
		if (offsets2 == NULL) {
			warning("dissolveEffect: out of memory");
			free(offsets);
			return;
		}

		memcpy(offsets2, offsets, w * h * sizeof(int));

		for (i = 1; i < w * h; i++) {
			int j;

			j = _rnd.getRandomNumber(i - 1);
			offsets[i] = offsets[j];
			offsets[j] = offsets2[i];
		}

		free(offsets2);
	}

	// Blit the image piece by piece to the screen. The idea here is that
	// the whole update should take about a quarter of a second, assuming
	// most of the time is spent in waitForTimer(). It looks good to me,
	// but might still need some tuning.

	blits = 0;
	blits_before_refresh = (3 * w * h) / 25;
	
	// Speed up the effect for CD Loom since it uses it so often. I don't
	// think the original had any delay at all, so on modern hardware it
	// wasn't even noticeable.
	if (_gameId == GID_LOOM256)
		blits_before_refresh *= 2;

	for (i = 0; i < w * h; i++) {
		x = offsets[i] % vs->pitch;
		y = offsets[i] / vs->pitch;
		_system->copyRectToScreen(vs->getPixels(x, y), vs->pitch, x, y + vs->topline, width, height);

		if (++blits >= blits_before_refresh) {
			blits = 0;
			_system->updateScreen();
			waitForTimer(30);
		}
	}

	free(offsets);

	if (blits != 0) {
		_system->updateScreen();
		waitForTimer(30);
	}
}

void ScummEngine::scrollEffect(int dir) {
	// It is at least technically possible that this function will be
	// called without _scrollBuffer having been set up, but will it ever
	// happen? I don't know.
	if (!_scrollBuffer)
		warning("scrollEffect: No scroll buffer. This may look bad");

	VirtScreen *vs = &virtscr[0];

	int x, y;
	int step;

	if ((dir == 0) || (dir == 1))
		step = vs->h;
	else
		step = vs->w;

	step = (step * kPictureDelay) / kScrolltime;

	switch (dir) {
	case 0:
		//up
		y = step;
		while (y < vs->h) {
			_system->copyRectToScreen(vs->getPixels(0, 0),
				vs->pitch,
				0, vs->h - y,
				vs->w, y);
			if (_scrollBuffer)
				_system->copyRectToScreen(_scrollBuffer + y * vs->w,
					vs->pitch,
					0, 0,
					vs->w, vs->h - y);
			_system->updateScreen();
			waitForTimer(kPictureDelay);

			y += step;
		}
		break;
	case 1:
		// down
		y = step;
		while (y < vs->h) {
			_system->copyRectToScreen(vs->getPixels(0, vs->h - y),
				vs->pitch,
				0, 0,
				vs->w, y);
			if (_scrollBuffer)
				_system->copyRectToScreen(_scrollBuffer,
					vs->pitch,
					0, y,
					vs->w, vs->h - y);
			_system->updateScreen();
			waitForTimer(kPictureDelay);

			y += step;
		}
		break;
	case 2:
		// left
		x = step;
		while (x < vs->w) {
			_system->copyRectToScreen(vs->getPixels(0, 0),
				vs->pitch,
				vs->w - x, 0,
				x, vs->h);
			if (_scrollBuffer)
				_system->copyRectToScreen(_scrollBuffer + x,
					vs->pitch,
					0, 0,
					vs->w - x, vs->h);
			_system->updateScreen();
			waitForTimer(kPictureDelay);

			x += step;
		}
		break;
	case 3:
		// right
		x = step;
		while (x < vs->w) {
			_system->copyRectToScreen(vs->getPixels(vs->w - x, 0),
				vs->pitch,
				0, 0,
				x, vs->h);
			if (_scrollBuffer)
				_system->copyRectToScreen(_scrollBuffer,
					vs->pitch,
					x, 0,
					vs->w - x, vs->h);
			_system->updateScreen();
			waitForTimer(kPictureDelay);

			x += step;
		}
		break;
	}

	free(_scrollBuffer);
	_scrollBuffer = NULL;
}

void ScummEngine::unkScreenEffect6() {
	// CD Loom (but not EGA Loom!) uses a more fine-grained dissolve
	if (_gameId == GID_LOOM256)
		dissolveEffect(1, 1);
	else
		dissolveEffect(8, 4);
}

void ScummEngine::unkScreenEffect5(int a) {
	// unkScreenEffect5(0), which is used by FOA during the opening
	// cutscene when Indy opens the small statue, has been replaced by
	// dissolveEffect(1, 1).
	//
	// I still don't know what unkScreenEffect5(1) is supposed to do.

	// FIXME: not implemented
	warning("stub unkScreenEffect(%d)", a);
}

} // End of namespace Scumm

#ifdef __PALM_OS__
#include "scumm_globals.h"

_GINIT(Gfx)
_GSETPTR(Scumm::transitionEffects, GBVARS_TRANSITIONEFFECTS_INDEX, Scumm::TransitionEffect, GBVARS_SCUMM)
_GEND

_GRELEASE(Gfx)
_GRELEASEPTR(GBVARS_TRANSITIONEFFECTS_INDEX, GBVARS_SCUMM)
_GEND

#endif
