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

#ifndef ACCESS_DATA_H
#define ACCESS_DATA_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace Access {

struct TimerEntry {
	int _initTm;
	int _timer;
	bool _flag;

	TimerEntry() {
		_initTm = _timer = 0;
		_flag = false;
	}
};

#define PLAYER_DATA_COUNT 8

class Player {
public:
	int _field0;
	byte *_monData;
	int _walkOffRight[PLAYER_DATA_COUNT];
	int _walkOffLeft[PLAYER_DATA_COUNT];
	int _walkOffUp[PLAYER_DATA_COUNT];
	int _walkOffDown[PLAYER_DATA_COUNT];
	Common::Point _walkOffUR[PLAYER_DATA_COUNT];
	Common::Point _walkOffDR[PLAYER_DATA_COUNT];
	Common::Point _walkOffUL[PLAYER_DATA_COUNT];
	Common::Point _walkOffDL[PLAYER_DATA_COUNT];
	int _rawTempL;
	int _rawXTemp;
	int _rawYTempL;
	int _rawYTemp;
	Common::Point _playerOffset;
	int _playerXLow;
	int _playerX;
	int _playerYLow;
	int _playerY;
	int _frame;
public:
	Player();
};

} // End of namespace Access

#endif /* ACCESS_DATA_H */
