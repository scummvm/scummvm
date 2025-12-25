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

#ifndef MM1_GFX_SYMBOLS_H
#define MM1_GFX_SYMBOLS_H

#include "graphics/managed_surface.h"

namespace MM {
namespace MM1 {
namespace Gfx {

#define MAX_SYMBOLS 20
#define SYMBOL_WIDTH 8
#define SYMBOL_HEIGHT 8

class Symbols {
private:
	Graphics::ManagedSurface _data[20];

public:
	void load();

	void draw(Graphics::ManagedSurface &dest, const Common::Point &destPos, int symbolNum);
};

} // namespace Gfx
} // namespace MM1
} // namespace MM

#endif
