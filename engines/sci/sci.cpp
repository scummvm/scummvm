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
 * $URL$
 * $Id$
 *
 */


#include "common/system.h"
#include "common/config-manager.h"

#include "engines/advancedDetector.h"
#include "sci/sci.h"
#include "sci/debug.h"
#include "sci/console.h"

#include "sci/engine/state.h"
#include "sci/engine/kernel.h"

#include "sci/gfx/operations.h"	// fog GfxState
#include "sci/gfx/gfx_state_internal.h"	// required for GfxContainer, GfxPort, GfxVisual
#include "sci/gui32/gui32.h"
#include "sci/gui/gui.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_cursor.h"
#include "sci/gui/gui_screen.h"

#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

class GfxDriver;

SciEngine::SciEngine(OSystem *syst, const ADGameDescription *desc)
		: Engine(syst), _gameDescription(desc), _system(syst) {
	_console = NULL;

	// Set up the engine specific debug levels
	Common::addDebugChannel(kDebugLevelError, "Error", "Script error debugging");
	Common::addDebugChannel(kDebugLevelNodes, "Lists", "Lists and nodes debugging");
	Common::addDebugChannel(kDebugLevelGraphics, "Graphics", "Graphics debugging");
	Common::addDebugChannel(kDebugLevelStrings, "Strings", "Strings debugging");
	Common::addDebugChannel(kDebugLevelMemory, "Memory", "Memory debugging");
	Common::addDebugChannel(kDebugLevelFuncCheck, "Func", "Function parameter debugging");
	Common::addDebugChannel(kDebugLevelBresen, "Bresenham", "Bresenham algorithms debugging");
	Common::addDebugChannel(kDebugLevelSound, "Sound", "Sound debugging");
	Common::addDebugChannel(kDebugLevelGfxDriver, "Gfxdriver", "Gfx driver debugging");
	Common::addDebugChannel(kDebugLevelBaseSetter, "Base", "Base Setter debugging");
	Common::addDebugChannel(kDebugLevelParser, "Parser", "Parser debugging");
	Common::addDebugChannel(kDebugLevelMenu, "Menu", "Menu handling debugging");
	Common::addDebugChannel(kDebugLevelSaid, "Said", "Said specs debugging");
	Common::addDebugChannel(kDebugLevelFile, "File", "File I/O debugging");
	Common::addDebugChannel(kDebugLevelTime, "Time", "Time debugging");
	Common::addDebugChannel(kDebugLevelRoom, "Room", "Room number debugging");
	Common::addDebugChannel(kDebugLevelAvoidPath, "Pathfinding", "Pathfinding debugging");
	Common::addDebugChannel(kDebugLevelDclInflate, "DCL", "DCL inflate debugging");
	Common::addDebugChannel(kDebugLevelVM, "VM", "VM debugging");
	Common::addDebugChannel(kDebugLevelScripts, "Scripts", "Notifies when scripts are unloaded");
	Common::addDebugChannel(kDebugLevelGC, "GC", "Garbage Collector debugging");
	Common::addDebugChannel(kDebugLevelSci0Pic, "Sci0Pic", "SCI0 pic drawing debugging");

	_gamestate = 0;

	SearchMan.addSubDirectoryMatching(_gameDataDir, "seq");	// KQ6 SEQ directory
}

SciEngine::~SciEngine() {
	// Remove all of our debug levels here
	Common::clearAllDebugChannels();

	delete _kernel;
	delete _vocabulary;
	delete _console;
	delete _resMan;
}

