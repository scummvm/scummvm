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

	for (int i = 0; i < 4; i++)
		_offsets[i] = rs.readUint16LE();

	int datasize = rs.size();
	rs.seek(0, SEEK_SET);
	_data = new uint8[datasize];

	_dataLen = rs.read(_data, datasize);
}

CombatDat::~CombatDat() {
	delete [] _data;
}

} // End of namespace Ultima8
} // End of namespace Ultima
