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

#include "stdafx.h"
#include "scumm.h"
#include "actor.h"
#include "gui/newgui.h"
#include "resource.h"
#include "util.h"

enum {
	kScrolltime = 500,  // ms scrolling is supposed to take
	kPictureDelay = 20
};

void Scumm::getGraphicsPerformance()
{
	int i;

	for (i = 10; i != 0; i--) {
		initScreens(0, 0, _realWidth, _realHeight);
	}

	if (!(_features & GF_SMALL_HEADER))	// Variable is reserved for game scripts in earlier games
		_vars[VAR_PERFORMANCE_1] = 0;

	for (i = 10; i != 0; i--) {
		setDirtyRange(0, 0, _realHeight);	//ender
		drawDirtyScreenParts();
	}

	if (!(_features & GF_SMALL_HEADER))	// Variable is reserved for game scripts in earlier games
		_vars[VAR_PERFORMANCE_2] = 0;

	if (_features & GF_AFTER_V7)
		initScreens(0, 0, _realWidth, _realHeight);
	else
		initScreens(0, 16, _realWidth, 144);
}

void Scumm::initScreens(int a, int b, int w, int h)
{
	int i;

	for (i = 0; i < 3; i++) {
		nukeResource(rtBuffer, i + 1);
		nukeResource(rtBuffer, i + 5);
	}

	if (!getResourceAddress(rtBuffer, 4)) {
		initVirtScreen(3, 0, 80, _realWidth, 13, false, false);
	}
	initVirtScreen(0, 0, b, _realWidth, h - b, true, true);
	initVirtScreen(1, 0, 0, _realWidth, b, false, false);
	initVirtScreen(2, 0, h, _realWidth, _realHeight - h, false, false);

	_screenB = b;
	_screenH = h;

}

void Scumm::initVirtScreen(int slot, int number, int top, int width, int height, bool twobufs,
													 bool scrollable)
{
	VirtScreen *vs = &virtscr[slot];
	int size;
	int i;
	byte *ptr;

	assert(height >= 0);
	assert(slot >= 0 && slot < 4);

	if (_features & GF_AFTER_V7) {
		if ((!slot) && (_scrHeight != 0))
			height = _scrHeight;
	}

	vs->number = slot;
	vs->unk1 = 0;
	vs->width = _realWidth;
	vs->topline = top;
	vs->height = height;
	vs->alloctwobuffers = twobufs;
	vs->scrollable = scrollable;
	vs->xstart = 0;
	size = vs->width * vs->height;
	vs->size = size;
	vs->backBuf = NULL;

	if ((vs->scrollable) && (_features & GF_AFTER_V7)) {
		size += _realWidth * 8;
	} else {
		size += _realWidth * 4;
	}

	createResource(rtBuffer, slot + 1, size);
	vs->screenPtr = getResourceAddress(rtBuffer, slot + 1);

	ptr = vs->screenPtr;
	for (i = 0; i < size; i++)		// reset background ?
		*ptr++ = 0;

	if (twobufs) {
		createResource(rtBuffer, slot + 5, size);
	}

	if (slot != 3) {
		setDirtyRange(slot, 0, height);
	}
}

void Scumm::setDirtyRange(int slot, int top, int bottom)
{
	int i;
	VirtScreen *vs = &virtscr[slot];
	for (i = 0; i < gdi._numStrips; i++) {
		vs->tdirty[i] = top;
		vs->bdirty[i] = bottom;
	}
}

/* power of 2 */
#define NUM_SHAKE_POSITIONS 8

static const int8 shake_positions[NUM_SHAKE_POSITIONS] = {
	0, 1 * 2, 2 * 2, 1 * 2, 0 * 2, 2 * 2, 3 * 2, 1 * 2
};

void Scumm::drawDirtyScreenParts()
{
	int i;
	VirtScreen *vs;
	byte *src;

	updateDirtyScreen(2);
	if (_features & GF_OLD256)
		updateDirtyScreen(1);

	if ((camera._last.x == camera._cur.x && camera._last.y == camera._cur.y && (_features & GF_AFTER_V7))
			|| (camera._last.x == camera._cur.x)) {
		updateDirtyScreen(0);
	} else {
		vs = &virtscr[0];

		src = vs->screenPtr + _screenStartStrip * 8 + _screenTop * _realWidth;
		_system->copy_rect(src, _realWidth, 0, vs->topline, _realWidth, vs->height);

		for (i = 0; i < gdi._numStrips; i++) {
			vs->tdirty[i] = (byte)vs->height;
			vs->bdirty[i] = 0;
		}
	}

	/* Handle shaking */
	if (_shakeEnabled && !_newgui->isActive()) {
		_shakeFrame = (_shakeFrame + 1) & (NUM_SHAKE_POSITIONS - 1);
		_system->set_shake_pos(shake_positions[_shakeFrame]);
	} else if (!_shakeEnabled &&_shakeFrame != 0) {
		_shakeFrame = 0;
		_system->set_shake_pos(shake_positions[_shakeFrame]);
	}
}

void Scumm::updateDirtyScreen(int slot)
{
	gdi.updateDirtyScreen(&virtscr[slot]);
}

void Gdi::updateDirtyScreen(VirtScreen *vs)
{
	if (vs->height == 0)
		return;

	_readOffs = 0;
	if (vs->scrollable)
		_readOffs = vs->xstart;

	if (_vm->_features & GF_AFTER_V7 && (_vm->camera._cur.y != _vm->camera._last.y))
		drawStripToScreen(vs, 0, _numStrips << 3, 0, vs->height);
	else {
		int i;
		int start, w, top, bottom;
	
		w = 8;
		start = 0;

		for (i = 0; i < _numStrips; i++) {
			bottom = vs->bdirty[i];
	
			if (bottom) {
				top = vs->tdirty[i];
				vs->tdirty[i] = (byte)vs->height;
				vs->bdirty[i] = 0;
				if (i != (_numStrips - 1) && vs->bdirty[i + 1] == (byte)bottom && vs->tdirty[i + 1] == (byte)top) {
					w += 8;
					continue;
				}
				if (_vm->_features & GF_AFTER_V7)
					drawStripToScreen(vs, start, w, 0, vs->height);
				else
					drawStripToScreen(vs, start, w, top, bottom);
				w = 8;
			}
			start = i + 1;
		}
	}
}

void Gdi::drawStripToScreen(VirtScreen *vs, int x, int w, int t, int b)
{
	byte *ptr;
	int scrollY;
	int height;

	if (b <= t)
		return;

	if (t > vs->height)
		t = 0;

	if (b > vs->height)
		b = vs->height;

	height = b - t;
	if (height > _vm->_realHeight)
		height = _vm->_realHeight;

	// FIXME - is this check really necessary?
	if (_vm->camera._cur.y == 0)
		scrollY = 0;
	else
		scrollY = _vm->_screenTop;

	ptr = vs->screenPtr + (t * _numStrips + x) * 8 + _readOffs + scrollY * _vm->_realWidth;
	_vm->_system->copy_rect(ptr, _vm->_realWidth, x * 8, vs->topline + t, w, height);
}

void Scumm::blit(byte *dst, byte *src, int w, int h)
{
	assert(h > 0);
	assert(src != NULL);
	assert(dst != NULL);

	do {
		memcpy(dst, src, w);
		dst += _realWidth;
		src += _realWidth;
	} while (--h);
}

void Scumm::setCursor(int cursor)
{
	if (cursor >= 0 && cursor <= 3)
		gdi._currentCursor = cursor;
	else
		warning("setCursor(%d)", cursor);
}

void Scumm::setCameraAt(int pos_x, int pos_y)
{

	if (_features & GF_AFTER_V7) {
		ScummPoint old;

		old = camera._cur;

		camera._cur.x = pos_x;
		camera._cur.y = pos_y;

		clampCameraPos(&camera._cur);

		camera._dest = camera._cur;

		assert(camera._cur.x >= (_realWidth / 2) && camera._cur.y >= (_realHeight / 2));

		if ((camera._cur.x != old.x || camera._cur.y != old.y)
				&& _vars[VAR_SCROLL_SCRIPT]) {
			_vars[VAR_CAMERA_POS_X] = camera._cur.x;
			_vars[VAR_CAMERA_POS_Y] = camera._cur.y;
			runScript(_vars[VAR_SCROLL_SCRIPT], 0, 0, 0);
		}
	} else {
		int t;

		if (camera._mode != CM_FOLLOW_ACTOR || abs(pos_x - camera._cur.x) > (_realWidth / 2)) {
			camera._cur.x = pos_x;
		}
		camera._dest.x = pos_x;

		t = _vars[VAR_CAMERA_MIN_X];
		if (camera._cur.x < t)
			camera._cur.x = t;

		t = _vars[VAR_CAMERA_MAX_X];
		if (camera._cur.x > t)
			camera._cur.x = t;

		if (_vars[VAR_SCROLL_SCRIPT]) {
			_vars[VAR_CAMERA_POS_X] = camera._cur.x;
			runScript(_vars[VAR_SCROLL_SCRIPT], 0, 0, 0);
		}

		if (camera._cur.x != camera._last.x && charset._hasMask)
			stopTalk();
	}
}

