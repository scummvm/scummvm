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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/gfx/fonts/shape_rendered_text.h"

#include "ultima/ultima8/gfx/fonts/shape_font.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/texture.h"

namespace Ultima {
namespace Ultima8 {

ShapeRenderedText::ShapeRenderedText(const Std::list<PositionedText> &lines,
									 int width, int height, int vLead,
									 ShapeFont *font)
	: _lines(lines), _font(font) {
	_width = width;
	_height = height;
	_vLead = vLead;
}

ShapeRenderedText::~ShapeRenderedText() {
}

void ShapeRenderedText::draw(RenderSurface *surface, int x, int y, bool /*destmasked*/) {
	// TODO support masking here???

	uint32 color = TEX32_PACK_RGB(0, 0, 0);
	Std::list<PositionedText>::const_iterator iter;

	surface->BeginPainting();

	for (const auto &line : _lines) {
		int line_x = x + line._dims.left;
		int line_y = y + line._dims.top;

		size_t textsize = line._text.size();

		for (size_t i = 0; i < textsize; ++i) {
			surface->Paint(_font, _font->charToFrameNum(line._text[i]),
			               line_x, line_y);

			if (i == line._cursor) {
				surface->fill32(color, line_x, line_y - _font->getBaseline(),
				                1, line._dims.height());
			}

			line_x += _font->getWidth(line._text[i]) - _font->getHlead();
		}

		if (line._cursor == textsize) {
			surface->fill32(color, line_x, line_y - _font->getBaseline(),
			                1, line._dims.height());
		}
	}

	surface->EndPainting();
}

void ShapeRenderedText::drawBlended(RenderSurface *surface, int x, int y,
									uint32 col, bool /*destmasked*/) {
	// TODO Support masking here ????

	Std::list<PositionedText>::const_iterator iter;

	for (const auto &line : _lines) {
		int line_x = x + line._dims.left;
		int line_y = y + line._dims.top;

		size_t textsize = line._text.size();

		for (size_t i = 0; i < textsize; ++i) {
			surface->PaintHighlight(_font,
			                        static_cast<unsigned char>(line._text[i]),
			                        line_x, line_y, false, false, col);
			line_x += _font->getWidth(line._text[i]) - _font->getHlead();
		}

	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
