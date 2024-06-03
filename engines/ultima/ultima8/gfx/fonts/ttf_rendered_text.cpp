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

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/gfx/fonts/ttf_rendered_text.h"
#include "ultima/ultima8/gfx/fonts/tt_font.h"
#include "ultima/ultima8/gfx/render_surface.h"

namespace Ultima {
namespace Ultima8 {

TTFRenderedText::TTFRenderedText(Graphics::ManagedSurface *texture, int width, int height,
		int vLead, int baseline, bool antiAliased) : _texture(texture), _baseline(baseline), _antiAliased(antiAliased) {
	_width = width;
	_height = height;
	_vLead = vLead;
}

TTFRenderedText::~TTFRenderedText() {
	delete _texture;
}

void TTFRenderedText::draw(RenderSurface *surface, int x, int y, bool destmasked) {
	if (!_width)
		return;
	Common::Rect srcRect(_width, _height);
	if (!destmasked)
		surface->Blit(*_texture, srcRect, x, y - _baseline, _antiAliased);
	else
		surface->MaskedBlit(*_texture, srcRect, x, y - _baseline, 0, _antiAliased);
}

void TTFRenderedText::drawBlended(RenderSurface *surface, int x, int y,
		uint32 col, bool destmasked) {
	Common::Rect srcRect(_width, _height);
	if (!destmasked)
		surface->FadedBlit(*_texture, srcRect, x, y - _baseline, col, _antiAliased);
	else
		surface->MaskedBlit(*_texture, srcRect, x, y - _baseline, col, _antiAliased);
}

} // End of namespace Ultima8
} // End of namespace Ultima
