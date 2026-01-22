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

#include "fool/fool.h"
#include "fool/utils.h"

namespace Fool {

Graphics::ManagedSurface *createRemappedSurface(const Graphics::Surface *surface, const byte *palette, uint colorCount) {
	Graphics::ManagedSurface *s = new Graphics::ManagedSurface();
	s->create(surface->w, surface->h, Graphics::PixelFormat::createFormatCLUT8());

	byte paletteMap[256];
	memset(paletteMap, 0, sizeof(paletteMap));

	const byte monoPalette[] = {
		0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00
	};

	if (colorCount == 0) {
		colorCount = 2;
		palette = monoPalette;
	}

	for (uint i = 0; i < colorCount; i++) {
		int r = palette[3 * i];
		int g = palette[3 * i + 1];
		int b = palette[3 * i + 2];

		uint32 c;

		c = g_engine->_wm.findBestColor(r, g, b);
		paletteMap[i] = c;
	}

	// Colors outside the palette are not remapped.

	for (uint i = colorCount; i < 256; i++)
		paletteMap[i] = i;

	if (palette) {
		for (int y = 0; y < s->h; y++) {
			for (int x = 0; x < s->w; x++) {
				uint color = surface->getPixel(x, y);
				if (color > colorCount)
					color = g_engine->_wm._colorBlack;
				else
					color = paletteMap[color];

				s->setPixel(x, y, color);
			}
		}
	} else {
		s->copyFrom(*surface);
	}

	return s;
}



} // End of namespace Fool
