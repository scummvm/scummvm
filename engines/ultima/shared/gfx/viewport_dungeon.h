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

#ifndef ULTIMA_SHARED_GFX_VIEWPORT_DUNGEON_H
#define ULTIMA_SHARED_GFX_VIEWPORT_DUNGEON_H

#include "ultima/shared/gfx/visual_item.h"
#include "ultima/shared/gfx/dungeon_surface.h"
#include "ultima/shared/maps/map_tile.h"

namespace Ultima {
namespace Shared {

class ViewportDungeon : public Gfx::VisualItem {
	DECLARE_MESSAGE_MAP;
private:
	/**
	 * Returns the distance to an occupied cell, if any
	 */
	uint distanceToOccupiedCell(const Point &delta);

	/**
	 * Returns if a cell at a given delta to the player is occupied
	 */
	bool isCellOccupied(const Point &delta);

	/**
	 * Returns true if a monster is at a given position, and it has the blocking attribute
	 */
	bool isMonsterBlocking(const Point &pt);

	/**
	 * Draw a monster or, failing that, the given tile at a given cell and distance
	 */
	void drawCell(uint distance, const Point &pt);

	/**
	 * Draw a cell to the left
	 */
	void drawLeftCell(uint distance, const Maps::MapTile &tile);

	/**
	 * Draw a cell to the left
	 */
	void drawRightCell(uint distance, const Maps::MapTile &tile);
protected:
	/**
	 * Returns the surface for rendering the dungeon
	 */
	virtual DungeonSurface getSurface() = 0;
public:
	CLASSDEF;
	ViewportDungeon(TreeItem *parent) : Gfx::VisualItem("ViewportDungeon", Rect(8, 8, 312, 152), parent) {}
	~ViewportDungeon() override {}

	/**
	 * Draws the dungeon
	 */
	void draw() override;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
