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
#include "ultima/ultima8/world/actors/npc_dat.h"

#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

NPCDat::NPCDat(Common::SeekableReadStream &rs, Common::SeekableReadStream &namers) {
	char namebuf[33] = {0};
	namers.read(namebuf, 32);
	_name.assign(namebuf);

	_minHp = rs.readUint16LE();
	_maxHp = rs.readUint16LE();
	// TODO: Read the other data.
	rs.skip(22);
	// offset 0x1a (26): wpntype
	_wpnType = rs.readUint16LE();
	rs.skip(2);
	// offset 30: default activity 0x6
	_defaultActivity[0] = rs.readUint16LE();
	// offset 0x3e (62): shape
	rs.skip(62 - 32);
	_shapeNo = rs.readUint16LE();
	// offset 64: default activity 0x8
	_defaultActivity[1] = rs.readUint16LE();
	// offset 66: default activity 0xA
	_defaultActivity[2] = rs.readUint16LE();
	rs.skip(142 - 68);
}

/*static*/
Std::vector<NPCDat *> NPCDat::load(RawArchive *archive) {
    Std::vector<NPCDat *> result;
    assert(archive);
	if (archive->getCount() < 2) {
		warning("NPCDat: Archive does not include the expected objects.");
		return result;
	}

	Common::MemoryReadStream datars(archive->get_object_nodel(0), archive->get_size(0));
	Common::MemoryReadStream namers(archive->get_object_nodel(2), archive->get_size(2));

	if (!datars.size() || !namers.size()) {
		warning("NPCDat: Archive appears to be corrupt.");
		return result;
	}

	while (!datars.eos() && !namers.eos() && (datars.size() - datars.pos() >= 142)) {
		result.push_back(new NPCDat(datars, namers));
	}

	return result;
}

} // End of namespace Ultima8
} // End of namespace Ultima
