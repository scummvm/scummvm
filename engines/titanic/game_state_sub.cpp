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

#include "titanic/game_state_sub.h"
#include "titanic/game_state.h"

namespace Titanic {

CGameStateSub::CGameStateSub(CGameState *owner) : _gameState(owner),
		_field0(0), _field4(0), _field8(0), _fieldC(0) {
}

void CGameStateSub::save(SimpleFile *file) const {
	file->writeNumber(_field4);
	file->writeNumber(_field8);
	file->writeNumber(_fieldC);
}

void CGameStateSub::load(SimpleFile *file) {
	_field0 = 0;
	_field4 = file->readNumber();
	_field8 = file->readNumber();
	_fieldC = file->readNumber();
}

int CGameStateSub::fn2() {
	warning("TODO");
	return 0;
}

} // End of namespace Titanic z
