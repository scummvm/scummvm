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

#ifndef ULTIMA_ULTIMA1_WIDGETS_DUNGEON_ITEM_H
#define ULTIMA_ULTIMA1_WIDGETS_DUNGEON_ITEM_H

#include "ultima/ultima1/widgets/dungeon_widget.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {


/**
 * Encapsulated class for drawing widgets within dungeons
 */
class DungeonItem : public DungeonWidget {
protected:
	/**
	 * Constructor
	 */
	DungeonItem(Ultima1Game *game, Maps::MapBase *map, DungeonWidgetId widgetId, const Point &pt);

	/**
	 * Constructor
	 */
	DungeonItem(Ultima1Game *game, Maps::MapBase *map, DungeonWidgetId widgetId);
public:
	/**
	 * Try to open/unlock the item
	 * @returns		True if item was capable of being opened or unlocked
	 */
	virtual bool open() { return false; }
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
