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
#include "gui.h"
#include "actor.h"

void Scumm::getGraphicsPerformance()
{
	int i;

	for (i = 10; i != 0; i--) {
		initScreens(0, 0, _realWidth, _realHeight); //ender
	}

	if (!(_features & GF_SMALL_HEADER))	/* FIXME TEST: Enders Zak fix */
		_vars[VAR_PERFORMANCE_1] = 0;	//_scummTimer;

	for (i = 10; i != 0; i--) {
		setDirtyRange(0, 0, _realHeight); //ender
		drawDirtyScreenParts();
	}

	if (!(_features & GF_SMALL_HEADER))	/* FIXME TEST: Enders Zak fix */
		_vars[VAR_PERFORMANCE_2] = 0;	//_scummTimer;

	if (_gameId == GID_DIG)
		initScreens(0, 0, _realWidth, _realHeight);
	else
		initScreens(0, 16, _realWidth, 144);
}

void Scumm::initScreens(int a, int b, int w, int h)
{
	int i;

	for (i = 0; i < 3; i++) {
	    nukeResource(rtBuffer, i+1);
		nukeResource(rtBuffer, i + 5);
	}

	if (!getResourceAddress(rtBuffer, 4)) {
		initVirtScreen(3, 0, 80, _realWidth, 13, false, false);
	}
	initVirtScreen(0, 0, b, _realWidth, h - b, true, true);
	initVirtScreen(1, 0, 0, _realWidth, b, false, false);
	initVirtScreen(2, 0, h, _realWidth, _realHeight - h, false, false); //ender

	_screenB = b;
	_screenH = h;

}

void Scumm::initVirtScreen(int slot, int number, int top, int width, int height, bool twobufs,
													 bool fourextra)
{
	VirtScreen *vs = &virtscr[slot];
	int size;
	int i;
	byte *ptr;

	assert(height >= 0);
	assert(slot >= 0 && slot < 4);

	vs->number = slot;
	vs->unk1 = 0;
	vs->width = 320;
	vs->topline = top;
	vs->height = height;
	vs->alloctwobuffers = twobufs;
	vs->scrollable = fourextra;
	vs->xstart = 0;
	size = vs->width * vs->height;
	vs->size = size;
	vs->backBuf = NULL;

	if (vs->scrollable)
		size += 320 * 4;

	createResource(rtBuffer, slot+1, size);
	vs->screenPtr = getResourceAddress(rtBuffer, slot+1);

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
	for (i = 0; i < 40; i++) {
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
	byte * src;

	updateDirtyScreen(2);
	if (_features & GF_OLD256)
		updateDirtyScreen(1);

	if ((camera._last.x == camera._cur.x &&
			 camera._last.y == camera._cur.y && (_features & GF_AFTER_V7))
			|| (camera._last.x == camera._cur.x)) {
		updateDirtyScreen(0);
	} else {
		vs = &virtscr[0];

		src = vs->screenPtr + _screenStartStrip * 8 + camera._cur.y - 100;
		_system->copy_rect(src , 320, 0, vs->topline, 320, vs->height);

		for (i = 0; i < 40; i++) {
			vs->tdirty[i] = (byte)vs->height;
			vs->bdirty[i] = 0;
		}
	}

	/* Handle shaking */
	if (_shakeEnabled &&  !_gui->isActive()) {
		_shakeFrame = (_shakeFrame + 1) & (NUM_SHAKE_POSITIONS - 1);
		_system->set_shake_pos(shake_positions[_shakeFrame]);
	}
}

void Scumm::updateDirtyScreen(int slot)
{
	gdi.updateDirtyScreen(&virtscr[slot]);
}

void Gdi::updateDirtyScreen(VirtScreen * vs)
{
	int i;
	int start, w, top, bottom;

	if (vs->height == 0)
		return;

	_readOffs = 0;
	if (vs->scrollable)
		_readOffs = vs->xstart;

	w = 8;
	start = 0;

	for (i = 0; i < 40; i++) {
		bottom = vs->bdirty[i];

		if (_vm->_features & GF_AFTER_V7 && (_vm->camera._cur.y != _vm->camera._last.y))
			drawStripToScreen(vs, start, w, 0, vs->height);
		else
		if (bottom) {
			top = vs->tdirty[i];
			vs->tdirty[i] = (byte)vs->height;
			vs->bdirty[i] = 0;
			if (i != 39 && vs->bdirty[i + 1] == (byte)bottom
					&& vs->tdirty[i + 1] == (byte)top) {
				w += 8;
				continue;
			}
			if(_vm->_features & GF_AFTER_V7)
				drawStripToScreen(vs, start, w, 0, vs->height);
			else
				drawStripToScreen(vs, start, w, top, bottom);
			w = 8;
		}
		start = i + 1;
	}
}

void Gdi::drawStripToScreen(VirtScreen * vs, int x, int w, int t, int b)
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
	if(height > 200)
		height = 200;

	scrollY = _vm->camera._cur.y - 100;
	if(scrollY == -100)
		scrollY = 0;

	ptr = vs->screenPtr + (t * 40 + x) * 8 + _readOffs + scrollY * 320;
		_vm->_system->copy_rect(ptr, 320, x * 8, vs->topline + t , w, height);
}

