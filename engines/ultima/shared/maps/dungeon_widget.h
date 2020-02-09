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

#ifndef ULTIMA_SHARED_MAPS_DUNGEON_WIDGET_H
#define ULTIMA_SHARED_MAPS_DUNGEON_WIDGET_H

#include "ultima/shared/maps/map_widget.h"
#include "ultima/shared/maps/map.h"
#include "ultima/shared/gfx/dungeon_surface.h"
#include "common/serializer.h"
#include "common/str.h"

namespace Ultima {
namespace Shared {

class Game;
class Map;

namespace Maps {

/**
 * Base class for things that appear within the dungeons
 */
class DungeonWidget : public MapWidget {
public:
	/**
	 * Constructor
	 */
	DungeonWidget(Game *game, Maps::MapBase *map) : MapWidget(game, map) {}
	DungeonWidget(Game *game, Maps::MapBase *map, const Point &pt, Direction dir = DIR_NONE) : MapWidget(game, map, pt, dir) {}
	DungeonWidget(Game *game, Maps::MapBase *map, const Common::String &name, const Point &pt, Direction dir = DIR_NONE) :
		MapWidget(game, map, name, pt, dir) {}

	/**
	 * Destructor
	 */
	~DungeonWidget() override {}

	/**
	 * Draws an item
	 */
	virtual void draw(DungeonSurface &s, uint distance) = 0;
};

} // End of namespace Maps
} // End of namespace Shared
} // End of namespace Ultima

#endif
