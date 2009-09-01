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

#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"
#include "sci/gfx/operations.h"

namespace Sci {

class GfxDriver;

SciEngine::SciEngine(OSystem *syst, const SciGameDescription *desc)
		: Engine(syst), _gameDescription(desc) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().

	// However this is the place to specify all default directories
	//File::addDefaultDirectory(_gameDataPath + "sound/");
	//printf("%s\n", _gameDataPath.c_str());

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

	_gamestate = 0;

	printf("SciEngine::SciEngine\n");
}

SciEngine::~SciEngine() {
	// Dispose your resources here
	printf("SciEngine::~SciEngine\n");

	// Remove all of our debug levels here
	Common::clearAllDebugChannels();

	delete _kernel;
	delete _vocabulary;
	delete _console;
	delete _resourceManager;
}

Common::Error SciEngine::run() {
	Graphics::PixelFormat gfxmode;
#if 0 && defined(USE_RGB_COLOR)
	initGraphics(320, 200, false, NULL);
#else
	initGraphics(320, 200, false);
#endif
	gfxmode = _system->getScreenFormat();

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);

	// Additional setup.
	printf("SciEngine::init\n");

	/* bool end = false;
	Common::EventManager *em = _system->getEventManager();
	while (!end) {
		Common::Event ev;
		if (em->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				end = true;
			}
		}
		_system->delayMillis(10);
	} */

	// FIXME/TODO: Move some of the stuff below to init()

	const uint32 flags = getFlags();

	_resourceManager = new ResourceManager();

	if (!_resourceManager) {
		printf("No resources found, aborting...\n");
		return Common::kNoGameDataFoundError;
	}

	_kernel = new Kernel(_resourceManager);
	_vocabulary = new Vocabulary(_resourceManager);

	_gamestate = new EngineState(_resourceManager, flags);

	if (script_init_engine(_gamestate))
		return Common::kUnknownError;

	if (game_init(_gamestate)) { /* Initialize */
		warning("Game initialization failed: Aborting...");
		// TODO: Add an "init failed" error?
		return Common::kUnknownError;
	}

	script_adjust_opcode_formats(_gamestate);

	// Set the savegame dir (actually, we set it to a fake value,
	// since we cannot let the game control where saves are stored)
	script_set_gamestate_save_dir(_gamestate, "/");

	GfxState gfx_state;
	_gamestate->gfx_state = &gfx_state;

	_gamestate->animation_granularity = 4;

	// Assign default values to the config manager, in case settings are missing
	ConfMan.registerDefault("cursor_filter", "0");
	ConfMan.registerDefault("view_filter", "0");
	ConfMan.registerDefault("pic_filter", "0");
	ConfMan.registerDefault("text_filter", "0");

	// Default config:
	gfx_options_t gfx_options;

#ifdef CUSTOM_GRAPHICS_OPTIONS
	gfx_options.buffer_pics_nr = 0;
	gfx_options.pic0_unscaled = 1;
	gfx_options.pic0_dither_mode = GFXR_DITHER_MODE_F256;
	gfx_options.pic0_dither_pattern = GFXR_DITHER_PATTERN_SCALED;
	gfx_options.pic0_brush_mode = GFX_BRUSH_MODE_RANDOM_ELLIPSES;
	gfx_options.pic0_line_mode = GFX_LINE_MODE_CORRECT;
	gfx_options.cursor_xlate_filter = (gfx_xlate_filter_t)ConfMan.getInt("cursor_filter");
	gfx_options.view_xlate_filter = (gfx_xlate_filter_t)ConfMan.getInt("view_filter");
	gfx_options.pic_xlate_filter = (gfx_xlate_filter_t)ConfMan.getInt("pic_filter");
	gfx_options.text_xlate_filter = (gfx_xlate_filter_t)ConfMan.getInt("text_filter");
	gfx_options.dirty_frames = GFXOP_DIRTY_FRAMES_CLUSTERS;
	for (int i = 0; i < GFX_RESOURCE_TYPES_NR; i++) {
		gfx_options.res_conf.assign[i] = NULL;
		gfx_options.res_conf.mod[i] = NULL;
	}
	gfx_options.workarounds = 0;
	// Default config ends
#endif

	gfxop_init(_resourceManager->sciVersion(), &gfx_state, &gfx_options, _resourceManager, gfxmode, 1, 1);

	if (game_init_graphics(_gamestate)) { // Init interpreter graphics
		warning("Game initialization failed: Error in GFX subsystem. Aborting...");
		return Common::kUnknownError;
	}

	if (game_init_sound(_gamestate, 0)) {
		warning("Game initialization failed: Error in sound subsystem. Aborting...");
		return Common::kUnknownError;
	}

	printf("Emulating SCI version %s\n", getSciVersionDesc(_resourceManager->sciVersion()).c_str());

	game_run(&_gamestate); // Run the game

	game_exit(_gamestate);
	script_free_engine(_gamestate); // Uninitialize game state
	script_free_breakpoints(_gamestate);

	delete _gamestate;

	gfxop_exit(&gfx_state);

	return Common::kNoError;
}

// Invoked by error() when a severe error occurs
GUI::Debugger *SciEngine::getDebugger() {
	if (_gamestate) {
		ExecStack *xs = &(_gamestate->_executionStack.back());
		xs->addr.pc.offset = scriptState.old_pc_offset;
		xs->sp = scriptState.old_sp;
	}

	scriptState.runningStep = 0; // Stop multiple execution
	scriptState.seeking = kDebugSeekNothing; // Stop special seeks

	return _console;
}

// Used to obtain the engine's console in order to print messages to it
Console *SciEngine::getSciDebugger() {
	return _console;
}

const char* SciEngine::getGameID() const {
	return _gameDescription->desc.gameid;
}

SciVersion SciEngine::getVersion() const {
	return _resourceManager->sciVersion();
}

Common::Language SciEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform SciEngine::getPlatform() const {
	return _gameDescription->desc.platform;
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

Common::String SciEngine::getSciVersionDesc(SciVersion version) const {
	switch (version) {
	case SCI_VERSION_AUTODETECT:
		return "Autodetect";
	case SCI_VERSION_0_EARLY:
		return "Early SCI0";
	case SCI_VERSION_0_LATE:
		return "Late SCI0";
	case SCI_VERSION_01:
		return "SCI01";
	case SCI_VERSION_1_EGA:
		return "SCI1 EGA";
	case SCI_VERSION_1_EARLY:
		return "Early SCI1";
	case SCI_VERSION_1_MIDDLE:
		return "Middle SCI1";
	case SCI_VERSION_1_LATE:
		return "Late SCI1";
	case SCI_VERSION_1_1:
		return "SCI1.1";
	case SCI_VERSION_2:
		return "SCI2";
	case SCI_VERSION_2_1:
		return "SCI2.1";
	case SCI_VERSION_3:
		return "SCI3";
	default:
		return "Unknown";
	}
}

} // End of namespace Sci
