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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/fonts/jp_font.h"
#include "ultima/ultima8/graphics/fonts/shape_font.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/fonts/jp_rendered_text.h"
#include "ultima/ultima8/misc/encoding.h"

namespace Ultima {
namespace Ultima8 {

JPFont::JPFont(ShapeFont *jpfont, unsigned int fontnum)
	: _fontNum(fontnum), _shapeFont(jpfont) {
	assert(_shapeFont->frameCount() > 256);
}


JPFont::~JPFont() {
}

int JPFont::getWidth(int c) {
	return _shapeFont->getFrame(c)->_width;
}

int JPFont::getHeight() {
	return _shapeFont->getHeight();
}

int JPFont::getBaseline() {
	return _shapeFont->getBaseline();
}

int JPFont::getBaselineSkip() {
	return _shapeFont->getBaselineSkip();
}


void JPFont::getStringSize(const Std::string &text, int32 &width, int32 &height) {
	int hlead = _shapeFont->getHlead();
	width = hlead;
	height = getHeight();

	for (unsigned int i = 0; i < text.size(); ++i) {
		if (text[i] == '\n' || text[i] == '\r') {
			// ignore
		} else {
			uint16 sjis = text[i] & 0xFF;
			if (sjis >= 0x80) {
				uint16 t = text[++i] & 0xFF;
				sjis += (t << 8);
			}
			width += getWidth(shiftjis_to_ultima8(sjis)) - hlead;
		}
	}
}

void JPFont::getTextSize(const Std::string &text,
                         int32 &resultwidth, int32 &resultheight,
                         unsigned int &remaining,
                         int32 width, int32 height, TextAlign align,
                         bool u8specials) {
	Std::list<PositionedText> tmp;
	tmp = typesetText<SJISTraits>(this, text, remaining,
	                              width, height, align, u8specials,
	                              resultwidth, resultheight);
}

RenderedText *JPFont::renderText(const Std::string &text,
                                 unsigned int &remaining,
                                 int32 width, int32 height, TextAlign align,
                                 bool u8specials,
                                 Std::string::size_type cursor) {
	int32 resultwidth, resultheight;
	Std::list<PositionedText> lines;
	lines = typesetText<SJISTraits>(this, text, remaining,
	                                width, height, align, u8specials,
	                                resultwidth, resultheight,
	                                cursor);

	return new JPRenderedText(lines, resultwidth, resultheight,
	                          _shapeFont->getVlead(), _shapeFont, _fontNum);
}

} // End of namespace Ultima8
} // End of namespace Ultima
