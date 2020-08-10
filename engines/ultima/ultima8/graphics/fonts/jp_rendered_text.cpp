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
#include "ultima/ultima8/graphics/fonts/jp_rendered_text.h"
#include "ultima/ultima8/graphics/fonts/shape_font.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/misc/encoding.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/palette_manager.h"

namespace Ultima {
namespace Ultima8 {

JPRenderedText::JPRenderedText(Std::list<PositionedText> &lines, int width, int height,
		int vLead, ShapeFont *font, unsigned int fontNum)
		: _lines(lines), _font(font), _fontNum(fontNum) {
	_width = width;
	_height = height;
	_vLead = vLead;
}

JPRenderedText::~JPRenderedText() {
}

void JPRenderedText::draw(RenderSurface *surface, int x, int y, bool /*destmasked*/) {
	// TODO support masking here??

	PaletteManager *palman = PaletteManager::get_instance();
	PaletteManager::PalIndex fontpal = static_cast<PaletteManager::PalIndex>
	                                   (PaletteManager::Pal_JPFontStart + _fontNum);
	Palette *pal = palman->getPalette(fontpal);
	const Palette *savepal = _font->getPalette();
	_font->setPalette(pal);

	Std::list<PositionedText>::const_iterator iter;

	for (iter = _lines.begin(); iter != _lines.end(); ++iter) {
		int line_x = x + iter->_dims.left;
		int line_y = y + iter->_dims.top;

		size_t textsize = iter->_text.size();

		for (size_t i = 0; i < textsize; ++i) {
			uint16 sjis = iter->_text[i] & 0xFF;
			if (sjis >= 0x80) {
				uint16 t = iter->_text[++i] & 0xFF;
				sjis += (t << 8);
			}
			uint16 u8char = shiftjis_to_ultima8(sjis);
			surface->Paint(_font, u8char, line_x, line_y);

			if (i == iter->_cursor) {
				surface->Fill32(0xFF000000, line_x, line_y - _font->getBaseline(),
				                1, iter->_dims.height());
			}

			line_x += (_font->getFrame(u8char))->_width - _font->getHlead();
		}

		if (iter->_cursor == textsize) {
			surface->Fill32(0xFF000000, line_x, line_y - _font->getBaseline(),
			                1, iter->_dims.height());
		}
	}

	_font->setPalette(savepal);
}

void JPRenderedText::drawBlended(RenderSurface *surface, int x, int y,
                                 uint32 col, bool /*destmasked*/) {
	// TODO Support masking here??

	PaletteManager *palman = PaletteManager::get_instance();
	PaletteManager::PalIndex fontpal = static_cast<PaletteManager::PalIndex>
	                                   (PaletteManager::Pal_JPFontStart + _fontNum);
	Palette *pal = palman->getPalette(fontpal);
	const Palette *savepal = _font->getPalette();
	_font->setPalette(pal);

	Std::list<PositionedText>::const_iterator iter;

	for (iter = _lines.begin(); iter != _lines.end(); ++iter) {
		int line_x = x + iter->_dims.left;
		int line_y = y + iter->_dims.top;

		size_t textsize = iter->_text.size();

		for (size_t i = 0; i < textsize; ++i) {
			uint16 sjis = iter->_text[i] & 0xFF;
			if (sjis >= 0x80) {
				uint16 t = iter->_text[++i] & 0xFF;
				sjis += (t << 8);
			}
			uint16 u8char = shiftjis_to_ultima8(sjis);

			surface->PaintHighlight(_font, u8char, line_x, line_y,
			                        false, false, col);
			line_x += (_font->getFrame(u8char))->_width - _font->getHlead();
		}

	}

	_font->setPalette(savepal);
}

} // End of namespace Ultima8
} // End of namespace Ultima
