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

#include "ultima8/misc/pent_include.h"

#include "ultima8/graphics/fonts/ttf_rendered_text.h"
#include "ultima8/graphics/fonts/tt_font.h"
#include "ultima8/graphics/render_surface.h"
#include "ultima8/graphics/texture.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TTFRenderedText, RenderedText)


TTFRenderedText::TTFRenderedText(Texture *texture_, int width_, int height_,
                                 int vlead_, TTFont *font_)
	: texture(texture_), font(font_) {
	width = width_;
	height = height_;
	vlead = vlead_;
}

TTFRenderedText::~TTFRenderedText() {
	delete texture;
}

void TTFRenderedText::draw(RenderSurface *surface, int x, int y,
                           bool destmasked) {
	if (!destmasked)
		surface->Blit(texture, 0, 0, width, height, x, y - font->getBaseline(),
		              font->isAntialiased());
	else
		surface->MaskedBlit(texture, 0, 0, width, height,
		                    x, y - font->getBaseline(),
		                    0, font->isAntialiased());
}

void TTFRenderedText::drawBlended(RenderSurface *surface, int x, int y,
                                  uint32 col, bool destmasked) {
	if (!destmasked)
		surface->FadedBlit(texture, 0, 0, width, height,
		                   x, y - font->getBaseline(), col,
		                   font->isAntialiased());
	else
		surface->MaskedBlit(texture, 0, 0, width, height,
		                    x, y - font->getBaseline(), col,
		                    font->isAntialiased());
}

} // End of namespace Ultima8
