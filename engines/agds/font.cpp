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

#include "agds/font.h"
#include "graphics/managed_surface.h"

namespace AGDS {

Font::Font(Graphics::ManagedSurface *surface, int gw, int gh) : _surface(surface),
																_glyphW(gw), _glyphH(gh),
																_cellW(surface->w / 16), _cellH(surface->h / 16) {

	// debug("surface cell %dx%d", _cellW, _cellH);
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 16; ++x) {
			const uint32 *pixels = static_cast<uint32 *>(_surface->getBasePtr(x * _cellW, y * _cellH));
			int w;
			int ch = (y << 4) | x;
			for (w = 0; w <= _glyphW; ++w, ++pixels) {
				uint8 r, g, b, a;
				// debug("%d color #%08x", ch, *pixels);
				surface->format.colorToARGB(*pixels, r, g, b, a);
				// debug("%d %d %d %d", r, g, b, a);
				if (r == 0) // fixme: mapped incorrectly
					break;
			}
			_width[ch] = w;
		}
	}
}

void Font::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (chr >= 0x100)
		return;

	Common::Rect srcRect(getCharWidth(chr), _glyphH);
	srcRect.moveTo(_cellW * (chr & 0x0f), _cellH * (chr >> 4));
	if (!srcRect.isEmpty())
		_surface->blendBlitTo(*dst, x, y, Graphics::FLIP_NONE, &srcRect);
}

} // namespace AGDS
