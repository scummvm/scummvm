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

DEFINE_RUNTIME_CLASSTYPE_CODE(JPRenderedText, RenderedText)


JPRenderedText::JPRenderedText(std::list<PositionedText> &lines_,
                               int width_, int height_, int vlead_,
                               ShapeFont *font_, unsigned int fontnum_)
	: lines(lines_), font(font_), fontnum(fontnum_) {
	width = width_;
	height = height_;
	vlead = vlead_;
}

JPRenderedText::~JPRenderedText() {

}

void JPRenderedText::draw(RenderSurface *surface, int x, int y, bool /*destmasked*/) {
	// TODO support masking here??

	PaletteManager *palman = PaletteManager::get_instance();
	PaletteManager::PalIndex fontpal = static_cast<PaletteManager::PalIndex>
	                                   (PaletteManager::Pal_JPFontStart + fontnum);
	Pentagram::Palette *pal = palman->getPalette(fontpal);
	const Pentagram::Palette *savepal = font->getPalette();
	font->setPalette(pal);

	std::list<PositionedText>::iterator iter;

	for (iter = lines.begin(); iter != lines.end(); ++iter) {
		int line_x = x + iter->dims.x;
		int line_y = y + iter->dims.y;

		size_t textsize = iter->text.size();

		for (size_t i = 0; i < textsize; ++i) {
			uint16 sjis = iter->text[i] & 0xFF;
			if (sjis >= 0x80) {
				uint16 t = iter->text[++i] & 0xFF;
				sjis += (t << 8);
			}
			uint16 u8char = Pentagram::shiftjis_to_ultima8(sjis);
			surface->Paint(font, u8char, line_x, line_y);

			if (i == iter->cursor) {
				surface->Fill32(0xFF000000, line_x, line_y - font->getBaseline(),
				                1, iter->dims.h);
			}

			line_x += (font->getFrame(u8char))->width - font->getHlead();
		}

		if (iter->cursor == textsize) {
			surface->Fill32(0xFF000000, line_x, line_y - font->getBaseline(),
			                1, iter->dims.h);
		}
	}

	font->setPalette(savepal);
}

void JPRenderedText::drawBlended(RenderSurface *surface, int x, int y,
                                 uint32 col, bool /*destmasked*/) {
	// TODO Support masking here??

	PaletteManager *palman = PaletteManager::get_instance();
	PaletteManager::PalIndex fontpal = static_cast<PaletteManager::PalIndex>
	                                   (PaletteManager::Pal_JPFontStart + fontnum);
	Pentagram::Palette *pal = palman->getPalette(fontpal);
	const Pentagram::Palette *savepal = font->getPalette();
	font->setPalette(pal);

	std::list<PositionedText>::iterator iter;

	for (iter = lines.begin(); iter != lines.end(); ++iter) {
		int line_x = x + iter->dims.x;
		int line_y = y + iter->dims.y;

		size_t textsize = iter->text.size();

		for (size_t i = 0; i < textsize; ++i) {
			uint16 sjis = iter->text[i] & 0xFF;
			if (sjis >= 0x80) {
				uint16 t = iter->text[++i] & 0xFF;
				sjis += (t << 8);
			}
			uint16 u8char = Pentagram::shiftjis_to_ultima8(sjis);

			surface->PaintHighlight(font, u8char, line_x, line_y,
			                        false, false, col);
			line_x += (font->getFrame(u8char))->width - font->getHlead();
		}

	}

	font->setPalette(savepal);
}

} // End of namespace Ultima8
} // End of namespace Ultima
