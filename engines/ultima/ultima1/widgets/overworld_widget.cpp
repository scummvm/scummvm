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

#include "ultima/ultima1/widgets/overworld_widget.h"
#include "ultima/ultima1/maps/map_base.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

Ultima1Game *OverworldWidget::getGame() const {
	return static_cast<Ultima1Game *>(_game);
}

Maps::MapBase *OverworldWidget::getMap() const {
	return static_cast<Maps::MapBase *>(_map);
}

void OverworldWidget::synchronize(Common::Serializer &s) {
	MapWidget::synchronize(s);
	s.syncAsUint16LE(_tileNum);
}

Shared::Maps::MapWidget::CanMove OverworldWidget::canMoveTo(const Point &destPos) {
	Shared::Maps::MapWidget::CanMove result = Shared::Maps::MapWidget::canMoveTo(destPos);
	if (result != Shared::Maps::MapWidget::UNSET)
		return result;

	Maps::U1MapTile tile;
	getMap()->getTileAt(destPos, &tile);

	// Default mobility for most monsters and transports is overland only
	return tile.isGround() ? Shared::Maps::MapWidget::YES : Shared::Maps::MapWidget::NO;
}


} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
