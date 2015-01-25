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

#include "common/scummsys.h"
#include "access/access.h"
#include "access/room.h"
#include "access/martian/martian_game.h"
#include "access/martian/martian_player.h"
#include "access/martian/martian_resources.h"

namespace Access {

namespace Martian {

MartianPlayer::MartianPlayer(AccessEngine *vm) : Player(vm) {
	_game = (MartianEngine *)vm;
}

void MartianPlayer::load() {
	Player::load();

	// Overwrite game-specific values
	_playerOffset.x = _vm->_screen->_scaleTable1[20];
	_playerOffset.y = _vm->_screen->_scaleTable1[52];
	_leftDelta = -9;
	_rightDelta = 33;
	_upDelta = 5;
	_downDelta = -5;
	_scrollConst = 5;

	for (int i = 0; i < _vm->_playerDataCount; ++i) {
		_walkOffRight[i] = SIDEOFFR[i];
		_walkOffLeft[i] = SIDEOFFL[i];
		_walkOffUp[i] = SIDEOFFU[i];
		_walkOffDown[i] = SIDEOFFD[i];
	}

	_sideWalkMin = 0;
	_sideWalkMax = 7;
	_upWalkMin = 8;
	_upWalkMax = 14;
	_downWalkMin = 15;
	_downWalkMax = 23;
}

} // End of namespace Martian

} // End of namespace Access
