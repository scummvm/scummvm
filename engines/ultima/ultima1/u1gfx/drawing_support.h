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

#ifndef ULTIMA_ULTIMA1_U1GFX_DRAWING_SUPPORT_H
#define ULTIMA_ULTIMA1_U1GFX_DRAWING_SUPPORT_H

#include "ultima/shared/gfx/visual_surface.h"

namespace Ultima {
namespace Ultima1 {

class Ultima1Game;

namespace U1Gfx {

/**
 * Implements various support methods for drawing onto visual surfaces
 */
class DrawingSupport {
private:
	Shared::Gfx::VisualSurface _surface;
	Ultima1Game *_game;
private:
	/**
	 * Tweaks the edges of a drawn border to give it a rounded effect
	 */
	void roundFrameCorners(bool skipBottom = false);
public:
	/**
	 * Constructor
	 */
	DrawingSupport(const Shared::Gfx::VisualSurface &s);

	/**
	 * Draws a frame around the entire screen
	 */
	void drawFrame();

	/**
	 * Draw a frame around the viewport area of the screen, and a vertical seperator line
	 * to the bottom of the screen to separate the status and info areas
	 */
	void drawGameFrame();

	/**
	 * Draw a right arrow glyph
	 */
	void drawRightArrow(const Point &pt);

	/**
	 * Draw a left arrow glyph
	 */
	void drawLeftArrow(const Point &pt);
};

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
