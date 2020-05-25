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

#include "ultima/ultima1/widgets/dungeon_widget.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map_base.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

DungeonWidget::DungeonWidget(Ultima1Game *game, Maps::MapBase *map, DungeonWidgetId widgetId,
		const Point &pt) : Shared::Maps::DungeonWidget(game, map, pt), _widgetId(widgetId) {
}

DungeonWidget::DungeonWidget(Ultima1Game *game, Maps::MapBase *map) : Shared::Maps::DungeonWidget(game, map), _widgetId(MONSTER_NONE) {
}

Ultima1Game *DungeonWidget::getGame() const {
	return static_cast<Ultima1Game *>(_game);
}

Maps::MapBase *DungeonWidget::getMap() const {
	return static_cast<Maps::MapBase *>(_map);
}

const byte OFFSET_Y[5] = { 139, 112, 96, 88, 84 };
enum { POINT_AT = 126, END_OF_DRAW = 127 };

void DungeonWidget::drawWidget(Graphics::ManagedSurface &s, DungeonWidgetId widgetId, uint distance, byte color) {
	Point pt, priorPt;

	if (distance == 0)
		// This can't happen in the original, but in case I add an 'etherial' cheat to allow moving
		// through monsters, this will guard against a crash
		distance = 1;
	int yOffset = OFFSET_Y[MIN((int)distance - 1, 4)];
	int shift = (distance == 5) ? 5 : distance - 1;

	// Get a pointer to the drawing data
	const byte *data = getData();
	data += READ_LE_UINT16(data + widgetId * 2);

	while (*data != END_OF_DRAW) {
		// Check for a point vs a line
		bool isPoint = *data == POINT_AT;
		if (isPoint)
			++data;

		// Get the next position
		getPos(data, shift, pt);
		pt.y += yOffset;

		// Draw point or line
		if (!isPoint)
			s.drawLine(priorPt.x, priorPt.y, pt.x, pt.y, color);
		priorPt = pt;
	}
}

const byte *DungeonWidget::getData() {
	Ultima1Game *game = static_cast<Ultima1Game *>(g_vm->_game);
	return game->_res->DUNGEON_DRAW_DATA;
}

void DungeonWidget::getPos(const byte *&data, int bitShift, Point &pt) {
	pt.x = ((int8)*data++ >> bitShift) + 160;
	pt.y = ((int8)*data++ >> bitShift);
}

void DungeonWidget::draw(Shared::DungeonSurface &s, uint distance) {
	Ultima1Game *game = static_cast<Ultima1Game *>(_game);
	drawWidget(s, _widgetId, distance, game->_edgeColor);
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
