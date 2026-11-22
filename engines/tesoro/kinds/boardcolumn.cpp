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

#include "common/textconsole.h"

#include "tesoro/kinds/boardcolumn.h"
#include "tesoro/world.h"

namespace Tesoro {

void BoardColumn::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world) {
	_world = &world;
	initAnimated(record, roomSortOffset, levelPlayers, 0);
	_column = record.dataByte(2);

	const Common::String name = Common::String::format("OBJCOL%d", _column + 1);
	_hoverName = world.phrase(name.c_str());

	// Never visible; it takes the pointer, collides and sits in the room list.
	_flags = (_flags & ~kVisible) | kCollidable | kInRoomList | kHitter;
}

int BoardColumn::setVerbAndClick(int verb) {
	if (_world->boardBusy())
		return 1;
	if (_column >= ConnectFour::kColumns) {
		warning("Tesoro: board column %d is outside the board", _column);
		return 1;
	}

	byte *board = _world->board().raw();
	byte *column = board + 1 + _column * 8;
	if (column[0] == ConnectFour::kHeight)
		return 0;

	_world->setBoardBusy();
	column[1 + (int8)column[0]] = 1;
	++column[0];
	board[0] = (byte)_column;
	_world->stampMoveTime();
	_world->showPieces();
	return 1;
}

} // End of namespace Tesoro
