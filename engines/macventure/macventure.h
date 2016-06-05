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

#ifndef MACVENTURE_H
#define MACVENTURE_H

#include "common/debug.h"
#include "common/random.h"

#include "engines/engine.h"

#include "graphics/managed_surface.h"
#include "graphics/macgui/macwindowmanager.h"
 
#include "gui/debugger.h"

struct ADGameDescription;

namespace MacVenture {

class Console;

enum {
	kScreenWidth = 512,
	kScreenHeight = 342
};

enum {
	kMacVentureDebugExample = 1 << 0,
	kMacVentureDebugExample2 = 1 << 1
	// next new level must be 1 << 2 (4)
	// the current limitation is 32 debug levels (1 << 31 is the last one)
};

class MacVentureEngine : public Engine {

public:
	MacVentureEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~MacVentureEngine();

	virtual Common::Error run();

private:
	void processEvents();

private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource *_rnd;
	
	Console *_debugger;

	Graphics::MacWindowManager *_wm;
	Graphics::ManagedSurface _screen;

	bool _shouldQuit;
};


class Console : public GUI::Debugger {
public:
	Console(MacVentureEngine *vm) {}
	virtual ~Console(void) {}
};
} // End of namespace MacVenture

#endif