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

typedef void(*DrawWidgetFn)(Graphics::ManagedSurface &s, uint widgetId, uint distance, byte color);

class Game;

/**
 * Acts as a handy container for the drawing methods for rendering the dungeon view
 */
class DungeonSurface : public Gfx::VisualSurface {
private:
	Point _penPos;
	byte _edgeColor;
	byte _highlightColor;
	byte _widgetColor;
	DrawWidgetFn _widgetFn;
public:
	/**
	 * Constructor
	 */
	DungeonSurface(const Graphics::ManagedSurface &src, const Rect &bounds, Game *game, DrawWidgetFn widgetFn);

	/**
	 * Draw a line
	 */
	void drawLine(int x0, int y0, int x1, int y1, uint32 color) {
		Gfx::VisualSurface::drawLine(x0 - 8, y0 - 8, x1 - 8, y1 - 8, color);
		_penPos = Point(x1, y1);
	}

	/**
	 * Draw a line from a prior line ending point to a new destination pos
	 */
	void drawLineTo(int x, int y, uint32 color) {
		Gfx::VisualSurface::drawLine(_penPos.x - 8, _penPos.y - 8, x - 8, y - 8, color);
		_penPos = Point(x, y);
	}

	/**
	 * Draw a horizontal line.
	 */
	void hLine(int x, int y, int x2, uint32 color) {
		Gfx::VisualSurface::hLine(x - 8, y - 8, x2 - 8, color);
		_penPos = Point(x2, y);
	}

	/**
	 * Draw a vertical line.
	 */
	void vLine(int x, int y, int y2, uint32 color) {
		Gfx::VisualSurface::vLine(x - 8, y - 8, y2 - 8, color);
		_penPos = Point(x, y2);
	}

	/**
	 * Draw a frame around a specified rect.
	 */
	void frameRect(const Common::Rect &r, uint32 color) {
		Gfx::VisualSurface::frameRect(Rect(r.left - 8, r.top - 8, r.right - 8, r.bottom - 8), color);
	}


	/**
	 * Draws a wall
	 */
	void drawWall(uint distance);

	/**
	 * Draws a doorway directly in front of the player
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
	 * Draws a monster or item at a given distance from the player
	 */
	void drawWidget(uint widgetId, uint distance, byte color);

	/**
	 * Draw a ladder down face on
	 */
	void drawLadderDownFaceOn(uint distance);

	/**
	 * Draw a ladder down side on
	 */
	void drawLadderDownSideOn(uint distance);

	/**
	 * Draw a ladder down face on
	 */
	void drawLadderUpFaceOn(uint distance);

	/**
	 * Draw a ladder down side on
	 */
	void drawLadderUpSideOn(uint distance);

	/**
	 * Draw beams
	 */
	void drawBeams(uint distance);

	/**
	 * Draws a door on the left hand side
	 */
	void drawLeftDoor(uint distance);

	/**
	 * Draws a wall on the left-hand side
	 */
	void drawLeftWall(uint distance);

	/**
	 * Draws the partial wall visible at the back of a corridor leading to the left
	 */
	void drawLeftBlank(uint distance);

	/**
	 * Draws a door on the right hand side
	 */
	void drawRightDoor(uint distance);

	/**
	 * Draws a wall on the right-hand side
	 */
	void drawRightWall(uint distance);

	/**
	 * Draws the partial wall visible at the back of a corridor leading to the right
	 */
	void drawRightBlank(uint distance);
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
