/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bagel/baglib/cursor.h"

namespace Bagel {


static const int CURSOR_W = 12;
static const int CURSOR_H = 20;
static const byte ARROW_CURSOR[CURSOR_W * CURSOR_H] = {
	1,1,0,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,1,1,1,1,1,
	1,2,2,2,1,2,2,1,0,0,0,0,
	1,2,2,1,1,2,2,1,0,0,0,0,
	1,2,1,0,0,1,2,2,1,0,0,0,
	1,1,0,0,0,1,2,2,1,0,0,0,
	1,0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,0,0,1,1,0,0,
};
static const byte CURSOR_PALETTE[] = { 0x80, 0x80, 0x80, 0, 0, 0, 0xff, 0xff, 0xff };

CBagCursor *CBagCursor::_currentCursor;
CSystemCursor *CBagCursor::_systemCursor;

void CBagCursor::initialize() {
	_currentCursor = nullptr;
	_systemCursor = new CSystemCursor();
}

void CBagCursor::shutdown() {
	delete _systemCursor;
}

void CBagCursor::showSystemCursor() {
	_systemCursor->setCurrent();
}


CBagCursor::CBagCursor() {
	_bitmap = nullptr;
	_x = 0;
	_y = 0;
	_sharedPalFl = false;

	// Default is that this is not a wielded cursor
	setWieldCursor(false);
}

CBagCursor::CBagCursor(CBofBitmap *bmp) {
	_bitmap = nullptr;
	_x = 0;
	_y = 0;
	_sharedPalFl = false;

	load(bmp);

	// Default is that this is not a wielded cursor
	setWieldCursor(false);
}

CBagCursor::CBagCursor(const char *fileName, bool sharedPalFl) {
	assert(fileName != nullptr);

	_bitmap = nullptr;
	_x = 0;
	_y = 0;
	_sharedPalFl = sharedPalFl;
	strncpy(_fileName, fileName, MAX_FNAME - 1);

	// Default is that this is not a wielded cursor
	setWieldCursor(false);
}

CBagCursor::~CBagCursor() {
	assert(isValidObject(this));

	unLoad();
}

ErrorCode CBagCursor::load(CBofBitmap *bmp) {
	assert(isValidObject(this));
	assert(bmp != nullptr);

	unLoad();

	_bitmap = bmp;

	return _errCode;
}

ErrorCode CBagCursor::load(const char *fileName, CBofPalette *pal) {
	assert(isValidObject(this));
	assert(fileName != nullptr);

	unLoad();

	_bitmap = loadBitmap(fileName, pal, _sharedPalFl);

	return _errCode;
}

void CBagCursor::unLoad() {
	assert(isValidObject(this));

	delete _bitmap;
	_bitmap = nullptr;
}

void CBagCursor::setCurrent() {
	_currentCursor = this;
	CursorMan.replaceCursorPalette(_bitmap->getPalette()->getData(), 0, PALETTE_COUNT);
	CursorMan.replaceCursor(_bitmap->getSurface(), _x, _y, 1);
}

void CSystemCursor::setCurrent() {
	_currentCursor = this;

	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	CursorMan.replaceCursorPalette(CURSOR_PALETTE, 0, ARRAYSIZE(CURSOR_PALETTE) / 3);
	CursorMan.replaceCursor(ARROW_CURSOR, CURSOR_W, CURSOR_H, 0, 0, 0, true, &format);
}

} // namespace Bagel
