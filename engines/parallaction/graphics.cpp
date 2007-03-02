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


byte *	_maskBackup;
byte *	_pathBackup;



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

Cnv 		Graphics::_font;
bool		Graphics::_proportionalFont = false;
Point		Graphics::_labelPosition[2] = { { 0, 0 }, { 0, 0 } };
StaticCnv	Graphics::_mouseComposedArrow;
byte *		Graphics::_buffers[];

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

void Graphics::drawBalloon(int16 left, int16 top, uint16 width, uint16 height, uint16 winding) {
//	printf("Graphics::drawBalloon(%i, %i, %i, %i, %i)...", left, top, width, height, winding);

	width+=5;
	floodFill(0, left, top, left+width, top+height, kBitFront);
	floodFill(1, left+1, top+2, left+width-1, top+height-1, kBitFront);

	winding = (winding == 0 ? 1 : 0);
	byte *s = _resBalloon[winding];
	byte *d = _buffers[kBitFront] + (left + width/2 - 5) + (top + height - 1) * SCREEN_WIDTH;

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

void Graphics::setPalette(byte *palette) {
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

void Graphics::getBlackPalette(byte *palette) {
	memcpy(palette, _black_palette, PALETTE_SIZE);
	return;
}

void Graphics::palUnk0(byte *palette) {
#if 0
	for (uint16 i = 0; i < PALETTE_SIZE; i++) {
		palette[PALETTE_BACKUP+i] = _palette[i]/2;
	}
#endif
	Graphics::setPalette(palette);

	return;
}

void Graphics::buildBWPalette(byte *palette) {

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

void Graphics::fadePalette(byte *palette) {

	for (uint16 i = 0; i < PALETTE_SIZE; i++)
		if (palette[i] < _palette[i]) palette[i]++;


	return;
}

void Graphics::quickFadePalette(byte *palette) {

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
void Graphics::animatePalette(byte *palette) {
// printf("Graphics::animatePalette()\n");

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




void Graphics::updateScreen() {
//	  printf("Graphics::updateScreen()\n");
	g_system->copyRectToScreen(_buffers[kBitFront], SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_system->updateScreen();
	return;
}


void Graphics::swapBuffers() {
	byte *temp = _buffers[kBitFront];
	_buffers[kBitFront] = _buffers[kBitBack];
	_buffers[kBitBack] = temp;
	updateScreen();
	return;
}


//
//	graphic primitives
//
void Graphics::clearScreen(Graphics::Buffers buffer) {
	memset(_buffers[buffer], 0, SCREEN_WIDTH*SCREEN_HEIGHT);

	if (buffer == kBitFront) updateScreen();

	return;
}


void Graphics::copyScreen(Graphics::Buffers srcbuffer, Graphics::Buffers dstbuffer) {
	memcpy(_buffers[dstbuffer], _buffers[srcbuffer], SCREEN_WIDTH*SCREEN_HEIGHT);

	if (dstbuffer == kBitFront) updateScreen();

	return;
}


void Graphics::copyRect(Graphics::Buffers srcbuffer, uint16 sx, uint16 sy, Graphics::Buffers dstbuffer, uint16 dx, uint16 dy, uint16 w, uint16 h) {

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


void Graphics::floodFill(byte color, uint16 left, uint16 top, uint16 right, uint16 bottom, Graphics::Buffers buffer) {
//	printf("Graphics::floodFill(%i, %i, %i, %i, %i)\n", color, left, top, right, bottom);

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


void Graphics::flatBlit(uint16 w, uint16 h, int16 x, int16 y, byte *data, Graphics::Buffers buffer) {
	debugC(9, kDebugGraphics, "Graphics::flatBlit(%i, %i, %i, %i)", w, h, x, y);

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

	debugC(9, kDebugGraphics, "Graphics::flatBlit CLIPPED (%i, %i, %i, %i) -> (%i, %i) %p %p", left, top, right, bottom, x, y, (const void*)s, (const void*)d);

	for (uint16 i = top; i < bottom; i++) {
		for (uint16 j = left; j < right; j++) {
			if (*s != 0) *d = *s;
			s++;
			d++;
		}

		s += (w - right + left);
		d += (SCREEN_WIDTH - right + left);
	}

	debugC(9, kDebugGraphics, "Graphics::flatBlit BLITTED");

	if (buffer == kBitFront) updateScreen();

	debugC(9, kDebugGraphics, "Graphics::flatBlit DONE");

	return;

}


void Graphics::blit(uint16 w, uint16 h, int16 x, int16 y, uint16 z, byte *data, Graphics::Buffers buffer, Graphics::Buffers mask) {
//	printf("Graphics::blit(%i, %i, %i, %i, %i)\n", w, h, x, y, z);

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
	_vm->_graphics->flatBlitCnv(&label->_cnv, Graphics::_labelPosition[0]._x, Graphics::_labelPosition[0]._y, Graphics::kBitBack, label->_cnv._data1);

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


	_vm->_graphics->restoreBackground(Graphics::_labelPosition[1]._x, Graphics::_labelPosition[1]._y, label->_cnv._width, label->_cnv._height);

	Graphics::_labelPosition[1]._x = Graphics::_labelPosition[0]._x;
	Graphics::_labelPosition[1]._y = Graphics::_labelPosition[0]._y;
	Graphics::_labelPosition[0]._x = _si;
	Graphics::_labelPosition[0]._y = _di;

	return;
}

void Graphics::initMouse(uint16 arg_0) {

	loadPointer(&_mouseComposedArrow);

	byte temp[16*16];
	memcpy(temp, _mouseArrow, 16*16);

	uint16 k = 0;
	for (uint16 i = 0; i < 4; i++) {
		for (uint16 j = 0; j < 64; j++) _mouseArrow[k++] = temp[i + j * 4];
	}

	return;
}

void Graphics::setMousePointer(int16 index) {

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
void Graphics::flatBlitCnv(StaticCnv *cnv, int16 x, int16 y, Graphics::Buffers buffer, byte *unused) {
	flatBlit(cnv->_width, cnv->_height, x, y, cnv->_data0, buffer);
	return;
}


void Graphics::blitCnv(StaticCnv *cnv, int16 x, int16 y, uint16 z, Graphics::Buffers buffer, Graphics::Buffers mask) {
	blit(cnv->_width, cnv->_height, x, y, z, cnv->_data0,  buffer, mask);
	return;
}


void Graphics::backupCnvBackground(StaticCnv *cnv, int16 x, int16 y) {

	byte *s = _buffers[kBit2] + x + y * SCREEN_WIDTH;
	byte *d = cnv->_data2;

	for (uint16 i = 0; i < cnv->_height ; i++) {
		memcpy(d, s, cnv->_width);

		s += SCREEN_WIDTH;
		d += cnv->_width;
	}

	return;
}


void Graphics::backupCnvBackgroundTransparent(StaticCnv *cnv, int16 x, int16 y) {

	byte *t = cnv->_data0;
	byte *s = _buffers[kBitBack] + x + y * SCREEN_WIDTH;
	byte *d = cnv->_data2;

	for (uint16 i = 0; i < cnv->_height ; i++) {
		for (uint16 j = 0; j < cnv->_width ; j++) {
			*d = (*t) ? *s : 0;

			d++;
			t++;
			s++;
		}

		s += (SCREEN_WIDTH - cnv->_width);
	}

	return;
}


//	restores a cnv backup on the background
//
//
void Graphics::restoreCnvBackground(StaticCnv *cnv, int16 x, int16 y) {

	byte *temp = cnv->_data0;
	cnv->_data0 = cnv->_data2;

	flatBlitCnv(cnv, x, y, kBitBack, cnv->_data1);
	flatBlitCnv(cnv, x, y, kBit2, cnv->_data1);

	cnv->_data0 = temp;

	return;
}



//
//	strings
//
void Graphics::displayString(uint16 x, uint16 y, const char *text) {
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


void Graphics::displayBalloonString(uint16 x, uint16 y, const char *text, byte color) {

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



bool Graphics::displayWrappedString(char *text, uint16 x, uint16 y, uint16 maxwidth, byte color) {
//	printf("Graphics::displayWrappedString(%s, %i, %i, %i, %i)...", text, x, y, maxwidth, color);

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



uint16 Graphics::getStringWidth(const char *text) {
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


void Graphics::getStringExtent(char *text, uint16 maxwidth, int16* width, int16* height) {

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


void Graphics::setFont(const char* name) {
	if (_font._array != NULL)
		freeCnv(&_font);

	loadExternalCnv(name, &_font);
}


//	backups background mask
//
//
void Graphics::backupBackgroundMask(Graphics::Buffers mask) {

	byte *s = _buffers[mask];
	byte *d = _maskBackup;

	memcpy(d, s, SCREENMASK_WIDTH*SCREEN_HEIGHT);

	return;
}

//	restores background mask
//
//
void Graphics::restoreBackgroundMask(Graphics::Buffers mask) {

	byte *s = _maskBackup;
	byte *d = _buffers[mask];

	memcpy(d, s, SCREENMASK_WIDTH*SCREEN_HEIGHT);

	return;
}

//	backups background path
//
//
void Graphics::backupBackgroundPath(Graphics::Buffers path) {

	byte *s = _buffers[path];
	byte *d = _pathBackup;

	memcpy(d, s, SCREENPATH_WIDTH*SCREEN_HEIGHT);

	return;
}

//
//	restores background path
//
void Graphics::restoreBackgroundPath(Graphics::Buffers path) {

	byte *s = _pathBackup;
	byte *d = _buffers[path];

	memcpy(d, s, SCREENPATH_WIDTH*SCREEN_HEIGHT);

	return;
}

//
// decompress a graphics block
//
uint16 Graphics::decompressChunk(byte *src, byte *dst, uint16 size) {

	uint16 written = 0;
	uint16 read = 0;
	uint16 len = 0;

	for (; written != size; written += len) {

		len = src[read];
		read++;

		if (len <= 127) {
			// copy run

			len++;
			memcpy(dst+written, src+read, len);
			read += len;

		} else {
			// expand run

			len = 257 - len;
			memset(dst+written, src[read], len);
			read++;

		}

	}

	return read;
}

void Graphics::restoreBackground(int16 left, int16 top, uint16 width, uint16 height) {
//	printf("restoreBackground(%i, %i, %i, %i)\n", left, top, width, height);

	if (left < 0) left = 0;
	if (top < 0) top = 0;

	if (left >= SCREEN_WIDTH) return;
	if (top >= SCREEN_HEIGHT) return;

	if (left+width >= SCREEN_WIDTH) width = SCREEN_WIDTH - left;
	if (top+height >= SCREEN_HEIGHT) height = SCREEN_HEIGHT - top;

	copyRect(kBit2, left, top, kBitBack, left, top, width, height);

	return;
}


void Graphics::makeCnvFromString(StaticCnv *cnv, char *text) {
//	printf("makeCnvFromString('%s')\n", text);

	uint16 len = strlen(text);

	cnv->_width = _font._width * len;
	cnv->_height = _font._height;

//	printf("%i x %i\n", cnv->_width, cnv->_height);

	cnv->_data0 = (byte*)memAlloc(cnv->_width * cnv->_height);

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
byte Graphics::mapChar(byte c) {

	if (c == 0xA5) return 0x5F;
	if (c == 0xDF) return 0x60;

	if (c > 0x7F) return c - 0x7F;

	return c - 0x20;
}


//
// loads a cnv from an external file
//
void Graphics::loadExternalCnv(const char *filename, Cnv *cnv) {
//	printf("Graphics::loadExternalCnv(%s)...", filename);

	char path[PATH_LEN];

	sprintf(path, "%s.cnv", filename);

	Common::File stream;

	if (!stream.open(path))
		errorFileNotFound(path);

	cnv->_count = stream.readByte();
	cnv->_width = stream.readByte();
	cnv->_height = stream.readByte();

	cnv->_array = (byte**)memAlloc(cnv->_count * sizeof(byte*));

	uint16 size = cnv->_width*cnv->_height;
	for (uint16 i = 0; i < cnv->_count; i++) {
		cnv->_array[i] = (byte*)memAlloc(size);
		stream.read(cnv->_array[i], size);
	}

	stream.close();

//	printf("done\n");


	return;
}




void Graphics::loadExternalStaticCnv(const char *filename, StaticCnv *cnv) {

	char path[PATH_LEN];

	sprintf(path, "%s.cnv", filename);

	Common::File stream;

	if (!stream.open(path))
		errorFileNotFound(path);

	cnv->_width = cnv->_height = 0;

	stream.skip(1);
	cnv->_width = stream.readByte();
	cnv->_height = stream.readByte();

	uint16 size = cnv->_width*cnv->_height;

	cnv->_data0 = (byte*)memAlloc(size);
	stream.read(cnv->_data0, size);

	stream.close();

	return;
}





void Graphics::loadStaticCnv(const char *filename, StaticCnv *cnv) {
//	printf("Graphics::loadStaticCnv(%s)\n", filename);

	char path[PATH_LEN];

	strcpy(path, filename);
	if (!_vm->_archive.openArchivedFile(path)) {
		sprintf(path, "%s.pp", filename);
		if (!_vm->_archive.openArchivedFile(path))
			errorFileNotFound(path);
	}

	_vm->_archive.skip(1);
	cnv->_width = _vm->_archive.readByte();
	cnv->_height = _vm->_archive.readByte();

	uint16 compressedsize = _vm->_archive.size() - 3;
	byte *compressed = (byte*)memAlloc(compressedsize);

	uint16 size = cnv->_width*cnv->_height;
	cnv->_data0 = (byte*)memAlloc(size);

	_vm->_archive.read(compressed, compressedsize);
	_vm->_archive.closeArchivedFile();

	decompressChunk(compressed, cnv->_data0, size);
	memFree(compressed);

	return;
}




void Graphics::loadCnv(const char *filename, Cnv *cnv) {
//	printf("Graphics::loadCnv(%s)\n", filename);

	char path[PATH_LEN];

	strcpy(path, filename);
	if (!_vm->_archive.openArchivedFile(path)) {
		sprintf(path, "%s.pp", filename);
		if (!_vm->_archive.openArchivedFile(path))
			errorFileNotFound(path);
	}

	cnv->_count = _vm->_archive.readByte();
	cnv->_width = _vm->_archive.readByte();
	cnv->_height = _vm->_archive.readByte();

	uint16 framesize = cnv->_width*cnv->_height;

	cnv->_array = (byte**)memAlloc(cnv->_count * sizeof(byte*));

	uint32 size = _vm->_archive.size() - 3;

	byte *buf = (byte*)memAlloc(size);
	_vm->_archive.read(buf, size);

	byte *s = buf;

	for (uint16 i = 0; i < cnv->_count; i++) {
		cnv->_array[i] = (byte*)memAlloc(framesize);
		uint16 read = decompressChunk(s, cnv->_array[i], framesize);

//		printf("frame %i decompressed: %i --> %i\n", i, read, framesize);

		s += read;
	}

	_vm->_archive.closeArchivedFile();

	memFree(buf);

	return;
}







void Graphics::freeCnv(Cnv *cnv) {
//	printf("Graphics::freeCnv()\n");

	if (!cnv) return;

	for (uint16 _si = 0; _si < cnv->_count; _si++) {
		memFree(cnv->_array[_si]);
	}
	memFree(cnv->_array);
	cnv->_array = NULL;

	return;
}



void Graphics::freeStaticCnv(StaticCnv *cnv) {
//	printf("free_static_cnv()\n");

	if (!cnv) return;

	if (!cnv || !cnv->_data0) return;
	memFree(cnv->_data0);
	cnv->_data0 = NULL;

	return;
}


//
//	slides (background images) are stored compressed by scanline in a rle fashion
//
//	the uncompressed data must then be unpacked to get:
//	* color data [bits 0-5]
//	* mask data [bits 6-7] (z buffer)
//	* path data [bit 8] (walkable areas)
//


void unpackBackgroundScanline(byte *src, byte *screen, byte *mask, byte *path) {

	// update mask, path and screen
	for (uint16 i = 0; i < SCREEN_WIDTH; i++) {
		path[i/8] |= ((src[i] & 0x80) >> 7) << (i & 7);
		mask[i/4] |= ((src[i] & 0x60) >> 5) << ((i & 3) << 1);
		screen[i] = src[i] & 0x1F;
	}

	return;
}




void Graphics::loadBackground(const char *filename, Graphics::Buffers buffer) {
//	printf("Graphics::loadBackground(%s)\n", filename);

	if (!_vm->_archive.openArchivedFile(filename))
		errorFileNotFound(filename);

//	byte palette[PALETTE_SIZE];
	byte v150[4];
	_vm->_archive.read(_palette, PALETTE_SIZE);
	_vm->_archive.read(&v150, 4);

	byte tempfx[sizeof(PaletteFxRange)*6];
	_vm->_archive.read(&tempfx, sizeof(PaletteFxRange)*6);

//	setPalette(palette);

	uint16 _si;
	for (_si = 0; _si < 4; _si++) {
		byte _al = v150[_si];
		_bgLayers[_si] = _al;
	}

	Common::MemoryReadStream sread(tempfx, sizeof(PaletteFxRange)*6);
	for (_si = 0; _si < 6; _si++) {
		_palettefx[_si]._timer = sread.readUint16BE();
		_palettefx[_si]._step = sread.readUint16BE();
		_palettefx[_si]._flags = sread.readUint16BE();
		_palettefx[_si]._first = sread.readByte();
		_palettefx[_si]._last = sread.readByte();
	}

#if 0
	uint16 v147;
	for (v147 = 0; v147 < PALETTE_SIZE; v147++) {
		byte _al = _palette[v147];
		_palette[PALETTE_SIZE+v147] = _al / 2;
	}
#endif

	memset(_buffers[kPath0], 0, SCREENPATH_WIDTH*SCREEN_HEIGHT);
	memset(_buffers[kMask0], 0, SCREENMASK_WIDTH*SCREEN_HEIGHT);

	byte *v4 = (byte*)memAlloc(SCREEN_SIZE);
	_vm->_archive.read(v4, SCREEN_SIZE);

	byte v144[SCREEN_WIDTH];

	byte *s = v4;
	for (uint16 i = 0; i < SCREEN_HEIGHT; i++) {
		s += decompressChunk(s, v144, SCREEN_WIDTH);
		unpackBackgroundScanline(v144, _buffers[buffer]+SCREEN_WIDTH*i, _buffers[kMask0]+SCREENMASK_WIDTH*i, _buffers[kPath0]+SCREENPATH_WIDTH*i);
	}

	memFree(v4);
	_vm->_archive.closeArchivedFile();

	return;
}

//
//	read background path and mask from a file
//
//	mask and path are normally combined (via OR) into the background picture itself
//	read the comment on the top of this file for more
//
void Graphics::loadMaskAndPath(const char *filename) {

	if (!_vm->_archive.openArchivedFile(filename))
		errorFileNotFound(filename);

	byte v4[4];
	_vm->_archive.read(v4, 4);
	_vm->_archive.read(_buffers[kPath0], SCREENPATH_WIDTH*SCREEN_HEIGHT);
	_vm->_archive.read(_buffers[kMask0], SCREENMASK_WIDTH*SCREEN_HEIGHT);

	for (uint16 _si = 0; _si < 4; _si++) _bgLayers[_si] = v4[_si];

	_vm->_archive.closeArchivedFile();
	return;
}


void Graphics::copyRect(Graphics::Buffers dstbuffer, uint16 x, uint16 y, uint16 w, uint16 h, byte *src, uint16 pitch) {

	byte *d = _buffers[dstbuffer] + x + SCREEN_WIDTH * y;
	byte *s = src;

	for (uint16 _si = 0; _si < h; _si++) {
		memcpy(d, s, w);

		s += pitch;
		d += SCREEN_WIDTH;
	}


}


void Graphics::drawBorder(Graphics::Buffers buffer, uint16 x, uint16 y, uint16 w, uint16 h, byte color) {

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

void Graphics::grabRect(Graphics::Buffers srcbuffer, byte *dst, uint16 x, uint16 y, uint16 w, uint16 h, uint16 pitch) {

	byte *s = _buffers[srcbuffer] + x + SCREEN_WIDTH * y;

	for (uint16 i = 0; i < h; i++) {
		memcpy(dst, s, w);

		s += SCREEN_WIDTH;
		dst += pitch;
	}

	return;
}


void Graphics::maskOpNot(uint16 x, uint16 y, uint16 unused, Graphics::Buffers mask) {

	uint16 _ax = x + y * SCREEN_WIDTH;
	_buffers[mask][_ax >> 2] &= ~(3 << ((_ax & 3) << 1));

	return;
}



void Graphics::maskClearRectangle(uint16 left, uint16 top, uint16 right, uint16 bottom, Graphics::Buffers mask) {

	uint16 _di = left/4 + top*80;

	for (uint16 _si = top; _si < bottom; _si++) {
		memset(&_buffers[mask][_di], 0, (right - left)/4+1);
		_di += 80;
	}

	return;

}

//	HACK
//	this routine is only invoked from the 'intgrotta scenario'
//
void Graphics::intGrottaHackMask() {
	memset(_buffers[kMask0] + 3600, 0, 3600);
	_bgLayers[1] = 500;
	return;
}

uint16 Graphics::queryPath(uint16 x, uint16 y) {

	byte *v6 = _buffers[kPath0];

	byte _al = v6[y*40 + x/8];
	byte _dl = 1 << (x % 8);

	return _al & _dl;

}

int16 Graphics::queryMask(int16 v) {

	for (uint16 _si = 0; _si < 3; _si++) {
		if (_bgLayers[_si+1] > v) return _si;
	}

	return 3;
}

void Graphics::initBuffers() {

	_buffers[kBitFront] = (byte*)memAlloc(SCREEN_SIZE);
	_buffers[kBitBack]	= (byte*)memAlloc(SCREEN_SIZE);
	_buffers[kBit2]   = (byte*)memAlloc(SCREEN_SIZE);
	_buffers[kBit3]   = (byte*)memAlloc(SCREEN_SIZE);	  // this buffer is also used by menu so it must stay this size

	_buffers[kMask0] = (byte*)memAlloc(SCREENMASK_WIDTH * SCREEN_HEIGHT);
	_buffers[kPath0] = (byte*)memAlloc(SCREENPATH_WIDTH * SCREEN_HEIGHT);

	_maskBackup = (byte*)memAlloc(SCREENMASK_WIDTH * SCREEN_HEIGHT);
	_pathBackup = (byte*)memAlloc(SCREENPATH_WIDTH * SCREEN_HEIGHT);

	return;
}


Graphics::Graphics(Parallaction* vm) :
	_vm(vm) {

	g_system->beginGFXTransaction();

	g_system->initSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	g_system->endGFXTransaction();

	initBuffers();

	byte palette[PALETTE_SIZE];
	getBlackPalette(palette);
	setPalette(palette);

	initMouse( 0 );

	_font._array = NULL;

	return;
}

Graphics::~Graphics() {

	memFree(_buffers[kMask0]);
	memFree(_buffers[kPath0]);

	memFree(_buffers[kBitFront]);
	memFree(_buffers[kBitBack]);
	memFree(_buffers[kBit2]);
	memFree(_buffers[kBit3]);

	freeCnv(&_font);

	return;
}


} // namespace Parallaction
