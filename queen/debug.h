/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#ifndef QUEENDEBUG_H
#define QUEENDEBUG_H

#include "common/util.h"

namespace Queen {

class Input;
class Logic;
class Graphics;

class Debug {
public:
	typedef void (Debug::*DebugFunc)();

	Debug(Input *, Logic *, Graphics *);

	void registerStub(const char *password, DebugFunc debugFunc);

	void update(int c);

	void jumpToRoom();
	void toggleFastMode();
	void printInfo();
	void toggleAreasDrawing();
	void changeGameState();
	void printGameState();
	void giveAllItems();

	static void digitKeyPressed(void *refCon, int key); 

	struct DebugStub {
		const char *password;
		DebugFunc function;
	};

	enum {
		MAX_STUB = 5
	};


private:

	char _password[16];
	uint _passwordCharCount;

	char _digitText[50];
	uint _digitTextCount;

	DebugStub _stub[MAX_STUB];
	uint _stubCount;

	Input *_input;
	Logic *_logic;
	Graphics *_graphics;

};


} // End of namespace Queen

#endif
