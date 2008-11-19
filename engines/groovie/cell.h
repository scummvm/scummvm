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

namespace Groovie {

class GroovieEngine;
class Script;

class CellGame {
public:
	CellGame(byte *board);
	~CellGame();
	byte getStartX();
	byte getStartY();
	byte getEndX();
	byte getEndY();

private:
	byte *_board;
};

} // End of Groovie namespace

#endif // GROOVIE_CELL_H
