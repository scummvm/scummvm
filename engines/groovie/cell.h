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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/groovie/script.h $
 * $Id: script.h 35095 2008-11-16 19:20:30Z spookypeanut $
 *
 */

#ifndef GROOVIE_CELL_H
#define GROOVIE_CELL_H

#include "common/file.h"
#include "common/util.h"

#define BOARDSIZE 7
#define CELL_CLEAR 0
#define CELL_BLUE 1
#define CELL_GREEN 2

namespace Groovie {

class GroovieEngine;
class Script;

class CellGame {
public:
	CellGame(byte *board);
	~CellGame();
	int8 calcMove(byte *origboard, uint8 color, uint8 depth);
	byte getStartX();
	byte getStartY();
	byte getEndX();
	byte getEndY();

private:
	bool validMove(byte *board, uint8 color, int8 endX, int8 endY);
	uint8 countBoard(byte* board, uint8 color);
	byte *_board;

	byte _startX;
	byte _startY;
	byte _endX;
	byte _endY;
};

} // End of Groovie namespace

#endif // GROOVIE_CELL_H
