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

void Map::MapBase::setDimensions(const Point &size) {
	_data.resize(size.y);
	for (int y = 0; y < size.y; ++y)
		_data[y]._data.resize(size.x);
	_size = size;
}

Point Map::MapBase::getDirectionDelta() const {
	switch (_direction) {
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
	return _position + delta;
} 

void Map::MapBase::setPosition(const Point &pt) {
	// Set the new party/player position
	_position = Point(pt.x * _tilesPerOrigTile.x, pt.y * _tilesPerOrigTile.y);

	// The party is kept in the first widget slot, so keep it's position up to date
	_widgets[0]->_position = _position;

	// Reset the viewport, so it's position will get recalculated
	_viewportPos.reset();
}

Point Map::MapBase::getViewportPosition(const Point &viewportSize) {
	Point &topLeft = _viewportPos._topLeft;

	if (!_viewportPos.isValid() || _viewportPos._size != viewportSize) {
		// Calculate the new position
		topLeft.x = _position.x - (viewportSize.x - 1) / 2;
		topLeft.y = _position.y - (viewportSize.y - 1) / 2;

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
	_widgets.push_back(Shared::MapWidgetPtr(widget));
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

/*-------------------------------------------------------------------*/

void Map::clear() {
	if (_mapArea)
		_mapArea->clear();
	_mapArea = nullptr;
}

void Map::load(MapId mapId) {
	_mapArea = nullptr;
}

} // End of namespace Shared
} // End of namespace Ultima
