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

#ifndef CHEWY_FONT_H
#define CHEWY_FONT_H

#include "common/str.h"
#include "graphics/surface.h"

namespace Chewy {

class ChewyFont {
public:
	ChewyFont(Common::String filename);
	virtual ~ChewyFont();

	Graphics::Surface *getLine(const Common::String &texts);
	void setDisplaySize(uint16 width, uint16 height);
	void setDeltaX(uint16 deltaX);

	uint16 getDataWidth() const { return _dataWidth; }
	uint16 getDataHeight() const { return _dataHeight; }
	uint16 getFirst() const { return _first; }
	uint16 getLast() const { return _last; }

private:
	uint16 _count, _first, _last;
	uint16 _dataWidth, _dataHeight;
	uint16 _displayWidth, _displayHeight;
	uint16 _deltaX;

	Graphics::Surface _fontSurface;
};

class FontMgr {
private:
	ChewyFont *_font;

public:
	FontMgr() : _font(nullptr) {};
	virtual ~FontMgr() {};

	Graphics::Surface *getLine(const Common::String &texts);

	void setFont(ChewyFont *font) { _font = font; }
	ChewyFont *getFont() { return _font; }
};

} // namespace Chewy

#endif