void blit(byte *dst, byte *src, int w, int h)
{
	assert(h > 0);


	do {
		memcpy(dst, src, w);
		dst += 320;
		src += 320;
	} while (--h);
}

void Scumm::setCursor(int cursor)
{
	warning("setCursor(%d)", cursor);
}

void Scumm::setCameraAt(int pos_x, int pos_y)
{

	if (_features & GF_AFTER_V7) {
		CameraData *cd = &camera;
		ScummPoint old;

		old = cd->_cur;

		cd->_cur.x = pos_x;
		cd->_cur.y = pos_y;

		clampCameraPos(&cd->_cur);

		cd->_dest = cd->_cur;

		assert(cd->_cur.x >= 160 && cd->_cur.y >= 100);

		if ((cd->_cur.x != old.x || cd->_cur.y != old.y)
				&& _vars[VAR_SCROLL_SCRIPT]) {
			_vars[VAR_CAMERA_POS_X] = cd->_cur.x;
			_vars[VAR_CAMERA_POS_Y] = cd->_cur.y;
			runScript(_vars[VAR_SCROLL_SCRIPT], 0, 0, 0);
		}
	} else {
		int t;
		CameraData *cd = &camera;

		if (cd->_mode != CM_FOLLOW_ACTOR || abs(pos_x - cd->_cur.x) > 160) {
			cd->_cur.x = pos_x;
		}
		cd->_dest.x = pos_x;

		t = _vars[VAR_CAMERA_MIN_X];
		if (cd->_cur.x < t)
			cd->_cur.x = t;

		t = _vars[VAR_CAMERA_MAX_X];
		if (cd->_cur.x > t)
			cd->_cur.x = t;

		if (_vars[VAR_SCROLL_SCRIPT]) {
			_vars[VAR_CAMERA_POS_X] = cd->_cur.x;
			runScript(_vars[VAR_SCROLL_SCRIPT], 0, 0, 0);
		}

		if (cd->_cur.x != cd->_last.x && charset._hasMask)
			stopTalk();
	}
}

void Scumm::setCameraFollows(Actor * a)
{
	if (_features & GF_AFTER_V7) {
		CameraData *cd = &camera;
		byte oldfollow = cd->_follows;
		int ax, ay;

		cd->_follows = a->number;

		if (a->room != _currentRoom) {
			startScene(a->room, 0, 0);
		}

		ax = abs(a->x - cd->_cur.x);
		ay = abs(a->y - cd->_cur.y);

		if (ax > _vars[VAR_CAMERA_THRESHOLD_X] ||
				ay > _vars[VAR_CAMERA_THRESHOLD_Y] || ax > 160 || ay > 100) {
			setCameraAt(a->x, a->y);
		}

		if (a->number != oldfollow)
			runHook(0);
	} else {
		int t, i;
		CameraData *cd = &camera;

		cd->_mode = CM_FOLLOW_ACTOR;
		cd->_follows = a->number;

		if (a->room != _currentRoom) {
			startScene(a->room, 0, 0);
			cd->_mode = CM_FOLLOW_ACTOR;
			cd->_cur.x = a->x;
			setCameraAt(cd->_cur.x, 0);
		}

		t = (a->x >> 3);

		if (t - _screenStartStrip < cd->_leftTrigger ||
				t - _screenStartStrip > cd->_rightTrigger)
			setCameraAt(a->x, 0);

		for (i = 1, a = getFirstActor(); ++a, i < NUM_ACTORS; i++) {
			if (a->room == _currentRoom)
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
		initVirtScreen(0, 0, virtscr[0].topline, 200, height, 1, 1);
	}

	room = getResourceAddress(rtRoom, _roomResource);
	if (_features & GF_SMALL_HEADER) {
		gdi._numZBuffer = 2;
	} else {
		ptr = findResource(MKID('RMIH'), findResource(MKID('RMIM'), room));
		gdi._numZBuffer = READ_LE_UINT16(ptr + 8) + 1;
	}
	assert(gdi._numZBuffer >= 1 && gdi._numZBuffer <= 5);

	if (_features & GF_AFTER_V7)
		itemsize = (virtscr[0].height +4) * 40;
	else
		itemsize = (_scrHeight + 4) * 40;


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

void Scumm::moveMemInPalRes(int start, int end, byte direction)
{
	byte *startptr, *endptr;
	byte *startptr2, *endptr2;
	int num;
	byte tmp[6];
	byte tmp2[6];

	if (!_palManipCounter)
		return;

	startptr = getResourceAddress(rtTemp, 4) + start * 6;
	endptr = getResourceAddress(rtTemp, 4) + end * 6;

	startptr2 = getResourceAddress(rtTemp, 5) + start * 6;
	endptr2 = getResourceAddress(rtTemp, 5) + end * 6;

	num = end - start;

	if (!endptr) {
		warning("moveMemInPalRes(%d,%d): Bad end pointer\n", start, end);
		return;
	}

	if (!direction) {
		memmove(tmp, endptr, 6);
		memmove(startptr + 6, startptr, num * 6);
		memmove(startptr, tmp, 6);
		memmove(tmp2, endptr2, 6);
		memmove(startptr2 + 6, startptr2, num * 6);
		memmove(startptr2, tmp2, 6);
	} else {
		memmove(tmp, startptr, 6);
		memmove(startptr, startptr + 6, num * 6);
		memmove(endptr, tmp, 6);
		memmove(tmp2, startptr2, 6);
		memmove(startptr2, startptr2 + 6, num * 6);
		memmove(endptr2, tmp2, 6);
	}
}

void Scumm::fadeToBlackEffect(int a)
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
		transitionEffect(a - 1);
		break;
	case 128:
		unkScreenEffect6();
		break;
	case 129:
		// Just blit screen 0 to the display (i.e. display will be black)
		setDirtyRange(0, 0, vs->height);
		updateDirtyScreen(0);
		/* XXX: EGA_proc4(0); */
//		warning("EGA_proc4");				/* FIXME */
		break;
	case 134:
		unkScreenEffect5(0);
		break;
	case 135:
		unkScreenEffect5(1);
		break;
	default:
		warning("fadeToBlackEffect: default case %d", a);
	}
}

