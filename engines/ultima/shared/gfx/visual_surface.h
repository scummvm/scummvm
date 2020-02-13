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

#ifndef ULTIMA_SHARED_GFX_VISUAL_SURFACE_H
#define ULTIMA_SHARED_GFX_VISUAL_SURFACE_H

#include "graphics/managed_surface.h"
#include "ultima/shared/core/named_item.h"
#include "ultima/shared/core/rect.h"
#include "ultima/shared/gfx/screen.h"

namespace Ultima {
namespace Shared {

class GameBase;
	
namespace Gfx {

class Font;

class VisualSurface : public Graphics::ManagedSurface {
private:
	Rect _bounds;
	Point _textPos;
	byte _textColor;
	byte _bgColor;
public:
	/**
	 * Constructor
	 */
	VisualSurface(const Graphics::ManagedSurface &src, const Rect &bounds, GameBase *game = nullptr);

	/**
	 * Draws a point on the surface
	 */
	void drawPoint(const Point &pt, byte color);

	/**
	 * Set the position for writing text to
	 */
	void setTextPos(const Point &pt) { _textPos = pt; }

	/**
	 * Write out a string
	 */
	void writeString(const Common::String &msg, const Point &pt, int color = -1, int bgColor = -1);

	/**
	 * Write out a string
	 */
	void writeString(const Common::String &msg, int color = -1, int bgColor = -1);

	/**
	 * Draw a character
	 */
	void writeChar(unsigned char c, const Point &pt, int color = -1, int bgColor = -1);

	/**
	 * Draw a character
	 */
	void writeChar(unsigned char c, int color = -1, int bgColor = -1);

	/**
	 * Get the current font height
	 */
	size_t fontHeight();
};

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
