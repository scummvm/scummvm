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

#ifndef ACCESS_PLAYER_H
#define ACCESS_PLAYER_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace Access {

#define PLAYER_DATA_COUNT 8

class AccessEngine;

class Player {
private:
	AccessEngine *_vm;
public:
	// Fields in original Player structure
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

	// Additional globals we've added to new Player class
	bool _playerOff;
public:
	Player(AccessEngine *vm);

	void walk();
};

} // End of namespace Access

#endif /* ACCESS_PLAYER_H */