void Scumm::redrawBGAreas()
{
	int i;
	int val;
	CameraData *cd = &camera;
	int diff;

	if (!(_features & GF_AFTER_V7))
		if (cd->_cur.x != cd->_last.x && charset._hasMask)
			stopTalk();

	val = 0;

	if (!_fullRedraw && _BgNeedsRedraw) {
		for (i = 0; i != 40; i++) {
			if (gfxUsageBits[_screenStartStrip + i] & 0x80000000) {
				redrawBGStrip(i, 1);
			}
		}
	}

	if (_features & GF_AFTER_V7) {
		diff = (cd->_cur.x >> 3) - (cd->_last.x >> 3);
		if (_fullRedraw == 0 && diff == 1) {
			val = 2;
			redrawBGStrip(39, 1);
		} else if (_fullRedraw == 0 && diff == -1) {
			val = 1;
			redrawBGStrip(0, 1);
		} else if (_fullRedraw != 0 || diff != 0) {
			_BgNeedsRedraw = false;
			_fullRedraw = false;
			redrawBGStrip(0, 40);
		}
	} else {
		if (_fullRedraw == 0 && cd->_cur.x - cd->_last.x == 8) {
			val = 2;
			redrawBGStrip(39, 1);
		} else if (_fullRedraw == 0 && cd->_cur.x - cd->_last.x == -8) {
			val = 1;
			redrawBGStrip(0, 1);
		} else if (_fullRedraw != 0 || cd->_cur.x != cd->_last.x) {
			_BgNeedsRedraw = false;
			redrawBGStrip(0, 40);
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

void Gdi::drawBitmap(byte *ptr, VirtScreen * vs, int x, int y, int h,
										 int stripnr, int numstrip, byte flag)
{
	byte *smap_ptr, *where_draw_ptr;
	int i;
	byte *zplane_list[6];

	int bottom;
	byte twobufs;
	int numzbuf;
	int sx;


	CHECK_HEAP;
	if (_vm->_features & GF_SMALL_HEADER)
		smap_ptr = _smap_ptr = ptr;
	else
		smap_ptr = findResource(MKID('SMAP'), ptr);

	assert(smap_ptr);

	numzbuf = _disable_zbuffer ? 0 : _numZBuffer;

	if (_vm->_features & GF_SMALL_HEADER) {
		/* this is really ugly, FIXME */
		if (ptr[-2] == 'B' && ptr[-1] == 'M' &&
		    READ_LE_UINT32(ptr - 6) > (READ_LE_UINT32(ptr) + 10)) {
			zplane_list[1] = smap_ptr + READ_LE_UINT32(ptr);
		} else if (ptr[-4] == 'O' && ptr[-3] == 'I' &&
			   READ_LE_UINT32(ptr - 8) > READ_LE_UINT32(ptr) + 12) {
			zplane_list[1] = smap_ptr + READ_LE_UINT32(ptr);
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
		warning("Gdi::drawBitmap, strip drawn to %d below window bottom %d", bottom,
					vs->height);
	}

	twobufs = vs->alloctwobuffers;

	_vertStripNextInc = h * 320 - 1;

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

		if ((uint) sx >= 40)
			return;

		if (y < vs->tdirty[sx])
			vs->tdirty[sx] = y;

		if (bottom > vs->bdirty[sx])
			vs->bdirty[sx] = bottom;

		_backbuff_ptr = vs->screenPtr + (y * 40 + x) * 8;
		_bgbak_ptr =
			_vm->getResourceAddress(rtBuffer, vs->number + 5) + (y * 40 + x) * 8;
		if (!twobufs) {
			_bgbak_ptr = _backbuff_ptr;
		}
		_mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + (y * 40 + x);

		where_draw_ptr = _bgbak_ptr;
		decompressBitmap();

		CHECK_HEAP;
		if (twobufs) {
			_bgbak_ptr = where_draw_ptr;

			if (_vm->hasCharsetMask(sx << 3, y, (sx + 1) << 3, bottom)) {
				if (flag & dbClear)
					clear8ColWithMasking();
				else
					draw8ColWithMasking();
			} else {
				if (flag & dbClear)
					clear8Col();
				else
					blit(_backbuff_ptr, _bgbak_ptr, 8, h);
			}
		}
		CHECK_HEAP;
		if (flag & dbDrawMaskOnBoth) {
			_z_plane_ptr = zplane_list[1] + READ_LE_UINT16(zplane_list[1] + stripnr * 2 + 8);
			_mask_ptr_dest = _vm->getResourceAddress(rtBuffer, 9) + y * 40 + x;
			if (_useOrDecompress && flag & dbAllowMaskOr)
				decompressMaskImgOr();
			else
				decompressMaskImg();
		}
		
		for (i = 1; i < numzbuf; i++) {
			uint16 offs;

			if (!zplane_list[i])
				continue;			
			
			if (_vm->_features & GF_SMALL_HEADER)
				if (_vm->_features & GF_OLD256)
					offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2 + 4);
				else
					offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2 + 2);
			else
				offs = READ_LE_UINT16(zplane_list[i] + stripnr * 2 + 8);

			_mask_ptr_dest = _vm->getResourceAddress(rtBuffer, 9) + y * 40 + x + _imgBufOffs[i];

			if (offs) {
				_z_plane_ptr = zplane_list[i] + offs;

				if (_useOrDecompress && flag & dbAllowMaskOr)
					decompressMaskImgOr();
				else
					decompressMaskImg();
			} else {
				if (_useOrDecompress && flag & dbAllowMaskOr)
					; /* nothing */
				else
					for (int h = 0; h < _numLinesToProcess; h++)
						_mask_ptr_dest[h * 40] = 0;
				/* needs better abstraction, FIXME */
			}
		}
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
/* alignment safe */
			((uint32 *)dst)[0] = ((uint32 *)src)[0];
			((uint32 *)dst)[1] = ((uint32 *)src)[1];
		}
		src += 320;
		dst += 320;
		mask += 40;
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
		if (!maskbits) {
			((uint32 *)dst)[1] = ((uint32 *)dst)[0] = 0;
		} else {
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
		}
		dst += 320;
		mask += 40;
	} while (--height);
}

