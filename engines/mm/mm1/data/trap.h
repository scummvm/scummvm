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

#ifndef MM1_DATA_TRAP_H
#define MM1_DATA_TRAP_H

#include "common/scummsys.h"
#include "mm/mm1/game/game_logic.h"

namespace MM {
namespace MM1 {

class TrapData : public Game::GameLogic {
private:
	/**
	 * Damages a party member from the trap
	 */
	void damageChar(uint partyIndex);

protected:
	static int8 RESISTANCE_INDEXES[11];
	static byte CONDITIONS1[11];
	static byte CONDITIONS2[11];
	static byte CONDITIONS3[11];
	static byte DAMAGE_TYPE[7];

	int _trapType = 0;
	int _hpInitial = 0;
	int _reduced = 0;
	int _resistanceIndex = 0;
	byte _condition = 0;

	virtual void trap();
public:
	virtual ~TrapData() {}
};

} // namespace MM1
} // namespace MM

#endif
