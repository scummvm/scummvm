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
#include "ultima/ultima3/data/arena.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

void Arena::load(const char *filename) {
	Common::File f;
	if (!f.open(filename))
		error("Could not load %s", filename);

	f.read(_tiles, sizeof(_tiles));
	f.skip(7);
	f.read(_monsterX, sizeof(_monsterX));
	f.read(_monsterY, sizeof(_monsterY));
	f.read(_monsterUnder, sizeof(_monsterUnder));
	f.read(_monsterHp, sizeof(_monsterHp));
	f.read(_playerX, sizeof(_playerX));
	f.read(_playerY, sizeof(_playerY));
	f.read(_playerUnder, sizeof(_playerUnder));
	f.read(_playerTile, sizeof(_playerTile));

	if (f.err() || f.eos())
		error("Could not load %s", filename);

	// What's in the file for these is left over from the last time it was used
	memset(_monsterUnder, 0, sizeof(_monsterUnder));
	memset(_monsterHp, 0, sizeof(_monsterHp));
	memset(_playerUnder, 0, sizeof(_playerUnder));
	memset(_playerTile, 0, sizeof(_playerTile));
}

} // namespace Data
} // namespace Ultima3
} // namespace Ultima
