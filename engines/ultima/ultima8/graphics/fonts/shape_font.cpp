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
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/graphics/fonts/shape_font.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/fonts/shape_rendered_text.h"

namespace Ultima {
namespace Ultima8 {

ShapeFont::ShapeFont(const uint8 *data, uint32 size,
                     const ConvertShapeFormat *format,
                     const uint16 flexId, const uint32 shapeNum)
	: Font(), Shape(data, size, format, flexId, shapeNum),
	  _height(0), _baseLine(0), _vLead(-1), _hLead(0) {
	_crusaderCharMap = GAME_IS_CRUSADER && shapeNum == 1;
}

ShapeFont::~ShapeFont() {
}


int ShapeFont::getWidth(char c) {
	const ShapeFrame *frame = getFrame(charToFrameNum(c));
	if (frame)
		return frame->_width;
	else
		return 7; // small space..
}

int ShapeFont::getHeight() {
	if (_height == 0) {
		for (uint32 i = 0; i < frameCount(); i++) {
			const ShapeFrame *frame = getFrame(i);
			if (!frame)
				continue;
			int h = frame->_height;

			if (h > _height) _height = h;
		}
	}

	return _height;
}

int ShapeFont::getBaseline() {
	if (_baseLine == 0) {
		for (uint32 i = 0; i < frameCount(); i++) {
			int b = getFrame(i)->_yoff;

			if (b > _baseLine) _baseLine = b;
		}
	}

	return _baseLine;
}

int ShapeFont::getBaselineSkip() {
	return getHeight() + getVlead();
}

void ShapeFont::getStringSize(const Std::string &text, int32 &width, int32 &height) {
	width = _hLead;
	height = getHeight();

	for (unsigned int i = 0; i < text.size(); ++i) {
		if (text[i] == '\n' || text[i] == '\r') {
			// ignore
		} else {
			width += getWidth(text[i]) - _hLead;
		}
	}
}

int ShapeFont::charToFrameNum(char c) const {
	if (_crusaderCharMap) {
		if (c < 41)
			// ( and ) are combined into a single shape
			return c;
		// weirdly X and Y are swapped in both upper and lowercase
		else if (c == 'X')
			return 'X';
		else if (c == 'Y')
			return 'W';
		else if (c < 96)
			return c - 1;
		else if (c == 96)
			// no backquote char
			return charToFrameNum('\'');
		else if (c == 'x')
			return 'w';
		else if (c == 'y')
			return 'v';
		else
			return c - 2;
	} else {
		return static_cast<unsigned char>(c);
	}
}

RenderedText *ShapeFont::renderText(const Std::string &text,
                                    unsigned int &remaining,
                                    int32 width, int32 height, TextAlign align,
                                    bool u8specials,
                                    Std::string::size_type cursor) {
	int32 resultwidth, resultheight;
	Std::list<PositionedText> lines;
	lines = typesetText<Traits>(this, text, remaining,
	                            width, height, align, u8specials,
	                            resultwidth, resultheight, cursor);

	return new ShapeRenderedText(lines, resultwidth, resultheight,
	                             getVlead(), this);
}

} // End of namespace Ultima8
} // End of namespace Ultima
