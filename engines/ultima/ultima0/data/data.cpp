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

#include "ultima/ultima0/data/data.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {

const _OInfStruct OBJECT_INFO[] = {
	{ "Food", 1, 0, 'F' },
	{ "Rapier", 8, 10, 'R' },
	{ "Axe", 5, 5, 'A' },
	{ "Shield", 6, 1, 'S' },
	{ "Bow and Arrow", 3, 4, 'B' },
	{ "Magic Amulet", 15, 0, 'M' }
};

const _MInfStruct MONSTER_INFO[] = {
	{ "Skeleton", 1 },
	{ "Thief", 2 },
	{ "Giant Rat", 3 },
	{ "Orc", 4 },
	{ "Viper", 5 },
	{ "Carrion Crawler", 6 },
	{ "Gremlin", 7 },
	{ "Mimic", 8 },
	{ "Daemon", 9 },
	{ "Balrog", 10 }
};

const char *ATTRIB_NAMES[] = { "Hit Points", "Strength", "Dexterity", "Stamina", "Wisdom", "Gold" };


void PLAYER::init() {
	Common::fill(Name, Name + MAX_NAME + 1, '\0');
	World.x = World.y = 0;
	Dungeon.x = Dungeon.y = 0;
	DungDir.x = DungDir.y = 0;
	Class = '?';
	HPGain = 0;
	LuckyNumber = 0;
	Level = 0;
	Skill = 0;
	Task = 0;
	TaskCompleted = 0;
	Common::fill(Attr, Attr + MAX_ATTR, 0);
	Common::fill(Object, Object + MAX_OBJ, 0);
}

void PLAYER::rollAttributes() {
	for (int i = 0; i < MAX_ATTR; ++i)
		Attr[i] = g_engine->getRandomNumber(21) + 4;
}

/*-------------------------------------------------------------------*/

void WORLDMAP::init(PLAYER &p) {
	int c, x, y, Size;

	g_engine->setRandomSeed(p.LuckyNumber);
	Size = MapSize;					// Get hard-coded map size

	// Set the boundaries
	for (x = 0; x <= Size; x++) {
		Map[Size][x] = WT_MOUNTAIN;
		Map[0][x] = WT_MOUNTAIN;
		Map[x][Size] = WT_MOUNTAIN;
		Map[x][0] = WT_MOUNTAIN;
	}

	// Set up the map contents
	for (x = 1; x < Size; x++) {
		for (y = 1; y < Size; y++) {
			c = (int)(pow(RND(), 5.0) * 4.5);	// Calculate what's there
			if (c == WT_TOWN && RND() > .5)		// Remove half the towns
				c = WT_SPACE;
			Map[x][y] = c;
		}
	}

	// Calculate player start
	x = g_engine->getRandomNumber(1, Size - 1);
	y = g_engine->getRandomNumber(1, Size - 1);
	p.World.x = x; p.World.y = y;			// Save it
	Map[x][y] = WT_TOWN;					// Make it a town

	// Find place for castle
	do {
		x = g_engine->getRandomNumber(1, Size - 1);
		y = g_engine->getRandomNumber(1, Size - 1);
	} while (Map[x][y] != WT_SPACE);

	Map[x][y] = WT_BRITISH;				// Put LBs castle there
}

} // namespace Ultima0
} // namespace Ultima