void Scumm::setCameraFollows(Actor *a)
{
	if (_features & GF_AFTER_V7) {
		byte oldfollow = camera._follows;
		int ax, ay;

		camera._follows = a->number;

		if (!a->isInCurrentRoom()) {
			startScene(a->getRoom(), 0, 0);
		}

		ax = abs(a->x - camera._cur.x);
		ay = abs(a->y - camera._cur.y);

		if (ax > _vars[VAR_CAMERA_THRESHOLD_X] || ay > _vars[VAR_CAMERA_THRESHOLD_Y] || ax > (_realWidth / 2) || ay > (_realHeight / 2)) {
			setCameraAt(a->x, a->y);
		}

		if (a->number != oldfollow)
			runHook(0);
	} else {
		int t, i;

		camera._mode = CM_FOLLOW_ACTOR;
		camera._follows = a->number;

		if (!a->isInCurrentRoom()) {
			startScene(a->getRoom(), 0, 0);
			camera._mode = CM_FOLLOW_ACTOR;
			camera._cur.x = a->x;
			setCameraAt(camera._cur.x, 0);
		}

		t = (a->x >> 3);

		if (t - _screenStartStrip < camera._leftTrigger || t - _screenStartStrip > camera._rightTrigger)
			setCameraAt(a->x, 0);

		for (i = 1, a = getFirstActor(); ++a, i < NUM_ACTORS; i++) {
			if (a->isInCurrentRoom())
				a->needRedraw = true;
		}
		runHook(0);
	}
}

void Scumm::initBGBuffers(int height)
{
	byte *ptr;
	int size, itemsize, i;
	byte *room;

	if (_features & GF_AFTER_V7) {
		initVirtScreen(0, 0, virtscr[0].topline, _realHeight, height, 1, 1);
	}

	room = getResourceAddress(rtRoom, _roomResource);
	if (_features & GF_OLD256) {
		// FIXME - maybe this should check for multiple planes like we do
		// for GF_SMALL_HEADER already.
		gdi._numZBuffer = 2;
	} else if (_features & GF_SMALL_HEADER) {

//#define DEBUG_ZPLANE_CODE
		
		ptr = findResourceData(MKID('SMAP'), room);
#ifdef DEBUG_ZPLANE_CODE
		printf("Trying to determine room zplanes:\n");
		hexdump(ptr-6, 0x20);
#endif

		int off;
		gdi._numZBuffer = 0;
		off = READ_LE_UINT32(ptr);
		for (i = 0; off && (i < 4); i++) {
#ifdef DEBUG_ZPLANE_CODE
			printf("Plane %d\n", i);
			hexdump(ptr, 0x20);
#endif

			gdi._numZBuffer++;
			ptr += off;
			off = READ_LE_UINT16(ptr);
		}
#ifdef DEBUG_ZPLANE_CODE
		printf("Real plane count = %d\n", gdi._numZBuffer);
#endif
	} else {
		ptr = findResource(MKID('RMIH'), findResource(MKID('RMIM'), room));
		gdi._numZBuffer = READ_LE_UINT16(ptr + 8) + 1;
	}
	assert(gdi._numZBuffer >= 1 && gdi._numZBuffer <= 5);

	if (_features & GF_AFTER_V7)
		itemsize = (virtscr[0].height + 4) * gdi._numStrips;
	else
		itemsize = (_scrHeight + 4) * gdi._numStrips;


	size = itemsize * gdi._numZBuffer;
	createResource(rtBuffer, 9, size);

	for (i = 0; i < 4; i++)
		gdi._imgBufOffs[i] = i * itemsize;
}

void Scumm::setPaletteFromPtr(byte *ptr)
{
	int i, r, g, b;
	byte *dest;
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
		if (i <= 15 || r < 252 || g < 252 || b < 252) {
			*dest++ = r;
			*dest++ = g;
			*dest++ = b;
		} else {
			dest += 3;
		}
	}

	setDirtyColors(0, numcolor - 1);
}

void Scumm::setPaletteFromRes()
{
	byte *ptr;
	ptr = getResourceAddress(rtRoom, _roomResource) + _CLUT_offs;
	setPaletteFromPtr(ptr);
}


void Scumm::setDirtyColors(int min, int max)
{
	if (_palDirtyMin > min)
		_palDirtyMin = min;
	if (_palDirtyMax < max)
		_palDirtyMax = max;
}

void Scumm::initCycl(byte *ptr)
{
	int j;
	ColorCycle *cycl;

	memset(_colorCycle, 0, sizeof(_colorCycle));

	while ((j = *ptr++) != 0) {
		if (j < 1 || j > 16) {
			error("Invalid color cycle index %d", j);
		}
		cycl = &_colorCycle[j - 1];

		ptr += 2;
		cycl->counter = 0;
		cycl->delay = 16384 / READ_BE_UINT16_UNALIGNED(ptr);
		ptr += 2;
		cycl->flags = READ_BE_UINT16_UNALIGNED(ptr);
		ptr += 2;
		cycl->start = *ptr++;
		cycl->end = *ptr++;
	}
}

void Scumm::stopCycle(int i)
{
	ColorCycle *cycl;

	checkRange(16, 0, i, "Stop Cycle %d Out Of Range");
	if (i != 0) {
		_colorCycle[i - 1].delay = 0;
		return;
	}

	for (i = 0, cycl = _colorCycle; i < 16; i++, cycl++)
		cycl->delay = 0;
}

void Scumm::cyclePalette()
{
	ColorCycle *cycl;
	int valueToAdd;
	int i, num;
	byte *start, *end;
	byte tmp[3];

	valueToAdd = _vars[VAR_TIMER];
	if (valueToAdd < _vars[VAR_TIMER_NEXT])
		valueToAdd = _vars[VAR_TIMER_NEXT];

	if (!_colorCycle)							// FIXME
		return;

	for (i = 0, cycl = _colorCycle; i < 16; i++, cycl++) {
		if (cycl->delay && (cycl->counter += valueToAdd) >= cycl->delay) {
			do {
				cycl->counter -= cycl->delay;
			} while (cycl->delay <= cycl->counter);

			setDirtyColors(cycl->start, cycl->end);
			moveMemInPalRes(cycl->start, cycl->end, cycl->flags & 2);
			start = &_currentPalette[cycl->start * 3];
			end = &_currentPalette[cycl->end * 3];

			num = cycl->end - cycl->start;

			if (!(cycl->flags & 2)) {
				memmove(tmp, end, 3);
				memmove(start + 3, start, num * 3);
				memmove(start, tmp, 3);
			} else {
				memmove(tmp, start, 3);
				memmove(start, start + 3, num * 3);
				memmove(end, tmp, 3);
			}
		}
	}
}

// Perform color cycling on the palManipulate data, too, otherwise
// color cycling will be disturbed by the palette fade.
void Scumm::moveMemInPalRes(int start, int end, byte direction)
{
	byte *startptr, *endptr;
	byte *startptr2, *endptr2;
	int num;
	byte tmp[6];

	if (!_palManipCounter)
		return;

	startptr = _palManipPalette + start * 3;
	endptr = _palManipPalette + end * 3;
	startptr2 = _palManipIntermediatePal + start * 6;
	endptr2 = _palManipIntermediatePal + end * 6;
	num = end - start;

	if (!endptr) {
		warning("moveMemInPalRes(%d,%d): Bad end pointer\n", start, end);
		return;
	}

	if (!direction) {
		memmove(tmp, endptr, 3);
		memmove(startptr + 3, startptr, num * 3);
		memmove(startptr, tmp, 3);
		memmove(tmp, endptr2, 6);
		memmove(startptr2 + 6, startptr2, num * 6);
		memmove(startptr2, tmp, 6);
	} else {
		memmove(tmp, startptr, 3);
		memmove(startptr, startptr + 3, num * 3);
		memmove(endptr, tmp, 3);
		memmove(tmp, startptr2, 6);
		memmove(startptr2, startptr2 + 6, num * 6);
		memmove(endptr2, tmp, 6);
	}
}

void Scumm::drawFlashlight()
{
	static byte *flashBuffer = NULL;
	static int flashX, flashY, flashW, flashH;

	int i, j, offset;
	int topline = virtscr[0].topline;

	// Remove the flash light first if it was previously drawn
	if (_flashlightIsDrawn) {
		updateDirtyRect(0, flashX<<3, (flashX+flashW)<<3, flashY, flashY+flashH, 0x80000000);
		
		if (flashBuffer) {

			offset = _realWidth - flashW*8;
			i = flashH;
			do {
				j = flashW*2;
				do {
					*(uint32 *)flashBuffer = 0;
					flashBuffer += 4;
				} while (--j);
				flashBuffer += offset;
			} while (--i);
		}

		_flashlightIsDrawn = false;
	}

	if (_flashlightXStrips == 0 || _flashlightYStrips == 0)
		return;
	
	// Calculate the area of the flashlight
	Actor *a = a = derefActorSafe(_vars[VAR_EGO], "drawFlashlight");
	flashW = _flashlightXStrips;
	flashH = _flashlightYStrips * 8;
	flashX = a->x/8 - flashW/2 - _screenStartStrip;
	flashY = a->y - flashH/2;
	
	// Clip the flashlight at the borders
	if (flashX < 0)
		flashX = 0;
	else if (flashX > gdi._numStrips - flashW)
		flashX = gdi._numStrips - flashW;
	if (flashY < 0)
		flashY = 0;
	else if (flashY > virtscr[0].height - flashH)
		flashY = virtscr[0].height - flashH;

	// Redraw any actors "under" the flashlight
	for (i = flashX; i < flashX+flashW; i++) {
		gfxUsageBits[_screenStartStrip + i] |= 0x80000000;
		virtscr[0].tdirty[i] = 0;
		virtscr[0].bdirty[i] = virtscr[0].height;
	}

	byte *bgbak;
	offset = (flashY - topline) * _realWidth + virtscr[0].xstart + flashX * 8;
	flashBuffer = virtscr[0].screenPtr + offset;
	bgbak = getResourceAddress(rtBuffer, 5) + offset;

	blit(flashBuffer, bgbak, flashW*8, flashH);

	// Round the corners. To do so, we simply hard-code a set of nicely
	// rounded corners.
	int corner_data[] = { 8, 6, 4, 3, 2, 2, 1, 1 };
	int minrow = 0;
	int maxcol = flashW * 8 - 1;
	int maxrow = (flashH - 1) * _realWidth;

	for (i = 0; i < 8; i++, minrow += _realWidth, maxrow -= _realWidth) {
		int d = corner_data[i];

		for (j = 0; j < d; j++) {
			flashBuffer[minrow + j] = 0;
			flashBuffer[minrow + maxcol - j] = 0;
			flashBuffer[maxrow + j] = 0;
			flashBuffer[maxrow + maxcol - j] = 0;
		}
	}
	
	_flashlightIsDrawn = true;
}

