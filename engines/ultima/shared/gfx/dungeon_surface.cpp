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

#include "ultima/shared/gfx/dungeon_surface.h"
#include "ultima/shared/core/map.h"
#include "ultima/shared/early/game.h"

namespace Ultima {
namespace Shared {

const byte WALL_ARRAY_X[] = { 0, 72, 108, 126, 135, 144 };
const byte WALL_ARRAY_Y[] = { 0, 36, 54, 63, 68, 72 };

DungeonSurface::DungeonSurface(const Graphics::ManagedSurface &src, const Rect &bounds, Game *game, DrawWidgetFn widgetFn) :
		Gfx::VisualSurface(src, bounds), _widgetFn(widgetFn) {
	_edgeColor = game->_edgeColor;
	_widgetColor = 0;
}

void DungeonSurface::drawWall(uint distance) {
	int offsetX = !distance ? 8 : 0, offsetY = !distance ? 8 : 0;

	if (distance <= 5) {
		hLine(WALL_ARRAY_X[distance] + 16 + offsetX, WALL_ARRAY_Y[distance] + 8 + offsetY,
			303 - WALL_ARRAY_X[distance] - offsetX, _edgeColor);
		hLine(WALL_ARRAY_X[distance] + 16 + offsetX, 15 - WALL_ARRAY_Y[distance] - offsetY,
			303 - WALL_ARRAY_X[distance] - offsetX, _edgeColor);
	}
}

void DungeonSurface::drawDoorway(uint distance) {
	int offsetY = !distance ? 8 : 0;
	byte color = !distance ? 0 : _edgeColor;

	if (distance < 5) {
		frameRect(Rect(WALL_ARRAY_X[distance + 1] + 16, WALL_ARRAY_Y[distance + 1] + 8,
			303 - WALL_ARRAY_X[distance + 1], 151 - WALL_ARRAY_Y[distance] - offsetY), color);
	}
}

void DungeonSurface::drawLeftEdge(uint distance) {
	if (distance <= 5) {
		vLine(WALL_ARRAY_X[distance] + 16, WALL_ARRAY_Y[distance] + 8, WALL_ARRAY_Y[distance] + 151, _edgeColor);
	}
}

void DungeonSurface::drawRightEdge(uint distance) {
	if (distance <= 5) {
		vLine(WALL_ARRAY_X[distance] + 303, WALL_ARRAY_Y[distance] + 8, WALL_ARRAY_Y[distance] + 151, _edgeColor);
	}
}

void DungeonSurface::drawMonsterAt(uint distance, uint monsterId) {
	if (distance < 5) {
		if (monsterId == 9) {
			// Gelatinous cube
			drawWall(distance);
			drawLeftEdge(distance);
			drawRightEdge(distance);
		} else {
			drawWidget(monsterId, distance, _edgeColor);
		}
	}
}

void DungeonSurface::drawWidget(uint widgetId, uint distance, byte color) {
	_widgetFn(*this, widgetId, distance, color);
}

} // End of namespace Shared
} // End of namespace Ultima
