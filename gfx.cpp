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
 * Change Log:
 * $Log$
 * Revision 1.8  2001/10/23 19:51:50  strigeus
 * recompile not needed when switching games
 * debugger skeleton implemented
 *
 * Revision 1.7  2001/10/17 10:07:39  strigeus
 * fixed verbs not saved in non dott games,
 * implemented a screen effect
 *
 * Revision 1.6  2001/10/17 07:12:37  strigeus
 * fixed nasty signed/unsigned bug
 *
 * Revision 1.5  2001/10/16 20:31:27  strigeus
 * misc fixes
 *
 * Revision 1.4  2001/10/16 10:01:47  strigeus
 * preliminary DOTT support
 *
 * Revision 1.3  2001/10/10 12:52:21  strigeus
 * fixed bug in GDI_UnkDecode7()
 *
 * Revision 1.2  2001/10/10 10:02:33  strigeus
 * alternative mouse cursor
 * basic save&load
 *
 * Revision 1.1.1.1  2001/10/09 14:30:14  strigeus
 *
 * initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"

void Scumm::getGraphicsPerformance() {
	int i;
	_scummTimer = 0;

	for (i=10; i!=0; i--) {
		initScreens(0, 0, 320, 200);
	}

	_vars[VAR_PERFORMANCE_1] = _scummTimer;
	_scummTimer = 0;

	for (i=10; i!=0; i--) {
		setDirtyRange(0, 0, 200);
		unkVirtScreen2();
	}

	_vars[VAR_PERFORMANCE_2] = _scummTimer;
	
	initScreens(0, 16, 320, 144);
}