void Scumm::fadeIn(int effect)
{
	switch (effect) {
	case 1:
	case 2:
	case 3:
	case 4:
		transitionEffect(effect - 1);
		break;
	case 128:
		unkScreenEffect6();
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
	case 129:
		break;
	default:
		warning("Unknown screen effect, %d", effect);
	}
	_screenEffectFlag = true;
}

void Scumm::fadeOut(int a)
{
	VirtScreen *vs;

	setDirtyRange(0, 0, 0);
	if (!(_features & GF_AFTER_V7))
		camera._last.x = camera._cur.x;

	if (!_screenEffectFlag)
		return;
	_screenEffectFlag = false;

	if (a == 0)
		return;

	// Fill screen 0 with black
	vs = &virtscr[0];
	gdi._backbuff_ptr = vs->screenPtr + vs->xstart;
	memset(gdi._backbuff_ptr, 0, vs->size);

	// Fade to black with the specified effect, if any.
	switch (a) {
	case 1:
	case 2:
	case 3:
	case 4:
		transitionEffect(a - 1);
		break;
	case 128:
		unkScreenEffect6();
		break;
	case 129:
		// Just blit screen 0 to the display (i.e. display will be black)
		setDirtyRange(0, 0, vs->height);
		updateDirtyScreen(0);
		break;
	case 134:
		dissolveEffect(1, 1);
		break;
	case 135:
		unkScreenEffect5(1);
		break;
	default:
		warning("fadeOut: default case %d", a);
	}
}

void Scumm::redrawBGAreas()
{
	int i;
	int val;
	int diff;

	if (!(_features & GF_AFTER_V7))
		if (camera._cur.x != camera._last.x && charset._hasMask)
			stopTalk();

	val = 0;

	if (!_fullRedraw && _BgNeedsRedraw) {
		for (i = 0; i != gdi._numStrips; i++) {
			if (gfxUsageBits[_screenStartStrip + i] & 0x80000000) {
				redrawBGStrip(i, 1);
			}
		}
	}

	if (_features & GF_AFTER_V7) {
		diff = (camera._cur.x >> 3) - (camera._last.x >> 3);
		if (_fullRedraw == 0 && diff == 1) {
			val = 2;
			redrawBGStrip(gdi._numStrips - 1, 1);
		} else if (_fullRedraw == 0 && diff == -1) {
			val = 1;
			redrawBGStrip(0, 1);
		} else if (_fullRedraw != 0 || diff != 0) {
			_BgNeedsRedraw = false;
			_fullRedraw = false;
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
			_flashlightIsDrawn = false;
			redrawBGStrip(0, gdi._numStrips);
		}
	}

	drawRoomObjects(val);
	_BgNeedsRedraw = false;
}

const uint32 zplane_tags[] = {
	MKID('ZP00'),
	MKID('ZP01'),
	MKID('ZP02'),
	MKID('ZP03'),
	MKID('ZP04')
};

void Gdi::drawBitmap(byte *ptr, VirtScreen *vs, int x, int y, int h,
										 int stripnr, int numstrip, byte flag)
{
	byte *smap_ptr, *where_draw_ptr;
	int i;
	byte *zplane_list[6];

	int bottom;
	byte twobufs;
	int numzbuf;
	int sx;
	bool lightsOn;

	// Check whether lights are turned on or not
	lightsOn = (_vm->_features & GF_AFTER_V6) || (vs->number != 0) || (_vm->_vars[_vm->VAR_CURRENT_LIGHTS] & LIGHTMODE_screen);

	CHECK_HEAP;
	if (_vm->_features & GF_SMALL_HEADER)
		smap_ptr = _smap_ptr = ptr;
	else
		smap_ptr = findResource(MKID('SMAP'), ptr);

	assert(smap_ptr);

	numzbuf = _disable_zbuffer ? 0 : _numZBuffer;

	if (_vm->_features & GF_SMALL_HEADER) {
		/* this is really ugly, FIXME */
		if (ptr[-2] == 'B' && ptr[-1] == 'M' && READ_LE_UINT32(ptr - 6) > (READ_LE_UINT32(ptr) + 10)) {
			zplane_list[1] = smap_ptr + READ_LE_UINT32(ptr);
			// FIXME - how does GF_OLD256 encode the multiple zplanes?
			if (!(_vm->_features & GF_OLD256))
				for (i = 2; i < numzbuf; i++) {
					zplane_list[i] = zplane_list[i-1] + READ_LE_UINT16(zplane_list[i-1]);
			}
		} else if (ptr[-4] == 'O' && ptr[-3] == 'I' && READ_LE_UINT32(ptr - 8) > READ_LE_UINT32(ptr) + 12) {
			zplane_list[1] = smap_ptr + READ_LE_UINT32(ptr);
			// FIXME - how does GF_OLD256 encode the multiple zplanes?
			if (!(_vm->_features & GF_OLD256))
				for (i = 2; i < numzbuf; i++) {
					zplane_list[i] = zplane_list[i-1] + READ_LE_UINT16(zplane_list[i-1]);
			}
		} else {
			zplane_list[1] = 0;
		}
	} else {
		for (i = 1; i < numzbuf; i++) {
			zplane_list[i] = findResource(zplane_tags[i], ptr);
		}
	}



	bottom = y + h;
	if (bottom > vs->height) {
		warning("Gdi::drawBitmap, strip drawn to %d below window bottom %d", bottom, vs->height);
	}

	twobufs = vs->alloctwobuffers;

	_vertStripNextInc = h * _vm->_realWidth - 1;

	_numLinesToProcess = h;

	do {
		if (_vm->_features & GF_SMALL_HEADER)
			_smap_ptr = smap_ptr + READ_LE_UINT32(smap_ptr + stripnr * 4 + 4);
		else
			_smap_ptr = smap_ptr + READ_LE_UINT32(smap_ptr + stripnr * 4 + 8);

		CHECK_HEAP;
		sx = x;
		if (vs->scrollable)
			sx -= vs->xstart >> 3;

		if (sx < 0)
			goto next_iter;

		if (sx >= _numStrips)
			return;

		if (y < vs->tdirty[sx])
			vs->tdirty[sx] = y;

		if (bottom > vs->bdirty[sx])
			vs->bdirty[sx] = bottom;

		_backbuff_ptr = vs->screenPtr + (y * _numStrips + x) * 8;
		if (twobufs)
			_bgbak_ptr = _vm->getResourceAddress(rtBuffer, vs->number + 5) + (y * _numStrips + x) * 8;
		else
			_bgbak_ptr = _backbuff_ptr;

		_mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + (y * _numStrips + x);

		where_draw_ptr = _bgbak_ptr;
		decompressBitmap();

		CHECK_HEAP;
		if (twobufs) {
			_bgbak_ptr = where_draw_ptr;

			if (_vm->hasCharsetMask(sx << 3, y, (sx + 1) << 3, bottom)) {
				if (flag & dbClear || !lightsOn)
					clear8ColWithMasking();
				else
					draw8ColWithMasking();
			} else {
				if (flag & dbClear || !lightsOn)
					clear8Col();
				else
					_vm->blit(_backbuff_ptr, _bgbak_ptr, 8, h);
			}
		}
		CHECK_HEAP;

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

		if (flag & dbDrawMaskOnAll) {
			_z_plane_ptr = zplane_list[1] + READ_LE_UINT16(zplane_list[1] + stripnr * 2 + 8);
			for (i = 0; i < numzbuf; i++) {
				_mask_ptr_dest = _vm->getResourceAddress(rtBuffer, 9) + y * _numStrips + x + _imgBufOffs[i];
				if (_useOrDecompress && flag & dbAllowMaskOr)
					decompressMaskImgOr();
				else
					decompressMaskImg();
			}
		} else {
			for (i = 1; i < numzbuf; i++) {
				uint16 offs;

				if (!zplane_list[i])
					continue;

				if (_vm->_features & GF_SMALL_HEADER) {
					if (_vm->_features & GF_OLD256)
						offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2 + 4);
					else
						offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2 + 2);
				} else
					offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2 + 8);

				_mask_ptr_dest = _vm->getResourceAddress(rtBuffer, 9) + y * _numStrips + x + _imgBufOffs[i];

				if (offs) {
					_z_plane_ptr = zplane_list[i] + offs;

					if (_useOrDecompress && flag & dbAllowMaskOr)
						decompressMaskImgOr();
					else
						decompressMaskImg();
				} else {
					if (!(_useOrDecompress && flag & dbAllowMaskOr))
						for (int height = 0; height < _numLinesToProcess; height++)
							_mask_ptr_dest[height * _numStrips] = 0;
					/* needs better abstraction, FIXME */
				}
			}
		}

next_iter:
		CHECK_HEAP;
		x++;
		stripnr++;
	} while (--numstrip);
}


