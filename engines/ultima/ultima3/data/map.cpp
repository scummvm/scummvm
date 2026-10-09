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
#include "ultima/ultima3/data/map.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

bool Map::exists(const char *filename) {
	return Common::File::exists(filename);
}

void Map::load(const char *filename) {
	Common::File f;
	if (!f.open(filename))
		error("Could not load %s", filename);

	Common::Serializer s(&f, nullptr);
	synchronizeData(s);
	if (f.err() || f.eos())
		error("Could not load %s", filename);
}

void Map::synchronize(Common::Serializer &s) {
	// Earlier saves only held the cells
	if (s.getVersion() < 3)
		s.syncBytes(_cells, sizeof(_cells));
	else
		synchronizeData(s);
}

void Map::synchronizeData(Common::Serializer &s) {
	s.syncBytes(_cells, sizeof(_cells));
	s.syncBytes(_text, sizeof(_text));
	s.syncBytes(_creatures._tile, CREATURE_COUNT);
	s.syncBytes(_creatures._floor, CREATURE_COUNT);
	s.syncBytes(_creatures._x, CREATURE_COUNT);
	s.syncBytes(_creatures._y, CREATURE_COUNT);
	s.syncBytes(_creatures._flags, CREATURE_COUNT);
	s.syncBytes(_extra, sizeof(_extra));
}

} // namespace Data
} // namespace Ultima3
} // namespace Ultima
