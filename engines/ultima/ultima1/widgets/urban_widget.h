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

#ifndef ULTIMA_ULTIMA1_WIDGETS_URBAN_WIDGET_H
#define ULTIMA_ULTIMA1_WIDGETS_URBAN_WIDGET_H

#include "ultima/shared/maps/map_widget.h"
#include "ultima/ultima1/maps/map.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

/**
 * Base class for widgets in urban maps
 */
class UrbanWidget : public Shared::Maps::MapWidget {
	DECLARE_MESSAGE_MAP;
private:
	uint _tileNum;
protected:
	CLASSDEF;

	/**
	 * Moves by a given delta if the destination is available
	 * @param delta		Delta to move character by
	 * @returns			True if the move was able to be done
	 */
	bool moveBy(const Point &delta);
public:
	/**
	 * Constructor
	 */
	UrbanWidget(Shared::Game *game, Shared::Maps::MapBase *map, uint tileNum) :
		Shared::Maps::MapWidget(game, map), _tileNum(tileNum) {}

	/**
	 * Destructor
	 */
	~UrbanWidget() override {}

	/**
	 * Get the tile number for the person
	 */
	uint getTileNum() const override { return _tileNum; }

	/**
	 * Returns true if the given widget can move to a given position on the map
	 */
	CanMove canMoveTo(const Point &destPos) override;

	/**
	 * Handles loading and saving games
	 */
	void synchronize(Common::Serializer &s) override;
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
