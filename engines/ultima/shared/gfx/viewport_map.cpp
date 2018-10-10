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

#include "ultima/shared/gfx/viewport_map.h"
#include "ultima/shared/maps/map.h"
#include "ultima/shared/early/game.h"

namespace Ultima {
namespace Shared {

EMPTY_MESSAGE_MAP(ViewportMap, Gfx::VisualItem);

void ViewportMap::draw() {
	// Get a surface reference and clear it's contents
	Gfx::VisualSurface s = getSurface();
	s.clear();

	// Figure out how many tiles can fit into the display
	const Point spriteSize = _sprites->getSpriteSize();
	const Point visibleTiles(_bounds.width() / spriteSize.x, _bounds.height() / spriteSize.y);

	// Get a reference to the map and get the starting tile position
	Maps::Map *map = getGame()->getMap();
	const Point topLeft = map->getViewportPosition(visibleTiles);

	// Iterate through drawing the map
	Maps::MapTile tile;
	for (int y = 0; y < visibleTiles.y; ++y) {
		for (int x = 0; x < visibleTiles.x; ++x) {
			Point drawPos(x * spriteSize.x, y * spriteSize.y);

			// Get the next tile to display and draw it
			map->getTileAt(Point(topLeft.x + x, topLeft.y + y), &tile);
			(*_sprites)[tile._tileDisplayNum].draw(s, drawPos);

			// Draw any widgets on the tile
			for (uint idx = 0; idx < tile._widgets.size(); ++idx)
				(*_sprites)[tile._widgets[idx]->getTileNum()].draw(s, drawPos);
		}
	}
}

} // End of namespace Shared
} // End of namespace Ultima
