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

#include "scumm/he/moonbase/map_katton.h"

namespace Scumm {

KattonGenerator::KattonGenerator(int seed) {
	_seed = seed;
}

int KattonGenerator::getRandomNumber() {
	// This is the exact linear congruential generator
	// algorithm used on MSVCRT (Windows Visual C++ Runtime), with
	// RAND_MAX being 0x7fff (32767).  This is implemented here
	// to match the RNG between the original Moonbase Console
	// program and ScummVM.
	//
	// (Common::RandomSource uses Xorshift and uses unsigned
	// integers compared to MSVCRT's rand)
	_seed = _seed * 214013 + 2531011;
	return (_seed >> 16) & 0x7fff;
}

MapFile *KattonGenerator::generateMap(int water, int tileSet, int mapSize, int energy, int terrain) {
	_size = mapSize;
	_tileset = tileSet;
	int inEnergy = energy;
	int inTerrain = terrain;
	int inWater = water;

	if (!(inEnergy == 0)) {
		inEnergy = inEnergy * 14 + plusminus(14);
	}

	if (!(inTerrain == 0)) {
		inTerrain = inTerrain * 14 + plusminus(14);
	}

	if (!(inWater == 0)) {
		inWater = inWater * 14 + plusminus(14);
	}

	float terrainpercent = ((float)inTerrain) / 100;
	float waterpercent = ((float)inWater) / 100;

	//****************************************Let the generation BEGIN!

	fillboards(0);

	// used everywhere
	int i, j, k, l, x, y, z;
	// used in making the basic landmass shapes
	int numsplotches, length;
	// used in making water
	int numwaterplaces = 0, numwatersplotches, multiplier;
	short int goodwater[1600][2];
	// used in making energy
	int maxnumclose = 0, maxnumfar = 0, maxnumrand = 0, smallmed = 0, goodplaceClose[300][2], goodplaceFar[300][2], numplaceClose, numplaceFar, placeFar[3], placeClose[2], counter, counterpools;
	int stringiness, randomx, randomy;

	//****************************************Make the basic landmass shapes.

	numsplotches = (int)(_size / 4) + (int)((terrainpercent - 0.5) * _size / 2);

	for (i = 0; i < numsplotches; i++) {
		length = (int)(((1 - waterpercent) / 6 * _size * _size) / numsplotches);
		length = length + plusminus((int)(length / 2));
		stringiness = 1 + plusminus(1);
		randomx = getRandomNumber() % _size;
		randomy = getRandomNumber() % _size;
		randomsplotch(length, 1 + stringiness, 2, randomx, randomy);
	}

	for (i = 0; i < (int)numsplotches / 4; i++) {
		length = (int)(((1 - waterpercent) / 3 * _size * _size) / numsplotches);
		length = length + plusminus((int)(length / 4));
		stringiness = 1 + plusminus(1);
		randomx = getRandomNumber() % _size;
		randomy = getRandomNumber() % _size;
		randomsplotch(length, stringiness, 1, randomx, randomy);
	}

	for (i = 0; i < (int)numsplotches / 2; i++) {
		length = (int)(_size + plusminus((int)(_size / 2)));
		stringiness = 1 + plusminus(1);
		randomx = getRandomNumber() % _size;
		randomy = getRandomNumber() % _size;
		randomsplotch(length, stringiness, 0, randomx, randomy);
	}

	for (i = 0; i < (int)waterpercent * _size; i++) {
		length = (int)(_size + plusminus((int)(_size / 2)));
		stringiness = 1 + plusminus(1);
		randomx = getRandomNumber() % _size;
		randomy = getRandomNumber() % _size;
		randomsplotch(length, stringiness, 0, randomx, randomy);
	}

	//****************************************Fatten up the landmasses

	// make the highhills bigger
	fattenall(-3, 2, NEVER_USED_NUM, TEMP_REPLACEMENT_NUMA);
	replacenum(TEMP_REPLACEMENT_NUMA, 2);

	// make the lowlands bigger, depending on wateriness
	fattenall(-((int)(waterpercent * 3)), 0, NEVER_USED_NUM, TEMP_REPLACEMENT_NUMA);
	replacenum(TEMP_REPLACEMENT_NUMA, 2);

	// surround the high hills with at least one med
	fattenall(2, 2, NEVER_USED_NUM, 1);

	// fatten the medium hills
	fattenall(-3, 1, 2, TEMP_REPLACEMENT_NUMA);
	replacenum(TEMP_REPLACEMENT_NUMA, 1);

	//****************************************Smooth out/rough up the landmasses

	randomflip((int)(terrainpercent * terrainpercent * _size * _size / 4 + _size / 4), inWater);

	//****************************************Make the start locations

	findstartloc();

	//****************************************Put down some water

	for (i = 0; i < _size; i += 2) {
		for (j = 0; j < _size; j += 2) {
			if (goodforwater(i, j)) {
				goodwater[numwaterplaces][0] = i;
				goodwater[numwaterplaces][1] = j;
				numwaterplaces++;
			}
		}
	}

	numwatersplotches = (int)(_size * waterpercent * waterpercent * 5);

	if (numwaterplaces <= numwatersplotches) {
		numwatersplotches = numwaterplaces;
	}
	if (numwatersplotches >= 1) {
		multiplier = (int)((float)numwaterplaces / (float)numwatersplotches);

		for (i = 0; i < numwatersplotches; i++) {
			z = getRandomNumber() % multiplier + i * multiplier;
			x = goodwater[z][0];
			y = goodwater[z][1];
			length = _size + plusminus(_size / 2);
			stringiness = getRandomNumber() % 3;
			randomwater(length, stringiness, x, y);
		}
	}

	//****************************************Put down the energy
	if (inEnergy >= 90) {
		maxnumclose = 2;
		maxnumfar = 3;
		maxnumrand = (int)(_size * _size / 150);
		smallmed = 2;
	} else if (inEnergy >= 75) {
		maxnumclose = 2;
		maxnumfar = 1;
		maxnumrand = (int)(_size * _size / 250) + 4;
		smallmed = 3;
	} else if (inEnergy >= 60) {
		maxnumclose = 1;
		maxnumfar = 2;
		maxnumrand = (int)(_size * _size / 250);
		smallmed = 4;
	} else if (inEnergy >= 45) {
		maxnumclose = 1;
		maxnumfar = 1;
		maxnumrand = (int)(_size * _size / 250);
		smallmed = 6;
	} else if (inEnergy >= 30) {
		maxnumclose = 1;
		maxnumfar = 0;
		maxnumrand = (int)(_size * _size / 350) + 2;
		smallmed = 8;
	} else if (inEnergy >= 15) {
		maxnumclose = 0;
		maxnumfar = 1;
		maxnumrand = (int)(_size * _size / 450);
		smallmed = 10;
	} else if (inEnergy >= 0) {
		maxnumclose = 0;
		maxnumfar = 0;
		maxnumrand = 0;
		smallmed = 1;
	}

	for (i = 0; i < 4; i++) {
		numplaceClose = 0;
		numplaceFar = 0;
		for (j = 0; j < _size; j++) {
			for (k = 0; k < _size; k++) {
				if ((goodforenergy(j, k, 0)) && (distance(_startloc[i][0], _startloc[i][1], j, k) >= (int)(_size / 10)) && (distance(_startloc[i][0], _startloc[i][1], j, k) <= (int)(_size / 8))) {
					goodplaceClose[numplaceClose][0] = j;
					goodplaceClose[numplaceClose][1] = k;
					numplaceClose++;
				} else if ((goodforenergy(j, k, 0)) && (distance(_startloc[i][0], _startloc[i][1], j, k) >= (int)(_size / 7)) && (distance(_startloc[i][0], _startloc[i][1], j, k) <= (int)(_size / 5))) {
					goodplaceFar[numplaceFar][0] = j;
					goodplaceFar[numplaceFar][1] = k;
					numplaceFar++;
				}
			}
		}
		if (numplaceClose >= 1) {
			placeClose[0] = getRandomNumber() % (int)(numplaceClose / 2) + 1;
		}
		if (numplaceClose >= 2) {
			placeClose[1] = getRandomNumber() % (int)(numplaceClose / 2) + (int)(numplaceClose / 2) - 1;
		}

		if (numplaceClose >= maxnumclose) {
			for (l = 0; l < maxnumclose; l++) {
				_special[goodplaceClose[placeClose[l]][0]][goodplaceClose[placeClose[l]][1]] = 100;
			}
		} else {
			for (l = 0; l < numplaceClose; l++) {
				_special[goodplaceClose[placeClose[l]][0]][goodplaceClose[placeClose[l]][1]] = 100;
			}
		}

		if (numplaceFar >= 1) {
			placeFar[0] = getRandomNumber() % (int)(numplaceFar / 3) + 1;
		}
		if (numplaceFar >= 2) {
			placeFar[1] = getRandomNumber() % (int)(numplaceFar / 3) + (int)(numplaceClose / 3);
		}
		if (numplaceFar >= 3) {
			placeFar[2] = getRandomNumber() % (int)(numplaceFar / 3) + (int)(2 * numplaceClose / 3) - 1;
		}

		if (numplaceFar >= maxnumfar) {
			for (l = 0; l < maxnumfar; l++) {
				_special[goodplaceFar[placeFar[l]][0]][goodplaceFar[placeFar[l]][1]] = 100;
			}
		} else {
			for (l = 0; l < numplaceFar; l++) {
				_special[goodplaceFar[placeFar[l]][0]][goodplaceFar[placeFar[l]][1]] = 100;
			}
		}
	}

	counter = 0;
	counterpools = 4 * (maxnumfar + maxnumclose);
	for (k = 0; k < maxnumrand && counterpools < 50; k++) {
		do {
			i = getRandomNumber() % _size;
			j = getRandomNumber() % _size;
			counter++;
		} while (!((distance(i, j, _startloc[0][0], _startloc[0][1]) >= 10) && (distance(i, j, _startloc[1][0], _startloc[1][1]) >= 10) && (distance(i, j, _startloc[2][0], _startloc[2][1]) >= 10) && (distance(i, j, _startloc[3][0], _startloc[3][1]) >= 10) && (goodforenergy(i, j, 1)) && (counter < 5000)));
		if (getRandomNumber() % smallmed == 0) {
			_special[i][j] = 200;
			counterpools++;
		} else {
			_special[i][j] = 100;
			counterpools++;
		}
	}

	//****************************************Do that saving thing that you do, BABY!
	MIF mif = MIF();
	Common::sprintf_s(mif._name, "Katton %04X", (uint16)_seed);
	mif._dimension = _size;
	mif._mapType = _tileset;
	for (j = 0; j < _size; j++) {
		for (i = 0; i < _size; i++)
			mif._cornerMap[i][j] = _board[i][j];
		for (i = 0; i < _size; i++)
			if (_special[i][j] == 0)
				mif._centerMap[i][j] = '.';
			else if (_special[i][j] == -1)
				mif._centerMap[i][j] = 'W';
			else if (_special[i][j] == 100)
				mif._centerMap[i][j] = 'S';
			else if (_special[i][j] == 200)
				mif._centerMap[i][j] = 'M';
			else if (_special[i][j] == 300)
				mif._centerMap[i][j] = 'L';
			else
				mif._centerMap[i][j] = -_special[i][j];
	}

	// Generate new map:
	MapFile *map = new MapFile();
	mif.generateMap(map);

	return map;
}

int KattonGenerator::distance(int x1, int y1, int x2, int y2) {
	int dx, dy, disp;
	dx = min((abs(x1 - x2)), (abs(x1 + _size - x2)), (abs(x2 + _size - x1)));
	dy = min((abs(y1 - y2)), (abs(y1 + _size - y2)), (abs(y2 + _size - y1)));
	disp = (int)sqrt((double)(dx * dx + dy * dy));
	return disp;
}

int KattonGenerator::min(int a, int b, int c) {
	if ((a <= b) && (a <= c)) {
		return a;
	} else if ((b < a) && (b <= c)) {
		return b;
	} else if ((c < a) && (c < b)) {
		return c;
	} else {
		return a;
	}
}

int KattonGenerator::goodforenergy(int x, int y, int poolsize) {
	switch (poolsize) {
	case 0:
		if ((_board[x][y] == _board[findcoord(x, +1)][y]) && (_board[findcoord(x, +1)][y] == _board[x][findcoord(y, +1)]) && (_board[x][findcoord(y, +1)] == _board[findcoord(x, +1)][findcoord(y, +1)])) {
			//check main map
			if (_special[x][y] == 0) { //specials are clear
				return 1;
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	case 1:
		if ((_board[x][y] == _board[findcoord(x, +1)][y]) && (_board[findcoord(x, +1)][y] == _board[x][findcoord(y, +1)]) && (_board[x][findcoord(y, +1)] == _board[findcoord(x, +1)][findcoord(y, +1)])) { //check main map
			if ((_special[x][y] == 0) && (_special[findcoord(x, +1)][y] == 0) && (_special[x][findcoord(y, +1)] == 0) && (_special[findcoord(x, +1)][findcoord(y, +1)] == 0)) {                           //specials are clear
				return 1;
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	default:
		return 0;
	}
}

int KattonGenerator::plusminus(int max) {
	int result = getRandomNumber() % (max + 1);
	if (getRandomNumber() % 2 == 0) {
		result *= (-1);
	}
	return result;
}

int KattonGenerator::fillboards(int num) {
	int i, j;
	for (i = 0; i < _size; i++) {
		for (j = 0; j < _size; j++) {
			_board[i][j] = num;
			_special[i][j] = num;
		}
	}
	return 0;
}

int KattonGenerator::randomplace(int numberofplaces, int placer) {
	int i, randx, randy;
	for (i = 0; i < numberofplaces; i++) {
		randx = (getRandomNumber() % _size);
		randy = (getRandomNumber() % _size);
		_board[randx][randy] = placer;
	}
	return 0;
}

int KattonGenerator::randomsplotch(int length, int stringiness, int placer, int x, int y) {
	int currx, curry, direction = 10, prevdirection, movex = 0, movey = 0, i = 0;
	currx = x;
	curry = y;
	while (i <= length) {
		_board[currx][curry] = placer;
		prevdirection = direction;
		direction = (getRandomNumber() % 4);
		if ((((direction + 2) == prevdirection) || ((direction - 2) == prevdirection)) && (stringiness == 2)) {
			direction = prevdirection;
		}
		if (!((((direction + 2) == prevdirection) || ((direction - 2) == prevdirection)) && (stringiness == 1))) {
			switch (direction) {
			case 0:
				movex = 0;
				movey = 1;
				break;
			case 1:
				movex = 1;
				movey = 0;
				break;
			case 2:
				movex = 0;
				movey = -1;
				break;
			case 3:
				movex = -1;
				movey = 0;
				break;
			}
			currx = findcoord(currx, movex);
			curry = findcoord(curry, movey);
			i++;
		}
	}
	return 0;
}

int KattonGenerator::findcoord(int value, int move) {
	move = move % _size;
	int final = value + move;
	if (final < 0) {
		final = _size + final;
	}
	if (final >= _size) {
		final = final % _size;
	}
	return final;
}

int KattonGenerator::replacenum(int replacee, int replacer) {
	int i, j;
	for (j = 0; j < _size; j++) {
		for (i = 0; i < _size; i++) {
			if (_board[i][j] == replacee) {
				_board[i][j] = replacer;
			}
		}
	}
	return 0;
}

int KattonGenerator::fattenall(int howfat, int middle, int ignorer, int replacer) {
	int i, j, temp;
	for (j = 0; j < _size; j++) {
		for (i = 0; i < _size; i++) {
			if (_board[i][j] == middle) {
				if (howfat <= 0) {
					temp = (int)(abs(howfat) + plusminus(2));
					if (temp <= 1) {
						temp = 2;
					} else if (temp >= 6) {
						temp = 5;
					}
					fattenone(i, j, temp, middle, ignorer, replacer);
				} else {
					fattenone(i, j, howfat, middle, ignorer, replacer);
				}
			}
		}
	}
	return 0;
}
int KattonGenerator::fattenone(int x, int y, int howfat, int middle, int ignorer, int replacer) {
	if (howfat == -100) {
		_board[x][y] = replacer;
		_board[findcoord(x, +1)][y] = replacer;
		_board[x][findcoord(y, +1)] = replacer;
		_board[findcoord(x, +1)][findcoord(y, +1)] = replacer;
	}
	if (howfat >= 1) {
		_board[x][findcoord(y, -1)] = ((_board[x][findcoord(y, -1)] == middle) || (_board[x][findcoord(y, -1)] == ignorer)) ? _board[x][findcoord(y, -1)] : replacer;
		_board[x][findcoord(y, +1)] = ((_board[x][findcoord(y, +1)] == middle) || (_board[x][findcoord(y, +1)] == ignorer)) ? _board[x][findcoord(y, +1)] : replacer;
		_board[findcoord(x, -1)][y] = ((_board[findcoord(x, -1)][y] == middle) || (_board[findcoord(x, -1)][y] == ignorer)) ? _board[findcoord(x, -1)][y] : replacer;
		_board[findcoord(x, +1)][y] = ((_board[findcoord(x, +1)][y] == middle) || (_board[findcoord(x, +1)][y] == ignorer)) ? _board[findcoord(x, +1)][y] : replacer;
	}
	if (howfat >= 2) {
		_board[findcoord(x, -1)][findcoord(y, -1)] = ((_board[findcoord(x, -1)][findcoord(y, -1)] == middle) || (_board[findcoord(x, -1)][findcoord(y, -1)] == ignorer)) ? _board[findcoord(x, -1)][findcoord(y, -1)] : replacer;
		_board[findcoord(x, -1)][findcoord(y, +1)] = ((_board[findcoord(x, -1)][findcoord(y, +1)] == middle) || (_board[findcoord(x, -1)][findcoord(y, +1)] == ignorer)) ? _board[findcoord(x, -1)][findcoord(y, +1)] : replacer;
		_board[findcoord(x, +1)][findcoord(y, -1)] = ((_board[findcoord(x, +1)][findcoord(y, -1)] == middle) || (_board[findcoord(x, +1)][findcoord(y, -1)] == ignorer)) ? _board[findcoord(x, +1)][findcoord(y, -1)] : replacer;
		_board[findcoord(x, +1)][findcoord(y, +1)] = ((_board[findcoord(x, +1)][findcoord(y, +1)] == middle) || (_board[findcoord(x, +1)][findcoord(y, +1)] == ignorer)) ? _board[findcoord(x, +1)][findcoord(y, +1)] : replacer;
	}
	if (howfat >= 3) {
		_board[x][findcoord(y, -2)] = ((_board[x][findcoord(y, -2)] == middle) || (_board[x][findcoord(y, -2)] == ignorer)) ? _board[x][findcoord(y, -2)] : replacer;
		_board[x][findcoord(y, +2)] = ((_board[x][findcoord(y, +2)] == middle) || (_board[x][findcoord(y, +2)] == ignorer)) ? _board[x][findcoord(y, +2)] : replacer;
		_board[findcoord(x, -2)][y] = ((_board[findcoord(x, -2)][y] == middle) || (_board[findcoord(x, -2)][y] == ignorer)) ? _board[findcoord(x, -2)][y] : replacer;
		_board[findcoord(x, +2)][y] = ((_board[findcoord(x, +2)][y] == middle) || (_board[findcoord(x, +2)][y] == ignorer)) ? _board[findcoord(x, +2)][y] : replacer;
	}
	if (howfat >= 4) {
		_board[findcoord(x, -1)][findcoord(y, -2)] = ((_board[findcoord(x, -1)][findcoord(y, -2)] == middle) || (_board[findcoord(x, -1)][findcoord(y, -2)] == ignorer)) ? _board[findcoord(x, -1)][findcoord(y, -2)] : replacer;
		_board[findcoord(x, -1)][findcoord(y, +2)] = ((_board[findcoord(x, -1)][findcoord(y, +2)] == middle) || (_board[findcoord(x, -1)][findcoord(y, +2)] == ignorer)) ? _board[findcoord(x, -1)][findcoord(y, +2)] : replacer;
		_board[findcoord(x, +1)][findcoord(y, -2)] = ((_board[findcoord(x, +1)][findcoord(y, -2)] == middle) || (_board[findcoord(x, +1)][findcoord(y, -2)] == ignorer)) ? _board[findcoord(x, +1)][findcoord(y, -2)] : replacer;
		_board[findcoord(x, +1)][findcoord(y, +2)] = ((_board[findcoord(x, +1)][findcoord(y, +2)] == middle) || (_board[findcoord(x, +1)][findcoord(y, +2)] == ignorer)) ? _board[findcoord(x, +1)][findcoord(y, +2)] : replacer;

		_board[findcoord(x, -2)][findcoord(y, -1)] = ((_board[findcoord(x, -2)][findcoord(y, -1)] == middle) || (_board[findcoord(x, -2)][findcoord(y, -1)] == ignorer)) ? _board[findcoord(x, -2)][findcoord(y, -1)] : replacer;
		_board[findcoord(x, -2)][findcoord(y, +1)] = ((_board[findcoord(x, -2)][findcoord(y, +1)] == middle) || (_board[findcoord(x, -2)][findcoord(y, +1)] == ignorer)) ? _board[findcoord(x, -2)][findcoord(y, +1)] : replacer;
		_board[findcoord(x, +2)][findcoord(y, -1)] = ((_board[findcoord(x, +2)][findcoord(y, -1)] == middle) || (_board[findcoord(x, +2)][findcoord(y, -1)] == ignorer)) ? _board[findcoord(x, +2)][findcoord(y, -1)] : replacer;
		_board[findcoord(x, +2)][findcoord(y, +1)] = ((_board[findcoord(x, +2)][findcoord(y, +1)] == middle) || (_board[findcoord(x, +2)][findcoord(y, +1)] == ignorer)) ? _board[findcoord(x, +2)][findcoord(y, +1)] : replacer;
	}
	if (howfat >= 5) {
		_board[findcoord(x, -2)][findcoord(y, -2)] = ((_board[findcoord(x, -2)][findcoord(y, -2)] == middle) || (_board[findcoord(x, -2)][findcoord(y, -2)] == ignorer)) ? _board[findcoord(x, -2)][findcoord(y, -2)] : replacer;
		_board[findcoord(x, -2)][findcoord(y, +2)] = ((_board[findcoord(x, -2)][findcoord(y, +2)] == middle) || (_board[findcoord(x, -2)][findcoord(y, +2)] == ignorer)) ? _board[findcoord(x, -2)][findcoord(y, +2)] : replacer;
		_board[findcoord(x, +2)][findcoord(y, -2)] = ((_board[findcoord(x, +2)][findcoord(y, -2)] == middle) || (_board[findcoord(x, +2)][findcoord(y, -2)] == ignorer)) ? _board[findcoord(x, +2)][findcoord(y, -2)] : replacer;
		_board[findcoord(x, +2)][findcoord(y, +2)] = ((_board[findcoord(x, +2)][findcoord(y, +2)] == middle) || (_board[findcoord(x, +2)][findcoord(y, +2)] == ignorer)) ? _board[findcoord(x, +2)][findcoord(y, +2)] : replacer;

		_board[x][findcoord(y, -3)] = ((_board[x][findcoord(y, -3)] == middle) || (_board[x][findcoord(y, -3)] == ignorer)) ? _board[x][findcoord(y, -3)] : replacer;
		_board[x][findcoord(y, +3)] = ((_board[x][findcoord(y, +3)] == middle) || (_board[x][findcoord(y, +3)] == ignorer)) ? _board[x][findcoord(y, +3)] : replacer;
		_board[findcoord(x, -3)][y] = ((_board[findcoord(x, -3)][y] == middle) || (_board[findcoord(x, -3)][y] == ignorer)) ? _board[findcoord(x, -3)][y] : replacer;
		_board[findcoord(x, +3)][y] = ((_board[findcoord(x, +3)][y] == middle) || (_board[findcoord(x, +3)][y] == ignorer)) ? _board[findcoord(x, +3)][y] : replacer;
	}
	return 0;
}

int KattonGenerator::findstartloc() {
	int temp, i, j, shiftx, shifty, secondshift;
	int start[4][2];
	shiftx = getRandomNumber() % _size;
	shifty = getRandomNumber() % _size;
	start[0][0] = findcoord((int)_size / 4, (plusminus(3) + shiftx));
	start[0][1] = findcoord((int)_size / 4, (plusminus(3) + shifty));
	start[1][0] = findcoord((int)3 * _size / 4, (plusminus(3) + shiftx));
	start[1][1] = findcoord((int)_size / 4, (plusminus(3) + shifty));
	start[2][0] = findcoord((int)_size / 4, (plusminus(3) + shiftx));
	start[2][1] = findcoord((int)3 * _size / 4, (plusminus(3) + shifty));
	start[3][0] = findcoord((int)3 * _size / 4, (plusminus(3) + shiftx));
	start[3][1] = findcoord((int)3 * _size / 4, (plusminus(3) + shifty));

	temp = getRandomNumber() % 2;
	secondshift = getRandomNumber() % _size;

	if (temp == 0) {
		start[0][0] = findcoord(start[0][0], secondshift);
		start[1][0] = findcoord(start[1][0], secondshift);
	}

	else {
		start[1][1] = findcoord(start[1][1], secondshift);
		start[3][1] = findcoord(start[3][1], secondshift);
	}

	temp = whatheightstartloc(start[0][0], start[0][1]);
	fattenone(start[0][0], start[0][1], -100, temp, NEVER_USED_NUM, temp);

	temp = whatheightstartloc(start[1][0], start[1][1]);
	fattenone(start[1][0], start[1][1], -100, temp, NEVER_USED_NUM, temp);

	temp = whatheightstartloc(start[2][0], start[2][1]);
	fattenone(start[2][0], start[2][1], -100, temp, NEVER_USED_NUM, temp);

	temp = whatheightstartloc(start[3][0], start[3][1]);
	fattenone(start[3][0], start[3][1], -100, temp, NEVER_USED_NUM, temp);

	for (j = 0; j < 4; j++) {
		for (i = 0; i < 2; i++) {
			_startloc[j][i] = start[j][i];
		}
	}

	temp = getRandomNumber() % 4;
	j = 4;
	for (i = 0; i < 4; i++) {
		if (temp == i) {
		} else {
			_startloc[j][0] = start[i][0];
			_startloc[j][1] = start[i][1];
			j++;
		}
	}

	_startloc[7][0] = _startloc[4][0];
	_startloc[7][1] = _startloc[4][1];
	_startloc[8][0] = _startloc[6][0];
	_startloc[8][1] = _startloc[6][1];

	for (j = 9; j < 13; j++) {
		for (i = 0; i < 2; i++) {
			_startloc[j][i] = start[j - 9][i];
		}
	}

	for (j = 13; j < 17; j++) {
		for (i = 0; i < 2; i++) {
			_startloc[j][i] = start[j - 13][i];
		}
	}

	for (j = 17; j < 20; j++) {
		for (i = 0; i < 2; i++) {
			_startloc[j][i] = _startloc[j - 13][i];
		}
	}

	// place on special map
	for (i = 0; i < 4; i++) {
		_special[_startloc[i][0]][_startloc[i][1]] += 1;
	}
	for (i = 4; i < 7; i++) {
		_special[_startloc[i][0]][_startloc[i][1]] += 2;
	}
	for (i = 7; i < 9; i++) {
		_special[_startloc[i][0]][_startloc[i][1]] += 4;
	}
	for (i = 9; i < 13; i++) {
		_special[_startloc[i][0]][_startloc[i][1]] += 8;
	}
	for (i = 13; i < 17; i++) {
		_special[_startloc[i][0]][_startloc[i][1]] += 16;
	}
	for (i = 17; i < 20; i++) {
		_special[_startloc[i][0]][_startloc[i][1]] += 32;
	}
	return 0;
}

int KattonGenerator::whatheightstartloc(int x, int y) {
	int heightfield[3] = {0, 0, 0};

	heightfield[_board[findcoord(x, +2)][findcoord(y, -1)]]++;
	heightfield[_board[findcoord(x, +2)][y]]++;
	heightfield[_board[findcoord(x, +2)][findcoord(y, +1)]]++;
	heightfield[_board[findcoord(x, +2)][findcoord(y, +2)]]++;

	heightfield[_board[findcoord(x, -1)][findcoord(y, -1)]]++;
	heightfield[_board[findcoord(x, -1)][y]]++;
	heightfield[_board[findcoord(x, -1)][findcoord(y, +1)]]++;
	heightfield[_board[findcoord(x, -1)][findcoord(y, +2)]]++;

	heightfield[_board[x][findcoord(y, -1)]]++;
	heightfield[_board[x][y]]++;
	heightfield[_board[x][findcoord(y, +1)]]++;
	heightfield[_board[x][findcoord(y, +2)]]++;

	heightfield[_board[findcoord(x, +1)][findcoord(y, -1)]]++;
	heightfield[_board[findcoord(x, +1)][y]]++;
	heightfield[_board[findcoord(x, +1)][findcoord(y, +1)]]++;
	heightfield[_board[findcoord(x, +1)][findcoord(y, +2)]]++;

	if (heightfield[0] == 0) {
		if (heightfield[1] >= heightfield[2]) {
			return 1;
		} else {
			return 2;
		}
	} else if (heightfield[1] == 0) {
		if (heightfield[0] >= heightfield[2]) {
			return 0;
		} else {
			return 2;
		}
	} else if (heightfield[2] == 0) {
		if (heightfield[1] >= heightfield[0]) {
			return 1;
		} else {
			return 0;
		}
	} else {
		return 1;
	}
}

int KattonGenerator::goodforwater(int x, int y) {
	if ((_board[x][y] == 0) && (_board[findcoord(x, +1)][y] == 0) && (_board[x][findcoord(y, +1)] == 0) && (_board[findcoord(x, +1)][findcoord(y, +1)] == 0)) {                                                                                                                                                                                                                                                       //check main map
		if ((_special[x][y] <= 0) && (_special[x][findcoord(y, 1)] <= 0) && (_special[findcoord(x, 1)][findcoord(y, 1)] <= 0) && (_special[findcoord(x, 1)][y] <= 0) && (_special[findcoord(x, 1)][findcoord(y, -1)] <= 0) && (_special[x][findcoord(y, -1)] <= 0) && (_special[findcoord(x, -1)][findcoord(y, -1)] <= 0) && (_special[findcoord(x, -1)][y] <= 0) && (_special[findcoord(x, -1)][findcoord(y, 1)] <= 0)) { //specials are clear
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

int KattonGenerator::randomwater(int length, int stringiness, int x, int y) {
	int currx, curry, direction = 10, prevdirection, i = 0;
	currx = x;
	curry = y;
	while (i <= length) {
		_special[currx][curry] = -1;
		prevdirection = direction;
		direction = (getRandomNumber() % 4);
		if ((((direction + 2) == prevdirection) || ((direction - 2) == prevdirection)) && (stringiness == 2)) {
			direction = prevdirection;
		}
		if (!((((direction + 2) == prevdirection) || ((direction - 2) == prevdirection)) && (stringiness == 1))) {
			switch (direction) {

			case 0: //north
				if (goodforwater(currx, findcoord(curry, 1))) {
					curry = findcoord(curry, 1);
				}
				break;

			case 1: //east
				if (goodforwater(findcoord(currx, 1), curry)) {
					currx = findcoord(currx, 1);
				}
				break;

			case 2: //south
				if (goodforwater(currx, findcoord(curry, -1))) {
					curry = findcoord(curry, -1);
				}
				break;

			case 3: //west
				if (goodforwater(findcoord(currx, -1), curry)) {
					currx = findcoord(currx, -1);
				}
				break;
			}
			i++;
		}
	}
	return 0;
}

int KattonGenerator::tileaverage(int x, int y, int threshold) {
	int heightfield[3] = {0, 0, 0};

	heightfield[_board[findcoord(x, -1)][findcoord(y, -1)]]++;
	heightfield[_board[findcoord(x, -1)][y]]++;
	heightfield[_board[findcoord(x, -1)][findcoord(y, +1)]]++;

	heightfield[_board[x][findcoord(y, -1)]]++;
	heightfield[_board[x][y]]++;
	heightfield[_board[x][findcoord(y, +1)]]++;

	heightfield[_board[findcoord(x, +1)][findcoord(y, -1)]]++;
	heightfield[_board[findcoord(x, +1)][y]]++;
	heightfield[_board[findcoord(x, +1)][findcoord(y, +1)]]++;

	if ((heightfield[2] == 0) && (heightfield[1] < heightfield[0]) && (heightfield[0] >= threshold)) {
		_board[x][y] = 0;
		return 0;
	} else if ((heightfield[0] == 0) && (heightfield[1] < heightfield[2]) && (heightfield[2] >= threshold)) {
		_board[x][y] = 2;
		return 2;
	} else if (heightfield[1] >= threshold) {
		_board[x][y] = 1;
		return 1;
	} else {
		return 0;
	}
}

int KattonGenerator::randomflip(int numberofplaces, int inWater) {
	int i, x, y, temp;
	for (i = 0; i < numberofplaces; i++) {
		x = getRandomNumber() % _size;
		y = getRandomNumber() % _size;
		if (_board[x][y] == 0) {
			if (inWater == 0) {
				temp = 0;
			} else {
				temp = getRandomNumber() % inWater;
			}

			if (temp <= 50) {
				_board[x][y] = 1;
			}
		} else if (_board[x][y] == 2) {
			_board[x][y] = 1;
		} else if (_board[x][y] == 1) {

			temp = getRandomNumber() % 2;
			int heightfield[3] = {0, 0, 0};

			heightfield[_board[findcoord(x, -1)][findcoord(y, -1)]]++;
			heightfield[_board[findcoord(x, -1)][y]]++;
			heightfield[_board[findcoord(x, -1)][findcoord(y, +1)]]++;

			heightfield[_board[x][findcoord(y, -1)]]++;
			heightfield[_board[x][findcoord(y, +1)]]++;

			heightfield[_board[findcoord(x, +1)][findcoord(y, -1)]]++;
			heightfield[_board[findcoord(x, +1)][y]]++;
			heightfield[_board[findcoord(x, +1)][findcoord(y, +1)]]++;
			temp = getRandomNumber() % 2;
			if (heightfield[0] == 0) {
				_board[x][y] = 2;
			} else if (heightfield[2] == 0) {
				_board[x][y] = 0;
			}
		}
	}
	return 0;
}

} // End of namespace Scumm
