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
 */

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "graphics/palette.h"
#include "access/screen.h"
#include "access/resources.h"

namespace Access {

Screen::Screen(AccessEngine *vm) : _vm(vm) {
	create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	Common::fill(&_tempPalette[0], &_tempPalette[PALETTE_SIZE], 0);
	_loadPalFlag = false;
	_leftSkip = _rightSkip = 0;
	_topSkip = _bottomSkip = 0;
	_clipWidth = _clipHeight = 0;
}

void Screen::setDisplayScan() {
	warning("TODO: setDisplayScan");
}

void Screen::updateScreen() {
	g_system->updateScreen();
}

void Screen::setInitialPalettte() {
	Common::copy(&INITIAL_PALETTE[0], &INITIAL_PALETTE[18 * 3], _rawPalette);
	Common::fill(&_rawPalette[18 * 3], &_rawPalette[PALETTE_SIZE], 0);

	g_system->getPaletteManager()->setPalette(INITIAL_PALETTE, 0, 18);
}

void Screen::loadPalette(Common::SeekableReadStream *stream) {
	stream->read(&_rawPalette[0], PALETTE_SIZE);
	setPalette();
	_loadPalFlag = true;
}

void Screen::setPalette() {
	g_system->getPaletteManager()->setPalette(&_rawPalette[0], 0, PALETTE_COUNT);
}

void Screen::updatePalette() {
	g_system->getPaletteManager()->setPalette(&_tempPalette[0], 0, PALETTE_COUNT);
	updateScreen();
}

void Screen::forceFadeOut() {
	const int FADE_AMOUNT = 2;
	bool repeatFlag;
	byte *srcP;
	int count;

	do {
		repeatFlag = false;
		for (srcP = &_tempPalette[0], count = 0; count < PALETTE_COUNT; ++count, ++srcP) {
			int v = *srcP;
			if (v) {
				repeatFlag = true;
				*srcP = MAX(*srcP - FADE_AMOUNT, 0);
			}
		}

		updatePalette();
		g_system->delayMillis(10);
	} while (repeatFlag);
}

void Screen::forceFadeIn() {
	Common::fill(&_tempPalette[0], &_tempPalette[PALETTE_SIZE], 0);

	const int FADE_AMOUNT = 2;
	bool repeatFlag;
	do {
		repeatFlag = false;
		const byte *srcP = &_rawPalette[0];
		byte *destP = &_tempPalette[0];

		for (int idx = 0; idx < PALETTE_SIZE; ++idx, ++srcP, ++destP) {
			if (*destP != *srcP) {
				repeatFlag = true;
				*destP = MAX((int)*destP + FADE_AMOUNT, (int)*srcP);
			}
		}

		updatePalette();
		g_system->delayMillis(10);
	} while (repeatFlag);
}

void Screen::copyBuffer(const byte *data) {
	byte *destP = (byte *)getPixels();
	Common::copy(data, data + (h * w), destP);
	g_system->copyRectToScreen(destP, w, 0, 0, w, h);
}

void Screen::plotImage(const byte *pData, int idx, const Common::Point &pt) {
	const byte *sizeP = pData + READ_LE_UINT16(pData + idx * 4);
	int w = READ_LE_UINT16(sizeP);
	int h = READ_LE_UINT16(sizeP + 2);
	Common::Rect r(pt.x, pt.y, pt.x + w, pt.y + h);

	if (!clip(r)) {
		_lastBounds = r;
		//plotf();
	}
}

bool Screen::clip(Common::Rect &r) {
	int skip;
	_leftSkip = _rightSkip = 0;
	_topSkip = _bottomSkip = 0;

	if (r.left > _clipWidth) {
		skip = -r.left;
		r.setWidth(r.width() - skip);
		_leftSkip = skip;
		r.moveTo(0, r.top);
	} else if (r.left >= 0)
		return true;

	int right = r.right - 1;
	if (right < 0)
		return true;
	else if (right > _clipWidth) {
		skip = right - _clipWidth;
		r.setWidth(r.width() - skip);
		_rightSkip = skip;
	}

	if (r.top > _clipHeight) {
		skip = -r.top;
		r.setHeight(r.height() - skip);
		_topSkip = skip;
		r.moveTo(r.left, 0);
	} else if (r.top >= 0)
		return true;

	int bottom = r.bottom - 1;
	if (bottom < 0)
		return true;
	else if (bottom > _clipHeight) {
		skip = bottom - _clipHeight;
		_bottomSkip = skip;
		r.setHeight(r.height() - skip);
	}

	return false;
}

} // End of namespace Access
