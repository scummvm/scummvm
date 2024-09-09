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

#ifndef SCUMM_HE_MOONBASE_MAP_KATTON_H
#define SCUMM_HE_MOONBASE_MAP_KATTON_H

#ifdef ENABLE_HE

#include "engines/scumm/he/moonbase/map_mif.h"

#define TEMP_REPLACEMENT_NUMA 5
#define NEVER_USED_NUM 99

namespace Scumm {

class KattonGenerator {
public:
	KattonGenerator(int seed);
	~KattonGenerator();

	MapFile *generateMap(int water, int tileSet, int mapSize, int energy, int terrain);

private:
	int _seed;

	int _size; // 32, 40, 48, or 56
	int _tileset;
	int _startloc[20][2];
	int _board[MAX_TILE_COUNT][MAX_TILE_COUNT];
	int _special[MAX_TILE_COUNT][MAX_TILE_COUNT];

	int getRandomNumber();

	int min(int a, int b, int c);
	int distance(int x1, int y1, int x2, int y2);
	int plusminus(int max);
	int fillboards(int num);
	int randomplace(int numberofplaces, int placer);
	int randomflip(int numberofplaces, int inWater);
	// stringiness: 0 = no change/random, 1 = no back, 2 = back goes forwards
	int randomsplotch(int length, int stringiness, int placer, int x, int y);
	int goodforwater(int x, int y);
	int randomwater(int length, int stringiness, int x, int y);
	int goodforenergy(int x, int y, int poolsize);
	int findcoord(int value, int move);
	int replacenum(int replacee, int replacer);
	int fattenone(int x, int y, int howfat, int middle, int ignorer, int replacer);
	// howfat: positive 1-5 for distance, -100 to 0 for random 3 spread from 2 to 5.
	int fattenall(int howfat, int middle, int ignorer, int replacer);
	int findstartloc();
	int whatheightstartloc(int x, int y);
	int tileaverage(int x, int y, int threshold);
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_MOONBASE_MAP_KATTON_H