void Gdi::decompressBitmap()
{
	const byte decompress_table[] = {
		0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF, 0x0,
	};

	_useOrDecompress = false;

	byte code = *_smap_ptr++;
	assert(_numLinesToProcess);

	if (_vm->_features & GF_AMIGA)
		_palette_mod = 16;
	else
		_palette_mod = 0;

	switch (code) {
	case 1:
		unkDecode7();
		break;

	case 2:
		unkDecode8();								/* Ender - Zak256/Indy256 */
		break;

	case 3:
		unkDecode9();								/* Ender - Zak256/Indy256 */
		break;

	case 4:
		unkDecode10();							/* Ender - Zak256/Indy256 */
		break;

	case 7:
		unkDecode11();							/* Ender - Zak256/Indy256 */
		break;

	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
		_decomp_shr = code - 10;
		_decomp_mask = decompress_table[code - 10];
		unkDecode6();
		break;

	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
		_decomp_shr = code - 20;
		_decomp_mask = decompress_table[code - 20];
		unkDecode5();
		break;

	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
		_useOrDecompress = true;
		_decomp_shr = code - 30;
		_decomp_mask = decompress_table[code - 30];
		unkDecode4();
		break;

	case 44:
	case 45:
	case 46:
	case 47:
	case 48:
		_useOrDecompress = true;
		_decomp_shr = code - 40;
		_decomp_mask = decompress_table[code - 40];
		unkDecode2();
		break;

	case 64:
	case 65:
	case 66:
	case 67:
	case 68:
		_decomp_shr = code - 60;
		_decomp_mask = decompress_table[code - 60];
		unkDecode1();
		break;

	case 84:
	case 85:
	case 86:
	case 87:
	case 88:
		_useOrDecompress = true;
		_decomp_shr = code - 80;
		_decomp_mask = decompress_table[code - 80];
		unkDecode3();
		break;

		/* New since version 6 */
	case 104:
	case 105:
	case 106:
	case 107:
	case 108:
		_decomp_shr = code - 100;
		_decomp_mask = decompress_table[code - 100];
		unkDecode1();
		break;

		/* New since version 6 */
	case 124:
	case 125:
	case 126:
	case 127:
	case 128:
		_useOrDecompress = true;
		_decomp_shr = code - 120;
		_decomp_mask = decompress_table[code - 120];
		unkDecode3();
		break;

	default:
		error("Gdi::decompressBitmap: default case %d", code);
	}
}

int Scumm::hasCharsetMask(int x, int y, int x2, int y2)
{
	if (!charset._hasMask || y > gdi._mask_bottom || x > gdi._mask_right ||
			y2 < gdi._mask_top || x2 < gdi._mask_left)
		return 0;
	return 1;
}

void Gdi::draw8ColWithMasking()
{
	int height = _numLinesToProcess;
	byte *mask = _mask_ptr;
	byte *dst = _backbuff_ptr;
	byte *src = _bgbak_ptr;
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
			((uint32 *)dst)[0] = ((uint32 *)src)[0];
			((uint32 *)dst)[1] = ((uint32 *)src)[1];
		}
		src += _vm->_realWidth;
		dst += _vm->_realWidth;
		mask += _numStrips;
	} while (--height);
}

void Gdi::clear8ColWithMasking()
{
	int height = _numLinesToProcess;
	byte *mask = _mask_ptr;
	byte *dst = _backbuff_ptr;
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
			((uint32 *)dst)[0] = 0;
			((uint32 *)dst)[1] = 0;
		}
		dst += _vm->_realWidth;
		mask += _numStrips;
	} while (--height);
}

void Gdi::clear8Col()
{
	int height = _numLinesToProcess;
	byte *dst = _backbuff_ptr;

	do {
		((uint32 *)dst)[0] = 0;
		((uint32 *)dst)[1] = 0;
		dst += _vm->_realWidth;
	} while (--height);
}

void Gdi::decompressMaskImg()
{
	byte *src = _z_plane_ptr;
	byte *dst = _mask_ptr_dest;
	int height = _numLinesToProcess;
	byte b, c;
	
	if (_vm->_gameId == GID_DIG)
		height--;
				// FIXME: This seems to fix The Dig nexus wrapping corrupting memory..
				//	  and doesn't break any other games.. but is it correct? If so,
				//	  do we need to mirror this change anywhere else?
	while (1) {
		b = *src++;

		if (b & 0x80) {
			b &= 0x7F;
			c = *src++;

			do {
				*dst = c;
				dst += _numStrips;
				if (!--height)
					return;
			} while (--b);
		} else {
			do {
				*dst = *src++;
				dst += _numStrips;
				if (!--height)
					return;
			} while (--b);
		}
	}
}

void Gdi::decompressMaskImgOr()
{
	byte *src = _z_plane_ptr;
	byte *dst = _mask_ptr_dest;
	int height = _numLinesToProcess;
	byte b, c;

	while (1) {
		b = *src++;
		if (b & 0x80) {
			b &= 0x7F;
			c = *src++;

			do {
				*dst |= c;
				dst += _numStrips;
				if (!--height)
					return;
			} while (--b);
		} else {
			do {
				*dst |= *src++;
				dst += _numStrips;
				if (!--height)
					return;
			} while (--b);
		}
	}
}

void Scumm::redrawBGStrip(int start, int num)
{
	int s = _screenStartStrip + start;

	assert(s >= 0 && (size_t) s < sizeof(gfxUsageBits) / sizeof(gfxUsageBits[0]));

	_curVirtScreen = &virtscr[0];

	for (int i = 0; i < num; i++)
		gfxUsageBits[s + i] |= 0x80000000;

	/*if (_curVirtScreen->height < _scrHeight) {  
	   warning("Screen Y size %d < Room height %d",
	   _curVirtScreen->height,
	   _scrHeight);
	   } */

	gdi.drawBitmap(getResourceAddress(rtRoom, _roomResource) + _IM00_offs,
								 _curVirtScreen, s, 0, _curVirtScreen->height, s, num, 0);
}

#define READ_BIT (cl--,bit = bits&1, bits>>=1,bit)
#define FILL_BITS if (cl <= 8) { bits |= (*src++ << cl); cl += 8;}

void Gdi::unkDecode1()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	byte incm, reps;
	_tempNumLines = _numLinesToProcess;

	do {
		_currentX = 8;
		do {
			FILL_BITS *dst++ = color + _palette_mod;;

		againPos:;

			if (!READ_BIT) {
			} else if (READ_BIT) {
				incm = (bits & 7) - 4;
				cl -= 3;
				bits >>= 3;
				if (!incm) {
					FILL_BITS reps = bits & 0xFF;
					do {
						if (!--_currentX) {
							_currentX = 8;
							dst += _vm->_realWidth - 8;
							if (!--_tempNumLines)
								return;
						}
						*dst++ = color + _palette_mod;
					} while (--reps);
					bits >>= 8;
					bits |= (*src++) << (cl - 8);
					goto againPos;
				} else {
					color += incm;
				}
			} else {
				FILL_BITS color = bits & _decomp_mask;
				cl -= _decomp_shr;
				bits >>= _decomp_shr;
			}
		} while (--_currentX);
		dst += _vm->_realWidth - 8;
	} while (--_tempNumLines);
}

void Gdi::unkDecode2()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	byte color = *src++;
	int8 inc = -1;
	uint bits = *src++;
	byte cl = 8;
	byte bit;

	_tempNumLines = _numLinesToProcess;

	do {
		_currentX = 8;
		do {
			FILL_BITS if (color != _transparency)
				 *dst = color + _palette_mod;
			dst++;
			if (!READ_BIT) {
			} else if (!READ_BIT) {
				FILL_BITS color = bits & _decomp_mask;
				bits >>= _decomp_shr;
				cl -= _decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--_currentX);
		dst += _vm->_realWidth - 8;
	} while (--_tempNumLines);
}

void Gdi::unkDecode3()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	byte incm, reps;

	_tempNumLines = _numLinesToProcess;

	do {
		_currentX = 8;
		do {
			FILL_BITS if (color != _transparency)
				 *dst = color + _palette_mod;
			dst++;

		againPos:;
			if (!READ_BIT) {
			} else if (READ_BIT) {
				incm = (bits & 7) - 4;

				cl -= 3;
				bits >>= 3;
				if (incm) {
					color += incm;
				} else {
					FILL_BITS reps = bits & 0xFF;
					if (color == _transparency) {
						do {
							if (!--_currentX) {
								_currentX = 8;
								dst += _vm->_realWidth - 8;
								if (!--_tempNumLines)
									return;
							}
							dst++;
						} while (--reps);
					} else {
						do {
							if (!--_currentX) {
								_currentX = 8;
								dst += _vm->_realWidth - 8;
								if (!--_tempNumLines)
									return;
							}
							*dst++ = color + _palette_mod;
						} while (--reps);
					}
					bits >>= 8;
					bits |= (*src++) << (cl - 8);
					goto againPos;
				}
			} else {
				FILL_BITS color = bits & _decomp_mask;
				cl -= _decomp_shr;
				bits >>= _decomp_shr;
			}
		} while (--_currentX);
		dst += _vm->_realWidth - 8;
	} while (--_tempNumLines);
}

void Gdi::unkDecode4()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	byte color = *src++;
	int8 inc = -1;
	uint bits = *src++;
	byte cl = 8;
	byte bit;

	_currentX = 8;
	do {
		_tempNumLines = _numLinesToProcess;
		do {
			FILL_BITS if (color != _transparency)
				 *dst = color + _palette_mod;
			dst += _vm->_realWidth;
			if (!READ_BIT) {
			} else if (!READ_BIT) {
				FILL_BITS color = bits & _decomp_mask;
				bits >>= _decomp_shr;
				cl -= _decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--_tempNumLines);
		dst -= _vertStripNextInc;
	} while (--_currentX);
}

void Gdi::unkDecode5()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	byte color = *src++;
	int8 inc = -1;
	uint bits = *src++;
	byte cl = 8;
	byte bit;

	_tempNumLines = _numLinesToProcess;

	do {
		_currentX = 8;
		do {
			FILL_BITS *dst++ = color + _palette_mod;
			if (!READ_BIT) {
			} else if (!READ_BIT) {
				FILL_BITS color = bits & _decomp_mask;
				bits >>= _decomp_shr;
				cl -= _decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--_currentX);
		dst += _vm->_realWidth - 8;
	} while (--_tempNumLines);
}

