/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"

#include "parallaction/graphics.h"
#include "parallaction/parser.h"
#include "parallaction/parallaction.h"
#include "parallaction/inventory.h"
#include "parallaction/disk.h"
#include "parallaction/zone.h"


extern OSystem *g_system;

namespace Parallaction {


uint16	_bgLayers[4];

//
//	proportional font glyphs width
//
const byte _glyphWidths[126] = {
  0x04, 0x03, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x04, 0x04, 0x06, 0x06, 0x03, 0x05, 0x03, 0x05,
  0x06, 0x06, 0x06, 0x06, 0x07, 0x06, 0x06, 0x06, 0x06, 0x06, 0x03, 0x03, 0x05, 0x04, 0x05, 0x05,
  0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
  0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07, 0x07, 0x05, 0x06, 0x05, 0x08, 0x07,
  0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x03, 0x04, 0x05, 0x05, 0x06, 0x06, 0x05,
  0x05, 0x06, 0x05, 0x05, 0x05, 0x05, 0x06, 0x07, 0x05, 0x05, 0x05, 0x05, 0x02, 0x05, 0x05, 0x07,
  0x08, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x04, 0x04, 0x04,
  0x05, 0x06, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x04, 0x06, 0x05, 0x05, 0x05, 0x05
};

bool		Gfx::_proportionalFont = false;
Point		Gfx::_labelPosition[2] = { { 0, 0 }, { 0, 0 } };
StaticCnv	Gfx::_mouseComposedArrow;
byte *		Gfx::_buffers[];

#define PALETTE_BACKUP	PALETTE_SIZE

PaletteFxRange		_palettefx[6];
byte				_palette[PALETTE_SIZE] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

byte _black_palette[PALETTE_SIZE] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


#define BALLOON_WIDTH	12
#define BALLOON_HEIGHT	10

byte _resBalloon[2][BALLOON_WIDTH*BALLOON_HEIGHT] = {
	{
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x00, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	},
	{
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x02, 0x02
	}
};

void Gfx::drawBalloon(const Common::Rect& r, uint16 winding) {
//	printf("Gfx::drawBalloon(%i, %i, %i, %i, %i)...", left, top, width, height, winding);

	floodFill(0, r.left, r.top, r.right+5, r.bottom, kBitFront);
	floodFill(1, r.left+1, r.top+2, r.right+5-1, r.bottom-1, kBitFront);

	winding = (winding == 0 ? 1 : 0);
	byte *s = _resBalloon[winding];
	byte *d = _buffers[kBitFront] + (r.left + (r.width()+5)/2 - 5) + (r.bottom - 1) * SCREEN_WIDTH;

	for (uint16 i = 0; i < BALLOON_HEIGHT; i++) {
		for (uint16 j = 0; j < BALLOON_WIDTH; j++) {
			if (*s != 2) *d = *s;
			d++;
			s++;
		}

		d += (SCREEN_WIDTH - BALLOON_WIDTH);
	}

//	printf("done\n");

	return;
}



//
//	palette management
//

void Gfx::setPalette(byte *palette) {
//	printf("setPalette()\n");
//	memcpy(_palette, palette, PALETTE_SIZE);

	byte syspal[PALETTE_COLORS*4];

	for (uint32 i = 0; i < PALETTE_COLORS; i++) {
		syspal[i*4]   = (palette[i*3] << 2) | (palette[i*3] >> 4);
		syspal[i*4+1] = (palette[i*3+1] << 2) | (palette[i*3+1] >> 4);
		syspal[i*4+2] = (palette[i*3+2] << 2) | (palette[i*3+2] >> 4);
		syspal[i*4+3] = 0;
	}

	g_system->setPalette(syspal, 0, PALETTE_COLORS);
	g_system->updateScreen();
	return;
}

void Gfx::getBlackPalette(byte *palette) {
	memcpy(palette, _black_palette, PALETTE_SIZE);
	return;
}

void Gfx::palUnk0(byte *palette) {
#if 0
	for (uint16 i = 0; i < PALETTE_SIZE; i++) {
		palette[PALETTE_BACKUP+i] = _palette[i]/2;
	}
#endif
	Gfx::setPalette(palette);

	return;
}

void Gfx::buildBWPalette(byte *palette) {

	for (uint16 i = 0; i < PALETTE_COLORS; i++) {
		byte max;

		if (_palette[i*3+1] > _palette[i*3+2]) {
			max = _palette[i*3+1];
		} else {
			max = _palette[i*3+2];
		}

		if (_palette[i*3] > max) {
			max = _palette[i*3];
		} else {
			if (_palette[i*3+1] > _palette[i*3+2]) {
				max = _palette[i*3+1];
			} else {
				max = _palette[i*3+2];
			}
		}

		palette[i*3] = max;
		palette[i*3+1] = max;
		palette[i*3+2] = max;
	}

	return;
}

void Gfx::fadePalette(byte *palette) {

	for (uint16 i = 0; i < PALETTE_SIZE; i++)
		if (palette[i] < _palette[i]) palette[i]++;


	return;
}

void Gfx::quickFadePalette(byte *palette) {

	for (uint16 i = 0; i < PALETTE_SIZE; i++) {
		if (palette[i] == _palette[i]) continue;
		palette[i] += (palette[i] < _palette[i] ? 4 : -4);
	}

	return;
}

//
//	palette Animation
//
//	FIXME: the effect is different from the original
//
void Gfx::animatePalette(byte *palette) {
// printf("Gfx::animatePalette()\n");

	byte tmp[3];

	for (uint16 i = 0; i < 4; i++) {

		if ((_palettefx[i]._flags & 1) == 0) continue;		// animated palette

		_palettefx[i]._timer += _palettefx[i]._step * 2;	// update timer

		if (_palettefx[i]._timer < 0x4000) continue;		// check timeout

		_palettefx[i]._timer = 0;							// reset timer

		if (_palettefx[i]._flags & 2) { 					// forward

			tmp[0] = _palette[_palettefx[i]._first * 3];
			tmp[1] = _palette[_palettefx[i]._first * 3 + 1];
			tmp[2] = _palette[_palettefx[i]._first * 3 + 2];

			memmove(palette+_palettefx[i]._first*3, _palette+(_palettefx[i]._first+1)*3, (_palettefx[i]._last - _palettefx[i]._first)*3);

			palette[_palettefx[i]._last * 3]	 = tmp[0];
			palette[_palettefx[i]._last * 3 + 1] = tmp[1];
			palette[_palettefx[i]._last * 3 + 2] = tmp[2];

		} else {											// backward

			tmp[0] = _palette[_palettefx[i]._last * 3];
			tmp[1] = _palette[_palettefx[i]._last * 3 + 1];
			tmp[2] = _palette[_palettefx[i]._last * 3 + 2];

			memmove(palette+(_palettefx[i]._first+1)*3, _palette+_palettefx[i]._first*3, (_palettefx[i]._last - _palettefx[i]._first)*3);

			palette[_palettefx[i]._first * 3]	  = tmp[0];
			palette[_palettefx[i]._first * 3 + 1] = tmp[1];
			palette[_palettefx[i]._first * 3 + 2] = tmp[2];

		}

	}

	return;
}




void Gfx::updateScreen() {
//	  printf("Gfx::updateScreen()\n");
	g_system->copyRectToScreen(_buffers[kBitFront], SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_system->updateScreen();
	return;
}


void Gfx::swapBuffers() {
	byte *temp = _buffers[kBitFront];
	_buffers[kBitFront] = _buffers[kBitBack];
	_buffers[kBitBack] = temp;
	updateScreen();
	return;
}


//
//	graphic primitives
//
void Gfx::clearScreen(Gfx::Buffers buffer) {
	memset(_buffers[buffer], 0, SCREEN_WIDTH*SCREEN_HEIGHT);

	if (buffer == kBitFront) updateScreen();

	return;
}


void Gfx::copyScreen(Gfx::Buffers srcbuffer, Gfx::Buffers dstbuffer) {
	memcpy(_buffers[dstbuffer], _buffers[srcbuffer], SCREEN_WIDTH*SCREEN_HEIGHT);

	if (dstbuffer == kBitFront) updateScreen();

	return;
}


void Gfx::copyRect(Gfx::Buffers srcbuffer, uint16 sx, uint16 sy, Gfx::Buffers dstbuffer, uint16 dx, uint16 dy, uint16 w, uint16 h) {

	byte *s = _buffers[srcbuffer] + (sx + sy * SCREEN_WIDTH);
	byte *d = _buffers[dstbuffer] + (dx + dy * SCREEN_WIDTH);

	for (uint16 i = 0; i < h; i++) {
		memcpy(d, s, w);

		s += SCREEN_WIDTH;
		d += SCREEN_WIDTH;
	}

	if (dstbuffer == kBitFront) updateScreen();

	return;

}


void Gfx::floodFill(byte color, uint16 left, uint16 top, uint16 right, uint16 bottom, Gfx::Buffers buffer) {
//	printf("Gfx::floodFill(%i, %i, %i, %i, %i)\n", color, left, top, right, bottom);

	byte *d = _buffers[buffer] + (left + top * SCREEN_WIDTH);
	uint16 w = right - left + 1;
	uint16 h = bottom - top + 1;

	for (uint16 i = 0; i < h; i++) {
		memset(d, color, w);

		d += SCREEN_WIDTH;
	}

	if (buffer == kBitFront) updateScreen();

	return;
}


void Gfx::flatBlit(uint16 w, uint16 h, int16 x, int16 y, byte *data, Gfx::Buffers buffer) {
	debugC(9, kDebugGraphics, "Gfx::flatBlit(%i, %i, %i, %i)", w, h, x, y);

	// source coordinates
	int16 left = 0, top = 0;
	int16 right = w, bottom = h;

	if (x + w > SCREEN_WIDTH)
		right = SCREEN_WIDTH - x;
	if (y + h > SCREEN_HEIGHT)
		bottom = SCREEN_HEIGHT - y;

	if (x < 0) {	// partially left clipped
		left = -x;
		x = 0;
	}

	if (y < 0) {	// partially top clipped
		top = -y;
		y = 0;
	}

	if (left > right || top > bottom || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;	// fully clipped

	byte *s = data + left + top * w;
	byte *d = _buffers[buffer] + x + y * SCREEN_WIDTH;

	debugC(9, kDebugGraphics, "Gfx::flatBlit CLIPPED (%i, %i, %i, %i) -> (%i, %i) %p %p", left, top, right, bottom, x, y, (const void*)s, (const void*)d);

	for (uint16 i = top; i < bottom; i++) {
		for (uint16 j = left; j < right; j++) {
			if (*s != 0) *d = *s;
			s++;
			d++;
		}

		s += (w - right + left);
		d += (SCREEN_WIDTH - right + left);
	}

	debugC(9, kDebugGraphics, "Gfx::flatBlit BLITTED");

	if (buffer == kBitFront) updateScreen();

	debugC(9, kDebugGraphics, "Gfx::flatBlit DONE");

	return;

}


void Gfx::blit(uint16 w, uint16 h, int16 x, int16 y, uint16 z, byte *data, Gfx::Buffers buffer, Gfx::Buffers mask) {
//	printf("Gfx::blit(%i, %i, %i, %i, %i)\n", w, h, x, y, z);

	int16 left = 0, top = 0;
	int16 right = w, bottom = h;

	if (x + w > SCREEN_WIDTH) right = SCREEN_WIDTH - x;
	if (y + h > SCREEN_HEIGHT) bottom = SCREEN_HEIGHT - y;

	if (x < 0) {	// partially left clipped
		left = -x;
		x = 0;
	}

	if (y < 0) {	// partially top clipped
		top = -y;
		y = 0;
	}

	if (left > right || top > bottom || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;	// fully clipped

	byte *s = data + left + top * w;
	byte *d = _buffers[buffer] + x + y * SCREEN_WIDTH;

	for (uint16 i = top; i < bottom; i++) {

		uint16 r = x % 4;
		byte *m = _buffers[mask] + x/4 + (y + i - top)*SCREENMASK_WIDTH;

		for (uint16 j = left; j < right; j++) {
			if (*s != 0) {
				uint16 v = ((3 << (r << 1)) & *m) >> (r << 1);
				if (z >= v) *d = *s;
			}

			r++;
			if (r==4) m++;
			r &= 0x3;

			s++;
			d++;
		}

		s += (w - right + left);
		d += (SCREEN_WIDTH - right + left);
	}

	if (buffer == kBitFront) updateScreen();

	return;

}


void jobDisplayLabel(void *parm, Job *j) {

	ZoneLabel *label = (ZoneLabel*)parm;
	debugC(1, kDebugJobs, "jobDisplayLabel (%p)", (const void*) label);

	if (label->_cnv._width == 0)
		return;
	_vm->_gfx->flatBlitCnv(&label->_cnv, Gfx::_labelPosition[0]._x, Gfx::_labelPosition[0]._y, Gfx::kBitBack, label->_cnv._data1);

	return;
}

void jobEraseLabel(void *parm, Job *j) {
	ZoneLabel *label = (ZoneLabel*)parm;

	debugC(1, kDebugJobs, "jobEraseLabel (%p)", (const void*) label);

	int16 _si, _di;

	if (_vm->_activeItem._id != 0) {
		_si = _mousePos._x + 16 - label->_cnv._width/2;
		_di = _mousePos._y + 34;
	} else {
		_si = _mousePos._x + 8 - label->_cnv._width/2;
		_di = _mousePos._y + 21;
	}

	if (_si < 0) _si = 0;
	if (_di > 190) _di = 190;

	if (label->_cnv._width + _si > SCREEN_WIDTH)
		_si = SCREEN_WIDTH - label->_cnv._width;

	Common::Rect r(label->_cnv._width, label->_cnv._height);
	r.moveTo(Gfx::_labelPosition[1]._x, Gfx::_labelPosition[1]._y);
	_vm->_gfx->restoreBackground(r);

	Gfx::_labelPosition[1]._x = Gfx::_labelPosition[0]._x;
	Gfx::_labelPosition[1]._y = Gfx::_labelPosition[0]._y;
	Gfx::_labelPosition[0]._x = _si;
	Gfx::_labelPosition[0]._y = _di;

	return;
}

void Gfx::initMouse(uint16 arg_0) {

	_vm->_disk->loadPointer(&_mouseComposedArrow);

	byte temp[16*16];
	memcpy(temp, _mouseArrow, 16*16);

	uint16 k = 0;
	for (uint16 i = 0; i < 4; i++) {
		for (uint16 j = 0; j < 64; j++) _mouseArrow[k++] = temp[i + j * 4];
	}

	return;
}

void Gfx::setMousePointer(int16 index) {

	if (index == kCursorArrow) {		// standard mouse pointer

		StaticCnv cnv;

		cnv._width = 16;
		cnv._height = 16;
		cnv._data0 = _mouseArrow;

		g_system->setMouseCursor(_mouseArrow, 16, 16, 0, 0, 0);
		g_system->showMouse(true);

	} else {

		// FIXME: standard mouse arrow must be combined with item
		//		  but it is not at the moment

		// inventory item pointer
		StaticCnv mouse_pointer;
		memcpy(&mouse_pointer, &_mouseComposedArrow, sizeof(StaticCnv));
		byte *v8 = mouse_pointer._data0;

	// FIXME: target offseting is not clear
		extractInventoryGraphics(index, v8 + 7 + 32 * 7);

		g_system->setMouseCursor(v8, 32, 32, 0, 0, 0);

	}

	return;
}




//
//	Cnv management
//
void Gfx::flatBlitCnv(StaticCnv *cnv, int16 x, int16 y, Gfx::Buffers buffer, byte *unused) {
	flatBlit(cnv->_width, cnv->_height, x, y, cnv->_data0, buffer);
	return;
}


void Gfx::blitCnv(StaticCnv *cnv, int16 x, int16 y, uint16 z, Gfx::Buffers buffer, Gfx::Buffers mask) {
	blit(cnv->_width, cnv->_height, x, y, z, cnv->_data0,  buffer, mask);
	return;
}

void Gfx::backupDoorBackground(DoorData *data, int16 x, int16 y) {

	byte *s = _buffers[kBit2] + x + y * SCREEN_WIDTH;
	byte *d = data->_background;

	for (uint16 i = 0; i < data->_cnv._height ; i++) {
		memcpy(d, s, data->_cnv._width);

		s += SCREEN_WIDTH;
		d += data->_cnv._width;
	}

	return;
}

void Gfx::backupGetBackground(GetData *data, int16 x, int16 y) {

	byte *t = data->_cnv._data0;
	byte *s = _buffers[kBitBack] + x + y * SCREEN_WIDTH;
	byte *d = data->_backup;

	for (uint16 i = 0; i < data->_cnv._height ; i++) {
		for (uint16 j = 0; j < data->_cnv._width ; j++) {
			*d = (*t) ? *s : 0;

			d++;
			t++;
			s++;
		}

		s += (SCREEN_WIDTH - data->_cnv._width);
	}

	return;
}

//
//	copies a rectangular bitmap on the background
//
void Gfx::restoreZoneBackground(const Common::Rect& r, byte *data) {

	StaticCnv cnv;

	cnv._data0 = data;
	cnv._data1 = NULL;
	cnv._width = r.width();
	cnv._height = r.height();

	flatBlitCnv(&cnv, r.left, r.top, kBitBack, cnv._data1);
	flatBlitCnv(&cnv, r.left, r.top, kBit2, cnv._data1);

	return;
}



//
//	strings
//
void Gfx::displayString(uint16 x, uint16 y, const char *text) {
	if (text == NULL)
		return;

	uint16 len = strlen(text);
	StaticCnv tmp;

	for (uint16 i = 0; i < len; i++) {
		byte c = mapChar(text[i]);

		tmp._width = _font._width;
		tmp._height = _font._height;
		tmp._data0 = _font._array[c];

		flatBlitCnv(&tmp, x, y, kBitFront, NULL);

		x += (_proportionalFont ? _glyphWidths[(int)c] : 8);

	}

	return;
}


void Gfx::displayBalloonString(uint16 x, uint16 y, const char *text, byte color) {

	uint16 len = strlen(text);

	for (uint16 i = 0; i < len; i++) {

		byte c = mapChar(text[i]);
		uint16 w = _proportionalFont ? _glyphWidths[(int)c] : 8;
		byte *s = _font._array[c];
		byte *d = _buffers[kBitFront] + x + y*SCREEN_WIDTH;

//		printf("%i\n", text[i]);

		for (uint16 j = 0; j < _font._height; j++) {
			for (uint16 k = 0; k < w; k++) {
				*d = (*s) ? 1 : color;
				d++;
				s++;
			}

			s += (8 - w);
			d += (SCREEN_WIDTH - w);
		}

		x += w;
	}

	updateScreen();

	return;
}



bool Gfx::displayWrappedString(char *text, uint16 x, uint16 y, uint16 maxwidth, byte color) {
//	printf("Gfx::displayWrappedString(%s, %i, %i, %i, %i)...", text, x, y, maxwidth, color);

	uint16 lines = 0;
	bool rv = false;
	uint16 linewidth = 0;

	_proportionalFont = true;
	uint16 rx = x + 10;
	uint16 ry = y + 4;

	char token[40];

	while (strlen(text) > 0) {

		text = parseNextToken(text, token, 40, "   ");
		linewidth += getStringWidth(token);

		if (linewidth > maxwidth) {
			// wrap line
			lines++;
			rx = x + 10;			// x
			ry = y + 4 + lines*10;	// y
			linewidth = getStringWidth(token);
		}

		if (!scumm_stricmp(token, "%s")) {
			sprintf(token, "%d", _score);
		}
		if (!scumm_stricmp(token, "%p")) {
			rv = true;
		} else
			displayBalloonString(rx, ry, token, color);

		rx += getStringWidth(token) + getStringWidth(" ");
		linewidth += getStringWidth(" ");

		text = Common::ltrim(text);
	}

//	printf("done\n");

	return rv;

}



uint16 Gfx::getStringWidth(const char *text) {
	if (text == NULL) return 0;

	uint16 len = strlen(text);

	if (_proportionalFont == 0) {
		// fixed font
		return len*8;
	}

	// proportional font
	uint16 w = 0;
	for (uint16 i = 0; i < len; i++) {
		byte c = mapChar(text[i]);
		w += _glyphWidths[(int)c];
	}

	return w;
}


void Gfx::getStringExtent(char *text, uint16 maxwidth, int16* width, int16* height) {

	uint16 lines = 0;
	uint16 w = 0;
	*width = 0;

	_proportionalFont = true;

	char token[40];

	while (strlen(text) != 0) {

		text = parseNextToken(text, token, 40, "   ");
		w += getStringWidth(token);

		if (w > maxwidth) {
			w -= getStringWidth(token);
			lines++;
			if (w > *width)
				*width = w;

			w = getStringWidth(token);
		}

		w += getStringWidth(" ");
		text = Common::ltrim(text);
	}

	if (*width < w) *width = w;
	*width += 10;

	*height = lines * 10 + 20;

	return;
}


void Gfx::setFont(const char* name) {
	freeCnv(&_font);
	_vm->_disk->loadFont(name, &_font);
}


void Gfx::restoreBackground(const Common::Rect& r) {
//	printf("restoreBackground(%i, %i, %i, %i)\n", left, top, width, height);

	int16 left = r.left;
	int16 top = r.top;
	int16 width = r.width();
	int16 height = r.height();

	if (left < 0) left = 0;
	if (top < 0) top = 0;

	if (left >= SCREEN_WIDTH) return;
	if (top >= SCREEN_HEIGHT) return;

	if (left+width >= SCREEN_WIDTH) width = SCREEN_WIDTH - left;
	if (top+height >= SCREEN_HEIGHT) height = SCREEN_HEIGHT - top;

	copyRect(kBit2, left, top, kBitBack, left, top, width, height);

	return;
}


void Gfx::makeCnvFromString(StaticCnv *cnv, char *text) {
//	printf("makeCnvFromString('%s')\n", text);

	uint16 len = strlen(text);

	cnv->_width = _font._width * len;
	cnv->_height = _font._height;

//	printf("%i x %i\n", cnv->_width, cnv->_height);

	cnv->_data0 = (byte*)malloc(cnv->_width * cnv->_height);

	for (uint16 i = 0; i < len; i++) {
		byte c = mapChar(text[i]);

		byte *s = _font._array[c];
		byte *d = cnv->_data0 + _font._width * i;

		for (uint16 j = 0; j < _font._height; j++) {
			memcpy(d, s, 8);

			s += 8;
			d += cnv->_width;
		}
	}

	return;
}

//
//	internal character mapping
//
byte Gfx::mapChar(byte c) {

	if (c == 0xA5) return 0x5F;
	if (c == 0xDF) return 0x60;

	if (c > 0x7F) return c - 0x7F;

	return c - 0x20;
}


void Gfx::freeCnv(Cnv *cnv) {
//	printf("Gfx::freeCnv()\n");

	if (!cnv) return;
	if (cnv->_count == 0) return;
	if (!cnv->_array) return;

	for (uint16 _si = 0; _si < cnv->_count; _si++) {
		if (cnv->_array[_si]) {
			free(cnv->_array[_si]);
		}
		cnv->_array[_si] = NULL;
	}

	if (cnv->_array)
		free(cnv->_array);

	cnv->_count = 0;
	cnv->_array = NULL;

	return;
}



void Gfx::freeStaticCnv(StaticCnv *cnv) {
//	printf("free_static_cnv()\n");

	if (!cnv) return;

	if (!cnv || !cnv->_data0) return;
	free(cnv->_data0);
	cnv->_data0 = NULL;

	return;
}

void Gfx::parseDepths(Common::SeekableReadStream &stream) {
	_bgLayers[0] = stream.readByte();
	_bgLayers[1] = stream.readByte();
	_bgLayers[2] = stream.readByte();
	_bgLayers[3] = stream.readByte();
}


void Gfx::parseBackground(Common::SeekableReadStream &stream) {

	stream.read(_palette, PALETTE_SIZE);

	parseDepths(stream);

	for (uint32 _si = 0; _si < 6; _si++) {
		_palettefx[_si]._timer = stream.readUint16BE();
		_palettefx[_si]._step = stream.readUint16BE();
		_palettefx[_si]._flags = stream.readUint16BE();
		_palettefx[_si]._first = stream.readByte();
		_palettefx[_si]._last = stream.readByte();
	}

#if 0
	uint16 v147;
	for (v147 = 0; v147 < PALETTE_SIZE; v147++) {
		byte _al = _palette[v147];
		_palette[PALETTE_SIZE+v147] = _al / 2;
	}
#endif

}

void Gfx::setBackground(byte *background) {
	memcpy(_buffers[kBitBack], background, SCREEN_WIDTH*SCREEN_HEIGHT);
	copyScreen(kBitBack, kBit2);
}

void Gfx::setMask(byte *mask) {
	memcpy(_buffers[kMask0], mask, SCREENMASK_WIDTH*SCREEN_HEIGHT);
}



void Gfx::copyRect(Gfx::Buffers dstbuffer, uint16 x, uint16 y, uint16 w, uint16 h, byte *src, uint16 pitch) {

	byte *d = _buffers[dstbuffer] + x + SCREEN_WIDTH * y;
	byte *s = src;

	for (uint16 _si = 0; _si < h; _si++) {
		memcpy(d, s, w);

		s += pitch;
		d += SCREEN_WIDTH;
	}


}


void Gfx::drawBorder(Gfx::Buffers buffer, uint16 x, uint16 y, uint16 w, uint16 h, byte color) {

	byte *d = _buffers[buffer] + x + SCREEN_WIDTH * y;

	memset(d, color, w);

	for (uint16 i = 0; i < h; i++) {
		d[i * SCREEN_WIDTH] = color;
		d[i * SCREEN_WIDTH + w - 1] = color;
	}

	d = _buffers[buffer] + x + SCREEN_WIDTH * (y + h - 1);
	memset(d, color, w);

	return;
}

void Gfx::grabRect(Gfx::Buffers srcbuffer, byte *dst, uint16 x, uint16 y, uint16 w, uint16 h, uint16 pitch) {

	byte *s = _buffers[srcbuffer] + x + SCREEN_WIDTH * y;

	for (uint16 i = 0; i < h; i++) {
		memcpy(dst, s, w);

		s += SCREEN_WIDTH;
		dst += pitch;
	}

	return;
}


void Gfx::maskOpNot(uint16 x, uint16 y, uint16 unused, Gfx::Buffers mask) {

	uint16 _ax = x + y * SCREEN_WIDTH;
	_buffers[mask][_ax >> 2] &= ~(3 << ((_ax & 3) << 1));

	return;
}



void Gfx::maskClearRectangle(const Common::Rect& r, Gfx::Buffers mask) {

	uint16 _di = r.left/4 + r.top*80;

	for (uint16 _si = r.top; _si < r.bottom; _si++) {
		memset(&_buffers[mask][_di], 0, r.width()/4+1);
		_di += 80;
	}

	return;

}

//	HACK
//	this routine is only invoked from the 'intgrotta scenario'
//
void Gfx::intGrottaHackMask() {
	memset(_buffers[kMask0] + 3600, 0, 3600);
	_bgLayers[1] = 500;
	return;
}

int16 Gfx::queryMask(int16 v) {

	for (uint16 _si = 0; _si < 3; _si++) {
		if (_bgLayers[_si+1] > v) return _si;
	}

	return 3;
}

void Gfx::initBuffers() {

	_buffers[kBitFront] = (byte*)malloc(SCREEN_SIZE);
	_buffers[kBitBack]	= (byte*)malloc(SCREEN_SIZE);
	_buffers[kBit2]   = (byte*)malloc(SCREEN_SIZE);
	_buffers[kBit3]   = (byte*)malloc(SCREEN_SIZE);	  // this buffer is also used by menu so it must stay this size

	_buffers[kMask0] = (byte*)malloc(SCREENMASK_WIDTH * SCREEN_HEIGHT);

	return;
}


Gfx::Gfx(Parallaction* vm) :
	_vm(vm) {

	g_system->beginGFXTransaction();

	g_system->initSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	g_system->endGFXTransaction();

	initBuffers();

	byte palette[PALETTE_SIZE];
	getBlackPalette(palette);
	setPalette(palette);

	initMouse( 0 );

	_font._count = 0;
	_font._array = NULL;

	return;
}

Gfx::~Gfx() {

	free(_buffers[kMask0]);

	free(_buffers[kBitFront]);
	free(_buffers[kBitBack]);
	free(_buffers[kBit2]);
	free(_buffers[kBit3]);

	freeCnv(&_font);

	return;
}


} // namespace Parallaction
