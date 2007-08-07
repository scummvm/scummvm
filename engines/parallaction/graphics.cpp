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


namespace Parallaction {

#define BALLOON_TAIL_WIDTH	12
#define BALLOON_TAIL_HEIGHT	10


byte _resBalloonTail[2][BALLOON_TAIL_WIDTH*BALLOON_TAIL_HEIGHT] = {
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

	// draws balloon
	q.right += 5;
	floodFill(kBitFront, q, 0);
	q.grow(-1);
	floodFill(kBitFront, q, 1);

	// draws tail
	// TODO: this bitmap tail should only be used for Dos games. Amiga should use a polygon fill.
	winding = (winding == 0 ? 1 : 0);
	byte *s = _resBalloonTail[winding];
	byte *d = (byte*)_buffers[kBitFront]->getBasePtr(r.left + (r.width()+5)/2 - 5, r.bottom - 1);
	uint pitch = _vm->_screenWidth - BALLOON_TAIL_WIDTH;
	for (uint16 i = 0; i < BALLOON_TAIL_HEIGHT; i++) {
		for (uint16 j = 0; j < BALLOON_TAIL_WIDTH; j++) {
			if (*s != 2)
				*d = *s;
			d++;
			s++;
		}

		d += pitch;
	}

//	printf("done\n");

	return;
}

void Gfx::showLocationComment(const char *text, bool end) {

	setFont(kFontDialogue);

	int16 w, h;
	getStringExtent(const_cast<char*>(text), 130, &w, &h);

	Common::Rect r(w + (end ? 5 : 10), h + 5);
	r.moveTo(5, 5);

	floodFill(kBitFront, r, 0);
	r.grow(-2);
	floodFill(kBitFront, r, 1);
	displayWrappedString(const_cast<char*>(text), 3, 5, 0, 130);

	updateScreen();

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

	return;
}

void Gfx::setBlackPalette() {
	Palette pal;
	memset(pal, 0, PALETTE_SIZE);
	setPalette(pal);
}


void Gfx::animatePalette() {

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

void Gfx::makeGrayscalePalette(Palette pal) {

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

void Gfx::fadePalette(Palette pal, Palette target, uint step) {

	if (step == 0)
		return;

	for (uint16 i = 0; i < BASE_PALETTE_COLORS * 3; i++) {
		if (pal[i] == target[i]) continue;

		if (pal[i] < target[i])
			pal[i] = CLIP(pal[i] + step, (uint)0, (uint)target[i]);
		else
			pal[i] = CLIP(pal[i] - step, (uint)target[i], (uint)255);

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
	g_system->copyRectToScreen((const byte*)_buffers[kBitFront]->pixels, _vm->_screenWidth, 0, 0, _vm->_screenWidth, _vm->_screenHeight);
	g_system->updateScreen();
	return;
}


void Gfx::swapBuffers() {
	Graphics::Surface *temp = _buffers[kBitFront];
	_buffers[kBitFront] = _buffers[kBitBack];
	_buffers[kBitBack] = temp;
	updateScreen();
	return;
}


//
//	graphic primitives
//
void Gfx::clearScreen(Gfx::Buffers buffer) {
	memset(_buffers[buffer]->pixels, 0, _vm->_screenSize);

	if (buffer == kBitFront) updateScreen();

	return;
}


void Gfx::copyScreen(Gfx::Buffers srcbuffer, Gfx::Buffers dstbuffer) {
	memcpy(_buffers[dstbuffer]->pixels, _buffers[srcbuffer]->pixels, _vm->_screenSize);

	return;
}

void Gfx::floodFill(Gfx::Buffers buffer, const Common::Rect& r, byte color) {

	byte *d = (byte*)_buffers[buffer]->getBasePtr(r.left, r.top);
	uint16 w = r.width() + 1;
	uint16 h = r.height() + 1;

	for (uint16 i = 0; i < h; i++) {
		memset(d, color, w);

		d += _vm->_screenWidth;
	}

	return;
}

void Gfx::screenClip(Common::Rect& r, Common::Point& p) {

	int32 x = r.left;
	int32 y = r.top;

	Common::Rect screen(0, 0, _vm->_screenWidth, _vm->_screenHeight);

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
	byte *d = (byte*)_buffers[buffer]->getBasePtr(dp.x, dp.y);

	uint sPitch = r.width() - q.width();
	uint dPitch = _vm->_screenWidth - q.width();

	for (uint16 i = q.top; i < q.bottom; i++) {
		for (uint16 j = q.left; j < q.right; j++) {
			if (*s != 0) *d = *s;
			s++;
			d++;
		}

		s += sPitch;
		d += dPitch;
	}

	return;

}

void Gfx::blit(const Common::Rect& r, uint16 z, byte *data, Gfx::Buffers buffer) {

	Common::Point dp;
	Common::Rect q(r);

	screenClip(q, dp);

	byte *s = data + q.left + q.top * r.width();
	byte *d = (byte*)_buffers[buffer]->getBasePtr(dp.x, dp.y);

	uint sPitch = r.width() - q.width();
	uint dPitch = _vm->_screenWidth - q.width();

	for (uint16 i = 0; i < q.height(); i++) {

		for (uint16 j = 0; j < q.width(); j++) {
			if (*s != 0) {
				byte v = _depthMask->getValue(dp.x + j, dp.y + i);
				if (z >= v) *d = *s;
			}

			s++;
			d++;
		}

		s += sPitch;
		d += dPitch;
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

	if (label->_cnv._width + _si > _vm->_screenWidth)
		_si = _vm->_screenWidth - label->_cnv._width;

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

	byte temp[MOUSEARROW_WIDTH*MOUSEARROW_HEIGHT];
	memcpy(temp, _mouseArrow, MOUSEARROW_WIDTH*MOUSEARROW_HEIGHT);

	uint16 k = 0;
	for (uint16 i = 0; i < 4; i++) {
		for (uint16 j = 0; j < 64; j++) _mouseArrow[k++] = temp[i + j * 4];
	}

	return;
}


void Gfx::setMousePointer(int16 index) {

	if (index == kCursorArrow) {		// standard mouse pointer

		g_system->setMouseCursor(_mouseArrow, MOUSEARROW_WIDTH, MOUSEARROW_HEIGHT, 0, 0, 0);
		g_system->showMouse(true);

	} else {
		// inventory item pointer
		byte *v8 = _mouseComposedArrow->_data0;

		// FIXME: destination offseting is not clear
		byte* s = _vm->_char._objs->getFramePtr(getInventoryItemIndex(index));
		byte* d = v8 + 7 + MOUSECOMBO_WIDTH * 7;

		for (uint32 i = 0; i < INVENTORYITEM_HEIGHT; i++) {
			memcpy(d, s, INVENTORYITEM_WIDTH);

			s += INVENTORYITEM_PITCH;
			d += MOUSECOMBO_WIDTH;
		}

		g_system->setMouseCursor(v8, MOUSECOMBO_WIDTH, MOUSECOMBO_HEIGHT, 0, 0, 0);
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
	scnv._data1 = NULL; // ->field_8[v60->_mood & 0xF];

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

	byte *s = (byte*)_buffers[kBit2]->getBasePtr(x, y);
	byte *d = data->_background;

	for (uint16 i = 0; i < data->_cnv->_height ; i++) {
		memcpy(d, s, data->_cnv->_width);

		s += _vm->_screenWidth;
		d += data->_cnv->_width;
	}

	return;
}

void Gfx::backupGetBackground(GetData *data, int16 x, int16 y) {

	byte *t = data->_cnv->_data0;
	byte *s = (byte*)_buffers[kBitBack]->getBasePtr(x, y);
	byte *d = data->_backup;

	uint pitch = _vm->_screenWidth - data->_cnv->_width;

	for (uint16 i = 0; i < data->_cnv->_height ; i++) {
		for (uint16 j = 0; j < data->_cnv->_width ; j++) {
			*d = (*t) ? *s : 0;

			d++;
			t++;
			s++;
		}

		s += pitch;
	}

	return;
}

//
//	restores background according to specified frame
//
void Gfx::restoreDoorBackground(StaticCnv *cnv, const Common::Rect& r, byte* background) {

	byte *t = cnv->_data0;
	byte *s = background;
	byte *d0 = (byte*)_buffers[kBitBack]->getBasePtr(r.left, r.top);
	byte *d1 = (byte*)_buffers[kBit2]->getBasePtr(r.left, r.top);

	uint pitch = _vm->_screenWidth - r.width();

	for (uint16 i = 0; i < r.height() ; i++) {
		for (uint16 j = 0; j < r.width() ; j++) {
			if (*t) {
				*d0 = *s;
				*d1 = *s;
			}

			d0++;
			d1++;
			t++;
			s++;
		}

		d0 += pitch;
		d1 += pitch;
	}


	return;
}


//
//	copies a rectangular bitmap on the background
//
void Gfx::restoreGetBackground(const Common::Rect& r, byte *data) {

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

void Gfx::displayString(uint16 x, uint16 y, const char *text, byte color) {
	byte *dst = (byte*)_buffers[kBitFront]->getBasePtr(x, y);
	_font->setColor(color);
	_font->drawString(dst, _vm->_screenWidth, text);
}

void Gfx::displayCenteredString(uint16 y, const char *text) {
	uint16 x = (_vm->_screenWidth - getStringWidth(text)) / 2;
	displayString(x, y, text, 1);
}

bool Gfx::displayWrappedString(char *text, uint16 x, uint16 y, byte color, int16 wrapwidth) {

	uint16 lines = 0;
	bool rv = false;
	uint16 linewidth = 0;

	uint16 rx = x + 10;
	uint16 ry = y + 4;

	char token[40];

	if (wrapwidth == -1)
		wrapwidth = _vm->_screenWidth;

	while (strlen(text) > 0) {

		text = parseNextToken(text, token, 40, "   ", true);
		linewidth += getStringWidth(token);

		if (linewidth > wrapwidth) {
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
			displayString(rx, ry, token, color);

		rx += getStringWidth(token) + getStringWidth(" ");
		linewidth += getStringWidth(" ");

		text = Common::ltrim(text);
	}

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

		text = parseNextToken(text, token, 40, "   ", true);
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

	if (left >= _vm->_screenWidth) return;
	if (top >= _vm->_screenHeight) return;

	if (left+width >= _vm->_screenWidth) width = _vm->_screenWidth - left;
	if (top+height >= _vm->_screenHeight) height = _vm->_screenHeight - top;

	Common::Rect q(width, height);
	q.moveTo(left, top);

	copyRect(kBitBack, q, (byte*)_buffers[kBit2]->getBasePtr(q.left, q.top), _vm->_screenWidth);

	return;
}

void Gfx::freeStaticCnv(StaticCnv *cnv) {

	if (!cnv) return;

	if (!cnv || !cnv->_data0) return;
	free(cnv->_data0);
	cnv->_data0 = NULL;

	return;
}



void Gfx::setBackground(Graphics::Surface *surface) {
	if (_buffers[kBit2])
		delete _buffers[kBit2];

	_buffers[kBit2] = surface;
	copyScreen(kBit2, kBitBack);
}

void Gfx::setMask(MaskBuffer *buffer) {
	if (_depthMask)
		delete _depthMask;

	_depthMask = buffer;
}



void Gfx::copyRect(Gfx::Buffers dstbuffer, const Common::Rect& r, byte *src, uint16 pitch) {

	byte *d = (byte*)_buffers[dstbuffer]->getBasePtr(r.left, r.top);
	byte *s = src;

	for (uint16 _si = 0; _si < r.height(); _si++) {
		memcpy(d, s, r.width());

		s += pitch;
		d += _vm->_screenWidth;
	}


}


void Gfx::grabRect(byte *dst, const Common::Rect& r, Gfx::Buffers srcbuffer, uint16 pitch) {

	byte *s = (byte*)_buffers[srcbuffer]->getBasePtr(r.left, r.top);

	for (uint16 i = 0; i < r.height(); i++) {
		memcpy(dst, s, r.width());

		s += _vm->_screenWidth;
		dst += pitch;
	}

	return;
}

/*
	the following 3 routines are hacks for Nippon Safes coming from the original code,
	so they shouldn't be modified when adding support for other games
*/

void Gfx::zeroMaskValue(uint16 x, uint16 y, byte color) {

	uint16 _ax = x + y * _vm->_screenWidth;
	_depthMask->data[_ax >> 2] &= ~(3 << ((_ax & 3) << 1));

	return;
}
void Gfx::fillMaskRect(const Common::Rect& r, byte color) {

	uint16 _di = r.left/4 + r.top * _depthMask->internalWidth;

	for (uint16 _si = r.top; _si < r.bottom; _si++) {
		memset(_depthMask->data + _di, color, r.width()/4+1);
		_di += _depthMask->internalWidth;
	}

	return;

}
void Gfx::intGrottaHackMask() {
	memset(_depthMask->data + 3600, 0, 3600);
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
	g_system->initSize(_vm->_screenWidth, _vm->_screenHeight);
	g_system->endGFXTransaction();

	_buffers[kBitFront] = new Graphics::Surface;
	_buffers[kBitFront]->create(_vm->_screenWidth, _vm->_screenHeight, 1);
	_buffers[kBitBack] = new Graphics::Surface;
	_buffers[kBitBack]->create(_vm->_screenWidth, _vm->_screenHeight, 1);

	_buffers[kBit2] = 0;

	_depthMask = 0;

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

	_depthMask->free();
	delete _depthMask;

	_buffers[kBitFront]->free();
	delete _buffers[kBitFront];
	_buffers[kBitBack]->free();
	delete _buffers[kBitBack];

	if (_buffers[kBit2])
		delete _buffers[kBit2];

	delete _fonts[kFontDialogue];
	delete _fonts[kFontLabel];
	delete _fonts[kFontMenu];

	freeStaticCnv(_mouseComposedArrow);
	delete _mouseComposedArrow;

	return;
}


} // namespace Parallaction
