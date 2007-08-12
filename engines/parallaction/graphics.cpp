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



Palette::Palette() {

	int gameType = _vm->getGameType();

	if (gameType == GType_Nippon) {
		_colors = 32;
		_hb = (_vm->getPlatform() == Common::kPlatformAmiga);
	} else
	if (gameType == GType_BRA) {
		_colors = 256;
		_hb = false;
	} else
		error("can't create palette for id = '%i'", gameType);

	_size = _colors * 3;

	makeBlack();
}

Palette::Palette(const Palette &pal) {
	clone(pal);
}

void Palette::clone(const Palette &pal) {
	_colors = pal._colors;
	_hb = pal._hb;
	_size = pal._size;
	memcpy(_data, pal._data, _size);
}


void Palette::makeBlack() {
	memset(_data, 0, _size);
}

void Palette::setEntry(uint index, int red, int green, int blue) {
	assert(index < _colors);

	if (red >= 0)
		_data[index*3] = red & 0xFF;

	if (green >= 0)
		_data[index*3+1] = green & 0xFF;

	if (blue >= 0)
		_data[index*3+2] = blue & 0xFF;
}

void Palette::makeGrayscale() {
	byte v;
	for (uint16 i = 0; i < _colors; i++) {
		v = MAX(_data[i*3+1], _data[i*3+2]);
		v = MAX(v, _data[i*3]);
		setEntry(i, v, v, v);
	}
}

void Palette::fadeTo(const Palette& target, uint step) {

	if (step == 0)
		return;

	for (uint16 i = 0; i < _size; i++) {
		if (_data[i] == target._data[i]) continue;

		if (_data[i] < target._data[i])
			_data[i] = CLIP(_data[i] + step, (uint)0, (uint)target._data[i]);
		else
			_data[i] = CLIP(_data[i] - step, (uint)target._data[i], (uint)255);
	}

	return;
}

uint Palette::fillRGBA(byte *rgba) {

	byte r, g, b;
	byte *hbPal = rgba + _size;

	for (uint32 i = 0; i < _colors; i++) {
		r = (_data[i*3]   << 2) | (_data[i*3]   >> 4);
		g = (_data[i*3+1] << 2) | (_data[i*3+1] >> 4);
		b = (_data[i*3+2] << 2) | (_data[i*3+2] >> 4);

		rgba[i*4]   = r;
		rgba[i*4+1] = g;
		rgba[i*4+2] = b;
		rgba[i*4+3] = 0;

		if (_hb) {
			hbPal[i*4]   = r >> 1;
			hbPal[i*4+1] = g >> 1;
			hbPal[i*4+2] = b >> 1;
			hbPal[i*4+3] = 0;
		}

	}

	return ((_hb) ? 2 : 1) * _colors;
}

void Palette::rotate(uint first, uint last, bool forward) {

	byte tmp[3];

	if (forward) { 					// forward

		tmp[0] = _data[first * 3];
		tmp[1] = _data[first * 3 + 1];
		tmp[2] = _data[first * 3 + 2];

		memmove(_data+first*3, _data+(first+1)*3, (last - first)*3);

		_data[last * 3]	 = tmp[0];
		_data[last * 3 + 1] = tmp[1];
		_data[last * 3 + 2] = tmp[2];

	} else {											// backward

		tmp[0] = _data[last * 3];
		tmp[1] = _data[last * 3 + 1];
		tmp[2] = _data[last * 3 + 2];

		memmove(_data+(first+1)*3, _data+first*3, (last - first)*3);

		_data[first * 3]	  = tmp[0];
		_data[first * 3 + 1] = tmp[1];
		_data[first * 3 + 2] = tmp[2];

	}

}


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
	Common::Rect s(BALLOON_TAIL_WIDTH, BALLOON_TAIL_HEIGHT);
	s.moveTo(r.left + (r.width()+5)/2 - 5, r.bottom - 1);
	flatBlit(s, _resBalloonTail[winding], kBitFront, 2);

	return;
}




void Gfx::setPalette(Palette pal) {
	byte sysPal[256*4];

	uint n = pal.fillRGBA(sysPal);
	g_system->setPalette(sysPal, 0, n);
}

void Gfx::setBlackPalette() {
	Palette pal;
	setPalette(pal);
}




