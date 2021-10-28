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

#include "groovie/groovie.h"
#include "groovie/logic/winerack.h"

namespace Groovie {

enum WineBottle {
	kWineBottleOpponent = 1,
	kWineBottlePlayer = 2
};

WineRackGame::WineRackGame() : _random("WineRackGame"), _totalBottles(0) {
	memset(_wineRackGrid, 0, 100);
}

void WineRackGame::run(byte *scriptVariables) {
	char op = scriptVariables[3];
	byte pos = 0;

	switch (op) {
	case 3:
		initGrid(scriptVariables[4]);
		break;
	case 4:
		pos = calculateNextMove(2);
		placeBottle(pos, 2);
		scriptVariables[0] = pos / 10;
		scriptVariables[1] = pos % 10;
		//scriptVariables[3] = (char)FUN_00412c90();
		break;
	case 5:
		scriptVariables[3] = 0;
		pos = calculateNextMove(1);
		placeBottle(pos, 1);
		scriptVariables[0] = pos / 10;
		scriptVariables[1] = pos % 10;
		//if ((char)FUN_00412cf0() != 0) {
		//	scriptVariables[3] = 1;
		//}
		break;
	default:
		scriptVariables[3] = 0;
		placeBottle(scriptVariables[0] * 10 + scriptVariables[1], 2);
		//if ((char)FUN_00412c90() != 0) {
		//	scriptVariables[3] = 2;
		//	return;
		//}

		pos = calculateNextMove(1);
		placeBottle(pos, 1);
		scriptVariables[0] = pos / 10;
		scriptVariables[1] = pos % 10;

		//if ((char)FUN_00412cf0() != 0) {
		//	scriptVariables[3] = 1;
		//}
		break;
	}
}

void WineRackGame::initGrid(byte difficulty) {
	memset(_wineRackGrid + 1, 0, 25);

	switch (difficulty) {
	case 0:
		_totalBottles = 24;

		_wineRackGrid[15] = kWineBottlePlayer;
		_wineRackGrid[18] = kWineBottleOpponent;
		_wineRackGrid[19] = kWineBottleOpponent;
		_wineRackGrid[20] = kWineBottleOpponent;
		_wineRackGrid[21] = kWineBottleOpponent;
		_wineRackGrid[22] = kWineBottleOpponent;
		_wineRackGrid[23] = kWineBottleOpponent;
		_wineRackGrid[25] = kWineBottlePlayer;
		_wineRackGrid[26] = kWineBottleOpponent;
		_wineRackGrid[27] = kWineBottleOpponent;
		_wineRackGrid[28] = kWineBottleOpponent;
		_wineRackGrid[33] = kWineBottleOpponent;
		_wineRackGrid[34] = kWineBottleOpponent;
		_wineRackGrid[35] = kWineBottlePlayer;
		_wineRackGrid[36] = kWineBottleOpponent;
		_wineRackGrid[44] = kWineBottlePlayer;
		_wineRackGrid[45] = kWineBottlePlayer;
		_wineRackGrid[54] = kWineBottlePlayer;
		_wineRackGrid[62] = kWineBottlePlayer;
		_wineRackGrid[63] = kWineBottlePlayer;
		_wineRackGrid[64] = kWineBottlePlayer;
		_wineRackGrid[72] = kWineBottlePlayer;
		_wineRackGrid[82] = kWineBottlePlayer;
		_wineRackGrid[91] = kWineBottlePlayer;
		break;
	case 1:
		_totalBottles = 12;

		_wineRackGrid[75] = kWineBottlePlayer;
		_wineRackGrid[56] = kWineBottlePlayer;
		_wineRackGrid[45] = kWineBottlePlayer;
		_wineRackGrid[27] = kWineBottlePlayer;
		_wineRackGrid[24] = kWineBottlePlayer;
		_wineRackGrid[15] = kWineBottlePlayer;
		_wineRackGrid[64] = kWineBottleOpponent;
		_wineRackGrid[34] = kWineBottleOpponent;
		_wineRackGrid[33] = kWineBottleOpponent;
		_wineRackGrid[18] = kWineBottleOpponent;
		_wineRackGrid[16] = kWineBottleOpponent;
		_wineRackGrid[14] = kWineBottleOpponent;
		break;
	}
}

void WineRackGame::placeBottle(byte pos, byte val) {
	_totalBottles++;
	_wineRackGrid[pos] = val;
}

byte WineRackGame::calculateNextMove(byte op) {
	// TODO
	return 0;
}

} // End of Groovie namespace
