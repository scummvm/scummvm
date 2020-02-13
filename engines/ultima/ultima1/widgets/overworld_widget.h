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

#ifndef ULTIMA_ULTIMA1_WIDGETS_OVERWORLD_WIDGET_H
#define ULTIMA_ULTIMA1_WIDGETS_OVERWORLD_WIDGET_H

#include "ultima/shared/maps/map.h"
#include "ultima/shared/maps/map_widget.h"

namespace Ultima {
namespace Ultima1 {

class Ultima1Game;

namespace Maps {
class MapBase;
}

namespace Widgets {

/**
 * Encapsulated class for drawing widgets within dungeons
 */
class OverworldWidget : public Shared::Maps::MapWidget {
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
	Common::String _name;
	uint _tileNum;
public:
	DECLARE_WIDGET(OverworldWidget)

	/**
	 * Constructor
	 */
	OverworldWidget(Shared::Game *game, Shared::Maps::MapBase *map, uint tileNum, const Point &pt, Shared::Maps::Direction dir = Shared::Maps::DIR_NONE) :
		Shared::Maps::MapWidget(game, map, pt, dir), _tileNum(tileNum)  {}

	/**
	 * Constructor
	 */
	OverworldWidget(Shared::Game *game, Shared::Maps::MapBase *map) : Shared::Maps::MapWidget(game, map),
		_tileNum(0) {}

	/**
	 * Get the tile number for the person
	 */
	uint getTileNum() const override { return _tileNum; }

	/**
	 * Handles loading and saving games
	 */
	void synchronize(Common::Serializer &s) override;

	/**
	 * Returns true if the given widget can move to a given position on the map
	 */
	CanMove canMoveTo(const Point &destPos) override;
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
