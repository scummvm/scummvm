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

#include "stdafx.h"
#include "common/util.h"
#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/bomp.h"
#include "scumm/charset.h"
#include "scumm/resource.h"
#include "scumm/usage_bits.h"

namespace Scumm {

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
static const TransitionEffect transitionEffects[5] = {
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
		8,		// Number of iterations
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
	}
};
#endif

/*
 * Mouse cursor cycle colors (for the default crosshair).
 */
static const byte default_v1_cursor_colors[4] = {
	1, 1, 12, 11
};

static const byte default_cursor_colors[4] = {
	15, 15, 7, 8
};



static const uint16 default_cursor_images[5][16] = {
	/* cross-hair */
	{ 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000, 0x7e3f,
	  0x0000, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0000 },
	/* hourglass */
	{ 0x0000, 0x7ffe, 0x6006, 0x300c, 0x1818, 0x0c30, 0x0660, 0x03c0,
	  0x0660, 0x0c30, 0x1998, 0x33cc, 0x67e6, 0x7ffe, 0x0000, 0x0000 },
	/* arrow */
	{ 0x0000, 0x4000, 0x6000, 0x7000, 0x7800, 0x7c00, 0x7e00, 0x7f00,
	  0x7f80, 0x78c0, 0x7c00, 0x4600, 0x0600, 0x0300, 0x0300, 0x0180 },
	/* hand */
	{ 0x1e00, 0x1200, 0x1200, 0x1200, 0x1200, 0x13ff, 0x1249, 0x1249,
	  0xf249, 0x9001, 0x9001, 0x9001, 0x8001, 0x8001, 0x8001, 0xffff },
	/* cross-hair zak256 - chrilith palmos */
/*
	{ 0x0080, 0x0080, 0x02a0, 0x01c0, 0x0080, 0x1004, 0x0808, 0x7c1f,
	  0x0808, 0x1004, 0x0080, 0x01c0, 0x02a0, 0x0080, 0x0080, 0x0000 },
*/
	{ 0x0080, 0x02a0, 0x01c0, 0x0080, 0x0000, 0x2002, 0x1004, 0x780f,
	  0x1004, 0x2002, 0x0000, 0x0080, 0x01c0, 0x02a0, 0x0080, 0x0000 },
};

static const byte default_cursor_hotspots[10] = {
	8, 7,   8, 7,   1, 1,   5, 0,
	8, 7, //zak256
};


static inline uint colorWeight(int red, int green, int blue) {
	return 3 * red * red + 6 * green * green + 2 * blue * blue;
}

void ScummEngine::getGraphicsPerformance() {
	int i;

	for (i = 10; i != 0; i--) {
		initScreens(0, 0, _screenWidth, _screenHeight);
	}

	if (VAR_PERFORMANCE_1 != 0xFF)	// Variable is reserved for game scripts in earlier games
		VAR(VAR_PERFORMANCE_1) = 0;

	for (i = 10; i != 0; i--) {
		virtscr[0].setDirtyRange(0, _screenHeight);	//ender
		drawDirtyScreenParts();
	}

	if (VAR_PERFORMANCE_2 != 0xFF)	// Variable is reserved for game scripts in earlier games
		VAR(VAR_PERFORMANCE_2) = 0;

	if (_version >= 7)
		initScreens(0, 0, _screenWidth, _screenHeight);
	else
		initScreens(0, 16, _screenWidth, 144);
}

void ScummEngine::initScreens(int a, int b, int w, int h) {
	int i;

	for (i = 0; i < 3; i++) {
		nukeResource(rtBuffer, i + 1);
		nukeResource(rtBuffer, i + 5);
	}

	if (!getResourceAddress(rtBuffer, 4)) {
		if (_version >= 7) {
			initVirtScreen(3, 0, (_screenHeight / 2) - 10, _screenWidth, 13, false, false);
		} else {
			initVirtScreen(3, 0, 80, _screenWidth, 13, false, false);
		}
	}
	initVirtScreen(0, 0, b, _screenWidth, h - b, true, true);
	initVirtScreen(1, 0, 0, _screenWidth, b, false, false);
	initVirtScreen(2, 0, h, _screenWidth, _screenHeight - h, false, false);

	_screenB = b;
	_screenH = h;

}

void ScummEngine::initVirtScreen(int slot, int number, int top, int width, int height, bool twobufs,
													 bool scrollable) {
	VirtScreen *vs = &virtscr[slot];
	int size;

	assert(height >= 0);
	assert(slot >= 0 && slot < 4);

	if (_version >= 7) {
		if (slot == 0 && (_roomHeight != 0))
			height = _roomHeight;
	}

	vs->number = slot;
	vs->width = _screenWidth;
	vs->topline = top;
	vs->height = height;
	vs->alloctwobuffers = twobufs;
	vs->scrollable = scrollable;
	vs->xstart = 0;
	size = vs->width * vs->height;
	vs->size = size;
	vs->backBuf = NULL;

	if (vs->scrollable) {
		if (_version >= 7) {
			size += _screenWidth * 8;
		} else {
			size += _screenWidth * 4;
		}
	}

	createResource(rtBuffer, slot + 1, size);
	vs->screenPtr = getResourceAddress(rtBuffer, slot + 1);
	memset(vs->screenPtr, 0, size);			// reset background

	if (twobufs) {
		createResource(rtBuffer, slot + 5, size);
	}

	if (slot != 3) {
		virtscr[slot].setDirtyRange(0, height);
	}
}

VirtScreen *ScummEngine::findVirtScreen(int y) {
	VirtScreen *vs = virtscr;
	int i;

	for (i = 0; i < 3; i++, vs++) {
		if (y >= vs->topline && y < vs->topline + vs->height) {
			return vs;
		}
	}
	return NULL;
}

