/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

void Scumm::getGraphicsPerformance() {
	int i;

	for (i=10; i!=0; i--) {
		initScreens(0, 0, 320, 200);
	}

	_vars[VAR_PERFORMANCE_1] = 0;//_scummTimer;

	for (i=10; i!=0; i--) {
		setDirtyRange(0, 0, 200);
		drawDirtyScreenParts();
	}

	_vars[VAR_PERFORMANCE_2] = 0;//_scummTimer;
	
	initScreens(0, 16, 320, 144);
}

void Scumm::initScreens(int a, int b, int w, int h) {
	int i;

	for (i=0; i<3; i++) {
		nukeResource(rtBuffer, i+1);
		nukeResource(rtBuffer, i+5);
	}

	if (!getResourceAddress(rtBuffer,4)) {
		initVirtScreen(3, 80, 13, false, false);
	}
	initVirtScreen(0, b, h-b, true, true);
	initVirtScreen(1, 0, b, false, false);
	initVirtScreen(2, h, 200-h, false, false);

	_screenB = b;
	_screenH = h;

}

void Scumm::initVirtScreen(int slot, int top, int height, bool twobufs, bool fourextra) {
	VirtScreen *vs = &virtscr[slot];
	int size;

	assert(height>=0);
	assert(slot>=0 && slot<4);

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
	if (vs->scrollable)
		size += 320*4;

	createResource(rtBuffer, slot+1, size);

	if (twobufs) {
		createResource(rtBuffer, slot+5, size);
	}

	if (slot != 3) {
		setDirtyRange(slot, 0, height);
	}
}

void Scumm::setDirtyRange(int slot, int top, int bottom) {
	int i;
	VirtScreen *vs = &virtscr[slot];
	for (i=0; i<40; i++) {
		vs->tdirty[i] = top;
		vs->bdirty[i] = bottom;
	}
}

void Scumm::drawDirtyScreenParts() {
	int i;
	VirtScreen *vs;

	updateDirtyScreen(2);

	if (camera._lastPos == camera._curPos) {
		updateDirtyScreen(0);
	} else {
		vs = &virtscr[0];

		blitToScreen(this, getResourceAddress(rtBuffer, 1) + _screenStartStrip*8,
			0, vs->topline, 320, vs->height);

		for (i = 0; i<40; i++) {
			vs->tdirty[i] = (byte)vs->height;
			vs->bdirty[i] = 0;
		}
	}
}

void Scumm::updateDirtyScreen(int slot) {
	gdi.updateDirtyScreen(&virtscr[slot]);
}

void Gdi::updateDirtyScreen(VirtScreen *vs) {
	int i;
	int start,w,top,bottom;

	if (vs->height==0)
		return;

	_readOffs = 0;
	if (vs->scrollable)
		_readOffs = vs->xstart;

	w = 8;
	start = 0;

	for (i=0; i<40; i++) {
		bottom = vs->bdirty[i];
		if (bottom) {
			top = vs->tdirty[i];
			vs->tdirty[i] = (byte)vs->height;
			vs->bdirty[i] = 0;
			if (i!=39 && vs->bdirty[i+1] == (byte)bottom &&	vs->tdirty[i+1] == (byte)top) {
				w += 8;
				continue;
			}
			drawStripToScreen(vs, start, w, top, bottom);
			w = 8;
		}
		start = i+1;
	}
}

void Gdi::drawStripToScreen(VirtScreen *vs, int x, int w, int t, int b) {
	byte *ptr;

	if (b <= t)
		return;

	if (t > vs->height)
		t = 0;

	if (b > vs->height)
		b = vs->height;

	ptr = _vm->getResourceAddress(rtBuffer, vs->number+1) + (t*40+x)*8 + _readOffs;
	blitToScreen(_vm, ptr, x*8, vs->topline+t, w, b-t);
}

void blit(byte *dst, byte *src, int w, int h) {
	assert(h>0);
	do {
		memcpy(dst, src, w);
		dst += 320;
		src += 320;
	} while (--h);
}

/* TODO: writes are being done to this data */
MouseCursor mouse_cursors[4] = {
	8,7,{15,15,7,8},
	{
	0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,
	0x00,0x80,0x00,0x80,0x00,0x00,0x7E,0x3F,
	0x00,0x00,0x00,0x80,0x00,0x80,0x00,0x80,
	0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x00,
	},
	8,7,{15,15,7,8},
	{
	0x00,0x00,0x7F,0xFE,0x60,0x06,0x30,0x0C,
	0x18,0x18,0x0C,0x30,0x06,0x60,0x03,0xC0,
	0x06,0x60,0x0C,0x30,0x19,0x98,0x33,0xCC,
	0x67,0xE6,0x7F,0xFE,0x00,0x00,0x00,0x00,
	},

	8,7,{15,15,7,8},
	{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	},
	8,7,{15,15,7,8},
	{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	},

};

void Scumm::setCursor(int cursor) {
	MouseCursor *cur = &mouse_cursors[cursor];
	int i,j;
	byte *mask;
	const byte *src;
	byte shramount;
	uint32 data;

	debug(1,"Loading cursor %d", cursor);
	gdi._hotspot_x = cur->hotspot_x;
	gdi._hotspot_y = cur->hotspot_y;
	gdi._currentCursor = cursor;

	for (i=0; i<4; i++)
		gdi._mouseColors[i] = cur->colors[i];
	
	mask = gdi._mouseMask;
	shramount = 0;

	for(j=0; j<8; j++) {
		src = cur->data;	
		i=16;
		do {
			data = ((src[0]<<16) | (src[1]<<8))>>shramount;
			src += 2;
			mask[0] = (byte)(data>>24);
			mask[1] = (byte)(data>>16);
			mask[2] = (byte)(data>>8);
			mask[3] = (byte)(data);
			mask += 4;
		} while (--i);
		shramount++;
	}
}

