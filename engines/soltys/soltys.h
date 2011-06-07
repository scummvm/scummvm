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

#ifndef SOLTYS_H
#define SOLTYS_H
 
#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"
#include "graphics/surface.h"
#include "engines/advancedDetector.h"
#include "soltys/console.h"

#define SOLTYS_SAVEGAME_VERSION 1

namespace Soltys {
 
class Console;
 
// our engine debug channels
enum {
	kSoltysDebug = 1 << 0
};
 
class SoltysEngine : public Engine {
public:
	SoltysEngine(OSystem *syst, const ADGameDescription *gameDescription);
	~SoltysEngine();

	const ADGameDescription *_gameDescription;

	virtual Common::Error run();
	GUI::Debugger *getDebugger() { return _console; }
 
private:
	SoltysConsole *_console;
};
 
// Example console class
class Console : public GUI::Debugger {
public:
	Console(SoltysEngine *vm) {}
	virtual ~Console(void) {}
};
 
} // End of namespace Soltys
 
#endif
