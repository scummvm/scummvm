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

#include "mm/mm1/views_enh/map_popup.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define ENTIRE_MAP_W (MAP_W * MAP_TILE_W)
#define ENTIRE_MAP_H (MAP_H * MAP_TILE_H)

MapPopup::MapPopup() : ScrollPopup("MapPopup"),
		_map(this) {
	// Center 16x16 map area
	Common::Rect area(
		(320 / 2) - (ENTIRE_MAP_W / 2) - 1,
		(200 / 2) - (ENTIRE_MAP_H / 2) - 1,
		(320 / 2) + (ENTIRE_MAP_W / 2) + 1,
		(200 / 2) + (ENTIRE_MAP_H / 2) + 1
	);
	_map.setBounds(area);

	// Allow room for scroll edges, and line at top & bottom
	area.left -= 9;
	area.right += 9;
	area.top -= 17;
	area.bottom += 17;
	setBounds(area);
}

void MapPopup::draw() {
	ScrollPopup::draw();

	// Write the map name
	Maps::Map &map = *g_maps->_currentMap;
	Common::String mapDesc = map.getDescription();
	writeString(0, 0, mapDesc, ALIGN_MIDDLE);

	// Write direction
	Common::String dir;
	switch (g_maps->_forwardMask) {
	case Maps::DIRMASK_N:
		dir = STRING["enhdialogs.map.north"];
		break;
	case Maps::DIRMASK_S:
		dir = STRING["enhdialogs.map.south"];
		break;
	case Maps::DIRMASK_E:
		dir = STRING["enhdialogs.map.east"];
		break;
	case Maps::DIRMASK_W:
		dir = STRING["enhdialogs.map.west"];
		break;
	default:
		break;
	}
	writeString(0, _innerBounds.height() - 9, dir, ALIGN_MIDDLE);

	// Write position
	writeString(2, _innerBounds.height() - 9,
		Common::String::format("X = %d", g_maps->_mapPos.x),
		ALIGN_LEFT
	);
	writeString(0, _innerBounds.height() - 9,
		Common::String::format("Y = %d", g_maps->_mapPos.y),
		ALIGN_RIGHT
	);
}

} // namespace Views
} // namespace MM1
} // namespace MM
