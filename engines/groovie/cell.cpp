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
	uint8 bestStartX, bestStartY, bestEndX, bestEndY;
	int8 bestDiff = -100;
	int8 origBoardCount = countBoard(origboard, color);
	int8 currDiff = -100;
	byte *newboard;
	uint8 boardmemsize = sizeof(byte) * BOARDSIZE * BOARDSIZE;
	uint8 oppColor = 3 - color;

	bestStartX = bestStartY = bestEndX = bestEndY = 255;
	newboard = (byte*) malloc(boardmemsize);
	memcpy(newboard, origboard, boardmemsize);

	if (0 == depth) {
		return 0;
	}
	
	for (i = 0; BOARDSIZE > i; i++) {						// For every square on the board
		for (j = 0; BOARDSIZE > j; j++) {					//
			if (color == *(origboard + i + (BOARDSIZE * j))) {		// If the square is the desired colour
				for (di = -2; 2 >= di; di++) {				// Check every square two or less in every direction
					for (dj = -2; 2 >= dj; dj++) {			//
						if (di != 0 || dj != 0) {		// Don't allow a move onto itself
							debugC(7, kGroovieDebugCell | kGroovieDebugAll, "Depth: %d: Testing move %d, %d-> %d, %d", depth, i, j, i+di, j+dj);
							if (validMove(origboard, color, i+di, j+dj)) {
								int8 nextlevel;
								debugC(5, kGroovieDebugCell | kGroovieDebugAll, "Depth: %d: Valid move %d, %d-> %d, %d", depth, i, j, i+di, j+dj);
								execMove (newboard, color, i, j, i+di, j+dj);
								
								nextlevel = calcMove (newboard, oppColor, depth - 1);
								debugC(5, kGroovieDebugCell | kGroovieDebugAll, "Depth: %d: Next level down returned %d", depth, nextlevel);
								currDiff = countBoard(newboard, color) - origBoardCount - nextlevel;
								if (currDiff > bestDiff) {
									debugC(4, kGroovieDebugCell | kGroovieDebugAll, "Depth: %d: Found new best move (diff of %d): %d, %d-> %d, %d", depth, currDiff, i, j, i+di, j+dj);
									bestDiff = currDiff;
									bestStartX = i;
									bestStartY = j;
									bestEndX = i+di;
									bestEndY = j+dj;

								}
								// TODO: ideal would be to revert the move, rather than copy the board again. I think.
								memcpy(newboard, origboard, boardmemsize);
							}
						}
					}
				}
			}
		}
	}
	
	_startX = bestStartX;
	_startY = bestStartY;
	_endX = bestEndX;
	_endY = bestEndY;

	debugC(2, kGroovieDebugCell | kGroovieDebugAll, "Depth: %d: Best move is (diff of %d): %d, %d-> %d, %d", depth, bestDiff, _startX, _startY, _endX, _endY);
	free(newboard);
	debugC(5, kGroovieDebugCell | kGroovieDebugAll, "Freed newboard");
	return bestDiff;
}

void CellGame::execMove(byte *board, uint8 color, int8 startX, int8 startY, int8 endX, int8 endY) {
	int8 i, j;
	uint8 colorToEat = 3 - color;		// The opposite of the colour passed: 2 -> 1, 1 -> 2

	if (abs(endX - startX) == 2 || abs(endY - startY) == 2) {
		*(board + startX + BOARDSIZE * startY) = 0;
	}

	*(board + endX + BOARDSIZE * endY) = color;

	for (i = (endX - 1); endX + 1 >= i; i++) {
		for (j = (endY - 1); endY + 1 >= j; j++) {
			if (BOARDSIZE > i && BOARDSIZE > j && 0 <= i && 0 <= j) {		// Don't wrap around the board edges!
				uint8 offset = i + BOARDSIZE * j;
				if (colorToEat == *(board + offset)) {
					*(board + offset) = color;
				}
			}
		}
	}
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