void Gdi::unkDecode6()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	byte color = *src++;
	int8 inc = -1;
	uint bits = *src++;
	byte cl = 8;
	byte bit;

	_currentX = 8;
	do {
		_tempNumLines = _numLinesToProcess;
		do {
			FILL_BITS *dst = color + _palette_mod;
			dst += _vm->_realWidth;
			if (!READ_BIT) {
			} else if (!READ_BIT) {
				FILL_BITS color = bits & _decomp_mask;
				bits >>= _decomp_shr;
				cl -= _decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--_tempNumLines);
		dst -= _vertStripNextInc;
	} while (--_currentX);
}

/* Ender - Zak256/Indy256 decoders */
#define READ_256BIT \
 if ((mask <<= 1) == 256) {buffer = *src++;  mask = 1;}     \
 bits = ((buffer & mask) != 0);

#define NEXT_ROW                                               \
                dst += _vm->_realWidth;                         \
                if (--h == 0) {                                 \
                        if (!--_currentX)                       \
                                return;                         \
                        dst -= _vertStripNextInc;               \
                        h = _numLinesToProcess;                 \
                }

void Gdi::unkDecode7()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	int height = _numLinesToProcess;
	uint h = _numLinesToProcess;


	if (_vm->_features & GF_OLD256) {
		_currentX = 8;
		for (;;) {
			byte color = *src++;
			*dst = color;
		NEXT_ROW}
		return;
	}

	do {
		/* Endian safe */
#if defined(SCUMM_NEED_ALIGNMENT)
		memcpy(dst, src, 8);
#else
		((uint32 *)dst)[0] = ((uint32 *)src)[0];
		((uint32 *)dst)[1] = ((uint32 *)src)[1];
#endif
		dst += _vm->_realWidth;
		src += 8;
	} while (--height);
}

void Gdi::unkDecode8()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	uint h = _numLinesToProcess;

	_currentX = 8;
	for (;;) {
		uint run = (*src++) + 1;
		byte color = *src++;

		do {
			*dst = color;
		NEXT_ROW} while (--run);
	}
}

void Gdi::unkDecode9()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	unsigned char c, bits, color, run;
	int x, y, i, z;
	uint buffer = 0, mask = 128;
	int h = _numLinesToProcess;
	x = y = i = z = run = 0;

	_currentX = 8;
	for (;;) {
		c = 0;
		for (i = 0; i < 4; i++) {
			READ_256BIT;
			c += (bits << i);
		}

		switch ((c >> 2)) {
		case 0:
			color = 0;
			for (i = 0; i < 4; i++) {
				READ_256BIT;
				color += bits << i;
			}
			for (i = 0; i < ((c & 3) + 2); i++) {
				*dst = (run * 16 + color);
			NEXT_ROW}
			break;

		case 1:
			for (i = 0; i < ((c & 3) + 1); i++) {
				color = 0;
				for (z = 0; z < 4; z++) {
					READ_256BIT;
					color += bits << z;
				}
				*dst = (run * 16 + color);
			NEXT_ROW}
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

void Gdi::unkDecode10()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	int i;
	unsigned char local_palette[256], numcolors = *src++;
	uint h = _numLinesToProcess;

	for (i = 0; i < numcolors; i++)
		local_palette[i] = *src++;

	_currentX = 8;

	for (;;) {
		byte color = *src++;
		if (color < numcolors) {
			*dst = local_palette[color];
		NEXT_ROW} else {
			uint run = color - numcolors + 1;
			color = *src++;
			do {
				*dst = color;
			NEXT_ROW} while (--run);
		}
	}
}


void Gdi::unkDecode11()
{
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	int bits, i;
	uint buffer = 0, mask = 128;
	unsigned char inc = 1, color = *src++;

	_currentX = 8;
	do {
		_tempNumLines = _numLinesToProcess;
		do {
			*dst = color;
			dst += _vm->_realWidth;
			for (i = 0; i < 3; i++) {
				READ_256BIT if (!bits)
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
					READ_256BIT color += bits << i;
				}
				break;
			}
		} while (--_tempNumLines);
		dst -= _vertStripNextInc;
	} while (--_currentX);
}


#undef NEXT_ROW
#undef READ_256BIT
#undef READ_BIT
#undef FILL_BITS

void Scumm::restoreCharsetBg()
{
	_bkColor = 0;

	if (gdi._mask_left != -1) {
		restoreBG(gdi._mask_left, gdi._mask_top, gdi._mask_right, gdi._mask_bottom);
		charset._hasMask = false;
		gdi._mask_left = -1;
		charset._strLeft = -1;
		charset._left = -1;
	}

	charset._xpos2 = _string[0].xpos;
	charset._ypos2 = _string[0].ypos;
}

void Scumm::restoreBG(int left, int top, int right, int bottom)
{
	VirtScreen *vs;
	int topline, height, width, widthmod;
	byte *backbuff, *bgbak, *mask;

	if (left == right || top == bottom)
		return;
	if (top < 0)
		top = 0;

	if ((vs = findVirtScreen(top)) == NULL)
		return;

	topline = vs->topline;
	height = topline + vs->height;
	if (vs->number == 0) {
		left += _lastXstart - vs->xstart;
		right += _lastXstart - vs->xstart;
	}

	right++;
	if (left < 0)
		left = 0;
	if (right < 0)
		right = 0;
	if (left > _realWidth)
		return;
	if (right > _realWidth)
		right = _realWidth;
	if (bottom >= height)
		bottom = height;

	updateDirtyRect(vs->number, left, right, top - topline, bottom - topline, 0x40000000);

	height = (top - topline) * _realWidth + vs->xstart + left;

	backbuff = vs->screenPtr + height;
	bgbak = getResourceAddress(rtBuffer, vs->number + 5) + height;
	mask = getResourceAddress(rtBuffer, 9) + top * gdi._numStrips + (left >> 3) + _screenStartStrip;
	if (vs->number == 0) {
		// FIXME: hardcoded value
		mask += vs->topline * 216;
	}

	height = bottom - top;
	width = right - left;
	widthmod = (width >> 2) + 2;

	if (vs->alloctwobuffers && _currentRoom != 0 /*&& _vars[VAR_V5_DRAWFLAGS]&2 */ ) {
		blit(backbuff, bgbak, width, height);
		if (vs->number == 0 && charset._hasMask && height) {
			do {
				memset(mask, 0, widthmod);
				mask += gdi._numStrips;
			} while (--height);
		}
	} else {
		if (height) {
			do {
				memset(backbuff, _bkColor, width);
				backbuff += _realWidth;
			} while (--height);
		}
	}
}

void Scumm::updateDirtyRect(int virt, int left, int right, int top, int bottom, uint32 dirtybits)
{
	VirtScreen *vs = &virtscr[virt];
	int lp, rp;
	uint32 *sp;
	int num;

	if (top > vs->height || left > vs->width || right < 0 || bottom < 0)
		return;

	if (top < 0)
		top = 0;
	if (left < 0)
		left = 0;
	if (bottom > vs->height)
		bottom = vs->height;
	if (right > vs->width)
		right = vs->width;

	if (virt == 0 && dirtybits) {
		rp = (right >> 3) + _screenStartStrip;
		lp = (left >> 3) + _screenStartStrip;
		if (lp < 0)
			lp = 0;
		if (rp >= 240)
			rp = 240;
		if (lp <= rp) {
			num = rp - lp + 1;
			sp = &gfxUsageBits[lp];
			do {
				*sp++ |= dirtybits;
			} while (--num);
		}
	}

	setVirtscreenDirty(vs, left, top, right, bottom);
}

void Scumm::setVirtscreenDirty(VirtScreen *vs, int left, int top, int right, int bottom)
{
	int lp = left >> 3;
	int rp = right >> 3;

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

VirtScreen *Scumm::findVirtScreen(int y)
{
	VirtScreen *vs = virtscr;
	int i;

	for (i = 0; i < 3; i++, vs++) {
		if (y >= vs->topline && y < vs->topline + vs->height) {
			return _curVirtScreen = vs;
		}
	}
	return _curVirtScreen = NULL;
}

void Scumm::unkScreenEffect1()
{
	/* XXX: not implemented */
	warning("stub unkScreenEffect1()");
}

void Scumm::unkScreenEffect2()
{
	/* XXX: not implemented */
	warning("stub unkScreenEffect2()");
}

void Scumm::unkScreenEffect3()
{
	/* XXX: not implemented */
	warning("stub unkScreenEffect3()");
}

void Scumm::unkScreenEffect4()
{
	/* XXX: not implemented */
	warning("stub unkScreenEffect4()");
}

/* *INDENT-OFF* */

static const int8 screen_eff7_table1[4][16] = {
	{ 1,  1, -1,  1, -1,  1, -1, -1,
	  1, -1, -1, -1,  1,  1,  1, -1},
	{ 0,  1,  2,  1,  2,  0,  2,  1,
	  2,  0,  2,  1,  0,  0,  0,  0},
	{-2, -1,  0, -1, -2, -1, -2,  0, -2, -1, -2, 0, 0, 0, 0, 0},
	{ 0, -1, -2, -1, -2,  0, -2, -1, -2, 0, -2, -1, 0, 0, 0, 0}
};

static const byte screen_eff7_table2[4][16] = {
	{ 0,  0, 39,  0,  39,  0, 39, 24,
	  0, 24, 39, 24,   0,  0,  0, 24},
	{ 0,  0,  0,  0,   0,  0,  0,  0,
	  1,  0,  1,  0, 255,  0,  0,  0},
	{39, 24, 39, 24,  39, 24, 39, 24,
	 38, 24, 38, 24, 255,  0,  0,  0},
	{ 0, 24, 39, 24,  39,  0, 39, 24,
	 38,  0, 38, 24, 255,  0,  0,  0}
};

static const byte transition_num_of_iterations[4] = {
	13, 25, 25, 25
};

/* *INDENT-ON* */

/* Transition effect. There are four different effects possible,
 * indicated by the value of a:
 * 0: Iris effect
 * 1: ?
 * 2: ?
 * 3: ?
 * All effects basically operate on 8x8 blocks of the screen. These blocks
 * are updated in a certain order; the exact order determines how the
 * effect appears to the user.
 */
void Scumm::transitionEffect(int a)
{
	int delta[16];								// Offset applied during each iteration
	int tab_2[16];
	int i, j;
	int bottom;
	int l, t, r, b;

	for (i = 0; i < 16; i++) {
		delta[i] = screen_eff7_table1[a][i];
		j = screen_eff7_table2[a][i];
		if (j == 24)
			j = (virtscr[0].height >> 3) - 1;
		tab_2[i] = j;
	}

	bottom = virtscr[0].height >> 3;
	for (j = 0; j < transition_num_of_iterations[a]; j++) {
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
				virtscr[0].tdirty[l] = t << 3;
				virtscr[0].bdirty[l] = (b + 1) << 3;
			}
			updateDirtyScreen(0);
		}

		for (i = 0; i < 16; i++)
			tab_2[i] += delta[i];

		// Draw the current state to the screen and wait half a sec so the user
		// can watch the effect taking place.
		updatePalette();
		_system->update_screen();
		waitForTimer(30);
	}
}

