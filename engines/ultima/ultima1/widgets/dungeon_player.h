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

#ifndef ULTIMA_ULTIMA1_WIDGETS_DUNGEON_PLAYER_H
#define ULTIMA_ULTIMA1_WIDGETS_DUNGEON_PLAYER_H

#include "ultima/shared/maps/dungeon_widget.h"
#include "ultima/shared/maps/map_widget.h"
#include "ultima/shared/early/game.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

class DungeonPlayer : public Shared::Maps::DungeonWidget {
public:
	DECLARE_WIDGET(DungeonPlayer)

	/**
	 * Constructor
	 */
	DungeonPlayer(Shared::Game *game, Shared::Maps::MapBase *map) : Shared::Maps::DungeonWidget(game, map) {}
	DungeonPlayer(Shared::Game *game, Shared::Maps::MapBase *map, const Point &pt, Shared::Maps::Direction dir = Shared::Maps::DIR_NONE) : Shared::Maps::DungeonWidget(game, map, pt, dir) {}
	DungeonPlayer(Shared::Game *game, Shared::Maps::MapBase *map, const Common::String &name, const Point &pt, Shared::Maps::Direction dir = Shared::Maps::DIR_NONE) :
		Shared::Maps::DungeonWidget(game, map, name, pt, dir) {}

	/**
	 * Destructor
	 */
	~DungeonPlayer() override {}

	/**
	 * The player's viewpoint has no intrinsic drawing
	 */
	void draw(Shared::DungeonSurface &s, uint distance) override {}

	/**
	 * Returns true if the given widget can move to a given position on the map
	 */
	CanMove canMoveTo(const Point &destPos) override;
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif

