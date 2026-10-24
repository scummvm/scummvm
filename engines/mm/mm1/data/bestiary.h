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

#ifndef MM1_DATA_BESTIARY_H
#define MM1_DATA_BESTIARY_H

#include "common/stream.h"
#include "mm/mm1/data/monsters.h"

namespace MM {
namespace MM1 {

/**
 * Tracks which monsters the party has encountered. It's saved as an optional
 * extension after the normal savegame data, so older saves still load
 */
class BestiaryState {
private:
	byte _seen[(MONSTERS_COUNT + 7) / 8] = {};

public:
	void clear() {
		memset(_seen, 0, sizeof(_seen));
	}

	bool hasSeen(uint id) const {
		return id >= 1 && id <= MONSTERS_COUNT &&
			(_seen[(id - 1) / 8] & (1 << ((id - 1) % 8)));
	}

	void discover(uint id) {
		if (id >= 1 && id <= MONSTERS_COUNT)
			_seen[(id - 1) / 8] |= 1 << ((id - 1) % 8);
	}

	uint count() const {
		uint result = 0;
		for (uint id = 1; id <= MONSTERS_COUNT; ++id) {
			if (hasSeen(id))
				++result;
		}

		return result;
	}

	void save(Common::WriteStream &stream) const {
		stream.writeUint32BE(0x42535431);	// 'BST1' extension marker
		stream.writeUint16BE(MONSTERS_COUNT);
		stream.write(_seen, sizeof(_seen));
	}

	void load(Common::SeekableReadStream &stream) {
		clear();

		const int64 start = stream.pos();
		if (stream.size() - start < 6 + (int64)sizeof(_seen))
			return;

		if (stream.readUint32BE() != 0x42535431 ||
				stream.readUint16BE() != MONSTERS_COUNT) {
			stream.seek(start);
			return;
		}

		if (stream.read(_seen, sizeof(_seen)) != sizeof(_seen))
			clear();
	}
};

} // namespace MM1
} // namespace MM

#endif
