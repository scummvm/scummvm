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

#include "ultima/ultima1/widgets/urban_widget.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/maps/map_tile.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

EMPTY_MESSAGE_MAP(UrbanWidget, Shared::Maps::MapWidget);

Shared::Maps::MapWidget::CanMove UrbanWidget::canMoveTo(const Point &destPos) {
	Shared::Maps::MapWidget::CanMove result = Shared::Maps::MapWidget::canMoveTo(destPos);
	if (result != UNSET)
		return result;

	// Get the details of the position
	Maps::U1MapTile destTile;
	_map->getTileAt(destPos, &destTile);

	return destTile._tileId == Maps::CTILE_GROUND || destTile._tileId >= Maps::CTILE_POND_EDGE1 ? YES : NO;
}

bool UrbanWidget::moveBy(const Point &delta) {
	// TODO: Movement allowed on tile 63.. is this the gate of the princess' cells?
	Point newPos = _position + delta;
	if (canMoveTo(newPos) == YES) {
		_position = newPos;
		return true;
	} else {
		return false;
	}
}

void UrbanWidget::synchronize(Common::Serializer &s) {
	MapWidget::synchronize(s);
	s.syncAsUint16LE(_tileNum);
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
