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
