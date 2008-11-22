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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/groovie/script.cpp $
 * $Id: script.cpp 35096 2008-11-16 20:20:31Z lordhoto $
 *
 */

#include "groovie/cell.h"

namespace Groovie {

CellGame::CellGame(byte *board) :
	_board(board) {

	_startX = _startY = _endX = _endY = 255;
}

int8 CellGame::calcMove(byte *origboard, uint8 color, uint8 depth) {
	uint8 i, j;
	int8 di, dj;
	byte *newboard;
	uint8 boardmemsize = sizeof(byte) * BOARDSIZE * BOARDSIZE;
	int8 maxdiff = -100;

	newboard = (byte*) malloc(boardmemsize);
	memcpy(newboard, origboard, boardmemsize);

	if (0 == depth) {
		return 0;
	}
	
	for (i = 0; BOARDSIZE > i; i++) {
		for (j = 0; BOARDSIZE > j; j++) {					// For every square on the board
			if (color == *(origboard + i + (BOARDSIZE * j))) {		// If the square is the desired colour
				for (di = -2; 2 >= di; di++) {
					for (dj = -2; 2 >= dj; dj++) {
						if (di != 0 || dj != 0) {		// Don't allow a move onto itself
							if (validMove(origboard, color, i+di, j+dj)) {
								_startX = i;
								_startY = j;
								_endX = i+di;
								_endY = j+dj;
							}
						}
					}
				}
			}
		}
	}
	
	free(newboard);
	return maxdiff;
}

bool CellGame::validMove(byte *board, uint8 color, int8 endX, int8 endY) {
	if (0 > endX || 0 > endY || BOARDSIZE <= endX || BOARDSIZE <= endY) {		// Move is out of bounds
		return false;
	}
	if (0 == *(board + endX + (BOARDSIZE * endY))) {
		return true;
	}
	return false;
}

uint8 CellGame::countBoard(byte *board, uint8 color) {
	uint8 total = 0;
	for (uint8 i = 0; BOARDSIZE > i; i++) {
		for (uint8 j = 0; BOARDSIZE > j; j++) {
			if (color == *(board + i + (BOARDSIZE * j))) {
				total++;
			}
		}
	}
	return total;
}

byte CellGame::getStartX() {
	if (_startX > BOARDSIZE) {
		warning ("CellGame::getStartX: not calculated yet!");
		return 0;
	} else {
		return _startX;
	}
}

byte CellGame::getStartY() {
	if (_startY > BOARDSIZE) {
		warning ("CellGame::getStartY: not calculated yet!");
		return 6;
	} else {
		return _startY;
	}
}

byte CellGame::getEndX() {
	if (_endX > BOARDSIZE) {
		warning ("CellGame::getEndX: not calculated yet!");
		return 1;	
	} else {
		return _endX;
	}
}

byte CellGame::getEndY() {
	if (_endY > BOARDSIZE) {
		warning ("CellGame::getEndY: not calculated yet!");
		return 6;
	} else {
		return _endY;
	}
}

CellGame::~CellGame() {
}

} // End of Groovie namespace
