/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "groovie/t11hgame.h"
#include "groovie/groovie.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/translation.h"

namespace Groovie {

/* Links between the pieces in the Gallery challenge */
byte kGalleryLinks[21][10] = {
	{ 2,  4,  5,  0,  0,  0,  0,  0,  0,  0 },	//  1
	{ 1,  5,  3,  0,  0,  0,  0,  0,  0,  0 },	//  2
	{ 2,  5,  9, 12,  0,  0,  0,  0,  0,  0 },	//  3
	{ 1,  5,  6,  7,  8,  0,  0,  0,  0,  0 },	//  4
	{ 1,  2,  3,  4,  7,  8,  9,  0,  0,  0 },	//  5
	{ 4,  7, 10, 11, 13, 14, 15, 16, 18,  0 },	//  6
	{ 4,  5,  6,  8,  9, 10,  0,  0,  0,  0 },	//  7
	{ 4,  5,  7,  0,  0,  0,  0,  0,  0,  0 },	//  8
	{ 3,  5,  7, 10, 11, 12, 18,  0,  0,  0 },	//  9
	{ 6,  7,  9, 11,  0,  0,  0,  0,  0,  0 },	// 10
	{ 6,  9, 10, 18,  0,  0,  0,  0,  0,  0 },	// 11
	{ 3,  9, 18, 21,  0,  0,  0,  0,  0,  0 },	// 12
	{ 6, 14, 17, 19,  0,  0,  0,  0,  0,  0 },	// 13
	{ 6, 13, 15, 17, 19, 20, 21,  0,  0,  0 },	// 14
	{ 6, 14, 16, 18, 21,  0,  0,  0,  0,  0 },	// 15
	{ 6, 15,  0,  0,  0,  0,  0,  0,  0,  0 },	// 16
	{13, 14, 19,  0,  0,  0,  0,  0,  0,  0 },	// 17
	{ 6,  9, 11, 12, 15, 21,  0,  0,  0,  0 },	// 18
	{13, 14, 17, 20,  0,  0,  0,  0,  0,  0 },	// 19
	{14, 19, 21,  0,  0,  0,  0,  0,  0,  0 },	// 20
	{12, 14, 15, 18, 20,  0,  0,  0,  0,  0 }	// 21
};

T11hGame::T11hGame() :
	_random("GroovieT11hGame"), _scriptVariables(NULL) {
}


T11hGame::~T11hGame() {
}


void T11hGame::setVariables(byte *scriptVariables) {
	_scriptVariables = scriptVariables;
}


/*
* Puzzle in the Gallery.
* The aim is to select the last part of the image. When selecting a part all surrounding parts are also selected
* +--------------------+--------------------------------+--------+
* |         1/1A       |       2/1B                     |        |
* |  +--------------+--+--------------------------+-----+        |
* |  |              |                             |              |
* +--+     4/1D     |            5/1E             |       3/1C   |
* |                 |                             |              |
* +-----+--------+--+--------+-----------------+--+--------+     |
* |     |        |           |                 |           |     |
* |     |        |           |                 |           |     |
* |     |        |   8/21    |                 |           |     |
* |     |        |           |     +-----------+           |     |
* |     |        |           |     |           |           |     |
* |     |        +-----------+     |   10/23   |   9/22    |     |
* |     |                          |           |           |     |
* |     |           7/20           +-----+-----+           +-----+
* |     |                          |     |     |           |     |
* |     +--------------------------+     |     |           |     |
* |              6/1F                    |     |           |     |
* +-----------+-----------+-----+--+     | 11  |           | 12  |
* |   13/26   |           |     |  |     | /   |           | /   |
* |     +-----+-----+     |     |  |     | 24  +-----------+ 25  |
* |     |           |     |     |  |     |     |           |     |
* +-----+   17/2A   |     |     |16|     |     |           |     |
* |     |           |     |     |/ |     |     |           |     |
* |     +-----+-----+     |     |29|     |     |           +-----+
* |           |           |     |  |     |     |           |     |
* |           |           |     |  |     +-----+   18/2B   |     |
* |   19/2C   |   14/27   |     |  |           |           |     |
* |           |           |     |  +-----------+           |     |
* |           |           |     |  |           |           |     |
* |           |           |     +--+   15/28   |           |     |
* |           |           |                    |           |     |
* |           +--------+--+--------------------+-----------+     |
* |           | 20/2D  |              21/2E                      |
* +-----------+--------+-----------------------------------------+
*/
void T11hGame::opGallery() {

	byte field1[21];
	byte field2[21];
	byte var_18[21];
	int var_1c, eax, edx, ecx, esi;

	// Copy RegMem to Field1
	for (int i = 0; i < 21; i++) {
		field1[i] = _scriptVariables[0x1A + i];
	}

	var_1c = 0;
	for (int ebx = 0; ebx < 21; ebx++) {
		var_18[ebx] = 0;
		if (field1[ebx] != 0) {
			memcpy(field2, field1, 21);

			field2[ebx] = 0;
			edx = kGalleryLinks[0][ebx];
			eax = 1;
			ecx = 0;
			while (edx != 0) {
				eax++;
				field2[edx - 1] = ecx;
				edx = kGalleryLinks[eax - 1][ebx];
			}
			var_18[ebx] = opGallerySub(1, field2);
			if (var_18[ebx] == 1) {
				var_1c++;
			}
		}
	}

	if (var_1c == 0) {
		esi = 0;
		for (eax = 0; eax < 21; eax++) {
			if (var_18[eax] > esi) {
				esi = var_18[eax];
			}
		}

		if (esi == 2) {
			esi = 1;
		} else {
			if (esi <= 20) {
				esi = 2;
			} else {
				esi -= 12;
			}
		}

		for (eax = 0; eax < 21; eax++) {
			if (var_18[eax] <= esi) {
				var_18[eax] = 1;
				var_1c++;
			}
		}
	}






	int selectedPart;
		
	do {
		selectedPart = _random.getRandomNumber(20) + 1;
	} while (_scriptVariables[0x19 + selectedPart] != 1);

	setScriptVar(0x2F, selectedPart / 10);
	setScriptVar(0x30, selectedPart % 10);
}

// This function is mainly for debugging purpose
void inline T11hGame::setScriptVar(uint16 var, byte value) {
	_scriptVariables[var] = value;
	debugC(5, kDebugTlcGame, "script variable[0x%03X] = %d (0x%04X)", var, value, value);
}

void inline T11hGame::setScriptVar16(uint16 var, uint16 value) {
	_scriptVariables[var] = value & 0xFF;
	_scriptVariables[var + 1] = (value >> 8) & 0xFF;
	debugC(5, kDebugTlcGame, "script variable[0x%03X, 0x%03X] = %d (0x%02X, 0x%02X)", var, var + 1, value, _scriptVariables[var], _scriptVariables[var + 1]);
}

uint16 inline T11hGame::getScriptVar16(uint16 var) {
	uint16 value;

	value = _scriptVariables[var];
	value += _scriptVariables[var + 1] << 8;

	return value;
}

} // End of Namespace Groovie
