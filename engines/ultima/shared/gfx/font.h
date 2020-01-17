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

#ifndef ULTIMA_SHARED_GFX_FONT_H
#define ULTIMA_SHARED_GFX_FONT_H

#include "common/array.h"
#include "common/stream.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

class Font {
public:
	Font();

	/**
	 * Draw a character
	 */
	void writeChar(Graphics::ManagedSurface &surface, Common::Point &textPos, char c);

	/**
	 * Write out a string
	 */
	void writeString(Graphics::ManagedSurface &surface, Common::Point &textPos, const Common::String &msg);

	/**
	 * Return the width of a character
	 */
	uint charWidth(char c) const;

	/**
	 * Return the width of a string
	 */
	uint stringWidth(const Common::String &msg) const;

	/**
	 * Returns the height of the font
	 */
	uint lineHeight() const { return 8; }
};

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
