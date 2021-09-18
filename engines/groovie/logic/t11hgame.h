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

#ifndef GROOVIE_LOGIC_T11HGAME_H
#define GROOVIE_LOGIC_T11HGAME_H

#include "common/textconsole.h"
#include "common/random.h"

namespace Groovie {

class GroovieEngine;

class T11hGame {
public:
#ifdef ENABLE_GROOVIE2
	T11hGame(byte *scriptVariables);
	~T11hGame();

	void handleOp(uint8 op);

private:
	Common::RandomSource _random;

	void opMouseTrap();
	void opConnectFour();
	byte connectFourAI();
	void opBeehive();
	void opPente();
	void opGallery();
	void opTriangle();

	byte opGallerySub(int one, byte *field);
	int8 opBeehiveGetHexDifference();
	int8 opBeehiveGetTotal(int8 *hexagons);
	void inline setScriptVar(uint16 var, byte value);
	void inline setScriptVar16(uint16 var, uint16 value);
	uint16 inline getScriptVar16(uint16 var);

	bool isCakeFull();
	byte cakeGetWinner();
	void clearCake();
	void cakePlaceBonBon(int x, byte team);
	byte cakeGetOpponent(byte team);
	int cakeGetLineLen(int start_x, int start_y, int slope_x, int slope_y, byte team);

	byte *_scriptVariables;

	static const int CAKE_BOARD_WIDTH = 8;
	static const int CAKE_BOARD_HEIGHT = 7;
	// (0, 0) is the bottom left of the board
	byte cake_board[CAKE_BOARD_WIDTH][CAKE_BOARD_HEIGHT];
	static const byte CAKE_TEAM_STAUF = 1;
	static const byte CAKE_TEAM_PLAYER = 2;
	static const int CAKE_GOAL_LEN = 4;

	int8 _beehiveHexagons[61];

	static const byte kGalleryLinks[21][10];
#endif
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_T11HGAME_H
