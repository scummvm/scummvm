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
#include "access/amazon/amazon_game.h"
#include "access/amazon/amazon_player.h"
#include "access/amazon/amazon_resources.h"

namespace Access {

namespace Amazon {

AmazonPlayer::AmazonPlayer(AccessEngine *vm) : Player(vm) {
	_game = (AmazonEngine *)vm;
}

void AmazonPlayer::load() {
	Player::load();

	// Special scene setup for the top-down view when on the Slaver ship
	if (_vm->_room->_roomFlag == 3) {
		_playerOffset.x = _vm->_screen->_scaleTable1[8];
		_playerOffset.y = _vm->_screen->_scaleTable1[11];
		_leftDelta = 0;
		_rightDelta = 8;
		_upDelta = 2;
		_downDelta = -2;
		_scrollConst = 2;

		for (int i = 0; i < _vm->_playerDataCount; ++i) {
			_walkOffRight[i] = OVEROFFR[i];
			_walkOffLeft[i] = OVEROFFL[i];
			_walkOffUp[i] = OVEROFFU[i];
			_walkOffDown[i] = OVEROFFD[i];
			_walkOffUR[i].x = OVEROFFURX[i];
			_walkOffUR[i].y = OVEROFFURY[i];
			_walkOffDR[i].x = OVEROFFDRX[i];
			_walkOffDR[i].y = OVEROFFDRY[i];
			_walkOffUL[i].x = OVEROFFULX[i];
			_walkOffUL[i].y = OVEROFFULY[i];
			_walkOffDL[i].x = OVEROFFDLX[i];
			_walkOffDL[i].y = OVEROFFDLY[i];
		}

		_vm->_timers[8]._initTm = 7;
		_vm->_timers[8]._timer = 7;
		++_vm->_timers[8]._flag;

		_sideWalkMin = 0;
		_sideWalkMax = 5;
		_upWalkMin = 12;
		_upWalkMax = 17;
		_downWalkMin = 6;
		_downWalkMax = 11;
		_diagUpWalkMin = 0;
		_diagUpWalkMax = 5;
		_diagDownWalkMin = 0;
		_diagDownWalkMax = 5;
		_game->_guard->setPosition(Common::Point(56, 190));
	} else {
		for (int i = 0; i < _vm->_playerDataCount; ++i) {
			_walkOffRight[i] = SIDEOFFR[i];
			_walkOffLeft[i] = SIDEOFFL[i];
			_walkOffUp[i] = SIDEOFFU[i];
			_walkOffDown[i] = SIDEOFFD[i];

			_walkOffUR[i].x = DIAGOFFURX[i];
			_walkOffUR[i].y = DIAGOFFURY[i];
			_walkOffDR[i].x = DIAGOFFDRX[i];
			_walkOffDR[i].y = DIAGOFFDRY[i];
			_walkOffUL[i].x = DIAGOFFULX[i];
			_walkOffUL[i].y = DIAGOFFULY[i];
			_walkOffDL[i].x = DIAGOFFDLX[i];
			_walkOffDL[i].y = DIAGOFFDLY[i];
		}
	}
}

} // End of namespace Amazon

} // End of namespace Access
