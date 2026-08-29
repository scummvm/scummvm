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

#ifndef ULTIMA2_VIEWS_LOCATION_MAP_H
#define ULTIMA2_VIEWS_LOCATION_MAP_H

#include "ultima/shared/engine/events.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Shared::Messages;

class LocationMap : public Shared::UIElement {
private:
	int _animIndex = 0;

	void prepareMapForDrawing();

	/**
	 * Returns the tile to draw for a given raw map tile, substituting
	 * in the appropriate animation frame for the castle and city flags.
	 * Returns -1 if the tile shouldn't be redrawn this tick (the castle
	 * flag is only redrawn on 2 of every 6 ticks)
	 */
	int animatedTileId(byte tileId) const;

public:
	LocationMap(Shared::UIElement *parent) : Shared::UIElement("LocationMap", parent) {}
	~LocationMap() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	void timeout() override;
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