Common::Error SciEngine::run() {
	initGraphics(320, 200, false);

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);

	// FIXME/TODO: Move some of the stuff below to init()

	_resMan = new ResourceManager();

	if (!_resMan) {
		warning("No resources found, aborting");
		return Common::kNoGameDataFoundError;
	}

	_kernel = new Kernel(_resMan);
	_vocabulary = new Vocabulary(_resMan);
	SciGuiScreen *screen = new SciGuiScreen(_resMan);
	SciGuiPalette *palette = new SciGuiPalette(_resMan, screen);
	SciGuiCursor *cursor = new SciGuiCursor(_resMan, palette, screen);

	// We'll set the GUI below
	_gamestate = new EngineState(_resMan, _kernel, _vocabulary, NULL, cursor);

	if (script_init_engine(_gamestate))
		return Common::kUnknownError;

	// Gui change
	//_gamestate->_gui = new SciGui(_gamestate, screen, palette, cursor);    // new
	_gamestate->_gui = new SciGui32(_gamestate, screen, palette, cursor);  // old

	if (game_init(_gamestate)) { /* Initialize */
		warning("Game initialization failed: Aborting...");
		// TODO: Add an "init failed" error?
		return Common::kUnknownError;
	}

	script_adjust_opcode_formats(_gamestate);

	// Set the savegame dir (actually, we set it to a fake value,
	// since we cannot let the game control where saves are stored)
	strcpy(_gamestate->sys_strings->_strings[SYS_STRING_SAVEDIR]._value, "/");

	GfxState gfx_state;
	_gamestate->gfx_state = &gfx_state;

	// Assign default values to the config manager, in case settings are missing
	ConfMan.registerDefault("undither", "true");
	screen->unditherSetState(ConfMan.getBool("undither"));

#ifdef INCLUDE_OLDGFX
	gfxop_init(&gfx_state, _resMan, screen, palette, 1);
#endif

	if (game_init_graphics(_gamestate)) { // Init interpreter graphics
		warning("Game initialization failed: Error in GFX subsystem. Aborting...");
		return Common::kUnknownError;
	}

	if (game_init_sound(_gamestate, 0)) {
		warning("Game initialization failed: Error in sound subsystem. Aborting...");
		return Common::kUnknownError;
	}

	_gamestate->_gui->init(_gamestate->usesOldGfxFunctions());

	debug("Emulating SCI version %s\n", getSciVersionDesc(getSciVersion()).c_str());

	game_run(&_gamestate); // Run the game

	game_exit(_gamestate);
	script_free_engine(_gamestate); // Uninitialize game state
	script_free_breakpoints(_gamestate);

	delete cursor;
	delete palette;
	delete screen;
	delete _gamestate;

#ifdef INCLUDE_OLDGFX
	gfxop_exit(&gfx_state);
#endif

	return Common::kNoError;
}

// Invoked by error() when a severe error occurs
GUI::Debugger *SciEngine::getDebugger() {
	if (_gamestate) {
		ExecStack *xs = &(_gamestate->_executionStack.back());
		xs->addr.pc.offset = g_debugState.old_pc_offset;
		xs->sp = g_debugState.old_sp;
	}

	g_debugState.runningStep = 0; // Stop multiple execution
	g_debugState.seeking = kDebugSeekNothing; // Stop special seeks

	return _console;
}

// Used to obtain the engine's console in order to print messages to it
Console *SciEngine::getSciDebugger() {
	return _console;
}

const char* SciEngine::getGameID() const {
	return _gameDescription->gameid;
}

Common::Language SciEngine::getLanguage() const {
	return _gameDescription->language;
}

Common::Platform SciEngine::getPlatform() const {
	return _gameDescription->platform;
}

uint32 SciEngine::getFlags() const {
	return _gameDescription->flags;
}

Common::String SciEngine::getSavegameName(int nr) const {
	return _targetName + Common::String::printf(".%03d", nr);
}

Common::String SciEngine::getSavegamePattern() const {
	return _targetName + ".???";
}

Common::String SciEngine::wrapFilename(const Common::String &name) const {
	return _targetName + "-" + name;
}

Common::String SciEngine::unwrapFilename(const Common::String &name) const {
	Common::String prefix = name + "-";
	if (name.hasPrefix(prefix.c_str()))
		return Common::String(name.c_str() + prefix.size());
	return name;
}

void SciEngine::pauseEngineIntern(bool pause) {
	_gamestate->_sound.sfx_suspend(pause);
	_mixer->pauseAll(pause);
}

} // End of namespace Sci
