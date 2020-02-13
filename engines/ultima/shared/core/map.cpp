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

#include "ultima/shared/core/map.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/gfx/visual_item.h"

namespace Ultima {
namespace Shared {

void MapTile::clear() {
	_tileId = _tileNum = -1;
	_widgetNum = -1;
	_widget = nullptr;
	_itemNum = -1;
	_isDoor = _isSecretDoor = false;
	_isLadderUp = _isLadderDown = false;
	_isWall = _isHallway = _isBeams = false;
}

/*-------------------------------------------------------------------*/

void Map::MapBase::clear() {
	_mapId = 0;
	_data.clear();
	_widgets.clear();
}

void Map::MapBase::load(MapId mapId) {
	_mapId = mapId;
}

void Map::MapBase::synchronize(Common::Serializer &s) {
	_viewportPos.synchronize(s);
	uint size;
	int transportIndex = -1;
	Common::String name;

	if (s.isSaving()) {
		// Save widgets
		size = 0;
		for (uint idx = 0; idx < _widgets.size(); ++idx) {
			if (_widgets[idx]->getClassName())
				++size;
			if (_playerWidget == _widgets[idx].get())
				transportIndex = (int)idx;
		}
		assert(transportIndex >= 0);

		s.syncAsUint16LE(size);
		for (uint idx = 0; idx < _widgets.size(); ++idx) {
			name = _widgets[idx]->getClassName();
			if (!name.empty()) {
				s.syncString(name);
				_widgets[idx]->synchronize(s);
			}
		}
		s.syncAsUint16LE(transportIndex);

	} else {
		// Load widgets
		s.syncAsUint16LE(size);
		_widgets.clear();
		for (uint idx = 0; idx < size; ++idx) {
			s.syncString(name);

			MapWidget *w = _map->createWidget(this, name);
			w->synchronize(s);
			addWidget(w);
		}

		s.syncAsUint16LE(transportIndex);
		_playerWidget = _widgets[transportIndex].get();
	}
}

void Map::MapBase::setDimensions(const Point &size) {
	_data.resize(size.y);
	for (int y = 0; y < size.y; ++y)
		_data[y]._data.resize(size.x);
	_size = size;
}

Point Map::MapBase::getDirectionDelta() const {
	switch (_playerWidget->_direction) {
	case DIR_LEFT:
		return Point(-1, 0);
	case DIR_RIGHT:
		return Point(1, 0);
	case DIR_UP:
		return Point(0, -1);
	default:
		return Point(0, 1);
	}
}

Point Map::MapBase::getDeltaPosition(const Point &delta) {
	return _playerWidget->_position + delta;
} 

void Map::MapBase::resetViewport() {
	// Reset the viewport, so it's position will get recalculated
	_viewportPos.reset();
}

Point Map::MapBase::getViewportPosition(const Point &viewportSize) {
	Point &topLeft = _viewportPos._topLeft;

	if (!_viewportPos.isValid() || _viewportPos._size != viewportSize) {
		// Calculate the new position
		topLeft.x = _playerWidget->_position.x - (viewportSize.x - 1) / 2;
		topLeft.y = _playerWidget->_position.y - (viewportSize.y - 1) / 2;

		// Fixed maps, so constrain top left corner so the map fills the viewport.
		// This will accomodate future renderings with more tiles, or greater tile size
		topLeft.x = CLIP((int)topLeft.x, 0, (int)(width() - viewportSize.x));
		topLeft.y = CLIP((int)topLeft.y, 0, (int)(height() - viewportSize.y));
	}

	return topLeft;
}

void Map::MapBase::shiftViewport(const Point &delta) {
	Point &topLeft = _viewportPos._topLeft;
	topLeft += delta;

	// Shift the viewport, but constraining the map to fill up the screen
	topLeft.x = CLIP(topLeft.x, (int16)0, (int16)(width() - _viewportPos._size.x));
	topLeft.y = CLIP(topLeft.y, (int16)0, (int16)(height() - _viewportPos._size.y));
}

void Map::MapBase::addWidget(MapWidget *widget) {
	_widgets.push_back(MapWidgetPtr(widget));
}

void Map::MapBase::removeWidget(MapWidget *widget) {
	for (uint idx = 0; idx < _widgets.size(); ++idx) {
		if (_widgets[idx].get() == widget) {
			_widgets.remove_at(idx);
			break;
		}
	}
}

void Map::MapBase::getTileAt(const Point &pt, MapTile *tile) {
	tile->clear();

	// Get the base tile
	tile->_tileNum = tile->_tileId = _data[pt.y][pt.x];

	// Check for any widget on that map tile
	for (int idx = (int)_widgets.size() - 1; idx >= 0; --idx) {
		MapWidget *widget = _widgets[idx].get();
		if (widget->_position == pt) {
			tile->_widgetNum = idx;
			tile->_widget = widget;
			break;
		}
	}
}

void Map::MapBase::update() {
	// Call the update method of each widget, to allow for things like npc movement, etc.
	for (uint idx = 0; idx < _widgets.size(); ++idx)
		_widgets[idx].get()->update(true);

	// Call the update method of each widget, to allow for things like npc movement, etc.
	for (uint idx = 0; idx < _widgets.size(); ++idx)
		_widgets[idx].get()->update(false);
}

Point Map::MapBase::getPosition() const {
	return _playerWidget->_position;
}

void Map::MapBase::setPosition(const Point &pt) {
	_playerWidget->_position = pt;
}

Direction Map::MapBase::getDirection() const {
	return _playerWidget->_direction;
}

void Map::MapBase::setDirection(Direction dir) {
	_playerWidget->_direction = dir;
}

/*------------------------------------------------------------------------*/

void MapWidget::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(_position.x);
	s.syncAsSint16LE(_position.y);
	s.syncAsByte(_direction);
	s.syncString(_name);
}

void MapWidget::addInfoMsg(const Common::String &text, bool newLine) {
	CInfoMsg msg(text, newLine);
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

/*-------------------------------------------------------------------*/

void Map::clear() {
	if (_mapArea)
		_mapArea->clear();
	_mapArea = nullptr;
}

void Map::load(MapId mapId) {
	_mapArea = nullptr;
}

void Map::synchronize(Common::Serializer &s) {
	int mapId;

	if (s.isSaving()) {
		// Saving
		mapId = _mapArea->getMapId();
		s.syncAsUint16LE(mapId);
	} else {
		// Loading
		s.syncAsUint16LE(mapId);
		load(mapId);
	}

	_mapArea->synchronize(s);
}

} // End of namespace Shared
} // End of namespace Ultima
