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

#ifndef SCOOBY_INDEXED_FRAME_H
#define SCOOBY_INDEXED_FRAME_H

#include "common/scummsys.h"
#include "common/algorithm.h"

#include "common/array.h"

#include "graphics/screen.h"
#include "palette_color.h"
#include "tucker/graphics.h"

// Carries one backend-neutral 320x224 indexed frame and its 64-color palette.

namespace Scooby {

class IndexedFrame {
public:
	static const int Width = 320;
	static const int Height = 224;

	IndexedFrame(Graphics::Screen &screen) : _palette(64), _screen(screen) {
		_pixels.resize(static_cast<std::size_t>(Width) * static_cast<std::size_t>(Height), 0);
		Common::fill(_palette.begin(), _palette.end(), PaletteColor());
	}

	// Gets the palette index selected for every logical pixel.
	Common::Array<uint8> _pixels;

	// Gets the colors available to the indexed pixels in this frame.
	Common::Array<PaletteColor> _palette;

	Graphics::Screen &_screen;
private:
};
} // namespace Scooby

#endif // SCOOBY_INDEXED_FRAME_H
