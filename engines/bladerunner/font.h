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

#include "bladerunner/bladerunner.h" // needed for definition of Common::ScopedPtr (subtitles font external font file support) -- and for the subtitles relevant macro defines
#include "common/util.h"
#include "common/str.h"

namespace Graphics {
struct Surface;
}

namespace BladeRunner {

class BladeRunnerEngine;

class Font {
	struct Character {
		int x;
		int y;
		int width;
		int height;
		int dataOffset;
	};

	BladeRunnerEngine *_vm;

	int           _characterCount;
	int           _maxWidth;
	int           _maxHeight;
	Character     _characters[256];
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

	#if BLADERUNNER_SUBTITLES_EXTERNAL_FONT
	bool openFromStream(Common::ScopedPtr<Common::SeekableReadStream> &s, int screenWidth, int screenHeight, int spacing1, int spacing2, uint16 color);
	#endif // BLADERUNNER_SUBTITLES_EXTERNAL_FONT
	bool open(const Common::String &fileName, int screenWidth, int screenHeight, int spacing1, int spacing2, uint16 color);
	void close();

	void setSpacing(int spacing1, int spacing2);
	void setColor(uint16 color);
	#if !BLADERUNNER_SUBTITLES_EXTERNAL_FONT
	void setBlackColor(); // for subtitles (when using internal font) - emulate shadows
	#endif // !BLADERUNNER_SUBTITLES_EXTERNAL_FONT

	void draw(const Common::String &text, Graphics::Surface &surface, int x, int y) const;
	void drawColor(const Common::String &text, Graphics::Surface &surface, int x, int y, uint16 color);
	void drawNumber(int num, Graphics::Surface &surface, int x, int y) const;

	int getTextWidth(const Common::String &text) const;
	int getTextHeight(const Common::String &text) const;

private:
	void reset();
	void replaceColor(uint16 oldColor, uint16 newColor);

	void drawCharacter(const uint8 character, Graphics::Surface &surface, int x, int y) const;
};

} // End of namespace BladeRunner

#endif
