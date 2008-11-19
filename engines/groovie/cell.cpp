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

	//printf ("*** In cellgame constructor ***");
}

byte CellGame::getStartX() {
	return 0;	// TODO: implement something here
}

byte CellGame::getStartY() {
	return 6;	// TODO: implement something here
}

byte CellGame::getEndX() {
	return 1;	// TODO: implement something here
}

byte CellGame::getEndY() {
	return 6;	// TODO: implement something here
}

CellGame::~CellGame() {
}

} // End of Groovie namespace