void Gfx::animatePalette() {

	for (uint16 i = 0; i < 4; i++) {

		if ((_palettefx[i]._flags & 1) == 0) continue;		// animated palette

		_palettefx[i]._timer += _palettefx[i]._step * 2;	// update timer

		if (_palettefx[i]._timer < 0x4000) continue;		// check timeout

		_palettefx[i]._timer = 0;							// reset timer

		_palette.rotate(_palettefx[i]._first, _palettefx[i]._last, (_palettefx[i]._flags & 2) != 0);
	}

	setPalette(_palette);

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
	g_system->copyRectToScreen((const byte*)_buffers[kBitFront]->pixels, _buffers[kBitFront]->pitch, _screenX, _screenY, _vm->_screenWidth, _vm->_screenHeight);
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

		d += _backgroundWidth;
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

void Gfx::flatBlit(const Common::Rect& r, byte *data, Gfx::Buffers buffer, byte transparentColor) {

	Common::Point dp;
	Common::Rect q(r);

	screenClip(q, dp);

	byte *s = data + q.left + q.top * r.width();
	byte *d = (byte*)_buffers[buffer]->getBasePtr(dp.x, dp.y);

	uint sPitch = r.width() - q.width();
	uint dPitch = _backgroundWidth - q.width();

	for (uint16 i = q.top; i < q.bottom; i++) {
		for (uint16 j = q.left; j < q.right; j++) {
			if (*s != transparentColor)
				*d = *s;

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
	uint dPitch = _backgroundWidth - q.width();

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

	if (label->_cnv.w == 0)
		return;
	_vm->_gfx->flatBlitCnv(&label->_cnv, _vm->_gfx->_labelPosition[0].x, _vm->_gfx->_labelPosition[0].y, Gfx::kBitBack);

	return;
}

void jobEraseLabel(void *parm, Job *j) {
	Label *label = (Label*)parm;

	debugC(9, kDebugJobs, "jobEraseLabel (%p)", (const void*) label);

	int16 _si, _di;

	if (_vm->_activeItem._id != 0) {
		_si = _vm->_mousePos.x + 16 - label->_cnv.w/2;
		_di = _vm->_mousePos.y + 34;
	} else {
		_si = _vm->_mousePos.x + 8 - label->_cnv.w/2;
		_di = _vm->_mousePos.y + 21;
	}

	if (_si < 0) _si = 0;
	if (_di > 190) _di = 190;

	if (label->_cnv.w + _si > _vm->_screenWidth)
		_si = _vm->_screenWidth - label->_cnv.w;

	Common::Rect r(label->_cnv.w, label->_cnv.h);
	r.moveTo(_vm->_gfx->_labelPosition[1]);
	_vm->_gfx->restoreBackground(r);

	_vm->_gfx->_labelPosition[1] = _vm->_gfx->_labelPosition[0];
	_vm->_gfx->_labelPosition[0].x = _si;
	_vm->_gfx->_labelPosition[0].y = _di;

	return;
}







//
//	Cnv management
//
void Gfx::flatBlitCnv(Cnv *cnv, uint16 frame, int16 x, int16 y, Gfx::Buffers buffer) {

	Graphics::Surface scnv;

	scnv.w = cnv->_width;
	scnv.h = cnv->_height;
	scnv.pixels = cnv->getFramePtr(frame);

	flatBlitCnv(&scnv, x, y, buffer);
}

void Gfx::flatBlitCnv(Graphics::Surface *cnv, int16 x, int16 y, Gfx::Buffers buffer) {
	Common::Rect r(cnv->w, cnv->h);
	r.moveTo(x, y);

	flatBlit(r, (byte*)cnv->pixels, buffer, 0);
	return;
}


void Gfx::blitCnv(Graphics::Surface *cnv, int16 x, int16 y, uint16 z, Gfx::Buffers buffer) {
	Common::Rect r(cnv->w, cnv->h);
	r.moveTo(x, y);

	blit(r, z, (byte*)cnv->pixels,  buffer);
	return;
}

void Gfx::backupDoorBackground(DoorData *data, int16 x, int16 y) {
	byte *s = (byte*)_buffers[kBit2]->getBasePtr(x, y);
	copyRect(data->_cnv->_width, data->_cnv->_height, data->_background, data->_cnv->_width, s,_backgroundWidth);
	return;
}

void Gfx::backupGetBackground(GetData *data, int16 x, int16 y) {

	byte *t = (byte*)data->_cnv->pixels;
	byte *s = (byte*)_buffers[kBitBack]->getBasePtr(x, y);
	byte *d = data->_backup;

	uint pitch = _backgroundWidth - data->_cnv->w;

	for (uint16 i = 0; i < data->_cnv->h ; i++) {
		for (uint16 j = 0; j < data->_cnv->w ; j++) {
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
void Gfx::restoreDoorBackground(const Common::Rect& r, byte *data, byte* background) {

	byte *t = data;
	byte *s = background;
	byte *d0 = (byte*)_buffers[kBitBack]->getBasePtr(r.left, r.top);
	byte *d1 = (byte*)_buffers[kBit2]->getBasePtr(r.left, r.top);

	uint pitch = _backgroundWidth - r.width();

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

	flatBlit(r, data, kBitBack, 0);
	flatBlit(r, data, kBit2, 0);

	return;
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


void Gfx::setFont(Font *font) {
	assert(font);
	_font = font;
}


void Gfx::restoreBackground(const Common::Rect& r) {

	int16 left = r.left;
	int16 top = r.top;
	int16 width = r.width();
	int16 height = r.height();

	if (left < 0) left = 0;
	if (top < 0) top = 0;

	if (left >= _backgroundWidth) return;
	if (top >= _backgroundHeight) return;

	if (left+width >= _backgroundWidth) width = _backgroundWidth - left;
	if (top+height >= _backgroundHeight) height = _backgroundHeight - top;

	Common::Rect q(width, height);
	q.moveTo(left, top);

	copyRect(kBitBack, q, (byte*)_buffers[kBit2]->getBasePtr(q.left, q.top), _backgroundWidth);

	return;
}



void Gfx::setBackground(Graphics::Surface *surface) {
	_buffers[kBit2] = surface;

	initBuffers(surface->w, surface->h);
	copyScreen(kBit2, kBitBack);
}

void Gfx::setMask(MaskBuffer *buffer) {
	_depthMask = buffer;
}

void Gfx::copyRect(uint width, uint height, byte *dst, uint dstPitch, byte *src, uint srcPitch) {

	for (uint16 _si = 0; _si < height; _si++) {
		memcpy(dst, src, width);

		src += srcPitch;
		dst += dstPitch;
	}

	return;
}

void Gfx::copyRect(Gfx::Buffers dstbuffer, const Common::Rect& r, byte *src, uint16 pitch) {
	byte *d = (byte*)_buffers[dstbuffer]->getBasePtr(r.left, r.top);
	copyRect(r.width(), r.height(), d, _backgroundWidth, src, pitch);
	return;
}


void Gfx::grabRect(byte *dst, const Common::Rect& r, Gfx::Buffers srcbuffer, uint16 pitch) {
	byte *s = (byte*)_buffers[srcbuffer]->getBasePtr(r.left, r.top);
	copyRect(r.width(), r.height(), dst, pitch, s, _backgroundWidth);
	return;
}

/*
	the following 3 routines are hacks for Nippon Safes coming from the original code,
	so they shouldn't be modified when adding support for other games
*/

void Gfx::zeroMaskValue(uint16 x, uint16 y, byte color) {

	uint16 _ax = x + y * _backgroundWidth;
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

	_buffers[kBit2] = 0;
	_depthMask = 0;

	initBuffers(_vm->_screenWidth, _vm->_screenHeight);

	setPalette(_palette);

	_screenX = 0;
	_screenY = 0;

	_bgLayers[0] = _bgLayers[1] = _bgLayers[2] = _bgLayers[3] = 0;

	memset(_palettefx, 0, sizeof(_palettefx));

	_halfbrite = false;

	_font = NULL;

	return;
}

Gfx::~Gfx() {

	freeBuffers();

	return;
}

void Gfx::initBuffers(int w, int h) {

	_backgroundWidth = w;
	_backgroundHeight = h;

	if (!_buffers[kBitFront]) {
		_buffers[kBitFront] = new Graphics::Surface;
	}

	if (!_buffers[kBitBack]) {
		_buffers[kBitBack] = new Graphics::Surface;
	}

	if (_buffers[kBitFront]->w != w || _buffers[kBitFront]->h != h) {
		_buffers[kBitFront]->create(w, h, 1);
	}

	if (_buffers[kBitBack]->w != w || _buffers[kBitBack]->h != h) {
		_buffers[kBitBack]->create(w, h, 1);
	}

}

void Gfx::freeBuffers() {

	if (_buffers[kBitFront]) {
		_buffers[kBitFront]->free();
		delete _buffers[kBitFront];
	}

	if (_buffers[kBitBack]) {
		_buffers[kBitBack]->free();
		delete _buffers[kBitBack];
	}

	_buffers[kBitFront] = 0;
	_buffers[kBitBack] = 0;
}


} // namespace Parallaction
