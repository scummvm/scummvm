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

#ifndef WORLD_ACTORS_COMBAT_DAT_H
#define WORLD_ACTORS_COMBAT_DAT_H

#include "common/stream.h"

#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

/**
 * A single entry in the Crusader combat.dat flex.  The files consist of 3 parts:
 * 1. human-readable name (zero-padded 16 bytes)
 * 2. offset table (10x2-byte offsets, in practice only the first 2 offsets are ever used)
 * 3. tactic blocks starting at the offsets given in the offset (in practice only 2 blocks are used)
 *
 * The tactic blocks are a sequence of opcodes of things the NPC should
 * do - eg, turn towards direction X.
 */
class CombatDat {
public:
	CombatDat(Common::SeekableReadStream &rs);

    ~CombatDat();

	const Std::string &getName() const {
		return _name;
	};

	const uint8 *getData() const {
		return _data;
	}

	uint16 getOffset(int block) const {
		assert(block < ARRAYSIZE(_offsets));
		return _offsets[block];
	}

	uint16 getDataLen() const {
		return _dataLen;
	}

private:
	Std::string _name;

	uint16 _offsets[4];
	uint8 *_data;
	uint16 _dataLen;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
