/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

	void update();
	void nextRoom();

	bool running() const { return _running; }

private:

	QueenEngine *_vm;
	LineReader *_credits;

	struct Line
	{
		short x,y,color,fontSize;
		char *text;
	};

	Line _list[15];

	bool _running;
	int _count;
	int _pause;
	int _justify;
	int _fontSize;
	int _color;
	int _zone;

};

} // End of namespace Queen

#endif
