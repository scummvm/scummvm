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

#ifndef ULTIMA_ULTIMA1_WIDGETS_DUNGEON_WIDGET_H
#define ULTIMA_ULTIMA1_WIDGETS_DUNGEON_WIDGET_H

#include "ultima/shared/maps/map.h"
#include "ultima/shared/maps/dungeon_widget.h"

namespace Ultima {
namespace Ultima1 {

class Ultima1Game;

namespace Maps {
class MapBase;
}

namespace Widgets {

enum DungeonWidgetId {
	MONSTER_NONE = -1, MONSTER_RANGER = 0, MONSTER_SKELETON = 1, MONSTER_THIEF = 2, MONSTER_GIANT_RAT = 3,
	MONSTER_RAT = 4, MONSTER_SPIDER = 5, MONSTER_VIPER = 6, MONSTER_ORC = 7, MONSTER_CYCLOPS = 8,
	MONSTER_GELATINOUS_CUBE = 9, MONSTER_ETTIN = 10, MONSTER_MIMIC = 11, MONSTER_LIZARD_MAN = 12,
	MONSTER_MINOTAUR = 13, MONSTER_CARRION_CREEPER = 14, MONSTER_TANGLER = 15, MONSTER_GREMLIN = 16,
	MONSTER_WANDERING_EYES = 17, MONSTER_WRAITH = 18, MONSTER_LICH = 19, MONSTER_INVISIBLE_SEEKER = 20,
	MONSTER_MIND_WHIPPER = 21, MONSTER_ZORN = 22, MONSTER_DAEMON = 23, MONSTER_BALRON = 24, UITEM_COFFIN = 25,
	UITEM_HOLE_UP = 26, UITEM_HOLE_DOWN = 27, UITEM_LADDER_UP = 28, UITEM_LADDER_DOWN = 29
};

/**
 * Encapsulated class for drawing widgets within dungeons
 */
class DungeonWidget : public Shared::Maps::DungeonWidget {
protected:
	DungeonWidgetId _widgetId;
private:
	/**
	 * Get the drawing data table
	 */
	static const byte *getData();

	/**
	 * Extracts a drawing position
	 */
	static void getPos(const byte *&data, int bitShift, Point &pt);
protected:
	/**
	 * Gets the Ultima 1 game
	 */
	Ultima1Game *getGame() const;

	/**
	 * Gets the Ultima 1 map
	 */
	Maps::MapBase *getMap() const;
public:
	Common::String _name;			// Name of item
public:
	/**
	 * Constructor
	 */
	DungeonWidget(Ultima1Game *game, Maps::MapBase *map, DungeonWidgetId widgetId, const Point &pt);

	/**
	 * Constructor
	 */
	DungeonWidget(Ultima1Game *game, Maps::MapBase *map);

	/**
	 * Draws a dungeon widget onto the passed surface
	 */
	static void drawWidget(Graphics::ManagedSurface &s, DungeonWidgetId widgetId, uint distance, byte color);

	/**
	 * Handles drawing the item
	 */
	void draw(Shared::DungeonSurface &s, uint distance) override;
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
