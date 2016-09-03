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
 *
 */

#ifndef ZVISION_TEXT_H
#define ZVISION_TEXT_H

#include "zvision/text/truetype_font.h"
#include "zvision/zvision.h"

namespace ZVision {

class ZVision;

enum TextJustification {
	TEXT_JUSTIFY_CENTER = 0,
	TEXT_JUSTIFY_LEFT = 1,
	TEXT_JUSTIFY_RIGHT = 2
};

enum TextChange {
	TEXT_CHANGE_NONE = 0x0,
	TEXT_CHANGE_FONT_TYPE = 0x1,
	TEXT_CHANGE_FONT_STYLE = 0x2,
	TEXT_CHANGE_NEWLINE = 0x4,
	TEXT_CHANGE_HAS_STATE_BOX = 0x8
};

class TextStyleState {
public:
	TextStyleState();
	TextChange parseStyle(const Common::String &str, int16 len);
	void readAllStyles(const Common::String &txt);
	void updateFontWithTextState(StyledTTFont &font);

	uint32 getTextColor(ZVision *engine) {
		return engine->_resourcePixelFormat.RGBToColor(_red, _green, _blue);
	}

public:
	Common::String _fontname;
	TextJustification _justification;
	int16 _size;
	uint8 _red;     // 0-255
	uint8 _green;   // 0-255
	uint8 _blue;    // 0-255
	bool _italic;
	bool _bold;
	bool _underline;
	bool _strikeout;
	int32 _statebox;
	bool _sharp;
};

class TextRenderer {
public:
	TextRenderer(ZVision *engine): _engine(engine) {};

	void drawTextWithJustification(const Common::String &text, StyledTTFont &font, uint32 color, Graphics::Surface &dest, int lineY, TextJustification jusification);
	int32 drawText(const Common::String &text, TextStyleState &state, Graphics::Surface &dest);
	void drawTextWithWordWrapping(const Common::String &text, Graphics::Surface &dest);

private:
	ZVision *_engine;
};

Common::String readWideLine(Common::SeekableReadStream &stream);
int8 getUtf8CharSize(char chr);
uint16 readUtf8Char(const char *chr);

} // End of namespace ZVision

#endif
