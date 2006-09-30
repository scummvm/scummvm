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

bool AGOSEngine::block1Supported(int i, int y)
{
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

void AGOSEngine::stopBlock(int a, int b) {
}

void AGOSEngine::remove0Block(int i)
{
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

void AGOSEngine::remove1Block(int i)
{
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

void AGOSEngine::pairCheck()
{
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

void AGOSEngine::dropBlock(int a, int b, int c) {
}

void AGOSEngine::gravityCheck()
{
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

bool AGOSEngine::hasBlockLanded(int16 yofs, int16 block, int16 priority, int16 type, int16 x, int16 y, int16 z)
{
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
