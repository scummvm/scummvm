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

#ifndef MM1_GFX_H
#define MM1_GFX_H

namespace MM {
namespace MM1 {
namespace Gfx {

#define SCREEN_W 320
#define SCREEN_H 200
#define EGA_PALETTE_COUNT 16

extern byte EGA_INDEXES[EGA_PALETTE_COUNT];

class GFX {
public:
	/**
	 * Sets the EGA palette
	 */
	static void setEgaPalette();

	/**
	 * Called after the Xeen palette has been loaded, to determine
	 * which palette indexes most closely match the EGA colors
	 */
	static void findPalette(const byte palette[256 * 3]);
};

} // namespace Gfx
} // namespace MM1
} // namespace MM

#endif