void Scumm::setCameraAt(int dest) {
	int t;
	CameraData *cd = &camera;

	if (cd->_mode!=2 || abs(dest - cd->_curPos) > 160) {
		cd->_curPos = dest;
	}
	cd->_destPos = dest;

	t = _vars[VAR_CAMERA_MIN];
	if (cd->_curPos < t) cd->_curPos = t;

	t = _vars[VAR_CAMERA_MAX];
	if (cd->_curPos > t) cd->_curPos = t;

	if (_vars[VAR_SCROLL_SCRIPT]) {
		_vars[VAR_CAMERA_CUR_POS] = cd->_curPos;
		runScript(_vars[VAR_SCROLL_SCRIPT], 0, 0, 0);
	}

	if (cd->_curPos != cd->_lastPos && charset._hasMask)
		stopTalk();
}

void Scumm::setCameraFollows(Actor *a) {
	int t,i;
	CameraData *cd = &camera;

	cd->_mode = 2;
	cd->_follows = a->number;

	if (a->room != _currentRoom) {
		startScene(a->room, 0, 0);
		cd->_mode = 2;
		cd->_curPos = a->x;
		setCameraAt(cd->_curPos);
	}

	t = (a->x >> 3);

	if (t-_screenStartStrip < cd->_leftTrigger ||
		  t-_screenStartStrip > cd->_rightTrigger)
		setCameraAt(a->x);

	for (i=1,a=getFirstActor(); ++a,i<NUM_ACTORS; i++) {
		if (a->room==_currentRoom)
			a->needRedraw = true;
	}
	runHook(0);
}

void Scumm::initBGBuffers() {
	byte *ptr;
	int size, itemsize, i;
	byte *room;

	room = getResourceAddress(rtRoom, _roomResource);

	ptr = findResource(MKID('RMIH'), findResource(MKID('RMIM'), room, 0), 0);

	gdi._numZBuffer = READ_LE_UINT16(ptr+8) + 1;

	assert(gdi._numZBuffer>=1 && gdi._numZBuffer<=4);
	
	itemsize = (_scrHeight + 4) * 40;
	size = itemsize * gdi._numZBuffer;

	createResource(rtBuffer, 9, size);
	
	for (i=0; i<4; i++)
		gdi._imgBufOffs[i] = i*itemsize;
}

void Scumm::setPaletteFromPtr(byte *ptr) {
	uint32 size = READ_BE_UINT32_UNALIGNED(ptr+4);
	int i, r, g, b;
	byte *dest, *epal;
	int numcolor;

	numcolor = (size-8) / 3;

	ptr += 8;

	checkRange(256, 0, numcolor, "Too many colors (%d) in Palette");

	dest = _currentPalette;

	for (i=0; i<numcolor; i++) {
		r = *ptr++;
		g = *ptr++;
		b = *ptr++;
		if (i<=15 || r<252 || g<252 || b<252) {
			*dest++ = r>>2;
			*dest++ = g>>2;
			*dest++ = b>>2;
		} else {
			dest += 3;
		}
	}

#if 0
	if (_videoMode==0xE) {
		epal = getResourceAddress(rtRoom, _roomResource) + _EPAL_offs + 8;
		for (i=0; i<256; i++,epal++) {
			_currentPalette[i] = *epal&0xF;
			_currentPalette[i+256] = *epal>>4;
		}
	}
#endif

	setDirtyColors(0, numcolor-1);
}

void Scumm::setPaletteFromRes() {
	byte *ptr;
	ptr = getResourceAddress(rtRoom, _roomResource) + _CLUT_offs;
	setPaletteFromPtr(ptr);
}


void Scumm::setDirtyColors(int min, int max) {
	if (_palDirtyMin > min)
		_palDirtyMin = min;
	if (_palDirtyMax < max)
		_palDirtyMax = max;
}

