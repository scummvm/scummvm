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

#include "ultima/ultima1/core/monsters.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {

bool U1DungeonMonster::isBlockingView() const {
	return _monsterId != MONSTER_INVISIBLE_SEEKER && _monsterId != MONSTER_MIMIC
		&& _monsterId != MONSTER_GELATINOUS_CUBE;
}

void U1DungeonMonster::draw(Shared::DungeonSurface &s, uint distance) {
	if (distance < 5) {
		if (_monsterId == MONSTER_GELATINOUS_CUBE) {
			s.drawWall(distance);
			s.drawLeftEdge(distance);
			s.drawRightEdge(distance);
		} else {
			Ultima1Game *game = static_cast<Ultima1Game *>(g_vm->_game);
			DungeonWidget::drawWidget(s, _monsterId, distance, game->_edgeColor);
		}
	}
}

/*-------------------------------------------------------------------*/

const byte OFFSET_Y[5] = { 139, 112, 96, 88, 84 };
enum { POINT_AT = 126, END_OF_DRAW = 127 };

void DungeonWidget::drawWidget(Graphics::ManagedSurface &s, DungeonWidgetId widgetId, uint distance, byte color) {
	Point pt, priorPt;
	int yOffset = OFFSET_Y[distance];
	int shift = (distance == 4) ? 5 : distance;

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

void DungeonWidget::postDraw(Shared::DungeonSurface &s) {
	if (_itemId == DITEM_COFFIN)
		drawWidget(s, UITEM_COFFIN, 1, _game->_edgeColor);
	else if (_itemId == DITEM_CHEST)
		drawWidget(s, MONSTER_MIMIC, 1, _game->_edgeColor);
}

} // End of namespace Ultima1
} // End of namespace Ultima
