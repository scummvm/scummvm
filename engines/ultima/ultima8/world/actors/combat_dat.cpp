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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/world/actors/combat_dat.h"

namespace Ultima {
namespace Ultima8 {

CombatDat::CombatDat(Common::SeekableReadStream &rs) {
	char namebuf[17] = {0};
	rs.read(namebuf, 16);
	_name.assign(namebuf);

	uint16 offset1 = rs.readUint16LE();
	uint16 offset2 = rs.readUint16LE();

	int data1size = offset2 - offset1;
	int data2size = rs.size() - offset2;
	_sequence1 = new uint8[data1size];
	_sequence2 = new uint8[data2size];

	rs.seek(offset1);
	_sequence1len = rs.read(_sequence1, data1size);

	rs.seek(offset2);
	_sequence2len = rs.read(_sequence2, data2size);
}

CombatDat::~CombatDat() {
	delete [] _sequence1;
	delete [] _sequence2;
}

} // End of namespace Ultima8
} // End of namespace Ultima
