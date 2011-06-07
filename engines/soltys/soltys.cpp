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

#include "common/scummsys.h"
 
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"
 
#include "engines/util.h"
 
#include "soltys/soltys.h"
 
namespace Soltys {
 
SoltysEngine::SoltysEngine(OSystem *syst, const ADGameDescription *gameDescription)
 : Engine(syst), _gameDescription(gameDescription) {
 
	DebugMan.addDebugChannel(kSoltysDebug, "general", "Soltys general debug channel");
 	_console = new SoltysConsole(this);

	debug("SoltysEngine::SoltysEngine");
}
 
SoltysEngine::~SoltysEngine() {
	debug("SoltysEngine::~SoltysEngine");
 
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}
 
Common::Error SoltysEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);
 
 	// Create debugger console. It requires GFX to be initialized
	_console = new SoltysConsole(this);
 
	// Additional setup.
	debug("SoltysEngine::init");
 
	return Common::kNoError;
}
 
} // End of namespace Soltys
