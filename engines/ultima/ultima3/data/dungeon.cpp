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

#include "common/file.h"
#include "ultima/ultima3/data/dungeon.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

void Dungeon::load(const char *filename) {
	Common::File f;
	if (!f.open(filename))
		error("Could not load %s", filename);

	Common::Serializer s(&f, nullptr);
	synchronize(s);
	if (f.err() || f.eos())
		error("Could not load %s", filename);
}

const char *Dungeon::sign(int level) const {
	if (level < 0 || level >= DUNGEON_LEVELS)
		return nullptr;

	// The strings are found by offsets listed at the start
	int offset = _signs[level * 2] | (_signs[level * 2 + 1] << 8);
	if (offset >= SIGNS_SIZE || !memchr(_signs + offset, 0, SIGNS_SIZE - offset))
		return nullptr;

	return (const char *)_signs + offset;
}

void Dungeon::synchronize(Common::Serializer &s) {
	s.syncBytes(_tiles, sizeof(_tiles));
	s.syncBytes(_signs, sizeof(_signs));
}

} // namespace Data
} // namespace Ultima3
} // namespace Ultima
