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

#ifndef WINTERMUTE_H
#define WINTERMUTE_H

#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"

namespace WinterMute {

class Console;
class CBGame;
// our engine debug channels
enum {
    kWinterMuteDebugExample = 1 << 0,
    kWinterMuteDebugExample2 = 1 << 1
                               // next new channel must be 1 << 2 (4)
                               // the current limitation is 32 debug channels (1 << 31 is the last one)
};

class WinterMuteEngine : public Engine {
public:
	WinterMuteEngine(OSystem *syst);
	~WinterMuteEngine();

	virtual Common::Error run();

private:
	int init();
	int messageLoop();
	Console *_console;
	CBGame *_game;
	// We need random numbers
	Common::RandomSource *_rnd;
};

// Example console class
class Console : public GUI::Debugger {
public:
	Console(WinterMuteEngine *vm) {}
	virtual ~Console(void) {}
};

extern WinterMuteEngine *g_wintermute;

} // End of namespace Wintermute

#endif
