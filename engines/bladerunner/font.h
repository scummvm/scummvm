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

#include "common/array.h"
#include "common/str.h"

#include "graphics/font.h"

namespace Graphics {
struct Surface;
}

namespace BladeRunner {

class BladeRunnerEngine;

class Font : public Graphics::Font {
	struct Character {
		int x;
		int y;
		int width;
		int height;
		int dataOffset;
	};

	uint32                   _characterCount;
	int                      _maxWidth;
	int                      _maxHeight;
	Common::Array<Character> _characters;
	int                      _dataSize;
	uint16                  *_data;
	int                      _screenWidth;
	int                      _screenHeight;
	int                      _spacing;
	bool                     _useFontColor;

public:
	~Font();

	static Font* load(BladeRunnerEngine *vm, const Common::String &fileName, int spacing, bool useFontColor);

	int getFontHeight() const;
	int getMaxCharWidth() const;
	int getCharWidth(uint32 chr) const;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const;

private:
	Font();
	void reset();
	void close();
	// void drawCharacter(const uint8 character, Graphics::Surface &surface, int x, int y) const;
};

} // End of namespace BladeRunner

#endif