void Scumm::initScreens(int a, int b, int w, int h) {
	int i;

	for (i=0; i<3; i++) {
		nukeResource(10, i+1);
		nukeResource(10, i+5);
	}

	if (!getResourceAddress(10,4)) {
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

	memset(
		createResource(10, slot+1, size),
		0,size);

	if (twobufs) {
		memset(
			createResource(10, slot+5, size),
			0x23,size);
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

void Scumm::unkVirtScreen2() {
	dseg_3DB6 = 1;

	updateDirtyScreen(2);

	if (_currentRoom != dseg_2456) {
		dseg_2456 = (byte)_currentRoom;
		dseg_4E8A = 1;
	}

	if (camera._lastPos == camera._curPos) {
		updateDirtyScreen(0);
		dseg_4E8A = 1;
		dseg_4F8C = _screenStartStrip;
	} else {
		gdi.readPtr = getResourceAddress(10, 1);
		gdi.readOffs = _screenStartStrip;
		gdi.drawY = virtscr[0].topline;
		gdi.drawHeight = virtscr[0].height;
		
		blitToScreen(this, gdi.readPtr + gdi.readOffs*8, 0, gdi.drawY, 320, gdi.drawHeight);

		for (gdi.draw8xPos = 0; gdi.draw8xPos<40; gdi.draw8xPos++) {
			virtscr[0].tdirty[gdi.draw8xPos] = (byte)gdi.drawHeight;
			virtscr[0].bdirty[gdi.draw8xPos] = 0;
		}
	}
}

void Scumm::updateDirtyScreen(int slot) {
	VirtScreen *vs = &virtscr[slot];
	int i;

	if (vs->height==0)
		return;

	gdi.virtScreen = slot;
	gdi.drawY = vs->topline;
	gdi.drawHeight = vs->height;
	gdi.readOffs = 0;
	if (vs->scrollable)
		gdi.readOffs = _screenStartStrip;

	if (_videoMode==0xE) {
		for (i=0; i<40; i++) {
			gdi.draw8xPos = i;
			gdi.drawBottom = vs->bdirty[i];
			if (gdi.drawBottom) {
				gdi.drawTop = vs->tdirty[i];
				drawStripToScreen();
				vs->tdirty[i] = (byte)gdi.drawHeight;
				vs->bdirty[i] = 0;
			}
		}
	} else {
		gdi.drawWidth = 8;
		gdi.draw8xPos = 0;

		for (i=0; i<40; i++) {
			gdi.drawBottom = vs->bdirty[i];
			if (gdi.drawBottom) {
				gdi.drawTop = vs->tdirty[i];
				vs->tdirty[i] = (byte)gdi.drawHeight;
				vs->bdirty[i] = 0;
				if (i!=39) {
					if (vs->bdirty[i+1] == gdi.drawBottom &&
							vs->tdirty[i+1] == gdi.drawTop) {
						gdi.drawWidth += 8;
						continue;
					}
					drawStripToScreen();
					gdi.drawWidth = 8;
				} else {
					drawStripToScreen();
				}
			}
			gdi.draw8xPos = i+1;
		}
	}
}

void Scumm::drawStripToScreen() {
	if (gdi.drawBottom <= gdi.drawTop)
		return;

	if (_videoMode==0xE) {
		gdi.drawTop &= 0xFE;
		if (gdi.drawBottom&1)
			gdi.drawBottom++;
	}

	if (gdi.drawBottom <= gdi.drawTop) {
		gdi.drawBottom = gdi.drawTop + 4;
	}

	if (gdi.drawHeight < gdi.drawTop) {
		gdi.drawTop = 0;
	}

	if (gdi.drawHeight < gdi.drawBottom) {
		gdi.drawBottom = gdi.drawHeight;
	}

	gdi.readPtr = getResourceAddress(10, gdi.virtScreen+1) + (gdi.drawTop*40+gdi.readOffs+gdi.draw8xPos)*8;

	blitToScreen(this, gdi.readPtr, gdi.draw8xPos*8, gdi.drawY+gdi.drawTop, gdi.drawWidth, gdi.drawBottom-gdi.drawTop);
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
	gdi.hotspot_x = cur->hotspot_x;
	gdi.hotspot_y = cur->hotspot_y;
	gdi.currentCursor = cursor;

	for (i=0; i<4; i++)
		gdi.mouseColors[i] = cur->colors[i];
	
	mask = gdi.mouseMask;
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

	for (i=1,a=getFirstActor(); ++a,i<13; i++) {
		if (a->room==_currentRoom)
			a->needRedraw = true;
	}
	runHook(0);
}

void Scumm::initBGBuffers() {
	byte *ptr;
	int size, itemsize, i;
	byte *room;

	room = getResourceAddress(1, _roomResource);

	ptr = findResource(MKID('RMIH'), findResource(MKID('RMIM'), room));

	_numZBuffer = READ_LE_UINT16(ptr+8) + 1;

	assert(_numZBuffer>=1 && _numZBuffer<=4);
	
	itemsize = (_scrHeight + 4) * 40;
	size = itemsize * _numZBuffer;

	memset(createResource(10, 9, size), 0, size);
	
	for (i=0; i<4; i++)
		_imgBufOffs[i] = i*itemsize;
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

	if (_videoMode==0x13) {
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
	}
	
	if (_videoMode==0xE) {
		epal = getResourceAddress(1, _roomResource) + _EPAL_offs + 8;
		for (i=0; i<256; i++,epal++) {
			_currentPalette[i] = *epal&0xF;
			_currentPalette[i+256] = *epal>>4;
		}
	}
	
	setDirtyColors(0, numcolor-1);
}

void Scumm::setPaletteFromRes() {
	byte *ptr;
	ptr = getResourceAddress(1, _roomResource) + _CLUT_offs;
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

	for (i=1; i<=16; i++)
		_colorCycleDelays[i] = 0;

	while ((j=*ptr++) != 0) {
		assert(j>=1 && j<=16);
		ptr += 2;
		_colorCycleCounter[j] = 0;
		_colorCycleDelays[j] = 16384 / READ_BE_UINT16_UNALIGNED(ptr);
		ptr += 2;
		_colorCycleFlags[j] = READ_BE_UINT16_UNALIGNED(ptr);
		ptr += 2;
		_colorCycleStart[j] = *ptr++;
		_colorCycleEnd[j] = *ptr++;
	}
}

void Scumm::stopCycle(int i) {
	checkRange(16, 0, i, "Stop Cycle %d Out Of Range");
	if (i!=0) {
		_colorCycleDelays[i] = 0;
		return;
	}
	for (i=1; i<=16; i++)
		_colorCycleDelays[i] = 0;
}

void Scumm::cyclePalette() {
	int valueToAdd;
	int i, num;
	byte *start, *end;
	byte tmp[3];

	if(_videoMode != 0x13)
		return;

	valueToAdd = _vars[VAR_TIMER];
	if (valueToAdd < _vars[VAR_TIMER_NEXT])
		valueToAdd = _vars[VAR_TIMER_NEXT];

	for (i=1; i<=16; i++) {
		if (_colorCycleDelays[i] &&
			(_colorCycleCounter[i]+=valueToAdd) >= 
				_colorCycleDelays[i]) {
			do {
				_colorCycleCounter[i] -= _colorCycleDelays[i];
			} while (_colorCycleDelays[i] <= _colorCycleCounter[i]);

			setDirtyColors(_colorCycleStart[i], _colorCycleEnd[i]);
			moveMemInPalRes(_colorCycleStart[i], _colorCycleEnd[i],
				_colorCycleFlags[i]&2);
			start = &_currentPalette[_colorCycleStart[i]*3];
			end = &_currentPalette[_colorCycleEnd[i]*3];

			num = _colorCycleEnd[i] - _colorCycleStart[i];

			if (!(_colorCycleFlags[i]&2)) {
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

	startptr = getResourceAddress(0xC, 4) + start * 6;
	endptr = getResourceAddress(0xC, 4) + end * 6;

	startptr2 = getResourceAddress(0xC, 5) + start * 6;
	endptr2 = getResourceAddress(0xC, 5) + end * 6;

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
	dseg_3DB6 = 2;
	dseg_3DB6 = 1;
	if (dseg_4EA0 == 0)
		return;
	dseg_4EA0 = 0;
	
	if (a==0)
		return;

	vs = &virtscr[0];
	gdi.bg_ptr = getResourceAddress(0xA, 1) + vs->xstart;

	memset(gdi.bg_ptr, 0, vs->size);

	switch(a) {
	case 1: case 2: case 3:
		unkScreenEffect7(a-1);
		break;
	case 128:
		unkScreenEffect6();
		break;
	case 129:
		setDirtyRange(0, 0, vs->height);
		dseg_3DB6 = 2;
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
		dseg_719E = 0;
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

void Scumm::drawBmp(byte *ptr, int a, int b, int c, const char *str, int objnr) {
	byte *smap_ptr;
	int i;
	byte *zplane_list[4];
	int t;
	VirtScreen *vs;
	byte twobufs;
	int x;
	byte *where_draw_ptr;

	CHECK_HEAP

	smap_ptr = findResource(MKID('SMAP'), ptr);

	for(i=1; i<_numZBuffer; i++) {
		zplane_list[i] = findResource(zplane_tags[i], ptr);
	}

	t = gdi.numLinesToProcess + _drawBmpY;

	vs = &virtscr[gdi.virtScreen];
	if (t > vs->height) {
		error("%s %d strip drawn to %d below window bottom %d",
			str, objnr, t, vs->height);
	}

	twobufs = vs->alloctwobuffers;

	dseg_4174 = vs->size;

	if (vs->scrollable)
		dseg_4174 += 5*256;
	
	gdi.vertStripNextInc = gdi.numLinesToProcess * 320 - 1;
	
	do {
		gdi.smap_ptr = smap_ptr + READ_LE_UINT32(smap_ptr + a*4 + 8);

		x = _drawBmpX;
		if (vs->scrollable)
			x -= _screenStartStrip;

		CHECK_HEAP

		if ((uint)x >= 40) 
			return;

		if (_drawBmpY < vs->tdirty[x])
			vs->tdirty[x]=_drawBmpY;

		if (t > vs->bdirty[x])
			vs->bdirty[x]=t;
		
		gdi.bg_ptr = getResourceAddress(0xA, gdi.virtScreen+1) + (_drawBmpY*40+_drawBmpX)*8;
		gdi.where_to_draw_ptr = getResourceAddress(0xA, gdi.virtScreen+5) + (_drawBmpY*40+_drawBmpX)*8;
		if (!twobufs) {
			gdi.where_to_draw_ptr = gdi.bg_ptr;
		}
		gdi.mask_ptr = getResourceAddress(0xA, 9) + (_drawBmpY*40+_drawBmpX);

		where_draw_ptr = gdi.where_to_draw_ptr;
		decompressBitmap();

		CHECK_HEAP

		if (twobufs) {
			gdi.where_to_draw_ptr = where_draw_ptr;
			if (_vars[VAR_DRAWFLAGS]&2) {
				if (hasCharsetMask(x<<3, _drawBmpY, (x+1)<<3, t))
					draw8ColWithMasking();
				else {
					blit(gdi.bg_ptr, gdi.where_to_draw_ptr, 8, gdi.numLinesToProcess);
				}
			} else {
				if (hasCharsetMask(x<<3, _drawBmpY, (x+1)<<3, t))
					clear8ColWithMasking();
				else
					clear8Col();
			}
		}
		CHECK_HEAP

		for (i=1; i<_numZBuffer; i++) {
			if (!zplane_list[i])
				continue;
			gdi.z_plane_ptr = zplane_list[i] + READ_LE_UINT16(zplane_list[i] + a*2 + 8);
			gdi.mask_ptr_dest = getResourceAddress(0xA, 9) + _drawBmpY*40 + _drawBmpX + _imgBufOffs[i];
			if (dseg_4E3B!=0 && c!=0)
				decompressMaskImgOr();
			else
				decompressMaskImg();
		}
		CHECK_HEAP
		_drawBmpX++;
		a++;
	} while (--b);
}


void Scumm::decompressBitmap() {
	const byte decompress_table[] = {
		0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF, 0x0,
	};

	dseg_4E3B = 0;

	byte code = *gdi.smap_ptr++;

	assert(gdi.numLinesToProcess);

	switch(code) {
	case 1:
		GDI_UnkDecode7();
		break;
	case 14: case 15: case 16: case 17: case 18:
		gdi.decomp_shr = code - 10;
		gdi.decomp_mask = decompress_table[code - 10];
		GDI_UnkDecode6();
		break;

	case 24: case 25: case 26: case 27: case 28:
		gdi.decomp_shr = code - 20;
		gdi.decomp_mask = decompress_table[code - 20];
		GDI_UnkDecode5();
		break;
	
	case 34: case 35: case 36: case 37: case 38:
		dseg_4E3B = 1;
		gdi.decomp_shr = code - 30;
		gdi.decomp_mask = decompress_table[code - 30 ];
		GDI_UnkDecode4();
		break;

	case 44: case 45: case 46: case 47: case 48:
		dseg_4E3B = 1;
		gdi.decomp_shr = code - 40;
		gdi.decomp_mask = decompress_table[code - 40];
		GDI_UnkDecode2();
		break;

	case 64: case 65: case 66: case 67: case 68:
		gdi.decomp_shr = code - 60;
		gdi.decomp_mask = decompress_table[code - 60];
		GDI_UnkDecode1();
		break;

	case 84: case 85: case 86: case 87: case 88:
		dseg_4E3B = 1;
		gdi.decomp_shr = code - 80;
		gdi.decomp_mask = decompress_table[code - 80];
		GDI_UnkDecode3();
		break;

	/* New since version 6 */
	case 104: case 105: case 106: case 107: case 108:
		gdi.decomp_shr = code - 100;
		gdi.decomp_mask = decompress_table[code - 100];
		GDI_UnkDecode1();
		break;
	
	/* New since version 6 */
	case 124: case 125: case 126: case 127: case 128:
		dseg_4E3B = 1;
		gdi.decomp_shr = code - 120;
		gdi.decomp_mask = decompress_table[code - 120];
		GDI_UnkDecode3();
		break;

	default:
		error("decompressBitmap: default case %d", code);
	}
}

int Scumm::hasCharsetMask(int x, int y, int x2, int y2) {
	if (!charset._hasMask || y > string[0].mask_bottom || x > string[0].mask_right || 
		y2 < string[0].mask_top || x2 < string[0].mask_left )	
		return 0;
	return 1;
}

void Scumm::draw8ColWithMasking() {
	int height = gdi.numLinesToProcess;
	byte *mask = gdi.mask_ptr;
	byte *dst = gdi.bg_ptr;
	byte *src = gdi.where_to_draw_ptr;
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

void Scumm::clear8ColWithMasking() {
	int height = gdi.numLinesToProcess;
	byte *mask = gdi.mask_ptr;
	byte *dst = gdi.bg_ptr;
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

void Scumm::clear8Col() {
	int height = gdi.numLinesToProcess;
	byte *dst = gdi.bg_ptr;
	do {
		((uint32*)dst)[1] = ((uint32*)dst)[0] = 0;
		dst += 320;
	} while (--height);
}

void Scumm::decompressMaskImg() {
	byte *src = gdi.z_plane_ptr;
	byte *dst = gdi.mask_ptr_dest;
	int height = gdi.numLinesToProcess;
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

void Scumm::decompressMaskImgOr() {
	byte *src = gdi.z_plane_ptr;
	byte *dst = gdi.mask_ptr_dest;
	int height = gdi.numLinesToProcess;
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

	gdi.virtScreen = 0;
	actorDrawBits[s]|=0x8000;
	_drawBmpX = s;
	_drawBmpY = 0;
	gdi.numLinesToProcess = virtscr[0].height;
	if (gdi.numLinesToProcess > _scrHeight) {
		error("Screen Y size %d < Room height %d",
			gdi.numLinesToProcess,
			_scrHeight);
	}
	
	drawBmp(
		getResourceAddress(1, _roomResource) + _IM00_offs,
		s,
		num,
		0,
		"Room",
		_roomResource);
}

#define READ_BIT (cl--,bit = bits&1, bits>>=1,bit)
#define FILL_BITS if (cl <= 8) { bits |= (*src++ << cl); cl += 8;}

void Scumm::GDI_UnkDecode1() {
	byte *src = gdi.smap_ptr;
	byte *dst = gdi.where_to_draw_ptr;
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	byte incm,reps;
	gdi.tempNumLines = gdi.numLinesToProcess;

	do {	
		gdi.currentX = 8;
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
						if (!--gdi.currentX) {
							gdi.currentX = 8;
							dst += 312;
							if (!--gdi.tempNumLines)
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
				color = bits&gdi.decomp_mask;
				cl -= gdi.decomp_shr;
				bits >>= gdi.decomp_shr;
			}
		} while (--gdi.currentX);
		dst += 312;
	} while (--gdi.tempNumLines);
}

#if 0
void Scumm::GDI_UnkDecode1() {
	byte *src = gdi.smap_ptr;
	byte *dst = gdi.where_to_draw_ptr;
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	gdi.tempNumLines = gdi.numLinesToProcess;

	do {	
		gdi.currentX = 8;
		do {
			FILL_BITS
			*dst++=color;

			if (!READ_BIT) {} 
			else if (READ_BIT) {
				color += (bits&7)-4;
				cl-=3;
				bits>>=3;
			} else {
				FILL_BITS
				color = bits&gdi.decomp_mask;
				cl -= gdi.decomp_shr;
				bits >>= gdi.decomp_shr;
			}
		} while (--gdi.currentX);
		dst += 312;
	} while (--gdi.tempNumLines);
}
#endif

void Scumm::GDI_UnkDecode2() {
	byte *src = gdi.smap_ptr;
	byte *dst = gdi.where_to_draw_ptr;
	byte color = *src++;
	int8 inc = -1;
	uint bits = *src++;
	byte cl = 8;
	byte bit;

	gdi.tempNumLines = gdi.numLinesToProcess;

	do {	
		gdi.currentX = 8;
		do {
			FILL_BITS
			if (color!=gdi.transparency)
				*dst=color;
			dst++;
			if (!READ_BIT) {}
			else if (!READ_BIT) {
				FILL_BITS
				color = bits&gdi.decomp_mask;
				bits >>= gdi.decomp_shr;
				cl -= gdi.decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--gdi.currentX);
		dst += 312;
	} while (--gdi.tempNumLines);
}

void Scumm::GDI_UnkDecode3() {
	byte *src = gdi.smap_ptr;
	byte *dst = gdi.where_to_draw_ptr;
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	byte incm,reps;

	gdi.tempNumLines = gdi.numLinesToProcess;

	do {	
		gdi.currentX = 8;
		do {
			FILL_BITS
			if (color!=gdi.transparency) *dst=color;
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
					if (color==gdi.transparency) {
						do {
							if (!--gdi.currentX) {
								gdi.currentX = 8;
								dst += 312;
								if (!--gdi.tempNumLines)
									return;
							}
							dst++;
						} while (--reps);
					} else {
						do {
							if (!--gdi.currentX) {
								gdi.currentX = 8;
								dst += 312;
								if (!--gdi.tempNumLines)
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
				color = bits&gdi.decomp_mask;
				cl -= gdi.decomp_shr;
				bits >>= gdi.decomp_shr;
			}
		} while (--gdi.currentX);
		dst += 312;
	} while (--gdi.tempNumLines);
}

#if 0
void Scumm::GDI_UnkDecode3() {
	byte *src = gdi.smap_ptr;
	byte *dst = gdi.where_to_draw_ptr;
	byte color = *src++;
	uint bits = *src++;
	byte cl = 8;
	byte bit;

	gdi.tempNumLines = gdi.numLinesToProcess;

	do {	
		gdi.currentX = 8;
		do {
			FILL_BITS
			if (color!=gdi.transparency) *dst=color;
			dst++;

			if (!READ_BIT) {}
			else if (READ_BIT) {
				color += (bits&7)-4;
				cl-=3;
				bits>>=3;
			} else {
				FILL_BITS
				color = bits&gdi.decomp_mask;
				cl -= gdi.decomp_shr;
				bits >>= gdi.decomp_shr;
			}
		} while (--gdi.currentX);
		dst += 312;
	} while (--gdi.tempNumLines);
}
#endif


void Scumm::GDI_UnkDecode4() {
	byte *src = gdi.smap_ptr;
	byte *dst = gdi.where_to_draw_ptr;
	byte color = *src++;
	int8 inc = -1;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	
	gdi.currentX = 8;
	do {	
		gdi.tempNumLines = gdi.numLinesToProcess;
		do {
			FILL_BITS
			if (color!=gdi.transparency)
				*dst=color;
			dst+=320;
			if (!READ_BIT) {}
			else if (!READ_BIT) {
				FILL_BITS
				color = bits&gdi.decomp_mask;
				bits >>= gdi.decomp_shr;
				cl -= gdi.decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--gdi.tempNumLines);	
		dst -= gdi.vertStripNextInc;
	} while (--gdi.currentX);
}

void Scumm::GDI_UnkDecode5() {
	byte *src = gdi.smap_ptr;
	byte *dst = gdi.where_to_draw_ptr;
	byte color = *src++;
	int8 inc = -1;
	uint bits = *src++;
	byte cl = 8;
	byte bit;

	gdi.tempNumLines = gdi.numLinesToProcess;

	do {	
		gdi.currentX = 8;
		do {
			FILL_BITS
			*dst++=color;
			if (!READ_BIT) {}
			else if (!READ_BIT) {
				FILL_BITS
				color = bits&gdi.decomp_mask;
				bits >>= gdi.decomp_shr;
				cl -= gdi.decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--gdi.currentX);
		dst += 312;
	} while (--gdi.tempNumLines);
}

void Scumm::GDI_UnkDecode6() {
	byte *src = gdi.smap_ptr;
	byte *dst = gdi.where_to_draw_ptr;
	byte color = *src++;
	int8 inc = -1;
	uint bits = *src++;
	byte cl = 8;
	byte bit;
	
	gdi.currentX = 8;
	do {	
		gdi.tempNumLines = gdi.numLinesToProcess;
		do {
			FILL_BITS
			*dst=color;
			dst+=320;
			if (!READ_BIT) {}
			else if (!READ_BIT) {
				FILL_BITS
				color = bits&gdi.decomp_mask;
				bits >>= gdi.decomp_shr;
				cl -= gdi.decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--gdi.tempNumLines);	
		dst -= gdi.vertStripNextInc;
	} while (--gdi.currentX);
}

void Scumm::GDI_UnkDecode7() {
	byte *src = gdi.smap_ptr;
	byte *dst = gdi.where_to_draw_ptr;
	int height = gdi.numLinesToProcess;
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
	dseg_4E3C = 0;
	if (string[0].mask_left != -1) {
		restoreBG(string[0].mask_left, string[0].mask_top, string[0].mask_right, string[0].mask_bottom);
		charset._hasMask = false;
		string[0].mask_left = -1;
		charset._strLeft = -1;
		charset._left = -1;
	}
	
	string[0].xpos2 = string[0].xpos;
	string[0].ypos2 = string[0].ypos;
}

void Scumm::restoreBG(int left, int top, int right, int bottom) {
	VirtScreen *vs;
	int topline, height, width, widthmod;

	if (left==right || top==bottom)
		return;
	if (top<0) top=0;
	
	if (findVirtScreen(top) == -1)
		return;
	
	vs = &virtscr[gdi.virtScreen];

	topline = vs->topline;
	height = topline + vs->height;
	if (gdi.virtScreen==0) {
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

	updateDirtyRect(gdi.virtScreen, left, right, top-topline,bottom-topline, 0x4000);

	vs = &virtscr[gdi.virtScreen];
	height = (top-topline) * 320 + vs->xstart + left;
	
	gdi.bg_ptr = getResourceAddress(0xA, gdi.virtScreen+1) + height;
	gdi.where_to_draw_ptr = getResourceAddress(0xA, gdi.virtScreen+5) + height;
	gdi.mask_ptr = getResourceAddress(0xA, 9) + top * 40 + (left>>3) + _screenStartStrip;
	if (gdi.virtScreen==0) {
		gdi.mask_ptr += vs->topline * 216;
	}

	height = bottom - top;
	width = right - left;
	widthmod = (width >> 2) + 2;

	if (vs->alloctwobuffers && _currentRoom!=0 && _vars[VAR_DRAWFLAGS]&2) {
		blit(gdi.bg_ptr, gdi.where_to_draw_ptr, width, height);
		if (gdi.virtScreen==0 && charset._hasMask && height) {
			do {
				memset(gdi.mask_ptr, 0, widthmod);
				gdi.mask_ptr += 40;
			} while (--height);
		}
	} else {
		if (height) {
			do {
				memset(gdi.bg_ptr, dseg_4E3C, width);
				gdi.bg_ptr+=320;
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

	rp = right >> 3;
	lp = left >> 3;

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

int Scumm::findVirtScreen(int y) {
	VirtScreen *vs = virtscr;
	int i;

	gdi.virtScreen=-1;

	for(i=0; i<3; i++,vs++) {
		if (y >= vs->topline && y < vs->topline+vs->height) {
			gdi.virtScreen = i;
			return i;
		}	
	}
	return -1;
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
		waitForTimer(this);
	}
	/* XXX: not implemented */
	warning("stub unkScreenEffect7(%d)", a);
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

void Scumm::clearUpperMask() {
	memset(
		getResourceAddress(0xA, 9),
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
	int old = camera._follows;

	setCameraFollows(derefActorSafe(act, "actorFollowCamera"));
	if (camera._follows != old) 
		runHook(0);

	camera._movingToActor = 0;
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
	srcptr = getResourceAddress(0xC, 4) + _palManipStart*6;
	destptr = getResourceAddress(0xC, 5) + _palManipStart*6;
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
		nukeResource(0xC, 4);
		nukeResource(0xC, 5);
	}
}

void Scumm::screenEffect(int effect) {
	dseg_3DB6 = 1;
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
	default:
		warning("Unknown screen effect, %d", effect);
	}
	dseg_4EA0 = 1;
}

void Scumm::resetActorBgs() {
	Actor *a;
	int i,bitpos;
	int top,bottom;
	uint16 onlyActorFlags;
	
	for(i=0; i<40; i++) {
		onlyActorFlags = (actorDrawBits[_screenStartStrip + i]&=0x3FFF);
		a = getFirstActor();
		bitpos = 1;

		while (onlyActorFlags) {
			if(onlyActorFlags&1 && a->top!=0xFF && a->needBgReset) {
				top = a->top;
				bottom = a->bottom;
				if (a->top < virtscr[0].tdirty[i])
					virtscr[0].tdirty[i] = a->top;

				if (a->bottom > virtscr[0].bdirty[i])
					virtscr[0].bdirty[i] = a->bottom;
				actorDrawBits[_screenStartStrip + i] ^= bitpos;
				
				gdi.where_to_draw_ptr = getResourceAddress(0xA, 5)
					+ ((top * 40 + _screenStartStrip + i)<<3);
				gdi.bg_ptr = getResourceAddress(0xA, 1)
					+ ((top * 40 + _screenStartStrip + i)<<3);
				gdi.mask_ptr = getResourceAddress(0xA, 9)
					+ (top * 40 + _screenStartStrip + i);
				gdi.numLinesToProcess = bottom - top;
				if (gdi.numLinesToProcess) {
					if (_vars[VAR_DRAWFLAGS]&2) {
						if(hasCharsetMask(i<<3, top, (i+1)<<3, bottom))
							draw8ColWithMasking();
						else
							blit(gdi.bg_ptr, gdi.where_to_draw_ptr, 8, gdi.numLinesToProcess);
					} else {
						clear8Col();
					}
				}
			}
			bitpos<<=1;
			onlyActorFlags>>=1;
			a++;
		}
	}

	for(i=1,a=getFirstActor(); ++a,i<13; i++) {
		a->needBgReset = false;
	}
}

void Scumm::setPalColor(int index, int r, int g, int b) {
	if(_videoMode==0x13) {
		_currentPalette[index*3+0] = r>>2;
		_currentPalette[index*3+1] = g>>2;
		_currentPalette[index*3+2] = b>>2;
		setDirtyColors(index,index);
	}
	if (_videoMode==0xE) {
		/* TODO: implement this */
		warning("stub setPalColor(%d,%d,%d,%d)",index,r,g,b);
	}
}

void Scumm::drawMouse() {
	/* TODO: handle shake here */

	::drawMouse(this,
		mouse.x - gdi.hotspot_x,
		mouse.y - gdi.hotspot_y,
		gdi.mouseColors[((++gdi.mouseColorIndex)>>2)&3],
		gdi.mouseMask + ((gdi.drawMouseX&7)<<6),
		gdi.unk4>0
		);
}

void Scumm::setCursorHotspot(int cursor, int x, int y) {
	MouseCursor *cur = &mouse_cursors[cursor];
	cur->hotspot_x = x;
	cur->hotspot_y = y;
}

void Scumm::setCursorImg(int cursor, int img) {
	MouseCursor *cur = &mouse_cursors[cursor];
	byte *ptr;

	ptr = getResourceAddress(6, 1);
	
//	offs = ((uint32*)ptr)[img+1];
//	if (!offs)
//		return;

	warning("setCursorImg: not implemented");
}

byte Scumm::isMaskActiveAt(int l, int t, int r, int b, byte *mem) {
	int w,h,inc,i;

	mem += b*40 + (l>>3);

	w = (r>>3) - (l>>3) + 1;
	inc = w+40;
	h = b-t-1;

	do {
		for(i=0; i<w; i++)
			if (mem[i])
				return true;
		mem -= 40;
	} while (--h);
	
	return false;
}

byte *Scumm::findPalInPals(byte *pal, int index) {
	byte *offs;
	uint32 size;	

	pal = findResource(MKID('WRAP'), pal);
	if (pal==NULL)
		return NULL;

	offs = findResource(MKID('OFFS'),pal);
	if (offs==NULL)
		return NULL;

	size = (READ_BE_UINT32_UNALIGNED(offs+4)-8) >> 2;
	
	if ((uint32)index >= (uint32)size)
		return NULL;

	return offs + READ_LE_UINT32(offs + 8 + index * sizeof(uint32));
}

void Scumm::setPalette(int palindex) {
	byte *pals;

	_curPalIndex = palindex;

	pals = getResourceAddress(1, _roomResource) + _PALS_offs;

	pals = findPalInPals(pals, palindex);

	if (pals==NULL)
		error("invalid palette %d", palindex);

	setPaletteFromPtr(pals);
}

#if 0
void Scumm::GDI_drawMouse() {
	byte *dst,*src,*dstorg;
	int y,h;
	byte color,val;

	byte mask1,mask2,mask3;

	int offs = gdi.drawMouseY*320 + (gdi.drawMouseX&0xFFF8);

	/* XXX: check bounds here */
	if (offs<0 || offs > 320*200 - 16*320)
		return;
	
	gdi.backupIsWhere = _vgabuf + offs;

	src = gdi.backupIsWhere;
	dst = gdi.mouseBackup;

	h=16;
	do {
		memcpy(dst,src,24);
		dst+=24;
		src+=320;
	} while (--h);

	src = gdi.mouseMaskPtr;
	color = gdi.mouseColor;
	dstorg = gdi.backupIsWhere;

	h=16;
	do {
		src++;
		mask1 = *src++;
		mask2 = *src++;
		mask3 = *src++;

		y = gdi.drawMouseY++;
		if (y>199)
			mask1 = mask2 = 0;

		val = mask1 & gdi.mouseClipMask1;
		dst = dstorg;
		do {
			if(val&0x80)
				*dst = color;
			dst++;
		} while (val<<=1);

		val = mask2 & gdi.mouseClipMask2;
		dst = dstorg + 8;
		do {
			if(val&0x80)
				*dst = color;
			dst++;
		} while (val<<=1);

		val = mask3 & gdi.mouseClipMask3;
		dst = dstorg + 16;
		do {
			if(val&0x80)
				*dst = color;
			dst++;
		} while (val<<=1);
		
		dstorg += 320;
	} while (--h);
}

void Scumm::GDI_removeMouse() {
	byte *dst,*src,h;
	if (gdi.backupIsWhere) {
		dst = gdi.backupIsWhere;
		gdi.backupIsWhere = NULL;
		src = gdi.mouseBackup;
		h=16;
		do {
			memcpy(dst,src,24);
			dst+=320;
			src+=24;
		} while (--h);
	}
}
#endif