void ScummEngine::updateDirtyRect(int virt, int left, int right, int top, int bottom, int dirtybit) {
	VirtScreen *vs = &virtscr[virt];
	int lp, rp;

	if (left > right || top > bottom)
		return;
	if (top > vs->height || bottom < 0)
		return;

	if (top < 0)
		top = 0;
	if (bottom > vs->height)
		bottom = vs->height;

	if (virt == 0 && dirtybit) {
		lp = (left >> 3) + _screenStartStrip;
		if (lp < 0)
			lp = 0;
		if (_version >= 7) {
#ifdef V7_SMOOTH_SCROLLING_HACK
			rp = (right + vs->xstart) >> 3;
#else
			rp = (right >> 3) + _screenStartStrip;
#endif
			if (rp > 409)
				rp = 409;
		} else {
			rp = (right >> 3) + _screenStartStrip;
			if (rp >= 200)
				rp = 200;
		}
		for (; lp <= rp; lp++)
			setGfxUsageBit(lp, dirtybit);
	}

	// The following code used to be in the seperate method setVirtscreenDirty
	lp = left >> 3;
	rp = right >> 3;

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

void ScummEngine::drawDirtyScreenParts() {
	int i;
	VirtScreen *vs;
	byte *src;

	updateDirtyScreen(2);
	if (_version <= 3)
		updateDirtyScreen(1);

	if (camera._last.x == camera._cur.x && (camera._last.y == camera._cur.y || !(_features & GF_NEW_CAMERA))) {
		updateDirtyScreen(0);
	} else {
		vs = &virtscr[0];

		src = vs->screenPtr + vs->xstart + _screenTop * _screenWidth;
		_system->copy_rect(src, _screenWidth, 0, vs->topline, _screenWidth, vs->height - _screenTop);

		for (i = 0; i < gdi._numStrips; i++) {
			vs->tdirty[i] = vs->height;
			vs->bdirty[i] = 0;
		}
	}

	/* Handle shaking */
	if (_shakeEnabled) {
		_shakeFrame = (_shakeFrame + 1) & (NUM_SHAKE_POSITIONS - 1);
		_system->set_shake_pos(shake_positions[_shakeFrame]);
	} else if (!_shakeEnabled &&_shakeFrame != 0) {
		_shakeFrame = 0;
		_system->set_shake_pos(shake_positions[_shakeFrame]);
	}
}

void ScummEngine::updateDirtyScreen(int slot) {
	gdi.updateDirtyScreen(&virtscr[slot]);
}

/**
 * Blit the data from the given VirtScreen to the display. If the camera moved,
 * a full blit is done, otherwise only the visible dirty areas are updated.
 */
void Gdi::updateDirtyScreen(VirtScreen *vs) {
	if (vs->height == 0)
		return;

	if (_vm->_features & GF_NEW_CAMERA && (_vm->camera._cur.y != _vm->camera._last.y)) {
		drawStripToScreen(vs, 0, _numStrips << 3, 0, vs->height);
	} else {
		int i;
		int start, w, top, bottom;

		w = 8;
		start = 0;

		for (i = 0; i < _numStrips; i++) {
			bottom = vs->bdirty[i];

			if (bottom) {
				top = vs->tdirty[i];
				vs->tdirty[i] = vs->height;
				vs->bdirty[i] = 0;
				if (i != (_numStrips - 1) && vs->bdirty[i + 1] == bottom && vs->tdirty[i + 1] == top) {
					// Simple optimizations: if two or more neighbouring strips form one bigger rectangle,
					// blit them all at once.
					w += 8;
					continue;
				}
				// handle vertically scrolling rooms
				if (_vm->_features & GF_NEW_CAMERA)
					drawStripToScreen(vs, start * 8, w, 0, vs->height);
				else
					drawStripToScreen(vs, start * 8, w, top, bottom);
				w = 8;
			}
			start = i + 1;
		}
	}
}

/**
 * Blit the specified rectangle from the given virtual screen to the display.
 */
void Gdi::drawStripToScreen(VirtScreen *vs, int x, int w, int t, int b) {
	byte *ptr;
	int height;

	if (b <= t)
		return;

	if (t > vs->height)
		t = 0;

	if (b > vs->height)
		b = vs->height;

	height = b - t;
	if (height > _vm->_screenHeight)
		height = _vm->_screenHeight;

	// Normally, _vm->_screenTop should always be >= 0, but for some old save games
	// it is not, hence we check & correct it here.
	if (_vm->_screenTop < 0)
		_vm->_screenTop = 0;

	ptr = vs->screenPtr + (x + vs->xstart) + (_vm->_screenTop + t) * _vm->_screenWidth;
	_vm->_system->copy_rect(ptr, _vm->_screenWidth, x, vs->topline + t, w, height);
}

void Gdi::clearCharsetMask() {
	memset(_vm->getResourceAddress(rtBuffer, 9), 0, _imgBufOffs[1]);
	_mask.top = _mask.left = 32767;
	_mask.right = _mask.bottom = 0;
}

/**
 * Reset the background behind an actor or blast object.
 */
void Gdi::resetBackground(int top, int bottom, int strip) {
	VirtScreen *vs = &_vm->virtscr[0];
	byte *backbuff_ptr, *bgbak_ptr;
	int offs, numLinesToProcess;

	assert(0 <= strip && strip < _numStrips);

	if (top < vs->tdirty[strip])
		vs->tdirty[strip] = top;

	if (bottom > vs->bdirty[strip])
		vs->bdirty[strip] = bottom;

	offs = (top * _numStrips + _vm->_screenStartStrip + strip) << 3;
	byte *mask_ptr = _vm->getMaskBuffer(strip * 8, top, 0);
	bgbak_ptr = _vm->getResourceAddress(rtBuffer, 5) + offs;
	backbuff_ptr = vs->screenPtr + offs;

	numLinesToProcess = bottom - top;
	if (numLinesToProcess) {
		if ((_vm->_features & GF_NEW_OPCODES) || (_vm->VAR(_vm->VAR_CURRENT_LIGHTS) & LIGHTMODE_screen)) {
			if (_vm->hasCharsetMask(strip << 3, top, (strip + 1) << 3, bottom))
				draw8ColWithMasking(backbuff_ptr, bgbak_ptr, numLinesToProcess, mask_ptr);
			else
				draw8Col(backbuff_ptr, bgbak_ptr, numLinesToProcess);
		} else {
			clear8Col(backbuff_ptr, numLinesToProcess);
		}
	}
}

void ScummEngine::blit(byte *dst, const byte *src, int w, int h) {
	assert(h > 0);
	assert(src != NULL);
	assert(dst != NULL);

	if (w==_screenWidth)
		memcpy (dst, src, w*h);
	else
	{
		do {
			memcpy(dst, src, w);
			dst += _screenWidth;
			src += _screenWidth;
		} while (--h);
	}
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
	else if (x >= vs->width)
		return;

	if (x2 < 0)
		return;
	else if (x2 > vs->width)
		x2 = vs->width;

	if (y < 0)
		y = 0;
	else if (y > vs->height)
		return;

	if (y2 < 0)
		return;
	else if (y2 > vs->height)
		y2 = vs->height;
	
	updateDirtyRect(vs->number, x, x2, y, y2, 0);

	backbuff = vs->screenPtr + vs->xstart + y * _screenWidth + x;

	width = x2 - x;
	height = y2 - y;
	if (color == -1) {
		if (vs->number != 0)
			error("can only copy bg to main window");
		bgbuff = getResourceAddress(rtBuffer, vs->number + 5) + vs->xstart + y * _screenWidth + x;
		blit(backbuff, bgbuff, width, height);
	} else {
		while (height--) {
			memset(backbuff, color, width);
			backbuff += _screenWidth;
		}
	}
}

#pragma mark -

void ScummEngine::initBGBuffers(int height) {
	const byte *ptr;
	int size, itemsize, i;
	byte *room;

	if (_version >= 7) {
		initVirtScreen(0, 0, virtscr[0].topline, _screenWidth, height, 1, 1);
	}

	room = getResourceAddress(rtRoom, _roomResource);
	if (_version <= 3) {
		gdi._numZBuffer = 2;
	} else if (_features & GF_SMALL_HEADER) {
		int off;
		ptr = findResourceData(MKID('SMAP'), room);
		gdi._numZBuffer = 0;

		if (_gameId == GID_MONKEY_EGA || _gameId == GID_PASS)
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
	memset(createResource(rtBuffer, 9, size), 0, size);

	for (i = 0; i < (int)ARRAYSIZE(gdi._imgBufOffs); i++) {
		if (i < gdi._numZBuffer)
			gdi._imgBufOffs[i] = i * itemsize;
		else
			gdi._imgBufOffs[i] = (gdi._numZBuffer - 1) * itemsize;
	}
}

void ScummEngine::drawFlashlight() {
	int i, j, offset, x, y;

	// Remove the flash light first if it was previously drawn
	if (_flashlight.isDrawn) {
		updateDirtyRect(0, _flashlight.x, _flashlight.x + _flashlight.w,
										_flashlight.y, _flashlight.y + _flashlight.h, USAGE_BIT_DIRTY);
		
		if (_flashlight.buffer) {
			i = _flashlight.h;
			do {
				memset(_flashlight.buffer, 0, _flashlight.w);
				_flashlight.buffer += _screenWidth;
			} while (--i);
		}
		_flashlight.isDrawn = false;
	}

	if (_flashlight.xStrips == 0 || _flashlight.yStrips == 0)
		return;

	// Calculate the area of the flashlight
	if (_gameId == GID_ZAK256 || _version <= 2) {
		x = _mouse.x + virtscr[0].xstart;
		y = _mouse.y - virtscr[0].topline;
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
	else if (_flashlight.y + _flashlight.h> virtscr[0].height)
		_flashlight.y = virtscr[0].height - _flashlight.h;

	// Redraw any actors "under" the flashlight
	for (i = _flashlight.x / 8; i < (_flashlight.x + _flashlight.w) / 8; i++) {
		assert(0 <= i && i < gdi._numStrips);
		setGfxUsageBit(_screenStartStrip + i, USAGE_BIT_DIRTY);
		virtscr[0].tdirty[i] = 0;
		virtscr[0].bdirty[i] = virtscr[0].height;
	}

	byte *bgbak;
	offset = _flashlight.y * _screenWidth + virtscr[0].xstart + _flashlight.x;
	_flashlight.buffer = virtscr[0].screenPtr + offset;
	bgbak = getResourceAddress(rtBuffer, 5) + offset;

	blit(_flashlight.buffer, bgbak, _flashlight.w, _flashlight.h);

	// Round the corners. To do so, we simply hard-code a set of nicely
	// rounded corners.
	int corner_data[] = { 8, 6, 4, 3, 2, 2, 1, 1 };
	int minrow = 0;
	int maxcol = _flashlight.w - 1;
	int maxrow = (_flashlight.h - 1) * _screenWidth;

	for (i = 0; i < 8; i++, minrow += _screenWidth, maxrow -= _screenWidth) {
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

/**
 * Redraw background as needed, i.e. the left/right sides if scrolling took place etc.
 * Note that this only updated the virtual screen, not the actual display.
 */
void ScummEngine::redrawBGAreas() {
	int i;
	int val;
	int diff;

	if (!(_features & GF_NEW_CAMERA))
		if (camera._cur.x != camera._last.x && _charset->_hasMask && _version > 3)
			stopTalk();

	val = 0;

	// Redraw parts of the background which are marked as dirty.
	if (!_fullRedraw && _BgNeedsRedraw) {
		for (i = 0; i != gdi._numStrips; i++) {
			if (testGfxUsageBit(_screenStartStrip + i, USAGE_BIT_DIRTY)) {
				redrawBGStrip(i, 1);
			}
		}
	}

	if (_features & GF_NEW_CAMERA) {
		diff = (camera._cur.x >> 3) - (camera._last.x >> 3);
		if (_fullRedraw == 0 && diff == 1) {
			val = 2;
			redrawBGStrip(gdi._numStrips - 1, 1);
		} else if (_fullRedraw == 0 && diff == -1) {
			val = 1;
			redrawBGStrip(0, 1);
		} else if (_fullRedraw != 0 || diff != 0) {
			_BgNeedsRedraw = false;
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
			_BgNeedsRedraw = false;
			_flashlight.isDrawn = false;
			redrawBGStrip(0, gdi._numStrips);
		}
	}

	drawRoomObjects(val);
	_BgNeedsRedraw = false;
}

void ScummEngine::redrawBGStrip(int start, int num) {
	int s = _screenStartStrip + start;

	assert(s >= 0 && (size_t) s < sizeof(gfxUsageBits) / (3 * sizeof(gfxUsageBits[0])));

	for (int i = 0; i < num; i++)
		setGfxUsageBit(s + i, USAGE_BIT_DIRTY);

	if (_version == 1) {
		gdi._C64ObjectMode = false;
	}
	gdi.drawBitmap(getResourceAddress(rtRoom, _roomResource) + _IM00_offs,
	               &virtscr[0], s, 0, _roomWidth, virtscr[0].height, s, num, 0, _roomStrips);
}

void ScummEngine::restoreCharsetBg() {
	if (_charset->_hasMask) {
		restoreBG(gdi._mask);
		_charset->_hasMask = false;
		gdi._mask.top = gdi._mask.left = 32767;
		gdi._mask.right = gdi._mask.bottom = 0;
		_charset->_str.left = -1;
		_charset->_left = -1;
	}

	_charset->_nextLeft = _string[0].xpos;
	_charset->_nextTop = _string[0].ypos;
}

void ScummEngine::restoreBG(Common::Rect rect, byte backColor) {
	VirtScreen *vs;
	int topline, height, width;
	byte *backbuff, *bgbak;
	bool lightsOn;

	if (rect.top < 0)
		rect.top = 0;
	if (rect.left >= rect.right || rect.top >= rect.bottom)
		return;

	if ((vs = findVirtScreen(rect.top)) == NULL)
		return;

	topline = vs->topline;
	height = topline + vs->height;

	if (rect.left < 0)
		rect.left = 0;
	if (rect.right < 0)
		rect.right = 0;
	if (rect.left > _screenWidth)
		return;
	if (rect.right > _screenWidth)
		rect.right = _screenWidth;
	if (rect.bottom >= height)
		rect.bottom = height;

	updateDirtyRect(vs->number, rect.left, rect.right, rect.top - topline, rect.bottom - topline, USAGE_BIT_RESTORED);

	int offset = (rect.top - topline) * _screenWidth + vs->xstart + rect.left;
	backbuff = vs->screenPtr + offset;
	bgbak = getResourceAddress(rtBuffer, vs->number + 5) + offset;

	height = rect.height();
	width = rect.width();

	// Check whether lights are turned on or not
	lightsOn = (_features & GF_NEW_OPCODES) || (vs->number != 0) || (VAR(VAR_CURRENT_LIGHTS) & LIGHTMODE_screen);

	if (vs->alloctwobuffers && _currentRoom != 0 && lightsOn ) {
		blit(backbuff, bgbak, width, height);
		if (vs->number == 0 && _charset->_hasMask && height) {
			byte *mask;
			// Note: At first sight it may look as if this could
			// be optimized to (rect.right - rect.left) >> 3 and
			// thus to width >> 3, but that's not the case since
			// we are dealing with integer math here.
			int mask_width = (rect.right >> 3) - (rect.left >> 3);

			if (rect.right & 0x07)
				mask_width++;

			mask = getMaskBuffer(rect.left, rect.top, 0);
			if (vs->number == 0)
				mask += vs->topline * gdi._numStrips;

			do {
				memset(mask, 0, mask_width);
				mask += gdi._numStrips;
			} while (--height);
		}
	} else {
		while (height--) {
			memset(backbuff, backColor, width);
			backbuff += _screenWidth;
		}
	}
}

bool ScummEngine::hasCharsetMask(int left, int top, int right, int bottom) {
	Common::Rect rect(left, top, right, bottom);
	
	return _charset->_hasMask && rect.intersects(gdi._mask);
}

byte *ScummEngine::getMaskBuffer(int x, int y, int z) {
	return getResourceAddress(rtBuffer, 9)
			+ _screenStartStrip + (x / 8) + y * gdi._numStrips + gdi._imgBufOffs[z];
}


#pragma mark -
#pragma mark --- Image drawing ---
#pragma mark -

/**
 * Draw a bitmap onto a virtual screen. This is main drawing method for room backgrounds
 * and objects, used throughout all SCUMM versions.
 */
void Gdi::drawBitmap(const byte *ptr, VirtScreen *vs, int x, int y, const int width, const int height,
                     int stripnr, int numstrip, byte flag, StripTable *table) {
	assert(ptr);
	assert(height > 0);
	byte *backbuff_ptr, *bgbak_ptr;
	const byte *smap_ptr;
	const byte *z_plane_ptr;
	byte *mask_ptr;

	int i;
	const byte *zplane_list[9];

	int bottom;
	int numzbuf;
	int sx;
	bool lightsOn;
	bool useOrDecompress = false;

	// Check whether lights are turned on or not
	lightsOn = (_vm->_features & GF_NEW_OPCODES) || (vs->number != 0) || (_vm->VAR(_vm->VAR_CURRENT_LIGHTS) & LIGHTMODE_screen);

	CHECK_HEAP;
	if (_vm->_features & GF_SMALL_HEADER)
		smap_ptr = ptr;
	else if (_vm->_version == 8)
		smap_ptr = ptr;
	else
		smap_ptr = findResource(MKID('SMAP'), ptr);

	assert(smap_ptr);

	zplane_list[0] = smap_ptr;

	if (_zbufferDisabled)
		numzbuf = 0;
	else if (_numZBuffer <= 1 || (_vm->_version <= 2))
		numzbuf = _numZBuffer;
	else {
		numzbuf = _numZBuffer;
		assert(numzbuf <= ARRAYSIZE(zplane_list));
		
		if (_vm->_features & GF_SMALL_HEADER) {
			if (_vm->_features & GF_16COLOR)
				zplane_list[1] = smap_ptr + READ_LE_UINT16(smap_ptr);
			else
				zplane_list[1] = smap_ptr + READ_LE_UINT32(smap_ptr);
			if (_vm->_features & GF_OLD256) {
				if (0 == READ_LE_UINT32(zplane_list[1]))
					zplane_list[1] = 0;
			}
			for (i = 2; i < numzbuf; i++) {
				zplane_list[i] = zplane_list[i-1] + READ_LE_UINT16(zplane_list[i-1]);
			}
		} else if (_vm->_version == 8) {
			// Find the OFFS chunk of the ZPLN chunk
			const byte *zplnOffsChunkStart = smap_ptr + READ_BE_UINT32(smap_ptr + 12) + 24;
			
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
				zplane_list[i] = findResource(zplane_tags[i], ptr);
			}
		}
	}
	
	if (_vm->_version == 8) {	
		// A small hack to skip to the BSTR->WRAP->OFFS chunk. Note: order matters, we do this
		// *after* the Z buffer code because that assumes' the orginal value of smap_ptr. 
		smap_ptr += 24;
	}

	bottom = y + height;
	if (bottom > vs->height) {
		warning("Gdi::drawBitmap, strip drawn to %d below window bottom %d", bottom, vs->height);
	}

	_vertStripNextInc = height * _vm->_screenWidth - 1;

	sx = x;
	if (vs->scrollable)
		sx -= vs->xstart >> 3;

	//
	// Since V3, all graphics data was encoded in strips, which is very efficient
	// for redrawing only parts of the screen. However, V2 is different: here
	// the whole graphics are encoded as one big chunk. That makes it rather
	// dificult to draw only parts of a room/object. We handle the V2 graphics
	// differently from all other (newer) graphic formats for this reason.
	//
	if (_vm->_version == 2) {
		
		if (vs->alloctwobuffers)
			bgbak_ptr = _vm->getResourceAddress(rtBuffer, vs->number + 5) + (y * _numStrips + x) * 8;
		else
			bgbak_ptr = vs->screenPtr + (y * _numStrips + x) * 8;

		mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + (y * _numStrips + x) + _imgBufOffs[1];

		const int left = (stripnr << 3);
		const int right = left + (numstrip << 3);
		byte *dst = bgbak_ptr;
		const byte *src;
		byte color, data = 0;
		int run;
		bool dither = false;
		byte dither_table[128];
		byte *ptr_dither_table;
		memset(dither_table, 0, sizeof(dither_table));
		int theX, theY, maxX;
		
		if (table) {
			run = table->run[stripnr];
			color = table->color[stripnr];
			src = smap_ptr + table->offsets[stripnr];
			theX = left;
			maxX = right;
		} else {
			run = 1;
			color = 0;
			src = smap_ptr;
			theX = 0;
			maxX = width;
		}
		
		// Draw image data. To do this, we decode the full RLE graphics data,
		// but only draw those parts we actually want to display.
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
					color = _vm->_roomPalette[data & 0x0f];
					if (run == 0) {
						run = *src++;
					}
				}
				if (!dither) {
					*ptr_dither_table = color;
				}
				if (left <= theX && theX < right) {
					*dst = *ptr_dither_table++;
					dst += _vm->_screenWidth;
				}
			}
			if (left <= theX && theX < right) {
				dst -= _vertStripNextInc;
			}
		}


		// Draw mask (zplane) data
		theY = 0;

		if (table) {
			src = smap_ptr + table->zoffsets[stripnr];
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

	while (numstrip--) {
		CHECK_HEAP;

		if (sx < 0)
			goto next_iter;

		if (sx >= _numStrips)
			return;

		if (y < vs->tdirty[sx])
			vs->tdirty[sx] = y;

		if (bottom > vs->bdirty[sx])
			vs->bdirty[sx] = bottom;

		backbuff_ptr = vs->screenPtr + (y * _numStrips + x) * 8;
		if (vs->alloctwobuffers)
			bgbak_ptr = _vm->getResourceAddress(rtBuffer, vs->number + 5) + (y * _numStrips + x) * 8;
		else
			bgbak_ptr = backbuff_ptr;

		if (_vm->_version == 1) {
			if (_C64ObjectMode)
				drawStripC64Object(bgbak_ptr, stripnr, width, height);
			else
				drawStripC64Background(bgbak_ptr, stripnr, height);
		} else if (_vm->_version > 2) {
			if (_vm->_features & GF_16COLOR) {
				decodeStripEGA(bgbak_ptr, smap_ptr + READ_LE_UINT16(smap_ptr + stripnr * 2 + 2), height);
			} else if (_vm->_features & GF_SMALL_HEADER) {
				useOrDecompress = decompressBitmap(bgbak_ptr, smap_ptr + READ_LE_UINT32(smap_ptr + stripnr * 4 + 4), height);
			} else {
				useOrDecompress = decompressBitmap(bgbak_ptr, smap_ptr + READ_LE_UINT32(smap_ptr + stripnr * 4 + 8), height);
			}
		}

		mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + (y * _numStrips + x);

		CHECK_HEAP;
		if (vs->alloctwobuffers) {
			if (_vm->hasCharsetMask(sx << 3, y, (sx + 1) << 3, bottom)) {
				if (flag & dbClear || !lightsOn)
					clear8ColWithMasking(backbuff_ptr, height, mask_ptr);
				else
					draw8ColWithMasking(backbuff_ptr, bgbak_ptr, height, mask_ptr);
			} else {
				if (flag & dbClear || !lightsOn)
					clear8Col(backbuff_ptr, height);
				else
					draw8Col(backbuff_ptr, bgbak_ptr, height);
			}
		}
		CHECK_HEAP;

		if (_vm->_version == 1) {
			mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + y * _numStrips + x + _imgBufOffs[1];
			drawStripC64Mask(mask_ptr, stripnr, width, height);
		} else if (_vm->_version == 2) {
			// Do nothing here for V2 games - zplane was handled already.
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
				mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + y * _numStrips + x + _imgBufOffs[i];
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

				mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + y * _numStrips + x + _imgBufOffs[i];

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
		
#if 0
		// HACK: blit mask(s) onto normal screen. Useful to debug masking 
		for (i = 0; i < numzbuf; i++) {
			mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + x + _imgBufOffs[i] + (y * _numStrips);
			byte *dst = backbuff_ptr;
			byte *dst2 = bgbak_ptr;
			for (int h = 0; h < height; h++) {
				int maskbits = *mask_ptr;
				for (int j = 0; j < 8; j++) {
					if (maskbits & 0x80)
						dst[j] = dst2[j] = 12+i;
					maskbits <<= 1;
				}
				dst += _vm->_screenWidth;
				dst2 += _vm->_screenWidth;
				mask_ptr += _numStrips;
			}
		}
#endif

next_iter:
		CHECK_HEAP;
		x++;
		sx++;
		stripnr++;
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
StripTable *Gdi::generateStripTable(const byte *src, int width, int height, StripTable *table) {

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
			assert(x/8 < 160);
			table->run[x/8] = run;
			table->color[x/8] = color;
			table->offsets[x/8] = src - bitmapStart;
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

void Gdi::drawStripC64Background(byte *dst, int stripnr, int height) {
	int charIdx;
	height >>= 3;
	for (int y = 0; y < height; y++) {
		_C64Colors[3] = (_C64ColorMap[y + stripnr * height] & 7);
		// Check for room color change in V1 zak
		if (_vm->_roomPalette[0] == 255) {
			_C64Colors[2] = _vm->_roomPalette[2];
			_C64Colors[1] = _vm->_roomPalette[1];
		}

		charIdx = _C64PicMap[y + stripnr * height] * 8;
		for (int i = 0; i < 8; i++) {
			byte c = _C64CharMap[charIdx + i];
			dst[0] = dst[1] = _C64Colors[(c >> 6) & 3];
			dst[2] = dst[3] = _C64Colors[(c >> 4) & 3];
			dst[4] = dst[5] = _C64Colors[(c >> 2) & 3];
			dst[6] = dst[7] = _C64Colors[(c >> 0) & 3];
			dst += _vm->_screenWidth;
		}
	}
}

void Gdi::drawStripC64Object(byte *dst, int stripnr, int width, int height) {
	int charIdx;
	height >>= 3;
	width >>= 3;
	for (int y = 0; y < height; y++) {
		_C64Colors[3] = (_C64ObjectMap[(y + height) * width + stripnr] & 7);
		charIdx = _C64ObjectMap[y * width + stripnr] * 8;
		for (int i = 0; i < 8; i++) {
			byte c = _C64CharMap[charIdx + i];
			dst[0] = dst[1] = _C64Colors[(c >> 6) & 3];
			dst[2] = dst[3] = _C64Colors[(c >> 4) & 3];
			dst[4] = dst[5] = _C64Colors[(c >> 2) & 3];
			dst[6] = dst[7] = _C64Colors[(c >> 0) & 3];
			dst += _vm->_screenWidth;
		}
	}
}

void Gdi::drawStripC64Mask(byte *dst, int stripnr, int width, int height) {
	int maskIdx;
	height >>= 3;
	width >>= 3;
	for (int y = 0; y < height; y++) {
		if (_C64ObjectMode)
			maskIdx = _C64ObjectMap[(y + 2 * height) * width + stripnr] * 8;
		else
			maskIdx = _C64MaskMap[y + stripnr * height] * 8;
		for (int i = 0; i < 8; i++) {
			byte c = _C64MaskChar[maskIdx + i];

			// V1/C64 masks are inverted compared to what ScummVM expects
			*dst = c ^ 0xFF;
			dst += _numStrips;
		}
	}
}

void Gdi::decodeC64Gfx(const byte *src, byte *dst, int size) {
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

void Gdi::decodeStripEGA(byte *dst, const byte *src, int height) {
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
					*(dst + y * _vm->_screenWidth + x) = (z&1) ? _vm->_roomPalette[((color & 0xf) + _palette_mod)] : _vm->_roomPalette[((color >> 4) + _palette_mod)];

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
					*(dst + y * _vm->_screenWidth + x) = *(dst + y * _vm->_screenWidth + x - 1);

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
				*(dst + y * _vm->_screenWidth + x) = _vm->_roomPalette[(color & 0xf) + _palette_mod];

				y++;
				if (y >= height) {
					y = 0;
					x++;
				}
			}
		}
	}
}

bool Gdi::decompressBitmap(byte *bgbak_ptr, const byte *src, int numLinesToProcess) {
	assert(numLinesToProcess);

	byte code = *src++;

	if (_vm->_features & GF_AMIGA)
		_palette_mod = 16;
	else
		_palette_mod = 0;

	bool useOrDecompress = false;
	_decomp_shr = code % 10;
	_decomp_mask = 0xFF >> (8 - _decomp_shr);
	
	switch (code) {
	case 1:
		unkDecode7(bgbak_ptr, src, numLinesToProcess);
		break;

	case 2:
		unkDecode8(bgbak_ptr, src, numLinesToProcess);       /* Ender - Zak256/Indy256 */
		break;

	case 3:
		unkDecode9(bgbak_ptr, src, numLinesToProcess);       /* Ender - Zak256/Indy256 */
		break;

	case 4:
		unkDecode10(bgbak_ptr, src, numLinesToProcess);      /* Ender - Zak256/Indy256 */
		break;

	case 7:
		unkDecode11(bgbak_ptr, src, numLinesToProcess);      /* Ender - Zak256/Indy256 */
		break;
	// FIXME implement these codecs...
	// 8/9 used in 3do version of puttputt joins the parade maybe others
	case 8:
	case 9:
		error("decompressBitmap: Graphics codec %d not yet supported\n", code);
	// used in amiga version of Monkey Island
	case 10:
		decodeStripEGA(bgbak_ptr, src, numLinesToProcess);
		break;

	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
		unkDecodeC(bgbak_ptr, src, numLinesToProcess);
		break;

	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
		unkDecodeB(bgbak_ptr, src, numLinesToProcess);
		break;

	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
		useOrDecompress = true;
		unkDecodeC_trans(bgbak_ptr, src, numLinesToProcess);
		break;

	case 44:
	case 45:
	case 46:
	case 47:
	case 48:
		useOrDecompress = true;
		unkDecodeB_trans(bgbak_ptr, src, numLinesToProcess);
		break;

	case 64:
	case 65:
	case 66:
	case 67:
	case 68:
	case 104:
	case 105:
	case 106:
	case 107:
	case 108:
		unkDecodeA(bgbak_ptr, src, numLinesToProcess);
		break;

	case 84:
	case 85:
	case 86:
	case 87:
	case 88:
	case 124:
	case 125:
	case 126:
	case 127:
	case 128:
		useOrDecompress = true;
		unkDecodeA_trans(bgbak_ptr, src, numLinesToProcess);
		break;

	default:
		error("Gdi::decompressBitmap: default case %d", code);
	}
	
	return useOrDecompress;
}

void Gdi::draw8ColWithMasking(byte *dst, const byte *src, int height, byte *mask) {
	byte maskbits;

	do {
		maskbits = *mask;
		if (maskbits) {
			if (!(maskbits & 0x80))
				dst[0] = src[0];
			if (!(maskbits & 0x40))
				dst[1] = src[1];
			if (!(maskbits & 0x20))
				dst[2] = src[2];
			if (!(maskbits & 0x10))
				dst[3] = src[3];
			if (!(maskbits & 0x08))
				dst[4] = src[4];
			if (!(maskbits & 0x04))
				dst[5] = src[5];
			if (!(maskbits & 0x02))
				dst[6] = src[6];
			if (!(maskbits & 0x01))
				dst[7] = src[7];
		} else {
#if defined(SCUMM_NEED_ALIGNMENT)
			memcpy(dst, src, 8);
#else
			((uint32 *)dst)[0] = ((const uint32 *)src)[0];
			((uint32 *)dst)[1] = ((const uint32 *)src)[1];
#endif
		}
		src += _vm->_screenWidth;
		dst += _vm->_screenWidth;
		mask += _numStrips;
	} while (--height);
}

void Gdi::clear8ColWithMasking(byte *dst, int height, byte *mask) {
	byte maskbits;

	do {
		maskbits = *mask;
		if (maskbits) {
			if (!(maskbits & 0x80))
				dst[0] = 0;
			if (!(maskbits & 0x40))
				dst[1] = 0;
			if (!(maskbits & 0x20))
				dst[2] = 0;
			if (!(maskbits & 0x10))
				dst[3] = 0;
			if (!(maskbits & 0x08))
				dst[4] = 0;
			if (!(maskbits & 0x04))
				dst[5] = 0;
			if (!(maskbits & 0x02))
				dst[6] = 0;
			if (!(maskbits & 0x01))
				dst[7] = 0;
		} else {
#if defined(SCUMM_NEED_ALIGNMENT)
			memset(dst, 0, 8);
#else
			((uint32 *)dst)[0] = 0;
			((uint32 *)dst)[1] = 0;
#endif
		}
		dst += _vm->_screenWidth;
		mask += _numStrips;
	} while (--height);
}

void Gdi::draw8Col(byte *dst, const byte *src, int height) {
	do {
#if defined(SCUMM_NEED_ALIGNMENT)
		memcpy(dst, src, 8);
#else
		((uint32 *)dst)[0] = ((const uint32 *)src)[0];
		((uint32 *)dst)[1] = ((const uint32 *)src)[1];
#endif
		dst += _vm->_screenWidth;
		src += _vm->_screenWidth;
	} while (--height);
}
void Gdi::clear8Col(byte *dst, int height)
{
	do {
#if defined(SCUMM_NEED_ALIGNMENT)
		memset(dst, 0, 8);
#else
		((uint32 *)dst)[0] = 0;
		((uint32 *)dst)[1] = 0;
#endif
		dst += _vm->_screenWidth;
	} while (--height);
}

void Gdi::decompressMaskImg(byte *dst, const byte *src, int height) {
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

void Gdi::decompressMaskImgOr(byte *dst, const byte *src, int height) {
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

#define READ_BIT (cl--, bit = bits&1, bits>>=1,bit)
#define FILL_BITS do {												\
										if (cl <= 8) {						\
											bits |= (*src++ << cl);	\
											cl += 8;								\
										}													\
									} while (0)

void Gdi::unkDecodeA(byte *dst, const byte *src, int height) {
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	byte incm, reps;

	do {
		int x = 8;
		do {
			FILL_BITS;
			*dst++ = (_vm->_features & GF_SMALL_HEADER) ? _vm->_roomPalette[color + _palette_mod] : color + _palette_mod;

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
					color = (byte)((color+incm)&0xFF);
				} else {
					FILL_BITS;
					reps = bits & 0xFF;
					do {
						if (!--x) {
							x = 8;
							dst += _vm->_screenWidth - 8;
							if (!--height)
								return;
						}
						*dst++ = color + _palette_mod;
					} while (--reps);
					bits >>= 8;
					bits |= (*src++) << (cl - 8);
					goto againPos;
				}
			}
		} while (--x);
		dst += _vm->_screenWidth - 8;
	} while (--height);
}

void Gdi::unkDecodeA_trans(byte *dst, const byte *src, int height) {
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	byte incm, reps;

	do {
		int x = 8;
		do {
			FILL_BITS;
			if (color != _transparentColor)
				*dst = (_vm->_features & GF_SMALL_HEADER) ? _vm->_roomPalette[color + _palette_mod] : color + _palette_mod;
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
							dst += _vm->_screenWidth - 8;
							if (!--height)
								return;
						}
						if (color != _transparentColor)
							*dst = color + _palette_mod;
						dst++;
					} while (--reps);
					bits >>= 8;
					bits |= (*src++) << (cl - 8);
					goto againPos;
				}
			}
		} while (--x);
		dst += _vm->_screenWidth - 8;
	} while (--height);
}