// Update width x height areas of the screen, in random order, until the whole
// screen has been updated. For instance:
//
// dissolveEffect(1, 1) produces a pixel-by-pixel dissolve
// dissolveEffect(8, 8) produces a square-by-square dissolve
// dissolveEffect(virtsrc[0].width, 1) produces a line-by-line dissolve

void Scumm::dissolveEffect(int width, int height) {
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

	// When used used correctly, vs->width % width and vs->height % height
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

			j = getRandomNumber(i - 1);
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

			j = getRandomNumber(i - 1);
			offsets[i] = offsets[j];
			offsets[j] = offsets2[i];
		}

		free(offsets2);
	}

	// Blit the image piece by piece to the screen. The idea here is that
	// the whole update should take about a quarter of a second, assuming
	// most of the time is spent in waitForTimer(). It looks good to me,
	// but might still need some tuning.

	updatePalette();

	blits = 0;
	blits_before_refresh = (3 * w * h) / 25;

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

void Scumm::scrollEffect(int dir) {

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


void Scumm::unkScreenEffect6() {
	if (_gameId == GID_LOOM256)
		dissolveEffect(1, 1);
	else
		dissolveEffect(8, 4);
}

void Scumm::unkScreenEffect5(int a) {
	// unkScreenEffect5(0), which is used by FOA during the opening
	// cutscene when Indy opens the small statue, has been replaced by
	// dissolveEffect(1, 1).
	//
	// I still don't know what unkScreenEffect5(1) is supposed to do.

	/* XXX: not implemented */
	warning("stub unkScreenEffect(%d)", a);
}

void Scumm::setShake(int mode)
{
	if (_shakeEnabled != (mode != 0))
		_fullRedraw = true;

	_shakeEnabled = mode != 0;
	_shakeFrame = 0;
	_system->set_shake_pos(0);
}

void Gdi::clearUpperMask()
{
	memset(_vm->getResourceAddress(rtBuffer, 9), 0, _imgBufOffs[1] - _imgBufOffs[0]
		);
}

void Scumm::clampCameraPos(ScummPoint *pt)
{
	if (pt->x < _vars[VAR_CAMERA_MIN_X])
		pt->x = _vars[VAR_CAMERA_MIN_X];

	if (pt->x > _vars[VAR_CAMERA_MAX_X])
		pt->x = _vars[VAR_CAMERA_MAX_X];

	if (pt->y < _vars[VAR_CAMERA_MIN_Y])
		pt->y = _vars[VAR_CAMERA_MIN_Y];

	if (pt->y > _vars[VAR_CAMERA_MAX_Y])
		pt->y = _vars[VAR_CAMERA_MAX_Y];
}


void Scumm::moveCamera()
{
	if (_features & GF_AFTER_V7) {
		ScummPoint old = camera._cur;
		Actor *a = NULL;

		if (camera._follows) {
			a = derefActorSafe(camera._follows, "moveCamera");
			if (abs(camera._cur.x - a->x) > _vars[VAR_CAMERA_THRESHOLD_X] ||
					abs(camera._cur.y - a->y) > _vars[VAR_CAMERA_THRESHOLD_Y]) {
				camera._movingToActor = true;
				if (_vars[VAR_CAMERA_THRESHOLD_X] == 0)
					camera._cur.x = a->x;
				if (_vars[VAR_CAMERA_THRESHOLD_Y] == 0)
					camera._cur.y = a->y;
				clampCameraPos(&camera._cur);
			}
		} else {
			camera._movingToActor = false;
		}

		if (camera._movingToActor) {
			camera._dest.x = a->x;
			camera._dest.y = a->y;
		}

		assert(camera._cur.x >= (_realWidth / 2) && camera._cur.y >= (_realHeight / 2));

		clampCameraPos(&camera._dest);

		if (camera._cur.x < camera._dest.x) {
			camera._cur.x += _vars[VAR_CAMERA_SPEED_X];
			if (camera._cur.x > camera._dest.x)
				camera._cur.x = camera._dest.x;
		}

		if (camera._cur.x > camera._dest.x) {
			camera._cur.x -= _vars[VAR_CAMERA_SPEED_X];
			if (camera._cur.x < camera._dest.x)
				camera._cur.x = camera._dest.x;
		}

		if (camera._cur.y < camera._dest.y) {
			camera._cur.y += _vars[VAR_CAMERA_SPEED_Y];
			if (camera._cur.y > camera._dest.y)
				camera._cur.y = camera._dest.y;
		}

		if (camera._cur.y > camera._dest.y) {
			camera._cur.y -= _vars[VAR_CAMERA_SPEED_Y];
			if (camera._cur.y < camera._dest.y)
				camera._cur.y = camera._dest.y;
		}

		if (camera._cur.x == camera._dest.x && camera._cur.y == camera._dest.y) {

			camera._movingToActor = false;
			camera._accel.x = camera._accel.y = 0;
			_vars[VAR_CAMERA_SPEED_X] = _vars[VAR_CAMERA_SPEED_Y] = 0;
		} else {

			camera._accel.x += _vars[VAR_CAMERA_ACCEL_X];
			camera._accel.y += _vars[VAR_CAMERA_ACCEL_Y];

			_vars[VAR_CAMERA_SPEED_X] += camera._accel.x / 100;
			_vars[VAR_CAMERA_SPEED_Y] += camera._accel.y / 100;

			if (_vars[VAR_CAMERA_SPEED_X] < 8)
				_vars[VAR_CAMERA_SPEED_X] = 8;

			if (_vars[VAR_CAMERA_SPEED_Y] < 8)
				_vars[VAR_CAMERA_SPEED_Y] = 8;

		}

		cameraMoved();

		if (camera._cur.x != old.x || camera._cur.y != old.y) {
			_vars[VAR_CAMERA_POS_X] = camera._cur.x;
			_vars[VAR_CAMERA_POS_Y] = camera._cur.y;

			_vars[VAR_CAMERA_DEST_X] = camera._dest.x;

			_vars[VAR_CAMERA_DEST_Y] = camera._dest.y;

			_vars[VAR_CAMERA_FOLLOWED_ACTOR] = camera._follows;

			if (_vars[VAR_SCROLL_SCRIPT])
				runScript(_vars[VAR_SCROLL_SCRIPT], 0, 0, 0);
		}
	} else {
		int pos = camera._cur.x;
		int actorx, t;
		Actor *a = NULL;

		camera._cur.x &= 0xFFF8;

		if (camera._cur.x < _vars[VAR_CAMERA_MIN_X]) {
			if (_vars[VAR_CAMERA_FAST_X])
				camera._cur.x = _vars[VAR_CAMERA_MIN_X];
			else
				camera._cur.x += 8;
			cameraMoved();
			return;
		}

		if (camera._cur.x > _vars[VAR_CAMERA_MAX_X]) {
			if (_vars[VAR_CAMERA_FAST_X])
				camera._cur.x = _vars[VAR_CAMERA_MAX_X];
			else
				camera._cur.x -= 8;
			cameraMoved();
			return;
		}

		if (camera._mode == CM_FOLLOW_ACTOR) {
			a = derefActorSafe(camera._follows, "moveCamera");

			actorx = a->x;
			t = (actorx >> 3) - _screenStartStrip;

			if (t < camera._leftTrigger || t > camera._rightTrigger) {
				if (_vars[VAR_CAMERA_FAST_X]) {
					if (t > 35)
						camera._dest.x = actorx + 80;
					if (t < 5)
						camera._dest.x = actorx - 80;
				} else
					camera._movingToActor = 1;
			}
		}

		if (camera._movingToActor) {
			a = derefActorSafe(camera._follows, "moveCamera(2)");
			camera._dest.x = a->x;
		}

		if (camera._dest.x < _vars[VAR_CAMERA_MIN_X])
			camera._dest.x = _vars[VAR_CAMERA_MIN_X];

		if (camera._dest.x > _vars[VAR_CAMERA_MAX_X])
			camera._dest.x = _vars[VAR_CAMERA_MAX_X];

		if (_vars[VAR_CAMERA_FAST_X]) {
			camera._cur.x = camera._dest.x;
		} else {
			if (camera._cur.x < camera._dest.x)
				camera._cur.x += 8;
			if (camera._cur.x > camera._dest.x)
				camera._cur.x -= 8;
		}

		/* a is set a bit above */
		if (camera._movingToActor && camera._cur.x >> 3 == a->x >> 3) {
			camera._movingToActor = 0;
		}

		cameraMoved();

		if (pos != camera._cur.x && _vars[VAR_SCROLL_SCRIPT]) {
			_vars[VAR_CAMERA_POS_X] = camera._cur.x;
			runScript(_vars[VAR_SCROLL_SCRIPT], 0, 0, 0);
		}
	}
}

void Scumm::cameraMoved()
{
	if (_features & GF_AFTER_V7) {

		assert(camera._cur.x >= (_realWidth / 2) && camera._cur.y >= (_realHeight / 2));

		_screenLeft = camera._cur.x - (_realWidth / 2);
		_screenTop = camera._cur.y - (_realHeight / 2);
	} else {

		if (camera._cur.x < (_realWidth / 2)) {
			camera._cur.x = (_realWidth / 2);
		} else if (camera._cur.x + (_realWidth / 2) >= _scrWidth) {
			camera._cur.x = _scrWidth - (_realWidth / 2);
		}
	}

	_screenStartStrip = (camera._cur.x - (_realWidth / 2)) >> 3;
	_screenEndStrip = _screenStartStrip + gdi._numStrips - 1;
	virtscr[0].xstart = _screenStartStrip << 3;
}

void Scumm::panCameraTo(int x, int y)
{
	if (_features & GF_AFTER_V7) {

		camera._follows = 0;
		camera._dest.x = x;
		camera._dest.y = y;
	} else {

		camera._dest.x = x;
		camera._mode = CM_PANNING;
		camera._movingToActor = 0;
	}
}

void Scumm::actorFollowCamera(int act)
{
	if (!(_features & GF_AFTER_V7)) {
		int old;

		/* mi1 compatibilty */
		if (act == 0) {
			camera._mode = CM_NORMAL;
			camera._follows = 0;
			camera._movingToActor = 0;
			return;
		}

		old = camera._follows;
		setCameraFollows(derefActorSafe(act, "actorFollowCamera"));
		if (camera._follows != old)
			runHook(0);

		camera._movingToActor = 0;
	}
}

void Scumm::setCameraAtEx(int at)
{
	if (!(_features & GF_AFTER_V7)) {
		camera._mode = CM_NORMAL;
		camera._cur.x = at;
		setCameraAt(at, 0);
		camera._movingToActor = 0;
	}
}

void Scumm::palManipulateInit(int start, int end, int string_id, int time)
{
	byte *pal, *target, *between;
	byte *string1, *string2, *string3;
	int i;

	string1 = getStringAddress(string_id);
	string2 = getStringAddress(string_id + 1);
	string3 = getStringAddress(string_id + 2);
	if (!string1 || !string2 || !string3) {
		warning("palManipulateInit(%d,%d,%d,%d): Cannot obtain string resources %d, %d and %d\n",
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
		*(uint16*)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16*)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16*)between = ((uint16) *pal++) << 8;
		between += 2;
	}

	_palManipCounter = time;
}

