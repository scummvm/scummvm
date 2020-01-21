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

#include "ultima/shared/maps/map_widget.h"
#include "ultima/shared/maps/map_base.h"
#include "ultima/shared/maps/map_tile.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/gfx/visual_item.h"

namespace Ultima {
namespace Shared {
namespace Maps {

EMPTY_MESSAGE_MAP(MapWidget, MessageTarget);

Direction MapWidget::directionFromKey(Common::KeyCode keycode) {
	switch (keycode) {
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4:
		return DIR_WEST;

	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6:
		return DIR_EAST;

	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		return DIR_NORTH;

	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		return DIR_SOUTH;

	default:
		return DIR_NONE;
	}
}

void MapWidget::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(_position.x);
	s.syncAsSint16LE(_position.y);
	s.syncAsByte(_direction);
	s.syncString(_name);
}

void MapWidget::addInfoMsg(const Common::String &text, bool newLine, bool replaceLine) {
	CInfoMsg msg(text, newLine, replaceLine);
	msg.execute(_game->getView());
}

MapWidget::CanMove MapWidget::canMoveTo(const Point &destPos) {
	if (destPos.x < 0 || destPos.y < 0 || destPos.x >= (int)_map->width() || destPos.y >= (int)_map->height()) {
		// If the map is fixed, allow moving beyond it's edges so it can be left
		if (!_map->isMapWrapped())
			return YES;
	}

	// Get the details of the position
	MapTile destTile;
	_map->getTileAt(destPos, &destTile);

	// If there's a widget blocking the tile, return false
	if (destTile._widget && destTile._widget->isBlocking())
		return NO;

	return UNSET;
}

void MapWidget::moveTo(const Point &destPos, Direction dir) {
	// If no direction is specified, we'll need to figure it out relative to the old position
	if (dir == DIR_NONE) {
		Point delta = destPos - _position;
		if (ABS(delta.x) > ABS(delta.y))
			_direction = delta.x > 0 ? DIR_EAST : DIR_WEST;
		else if (delta.y != 0)
			_direction = delta.y > 0 ? DIR_SOUTH : DIR_NORTH;
	} else {
		_direction = dir;
	}

	// Set new location
	_position = destPos;

	// Handle wrap around if need be on maps that wrap
	if (_map->isMapWrapped()) {
		if (_position.x < 0)
			_position.x += _map->width();
		else if (_position.x >= (int)_map->width())
			_position.x -= _map->width();
		if (_position.y < 0)
			_position.y += _map->height();
		else if (_position.y >= (int)_map->height())
			_position.y -= _map->height();
	}
}

} // End of namespace Maps
} // End of namespace Shared
} // End of namespace Ultima
