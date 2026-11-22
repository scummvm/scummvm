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

#ifndef TESORO_KINDS_BOARDCOLUMN_H
#define TESORO_KINDS_BOARDCOLUMN_H

#include "tesoro/core/connectfour.h"
#include "tesoro/kinds/interactive.h"

namespace Tesoro {

class World;

/**
 * Object kind 21: one column of the "Connect-4" board. Clicking it drops the player's piece on the column unless a move is
 * still playing out or the column is full.
 */
class BoardColumn : public InteractiveObject {
public:
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host);

	int setVerbAndClick(int verb) override;

	int column() const { return _column; }

private:
	int _column = 0;
};

} // End of namespace Tesoro

#endif