void Scumm::initCycl(byte *ptr) {
	int i, j;
	ColorCycle *cycl;

	for (i=0,cycl=_colorCycle; i<16; i++,cycl++)
		cycl->delay = 0;

	while ((j=*ptr++) != 0) {
		if (j<1 || j>16) {
			error("Invalid color cycle index %d", j);
		}
		cycl = &_colorCycle[j-1];

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

void Scumm::stopCycle(int i) {
	ColorCycle *cycl;

	checkRange(16, 0, i, "Stop Cycle %d Out Of Range");
	if (i!=0) {
		_colorCycle[i-1].delay = 0;
		return;
	}

	for (i=0,cycl=_colorCycle; i<16; i++,cycl++)
		cycl->delay = 0;
}

void Scumm::cyclePalette() {
	ColorCycle *cycl;
	int valueToAdd;
	int i, num;
	byte *start, *end;
	byte tmp[3];

	valueToAdd = _vars[VAR_TIMER];
	if (valueToAdd < _vars[VAR_TIMER_NEXT])
		valueToAdd = _vars[VAR_TIMER_NEXT];

	for (i=0,cycl=_colorCycle; i<16; i++,cycl++) {
		if (cycl->delay &&
			(cycl->counter+=valueToAdd) >= cycl->delay) {
			do {
				cycl->counter -= cycl->delay;
			} while (cycl->delay <= cycl->counter);

			setDirtyColors(cycl->start, cycl->end);
			moveMemInPalRes(cycl->start, cycl->end, cycl->flags&2);
			start = &_currentPalette[cycl->start*3];
			end = &_currentPalette[cycl->end*3];

			num = cycl->end - cycl->start;

			if (!(cycl->flags&2)) {
				memmove(tmp, end, 3);
				memmove(start+3, start, num*3);
				memmove(start, tmp, 3);
			} else {
				memmove(tmp, start, 3);
				memmove(start, start+3, num*3);
				memmove(end, tmp, 3);
			}
		}
	}
}

void Scumm::moveMemInPalRes(int start, int end, byte direction) {
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
	
	if (!direction) {
		memmove(tmp, endptr, 6);
		memmove(startptr+6, startptr, num*6);
		memmove(startptr, tmp, 6);
		memmove(tmp2, endptr2, 6);
		memmove(startptr2+6, startptr2, num*6);
		memmove(startptr2, tmp2, 6);
	} else {
		memmove(tmp, startptr, 6);
		memmove(startptr, startptr+6, num*6);
		memmove(endptr, tmp, 6);
		memmove(tmp2, startptr2, 6);
		memmove(startptr2, startptr2+6, num*6);
		memmove(endptr2, tmp2, 6);
	}
}

void Scumm::unkVirtScreen4(int a) {
	VirtScreen *vs;

	setDirtyRange(0, 0, 0);
	camera._lastPos = camera._curPos;
	if (!_screenEffectFlag)
		return;
	_screenEffectFlag = false;
	
	if (a==0)
		return;

	vs = &virtscr[0];
	gdi._backbuff_ptr = getResourceAddress(rtBuffer, 1) + vs->xstart;
	memset(gdi._backbuff_ptr, 0, vs->size);

	switch(a) {
	case 1: case 2: case 3:
		unkScreenEffect7(a-1);
		break;
	case 128:
		unkScreenEffect6();
		break;
	case 129:
		setDirtyRange(0, 0, vs->height);
		updateDirtyScreen(0);
		/* XXX: EGA_proc4(0); */
		break;
	case 134:
		unkScreenEffect5(0);
		break;
	case 135:
		unkScreenEffect5(1);
		break;
	default:
		error("unkVirtScreen4: default case %d", a);
	}
}

void Scumm::redrawBGAreas() {
	int i;
	int val;
	CameraData *cd = &camera;

	if (cd->_curPos!=cd->_lastPos && charset._hasMask)
		stopTalk();

	val = 0;

	if (_fullRedraw==0 && _BgNeedsRedraw) {
		for (i=0; i<40; i++) {
			if (actorDrawBits[_screenStartStrip + i]&0x8000) {
				redrawBGStrip(i, 1);
			}
		}
	}

	if (_fullRedraw==0 && cd->_curPos - cd->_lastPos == 8) {
		val = 2;
		redrawBGStrip(39, 1);
	} else if (_fullRedraw==0 && cd->_curPos - cd->_lastPos == -8) {
		val = 1;
		redrawBGStrip(0, 1);
	} else if (_fullRedraw!=0 || cd->_curPos != cd->_lastPos) {
		_BgNeedsRedraw = 0;
		redrawBGStrip(0, 40);
	}

	drawRoomObjects(val);
	_BgNeedsRedraw = 0;
}

const uint32 zplane_tags[] = {
	MKID('ZP00'),
	MKID('ZP01'),
	MKID('ZP02'),
	MKID('ZP03')
};

void Gdi::drawBitmap(byte *ptr, VirtScreen *vs, int x, int y, int h, int stripnr, int numstrip, bool flag) {
	byte *smap_ptr,*where_draw_ptr;
	int i;
	byte *zplane_list[4];
	int bottom;
	byte twobufs;
	int numzbuf;
	int sx;
	
	CHECK_HEAP

	smap_ptr = findResource(MKID('SMAP'), ptr, 0);

	assert(smap_ptr);

	numzbuf = _disable_zbuffer ? 0 : _numZBuffer;

	for(i=1; i<numzbuf; i++) {
		zplane_list[i] = findResource(zplane_tags[i], ptr, 0);
	}

	bottom = y + h;
	if (bottom > vs->height) {
		error("Gdi::drawBitmap, strip drawn to %d below window bottom %d", bottom, vs->height);
	}

	twobufs = vs->alloctwobuffers;

	_vertStripNextInc = h * 320 - 1;

	_numLinesToProcess = h;

	do {
		_smap_ptr = smap_ptr + READ_LE_UINT32(smap_ptr + stripnr*4 + 8);

		CHECK_HEAP

		sx = x;
		if (vs->scrollable)
			sx -= vs->xstart>>3;
	
		if ((uint)sx >= 40) 
			return;

		if (y < vs->tdirty[sx])
			vs->tdirty[sx]=y;

		if (bottom > vs->bdirty[sx])
			vs->bdirty[sx] = bottom;
		
		_backbuff_ptr = _vm->getResourceAddress(rtBuffer, vs->number+1) + (y*40+x)*8;
		_bgbak_ptr = _vm->getResourceAddress(rtBuffer, vs->number+5) + (y*40+x)*8;
		if (!twobufs) {
			_bgbak_ptr = _backbuff_ptr;
		}
		_mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + (y*40+x);

		where_draw_ptr = _bgbak_ptr;
		decompressBitmap();

		CHECK_HEAP

		if (twobufs) {
			_bgbak_ptr = where_draw_ptr;
			
			if (_vm->hasCharsetMask(sx<<3, y, (sx+1)<<3, bottom)) {
				if (_vm->_vars[VAR_V5_DRAWFLAGS]&2)
					draw8ColWithMasking();
				else
					clear8ColWithMasking();
			} else {
				if (_vm->_vars[VAR_V5_DRAWFLAGS]&2)
					blit(_backbuff_ptr, _bgbak_ptr, 8, h);
				else
					clear8Col();
			}
		}
		CHECK_HEAP

		for (i=1; i<numzbuf; i++) {
			if (!zplane_list[i])
				continue;
			_z_plane_ptr = zplane_list[i] + READ_LE_UINT16(zplane_list[i] + stripnr*2 + 8);
			_mask_ptr_dest = _vm->getResourceAddress(rtBuffer, 9) + y*40 + x + _imgBufOffs[i];
			if (_useOrDecompress && flag)
				decompressMaskImgOr();
			else
				decompressMaskImg();
		}
		CHECK_HEAP
		x++;
		stripnr++;
	} while (--numstrip);
}


void Gdi::decompressBitmap() {
	const byte decompress_table[] = {
		0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF, 0x0,
	};

	_useOrDecompress = false;

	byte code = *_smap_ptr++;

	assert(_numLinesToProcess);

	switch(code) {
	case 1:
		unkDecode7();
		break;
	case 14: case 15: case 16: case 17: case 18:
		_decomp_shr = code - 10;
		_decomp_mask = decompress_table[code - 10];
		unkDecode6();
		break;

	case 24: case 25: case 26: case 27: case 28:
		_decomp_shr = code - 20;
		_decomp_mask = decompress_table[code - 20];
		unkDecode5();
		break;
	
	case 34: case 35: case 36: case 37: case 38:
		_useOrDecompress = true;
		_decomp_shr = code - 30;
		_decomp_mask = decompress_table[code - 30 ];
		unkDecode4();
		break;

	case 44: case 45: case 46: case 47: case 48:
		_useOrDecompress = true;
		_decomp_shr = code - 40;
		_decomp_mask = decompress_table[code - 40];
		unkDecode2();
		break;

	case 64: case 65: case 66: case 67: case 68:
		_decomp_shr = code - 60;
		_decomp_mask = decompress_table[code - 60];
		unkDecode1();
		break;

	case 84: case 85: case 86: case 87: case 88:
		_useOrDecompress = true;
		_decomp_shr = code - 80;
		_decomp_mask = decompress_table[code - 80];
		unkDecode3();
		break;

	/* New since version 6 */
	case 104: case 105: case 106: case 107: case 108:
		_decomp_shr = code - 100;
		_decomp_mask = decompress_table[code - 100];
		unkDecode1();
		break;
	
	/* New since version 6 */
	case 124: case 125: case 126: case 127: case 128:
		_useOrDecompress = true;
		_decomp_shr = code - 120;
		_decomp_mask = decompress_table[code - 120];
		unkDecode3();
		break;

	default:
		error("Gdi::decompressBitmap: default case %d", code);
	}
}

int Scumm::hasCharsetMask(int x, int y, int x2, int y2) {
	if (!charset._hasMask || y > gdi._mask_bottom || x > gdi._mask_right || 
		y2 < gdi._mask_top || x2 < gdi._mask_left )	
		return 0;
	return 1;
}

void Gdi::draw8ColWithMasking() {
	int height = _numLinesToProcess;
	byte *mask = _mask_ptr;
	byte *dst = _backbuff_ptr;
	byte *src = _bgbak_ptr;
	byte maskbits;

	do {
		maskbits = *mask;
		if (maskbits) {
			if (!(maskbits&0x80)) dst[0] = src[0];
			if (!(maskbits&0x40)) dst[1] = src[1];
			if (!(maskbits&0x20)) dst[2] = src[2];
			if (!(maskbits&0x10)) dst[3] = src[3];
			if (!(maskbits&0x08)) dst[4] = src[4];
			if (!(maskbits&0x04)) dst[5] = src[5];
			if (!(maskbits&0x02)) dst[6] = src[6];
			if (!(maskbits&0x01)) dst[7] = src[7];
		} else {
/* alignment safe */
			((uint32*)dst)[0] = ((uint32*)src)[0];
			((uint32*)dst)[1] = ((uint32*)src)[1];
		}
		src += 320;
		dst += 320;
		mask += 40;
	} while (--height);
}

void Gdi::clear8ColWithMasking() {
	int height = _numLinesToProcess;
	byte *mask = _mask_ptr;
	byte *dst = _backbuff_ptr;
	byte maskbits;

	do {
		maskbits = *mask;
		if (!maskbits) {
			((uint32*)dst)[1] = ((uint32*)dst)[0] = 0;
		} else {
			if (!(maskbits&0x80)) dst[0] = 0;
			if (!(maskbits&0x40)) dst[1] = 0;
			if (!(maskbits&0x20)) dst[2] = 0;
			if (!(maskbits&0x10)) dst[3] = 0;
			if (!(maskbits&0x08)) dst[4] = 0;
			if (!(maskbits&0x04)) dst[5] = 0;
			if (!(maskbits&0x02)) dst[6] = 0;
			if (!(maskbits&0x01)) dst[7] = 0;
		}
		dst += 320;
		mask += 40;
	} while (--height);
}

void Gdi::clear8Col() {
	int height = _numLinesToProcess;
	byte *dst = _backbuff_ptr;
	do {
		((uint32*)dst)[1] = ((uint32*)dst)[0] = 0;
		dst += 320;
	} while (--height);
}

void Gdi::decompressMaskImg() {
	byte *src = _z_plane_ptr;
	byte *dst = _mask_ptr_dest;
	int height = _numLinesToProcess;
	byte b, c;

	while(1) {
		b = *src++;

		if (b&0x80) {
			b&=0x7F;
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

void Gdi::decompressMaskImgOr() {
	byte *src = _z_plane_ptr;
	byte *dst = _mask_ptr_dest;
	int height = _numLinesToProcess;
	byte b, c;

	while(1) {
		b = *src++;
		if (b&0x80) {
			b&=0x7F;
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

void Scumm::redrawBGStrip(int start, int num) {
	int s = _screenStartStrip + start;

	assert(s>=0 && s<sizeof(actorDrawBits)/sizeof(actorDrawBits[0]));

	_curVirtScreen = &virtscr[0];

	actorDrawBits[s]|=0x8000;
	if (_curVirtScreen->height > _scrHeight) {
		error("Screen Y size %d < Room height %d",
			_curVirtScreen->height,
			_scrHeight);
	}

	gdi.drawBitmap(getResourceAddress(rtRoom, _roomResource)+_IM00_offs,
		_curVirtScreen, s, 0, _curVirtScreen->height, s, num, 0);
}

#define READ_BIT (cl--,bit = bits&1, bits>>=1,bit)
#define FILL_BITS if (cl <= 8) { bits |= (*src++ << cl); cl += 8;}

void Gdi::unkDecode1() {
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	byte incm,reps;
	_tempNumLines = _numLinesToProcess;

	do {	
		_currentX = 8;
		do {
			FILL_BITS
			*dst++=color;

againPos:;

			if (!READ_BIT) {} 
			else if (READ_BIT) {
				incm = (bits&7)-4;
				cl-=3;
				bits>>=3;
				if (!incm) {
					FILL_BITS
					reps = bits&0xFF;
					do {
						if (!--_currentX) {
							_currentX = 8;
							dst += 312;
							if (!--_tempNumLines)
								return;
						}
						*dst++=color;
					} while (--reps);
					bits>>=8;
					bits |= (*src++)<<(cl-8);
					goto againPos;
				} else {
					color += incm;
				}
			} else {
				FILL_BITS
				color = bits&_decomp_mask;
				cl -= _decomp_shr;
				bits >>= _decomp_shr;
			}
		} while (--_currentX);
		dst += 312;
	} while (--_tempNumLines);
}

void Gdi::unkDecode2() {
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
			FILL_BITS
			if (color!=_transparency)
				*dst=color;
			dst++;
			if (!READ_BIT) {}
			else if (!READ_BIT) {
				FILL_BITS
				color = bits&_decomp_mask;
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

void Gdi::unkDecode3() {
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	byte incm,reps;

	_tempNumLines = _numLinesToProcess;

	do {	
		_currentX = 8;
		do {
			FILL_BITS
			if (color!=_transparency) *dst=color;
			dst++;

againPos:;
			if (!READ_BIT) {}
			else if (READ_BIT) {
				incm = (bits&7)-4;
				
				cl-=3;
				bits>>=3;
				if (incm) {
					color += incm;
				} else {
					FILL_BITS
					reps = bits&0xFF;
					if (color==_transparency) {
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
							*dst++=color;
						} while (--reps);
					}
					bits>>=8;
					bits |= (*src++)<<(cl-8);
					goto againPos;
				}
			} else {
				FILL_BITS
				color = bits&_decomp_mask;
				cl -= _decomp_shr;
				bits >>= _decomp_shr;
			}
		} while (--_currentX);
		dst += 312;
	} while (--_tempNumLines);
}


void Gdi::unkDecode4() {
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
			FILL_BITS
			if (color!=_transparency)
				*dst=color;
			dst+=320;
			if (!READ_BIT) {}
			else if (!READ_BIT) {
				FILL_BITS
				color = bits&_decomp_mask;
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

void Gdi::unkDecode5() {
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
			FILL_BITS
			*dst++=color;
			if (!READ_BIT) {}
			else if (!READ_BIT) {
				FILL_BITS
				color = bits&_decomp_mask;
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

void Gdi::unkDecode6() {
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
			FILL_BITS
			*dst=color;
			dst+=320;
			if (!READ_BIT) {}
			else if (!READ_BIT) {
				FILL_BITS
				color = bits&_decomp_mask;
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

void Gdi::unkDecode7() {
	byte *src = _smap_ptr;
	byte *dst = _bgbak_ptr;
	int height = _numLinesToProcess;
	do {
		/* Endian safe */
#if defined(SCUMM_NEED_ALIGNMENT)
		memcpy(dst, src, 8);
#else
		((uint32*)dst)[0] = ((uint32*)src)[0];
		((uint32*)dst)[1] = ((uint32*)src)[1];
#endif
		dst += 320;
		src += 8;
	} while (--height);
}

#undef READ_BIT
#undef FILL_BITS

void Scumm::restoreCharsetBg() {
	_bkColor = 0;
	if (gdi._mask_left != -1) {
		restoreBG(gdi._mask_left, gdi._mask_top, gdi._mask_right, gdi._mask_bottom);
		charset._hasMask = false;
		gdi._mask_left = -1;
		charset._strLeft = -1;
		charset._left = -1;
	}
	
	charset._xpos2 = string[0].xpos;
	charset._ypos2 = string[0].ypos;
}

void Scumm::restoreBG(int left, int top, int right, int bottom) {
	VirtScreen *vs;
	int topline, height, width, widthmod;
	byte *backbuff,*bgbak,*mask;

	if (left==right || top==bottom)
		return;
	if (top<0) top=0;
	
	if ((vs=findVirtScreen(top)) == NULL)
		return;

	topline = vs->topline;
	height = topline + vs->height;
	if (vs->number==0) {
		left += _lastXstart - vs->xstart;
		right += _lastXstart - vs->xstart;
	}

	right++;
	if (left<0) left=0;
	if (right<0)right=0;
	if (left>320)
		return;
	if (right>320)
		right=320;
	if (bottom>=height)
		bottom=height;

	updateDirtyRect(vs->number, left, right, top-topline,bottom-topline, 0x4000);

	height = (top-topline) * 320 + vs->xstart + left;
	
	backbuff = getResourceAddress(rtBuffer, vs->number+1) + height;
	bgbak = getResourceAddress(rtBuffer, vs->number+5) + height;
	mask = getResourceAddress(rtBuffer, 9) + top * 40 + (left>>3) + _screenStartStrip;
	if (vs->number==0) {
		mask += vs->topline * 216;
	}

	height = bottom - top;
	width = right - left;
	widthmod = (width >> 2) + 2;

	if (vs->alloctwobuffers && _currentRoom!=0 && _vars[VAR_V5_DRAWFLAGS]&2) {
		blit(backbuff, bgbak, width, height);
		if (vs->number==0 && charset._hasMask && height) {
			do {
				memset(mask, 0, widthmod);
				mask += 40;
			} while (--height);
		}
	} else {
		if (height) {
			do {
				memset(backbuff, _bkColor, width);
				backbuff+=320;
			} while (--height);
		}
	}
}

void Scumm::updateDirtyRect(int virt, int left, int right, int top, int bottom, uint16 dirtybits) {
	VirtScreen *vs = &virtscr[virt];
	int lp,rp;
	uint16 *sp;
	int num;

	if (top > vs->height || bottom < 0)
		return;
	
	if (top<0)
		top=0;
	if (bottom > vs->height)
		bottom = vs->height;

	if (virt==0 && dirtybits) {
		rp = (right >> 3) + _screenStartStrip;
		lp = (left >> 3) + _screenStartStrip;
		if (lp<0) lp=0;
		if (rp >= 200)
			rp = 200;
		if (lp <= rp) {
			num = rp - lp + 1;
			sp = &actorDrawBits[lp];
			do {
				*sp++ |= dirtybits;
			} while (--num);
		}
	}

	setVirtscreenDirty(vs, left, top, right, bottom);
}

void Scumm::setVirtscreenDirty(VirtScreen *vs, int left, int top, int right, int bottom) {
	int lp = left >> 3;
	int rp = right >> 3;

	if (lp>=40 || rp<0)
		return;
	if (lp<0) lp=0;
	if (rp>=40) rp=39;

	while (lp<=rp) {
		if (top < vs->tdirty[lp])
			vs->tdirty[lp] = top;
		if (bottom > vs->bdirty[lp])
			vs->bdirty[lp] = bottom;
		lp++;
	}	
}

VirtScreen *Scumm::findVirtScreen(int y) {
	VirtScreen *vs = virtscr;
	int i;
	
	for(i=0; i<3; i++,vs++) {
		if (y >= vs->topline && y < vs->topline+vs->height) {
			return _curVirtScreen=vs;
		}	
	}
	return _curVirtScreen=NULL;
}

void Scumm::unkScreenEffect1() {
	/* XXX: not implemented */
	warning("stub unkScreenEffect1()");
}

void Scumm::unkScreenEffect2() {
	/* XXX: not implemented */
	warning("stub unkScreenEffect2()");
}

void Scumm::unkScreenEffect3() {
	/* XXX: not implemented */
	warning("stub unkScreenEffect3()");
}

void Scumm::unkScreenEffect4() {
	/* XXX: not implemented */
	warning("stub unkScreenEffect4()");
}

static const int8 screen_eff7_table1[4][16] = {
	{1,1,-1,1,-1,1,-1,-1,
     1,-1,-1,-1,1,1,1,-1},
	{0,1,2,1,2,0,2,1,
	 2,0,2,1,0,0,0,0},
	{-2,-1,0,-1,-2,-1,-2,0
	-2,-1,-2,0,0,0,0,0},
	{0,-1,-2,-1,-2,0,-2,-1
	-2,0,-2,-1,0,0,0,0}
};

static const byte screen_eff7_table2[4][16] = {
	{0,0,39,0,39,0,39,24,
	0,24,39,24,0,0,0,24},
	{0,0,0,0,0,0,0,0,
	1,0,1,0,255,0,0,0},
	{39,24,39,24,39,24,39,24,
	38,24,38,24,255,0,0,0},
	{0,24,39,24,39,0,39,24,
	38,0,38,24,255,0,0,0}
};

static const byte screen_eff7_table3[4] = {
	13,25,25,25
};

void Scumm::unkScreenEffect7(int a) {
	int tab_1[16];
	int tab_2[16];
	int i,j;
	int bottom;
	int *tab2_ptr;
	int l,t,r,b;

	for (i=0; i<16; i++) {
		tab_1[i] = screen_eff7_table1[a][i];
		j = screen_eff7_table2[a][i];
		if (j==24)
			j = (virtscr[0].height>>3)-1;
		tab_2[i] = j;
	}

	bottom = virtscr[0].height >> 3;
	for (j=0; j < screen_eff7_table3[a]; j++) {
		for (i=0; i<4; i++) {
			l = tab_2[i*4];
			t = tab_2[i*4+1];
			r = tab_2[i*4+2];
			b = tab_2[i*4+3];
			if (t==b) {
				while (l <= r) {
					if (l>=0 && l<40 && (uint)t<(uint)bottom) {
						virtscr[0].tdirty[l] = t<<3;
						virtscr[0].bdirty[l] = (t+1)<<3;
					}
					l++;
				}
			} else {
				/* DE92 */
				if (l<0 || l>=40 || b<=t)
					continue;
				if (b>bottom)
					b=bottom;
				virtscr[0].tdirty[l] = t<<3;
				virtscr[0].bdirty[l] = (b+1)<<3;
			}
			updateDirtyScreen(0);
		}

		for (i=0; i<16; i++)
			tab_2[i] += tab_1[i];
		
		updateScreen(this);
		waitForTimer(this,30);
	}
}

void Scumm::unkScreenEffect6() {
	/* XXX: not implemented */
	warning("stub unkScreenEffect6");
}
	
void Scumm::unkScreenEffect5(int a) {
	/* XXX: not implemented */
	warning("stub unkScreenEffect5(%d)",a);
}

void Scumm::setShake(int mode) {
	if (mode!=-1)
		_shakeMode = mode;
	else
		mode = 0;
	/* XXX: not implemented */
	warning("stub setShake(%d)",mode);
}

void Gdi::clearUpperMask() {
	memset(
		_vm->getResourceAddress(rtBuffer, 9),
		0,
		_imgBufOffs[1] - _imgBufOffs[0]
	);
}

void Scumm::moveCamera() {
	CameraData *cd = &camera;
	int pos = cd->_curPos;
	int actorx, t;
	Actor *a;

	cd->_curPos &= 0xFFF8;

	if (cd->_curPos < _vars[VAR_CAMERA_MIN]) {
		if (_vars[VAR_CAMERA_FAST])
			cd->_curPos = _vars[VAR_CAMERA_MIN];
		else
			cd->_curPos += 8;
		cameraMoved();
		return;
	}

	if (cd->_curPos > _vars[VAR_CAMERA_MAX]) {
		if (_vars[VAR_CAMERA_FAST])
			cd->_curPos = _vars[VAR_CAMERA_MAX];
		else
			cd->_curPos-=8;
		cameraMoved();
		return;
	}

	if (cd->_mode==2) {
		a = derefActorSafe(cd->_follows, "moveCamera");
		
		actorx = a->x;
		t = (actorx>>3) - _screenStartStrip;
		
		if (t < cd->_leftTrigger || t > cd->_rightTrigger) {
			if (_vars[VAR_CAMERA_FAST]) {
				if (t > 35)
					cd->_destPos = actorx + 80;
				if (t < 5)
					cd->_destPos = actorx - 80;
			} else 
				cd->_movingToActor = 1;
		}
	}

	if (cd->_movingToActor) {
		a = derefActorSafe(cd->_follows, "moveCamera(2)");
		cd->_destPos = a->x;
	}

	if (cd->_destPos < _vars[VAR_CAMERA_MIN])
		cd->_destPos = _vars[VAR_CAMERA_MIN];

	if (cd->_destPos > _vars[VAR_CAMERA_MAX])
		cd->_destPos = _vars[VAR_CAMERA_MAX];

	if (_vars[VAR_CAMERA_FAST]) {
		cd->_curPos = cd->_destPos;
	} else {
		if (cd->_curPos < cd->_destPos)
			cd->_curPos+=8;
		if (cd->_curPos > cd->_destPos)
			cd->_curPos-=8;
	}

	/* a is set a bit above */
	if (cd->_movingToActor && cd->_curPos>>3 == a->x>>3) {
		cd->_movingToActor = 0;
	}

	cameraMoved();

	if (pos != cd->_curPos && _vars[VAR_SCROLL_SCRIPT]) {
		_vars[VAR_CAMERA_CUR_POS] = cd->_curPos;
		runScript(_vars[VAR_SCROLL_SCRIPT], 0, 0, 0);
	}
}

void Scumm::cameraMoved() {
	CameraData *cd = &camera;

	if (cd->_curPos < 160) {
		cd->_curPos = 160;
	} else if (cd->_curPos + 160 >= _scrWidthIn8Unit<<3) {
		cd->_curPos = (_scrWidthIn8Unit-20)<<3;
	}

	_screenStartStrip = (cd->_curPos >> 3) - 20;
	_screenEndStrip = _screenStartStrip + 39;
	virtscr[0].xstart = _screenStartStrip << 3;
}

void Scumm::panCameraTo(int x) {
	CameraData *cd = &camera;
	cd->_destPos = x;
	cd->_mode = 3;
	cd->_movingToActor = 0;
}

void Scumm::actorFollowCamera(int act) {
	int old;
	CameraData *cd = &camera;

	/* mi1 compatibilty */
	if (act==0) {
		cd->_mode = 1;
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

void Scumm::setCameraAtEx(int at) {
	CameraData *cd = &camera;
	cd->_mode = 1;
	cd->_curPos = at;
	setCameraAt(at);
	cd->_movingToActor = 0;
}

void Scumm::palManipulate() {
	byte *srcptr, *destptr;
	byte *pal;
	int i,j;

	if (!_palManipCounter)
		return;
	srcptr = getResourceAddress(rtTemp, 4) + _palManipStart*6;
	destptr = getResourceAddress(rtTemp, 5) + _palManipStart*6;
	pal = _currentPalette + _palManipStart * 3;

	i = _palManipStart;
	while (i < _palManipEnd) {
		j = (*((uint16*)srcptr) += *(uint16*)destptr );
		*pal++ = j>>8;
		srcptr += 2;
		destptr += 2;

		j = (*((uint16*)srcptr) += *(uint16*)destptr );
		*pal++ = j>>8;
		srcptr += 2;
		destptr += 2;

		j = (*((uint16*)srcptr) += *(uint16*)destptr );
		*pal++ = j>>8;
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

void Scumm::swapPalColors(int a, int b) {
	byte *ap,*bp;
	byte t;

	if ((uint)a>=256 || (uint)b>=256)
		error("swapPalColors: invalid values, %d, %d", a, b);
	
	ap = &_currentPalette[a*3];
	bp = &_currentPalette[b*3];

	t=ap[0]; ap[0]=bp[0]; bp[0]=t;
	t=ap[1]; ap[1]=bp[1]; bp[1]=t;
	t=ap[2]; ap[2]=bp[2]; bp[2]=t;

	setDirtyColors(a,b);
}

void Scumm::screenEffect(int effect) {
	switch(effect) {
	case 1:
	case 2:
	case 3:		unkScreenEffect7(effect-1);	break;
	case 128:	unkScreenEffect6(); break;
	case 130:	unkScreenEffect1(); break;
	case 131:	unkScreenEffect2();	break;
	case 132:	unkScreenEffect3();	break;
	case 133:	unkScreenEffect4();	break;
	case 134:	unkScreenEffect5(0); break;
	case 135:	unkScreenEffect5(1); break;
	case 129: break;
	default:
		warning("Unknown screen effect, %d", effect);
	}
	_screenEffectFlag = true;
}

void Scumm::resetActorBgs() {
	Actor *a;
	int i,bitpos;
	int top,bottom;
	uint16 onlyActorFlags;
	int offs;
	
	for(i=0; i<40; i++) {
		onlyActorFlags = (actorDrawBits[_screenStartStrip + i]&=0x3FFF);
		a = getFirstActor();
		bitpos = 1;

		while (onlyActorFlags) {
			if(onlyActorFlags&1 && a->top!=0xFF && a->needBgReset) {
				top = a->top;
				bottom = a->bottom;
				actorDrawBits[_screenStartStrip + i] ^= bitpos;
				gdi.resetBackground(a->top, a->bottom, i);

			}
			bitpos<<=1;
			onlyActorFlags>>=1;
			a++;
		}
	}

	for(i=1,a=getFirstActor(); ++a,i<NUM_ACTORS; i++) {
		a->needBgReset = false;
	}
}

void Gdi::resetBackground(byte top, byte bottom, int strip) {
	VirtScreen *vs = &_vm->virtscr[0];
	int offs;

	if (top < vs->tdirty[strip])
		vs->tdirty[strip] = top;

	if (bottom > vs->bdirty[strip])
		vs->bdirty[strip] = bottom;
	
	offs = (top * 40 + _vm->_screenStartStrip + strip);
	_mask_ptr = _vm->getResourceAddress(rtBuffer, 9)	+ offs;
	_bgbak_ptr = _vm->getResourceAddress(rtBuffer, 5)	+ (offs<<3);
	_backbuff_ptr = _vm->getResourceAddress(rtBuffer, 1)	+ (offs<<3);
	
	_numLinesToProcess = bottom - top;
	if (_numLinesToProcess) {
		if (_vm->_vars[VAR_V5_DRAWFLAGS]&2) {
			if(_vm->hasCharsetMask(strip<<3, top, (strip+1)<<3, bottom))
				draw8ColWithMasking();
			else
				blit(_backbuff_ptr, _bgbak_ptr, 8, _numLinesToProcess);
		} else {
			clear8Col();
		}
	}
}

void Scumm::setPalColor(int index, int r, int g, int b) {
	_currentPalette[index*3+0] = r>>2;
	_currentPalette[index*3+1] = g>>2;
	_currentPalette[index*3+2] = b>>2;
	setDirtyColors(index,index);
}

void Scumm::drawMouse() {
	/* TODO: handle shake here */

	if (_cursorAnimate) {
		if (!(_cursorAnimateIndex&0x3))
			decompressDefaultCursor((_cursorAnimateIndex>>2)&3);
		_cursorAnimateIndex++;
		
	}

	::drawMouse(this, 
		mouse.x - _cursorHotspotX, 
		mouse.y - _cursorHotspotY,
		_cursorWidth,
		_cursorHeight,
		_grabbedCursor,
		gdi._cursorActive>0
	);
}

void Scumm::setCursorHotspot(int cursor, int x, int y) {
	MouseCursor *cur = &mouse_cursors[cursor];
	cur->hotspot_x = x;
	cur->hotspot_y = y;
}

void Scumm::setCursorHotspot2(int x,int y) {
	_cursorHotspotX = x;
	_cursorHotspotY = y;
}

byte Scumm::isMaskActiveAt(int l, int t, int r, int b, byte *mem) {
	int w,h,i;
	
	l>>=3;
	if (l<0) l = 0;
	if (t<0) t = 0;

	r>>=3;
	if (r>39) r=39;

	mem += l + t*40;

	w = r-l;
	h = b-t+1;

	do {
		for(i=0; i<=w; i++)
			if (mem[i])
				return true;
		mem += 40;
	} while (--h);
	
	return false;
}

void Scumm::setPalette(int palindex) {
	byte *pals;

	_curPalIndex = palindex;
	pals = getPalettePtr();
	if (pals==NULL)
		error("invalid palette %d", palindex);
	setPaletteFromPtr(pals);
}

byte *Scumm::findPalInPals(byte *pal, int index) {
	byte *offs;
	uint32 size;	

	pal = findResource(MKID('WRAP'), pal, 0);
	if (pal==NULL)
		return NULL;

	offs = findResource(MKID('OFFS'),pal, 0);
	if (offs==NULL)
		return NULL;

	size = (READ_BE_UINT32_UNALIGNED(offs+4)-8) >> 2;
	
	if ((uint32)index >= (uint32)size)
		return NULL;

	return offs + READ_LE_UINT32(offs + 8 + index * sizeof(uint32));
}

byte *Scumm::getPalettePtr() {
	byte *cptr;

	cptr = getResourceAddress(rtRoom, _roomResource);
	if (_CLUT_offs) {
		cptr += _CLUT_offs;
	} else {
		cptr = findPalInPals(cptr + _PALS_offs, _curPalIndex);
	}
	return cptr;
}

void Scumm::darkenPalette(int a, int b, int c, int d, int e) {
	byte *cptr, *cur;
	int num;
	byte color;

	cptr = getPalettePtr();
	cptr += 8 + a*3;
	cur = _currentPalette + a*3;
	if (a <= b) {
		num = b - a + 1;

		do {
			if (c != 0xFF) {
				color = *cptr++ * (c>>2) / 0xFF;
			} else {
				color = *cptr++ >> 2;
			}
			if(color>63) color = 63;
			*cur++=color;

			if (d != 0xFF) {
				color = *cptr++ * (d>>2) / 0xFF;
			} else {
				color = *cptr++ >> 2;
			}
			if(color>63) color = 63;
			*cur++=color;

			if (e != 0xFF) {
				color = *cptr++ * (e>>2) / 0xFF;
			} else {
				color = *cptr++ >> 2;
			}
			if(color>63) color = 63;
			*cur++=color;
		} while (--num);
	}
	setDirtyColors(a,b);
}

void Scumm::grabCursor(int x, int y, int w, int h) {
	VirtScreen *vs = findVirtScreen(y);

	if (vs==NULL) {
		warning("grabCursor: invalid Y %d", y);
		return;
	}

	grabCursor(
		getResourceAddress(rtBuffer, vs->number+1) + (y-vs->topline)*320 + x, 
		w,h);

}

void Scumm::decompressBomp(byte *dst, byte *src, int w, int h) {
	int len,num;
	byte code,color;

	src += 8;

	do {
		len = w;
		src += 2;
		while (len) {
			code = *src++;
			num = (code>>1)+1;
			if (num>len) num=len;
			len -= num;
			if (code&1) {
				color = *src++;
				do *dst++ = color; while (--num);
			} else {
				do *dst++ = *src++; while (--num);
			}
		}
	} while (--h);
}

void Scumm::grabCursor(byte *ptr, int width, int height) {
	uint size;
	byte *dst;
	
	size = width * height;
	if (size > sizeof(_grabbedCursor))
		error("grabCursor: grabbed cursor too big");

	_cursorWidth = width;
	_cursorHeight = height;
	_cursorAnimate = 0;

	dst = _grabbedCursor;
	for(;height;height--) {
		memcpy(dst, ptr, width);
		dst += width;
		ptr += 320;
	}

}

void Scumm::useIm01Cursor(byte *im, int w, int h) {
	VirtScreen *vs = &virtscr[0];
	
	w<<=3;
	h<<=3;
	
	drawBox(0,0,w-1,h-1,0xFF);

	vs->alloctwobuffers = false;
	gdi._disable_zbuffer = true;
	gdi.drawBitmap(im, vs, _screenStartStrip, 0, h, 0, w>>3, 0);
	vs->alloctwobuffers = true;
	gdi._disable_zbuffer = false;

	grabCursor(getResourceAddress(rtBuffer, 1) + vs->xstart, w, h);
	
	blit(getResourceAddress(rtBuffer, 1) + vs->xstart, getResourceAddress(rtBuffer, 5) + vs->xstart, w, h);
}

void Scumm::useBompCursor(byte *im, int width, int height) {
	uint size;

	width<<=3;
	height<<=3;
	
	size = width * height;
	if (size > sizeof(_grabbedCursor))
		error("useBompCursor: cursor too big");

	_cursorWidth = width;
	_cursorHeight = height;
	_cursorAnimate = 0;

	decompressBomp(_grabbedCursor, im+10, width, height);
}

static const byte default_cursor_colors[4] = {
	15,15,7,8
};

void Scumm::decompressDefaultCursor(int index) {
	int i;
	byte color;

	memset(_grabbedCursor, 0xFF, sizeof(_grabbedCursor));
	_cursorWidth = 16;
	_cursorHeight = 16;
	_cursorHotspotX = 8;
	_cursorHotspotY = 7;

	color = default_cursor_colors[index];
	
	for(i=0; i<16; i++) {
		_grabbedCursor[16*8+i] = color;
		_grabbedCursor[16*i+8] = color;
	}
}
