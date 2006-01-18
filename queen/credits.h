/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef CREDITS_H
#define CREDITS_H

#include "common/util.h"
#include "queen/defs.h"

namespace Queen {

class QueenEngine;
class LineReader;

class Credits {
public:

	Credits(QueenEngine *vm, const char* filename);
	~Credits();

	//! update/display credits for current room
	void update();

	//! handles room switching
	void nextRoom();

	//! returns true if the credits are running
	bool running() const { return _running; }

private:

	struct Line {
		short x, y, color, fontSize;
		const char *text;
	};

	//! contains the formatted lines of texts to display
	Line _list[19];

	//! true if end of credits description hasn't been reached
	bool _running;

	//! number of elements in _list array
	int _count;

	//! pause counts for next room
	int _pause;

	//! current text justification mode
	int _justify;

	//! current font size (unused ?)
	int _fontSize;

	//! current text color
	int _color;

	//! current text position
	int _zone;

	//! contains the credits description
	LineReader *_credits;

	QueenEngine *_vm;
};

} // End of namespace Queen

#endif