void Gdi::clear8Col()
{
	int height = _numLinesToProcess;
	byte *dst = _backbuff_ptr;
	do {
		((uint32 *)dst)[1] = ((uint32 *)dst)[0] = 0;
		dst += 320;
	} while (--height);
}

void Gdi::decompressMaskImg()
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
				*dst = c;
				dst += 40;
				if (!--height)
					return;
			} while (--b);
		} else {
			do {
				*dst = *src++;
				dst += 40;
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
				dst += 40;
				if (!--height)
					return;
			} while (--b);
		} else {
			do {
				*dst |= *src++;
				dst += 40;
				if (!--height)
					return;
			} while (--b);
		}
	}
}

void Scumm::redrawBGStrip(int start, int num)
{
	int s = _screenStartStrip + start;

	assert(s >= 0
				 && (size_t) s < sizeof(gfxUsageBits) / sizeof(gfxUsageBits[0]));

	_curVirtScreen = &virtscr[0];

	gfxUsageBits[s] |= 0x80000000;

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
			FILL_BITS * dst++ = color + _palette_mod;;

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
							dst += 312;
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
		dst += 312;
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
		dst += 312;
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
								dst += 312;
								if (!--_tempNumLines)
									return;
							}
							dst++;
						} while (--reps);
					} else {
						do {
							if (!--_currentX) {
								_currentX = 8;
								dst += 312;
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
		dst += 312;
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
			dst += 320;
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
			FILL_BITS * dst++ = color + _palette_mod;
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
		dst += 312;
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
			FILL_BITS * dst = color + _palette_mod;
			dst += 320;
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
                dst += 320;                                     \
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
		dst += 320;
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
			dst += 320;
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
		restoreBG(gdi._mask_left, gdi._mask_top, gdi._mask_right,
							gdi._mask_bottom);
		charset._hasMask = false;
		gdi._mask_left = -1;
		charset._strLeft = -1;
		charset._left = -1;
	}

	charset._xpos2 = string[0].xpos;
	charset._ypos2 = string[0].ypos;
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
	if (left > 320)
		return;
	if (right > 320)
		right = 320;
	if (bottom >= height)
		bottom = height;

	updateDirtyRect(vs->number, left, right, top - topline, bottom - topline,
									0x40000000);

	height = (top - topline) * 320 + vs->xstart + left;

	backbuff = vs->screenPtr + height;
	bgbak = getResourceAddress(rtBuffer, vs->number + 5) + height;
	mask =
		getResourceAddress(rtBuffer,
											 9) + top * 40 + (left >> 3) + _screenStartStrip;
	if (vs->number == 0) {
		mask += vs->topline * 216;
	}

	height = bottom - top;
	width = right - left;
	widthmod = (width >> 2) + 2;

	if (vs->alloctwobuffers
			&& _currentRoom != 0 /*&& _vars[VAR_V5_DRAWFLAGS]&2 */ ) {
		blit(backbuff, bgbak, width, height);
		if (vs->number == 0 && charset._hasMask && height) {
			do {
				memset(mask, 0, widthmod);
				mask += 40;
			} while (--height);
		}
	} else {
		if (height) {
			do {
				memset(backbuff, _bkColor, width);
				backbuff += 320;
			} while (--height);
		}
	}
}

void Scumm::updateDirtyRect(int virt, int left, int right, int top,
														int bottom, uint32 dirtybits)
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
		if (rp >= 200)
			rp = 200;
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

void Scumm::setVirtscreenDirty(VirtScreen * vs, int left, int top, int right,
															 int bottom)
{
	int lp = left >> 3;
	int rp = right >> 3;

	if (lp >= 40 || rp < 0)
		return;
	if (lp < 0)
		lp = 0;
	if (rp >= 40)
		rp = 39;

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
	int delta[16];	// Offset applied during each iteration
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
					if (l >= 0 && l < 40 && (uint) t < (uint) bottom) {
						virtscr[0].tdirty[l] = t << 3;
						virtscr[0].bdirty[l] = (t + 1) << 3;
					}
					l++;
				}
			} else {
				if (l < 0 || l >= 40 || b <= t)
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

void Scumm::unkScreenEffect6()
{
	/* XXX: not implemented */
	warning("stub unkScreenEffect6");
}

void Scumm::unkScreenEffect5(int a)
{
	/* XXX: not implemented */
	warning("stub unkScreenEffect5(%d)", a);
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
	memset(_vm->getResourceAddress(rtBuffer, 9),
				 0, _imgBufOffs[1] - _imgBufOffs[0]
		);
}

void Scumm::clampCameraPos(ScummPoint * pt)
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
		CameraData *cd = &camera;
		ScummPoint old = cd->_cur;
		Actor *a = NULL;

		if (cd->_follows) {
			a = derefActorSafe(cd->_follows, "moveCamera");
			if (abs(cd->_cur.x - a->x) > _vars[VAR_CAMERA_THRESHOLD_X] ||
					abs(cd->_cur.y - a->y) > _vars[VAR_CAMERA_THRESHOLD_Y]) {
				cd->_movingToActor = true;
				if (_vars[VAR_CAMERA_THRESHOLD_X] == 0)
					cd->_cur.x = a->x;
				if (_vars[VAR_CAMERA_THRESHOLD_Y] == 0)
					cd->_cur.y = a->y;
				clampCameraPos(&cd->_cur);
			}
		} else {
			cd->_movingToActor = false;
		}

		if (cd->_movingToActor) {
			cd->_dest.x = a->x;
			cd->_dest.y = a->y;
		}

		assert(cd->_cur.x >= 160 && cd->_cur.y >= 100);

		clampCameraPos(&cd->_dest);

		if (cd->_cur.x < cd->_dest.x) {
			cd->_cur.x += _vars[VAR_CAMERA_SPEED_X];
			if (cd->_cur.x > cd->_dest.x)
				cd->_cur.x = cd->_dest.x;
		}

		if (cd->_cur.x > cd->_dest.x) {
			cd->_cur.x -= _vars[VAR_CAMERA_SPEED_X];
			if (cd->_cur.x < cd->_dest.x)
				cd->_cur.x = cd->_dest.x;
		}

		if (cd->_cur.y < cd->_dest.y) {
			cd->_cur.y += _vars[VAR_CAMERA_SPEED_Y];
			if (cd->_cur.y > cd->_dest.y)
				cd->_cur.y = cd->_dest.y;
		}

		if (cd->_cur.y > cd->_dest.y) {
			cd->_cur.y -= _vars[VAR_CAMERA_SPEED_Y];
			if (cd->_cur.y < cd->_dest.y)
				cd->_cur.y = cd->_dest.y;
		}

		if (cd->_cur.x == cd->_dest.x && cd->_cur.y == cd->_dest.y) {

			cd->_movingToActor = false;
			cd->_accel.x = cd->_accel.y = 0;
			_vars[VAR_CAMERA_SPEED_X] = _vars[VAR_CAMERA_SPEED_Y] = 0;
		} else {

			cd->_accel.x += _vars[VAR_CAMERA_ACCEL_X];
			cd->_accel.y += _vars[VAR_CAMERA_ACCEL_Y];

			_vars[VAR_CAMERA_SPEED_X] += cd->_accel.x / 100;
			_vars[VAR_CAMERA_SPEED_Y] += cd->_accel.y / 100;

			if (_vars[VAR_CAMERA_SPEED_X] < 8)
				_vars[VAR_CAMERA_SPEED_X] = 8;

			if (_vars[VAR_CAMERA_SPEED_Y] < 8)
				_vars[VAR_CAMERA_SPEED_Y] = 8;

		}

		cameraMoved();

		if (cd->_cur.x != old.x || cd->_cur.y != old.y) {
			_vars[VAR_CAMERA_POS_X] = cd->_cur.x;
			_vars[VAR_CAMERA_POS_Y] = cd->_cur.y;

			_vars[VAR_CAMERA_DEST_X] = cd->_dest.x;

			_vars[VAR_CAMERA_DEST_Y] = cd->_dest.y;

			_vars[VAR_CAMERA_FOLLOWED_ACTOR] = cd ->_follows;

			if(_vars[VAR_SCROLL_SCRIPT])
				runScript(_vars[VAR_SCROLL_SCRIPT], 0, 0, 0);
		}
	} else {
		CameraData *cd = &camera;
		int pos = cd->_cur.x;
		int actorx, t;
		Actor *a = NULL;

		cd->_cur.x &= 0xFFF8;

		if (cd->_cur.x < _vars[VAR_CAMERA_MIN_X]) {
			if (_vars[VAR_CAMERA_FAST_X])
				cd->_cur.x = _vars[VAR_CAMERA_MIN_X];
			else
				cd->_cur.x += 8;
			cameraMoved();
			return;
		}

		if (cd->_cur.x > _vars[VAR_CAMERA_MAX_X]) {
			if (_vars[VAR_CAMERA_FAST_X])
				cd->_cur.x = _vars[VAR_CAMERA_MAX_X];
			else
				cd->_cur.x -= 8;
			cameraMoved();
			return;
		}

		if (cd->_mode == CM_FOLLOW_ACTOR) {
			a = derefActorSafe(cd->_follows, "moveCamera");

			actorx = a->x;
			t = (actorx >> 3) - _screenStartStrip;

			if (t < cd->_leftTrigger || t > cd->_rightTrigger) {
				if (_vars[VAR_CAMERA_FAST_X]) {
					if (t > 35)
						cd->_dest.x = actorx + 80;
					if (t < 5)
						cd->_dest.x = actorx - 80;
				} else
					cd->_movingToActor = 1;
			}
		}

		if (cd->_movingToActor) {
			a = derefActorSafe(cd->_follows, "moveCamera(2)");
			cd->_dest.x = a->x;
		}

		if (cd->_dest.x < _vars[VAR_CAMERA_MIN_X])
			cd->_dest.x = _vars[VAR_CAMERA_MIN_X];

		if (cd->_dest.x > _vars[VAR_CAMERA_MAX_X])
			cd->_dest.x = _vars[VAR_CAMERA_MAX_X];

		if (_vars[VAR_CAMERA_FAST_X]) {
			cd->_cur.x = cd->_dest.x;
		} else {
			if (cd->_cur.x < cd->_dest.x)
				cd->_cur.x += 8;
			if (cd->_cur.x > cd->_dest.x)
				cd->_cur.x -= 8;
		}

		/* a is set a bit above */
		if (cd->_movingToActor && cd->_cur.x >> 3 == a->x >> 3) {
			cd->_movingToActor = 0;
		}

		cameraMoved();

		if (pos != cd->_cur.x && _vars[VAR_SCROLL_SCRIPT]) {
			_vars[VAR_CAMERA_POS_X] = cd->_cur.x;
			runScript(_vars[VAR_SCROLL_SCRIPT], 0, 0, 0);
		}
	}
}

