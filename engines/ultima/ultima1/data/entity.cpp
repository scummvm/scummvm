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

#include "ultima/ultima1/data/entity.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

const char *OVERWORLD_MONSTERS[] = {
	"Ness creature",
	"Giant squid",
	"Dragon turtle",
	"Pirate ship",
	"Hood",
	"Bear",
	"Hidden archer",
	"Dark knight",
	"Evil trent",
	"Thief",
	"Orc",
	"Knight",
	"Necromancer",
	"Evil ranger",
	"Wandering warlock"
};

const int OVERWORLD_MONSTERS_DAMAGE[MONSTER_TYPE_COUNT] = {
	10, 5, 4, 3, 2, 1, 4, 6, 8, 10, 1, 2, 4, 6, 8
};

const int OVERWORLD_MONSTERS_HITS[MONSTER_TYPE_COUNT] = {
	4, 6, 8, 10, 1, 2, 4, 6, 8, 2, 4, 6, 8, 9, 10
};

const int MONSTER_THRESHOLDS_PER_LEVEL[10] = {
	35, 55, 75, 95, 115, 135, 155, 175, 195, 205
};

void OverworldEntity::synchronize(Common::Serializer &s) {
	s.syncMultipleLE(_type, _data, _x, _y, _hits, _unused1, _unused2, _unused3);
}

int OverworldEntity::getMonsterAttackDistance(int tileNum, int xDiff, int yDiff) {
	int threshold = (tileNum == 23 || tileNum == 25 || tileNum == 31 || tileNum == 47) ? 3 : 1;
	int maxDiff = MAX(ABS(xDiff), ABS(yDiff));

	return (maxDiff > threshold) ? 0 : threshold;
}


} // namespace Data
} // namespace Ultima1
} // namespace Ultima
