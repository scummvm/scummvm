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
 *
 */

#ifndef ZVISION_TEXT_H
#define ZVISION_TEXT_H

#include "zvision/detection.h"
#include "zvision/graphics/truetype_font.h"
#include "zvision/zvision.h"

namespace ZVision {

class ZVision;

enum txtJustify {
	TXT_JUSTIFY_CENTER = 0,
	TXT_JUSTIFY_LEFT = 1,
	TXT_JUSTIFY_RIGHT = 2
};

enum txtReturn {
	TXT_RET_NOTHING = 0x0,
	TXT_RET_FNTCHG = 0x1,
	TXT_RET_FNTSTL = 0x2,
	TXT_RET_NEWLN = 0x4,
	TXT_RET_HASSTBOX = 0x8
};

class cTxtStyle {
public:
	cTxtStyle();
	txtReturn parseStyle(const Common::String &strin, int16 len);
	void readAllStyle(const Common::String &txt);
	void setFontStyle(StyledTTFont &font);
	void setFont(StyledTTFont &font);

public:
	Common::String _fontname;
	txtJustify _justify;  // 0 - center, 1-left, 2-right
	int16 _size;
	uint8 _red;     // 0-255
	uint8 _green;   // 0-255
	uint8 _blue;    // 0-255
	int8 _newline;
	int8 _escapement;
	bool _italic;
	bool _bold;
	bool _underline;
	bool _strikeout;
	bool _skipcolor;
	int32 _statebox;
	bool _sharp;
	// char image ??
};

class TextRenderer {
public:
	TextRenderer(ZVision *engine): _engine(engine) {};

	void drawTxtWithJustify(const Common::String &txt, StyledTTFont &fnt, uint32 color, Graphics::Surface &dst, int lineY, txtJustify justify);
	int32 drawTxt(const Common::String &txt, cTxtStyle &fontStyle, Graphics::Surface &dst);
	Graphics::Surface *render(StyledTTFont &fnt, const Common::String &txt, cTxtStyle &style);
	void drawTxtInOneLine(const Common::String &txt, Graphics::Surface &dst);

private:
	ZVision *_engine;
};

Common::String readWideLine(Common::SeekableReadStream &stream);
int8 getUtf8CharSize(char chr);
uint16 readUtf8Char(const char *chr);

} // End of namespace ZVision

#endif
