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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/world/actors/npc_dat.h"

#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/ultima8.h"
#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

NPCDat::NPCDat(Common::SeekableReadStream &rs, Common::SeekableReadStream &namers) {
	char namebuf[33] = {0};
	namers.read(namebuf, 32);
	_name.assign(namebuf);

	_minHp = rs.readUint16LE();
	_maxHp = rs.readUint16LE();
	//
	rs.skip(20);
	// offset 0x18 (24): wpntable offset
	_wpnType2 = rs.readUint16LE();
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


/*static*/
uint16 NPCDat::randomlyGetStrongerWeaponTypes(uint shapeno) {
	// Apologies for the massive stack of constants, that's how
	// it is in the original (fn at 10a0:3b10) :(

	// This also combines the version from No Regret, which is the same
	// for 899, 0x371, 0x385, 0x1b4, 0x2cb, 0x528, 0x338, 0x4d1, 0x4e6,
	// and other differences as noted.
	// Some shapes are only valid in each game, but that's ok.

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	uint rnd = rs.getRandomNumber(UINT_MAX);

	switch (shapeno) {
	case 899:	/* shape 899 - android */
		if (rnd % 3 == 0)
			return 10;
		else
			return 7;
	case 0x2fd:
	case 0x319: /* shape 793 - guardsq */
		if (GAME_IS_REMORSE) {
			if (rnd % 4 == 0)
				return 0xc;
			else
				return 3;
		} else {
			if (rnd % 2 == 0)
				return 8;
			else
				return 9;
		}
	case 0x1b4:
		if (rnd % 4 == 0)
			return 0xd;
		else
			return 9;
	case 0x2cb: /* shape 715 - roaming (robot) */
		if (rnd % 2 == 0)
			return 3;
		else
			return 7;
	case 0x338: /* shape 824 - thermatr (robot) */
		if (rnd % 3 == 0)
			return 5;
		else
			return 7;
	case 0x371:
		if (rnd % 3 == 0)
			return 9;
		else
			return 10;
	case 0x4d1:
		if (rnd % 2 == 0)
			return 4;
		else
			return 0xb;
	case 900:
		if (rnd % 3 == 0)
			return 5;
		else
			return 10;
	case 0x385:
		if (rnd % 4 == 0)
			return 8;
		else
			return 9;
	case 0x3ac:
		if (rnd % 2 == 0)
			return 9;
		else
			return 0xd;
	case 0x4e6:
		if (rnd % 3 == 0)
			return 5;
		else
			return 0xb;
	case 0x528:
		if (rnd % 3 == 0)
			return 9;
		else
			return 8;
	case 0x30c: // for No Remorse
		if (rnd % 2 == 0)
			return 4;
		else
			return 0xf;
	case 0x308: // for No Remorse
		if (rnd % 2 == 0)
			return 10;
		else
			return 0xb;
	case 0x57a: // for No Remorse
		if (rnd % 2 == 0)
			return 0xd;
		else
			return 0xf;
	case 0x5e2: // for No Remorse
			return 0xe;
	default:
		return GAME_IS_REMORSE ? 7 : 0xf;
	}
}


} // End of namespace Ultima8
} // End of namespace Ultima
