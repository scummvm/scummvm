/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "common/stdafx.h"
#include "common/system.h"
#include "common/file.h"

#include "parallaction/parallaction.h"



extern OSystem *g_system;

namespace Parallaction {

byte *		Gfx::_buffers[];

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

	Common::Rect q = r;

	q.right += 5;
	floodFill(kBitFront, q, 0);

	q.left++;
	q.top+=2;
	q.right--;
	q.bottom--;
	floodFill(kBitFront, q, 1);

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


void Gfx::setPalette(Palette pal, uint32 first, uint32 num) {
//	printf("setPalette(%i, %i)\n", first, num);

	if (first + num > BASE_PALETTE_COLORS)
		error("wrong parameters for setPalette()");

	byte sysBasePal[EHB_PALETTE_COLORS*4];
	byte sysExtraPal[BASE_PALETTE_COLORS*4];

	byte r, g, b;
	uint32 j = 0;
	for (uint32 i = first; i < first+num; i++) {
		r = (pal[i*3] << 2) | (pal[i*3] >> 4);
		g = (pal[i*3+1] << 2) | (pal[i*3+1] >> 4);
		b = (pal[i*3+2] << 2) | (pal[i*3+2] >> 4);

		sysBasePal[j*4]   = r;
		sysBasePal[j*4+1] = g;
		sysBasePal[j*4+2] = b;
		sysBasePal[j*4+3] = 0;

		if (_vm->getPlatform() == Common::kPlatformAmiga) {
			sysExtraPal[j*4]   = r >> 1;
			sysExtraPal[j*4+1] = g >> 1;
			sysExtraPal[j*4+2] = b >> 1;
			sysExtraPal[j*4+3] = 0;
		} else {
			sysExtraPal[j*4]   = 0;
			sysExtraPal[j*4+1] = 0;
			sysExtraPal[j*4+2] = 0;
			sysExtraPal[j*4+3] = 0;
		}

		j++;
	}

	g_system->setPalette(sysBasePal, first, num);

	if (_vm->getPlatform() == Common::kPlatformAmiga)
		g_system->setPalette(sysExtraPal, first+FIRST_EHB_COLOR, num);

	g_system->updateScreen();

	return;
}

void Gfx::setBlackPalette() {
	Palette pal;
	memset(pal, 0, PALETTE_SIZE);
	setPalette(pal);
}

//
//	palette Animation
//
//	FIXME: the effect is different from the original
//
void Gfx::animatePalette() {
//	printf("Gfx::animatePalette()\n");

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

			memmove(_palette+_palettefx[i]._first*3, _palette+(_palettefx[i]._first+1)*3, (_palettefx[i]._last - _palettefx[i]._first)*3);

			_palette[_palettefx[i]._last * 3]	 = tmp[0];
			_palette[_palettefx[i]._last * 3 + 1] = tmp[1];
			_palette[_palettefx[i]._last * 3 + 2] = tmp[2];

		} else {											// backward

			tmp[0] = _palette[_palettefx[i]._last * 3];
			tmp[1] = _palette[_palettefx[i]._last * 3 + 1];
			tmp[2] = _palette[_palettefx[i]._last * 3 + 2];

			memmove(_palette+(_palettefx[i]._first+1)*3, _palette+_palettefx[i]._first*3, (_palettefx[i]._last - _palettefx[i]._first)*3);

			_palette[_palettefx[i]._first * 3]	  = tmp[0];
			_palette[_palettefx[i]._first * 3 + 1] = tmp[1];
			_palette[_palettefx[i]._first * 3 + 2] = tmp[2];

		}

	}

	setPalette(_palette);

	return;
}

void Gfx::fadePalette(Palette pal) {
	for (uint16 i = 0; i < BASE_PALETTE_COLORS * 3; i++)
		if (pal[i] < _palette[i]) pal[i]++;

	return;
}

void Gfx::buildBWPalette(Palette pal) {

	for (uint16 i = 0; i < BASE_PALETTE_COLORS; i++) {
		byte max;

		max = MAX(_palette[i*3+1], _palette[i*3+2]);
		max = MAX(max, _palette[i*3]);

		pal[i*3] = max;
		pal[i*3+1] = max;
		pal[i*3+2] = max;
	}

	return;
}

void Gfx::quickFadePalette(Palette pal) {

	for (uint16 i = 0; i < BASE_PALETTE_COLORS * 3; i++) {
		if (pal[i] == _palette[i]) continue;
		pal[i] += (pal[i] < _palette[i] ? 4 : -4);
	}

	return;
}

