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

#include "access/data.h"
#include "common/algorithm.h"

namespace Access {

Player::Player() {
	_field0 = 0;
	_monData = nullptr;
	Common::fill(&_walkOffRight[0], &_walkOffRight[PLAYER_DATA_COUNT], 0);
	Common::fill(&_walkOffLeft[0], &_walkOffLeft[PLAYER_DATA_COUNT], 0);
	Common::fill(&_walkOffUp[0], &_walkOffUp[PLAYER_DATA_COUNT], 0);
	Common::fill(&_walkOffDown[0], &_walkOffDown[PLAYER_DATA_COUNT], 0);
	_rawTempL = 0;
	_rawXTemp = 0;
	_rawYTempL = 0;
	_rawYTemp = 0;
	_playerXLow = 0;
	_playerX = 0;
	_playerYLow = 0;
	_playerY = 0;
	_frame = 0;
}

} // End of namespace Access
