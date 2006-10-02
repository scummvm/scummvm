/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id: vga.cpp 24023 2006-09-30 02:15:02Z kirben $
 *
 */

#include "common/stdafx.h"

#include "common/system.h"

#include "agos/agos.h"

namespace AGOS {

static const byte tileDefs[] = {
	2,0,0,0,
	3,0,0,0,
	2,0,4,0,
	3,0,5,0,
	4,0,6,0,
	5,0,7,0,
	6,0,8,0,
	7,0,9,0,
	8,0,10,0,
	9,0,11,0,
	10,0,0,0,
	11,0,0,0,
	14,85,0,0,
	15,86,0,0,
	14,85,16,87,
	15,86,17,88,
	16,87,18,89,
	17,88,19,90,
	18,89,0,0,
	19,90,0,0,
	22,0,0,0,
	23,91,0,0,
	22,0,24,92,
	23,91,25,93,
	24,92,26,94,
	25,93,27,95,
	26,94,28,96,
	27,95,29,0,
	28,96,0,0,
	29,0,0,0,
	32,0,0,0,
	31,0,33,0,
	32,0,34,97,
	33,0,35,98,
	34,97,36,99,
	35,98,37,100,
	36,99,38,101,
	37,100,39,102,
	38,101,40,0,
	39,102,41,0,
	40,0,42,0,
	41,0,0,0,
	44,0,0,0,
	43,0,45,0,
	44,0,46,103,
	45,0,47,104,
	46,103,48,105,
	47,104,49,106,
	48,105,50,107,
	49,106,51,108,
	50,107,52,0,
	51,108,53,0,
	52,0,54,0,
	53,0,0,0,
	56,0,0,0,
	57,109,0,0,
	56,0,58,110,
	57,109,59,111,
	58,110,60,112,
	59,111,61,113,
	60,112,62,114,
	61,113,63,0,
	62,114,0,0,
	63,0,0,0,
	66,115,0,0,
	67,116,0,0,
	66,115,68,117,
	67,116,69,118,
	68,117,70,119,
	69,118,71,120,
	70,119,0,0,
	71,120,0,0,
	74,0,0,0,
	75,0,0,0,	
	74,0,76,0,
	75,0,77,0,
	76,0,78,0,
	77,0,79,0,
	78,0,80,0,
	79,0,81,0,
	80,0,82,0,
	81,0,83,0,
	82,0,0,0,
	83,0,0,0,
	86,0,0,0,
	87,0,0,0,
	86,0,88,0,
	87,0,89,0,
	88,0,0,0,
	89,0,0,0,	
	92,121,0,0,
	93,122,0,0,
	92,121,94,123,
	93,122,95,124,
	94,123,0,0,
	95,124,0,0,
	98,125,0,0,
	99,126,0,0,
	98,125,100,127,
	99,126,101,128,
	100,127,0,0,
	101,128,0,0,
	104,129,0,0,
	105,130,0,0,
	104,129,106,131,
	105,130,107,132,
	106,131,0,0,
	107,132,0,0,
	110,133,0,0,
	111,134,0,0,
	110,133,112,135,
	111,134,113,136,
	112,135,0,0,
	113,136,0,0,
	116,0,0,0,
	117,0,0,0,
	116,0,118,0,
	117,0,119,0,
	118,0,0,0,
	119,0,0,0,
	122,0,0,0,
	123,0,0,0,			
	122,0,0,0,	
	123,0,0,0,
	126,137,0,0,
	127,138,0,0,
	126,137,0,0,
	127,138,0,0,
	130,139,0,0,
	131,140,0,0,
	130,139,0,0,
	131,140,0,0,
	134,0,0,0,
	135,0,0,0,
	134,0,0,0,
	135,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,	
	31,0,43,0,
	42,0,54,0,
	143,0,0,0,
};

static const byte tileDefs1[] = {
	14,13,14,15,0,0,0,0,
	15,14,15,16,0,0,0,0,
	16,15,16,17,0,0,0,0,
	17,16,17,18,0,0,0,0,
	18,17,18,19,0,0,0,0,
	19,18,19,20,0,0,0,0,
	23,22,23,24,0,0,0,0,
	24,23,24,25,0,0,0,0,
	25,24,25,26,0,0,0,0,
	26,25,26,27,0,0,0,0,
	27,26,27,28,0,0,0,0,
	28,27,28,29,0,0,0,0,
	34,33,34,35,0,0,0,0,
	35,34,35,36,0,0,0,0,
	36,35,36,37,0,0,0,0,
	37,36,37,38,0,0,0,0,
	38,37,38,39,0,0,0,0,
	39,38,39,40,0,0,0,0,
	46,45,46,47,0,0,0,0,
	47,46,47,48,0,0,0,0,
	48,47,48,49,0,0,0,0,
	49,48,49,50,0,0,0,0,
	50,49,50,51,0,0,0,0,
	51,50,51,52,0,0,0,0,
	57,56,57,58,0,0,0,0,
	58,57,58,59,0,0,0,0,
	59,58,59,60,0,0,0,0,
	60,59,60,61,0,0,0,0,
	61,60,61,62,0,0,0,0,
	62,61,62,63,0,0,0,0,
	66,65,66,67,0,0,0,0,
	67,66,67,68,0,0,0,0,
	68,67,68,69,0,0,0,0,
	69,68,69,70,0,0,0,0,
	70,69,70,71,0,0,0,0,
	71,70,71,72,0,0,0,0,
	92,91,92,93,0,0,0,0,
	93,92,93,94,0,0,0,0,
	94,93,94,95,0,0,0,0,
	95,94,95,96,0,0,0,0,
	98,97,98,99,0,0,0,0,
	99,98,99,100,0,0,0,0,
	100,99,100,101,0,0,0,0,
	101,100,101,102,0,0,0,0,
	104,103,104,105,0,0,0,0,
	105,104,105,106,0,0,0,0,
	106,105,106,107,0,0,0,0,
	107,106,107,108,0,0,0,0,
	110,109,110,111,0,0,0,0,
	111,110,111,112,0,0,0,0,
	112,111,112,113,0,0,0,0,
	113,112,113,114,0,0,0,0,
	126,125,126,127,0,0,0,0,
	127,126,127,128,0,0,0,0,
	130,129,130,131,0,0,0,0,
	131,130,131,132,0,0,0,0,
	137,0,138,0,139,0,140,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
};

bool AGOSEngine::block0Supported(int i, int y) {
	if (y == 0) {
		// Always supported at bottom level
		return true;
	}

	int a = _variableArray[i];

	while(1) {
		if (_variableArray[i - 8] != 0) {
			// Supported
			return true;
		}

		i++;

		if (_variableArray[i] != a) {
			// Got to end of block and found no support
			return false;
		}
	}
}

bool AGOSEngine::block1Supported(int i, int y) {
	if (y == 0) {
		// Always supported at bottom level
		return true;
	}

	int a = _variableArray[i];

	while(1) {
		if(_variableArray[i - 8] != 0) {
			// Supported
			return true;
		}

		i += 128;

		if (_variableArray[i] != a) {
			// Got to end of block and found no support
			return false;
		}
	}
}

void AGOSEngine::checkTiles() {
	printf("checkTiles\n");

	const byte *tile;
	uint i, slot;

	slot = _variableArray[400] - 1;

	tile = &tileDefs[slot * 4 + 1];
	for (i = 0; i < 2; i++) {
		if (tile[1] != 0 && findBox(tile[1]) != 0)
			continue;
		if (tile[0] != 0)
			enableBox(tile[0]);
		tile += 2;
	}

	tile = &tileDefs1[slot * 8 + 1];
	for (i = 0; i < 4; i++) {
		if (tile[1] != 0 && findBox(tile[1]) != 0)
			continue;
		if (tile[0] != 0)
			enableBox(tile[0]);
		tile += 2;
	}
}

void AGOSEngine::moveBlocks() {
	printf("moveBlocks\n");

	VgaSprite *vsp;
	const byte *vcPtrOrg;
	uint16 tmp;
	uint num, i;

	num = _variableArray[414] + 1;

	for (i = 0; i < num; i++) {
		_vgaCurSpriteId = _variableArray[410];
		_vgaCurZoneNum = 60;

		vsp = findCurSprite();
		vsp->x += _variableArray[411];
		vsp->y += _variableArray[412];

		tmp = to16Wrapper(_variableArray[413]);

		vcPtrOrg = _vcPtr;
		_vcPtr = (byte *)&tmp;
		vc23_setSpritePriority();
		_vcPtr = vcPtrOrg;
		
		_variableArray[410] += 1;
		_variableArray[413] -= 8;
	}

	setBitFlag(99, false);
	_marks |= (1 << 1);
}

void AGOSEngine::dropBlock(int id, int priority, int num) {
	VgaSprite *vsp;
	const byte *vcPtrOrg;
	uint16 tmp;
	int i;

	for (i = 0; i < num; i++) {
		_vgaCurSpriteId = id;
		_vgaCurZoneNum = 60;

		vsp = findCurSprite();
		vsp->y += 20;

		tmp = to16Wrapper(priority);

		vcPtrOrg = _vcPtr;
		_vcPtr = (byte *)&tmp;
		vc23_setSpritePriority();
		_vcPtr = vcPtrOrg;
		
		id += 1;
		priority -= 8;
	}
}

void AGOSEngine::droppingBlock() {
	if (_droppingBlockAnim == 0)
		return;

	printf("droppingBlock\n");

	VgaSprite *vsp;
	const byte *vcPtrOrg;
	uint i, result, speed;
	uint16 tmp;

	speed = (getBitFlag(82) != 0) ? 10 : 5;
	for (i = 0; i < _droppingBlockLength; i++) {
		_vgaCurSpriteId = _droppingBlockAnim + i;
		_vgaCurZoneNum = 60;
		vsp = findCurSprite();
		vsp->y += speed;
	}

	_vgaCurSpriteId = _droppingBlockAnim;
	_vgaCurZoneNum = 60;
	vsp = findCurSprite();

	result = _droppingBlockCount - 1;
	if (result  == 0 || (speed == 10 && result == 2)) {
		result = hasBlockLanded(vsp->y, _droppingBlockAnim, vsp->priority, _droppingBlockType, _droppingBlockX, _droppingBlockY, _droppingBlockZ);
		result += 4;
		if (result == 0) {
			_droppingBlockAnim = 0;
			_droppingBlockLand = 0;
			setBitFlag(94, 0);
		}
	}

	_droppingBlockCount = result;
	if (result == 3) {
		for (i = 0; i < _droppingBlockLength; i++) {
			vsp = findCurSprite();

			tmp = to16Wrapper(vsp->priority - 64);

			vcPtrOrg = _vcPtr;
			_vcPtr = (byte *)&tmp;
			vc23_setSpritePriority();
			_vcPtr = vcPtrOrg;
	
			_vgaCurSpriteId += 1;
		}
	}

	if (_droppingBlockLand == 0) {
		droppingBlock();
	}
}


void AGOSEngine::stopBlock(int id, int num) {
	const byte *vcPtrOrg;
	uint16 i, items[2];

	for (i = 0; i < num; i++) {
		items[0] = to16Wrapper(60);
		items[1] = to16Wrapper(id);

		vcPtrOrg = _vcPtr;
		_vcPtr = (byte *)&items;
		vc60_killSprite();
		_vcPtr = vcPtrOrg;

		id++;
	}
}

void AGOSEngine::remove0Block(int i) {
	int a = _variableArray[i];
	if(a == 0)
		return;
	while (_variableArray[i] == a)
		_variableArray[i++] = 0;
	stopBlock(a, 0);
	a = a & 0xff80;		// Point to flags 420 - 425 containing counts
	a = a / 128;		// For each colour block
	a += 419;					
	_variableArray[a]--;
}

void AGOSEngine::remove1Block(int i) {
	int a = _variableArray[i];
	int b = -1;
	if (a == 0)
		return;
	while (_variableArray[i] == a) {
		_variableArray[i] = 0;
		i += 128;
		b++;
	}
	stopBlock(a, b);
	a = a & 0xff80;		// Point to flags 420 - 425 containing counts
	a -= 1024;
	a = a / 128;		// for each colour block
	a += 419;					
	_variableArray[a]--;
}

void AGOSEngine::pairCheck() {
	printf("pairCheck\n");

	int i, j, j1, k;
	int x, y, z;
	int flag;
	int block;
	int blockColour;
	int pairsList0[50][2];
	int pairsList1[50][2];

	j = 0;

	// For each level
	for (y = 0; y < 16; y++) {
		// Check Type 0 blocks - blocks totally covered by others
		// further away along Z axis

 		for (z = 0; z < 7; z++) {
			flag = 1000 + z * 128 + y * 8;
			// This assumes minimum block length of 3
			for (x = 0; x < 6; x++) {
				block = _variableArray[flag + x];
				blockColour = block & 0xff80;
				if (block < 999 && block != 0) {
					i = x;
					while (1) {
						// Not same colour?
						if (((_variableArray[flag + x + 128]) & 0xff80) != blockColour) {
							// To end of block
							x++;
							while (_variableArray[flag + x] == block)
								x++;
							x--;
							break;
						}
						x++;
						if (_variableArray[flag + x] != block) {
							// Got a pair
							i += flag;
							k = i + 128;
							block = _variableArray[k];
							// Back to start of covering block
							k--;
							while (_variableArray[k] == block)
								k--;
							k++;
							if (block0Supported(i, y) && block0Supported(k, y)) {
								// Form a list and remove blocks when finished check
								pairsList0[j][0] = i;
								pairsList0[j++][1] = k;
							}
							x--;
							break;
						}
					}
				}
			}
		}

		// Check Type 0 blocks - blocks totally covered by others
		// further towards along Z axis

 		for (z = 7; z > 0; z--) {
			flag = 1000 + z * 128 + y * 8;
			// This assumes minimum block length of 3
			for (x = 0; x < 6; x++) {
				block = _variableArray[flag + x];
				blockColour = block & 0xff80;
				if (block < 999 && block != 0) {
					i = x;
					while (1) {
						// Not same colour
						if (((_variableArray[flag + x - 128]) & 0xff80) != blockColour) {
							// To end of block
							x++;
							while (_variableArray[flag + x] == block)
								x++;
							x--;
							break;
						}
						x++;
						if (_variableArray[flag + x] != block) {
							// Got a pair
							i += flag;
							k = i - 128;
							block = _variableArray[k];
							// Back to start of covering block
							k--;
							while (_variableArray[k]==block)
								k--;
							k++;
							if (block0Supported(i, y) && block0Supported(k, y)) {
								// Form a list and remove blocks when finished check
								pairsList0[j][0] = i;
								pairsList0[j++][1] = k;
							}
							x--;
							break;
						}
					}	
				}
			}
		}
	}

	j1 = 0;

	// For each level
	for (y = 0; y < 16; y++) {
		// Check Type 1 blocks - blocks totally covered by others
		// further right along X axis

		for (x = 0; x < 7; x++) {
			flag = 1000 + x + y * 8;
			// This assumes minimum block length of 3
			for (z = 0; z < (6 * 128); z += 128) {
				block = _variableArray[flag + z];
				blockColour = block & 0xff80;
				if (block > 999) {
					i = z;
					while (1) {
						// Not the same colour?
						if (((_variableArray[flag + z + 1]) & 0xff80) != blockColour) {
							z += 128;
							// To end of block
							while (_variableArray[flag + z] == block)
								z += 128;
							z -= 128;
							break;
						}
						z += 128;
						if (_variableArray[flag + z] != block) {
							// Got a pair
							i += flag;
							k = i + 1;
							block = _variableArray[k];
							k -= 128;
							// Back to start of covering block
							while (_variableArray[k] == block)
								k -= 128;
							k += 128;
							if (block1Supported(i, y) && block1Supported(k, y)) {
								// Form a list and remove blocks when finished check
								pairsList1[j1][0] = i;
								pairsList1[j1++][1] = k;
							}
							z -= 128;
							break;
						}
					}	
				}
			}
		}

		// Check Type 1 blocks - blocks totally covered by others
		// further left along X axis

		for (x = 7; x > 0; x--) {
			flag = 1000 + x + y * 8;
			// This assumes minimum block length of 3
			for (z = 0; z < (6 * 128); z += 128) {
				block = _variableArray[flag + z];
				blockColour = block & 0xff80;
				if (block > 999) {
					i = z;
					while (1) {
						// Not same colour?
						if (((_variableArray[flag + z - 1]) & 0xff80) != blockColour)
						{
							z += 128;
							// To end of block
							while (_variableArray[flag + z] == block)
								z += 128;
							z -= 128;
							break;
						}
						z += 128;
						if (_variableArray[flag + z] != block) {
							// Got a pair
							i += flag;
							k = i - 1;
							block = _variableArray[k];
							k -= 128;
							// Back to start of covering block
							while (_variableArray[k] == block)
								k -= 128;
							k += 128;
							if (block1Supported(i, y) && block1Supported(k, y)) {
								// Form a list and remove blocks when finished check
								pairsList1[j1][0] = i;
								pairsList1[j1++][1] = k;
							}
							z -= 128;
							break;
						}
					}	
				}
			}
		}
	}

	// Remove any pairs

	if (j != 0) {
		for (i = 0; i < j; i++) {
			remove0Block(pairsList0[i][0]);
			remove0Block(pairsList0[i][1]);
		}
	}

	if (j1 != 0) {
		for (i = 0; i < j1; i++) {
			remove1Block(pairsList1[i][0]);
			remove1Block(pairsList1[i][1]);
		}
	}
}

void AGOSEngine::gravityCheck() {
	printf("GravityCheck\n");

	int i, k;
	int x, y, z;
	int flag;
	int block;

	// For each level
	for (y = 1; y < 16; y++) {
		// Check Type 0 blocks
		for (z = 0; z < 8; z++) {
			flag = 1000 + z * 128 + y * 8;
			// This assumes minimum block length of 3
			for (x = 0; x < 6; x++) {
				block = _variableArray[flag + x];
				if (block < 999 && block != 0) {
					i = x;
					while (1) {
						// Can't drop?
						if (_variableArray[flag + x - 8] != 0) {
							x++;
							// To end of block
							while (_variableArray[flag + x] == block)
								;
							x--;
							break;
						}
						x++;
						// Can drop?
						if (_variableArray[flag + x] != block) {
							x = i;
							while (_variableArray[flag + x] == block) {
								_variableArray[flag + x] = 0;
								_variableArray[flag + x - 8] = block;
								x++;
							}
							dropBlock(block, (y - 1) * 64 + (8 - z) * 8 + i, 0);
							x--;
							break;
						}
					}	
				}
			}
		}

		// Check Type 1 blocks

		for (x = 0; x < 8; x++) {
			flag = 1000 + y * 8 + x;
			for (z = 0; z < (6 * 128); z += 128) {
				block = _variableArray[flag + z];
				if (block > 999) {
					i = z;
					while (1) {
						// Can't drop?
						if (_variableArray[flag + z - 8] != 0) {
							z += 128;
							// To end of block
							while (_variableArray[flag + z] == block)
								z += 128;
							z -= 128;
							break;
						}
						z += 128;
						// Can drop?
						if (_variableArray[flag + z] != block) {
							z = i;
							k = -1;
							while (_variableArray[flag + z] == block) {
								_variableArray[flag + z] = 0;
								_variableArray[flag + z - 8] = block;
								z += 128;
								k++;
							}
							dropBlock(block, (y - 1) * 64 + (8 - i / 128) * 8 + x, k);
							z -= 128;
							break;
						}
					}	
				}
			}
		}
	}
}

bool AGOSEngine::hasBlockLanded(int16 yofs, int16 block, int16 priority, int16 type, int16 x, int16 y, int16 z) {
	if (type == 1) {
		x++;
		z--;
	}

	if (y > 15)
		return true;

	bool fail = false;
	
	if (y == 15) {
		// Failed if land on top level, or ceiling dropped to one level
		fail = true;
	}

	if (_variableArray[432] > 2 && y + _variableArray[432] > 16) {
		// above landed block
		fail = true;
	}

	int flag = 1000 + z * 128 + y * 8 + x;

	if (type == 0) {
		if (y != 0 && _variableArray[flag - 8] == 0 && _variableArray[flag + 1 - 8] == 0 && _variableArray[flag + 2 - 8] == 0) {
			return true;
		}

		_variableArray[flag] = block;
		_variableArray[flag + 1] = block;
		_variableArray[flag + 2] = block;

		if (fail) {
			_variableArray[254] = 6070;
		} else {
			// Start next block
			sendSync(6007);
		}
		return false;
	}

	if (y != 0 && _variableArray[flag - 8] == 0 && _variableArray[flag + 128 - 8] == 0 && _variableArray[flag + 256 - 8] == 0) {
		return true;
	}

	_variableArray[flag] = block;
	_variableArray[flag +128] = block;
	_variableArray[flag + 256] = block;

	if (fail) {
		_variableArray[254] = 6070;
	} else {
		// Start next block
		sendSync(6007);
	}
	return false;
}

} // End of namespace AGOS
