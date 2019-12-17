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

DEFINE_RUNTIME_CLASSTYPE_CODE(JPFont, Pentagram::Font)


JPFont::JPFont(ShapeFont *jpfont, unsigned int fontnum_)
	: fontnum(fontnum_), shapefont(jpfont) {
	assert(shapefont->frameCount() > 256);
}


JPFont::~JPFont() {

}

int JPFont::getWidth(int c) {
	return shapefont->getFrame(c)->width;
}

int JPFont::getHeight() {
	return shapefont->getHeight();
}

int JPFont::getBaseline() {
	return shapefont->getBaseline();
}

int JPFont::getBaselineSkip() {
	return shapefont->getBaselineSkip();
}


void JPFont::getStringSize(const std::string &text, int &width, int &height) {
	int hlead = shapefont->getHlead();
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
			width += getWidth(Pentagram::shiftjis_to_ultima8(sjis)) - hlead;
		}
	}
}

void JPFont::getTextSize(const std::string &text,
                         int &resultwidth, int &resultheight,
                         unsigned int &remaining,
                         int width, int height, TextAlign align,
                         bool u8specials) {
	std::list<PositionedText> tmp;
	tmp = typesetText<SJISTraits>(this, text, remaining,
	                              width, height, align, u8specials,
	                              resultwidth, resultheight);
}

RenderedText *JPFont::renderText(const std::string &text,
                                 unsigned int &remaining,
                                 int width, int height, TextAlign align,
                                 bool u8specials,
                                 std::string::size_type cursor) {
	int resultwidth, resultheight;
	std::list<PositionedText> lines;
	lines = typesetText<SJISTraits>(this, text, remaining,
	                                width, height, align, u8specials,
	                                resultwidth, resultheight,
	                                cursor);

	return new JPRenderedText(lines, resultwidth, resultheight,
	                          shapefont->getVlead(), shapefont, fontnum);
}

} // End of namespace Ultima8
} // End of namespace Ultima
