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
	_tileNum = -1;
}

/*-------------------------------------------------------------------*/

Map::Map() {
	_mapId = 0;
	_direction = DIR_UP;
	_fixed = false;
}

Point Map::getDeltaPosition(const Point &delta) {
	Point pt = _position + delta;
	if (pt.x < 0)
		pt.x += _size.x;
	else if (pt.x >= _size.x)
		pt.x -= _size.x;
	if (pt.y < 0)
		pt.y += _size.y;
	else if (pt.y >= _size.y)
		pt.y -= _size.y;

	return pt;
}

void Map::loadMap(int mapId, uint videoMode) {
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
		if (_fixed) {
			// Whilst in the original Ultima i cities and castles fit onto a single screen, with enhancements this
			// may not be the case. So allow for showing the relevant part of a fixed map so the player is on-screen
			if (_position.x < (int)(width() / 2)) {
				topLeft.x = MAX(_position.x - 5, 0);
			} else {
				topLeft.x = MAX((int)(_position.x + 5 - width()), 0);
			}

			if (_position.y < (int)(height() / 2)) {
				topLeft.y = MAX(_position.y - 5, 0);
			} else {
				topLeft.y = MAX((int)(_position.y + 5 - height()), 0);
			}
		} else {
			// Non-fixed map, so it wraps around the edges if necessary
			topLeft.x = _position.x - (viewportSize.x - 1) / 2;
			topLeft.y = _position.y - (viewportSize.y - 1) / 2;

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
	// Get the base tile
	tile->_tileNum = tile->_tileId = _data[pt.y * _size.x + pt.x];

	// Get the tiles for any widgets on that map tile
	tile->_widgetTiles.clear();
	for (uint idx = 0; idx < _widgets.size(); ++idx) {
		if (_widgets[idx]->_position == pt)
			tile->_widgetTiles.push_back(_widgets[idx]->getTileNum());
	}
}

void Map::addWidget(MapWidget *widget) {
	_widgets.push_back(Shared::MapWidgetPtr(widget));
}

} // End of namespace Shared
} // End of namespace Ultima