void Gdi::unkDecodeB(byte *dst, const byte *src, int height) {
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	int8 inc = -1;

	do {
		int x = 8;
		do {
			FILL_BITS;
			*dst++ = (_vm->_features & GF_SMALL_HEADER) ? _vm->_roomPalette[color + _palette_mod] : color + _palette_mod;
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
		dst += _vm->_screenWidth - 8;
	} while (--height);
}

void Gdi::unkDecodeB_trans(byte *dst, const byte *src, int height) {
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	int8 inc = -1;

	do {
		int x = 8;
		do {
			FILL_BITS;
			if (color != _transparentColor)
				*dst = (_vm->_features & GF_SMALL_HEADER) ? _vm->_roomPalette[color + _palette_mod] : color + _palette_mod;
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
		dst += _vm->_screenWidth - 8;
	} while (--height);
}

void Gdi::unkDecodeC(byte *dst, const byte *src, int height) {
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
			*dst = (_vm->_features & GF_SMALL_HEADER) ? _vm->_roomPalette[color + _palette_mod] : color + _palette_mod;
			dst += _vm->_screenWidth;
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

void Gdi::unkDecodeC_trans(byte *dst, const byte *src, int height) {
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
			if (color != _transparentColor)
				*dst = (_vm->_features & GF_SMALL_HEADER) ? _vm->_roomPalette[color + _palette_mod] : color + _palette_mod;
			dst += _vm->_screenWidth;
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
#define READ_256BIT                        \
		do {                               \
			if ((mask <<= 1) == 256) {     \
				buffer = *src++;           \
				mask = 1;                  \
			}                              \
			bits = ((buffer & mask) != 0); \
		} while (0)

#define NEXT_ROW                           \
		do {                               \
			dst += _vm->_screenWidth;      \
			if (--h == 0) {                \
				if (!--x)                  \
					return;                \
				dst -= _vertStripNextInc;  \
				h = height;                \
			}                              \
		} while (0)

void Gdi::unkDecode7(byte *dst, const byte *src, int height) {
	uint h = height;

	if (_vm->_features & GF_OLD256) {
		int x = 8;
		for (;;) {
			*dst = *src++;
			NEXT_ROW;
		}
		return;
	}

	do {
#if defined(SCUMM_NEED_ALIGNMENT)
		memcpy(dst, src, 8);
#else
		((uint32 *)dst)[0] = ((const uint32 *)src)[0];
		((uint32 *)dst)[1] = ((const uint32 *)src)[1];
#endif
		dst += _vm->_screenWidth;
		src += 8;
	} while (--height);
}

void Gdi::unkDecode8(byte *dst, const byte *src, int height) {
	uint h = height;

	int x = 8;
	for (;;) {
		uint run = (*src++) + 1;
		byte color = *src++;

		do {
			*dst = _vm->_roomPalette[color];
			NEXT_ROW;
		} while (--run);
	}
}

void Gdi::unkDecode9(byte *dst, const byte *src, int height) {
	unsigned char c, bits, color, run;
	int i, j;
	uint buffer = 0, mask = 128;
	int h = height;
	i = j = run = 0;

	int x = 8;
	for (;;) {
		c = 0;
		for (i = 0; i < 4; i++) {
			READ_256BIT;
			c += (bits << i);
		}

		switch (c >> 2) {
		case 0:
			color = 0;
			for (i = 0; i < 4; i++) {
				READ_256BIT;
				color += bits << i;
			}
			for (i = 0; i < ((c & 3) + 2); i++) {
				*dst = (run * 16 + _vm->_roomPalette[color]);
				NEXT_ROW;
			}
			break;

		case 1:
			for (i = 0; i < ((c & 3) + 1); i++) {
				color = 0;
				for (j = 0; j < 4; j++) {
					READ_256BIT;
					color += bits << j;
				}
				*dst = (run * 16 + _vm->_roomPalette[color]);
				NEXT_ROW;
			}
			break;

		case 2:
			run = 0;
			for (i = 0; i < 4; i++) {
				READ_256BIT;
				run += bits << i;
			}
			break;
		}
	}
}

void Gdi::unkDecode10(byte *dst, const byte *src, int height) {
	int i;
	unsigned char local_palette[256], numcolors = *src++;
	uint h = height;

	for (i = 0; i < numcolors; i++)
		local_palette[i] = *src++;

	int x = 8;

	for (;;) {
		byte color = *src++;
		if (color < numcolors) {
			*dst = local_palette[_vm->_roomPalette[color]];
			NEXT_ROW;
		} else {
			uint run = color - numcolors + 1;
			color = *src++;
			do {
				*dst = _vm->_roomPalette[color];
				NEXT_ROW;
			} while (--run);
		}
	}
}


void Gdi::unkDecode11(byte *dst, const byte *src, int height) {
	int bits, i;
	uint buffer = 0, mask = 128;
	unsigned char inc = 1, color = *src++;

	int x = 8;
	do {
		int h = height;
		do {
			*dst = _vm->_roomPalette[color];
			dst += _vm->_screenWidth;
			for (i = 0; i < 3; i++) {
				READ_256BIT;
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
				color = 0;
				inc = 1;
				for (i = 0; i < 8; i++) {
					READ_256BIT;
					color += bits << i;
				}
				break;
			}
		} while (--h);
		dst -= _vertStripNextInc;
	} while (--x);
}

#undef NEXT_ROW
#undef READ_256BIT

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

	if (_screenEffectFlag && effect != 0) {
	
		// Fill screen 0 with black
		
		memset(vs->screenPtr + vs->xstart, 0, vs->size);
	
		// Fade to black with the specified effect, if any.
		switch (effect) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			transitionEffect(effect - 1);
			break;
		case 128:
			unkScreenEffect6();
			break;
		case 129:
			// Just blit screen 0 to the display (i.e. display will be black)
			vs->setDirtyRange(0, vs->height);
			updateDirtyScreen(0);
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

	for (i = 0; i < 16; i++) {
		delta[i] = transitionEffects[a].deltaTable[i];
		j = transitionEffects[a].stripTable[i];
		if (j == 24)
			j = (virtscr[0].height >> 3) - 1;
		tab_2[i] = j;
	}

	bottom = virtscr[0].height >> 3;
	for (j = 0; j < transitionEffects[a].numOfIterations; j++) {
		for (i = 0; i < 4; i++) {
			l = tab_2[i * 4];
			t = tab_2[i * 4 + 1];
			r = tab_2[i * 4 + 2];
			b = tab_2[i * 4 + 3];
			if (t == b) {
				while (l <= r) {
					if (l >= 0 && l < gdi._numStrips && (uint) t < (uint) bottom) {
						virtscr[0].tdirty[l] = t << 3;
						virtscr[0].bdirty[l] = (t + 1) << 3;
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
 				virtscr[0].tdirty[l] = t << 3;
				virtscr[0].bdirty[l] = (b + 1) << 3;
			}
			updateDirtyScreen(0);
		}

		for (i = 0; i < 16; i++)
			tab_2[i] += delta[i];

		// Draw the current state to the screen and wait half a sec so the user
		// can watch the effect taking place.
		_system->update_screen();
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
	VirtScreen *vs = &virtscr[0];
	int *offsets;
	int blits_before_refresh, blits;
	int x, y;
	int w, h;
	int i;

	// There's probably some less memory-hungry way of doing this. But
	// since we're only dealing with relatively small images, it shouldn't
	// be too bad.

	w = vs->width / width;
	h = vs->height / height;

	// When used correctly, vs->width % width and vs->height % height
	// should both be zero, but just to be safe...

	if (vs->width % width)
		w++;

	if (vs->height % height)
		h++;

	offsets = (int *) malloc(w * h * sizeof(int));
	if (offsets == NULL) {
		warning("dissolveEffect: out of memory");
		return;
	}

	// Create a permutation of offsets into the frame buffer

	if (width == 1 && height == 1) {
		// Optimized case for pixel-by-pixel dissolve

		for (i = 0; i < vs->size; i++)
			offsets[i] = i;

		for (i = 1; i < w * h; i++) {
			int j;

			j = _rnd.getRandomNumber(i - 1);
			offsets[i] = offsets[j];
			offsets[j] = i;
		}
	} else {
		int *offsets2;

		for (i = 0, x = 0; x < vs->width; x += width)
			for (y = 0; y < vs->height; y += height)
				offsets[i++] = y * vs->width + x;

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
		x = offsets[i] % vs->width;
		y = offsets[i] / vs->width;
		_system->copy_rect(vs->screenPtr + vs->xstart + y * vs->width + x, vs->width, x, y + vs->topline, width, height);

		if (++blits >= blits_before_refresh) {
			blits = 0;
			_system->update_screen();
			waitForTimer(30);
		}
	}

	free(offsets);

	if (blits != 0) {
		_system->update_screen();
		waitForTimer(30);
	}
}

void ScummEngine::scrollEffect(int dir) {
	VirtScreen *vs = &virtscr[0];

	int x, y;
	int step;

	if ((dir == 0) || (dir == 1))
		step = vs->height;
	else
		step = vs->width;

	step = (step * kPictureDelay) / kScrolltime;

	switch (dir) {
	case 0:
		//up
		y = 1 + step;
		while (y < vs->height) {
			_system->move_screen(0, -step, vs->height);
			_system->copy_rect(vs->screenPtr + vs->xstart + (y - step) * vs->width,
				vs->width,
				0, vs->height - step,
				vs->width, step);
			_system->update_screen();
			waitForTimer(kPictureDelay);

			y += step;
		}
		break;
	case 1:
		// down
		y = 1 + step;
		while (y < vs->height) {
			_system->move_screen(0, step, vs->height);
			_system->copy_rect(vs->screenPtr + vs->xstart + vs->width * (vs->height-y),
				vs->width,
				0, 0,
				vs->width, step);
			_system->update_screen();
			waitForTimer(kPictureDelay);

			y += step;
		}
		break;
	case 2:
		// left
		x = 1 + step;
		while (x < vs->width) {
			_system->move_screen(-step, 0, vs->height);
			_system->copy_rect(vs->screenPtr + vs->xstart + x - step,
				vs->width,
				vs->width - step, 0,
				step, vs->height);
			_system->update_screen();
			waitForTimer(kPictureDelay);

			x += step;
		}
		break;
	case 3:
		// right
		x = 1 + step;
		while (x < vs->width) {
			_system->move_screen(step, 0, vs->height);
			_system->copy_rect(vs->screenPtr + vs->xstart + vs->width - x,
				vs->width,
				0, 0,
				step, vs->height);
			_system->update_screen();
			waitForTimer(kPictureDelay);

			x += step;
		}
		break;
	}
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

	/* XXX: not implemented */
	warning("stub unkScreenEffect(%d)", a);
}

void ScummEngine::setShake(int mode) {
	if (_shakeEnabled != (mode != 0))
		_fullRedraw = true;

	_shakeEnabled = mode != 0;
	_shakeFrame = 0;
	_system->set_shake_pos(0);
}

#pragma mark -
#pragma mark --- Palette ---
#pragma mark -

void ScummEngine::setupAmigaPalette() {
	setPalColor( 0,   0,   0,   0);
	setPalColor( 1,   0,   0, 187);
	setPalColor( 2,   0, 187,   0);
	setPalColor( 3,   0, 187, 187);
	setPalColor( 4, 187,   0,   0);
	setPalColor( 5, 187,   0, 187);
	setPalColor( 6, 187, 119,   0);
	setPalColor( 7, 187, 187, 187);
	setPalColor( 8, 119, 119, 119);
	setPalColor( 9, 119, 119, 255);
	setPalColor(10,  0,  255,   0);
	setPalColor(11,   0, 255, 255);
	setPalColor(12, 255, 136, 136);
	setPalColor(13, 255,   0, 255);
	setPalColor(14, 255, 255,   0);
	setPalColor(15, 255, 255, 255);
}

void ScummEngine::setupEGAPalette() {
	setPalColor( 0,   0,   0,   0);
	setPalColor( 1,   0,   0, 168);
	setPalColor( 2,   0, 168,   0);
	setPalColor( 3,   0, 168, 168);
	setPalColor( 4, 168,   0,   0);
	setPalColor( 5, 168,   0, 168); 
	setPalColor( 6, 168,  84,   0);
	setPalColor( 7, 168, 168, 168);
	setPalColor( 8,  84,  84,  84);
	setPalColor( 9,  84,  84, 252);
	setPalColor(10,  84, 252,  84);
	setPalColor(11,  84, 252, 252);
	setPalColor(12, 252,  84,  84);
	setPalColor(13, 252,  84, 252);
	setPalColor(14, 252, 252,  84);
	setPalColor(15, 252, 252, 252);
}

void ScummEngine::setupV1ManiacPalette() {
	setPalColor( 0,   0,   0,   0); 
	setPalColor( 1, 252, 252, 252); 
	setPalColor( 2, 168,   0,   0);
	setPalColor( 3,   0, 168, 168); 
	setPalColor( 4, 168,   0, 168); 
	setPalColor( 5,   0, 168,   0);
	setPalColor( 6,   0,   0, 168);
	setPalColor( 7, 252, 252,  84);
	setPalColor( 8, 252,  84,  84);
	setPalColor( 9, 168,  84,   0);
	setPalColor(10, 252,  84,  84); 
	setPalColor(11,  84,  84,  84); 
	setPalColor(12, 168, 168, 168);
	setPalColor(13,  84, 252,  84);
	setPalColor(14,  84,  84, 252);
	setPalColor(15,  84,  84,  84);
}

void ScummEngine::setupV1ZakPalette() {
	setPalColor( 0,   0,   0,   0); 
	setPalColor( 1, 252, 252, 252); 
	setPalColor( 2, 168,   0,   0);
	setPalColor( 3,   0, 168, 168); 
	setPalColor( 4, 168,   0, 168); 
	setPalColor( 5,   0, 168,   0);
	setPalColor( 6,   0,   0, 168);
	setPalColor( 7, 252, 252,  84);
	setPalColor( 8, 252,  84,  84);
	setPalColor( 9, 168,  84,   0);
	setPalColor(10, 252,  84,  84); 
	setPalColor(11,  84,  84,  84); 
	setPalColor(12, 168, 168, 168);
	setPalColor(13,  84, 252,  84);
	setPalColor(14,  84,  84, 252);
	setPalColor(15, 168, 168, 168);
}

void ScummEngine::setPaletteFromPtr(const byte *ptr) {
	int i;
	byte *dest, r, g, b;
	int numcolor;

	if (_features & GF_SMALL_HEADER) {
		if (_features & GF_OLD256)
			numcolor = 256;
		else
			numcolor = READ_LE_UINT16(ptr + 6) / 3;
		ptr += 8;
	} else {
		numcolor = getResourceDataSize(ptr) / 3;
	}

	checkRange(256, 0, numcolor, "Too many colors (%d) in Palette");

	dest = _currentPalette;

	for (i = 0; i < numcolor; i++) {
		r = *ptr++;
		g = *ptr++;
		b = *ptr++;

		// This comparison might look weird, but it's what the disassembly (DOTT) says!
		// FIXME: Fingolfin still thinks it looks weird: the value 252 = 4*63 clearly comes from
		// the days 6/6/6 palettes were used, OK. But it breaks MonkeyVGA, so I had to add a
		// check for that. And somebody before me added a check for V7 games, turning this
		// off there, too... I wonder if it hurts other games, too? What exactly is broken
		// if we remove this patch?
		// Since it also causes problems in Zak256, I am turning it off for all V4 games and older.
		if ((_version <= 4) || (_version >= 7) || (i <= 15 || r < 252 || g < 252 || b < 252)) {
			*dest++ = r;
			*dest++ = g;
			*dest++ = b;
		} else {
			dest += 3;
		}
	}
	setDirtyColors(0, numcolor - 1);
}

void ScummEngine::setPaletteFromRes() {
	byte *ptr;
	ptr = getResourceAddress(rtRoom, _roomResource) + _CLUT_offs;
	setPaletteFromPtr(ptr);
}

void ScummEngine::setDirtyColors(int min, int max) {
	if (_palDirtyMin > min)
		_palDirtyMin = min;
	if (_palDirtyMax < max)
		_palDirtyMax = max;
}

void ScummEngine::initCycl(const byte *ptr) {
	int j;
	ColorCycle *cycl;

	memset(_colorCycle, 0, sizeof(_colorCycle));

	if (_features & GF_SMALL_HEADER) {
		cycl = _colorCycle;
		ptr += 6;
		for (j = 0; j < 16; ++j, ++cycl) {
			uint16 delay = READ_BE_UINT16(ptr);
			ptr += 2;
			byte start = *ptr++;
			byte end = *ptr++;

			if (!delay || delay == 0x0aaa || start >= end)
				continue;

			cycl->counter = 0;
			cycl->delay = 16384 / delay;
			cycl->flags = 2;
			cycl->start = start;
			cycl->end = end;
		}
	} else {
		while ((j = *ptr++) != 0) {
			if (j < 1 || j > 16) {
				error("Invalid color cycle index %d", j);
			}
			cycl = &_colorCycle[j - 1];

			ptr += 2;
			cycl->counter = 0;
			cycl->delay = 16384 / READ_BE_UINT16(ptr);
			ptr += 2;
			cycl->flags = READ_BE_UINT16(ptr);
			ptr += 2;
			cycl->start = *ptr++;
			cycl->end = *ptr++;
		}
	}
}

void ScummEngine::stopCycle(int i) {
	ColorCycle *cycl;

	checkRange(16, 0, i, "Stop Cycle %d Out Of Range");
	if (i != 0) {
		_colorCycle[i - 1].delay = 0;
		return;
	}

	for (i = 0, cycl = _colorCycle; i < 16; i++, cycl++)
		cycl->delay = 0;
}

/**
 * Cycle the colors in the given palette in the intervael [cycleStart, cycleEnd]
 * either one step forward or backward.
 */
static void doCyclePalette(byte *palette, int cycleStart, int cycleEnd, int size, bool forward) {
	byte *start = palette + cycleStart * size;
	byte *end = palette + cycleEnd * size;
	int num = cycleEnd - cycleStart;
	byte tmp[6];
	
	assert(size <= 6);
	
	if (forward) {
		memmove(tmp, end, size);
		memmove(start + size, start, num * size);
		memmove(start, tmp, size);
	} else {
		memmove(tmp, start, size);
		memmove(start, start + size, num * size);
		memmove(end, tmp, size);
	}
}

/**
 * Adjust an 'indirect' color palette for the color cycling performed on its master
 * palette. An indirect palette is a palette which contains indices pointing into
 * another palette - it provides a level of indirection to map palette colors to
 * other colors. Now when the target palette is cycled, the indirect palette suddenly
 * point at the wrong color(s). This function takes care of adjusting an indirect
 * palette by searching through it and replacing all indices that are in the
 * cycle range by the new (cycled) index.
 */
static void doCycleIndirectPalette(byte *palette, int cycleStart, int cycleEnd, bool forward) {
	int num = cycleEnd - cycleStart + 1;
	int i;
	int offset = forward ? 1 : num - 1;
	
	for (i = 0; i < 256; i++) {
		if (cycleStart <= palette[i] && palette[i] <= cycleEnd) {
			palette[i] = (palette[i] - cycleStart + offset) % num + cycleStart;
		}
	}
}


void ScummEngine::cyclePalette() {
	ColorCycle *cycl;
	int valueToAdd;
	int i, j;

	if (VAR_TIMER == 0xFF) {
		// FIXME - no idea if this is right :-/
		// Needed for both V2 and V8 at this time
		valueToAdd = VAR(VAR_TIMER_NEXT);
	} else {
		valueToAdd = VAR(VAR_TIMER);
		if (valueToAdd < VAR(VAR_TIMER_NEXT))
			valueToAdd = VAR(VAR_TIMER_NEXT);
	}

	if (!_colorCycle)							// FIXME
		return;

	for (i = 0, cycl = _colorCycle; i < 16; i++, cycl++) {
		if (!cycl->delay || cycl->start > cycl->end)
			continue;
		cycl->counter += valueToAdd;
		if (cycl->counter >= cycl->delay) {
			cycl->counter %= cycl->delay;

			setDirtyColors(cycl->start, cycl->end);
			moveMemInPalRes(cycl->start, cycl->end, cycl->flags & 2);

			if (!(_features & GF_SMALL_HEADER && _version > 2))
				doCyclePalette(_currentPalette, cycl->start, cycl->end, 3, !(cycl->flags & 2));

			// Also cycle the other, indirect palettes
			if (_proc_special_palette) {
				doCycleIndirectPalette(_proc_special_palette, cycl->start, cycl->end, !(cycl->flags & 2));
			}
			
			if (_shadowPalette) {
				if (_version >= 7) {
					for (j = 0; j < NUM_SHADOW_PALETTE; j++)
						doCycleIndirectPalette(_shadowPalette + j * 256, cycl->start, cycl->end, !(cycl->flags & 2));
				} else {
					doCycleIndirectPalette(_shadowPalette, cycl->start, cycl->end, !(cycl->flags & 2));
				}
			}
		}
	}
}

/**
 * Perform color cycling on the palManipulate data, too, otherwise
 * color cycling will be disturbed by the palette fade.
 */
void ScummEngine::moveMemInPalRes(int start, int end, byte direction) {
	if (!_palManipCounter)
		return;

	doCyclePalette(_palManipPalette, start, end, 3, !direction);
	doCyclePalette(_palManipIntermediatePal, start, end, 6, !direction);
}

void ScummEngine::palManipulateInit(int start, int end, int string_id, int time) {
	byte *pal, *target, *between;
	byte *string1, *string2, *string3;
	int i;

	string1 = getStringAddress(string_id);
	string2 = getStringAddress(string_id + 1);
	string3 = getStringAddress(string_id + 2);
	if (!string1 || !string2 || !string3) {
		warning("palManipulateInit(%d,%d,%d,%d): Cannot obtain string resources %d, %d and %d",
		        start, end, string_id, time, string_id, string_id + 1, string_id + 2);
		return;
	}

	string1+=start;
	string2+=start;
	string3+=start;

	_palManipStart = start;
	_palManipEnd = end;
	_palManipCounter = 0;
	
	if (!_palManipPalette)
		_palManipPalette = (byte *)calloc(0x300, 1);
	if (!_palManipIntermediatePal)
		_palManipIntermediatePal = (byte *)calloc(0x600, 1);

	pal = _currentPalette + start * 3;
	target = _palManipPalette + start * 3;
	between = _palManipIntermediatePal + start * 6;

	for (i = start; i < end; ++i) {
		*target++ = *string1++;
		*target++ = *string2++;
		*target++ = *string3++;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
	}

	_palManipCounter = time;
}

void ScummEngine::palManipulate() {
	byte *target, *pal, *between;
	int i, j;

	if (!_palManipCounter || !_palManipPalette || !_palManipIntermediatePal)
		return;
	
	target = _palManipPalette + _palManipStart * 3;
	pal = _currentPalette + _palManipStart * 3;
	between = _palManipIntermediatePal + _palManipStart * 6;

	for (i = _palManipStart; i < _palManipEnd; ++i) {
		j = (*((uint16 *)between) += ((*target++ << 8) - *((uint16 *)between)) / _palManipCounter);
		*pal++ = j >> 8;
		between += 2;
		j = (*((uint16 *)between) += ((*target++ << 8) - *((uint16 *)between)) / _palManipCounter);
		*pal++ = j >> 8;
		between += 2;
		j = (*((uint16 *)between) += ((*target++ << 8) - *((uint16 *)between)) / _palManipCounter);
		*pal++ = j >> 8;
		between += 2;
	}
	setDirtyColors(_palManipStart, _palManipEnd);
	_palManipCounter--;
}

void ScummEngine::setupShadowPalette(int slot, int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	byte *table;
	int i;
	byte *curpal;

	if (slot < 0 || slot >= NUM_SHADOW_PALETTE)
		error("setupShadowPalette: invalid slot %d", slot);

	if (startColor < 0 || startColor > 255 || endColor < 0 || startColor > 255 || endColor < startColor)
		error("setupShadowPalette: invalid range from %d to %d", startColor, endColor);

	table = _shadowPalette + slot * 256;
	for (i = 0; i < 256; i++)
		table[i] = i;

	table += startColor;
	curpal = _currentPalette + startColor * 3;
	for (i = startColor; i <= endColor; i++) {
		*table++ = remapPaletteColor((curpal[0] * redScale) >> 8,
									 (curpal[1] * greenScale) >> 8,
									 (curpal[2] * blueScale) >> 8,
									 (uint) - 1);
		curpal += 3;
	}
}

void ScummEngine::setupShadowPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	const byte *basepal = getPalettePtr();
	const byte *pal = basepal;
	const byte *compareptr;
	byte *table = _shadowPalette;
	int i;

	// This is a correction of the patch supplied for BUG #588501.
	// It has been tested in all four known rooms where unkRoomFunc3 is used:
	//
	// 1) FOA Room 53: subway departing Knossos for Atlantis.
	// 2) FOA Room 48: subway crashing into the Atlantis entrance area
	// 3) FOA Room 82: boat/sub shadows while diving near Thera
	// 4) FOA Room 23: the big machine room inside Atlantis
	//
	// The implementation behaves well in all tests.
	// Pixel comparisons show that the resulting palette entries being
	// derived from the shadow palette generated here occassionally differ
	// slightly from the ones derived in the LEC executable.
	// Not sure yet why, but the differences are VERY minor.
	//
	// There seems to be no explanation for why this function is called
	// from within Room 23 (the big machine), as it has no shadow effects
	// and thus doesn't result in any visual differences.

	for (i = 0; i <= 255; i++) {
		int r = (int) (*pal++ * redScale) >> 8;
		int g = (int) (*pal++ * greenScale) >> 8;
		int b = (int) (*pal++ * blueScale) >> 8;

		// The following functionality is similar to remapPaletteColor, except
		// 1) we have to work off the original CLUT rather than the current palette, and
		// 2) the target shadow palette entries must be bounded to the upper and lower
		//    bounds provided by the opcode. (This becomes significant in Room 48, but
		//    is not an issue in all other known case studies.)
		int j;
		int ar, ag, ab;
		uint sum, bestsum, bestitem = 0;

		if (r > 255)
			r = 255;
		if (g > 255)
			g = 255;
		if (b > 255)
			b = 255;

		bestsum = (uint)-1;

		r &= ~3;
		g &= ~3;
		b &= ~3;

		compareptr = basepal + startColor * 3;
		for (j = startColor; j <= endColor; j++, compareptr += 3) {
			ar = compareptr[0] & ~3;
			ag = compareptr[1] & ~3;
			ab = compareptr[2] & ~3;
			if (ar == r && ag == g && ab == b) {
				bestitem = j;
				break;
			}

			sum = colorWeight(ar - r, ag - g, ab - b);

			if (sum < bestsum) {
				bestsum = sum;
				bestitem = j;
			}
		}
		*table++ = bestitem;
	}
}

/** This function create the specialPalette used for semi-transparency in SamnMax */
void ScummEngine::createSpecialPalette(int16 from, int16 to, int16 redScale, int16 greenScale, int16 blueScale,
			int16 startColor, int16 endColor) {
	const byte *palPtr, *curPtr;
	const byte *searchPtr;

	uint bestResult;
	uint currentResult;

	byte currentIndex;

	int i, j;

	palPtr = getPalettePtr();

	for (i = 0; i < 256; i++)
		_proc_special_palette[i] = i;

	curPtr = palPtr + startColor * 3;

	for (i = startColor; i < endColor; i++) {
		int r = (int) (*curPtr++ * redScale) >> 8;
		int g = (int) (*curPtr++ * greenScale) >> 8;
		int b = (int) (*curPtr++ * blueScale) >> 8;

		if (r > 255)
			r = 255;
		if (g > 255)
			g = 255;
		if (b > 255)
			b = 255;

		searchPtr = palPtr + from * 3;
		bestResult = (uint)-1;
		currentIndex = (byte) from;

		for (j = from; j <= to; j++) {
			int ar = (*searchPtr++);
			int ag = (*searchPtr++);
			int ab = (*searchPtr++);

			currentResult = colorWeight(ar - r, ag - g, ab - b);

			if (currentResult < bestResult) {
				_proc_special_palette[i] = currentIndex;
				bestResult = currentResult;
			}
			currentIndex++;
		}
	}
}

void ScummEngine::darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	if (_roomResource == 0) // FIXME - HACK to get COMI demo working
		return;

	if (startColor <= endColor) {
		const byte *cptr;
		byte *cur;
		int j;
		int color;

		cptr = getPalettePtr() + startColor * 3;
		cur = _currentPalette + startColor * 3;

		for (j = startColor; j <= endColor; j++) {
			color = *cptr++;
			color = color * redScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;

			color = *cptr++;
			color = color * greenScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;

			color = *cptr++;
			color = color * blueScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;
		}
		setDirtyColors(startColor, endColor);
	}
}

static int value(int n1, int n2, int hue) {
	if (hue > 360)
		hue = hue - 360;
	else if (hue < 0)
		hue = hue + 360;

	if (hue < 60)
		return n1 + (n2 - n1) * hue / 60;
	if (hue < 180)
		return n2;
	if (hue < 240)
		return n1 + (n2 - n1) * (240 - hue) / 60;
	return n1;
}

/**
 * This function scales the HSL (Hue, Saturation and Lightness)
 * components of the palette colors. It's used in CMI when Guybrush
 * walks from the beach towards the swamp.
 */
void ScummEngine::desaturatePalette(int hueScale, int satScale, int lightScale, int startColor, int endColor) {

	if (startColor <= endColor) {
		const byte *cptr;
		byte *cur;
		int j;

		cptr = getPalettePtr() + startColor * 3;
		cur = _currentPalette + startColor * 3;

		for (j = startColor; j <= endColor; j++) {
			int R = *cptr++;
			int G = *cptr++;
			int B = *cptr++;

			// RGB to HLS (Foley and VanDam)

			const int min = MIN(R, MIN(G, B));
			const int max = MAX(R, MAX(G, B));
			const int diff = (max - min);
			const int sum = (max + min);

			if (diff != 0) {
				int H, S, L;
				
				if (sum <= 255)
					S = 255 * diff / sum;
				else
					S = 255 * diff / (255 * 2 - sum);

				if (R == max)
					H = 60 * (G - B) / diff;
				else if (G == max)
					H = 120 + 60 * (B - R) / diff;
				else
					H = 240 + 60 * (R - G) / diff;

				if (H < 0)
					H = H + 360;

				// Scale the result
	
				H = (H * hueScale) / 255;
				S = (S * satScale) / 255;
				L = (sum * lightScale) / 255;
	
				// HLS to RGB (Foley and VanDam)
	
				int m1, m2;
				if (L <= 255)
					m2 = L * (255 + S) / (255 * 2);
				else
					m2 = L * (255 - S) / (255 * 2) + S;

				m1 = L - m2;

				R = value(m1, m2, H + 120);
				G = value(m1, m2, H);
				B = value(m1, m2, H - 120);
			} else {
				// Maximal color = minimal color -> R=G=B -> it's a grayscale.
				R = G = B = (R * lightScale) / 255;
			}

			*cur++ = R;
			*cur++ = G;
			*cur++ = B;
		}

		setDirtyColors(startColor, endColor);
	}
}


int ScummEngine::remapPaletteColor(int r, int g, int b, uint threshold) {
	int i;
	int ar, ag, ab;
	uint sum, bestsum, bestitem = 0;
	byte *pal = _currentPalette;

	if (r > 255)
		r = 255;
	if (g > 255)
		g = 255;
	if (b > 255)
		b = 255;

	bestsum = (uint) - 1;

	r &= ~3;
	g &= ~3;
	b &= ~3;

	for (i = 0; i < 256; i++, pal += 3) {
		ar = pal[0] & ~3;
		ag = pal[1] & ~3;
		ab = pal[2] & ~3;
		if (ar == r && ag == g && ab == b)
			return i;

		sum = colorWeight(ar - r, ag - g, ab - b);

		if (sum < bestsum) {
			bestsum = sum;
			bestitem = i;
		}
	}

	if (threshold != (uint) - 1 && bestsum > colorWeight(threshold, threshold, threshold)) {
		// Best match exceeded threshold. Try to find an unused palette entry and
		// use it for our purpose.
		pal = _currentPalette + (256 - 2) * 3;
		for (i = 254; i > 48; i--, pal -= 3) {
			if (pal[0] >= 252 && pal[1] >= 252 && pal[2] >= 252) {
				setPalColor(i, r, g, b);
				return i;
			}
		}
	}

	return bestitem;
}

void ScummEngine::swapPalColors(int a, int b) {
	byte *ap, *bp;
	byte t;

	if ((uint) a >= 256 || (uint) b >= 256)
		error("swapPalColors: invalid values, %d, %d", a, b);

	ap = &_currentPalette[a * 3];
	bp = &_currentPalette[b * 3];

	t = ap[0];
	ap[0] = bp[0];
	bp[0] = t;
	t = ap[1];
	ap[1] = bp[1];
	bp[1] = t;
	t = ap[2];
	ap[2] = bp[2];
	bp[2] = t;

	setDirtyColors(a, a);
	setDirtyColors(b, b);
}

void ScummEngine::copyPalColor(int dst, int src) {
	byte *dp, *sp;

	if ((uint) dst >= 256 || (uint) src >= 256)
		error("copyPalColor: invalid values, %d, %d", dst, src);

	dp = &_currentPalette[dst * 3];
	sp = &_currentPalette[src * 3];

	dp[0] = sp[0];
	dp[1] = sp[1];
	dp[2] = sp[2];

	setDirtyColors(dst, dst);
}

void ScummEngine::setPalColor(int idx, int r, int g, int b) {
	_currentPalette[idx * 3 + 0] = r;
	_currentPalette[idx * 3 + 1] = g;
	_currentPalette[idx * 3 + 2] = b;
	setDirtyColors(idx, idx);
}

void ScummEngine::setPalette(int palindex) {
	const byte *pals;

	_curPalIndex = palindex;
	pals = getPalettePtr();
	setPaletteFromPtr(pals);
}

const byte *ScummEngine::findPalInPals(const byte *pal, int idx) {
	const byte *offs;
	uint32 size;

	pal = findResource(MKID('WRAP'), pal);
	if (pal == NULL)
		return NULL;

	offs = findResourceData(MKID('OFFS'), pal);
	if (offs == NULL)
		return NULL;

	size = getResourceDataSize(offs) >> 2;

	if ((uint32)idx >= (uint32)size)
		return NULL;

	return offs + READ_LE_UINT32(offs + idx * sizeof(uint32));
}

const byte *ScummEngine::getPalettePtr() {
	const byte *cptr;

	cptr = getResourceAddress(rtRoom, _roomResource);
	assert(cptr);
	if (_CLUT_offs) {
		cptr += _CLUT_offs;
	} else {
		cptr = findPalInPals(cptr + _PALS_offs, _curPalIndex);
	}
	assert(cptr);
	return cptr;
}

void ScummEngine::updatePalette() {
	if (_palDirtyMax == -1)
		return;

	bool noir_mode = (_gameId == GID_SAMNMAX && readVar(0x8000));
	int first = _palDirtyMin;
	int num = _palDirtyMax - first + 1;
	int i;

	byte palette_colors[1024];
	byte *p = palette_colors;

	for (i = _palDirtyMin; i <= _palDirtyMax; i++) {
		byte *data;

		if (_features & GF_SMALL_HEADER && _version > 2)
			data = _currentPalette + _shadowPalette[i] * 3;
		else
			data = _currentPalette + i * 3;

		// Sam & Max film noir mode. Convert the colours to grayscale
		// before uploading them to the backend.

		if (noir_mode) {
			int r, g, b;
			byte brightness;

			r = data[0];
			g = data[1];
			b = data[2];

			brightness = (byte)((0.299 * r + 0.587 * g + 0.114 * b) + 0.5);

			*p++ = brightness;
			*p++ = brightness;
			*p++ = brightness;
			*p++ = 0;
		} else {
			*p++ = data[0];
			*p++ = data[1];
			*p++ = data[2];
			*p++ = 0;
		}
	}
	
	_system->set_palette(palette_colors, first, num);

	_palDirtyMax = -1;
	_palDirtyMin = 256;
}

#pragma mark -
#pragma mark --- Cursor ---
#pragma mark -

void ScummEngine::setupCursor() {
	_cursor.animate = 1;
	if (_gameId == GID_TENTACLE && res.roomno[rtRoom][60]) {
		// HACK: For DOTT we manually set the default cursor. See also bug #786994
		setCursorImg(697, 60, 1);
		makeCursorColorTransparent(1);
	}
}

void ScummEngine::grabCursor(int x, int y, int w, int h) {
	VirtScreen *vs = findVirtScreen(y);

	if (vs == NULL) {
		warning("grabCursor: invalid Y %d", y);
		return;
	}

	grabCursor(vs->screenPtr + (y - vs->topline) * _screenWidth + x, w, h);

}

void ScummEngine::grabCursor(byte *ptr, int width, int height) {
	uint size;
	byte *dst;

	size = width * height;
	if (size > sizeof(_grabbedCursor))
		error("grabCursor: grabbed cursor too big");

	_cursor.width = width;
	_cursor.height = height;
	_cursor.animate = 0;

	dst = _grabbedCursor;
	for (; height; height--) {
		memcpy(dst, ptr, width);
		dst += width;
		ptr += _screenWidth;
	}

	updateCursor();
}

void ScummEngine::useIm01Cursor(const byte *im, int w, int h) {
	VirtScreen *vs = &virtscr[0];
	byte *buf, *dst;
	const byte *src;
	int i;

	w <<= 3;
	h <<= 3;

	dst = buf = (byte *) malloc(w * h);
	src = vs->screenPtr + vs->xstart;

	for (i = 0; i < h; i++) {
		memcpy(dst, src, w);
		dst += w;
		src += _screenWidth;
	}

	drawBox(0, 0, w - 1, h - 1, 0xFF);

	vs->alloctwobuffers = false;
	gdi.disableZBuffer();
	gdi.drawBitmap(im, vs, _screenStartStrip, 0, w, h, 0, w >> 3, 0);
	vs->alloctwobuffers = true;
	gdi.enableZBuffer();

	grabCursor(vs->screenPtr + vs->xstart, w, h);

	src = buf;
	dst = vs->screenPtr + vs->xstart;

	for (i = 0; i < h; i++) {
		memcpy(dst, src, w);
		dst += _screenWidth;
		src += w;
	}

	free(buf);
}

void ScummEngine::setCursor(int cursor) {
	if (cursor >= 0 && cursor <= 3)
		_currentCursor = cursor;
	else
		warning("setCursor(%d)", cursor);
}

void ScummEngine::setCursorHotspot(int x, int y) {
	_cursor.hotspotX = x;
	_cursor.hotspotY = y;
	// FIXME this hacks around offset cursor in the humongous games
	if (_features & GF_HUMONGOUS) {
		_cursor.hotspotX += 15;
		_cursor.hotspotY += 15;
	}
}

void ScummEngine::updateCursor() {
	_system->set_mouse_cursor(_grabbedCursor, _cursor.width, _cursor.height,
	                          _cursor.hotspotX, _cursor.hotspotY);
}

void ScummEngine::animateCursor() {
	if (_cursor.animate) {
		if (!(_cursor.animateIndex & 0x1)) {
			decompressDefaultCursor((_cursor.animateIndex >> 1) & 3);
		}
		_cursor.animateIndex++;
	}
}

void ScummEngine::useBompCursor(const byte *im, int width, int height) {
	uint size;

	width <<= 3;
	height <<= 3;

	size = width * height;
	if (size > sizeof(_grabbedCursor))
		error("useBompCursor: cursor too big (%d)", size);

	_cursor.width = width;
	_cursor.height = height;
	_cursor.animate = 0;

	// Skip the header
	if (_version == 8) {
		im += 16;
	} else {
		im += 18;
	}
	decompressBomp(_grabbedCursor, im, width, height);

	updateCursor();
}

void ScummEngine::decompressDefaultCursor(int idx) {
	int i, j;
	byte color;

	memset(_grabbedCursor, 0xFF, sizeof(_grabbedCursor));

	if (_version == 1)
		color = default_v1_cursor_colors[idx];
	else
		color = default_cursor_colors[idx];

	// FIXME: None of the stock cursors are right for Loom. Why is that?

	if (_gameId == GID_LOOM || _gameId == GID_LOOM256) {
		int w = 0;

		_cursor.width = 8;
		_cursor.height = 8;
		_cursor.hotspotX = 0;
		_cursor.hotspotY = 0;
		
		for (i = 0; i < 8; i++) {
			w += (i >= 6) ? -2 : 1;
			for (j = 0; j < w; j++)
				_grabbedCursor[i * 8 + j] = color;
		}
	} else if (_version <= 2) {
		_cursor.width = 23;
		_cursor.height = 21;
		_cursor.hotspotX = 11;
		_cursor.hotspotY = 10;

		byte *hotspot = _grabbedCursor + _cursor.hotspotY * _cursor.width + _cursor.hotspotX;

		// Crosshair, slightly assymetric

		for (i = 0; i < 7; i++) {
			*(hotspot - 5 - i) = color;
			*(hotspot + 5 + i) = color;
		}

		for (i = 0; i < 8; i++) {
			*(hotspot - _cursor.width * (3 + i)) = color;
			*(hotspot + _cursor.width * (3 + i)) = color;
		}

		// Arrow heads, diagonal lines

		for (i = 1; i <= 3; i++) {
			*(hotspot - _cursor.width * i - 5 - i) = color;
			*(hotspot + _cursor.width * i - 5 - i) = color;
			*(hotspot - _cursor.width * i + 5 + i) = color;
			*(hotspot + _cursor.width * i + 5 + i) = color;
			*(hotspot - _cursor.width * (i + 3) - i) = color;
			*(hotspot - _cursor.width * (i + 3) + i) = color;
			*(hotspot + _cursor.width * (i + 3) - i) = color;
			*(hotspot + _cursor.width * (i + 3) + i) = color;
		}

		// Final touches

		*(hotspot - _cursor.width - 7) = color;
		*(hotspot - _cursor.width + 7) = color;
		*(hotspot + _cursor.width - 7) = color;
		*(hotspot + _cursor.width + 7) = color;
		*(hotspot - (_cursor.width * 5) - 1) = color;
		*(hotspot - (_cursor.width * 5) + 1) = color;
		*(hotspot + (_cursor.width * 5) - 1) = color;
		*(hotspot + (_cursor.width * 5) + 1) = color;
	} else {
		byte currentCursor = _currentCursor;

#ifdef __PALM_OS__
		if (_gameId == GID_ZAK256 && currentCursor == 0)
			currentCursor = 4;
#endif

		_cursor.width = 16;
		_cursor.height = 16;
		_cursor.hotspotX = default_cursor_hotspots[2 * currentCursor];
		_cursor.hotspotY = default_cursor_hotspots[2 * currentCursor + 1];

		for (i = 0; i < 16; i++) {
			for (j = 0; j < 16; j++) {
				if (default_cursor_images[currentCursor][i] & (1 << j))	
					_grabbedCursor[16 * i + 15 - j] = color;
			}
		}
	}

	updateCursor();
}

void ScummEngine::makeCursorColorTransparent(int a) {
	int i, size;

	size = _cursor.width * _cursor.height;

	for (i = 0; i < size; i++)
		if (_grabbedCursor[i] == (byte)a)
			_grabbedCursor[i] = 0xFF;

	updateCursor();
}


#ifdef __PALM_OS__
#include "scumm_globals.h"

_GINIT(Gfx)
_GSETPTR(transitionEffects, GBVARS_TRANSITIONEFFECTS_INDEX, TransitionEffect, GBVARS_SCUMM)
_GEND

_GRELEASE(Gfx)
_GRELEASEPTR(GBVARS_TRANSITIONEFFECTS_INDEX, GBVARS_SCUMM)
_GEND

#endif

} // End of namespace Scumm
