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
#include "ultima/ultima8/graphics/fonts/ttf_rendered_text.h"
#include "ultima/ultima8/graphics/fonts/tt_font.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/texture.h"

namespace Ultima {
namespace Ultima8 {

TTFRenderedText::TTFRenderedText(Graphics::ManagedSurface *texture, int width, int height,
		int vLead, TTFont *font) : _texture(texture), _font(font) {
	_width = width;
	_height = height;
	_vLead = vLead;
}

TTFRenderedText::~TTFRenderedText() {
	delete _texture;
}

void TTFRenderedText::draw(RenderSurface *surface, int x, int y, bool destmasked) {
	if (!destmasked)
		surface->Blit(_texture, 0, 0, _width, _height, x, y - _font->getBaseline(),
			_font->isAntialiased());
	else
		surface->MaskedBlit(_texture, 0, 0, _width, _height,
			x, y - _font->getBaseline(), 0, _font->isAntialiased());
}

void TTFRenderedText::drawBlended(RenderSurface *surface, int x, int y,
		uint32 col, bool destmasked) {
	if (!destmasked)
		surface->FadedBlit(_texture, 0, 0, _width, _height,
			x, y - _font->getBaseline(), col, _font->isAntialiased());
	else
		surface->MaskedBlit(_texture, 0, 0, _width, _height,
			x, y - _font->getBaseline(), col, _font->isAntialiased());
}

} // End of namespace Ultima8
} // End of namespace Ultima
