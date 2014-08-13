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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "common/textconsole.h"
#include "access/player.h"
#include "access/access.h"
#include "access/resources.h"

namespace Access {

Player::Player(AccessEngine *vm): Manager(vm) {
	Common::fill(&_walkOffRight[0], &_walkOffRight[PLAYER_DATA_COUNT], 0);
	Common::fill(&_walkOffLeft[0], &_walkOffLeft[PLAYER_DATA_COUNT], 0);
	Common::fill(&_walkOffUp[0], &_walkOffUp[PLAYER_DATA_COUNT], 0);
	Common::fill(&_walkOffDown[0], &_walkOffDown[PLAYER_DATA_COUNT], 0);

	_field0 = 0;
	_monData = nullptr;
	_rawTempL = 0;
	_rawXTemp = 0;
	_rawYTempL = 0;
	_rawYTemp = 0;
	_playerXLow = 0;
	_playerX = 0;
	_playerYLow = 0;
	_playerY = 0;
	_frame = 0;
	_playerOff = false;
	_leftDelta = _rightDelta = 0;
	_upDelta = _downDelta = 0;
	_scrollConst = 0;
	_roomNumber = 0;
}

void Player::load() {
	if (_vm->_room->_roomFlag == 3) {
		_playerOffset.x = _vm->_screen->_scaleTable1[8];
		_playerOffset.y = _vm->_screen->_scaleTable1[11];
		_leftDelta = 0;
		_rightDelta = 8;
		_upDelta = 2;
		_downDelta = -2;
		_scrollConst = 2;

		for (int i = 0; i < PLAYER_DATA_COUNT; ++i) {
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
		_vm->_timers[8]._flag = true;

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
		_guard = Common::Point(56, 190);
	} else {
		_playerOffset.x = _vm->_screen->_scaleTable1[25];
		_playerOffset.y = _vm->_screen->_scaleTable1[67];
		_leftDelta = -3;
		_rightDelta = 33;
		_upDelta = 5;
		_downDelta = -10;
		_scrollConst = 5;

		for (int i = 0; i < PLAYER_DATA_COUNT; ++i) {
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

		_sideWalkMin = 0;
		_sideWalkMax = 7;
		_upWalkMin = 16;
		_upWalkMax = 23;
		_downWalkMin = 8;
		_downWalkMax = 15;
		_diagUpWalkMin = 0;
		_diagUpWalkMax = 7;
		_diagDownWalkMin = 0;
		_diagDownWalkMax = 7;
	}

	_vm->_man = _vm->_man1;
	if (_vm->_manPal1) {
		Common::copy(_vm->_manPal1 + 0x270, _vm->_manPal1 + 0x270 + 0x60, _vm->_screen->_manPal);
	} else {
		Common::fill(_vm->_screen->_manPal, _vm->_screen->_manPal + 0x60, 0);
	}
}

void Player::calcManScale() {
	if (!_vm->_manScaleOff) {
		_vm->_scale = (((_rawPlayer.y - _vm->_scaleMaxY + _vm->_scaleN1) * 
			_vm->_scaleT1 + (_vm->_scaleH2 << 8)) / _vm->_scaleH1 * _vm->_scaleI) >> 8;
		_vm->_screen->setScaleTable(_vm->_scale);

		_playerOffset.x = _vm->_screen->_scaleTable1[20];
		_playerOffset.y = _vm->_screen->_scaleTable1[67];
	}
}

void Player::walk() {
	warning("TODO: Player::walk");
}

void Player::calcPlayer() {
	Screen &scr = *_vm->_screen;
	scr._bufferStart.x = (scr._scrollCol << 4) + scr._scrollX;
	scr._bufferStart.y = (scr._scrollRow << 4) + scr._scrollY;
	_playerX = _rawPlayer.x - scr._bufferStart.x;
	_playerY = _rawPlayer.y - scr._bufferStart.y;
}

} // End of namespace Access
