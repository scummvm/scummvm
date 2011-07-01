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

#include "cge/general.h"
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
	bool _isDemo;

	virtual Common::Error run();
	GUI::Debugger *getDebugger() {
		return _console;
	}

	void cge_main();
	void switchCave(int cav);
	void startCountDown();
	void quit();
	void resetQSwitch();
	void optionTouch(int opt, uint16 mask);
	void loadGame(XFile &file, bool tiny);
	void setMapBrick(int x, int z);
	void switchMapping();
	void loadSprite(const char *fname, int ref, int cav, int col, int row, int pos);
	void loadScript(const char *fname);
	void loadUser();
	void runGame();
	bool showTitle(const char *name);
	void movie(const char *ext);
	void takeName();
	void inf(const char *txt);
	void selectSound();
	void SNSelect();
	void dummy() {}
	void NONE();
	void SB();
	void caveDown();
	void xCave();
	void qGame();
	void SBM();
	void GUS();
	void GUSM();
	void MIDI();
	void AUTO();
	void setPortD();
	void setPortM();
	void setIRQ();
	void setDMA();
	void mainLoop();

private:
	CGEConsole *_console;
	void setup();
};

// Example console class
class Console : public GUI::Debugger {
public:
	Console(CGEEngine *vm) {}
	virtual ~Console(void) {}
};

} // End of namespace CGE

#endif