void Scumm::cameraMoved()
{
	if (_features & GF_AFTER_V7) {
		CameraData *cd = &camera;

		assert(cd->_cur.x >= 160 && cd->_cur.y >= 100);

		_screenStartStrip = (cd->_cur.x - 160) >> 3;
		_screenEndStrip = _screenStartStrip + 39;
		virtscr[0].xstart = _screenStartStrip << 3;

		_screenLeft = cd->_cur.x - 160;
		_screenTop = cd->_cur.y - 100;
	} else {

		CameraData *cd = &camera;

		if (cd->_cur.x < 160) {
			cd->_cur.x = 160;
		} else if (cd->_cur.x + 160 >= _scrWidth) {
			cd->_cur.x = _scrWidth - 160;
		}

		_screenStartStrip = (cd->_cur.x >> 3) - 20;
		_screenEndStrip = _screenStartStrip + 39;
		virtscr[0].xstart = _screenStartStrip << 3;
	}
}

void Scumm::panCameraTo(int x, int y)
{
	if (_features & GF_AFTER_V7) {
		CameraData *cd = &camera;
		cd->_follows = 0;
		cd->_dest.x = x;
		cd->_dest.y = y;
	} else {

		CameraData *cd = &camera;
		cd->_dest.x = x;
		cd->_mode = CM_PANNING;
		cd->_movingToActor = 0;
	}
}

