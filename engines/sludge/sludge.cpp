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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "common/scummsys.h"
 
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"

#include "engines/util.h"

#include "sludge/sludge.h"
#include "main_loop.h"

namespace Sludge {
 
SludgeEngine::SludgeEngine(OSystem *syst, const SludgeGameDescription *gameDesc)
 : Engine(syst), _gameDescription(gameDesc), _console(nullptr) {
 
	// register your random source
	_rnd = new Common::RandomSource("sludge");

	// Add debug channels
	DebugMan.addDebugChannel(kSludgeDebugScript, "Script", "Script debug level");
	DebugMan.addDebugChannel(kSludgeDebugDataLoad, "Data Load", "Data loading debug level");
	DebugMan.addDebugChannel(kSludgeDebugStackMachine, "Stack Machine", "Stack Machine debug level");
	DebugMan.addDebugChannel(kSludgeDebugBuiltin, "Built-in", "Built-in debug level");
	DebugMan.addDebugChannel(kSludgeDebugGraphics, "Graphics", "Graphics debug level");

	// check init
	debug("SludgeEngine::SludgeEngine");
}
 
SludgeEngine::~SludgeEngine() {

	// Dispose resources
	delete _rnd;
 
	// Remove debug levels
	DebugMan.clearAllDebugChannels();
	
	// Dispose console
	delete _console;
}
 
Common::Error SludgeEngine::run() {
	// init graphics
	Graphics::PixelFormat format = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	initGraphics(640, 480, false, &format);

	// create console
	_console = new SludgeConsole(this);
 
	// debug log
	main_loop("Welcome.slg");
 
	return Common::kNoError;
}
 
} // End of namespace Sludge