void Gfx::setHalfbriteMode(bool enable) {
#ifdef HALFBRITE
	if (_vm->getPlatform() != Common::kPlatformAmiga) return;
	if (enable == _halfbrite) return;

	byte *buf = _buffers[kBitBack];
	for (uint32 i = 0; i < SCREEN_SIZE; i++)
		*buf++ ^= 0x20;

	buf = _buffers[kBitFront];
	for (uint32 i = 0; i < SCREEN_SIZE; i++)
		*buf++ ^= 0x20;

	_halfbrite = !_halfbrite;
#endif
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

void Gfx::floodFill(Gfx::Buffers buffer, const Common::Rect& r, byte color) {
//	printf("Gfx::floodFill(%i, %i, %i, %i, %i)\n", color, left, top, right, bottom);

	byte *d = _buffers[buffer] + (r.left + r.top * SCREEN_WIDTH);
	uint16 w = r.width() + 1;
	uint16 h = r.height() + 1;

	for (uint16 i = 0; i < h; i++) {
		memset(d, color, w);

		d += SCREEN_WIDTH;
	}

	return;
}

void screenClip(Common::Rect& r, Common::Point& p) {

	int32 x = r.left;
	int32 y = r.top;

	Common::Rect screen(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	r.clip(screen);

	if (!r.isValidRect()) return;

	p.x = r.left;
	p.y = r.top;

	r.translate(screen.left - x, screen.top - y);

}

void Gfx::flatBlit(const Common::Rect& r, byte *data, Gfx::Buffers buffer) {

	Common::Point dp;
	Common::Rect q(r);

	screenClip(q, dp);

	byte *s = data + q.left + q.top * r.width();
	byte *d = _buffers[buffer] + dp.x + dp.y * SCREEN_WIDTH;

	for (uint16 i = q.top; i < q.bottom; i++) {
		for (uint16 j = q.left; j < q.right; j++) {
			if (*s != 0) *d = *s;
			s++;
			d++;
		}

		s += (r.width() - q.width());
		d += (SCREEN_WIDTH - q.width());
	}

	return;

}

void Gfx::blit(const Common::Rect& r, uint16 z, byte *data, Gfx::Buffers buffer) {

	Common::Point dp;
	Common::Rect q(r);

	screenClip(q, dp);

	byte *s = data + q.left + q.top * r.width();
	byte *d = _buffers[buffer] + dp.x + dp.y * SCREEN_WIDTH;

	for (uint16 i = q.top; i < q.bottom; i++) {

		uint16 n = dp.x % 4;
		byte *m = _buffers[kMask0] + dp.x/4 + (dp.y + i - q.top)*SCREENMASK_WIDTH;

		for (uint16 j = q.left; j < q.right; j++) {
			if (*s != 0) {
				uint16 v = ((3 << (n << 1)) & *m) >> (n << 1);
				if (z >= v) *d = *s;
			}

			n++;
			if (n==4) m++;
			n &= 0x3;

			s++;
			d++;
		}

		s += (r.width() - q.right + q.left);
		d += (SCREEN_WIDTH - q.right + q.left);
	}

	return;

}


void jobDisplayLabel(void *parm, Job *j) {

	Label *label = (Label*)parm;
	debugC(9, kDebugJobs, "jobDisplayLabel (%p)", (const void*) label);

	if (label->_cnv._width == 0)
		return;
	_vm->_gfx->flatBlitCnv(&label->_cnv, _vm->_gfx->_labelPosition[0].x, _vm->_gfx->_labelPosition[0].y, Gfx::kBitBack);

	return;
}

void jobEraseLabel(void *parm, Job *j) {
	Label *label = (Label*)parm;

	debugC(9, kDebugJobs, "jobEraseLabel (%p)", (const void*) label);

	int16 _si, _di;

	if (_vm->_activeItem._id != 0) {
		_si = _vm->_mousePos.x + 16 - label->_cnv._width/2;
		_di = _vm->_mousePos.y + 34;
	} else {
		_si = _vm->_mousePos.x + 8 - label->_cnv._width/2;
		_di = _vm->_mousePos.y + 21;
	}

	if (_si < 0) _si = 0;
	if (_di > 190) _di = 190;

	if (label->_cnv._width + _si > SCREEN_WIDTH)
		_si = SCREEN_WIDTH - label->_cnv._width;

	Common::Rect r(label->_cnv._width, label->_cnv._height);
	r.moveTo(_vm->_gfx->_labelPosition[1]);
	_vm->_gfx->restoreBackground(r);

	_vm->_gfx->_labelPosition[1] = _vm->_gfx->_labelPosition[0];
	_vm->_gfx->_labelPosition[0].x = _si;
	_vm->_gfx->_labelPosition[0].y = _di;

	return;
}

void Gfx::initMouse(uint16 arg_0) {

	_mouseComposedArrow = _vm->_disk->loadPointer();

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

		g_system->setMouseCursor(_mouseArrow, 16, 16, 0, 0, 0);
		g_system->showMouse(true);

	} else {
		// inventory item pointer
		byte *v8 = _mouseComposedArrow->_data0;

		// FIXME: target offseting is not clear
		extractInventoryGraphics(index, v8 + 7 + 32 * 7);
		g_system->setMouseCursor(v8, 32, 32, 0, 0, 0);
	}

	return;
}




//
//	Cnv management
//
void Gfx::flatBlitCnv(Cnv *cnv, uint16 frame, int16 x, int16 y, Gfx::Buffers buffer) {

	StaticCnv scnv;

	scnv._width = cnv->_width;
	scnv._height = cnv->_height;
	scnv._data0 = cnv->getFramePtr(frame);
	scnv._data1 = NULL; // _questioner->field_8[v60->_mood & 0xF];

	flatBlitCnv(&scnv, x, y, buffer);
}

void Gfx::flatBlitCnv(StaticCnv *cnv, int16 x, int16 y, Gfx::Buffers buffer) {
	Common::Rect r(cnv->_width, cnv->_height);
	r.moveTo(x, y);

	flatBlit(r, cnv->_data0, buffer);
	return;
}


void Gfx::blitCnv(StaticCnv *cnv, int16 x, int16 y, uint16 z, Gfx::Buffers buffer) {
	Common::Rect r(cnv->_width, cnv->_height);
	r.moveTo(x, y);

	blit(r, z, cnv->_data0,  buffer);
	return;
}

void Gfx::backupDoorBackground(DoorData *data, int16 x, int16 y) {

	byte *s = _buffers[kBit2] + x + y * SCREEN_WIDTH;
	byte *d = data->_background;

	for (uint16 i = 0; i < data->_cnv->_height ; i++) {
		memcpy(d, s, data->_cnv->_width);

		s += SCREEN_WIDTH;
		d += data->_cnv->_width;
	}

	return;
}

void Gfx::backupGetBackground(GetData *data, int16 x, int16 y) {

	byte *t = data->_cnv->_data0;
	byte *s = _buffers[kBitBack] + x + y * SCREEN_WIDTH;
	byte *d = data->_backup;

	for (uint16 i = 0; i < data->_cnv->_height ; i++) {
		for (uint16 j = 0; j < data->_cnv->_width ; j++) {
			*d = (*t) ? *s : 0;

			d++;
			t++;
			s++;
		}

		s += (SCREEN_WIDTH - data->_cnv->_width);
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

	flatBlitCnv(&cnv, r.left, r.top, kBitBack);
	flatBlitCnv(&cnv, r.left, r.top, kBit2);

	return;
}

void Gfx::makeCnvFromString(StaticCnv *cnv, char *text) {
	assert(_font == _fonts[kFontLabel]);

	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		cnv->_width = _font->getStringWidth(text) + 16;
		cnv->_height = 10;
		cnv->_data0 = (byte*)malloc(cnv->_width * cnv->_height);
		memset(cnv->_data0, 0, cnv->_width * cnv->_height);

		_font->setColor(7);
		_font->drawString(cnv->_data0 + 1, cnv->_width, text);
		_font->drawString(cnv->_data0 + 1 + cnv->_width * 2, cnv->_width, text);
		_font->drawString(cnv->_data0 + cnv->_width, cnv->_width, text);
		_font->drawString(cnv->_data0 + 2 + cnv->_width, cnv->_width, text);
		_font->setColor(1);
		_font->drawString(cnv->_data0 + 1 + cnv->_width, cnv->_width, text);
	} else {
		cnv->_width = _font->getStringWidth(text);
		cnv->_height = _font->height();
		cnv->_data0 = (byte*)malloc(cnv->_width * cnv->_height);
		memset(cnv->_data0, 0, cnv->_width * cnv->_height);
		_font->drawString(cnv->_data0, cnv->_width, text);
	}

}

void Gfx::displayString(uint16 x, uint16 y, const char *text) {
	assert(_font == _fonts[kFontMenu]);

	byte *dst = _buffers[kBitFront] + x + y*SCREEN_WIDTH;
	_font->setColor(1);
	_font->drawString(dst, SCREEN_WIDTH, text);
}

void Gfx::displayCenteredString(uint16 y, const char *text) {
	uint16 x = (SCREEN_WIDTH - getStringWidth(text)) / 2;
	displayString(x, y, text);
}

void Gfx::displayBalloonString(uint16 x, uint16 y, const char *text, byte color) {
	assert(_font == _fonts[kFontDialogue]);

	byte *dst = _buffers[kBitFront] + x + y*SCREEN_WIDTH;

	_font->setColor(color);
	_font->drawString(dst, SCREEN_WIDTH, text);
}

bool Gfx::displayWrappedString(char *text, uint16 x, uint16 y, uint16 maxwidth, byte color) {
//	printf("Gfx::displayWrappedString(%s, %i, %i, %i, %i)...", text, x, y, maxwidth, color);

	uint16 lines = 0;
	bool rv = false;
	uint16 linewidth = 0;

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
	return _font->getStringWidth(text);
}

void Gfx::getStringExtent(char *text, uint16 maxwidth, int16* width, int16* height) {

	uint16 lines = 0;
	uint16 w = 0;
	*width = 0;

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


void Gfx::setFont(Fonts name) {
	assert(name < 3);
	_font = _fonts[name];
}


void Gfx::restoreBackground(const Common::Rect& r) {

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

	Common::Rect q(width, height);
	q.moveTo(left, top);

	copyRect(
		kBitBack,
		q,
		_buffers[kBit2] + q.left + q.top * SCREEN_WIDTH,
		SCREEN_WIDTH
	);

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



void Gfx::setBackground(byte *background) {
	memcpy(_buffers[kBitBack], background, SCREEN_WIDTH*SCREEN_HEIGHT);
	copyScreen(kBitBack, kBit2);
}

void Gfx::setMask(byte *mask) {
	memcpy(_buffers[kMask0], mask, SCREENMASK_WIDTH*SCREEN_HEIGHT);
}



void Gfx::copyRect(Gfx::Buffers dstbuffer, const Common::Rect& r, byte *src, uint16 pitch) {

	byte *d = _buffers[dstbuffer] + r.left + SCREEN_WIDTH * r.top;
	byte *s = src;

	for (uint16 _si = 0; _si < r.height(); _si++) {
		memcpy(d, s, r.width());

		s += pitch;
		d += SCREEN_WIDTH;
	}


}


void Gfx::grabRect(byte *dst, const Common::Rect& r, Gfx::Buffers srcbuffer, uint16 pitch) {

	byte *s = _buffers[srcbuffer] + r.left + SCREEN_WIDTH * r.top;

	for (uint16 i = 0; i < r.height(); i++) {
		memcpy(dst, s, r.width());

		s += SCREEN_WIDTH;
		dst += pitch;
	}

	return;
}


void Gfx::plotMaskPixel(uint16 x, uint16 y, byte color) {

	uint16 _ax = x + y * SCREEN_WIDTH;
	_buffers[kMask0][_ax >> 2] &= ~(3 << ((_ax & 3) << 1));

	return;
}



void Gfx::fillMaskRect(const Common::Rect& r, byte color) {

	uint16 _di = r.left/4 + r.top*80;

	for (uint16 _si = r.top; _si < r.bottom; _si++) {
		memset(&_buffers[kMask0][_di], color, r.width()/4+1);
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

Gfx::Gfx(Parallaction* vm) :
	_vm(vm) {

	g_system->beginGFXTransaction();
	g_system->initSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_system->endGFXTransaction();

	_buffers[kBitFront] = (byte*)malloc(SCREEN_SIZE);
	_buffers[kBitBack]	= (byte*)malloc(SCREEN_SIZE);
	_buffers[kBit2]   = (byte*)malloc(SCREEN_SIZE);
	_buffers[kMask0] = (byte*)malloc(SCREENMASK_WIDTH * SCREEN_HEIGHT);

	setBlackPalette();

	_bgLayers[0] = _bgLayers[1] = _bgLayers[2] = _bgLayers[3] = 0;

	memset(_palette, 0, sizeof(_palette));
	memset(_palettefx, 0, sizeof(_palettefx));

	initMouse( 0 );
	initFonts();

	_halfbrite = false;

	_font = NULL;

	return;
}

Gfx::~Gfx() {

	free(_buffers[kMask0]);
	free(_buffers[kBitFront]);
	free(_buffers[kBitBack]);
	free(_buffers[kBit2]);

	delete _fonts[kFontDialogue];
	delete _fonts[kFontLabel];
	delete _fonts[kFontMenu];

	freeStaticCnv(_mouseComposedArrow);
	delete _mouseComposedArrow;

	return;
}


} // namespace Parallaction
