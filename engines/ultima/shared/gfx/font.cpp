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

#include "ultima/shared/gfx/font.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/palette.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

Font::Font() {
}

void Font::writeChar(Graphics::ManagedSurface &surface, Common::Point &textPos, char c) {
	// TODO
}

void Font::writeString(Graphics::ManagedSurface &surface, Common::Point &textPos, const Common::String &msg) {
	// TODO
}

uint Font::charWidth(char c) const {
	// TODO
	return 0;
}

uint Font::stringWidth(const Common::String &msg) const {
	// TODO
	return 0;
}

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
