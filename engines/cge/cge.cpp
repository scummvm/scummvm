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
#include "cge/cge.h"
#include "cge/cge_main.h"
#include "cge/text.h"

namespace CGE {

CGEEngine::CGEEngine(OSystem *syst, const ADGameDescription *gameDescription)
	: Engine(syst), _gameDescription(gameDescription) {

	DebugMan.addDebugChannel(kCGEDebug, "general", "CGE general debug channel");
	_console = new CGEConsole(this);
	Text = new TEXT(ProgName());
	Vga = new VGA(M13H);
	Heart = new HEART;


	OffUseCount = atoi(Text->getText(OFF_USE_COUNT));

	debug("CGEEngine::CGEEngine");
}

CGEEngine::~CGEEngine() {
	debug("CGEEngine::~CGEEngine");

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

Common::Error CGEEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);

	// Create debugger console. It requires GFX to be initialized
	_console = new CGEConsole(this);

	// Additional setup.
	debug("CGEEngine::init");

	cge_main();

	return Common::kNoError;
}

} // End of namespace CGE
