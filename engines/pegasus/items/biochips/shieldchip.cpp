/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "pegasus/gamestate.h"
#include "pegasus/items/biochips/shieldchip.h"
#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

ShieldChip *g_shield = nullptr;

ShieldChip::ShieldChip(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) :
		BiochipItem(id, neighborhood, room, direction) {
	g_shield = this;
}

void ShieldChip::select() {
	BiochipItem::select();
	GameState.setShieldOn(true);
	if (g_neighborhood)
		g_neighborhood->shieldOn();
}

void ShieldChip::deselect() {
	BiochipItem::deselect();
	GameState.setShieldOn(false);
	if (g_neighborhood)
		g_neighborhood->shieldOff();
}

} // End of namespace Pegasus
