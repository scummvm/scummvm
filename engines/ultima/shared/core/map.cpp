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
	_item = nullptr;
}

/*-------------------------------------------------------------------*/

Map::Map() {
	_mapId = 0;
	_direction = DIR_UP;
	_fixed = false;
	_dungeonLevel = 0;
}

void Map::clear() {
	_mapId = 0;
	_data.clear();
	_widgets.clear();
	_items.clear();
}

void Map::setDimensions(const Point &size) {
	_data.resize(size.y);
	for (int y = 0; y < size.y; ++y)
		_data[y]._data.resize(size.x);
	_size = size;
}

Point Map::getDeltaPosition(const Point &delta) {
	Point pt = _position + delta;

	if (!_fixed) {
		if (pt.x < 0)
			pt.x += _size.x;
		else if (pt.x >= _size.x)
			pt.x -= _size.x;
		if (pt.y < 0)
			pt.y += _size.y;
		else if (pt.y >= _size.y)
			pt.y -= _size.y;
	}

	return pt;
}

void Map::loadMap(int mapId, uint videoMode) {
	clear();
	_mapId = mapId;
	_fixed = false;
}

void Map::setPosition(const Point &pt) {
	// Set the new party/player position
	_position = Point(pt.x * _tilesPerOrigTile.x, pt.y * _tilesPerOrigTile.y);

	// The party is kept in the first widget slot, so keep it's position up to date
	_widgets[0]->_position = _position;

	// Reset the viewport, so it's position will get recalculated
	_viewportPos.reset();
}

Point Map::getViewportPosition(const Point &viewportSize) {
	Point &topLeft = _viewportPos._topLeft;

	if (!_viewportPos.isValid() || _viewportPos._size != viewportSize) {
		// Calculate the new position
		topLeft.x = _position.x - (viewportSize.x - 1) / 2;
		topLeft.y = _position.y - (viewportSize.y - 1) / 2;

		if (_fixed) {
			// Fixed maps, so constrain top left corner so the map fills the viewport. This will accomodate
			// future renderings with more tiles, or greater tile size
			topLeft.x = CLIP((int)topLeft.x, 0, (int)(width() - viewportSize.x));
			topLeft.y = CLIP((int)topLeft.y, 0, (int)(height() - viewportSize.y));
		} else {
			// Non-fixed map, so it wraps around the edges if necessary
			if (topLeft.x < 0)
				topLeft.x += width();
			else if (topLeft.x >= (int)width())
				topLeft.x -= width();

			if (topLeft.y < 0)
				topLeft.y += height();
			else if (topLeft.y >= (int)height())
				topLeft.y -= height();
		}

		_viewportPos._mapId = _mapId;
		_viewportPos._size = viewportSize;
	}

	return topLeft;
}

void Map::shiftViewport(const Point &delta) {
	Point &topLeft = _viewportPos._topLeft;
	topLeft += delta;

	if (_fixed) {
		// Shift the viewport, but constraining the map to fill up the screen
		topLeft.x = CLIP(topLeft.x, (int16)0, (int16)(width() - _viewportPos._size.x));
		topLeft.y = CLIP(topLeft.y, (int16)0, (int16)(height() - _viewportPos._size.y));
	} else {
		if (topLeft.x < 0)
			topLeft.x += width();
		else if (topLeft.x >= (int16)width())
			topLeft.x -= width();
		if (topLeft.y < 0)
			topLeft.y += height();
		else if (topLeft.y >= (int16)height())
			topLeft.y -= height();
	}
}

void Map::getTileAt(const Point &pt, MapTile *tile) {
	tile->clear();

	// Get the base tile
	tile->_tileNum = tile->_tileId = _data[pt.y][pt.x];

	// Check for any widget on that map tile
	for (uint idx = 0; idx < _widgets.size(); ++idx) {
		if (_widgets[idx]->_position == pt) {
			tile->_widgetNum = idx;
			tile->_widget = _widgets[idx].get();
			break;
		}
	}

	// Check for any item on that map tile
	for (uint idx = 0; idx < _items.size(); ++idx) {
		if (_items[idx]->_position == pt) {
			tile->_itemNum = idx;
			tile->_item = _items[idx].get();
			break;
		}
	}
}

void Map::addWidget(MapWidget *widget) {
	_widgets.push_back(Shared::MapWidgetPtr(widget));
}

} // End of namespace Shared
} // End of namespace Ultima