void Scumm::actorFollowCamera(int act)
{
	if (!(_features & GF_AFTER_V7)) {
		int old;
		CameraData *cd = &camera;

		/* mi1 compatibilty */
		if (act == 0) {
			cd->_mode = CM_NORMAL;
			cd->_follows = 0;
			cd->_movingToActor = 0;
			return;
		}

		old = cd->_follows;
		setCameraFollows(derefActorSafe(act, "actorFollowCamera"));
		if (cd->_follows != old)
			runHook(0);

		cd->_movingToActor = 0;
	}
}

void Scumm::setCameraAtEx(int at)
{
	if (!(_features & GF_AFTER_V7)) {
		CameraData *cd = &camera;
		cd->_mode = CM_NORMAL;
		cd->_cur.x = at;
		setCameraAt(at, 0);
		cd->_movingToActor = 0;
	}
}

void Scumm::palManipulate()
{
	byte *srcptr, *destptr;
	byte *pal;
	int i, j;

	if (!_palManipCounter)
		return;
	srcptr = getResourceAddress(rtTemp, 4) + _palManipStart * 6;
	destptr = getResourceAddress(rtTemp, 5) + _palManipStart * 6;
	pal = _currentPalette + _palManipStart * 3;

	i = _palManipStart;
	while (i < _palManipEnd) {
		j = (*((uint16 *)srcptr) += *(uint16 *)destptr);
		*pal++ = j >> 8;
		srcptr += 2;
		destptr += 2;

		j = (*((uint16 *)srcptr) += *(uint16 *)destptr);
		*pal++ = j >> 8;
		srcptr += 2;
		destptr += 2;

		j = (*((uint16 *)srcptr) += *(uint16 *)destptr);
		*pal++ = j >> 8;
		srcptr += 2;
		destptr += 2;

		i++;
	}
	setDirtyColors(_palManipStart, _palManipEnd);
	if (!--_palManipCounter) {
		nukeResource(rtTemp, 4);
		nukeResource(rtTemp, 5);
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

	setDirtyColors(a, b);
}

void Scumm::screenEffect(int effect)
{
	switch (effect) {
	case 1:
	case 2:
	case 3:
		transitionEffect(effect - 1);
		break;
	case 128:
		unkScreenEffect6();
		break;
	case 130:
		unkScreenEffect1();
		break;
	case 131:
		unkScreenEffect2();
		break;
	case 132:
		unkScreenEffect3();
		break;
	case 133:
		unkScreenEffect4();
		break;
	case 134:
		unkScreenEffect5(0);
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

void Scumm::resetActorBgs()
{
	Actor *a;
	int i;
	uint32 onlyActorFlags, bitpos;

	for (i = 0; i < 40; i++) {
		onlyActorFlags = (gfxUsageBits[_screenStartStrip + i] &= 0x3FFFFFFF);
		a = getFirstActor();
		bitpos = 1;

		while (onlyActorFlags) {
			if (onlyActorFlags & 1 && a->top != 0xFF && a->needBgReset) {
				gfxUsageBits[_screenStartStrip + i] ^= bitpos;

				if((a->bottom - a->top) >=0)
					gdi.resetBackground(a->top, a->bottom, i);
			}
			bitpos <<= 1;
			onlyActorFlags >>= 1;
			a++;
		}
	}

	for (i = 1, a = getFirstActor(); ++a, i < NUM_ACTORS; i++) {
		a->needBgReset = false;
	}
}

void Gdi::resetBackground(int top, int bottom, int strip)
{
	VirtScreen *vs = &_vm->virtscr[0];
	int offs;

	if (top < vs->tdirty[strip])
		vs->tdirty[strip] = top;

	if (bottom > vs->bdirty[strip])
		vs->bdirty[strip] = bottom;

	offs = (top * 40 + _vm->_screenStartStrip + strip );
	_mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + offs;
	_bgbak_ptr = _vm->getResourceAddress(rtBuffer, 5) + (offs << 3);
	_backbuff_ptr = vs->screenPtr + (offs << 3);

	_numLinesToProcess = bottom - top;
	if (_numLinesToProcess) {
		if (1 /*_vm->_vars[VAR_V5_DRAWFLAGS]&2*/ ) {
			if (_vm->hasCharsetMask(strip << 3, top, (strip + 1) << 3, bottom))
				draw8ColWithMasking();
			else
				blit(_backbuff_ptr, _bgbak_ptr, 8, _numLinesToProcess);
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
	if (r > 39)
		r = 39;

	mem += l + t * 40;

	w = r - l;
	h = b - t + 1;

	do {
		for (i = 0; i <= w; i++)
			if (mem[i]) {				
				return true;
			}
		mem += 40;
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

void Scumm::darkenPalette(int a, int b, int c, int d, int e)
{
	byte *cptr, *cur;
	int num;
	int color;

	cptr = getPalettePtr() + a * 3;
	cur = _currentPalette + a * 3;
	if (a <= b) {
		num = b - a + 1;

		do {
			color = *cptr++;
			if (c != 0xFF)
				color = color * c / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;

			color = *cptr++;
			if (d != 0xFF)
				color = color * d / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;

			color = *cptr++;
			if (e != 0xFF)
				color = color * e / 0xFF;
			if (color > 255)
				color = 255;
			*cur++ = color;
		} while (--num);
	}
	setDirtyColors(a, b);
}

void Scumm::grabCursor(int x, int y, int w, int h)
{
	VirtScreen *vs = findVirtScreen(y);

	if (vs == NULL) {
		warning("grabCursor: invalid Y %d", y);
		return;
	}

	grabCursor(vs->screenPtr + (y - vs->topline) * 320 + x, w, h);

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
		ptr += 320;
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

	blit(vs->screenPtr + vs->xstart,
			 getResourceAddress(rtBuffer, 5) + vs->xstart, w, h);
}

void Scumm::updateCursor() {
	_system->set_mouse_cursor(_grabbedCursor, _cursorWidth, _cursorHeight,
		_cursorHotspotX, _cursorHotspotY);
}

void Scumm::animateCursor() {
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

void Scumm::decompressDefaultCursor(int idx)
{
	int i;
	byte color;

	memset(_grabbedCursor, 0xFF, sizeof(_grabbedCursor));
	_cursorWidth = 16;
	_cursorHeight = 16;
	_cursorHotspotX = 8;
	_cursorHotspotY = 8;

	color = default_cursor_colors[idx];

	for (i = 0; i < 16; i++) {
		_grabbedCursor[16 * 8 + i] = color;
		_grabbedCursor[16 * i + 8] = color;
	}

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

		j = abs(ar - r);
		sum = j * j * 3;
		j = abs(ag - g);
		sum += j * j * 6;
		j = abs(ab - b);
		sum += j * j * 2;

		if (sum < bestsum) {
			bestsum = sum;
			bestitem = i;
		}
	}

	if (threshold != (uint) - 1
			&& bestsum > threshold * threshold * (2 + 3 + 6)) {
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


// param3= clipping

// param2= mirror

// param1= never used ?
void Scumm::drawBomp(BompDrawData *bd, int param1, byte* dataPtr, int param2, int param3)
{
	byte *dest = bd->out + bd->y * bd->outwidth, *src;
	int h = bd->srcheight;
	bool inside;

	if (h == 0 || bd->srcwidth == 0)
		return;

	inside = (bd->x >= 0) && (bd->y >= 0) &&
		(bd->x <= bd->outwidth - bd->srcwidth) &&
		(bd->y <= bd->outheight - bd->srcheight);

	if (1 || bd->scale_x == 255 && bd->scale_y == 255) {
		/* Routine used when no scaling is needed */
		if (inside) {
			dest += bd->x;
			src = bd->dataptr;
			do {
				byte code, color;
				uint len = bd->srcwidth, num, i;
				byte *d = dest;
				src += 2;
				do {
					code = *src++;
					num = (code >> 1) + 1;
					if (num > len)
						num = len;
					len -= num;
					if (code & 1) {
						color = *src++;
						if (color != 255) {
							do
								*d++ = color;
							while (--num);
						} else {
							d += num;
						}
					} else {
						for (i = 0; i < num; i++)
							if ((color = src[i]) != 255)
								d[i] = color;
						d += num;
						src += num;
					}
				} while (len);
				dest += bd->outwidth;
			} while (--h);
		} else {
			uint y = bd->y;
			src = bd->dataptr;

			do {
				byte color;
				uint len, num;
				uint x;
				if ((uint) y >= (uint) bd->outheight) {
					src += READ_LE_UINT16(src) + 2;
					continue;
				}
				len = bd->srcwidth;
				x = bd->x;

				src += 2;
				do {
					byte code = *src++;
					num = (code >> 1) + 1;
					if (num > len)
						num = len;
					len -= num;
					if (code & 1) {
						if ((color = *src++) != 255) {
							do {
								if ((uint) x < (uint) bd->outwidth)
									dest[x] = color;
							} while (++x, --num);
						} else {
							x += num;
						}
					} else {
						do {
							if ((color = *src++) != 255 && (uint) x < (uint) bd->outwidth)
								dest[x] = color;
						} while (++x, --num);
					}
				} while (len);
			} while (dest += bd->outwidth, y++, --h);
		}
	} else {
		/* scaling of bomp images not supported yet */
	}

CHECK_HEAP;
}


/* Yazoo: This function create the specialPalette used for semi-transparency in SamnMax */


void Scumm::createSpecialPalette(int16 a, int16 b, int16 c, int16 d, int16 e, int16 colorMin, int16 colorMax)
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

	for(i=0;i<256;i++)
		_proc_special_palette[i]=i;

	curPtr = palPtr + colorMin*3;

	for(i=colorMin;i<colorMax;i++) {
		readComp1=*(curPtr++);
		readComp2=*(curPtr++);
		readComp3=*(curPtr++);

		colorComp1=((readComp1)*c)>>8;
		colorComp2=((readComp2)*d)>>8;
		colorComp3=((readComp3)*e)>>8;

		searchPtr = palPtr;
		bestResult = 32000;
		currentIndex = 0;

		for(j=a;j<b;j++) {
			searchComp1 = (*searchPtr++);
			searchComp2 = (*searchPtr++);
			searchComp3 = (*searchPtr++);

			currentResult = abs(searchComp1-colorComp1) + abs(searchComp2-colorComp2) + abs(searchComp3-colorComp3);

			if(currentResult<bestResult) {
				_proc_special_palette[i]=currentIndex;
				bestResult=currentResult;
			}
			currentIndex++;
		}
	}
}
