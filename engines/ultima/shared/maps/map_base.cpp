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

#include "ultima/shared/maps/map_base.h"
#include "ultima/shared/maps/map_widget.h"
#include "ultima/shared/maps/map.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/gfx/visual_item.h"

namespace Ultima {
namespace Shared {
namespace Maps {

MapWidget *MapBase::WidgetsArray::findByClass(const ClassDef &classDef) const {
	for (uint idx = 0; idx < size(); ++idx) {
		MapWidget *w = (*this)[idx].get();
		if (w->isInstanceOf(classDef))
			return w;
	}

	return nullptr;
}

/*-------------------------------------------------------------------*/

void MapBase::clear() {
	_mapId = 0;
	_data.clear();
	_widgets.clear();
}

void MapBase::load(MapId mapId) {
	_mapId = mapId;
}

void MapBase::synchronize(Common::Serializer &s) {
	_viewportPos.synchronize(s);
	uint size;
	int transportIndex = -1;
	Common::String name;

	// If the map is modified in any way, it gets synchronized as well
	s.syncAsByte(_mapModified);
	if (_mapModified) {
		for (uint y = 0; y < height(); ++y) {
			for (uint x = 0; x < width(); ++x)
				s.syncAsByte(_data[y][x]);
		}
	}

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

			MapWidget *w = createWidget(name);
			w->synchronize(s);
			addWidget(w);
		}

		s.syncAsUint16LE(transportIndex);
		_playerWidget = _widgets[transportIndex].get();
	}
}

void MapBase::setDimensions(const Point &size) {
	_data.resize(size.y);
	for (int y = 0; y < size.y; ++y)
		_data[y].resize(size.x);
	_size = size;
}

Point MapBase::getDirectionDelta() const {
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

Point MapBase::getDeltaPosition(const Point &delta) {
	return _playerWidget->_position + delta;
} 

void MapBase::resetViewport() {
	// Reset the viewport, so it's position will get recalculated
	_viewportPos.reset();
}

Point MapBase::getViewportPosition(const Point &viewportSize) {
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

void MapBase::shiftViewport(const Point &delta) {
	Point &topLeft = _viewportPos._topLeft;
	topLeft += delta;

	// Shift the viewport, but constraining the map to fill up the screen
	topLeft.x = CLIP(topLeft.x, (int16)0, (int16)(width() - _viewportPos._size.x));
	topLeft.y = CLIP(topLeft.y, (int16)0, (int16)(height() - _viewportPos._size.y));
}

void MapBase::addWidget(MapWidget *widget) {
	_widgets.push_back(MapWidgetPtr(widget));
}

void MapBase::removeWidget(MapWidget *widget) {
	for (uint idx = 0; idx < _widgets.size(); ++idx) {
		if (_widgets[idx].get() == widget) {
			_widgets.remove_at(idx);
			break;
		}
	}
}

void MapBase::getTileAt(const Point &pt, MapTile *tile, bool includePlayer) {
	tile->clear();

	// Get the base tile
	tile->_tileDisplayNum = tile->_tileId = _data[pt.y][pt.x];

	// Check for any widgets on that map tile
	tile->_widgets.clear();
	tile->_widget = nullptr;
	tile->_widgetNum = -1;

	for (int idx = (int)_widgets.size() - 1; idx >= 0; --idx) {
		MapWidget *widget = _widgets[idx].get();
		if (widget->_position == pt && (includePlayer || widget != _playerWidget)) {
			tile->_widgets.push_back(widget);

			if (!tile->_widget) {
				tile->_widgetNum = idx;
				tile->_widget = widget;
			}
		}
	}
}

void MapBase::setTileAt(const Point &pt, uint tileId) {
	_data[pt.y][pt.x] = tileId;
}

void MapBase::update() {
	// Call the update method of each widget, to allow for things like npc movement, etc.
	for (uint idx = 0; idx < _widgets.size(); ++idx)
		_widgets[idx].get()->update(true);

	// Call the update method of each widget, to allow for things like npc movement, etc.
	for (uint idx = 0; idx < _widgets.size(); ++idx)
		_widgets[idx].get()->update(false);
}

Point MapBase::getPosition() const {
	return _playerWidget->_position;
}

void MapBase::setPosition(const Point &pt) {
	_playerWidget->_position = pt;
}

Direction MapBase::getDirection() const {
	return _playerWidget->_direction;
}

void MapBase::setDirection(Direction dir) {
	_playerWidget->_direction = dir;
}

void MapBase::addInfoMsg(const Common::String &text, bool newLine, bool replaceLine) {
	CInfoMsg msg(text, newLine, replaceLine);
	msg.execute(_game->getView());
}

} // End of namespace Maps
} // End of namespace Shared
} // End of namespace Ultima
