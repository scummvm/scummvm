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
*/

#ifndef CGE_H
#define CGE_H

#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"
#include "graphics/surface.h"
#include "engines/advancedDetector.h"
#include "cge/console.h"

#define CGE_SAVEGAME_VERSION 1

namespace CGE {

class Console;

// our engine debug channels
enum {
    kCGEDebug = 1 << 0
};

class CGEEngine : public Engine {
public:
	CGEEngine(OSystem *syst, const ADGameDescription *gameDescription);
	~CGEEngine();

	const ADGameDescription *_gameDescription;

	virtual Common::Error run();
	GUI::Debugger *getDebugger() {
		return _console;
	}

private:
	CGEConsole *_console;
};

// Example console class
class Console : public GUI::Debugger {
public:
	Console(CGEEngine *vm) {}
	virtual ~Console(void) {}
};

} // End of namespace CGE

#endif
