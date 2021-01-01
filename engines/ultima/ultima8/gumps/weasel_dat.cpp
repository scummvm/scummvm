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

#include "ultima/ultima8/gumps/weasel_dat.h"
#include "common/util.h"
#include "common/stream.h"

namespace Ultima {
namespace Ultima8 {

static const int BLOCKS = 20;

WeaselDat::WeaselDat(Common::ReadStream *rs) {
	uint16 numentries = rs->readUint16LE();
	if (numentries > BLOCKS)
		numentries = BLOCKS;

	// each block is 16 bytes
	for (uint i = 0; i < numentries; i++) {
		WeaselEntry entry;
		// 4 byte string ID
		for (int j = 0; j < 4; j++)
			entry._id[j] = rs->readByte();

		// Unknown 4 bytes
		rs->readUint16LE();
		rs->readUint16LE();

		// Shapeno (2 bytes)
		entry._shapeNo = rs->readUint16LE();
		// Cost (2 bytes)
		entry._cost = rs->readUint16LE();
		entry._entryNo = rs->readUint16LE();
		entry._unk = rs->readUint16LE();
		if (entry._id[0] == 'W')
			entry._type = kWeapon;
		else if (entry._id[0] == 'I')
			entry._type = kItem;
		else
			entry._type = kUnknown;

		_items.push_back(entry);
	}

	const uint skip = (BLOCKS - numentries) * 16;
	for (uint i = 0; i < skip; i++)
		rs->readByte();
}

uint16 WeaselDat::getNumOfType(WeaselType type) const {
	int count = 0;
	for (Std::vector<WeaselEntry>::const_iterator iter = _items.begin(); iter != _items.end(); iter++) {
		if (iter->_type == type)
			count++;
	}
	return count;
}


} // End of namespace Ultima8
} // End of namespace Ultima