void Scumm::palManipulate()
{
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

void Scumm::unkRoomFunc3(int palstart, int palend, int rfact, int gfact, int bfact)
{
	byte *basepal = getPalettePtr();
	byte *pal = basepal;
	byte *compareptr;
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
		int r = (int) (*pal++ * rfact) >> 8;
		int g = (int) (*pal++ * gfact) >> 8;
		int b = (int) (*pal++ * bfact) >> 8;

		// The following functionality is similar to remapPaletteColor, except
		// 1) we have to work off the original CLUT rather than the current palette, and
		// 2) the target shadow palette entries must be bounded to the upper and lower
		//    bounds provided by the opcode. (This becomes significant in Room 48, but
		//    is not an issue in all other known case studies.)
		int j;
		int ar, ag, ab;
		uint sum, diff, bestsum, bestitem = 0;
		compareptr = basepal + palstart * 3;

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

		for (j = palstart; j <= palend; j++, compareptr += 3) {
			ar = compareptr[0] & ~3;
			ag = compareptr[1] & ~3;
			ab = compareptr[2] & ~3;
			if (ar == r && ag == g && ab == b) {
				bestitem = j;
				break;
			}

			diff = ar - r;
			sum = diff * diff * 3;
			diff = ag - g;
			sum += diff * diff * 6;
			diff = ab - b;
			sum += diff * diff * 2;

			if (sum < bestsum) {
				bestsum = sum;
				bestitem = j;
			}
		}
		*table++ = bestitem;
	}
}

