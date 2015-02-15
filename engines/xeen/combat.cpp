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

#include "xeen/combat.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Xeen {

Combat::Combat(XeenEngine *vm): _vm(vm) {
	Common::fill(&_attackMonsters[0], &_attackMonsters[26], 0);
	Common::fill(&_charsArray1[0], &_charsArray1[12], 0);
	Common::fill(&_monPow[0], &_monPow[12], 0);
	Common::fill(&_monsterScale[0], &_monsterScale[12], 0);
	Common::fill(&_elemPow[0], &_elemPow[12], 0);
	Common::fill(&_elemScale[0], &_elemScale[12], 0);
	Common::fill(&_shooting[0], &_shooting[8], 0);
	_globalCombat = 0;
	_whosTurn = -1;
	_itemFlag = false;
}

void Combat::clear() {
	Common::fill(&_attackMonsters[0], &_attackMonsters[26], -1);
}

void Combat::doCombat() {
	error("TODO: doCombat");
}


} // End of namespace Xeen
