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

#ifndef XEEN_COMBAT_H
#define XEEN_COMBAT_H

#include "common/scummsys.h"

namespace Xeen {

class XeenEngine;

class Combat {
private:
	XeenEngine *_vm;
public:
	int _charsArray1[12];
	bool _monPow[12];
	int _monsterScale[12];
	int _elemPow[12];
	int _elemScale[12];
	int _attackMon1, _attackMon2, _attackMon3;
public:
	Combat(XeenEngine *vm);
};

} // End of namespace Xeen

#endif	/* XEEN_COMBAT_H */
