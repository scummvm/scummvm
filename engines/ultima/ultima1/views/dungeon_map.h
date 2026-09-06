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

#ifndef ULTIMA1_VIEWS_DUNGEON_MAP_H
#define ULTIMA1_VIEWS_DUNGEON_MAP_H

#include "ultima/ultima1/views/dialog.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/map_dungeon.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Shared::Messages;

class DungeonMap : public Dialog {
private:
	/**
	 * Returns the cell at a given position, or a solid wall cell if the
	 * position is outside the dungeon's bounds
	 */
	Data::DungeonCell cellAt(int x, int y) const;

	/**
	 * Returns the X/Y delta of a single step in the direction the player
	 * is currently facing
	 */
	int getDirDeltaX() const;
	int getDirDeltaY() const;

	/**
	 * Returns true if a monster occupying a cell should block the view -
	 * disguised/see-through monster types don't
	 */
	bool isMonsterBlocking(int x, int y) const;

	/**
	 * Returns true if the cell a given offset from the player blocks the
	 * view, either because it's solid or because a monster is there
	 */
	bool isCellOccupied(int deltaX, int deltaY) const;

	/**
	 * Returns how many tiles ahead, in a given direction, the view extends
	 * before it's blocked, capped at 5
	 */
	int distanceToOccupiedCell(int deltaX, int deltaY) const;

	/**
	 * Returns true for tiles that block movement/view (walls, doors,
	 * secret doors) - false for hallways, ladders, and support beams
	 */
	bool isSolidTile(int tileNum) const;

	/**
	 * Shows the direction the player is currently facing at the top of
	 * the view
	 */
	void drawFacingDirection();

	/**
	 * Draws the wall directly facing the player at a given distance
	 */
	void drawWall(int distance);

	/**
	 * Draws the receding side wall connecting distance-1 to distance, on
	 * the left/right
	 */
	void drawLeftWall(int distance);
	void drawRightWall(int distance);

	/**
	 * Draws the open threshold framing a side passage at a given distance,
	 * on the left/right
	 */
	void drawLeftBlank(int distance);
	void drawRightBlank(int distance);

	/**
	 * Draws the vertical edge marking where a side wall starts/stops at a
	 * given distance, on the left/right
	 */
	void drawLeftEdge(int distance);
	void drawRightEdge(int distance);

	/**
	 * Draws a doorway set into the left/right flanking wall at a given
	 * distance
	 */
	void drawLeftDoor(int distance);
	void drawRightDoor(int distance);

	/**
	 * Draws the doorway directly facing the player at a given distance
	 */
	void drawDoorway(int distance);

	/**
	 * Draws a row of horizontal ceiling support beams across the passage
	 * at a given distance
	 */
	void drawBeams(int distance);

	/**
	 * Draws a ladder (its "hole" opening plus shaft) at a given distance.
	 * The face-on variants are used when facing up/down the dungeon grid,
	 * the sideways variants when facing left/right, since the ladder's
	 * shaft is only visible face-on
	 */
	void drawLadderUpFaceOn(int distance);
	void drawLadderDownFaceOn(int distance);
	void drawLadderUpSideways(int distance);
	void drawLadderDownSideways(int distance);

	/**
	 * Draws the left/right flanking cell at a given distance, based on its
	 * tile type
	 */
	void drawLeftCell(int distance, int tileNum);
	void drawRightCell(int distance, int tileNum);

	/**
	 * Draws the cell directly ahead at a given distance
	 */
	void dungeonDrawCell(int distance, int x, int y);

	/**
	 * Draws a monster/item widget shape - a small polyline figure made up
	 * of moveto/lineto segments with coordinates relative to screen
	 * center, scaled down the further away it is
	 */
	void dungeonDrawWidget(const uint8 *data, int distance, int color);

	/**
	 * Draws whichever monster occupies a cell. The gelatinous cube fills
	 * the whole passage instead of using a widget shape
	 */
	void dungeonDrawMonster(int monsterId, int distance);

	/**
	 * Draws a coffin or chest widget at a given distance. In the original
	 * these (writeCoffin/writeChest) also print the item's name in the
	 * commands area - that's left for DungeonLogic, as with monster names
	 */
	void drawCoffin(int distance);
	void drawChest(int distance);

	/**
	 * Draws the full first-person dungeon corridor view
	 */
	void drawDungeonView();

public:
	DungeonMap() : Dialog("DungeonMap") {}
	~DungeonMap() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
