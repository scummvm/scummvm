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

#ifndef ULTIMA_SHARED_GFX_DUNGEON_H
#define ULTIMA_SHARED_GFX_DUNGEON_H

#include "ultima/shared/gfx/visual_surface.h"

namespace Ultima {
namespace Shared {

class Game;

/**
 * Acts as a handy container for the drawing methods for rendering the dungeon view
 */
class DungeonSurface : public Gfx::VisualSurface {
private:
	Game *_game;
	byte _edgeColor;
public:
	/**
	 * Constructor
	 */
	DungeonSurface(const Graphics::ManagedSurface &src, const Rect &bounds, Game *game);

	/**
	 * Draws a wall
	 */
	void drawWall(uint distance);

	/**
	 * Draws a doorway
	 */
	void drawDoorway(uint distance);

	/**
	 * Draws a vertical line forming the edge of cells to the left of the player
	 */
	void drawLeftEdge(uint distance);

	/**
	 * Draws a vertical line forming the edge of cells to the right of the player
	 */
	void drawRightEdge(uint distance);

	/**
	 * Draw a monster or, failing that, the given tile at a given cell and distance
	 */
	void drawCell(uint distance, const Point &pt);

};

} // End of namespace Shared
} // End of namespace Xeen

#endif
