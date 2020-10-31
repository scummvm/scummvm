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

#ifndef WORLD_ACTORS_NPC_DAT_H
#define WORLD_ACTORS_NPC_DAT_H

#include "ultima/ultima8/filesys/raw_archive.h"

namespace Ultima {
namespace Ultima8 {

class NPCDat {
public:
	NPCDat();

	static Std::vector<NPCDat *> load(RawArchive *archive);

	const Std::string &getName() const {
		return _name;
	};

	uint16 getShapeNo() const {
		return _shapeNo;
	};

	uint16 getMinHp() const {
		return _minHp;
	};

	uint16 getMaxHp() const {
		return _maxHp;
	};

	uint16 getWpnType() const {
		return _wpnType;
	};

	uint16 getWpnType2() const {
		return _wpnType2;
	};

	uint16 getDefaultActivity(int no) const {
		assert(no >= 0 && no < 3);
		return _defaultActivity[no];
	}

	//!< A function for randomly assigning stronger weapons for the highest difficulty level.
	static uint16 randomlyGetStrongerWeaponTypes(uint shapeno);

private:
    NPCDat(Common::SeekableReadStream &datars, Common::SeekableReadStream &namers);

	Std::string _name;
	uint16 _minHp;
	uint16 _maxHp;
	uint16 _shapeNo;
	uint16 _wpnType;
	uint16 _wpnType2;
	uint16 _defaultActivity[3];  // activities 0x6, 0x8, and 0xA in game.
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