void Scumm::swapPalColors(int a, int b)
{
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

void Scumm::copyPalColor(int dst, int src)
{
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

void Gdi::resetBackground(int top, int bottom, int strip)
{
	VirtScreen *vs = &_vm->virtscr[0];
	int offs;

	if (top < vs->tdirty[strip])
		vs->tdirty[strip] = top;

	if (bottom > vs->bdirty[strip])
		vs->bdirty[strip] = bottom;

	offs = (top * _numStrips + _vm->_screenStartStrip + strip);
	_mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + offs;
	_bgbak_ptr = _vm->getResourceAddress(rtBuffer, 5) + (offs << 3);
	_backbuff_ptr = vs->screenPtr + (offs << 3);

	_numLinesToProcess = bottom - top;
	if (_numLinesToProcess) {
		if ((_vm->_features & GF_AFTER_V6) || (_vm->_vars[_vm->VAR_CURRENT_LIGHTS] & LIGHTMODE_screen)) {
			if (_vm->hasCharsetMask(strip << 3, top, (strip + 1) << 3, bottom))
				draw8ColWithMasking();
			else
				_vm->blit(_backbuff_ptr, _bgbak_ptr, 8, _numLinesToProcess);
		} else {
			clear8Col();
		}
	}
}

void Scumm::setPalColor(int idx, int r, int g, int b)
{
	_currentPalette[idx * 3 + 0] = r;
	_currentPalette[idx * 3 + 1] = g;
	_currentPalette[idx * 3 + 2] = b;
	setDirtyColors(idx, idx);
}

void Scumm::setCursorHotspot2(int x, int y)
{
	_cursorHotspotX = x;
	_cursorHotspotY = y;
}

byte Scumm::isMaskActiveAt(int l, int t, int r, int b, byte *mem)
{
	int w, h, i;

	l >>= 3;
	if (l < 0)
		l = 0;
	if (t < 0)
		t = 0;

	r >>= 3;
	if (r > gdi._numStrips - 1)
		r = gdi._numStrips - 1;

	mem += l + t * gdi._numStrips;

	w = r - l;
	h = b - t + 1;

	do {
		for (i = 0; i <= w; i++)
			if (mem[i]) {
				return true;
			}
		mem += gdi._numStrips;
	} while (--h);

	return false;
}

void Scumm::setPalette(int palindex)
{
	byte *pals;

	_curPalIndex = palindex;
	pals = getPalettePtr();
	if (pals == NULL)
		error("invalid palette %d", palindex);
	setPaletteFromPtr(pals);
}

byte *Scumm::findPalInPals(byte *pal, int idx)
{
	byte *offs;
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

byte *Scumm::getPalettePtr()
{
	byte *cptr;

	cptr = getResourceAddress(rtRoom, _roomResource);
	if (_CLUT_offs) {
		cptr += _CLUT_offs;
	} else {
		cptr = findPalInPals(cptr + _PALS_offs, _curPalIndex);
	}
	return cptr;
}

void Scumm::darkenPalette(int startColor, int endColor, int redScale, int greenScale, int blueScale)
{
	if (startColor <= endColor) {
		byte *cptr, *cur;
		int num;
		int color;

		cptr = getPalettePtr() + startColor * 3;
		cur = _currentPalette + startColor * 3;
		num = endColor - startColor + 1;

		do {
			color = *cptr++;
			if (redScale != 0xFF)
				color = color * redScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;

			color = *cptr++;
			if (greenScale != 0xFF)
				color = color * greenScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;

			color = *cptr++;
			if (blueScale != 0xFF)
				color = color * blueScale / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;
		} while (--num);
		setDirtyColors(startColor, endColor);
	}
}

void Scumm::grabCursor(int x, int y, int w, int h)
{
	VirtScreen *vs = findVirtScreen(y);

	if (vs == NULL) {
		warning("grabCursor: invalid Y %d", y);
		return;
	}

	grabCursor(vs->screenPtr + (y - vs->topline) * _realWidth + x, w, h);

}

void Scumm::decompressBomp(byte *dst, byte *src, int w, int h)
{
	int len, num;
	byte code, color;

	src += 8;

	do {
		len = w;
		src += 2;
		while (len) {
			code = *src++;
			num = (code >> 1) + 1;
			if (num > len)
				num = len;
			len -= num;
			if (code & 1) {
				color = *src++;
				do
					*dst++ = color;
				while (--num);
			} else {
				do
					*dst++ = *src++;
				while (--num);
			}
		}
	} while (--h);
}

void Scumm::grabCursor(byte *ptr, int width, int height)
{
	uint size;
	byte *dst;

	size = width * height;
	if (size > sizeof(_grabbedCursor))
		error("grabCursor: grabbed cursor too big");

	_cursorWidth = width;
	_cursorHeight = height;
	_cursorAnimate = 0;

	dst = _grabbedCursor;
	for (; height; height--) {
		memcpy(dst, ptr, width);
		dst += width;
		ptr += _realWidth;
	}

	updateCursor();
}

void Scumm::useIm01Cursor(byte *im, int w, int h)
{
	VirtScreen *vs = &virtscr[0];

	w <<= 3;
	h <<= 3;

	drawBox(0, 0, w - 1, h - 1, 0xFF);

	vs->alloctwobuffers = false;
	gdi._disable_zbuffer = true;
	gdi.drawBitmap(im, vs, _screenStartStrip, 0, h, 0, w >> 3, 0);
	vs->alloctwobuffers = true;
	gdi._disable_zbuffer = false;

	grabCursor(vs->screenPtr + vs->xstart, w, h);

	blit(vs->screenPtr + vs->xstart, getResourceAddress(rtBuffer, 5) + vs->xstart, w, h);
}

void Scumm::updateCursor()
{
	_system->set_mouse_cursor(_grabbedCursor, _cursorWidth, _cursorHeight,
														_cursorHotspotX, _cursorHotspotY);
}

void Scumm::animateCursor()
{
	if (_cursorAnimate) {
		if (!(_cursorAnimateIndex & 0x3)) {
			decompressDefaultCursor((_cursorAnimateIndex >> 2) & 3);
		}
		_cursorAnimateIndex++;
	}

}

void Scumm::useBompCursor(byte *im, int width, int height)
{
	uint size;

	width <<= 3;
	height <<= 3;

	size = width * height;
	if (size > sizeof(_grabbedCursor))
		error("useBompCursor: cursor too big");

	_cursorWidth = width;
	_cursorHeight = height;
	_cursorAnimate = 0;

	decompressBomp(_grabbedCursor, im + 10, width, height);

	updateCursor();
}

static const byte default_cursor_colors[4] = {
	15, 15, 7, 8
};

static const uint16 default_cursor_images[4][16] = {
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
};

static const byte default_cursor_hotspots[8] = {
	8, 7,   8, 7,   1, 1,   5, 0
};

void Scumm::decompressDefaultCursor(int idx)
{
	int i, j;
	byte color;

	memset(_grabbedCursor, 0xFF, sizeof(_grabbedCursor));

	color = default_cursor_colors[idx];

	// FIXME: None of the stock cursors are right for Loom. Why is that?

	if (_gameId == GID_LOOM256) {
		int w;

		_cursorWidth = 8;
		_cursorHeight = 8;
		_cursorHotspotX = 0;
		_cursorHotspotY = 0;

		for (i = 0, w = 0; i < 8; i++) {
			w += (i >= 6) ? -2 : 1;
			for (j = 0; j < w; j++)
				_grabbedCursor[i * 8 + j] = color;
		}
	} else {
		_cursorWidth = 16;
		_cursorHeight = 16;
		_cursorHotspotX = default_cursor_hotspots[2 * gdi._currentCursor];
		_cursorHotspotY = default_cursor_hotspots[2 * gdi._currentCursor + 1];

		for (i = 0; i < 16; i++) {
			for (j = 0; j < 16; j++) {
				if (default_cursor_images[gdi._currentCursor][i] & (1 << j))
					_grabbedCursor[16 * i + 15 - j] = color;
			}
		}
	}

	updateCursor();
}

void Scumm::makeCursorColorTransparent(int a)
{
	int i, size;

	size = _cursorWidth * _cursorHeight;

	for (i = 0; i < size; i++)
		if (_grabbedCursor[i] == (byte)a)
			_grabbedCursor[i] = 0xFF;

	updateCursor();
}

int Scumm::remapPaletteColor(int r, int g, int b, uint threshold)
{
	int i;
	int ar, ag, ab;
	uint sum, j, bestsum, bestitem = 0;
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

		j = ar - r;
		sum = j * j * 3;
		j = ag - g;
		sum += j * j * 6;
		j = ab - b;
		sum += j * j * 2;

		if (sum < bestsum) {
			bestsum = sum;
			bestitem = i;
		}
	}

	if (threshold != (uint) - 1 && bestsum > threshold * threshold * (2 + 3 + 6)) {
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

static int blend_cache[3][256];

static void clear_blend_cache()
{
	int i, j;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 256; j++)
			blend_cache[i][j] = -1;
}

static byte blend(byte *pal, byte method, int dest_color)
{
	int val = 0;
	int cache = 0;

	// FIXME: Check if this gives the correct blending for the Dig
	// inventory box and conversation menus. For now, I have deliberately
	// selected them so that the subsequent multiplication and shift could
	// be replaced by just a shift.

	switch (method) {
		case 1:
			cache = 0;
			val = 128;
			break;

		case 2:
			cache = 1;
			val = 64;
			break;

		case 3:
			cache = 2;
			val = 256;
			break;

		case 255:
			return dest_color;

		default:
			return method;
	}

	if (blend_cache[cache][dest_color] == -1) {
		byte r = *(pal + 3 * dest_color + 0);
		byte g = *(pal + 3 * dest_color + 1);
		byte b = *(pal + 3 * dest_color + 2);

		int new_r = (val * r) >> 8;
		int new_g = (val * g) >> 8;
		int new_b = (val * b) >> 8;

		if (new_r > 255)
			new_r = 255;
		if (new_g > 255)
			new_g = 255;
		if (new_b > 255)
			new_g = 255;

		blend_cache[cache][dest_color] = RGBMatch(pal, new_r, new_g, new_b);
	}

	return blend_cache[cache][dest_color];
}

// param3= clipping
// param2= mirror
// param1= never used ?

void Scumm::drawBomp(BompDrawData *bd, int param1, byte *dataPtr, int param2, int param3)
{
	byte *scale_rows = NULL;
	byte *scale_cols = NULL;
	byte *dest = bd->out + bd->y * bd->outwidth, *src;
	int src_x, src_y, dst_x, dst_y;
	uint scaled_width, scaled_height;
	int h = bd->srcheight;
	byte *mask = NULL;
	uint i;

	if (h == 0 || bd->srcwidth == 0)
		return;

	if (bd->scale_x != 255) {
		scale_rows = (byte *) calloc(bd->srcheight, 1);
		if (scale_rows == NULL) {
			warning("drawBomp: out of memory");
			return;
		}
	}

	if (bd->scale_y != 255) {	
		scale_cols = (byte *) calloc(bd->srcwidth, 1);
		if (scale_cols == NULL) {
			warning("drawBomp: out of memory");
			if (scale_rows)
				free(scale_rows);
			return;
		}
	}

	// We take charset masking into consideration, because otherwise the
	// inventory window in The Dig may overwrite text.

	mask = getResourceAddress(rtBuffer, 9) + _screenStartStrip;
	
	// Select which rows and columns from the original to show in the
	// scaled version of the image. This is a pretty stupid way of scaling
	// images, but it will have to do for now.

	if (bd->scale_x < 255) {
		scaled_width = (bd->srcwidth * bd->scale_x) / 255;
		for (i = 0; i < scaled_width; i++)
			scale_cols[(i * 255) / bd->scale_x] = 1;
	}

	if (bd->scale_y < 255) {
		scaled_height = (bd->srcheight * bd->scale_y) / 255;
		for (i = 0; i < scaled_height; i++)
			scale_rows[(i * 255) / bd->scale_y] = 1;
	}

	// FIXME: Be more intelligent about clearing the blend cache. It
	// should be possible to clear it only for the parts of the palette
	// that have changed since the last time.

	clear_blend_cache();

	dest += bd->x;
	src = bd->dataptr;

	for (src_y = 0, dst_y = bd->y; src_y < bd->srcheight; src_y++) {
		byte code, color;
		uint len, num;
		byte *d = dest;

		if ((dst_y < 0 || dst_y >= bd->outheight) || (bd->scale_y != 255 && !scale_rows[src_y])) {
			src += READ_LE_UINT16(src) + 2;
			continue;
		}

		len = bd->srcwidth;
		src_x = 0;
		dst_x = bd->x;
		src += 2;
		
		while (src_x < bd->srcwidth) {
			code = *src++;
			num = (code >> 1) + 1;
			if (num > len)
				num = len;
			len -= num;
			if (code & 1) {
				color = *src++;
				for (i = 0; i < num; i++) {
					if (bd->scale_x == 255 || scale_cols[src_x]) {
						if (dst_x >= 0 && dst_x < bd->outwidth) {
							if (!(*(mask + dst_y * gdi._numStrips + (dst_x >> 3)) & revBitMask[dst_x & 7]))
							
								*d = blend(_currentPalette, color, *d);
						}
						d++;
						dst_x++;
					}
					src_x++;
				}
			} else {
				for (i = 0; i < num; i++) {
					if (bd->scale_x == 255 || scale_cols[src_x]) {
						if (dst_x >= 0 && dst_x < bd->outwidth)
							if (!(*(mask + dst_y * gdi._numStrips + (dst_x >> 3)) & revBitMask[dst_x & 7]))
								*d = blend(_currentPalette, src[i], *d);
						d++;
						dst_x++;
					}
					src_x++;
				}
				src += num;
			}
		}
		dest += bd->outwidth;
		dst_y++;
		
	}

	if (scale_rows)
		free(scale_rows);
	if (scale_cols)
		free(scale_cols);
	CHECK_HEAP;
}


/* Yazoo: This function create the specialPalette used for semi-transparency in SamnMax */


void Scumm::createSpecialPalette(int16 a, int16 b, int16 c, int16 d, int16 e, int16 colorMin,
																 int16 colorMax)
{
	byte *palPtr;
	byte *curPtr;
	byte *searchPtr;

	byte readComp1;
	byte readComp2;
	byte readComp3;

	int colorComp1;
	int colorComp2;
	int colorComp3;

	int searchComp1;
	int searchComp2;
	int searchComp3;

	short int bestResult;
	short int currentResult;

	byte currentIndex;

	int i;
	int j;

	palPtr = getPalettePtr();

	for (i = 0; i < 256; i++)
		_proc_special_palette[i] = i;

	curPtr = palPtr + colorMin * 3;

	for (i = colorMin; i < colorMax; i++) {
		readComp1 = *(curPtr++);
		readComp2 = *(curPtr++);
		readComp3 = *(curPtr++);

		colorComp1 = ((readComp1) * c) >> 8;
		colorComp2 = ((readComp2) * d) >> 8;
		colorComp3 = ((readComp3) * e) >> 8;

		searchPtr = palPtr;
		bestResult = 32000;
		currentIndex = 0;

		for (j = a; j < b; j++) {
			searchComp1 = (*searchPtr++);
			searchComp2 = (*searchPtr++);
			searchComp3 = (*searchPtr++);

			currentResult =
				abs(searchComp1 - colorComp1) + abs(searchComp2 - colorComp2) + abs(searchComp3 - colorComp3);

			if (currentResult < bestResult) {
				_proc_special_palette[i] = currentIndex;
				bestResult = currentResult;
			}
			currentIndex++;
		}
	}
}

