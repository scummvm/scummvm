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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_FONT_H
#define BLADERUNNER_FONT_H

#include "common/str.h"

namespace Graphics {
struct Surface;
}

namespace BladeRunner {

class BladeRunnerEngine;

struct FontCharacter {
	int _x;
	int _y;
	int _width;
	int _height;
	int _dataOffset;
};

class Font {
	BladeRunnerEngine *_vm;

	int           _characterCount;
	int           _maxWidth;
	int           _maxHeight;
	FontCharacter _characters[256];
	int           _dataSize;
	uint16       *_data;
	int           _screenWidth;
	int           _screenHeight;
	int           _spacing1;
	int           _spacing2;
	uint16        _color;
	int           _intersperse;

public:
	Font(BladeRunnerEngine *vm);
	~Font();

	bool open(const Common::String &fileName, int screenWidth, int screenHeight, int spacing1, int spacing2, uint16 color);
	void close();

	void setSpacing(int spacing1, int spacing2);
	void setColor(uint16 color);

	void draw(const Common::String &text, Graphics::Surface &surface, int x, int y);
	void drawColor(const Common::String &text, Graphics::Surface &surface, int x, int y, uint16 color);

	int getTextWidth(const Common::String &text);

private:
	void reset();
	void replaceColor(uint16 oldColor, uint16 newColor);

	void drawCharacter(const char character, Graphics::Surface &surface, int x, int y);
};

} // End of namespace BladeRunner

#endif
