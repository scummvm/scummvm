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
#include "sci/console.h"

#include "sci/engine/sciconsole.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"

#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"
#include "sci/gfx/operations.h"

namespace Sci {

extern gfx_driver_t gfx_driver_scummvm;

const char *versionNames[9] = {
	"Autodetected",
	"SCI0",
	"SCI01 EGA",
	"SCI01 VGA",
	"SCI01 VGA ODD",
	"SCI1 early",
	"SCI1 late",
	"SCI1.1",
	"SCI32"
};


int c_quit(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	script_abort_flag = 1; // Terminate VM
	_debugstate_valid = 0;
	_debug_seeking = 0;
	_debug_step_running = 0;
	return 0;
}

int c_die(EngineState *s, const Common::Array<cmd_param_t> &cmdParams) {
	exit(0); //
	return 0;
}

static void init_console() {
	con_hook_command(&c_quit, "quit", "", "console: Quits gracefully");
	con_hook_command(&c_die, "die", "", "console: Quits ungracefully");

	/*
	con_hook_int(&(gfx_options.buffer_pics_nr), "buffer_pics_nr",
		"Number of pics to buffer in LRU storage\n");
	con_hook_int(&(gfx_options.pic0_dither_mode), "pic0_dither_mode",
		"Mode to use for pic0 dithering\n");
	con_hook_int(&(gfx_options.pic0_dither_pattern), "pic0_dither_pattern",
		"Pattern to use for pic0 dithering\n");
	con_hook_int(&(gfx_options.pic0_unscaled), "pic0_unscaled",
		"Whether pic0 should be drawn unscaled\n");
	con_hook_int(&(gfx_options.dirty_frames), "dirty_frames",
		"Dirty frames management\n");
	*/
	con_hook_int(&gfx_crossblit_alpha_threshold, "alpha_threshold",
	             "Alpha threshold for crossblitting\n");
	con_hook_int(&sci0_palette, "sci0_palette",
	             "SCI0 palette- 0: EGA, 1:AGI/Amiga, 2:Grayscale\n");
	con_hook_int(&sci01_priority_table_flags, "sci01_priority_table_flags",
	             "SCI01 priority table debugging flags: 1:Disable, 2:Print on change\n");
}

static int init_gamestate(EngineState *gamestate, sci_version_t version) {
	return script_init_engine(gamestate, version);
}

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
	Common::addDebugChannel(kDebugLevelMem, "Memory", "Memory debugging");
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

	printf("SciEngine::SciEngine\n");
}

SciEngine::~SciEngine() {
	// Dispose your resources here
	printf("SciEngine::~SciEngine\n");

	// Remove all of our debug levels here
	Common::clearAllDebugChannels();

	delete _console;
}

Common::Error SciEngine::run() {
	initGraphics(320, 200, false);

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

	init_console(); /* So we can get any output */

	script_debug_flag = 0;

	sci_version_t version;
	int res_version = getResourceVersion();

	version = getVersion();

	_resmgr = new ResourceManager(res_version, 256 * 1024);

	if (!_resmgr) {
		printf("No resources found, aborting...\n");
		return Common::kNoGameDataFoundError;
	}

	script_adjust_opcode_formats(_resmgr->_sciVersion);

#if 0
	printf("Mapping instruments to General Midi\n");

	map_MIDI_instruments(_resmgr);
#endif

	EngineState *gamestate = new EngineState();
	gamestate->resmgr = _resmgr;
	gamestate->gfx_state = NULL;
	gamestate->flags = getFlags();

	// Verify that we haven't got an invalid game detection entry
	if (version < SCI_VERSION_1_EARLY) {
		// SCI0/SCI01
		if (gamestate->flags & GF_SCI1_EGA ||
			gamestate->flags & GF_SCI1_LOFSABSOLUTE ||
			gamestate->flags & GF_SCI1_NEWDOSOUND ||
			gamestate->flags & GF_SCI1_NEWSETCURSOR) {
			error("This game entry is erroneous. It's marked as SCI0/SCI01, but it has SCI1 flags set");
		}
	} else if (version >= SCI_VERSION_1_EARLY && version <= SCI_VERSION_1_LATE) {
		// SCI1
		
		if (gamestate->flags & GF_SCI0_OLD ||
			gamestate->flags & GF_SCI0_OLDGFXFUNCS ||
			gamestate->flags & GF_SCI0_OLDGETTIME) {
			error("This game entry is erroneous. It's marked as SCI1, but it has SCI0 flags set");
		}
	} else if (version == SCI_VERSION_1_1 || version == SCI_VERSION_32) {
		if (gamestate->flags & GF_SCI1_EGA ||
			gamestate->flags & GF_SCI1_LOFSABSOLUTE ||
			gamestate->flags & GF_SCI1_NEWDOSOUND ||
			gamestate->flags & GF_SCI1_NEWSETCURSOR) {
			error("This game entry is erroneous. It's marked as SCI1.1/SCI32, but it has SCI1 flags set");
		}

		if (gamestate->flags & GF_SCI0_OLD ||
			gamestate->flags & GF_SCI0_OLDGFXFUNCS ||
			gamestate->flags & GF_SCI0_OLDGETTIME) {
			error("This game entry is erroneous. It's marked as SCI1.1/SCI32, but it has SCI0 flags set");
		}

		// SCI1.1 / SCI32
	} else {
		error ("Unknown SCI version in game entry");
	}

	if (init_gamestate(gamestate, version))
		return Common::kUnknownError;


	if (game_init(gamestate)) { /* Initialize */
		fprintf(stderr, "Game initialization failed: Aborting...\n");
		// TODO: Add an "init failed" error?
		return Common::kUnknownError;
	}

	// Set the savegame dir (actually, we set it to a fake value,
	// since we cannot let the game control where saves are stored)
	script_set_gamestate_save_dir(gamestate, "/");

	gfx_crossblit_alpha_threshold = 0x90;
	GfxState gfx_state;
	gfx_state.driver = &gfx_driver_scummvm;

	gamestate->have_mouse_flag = 1;
	gamestate->animation_delay = 5;
	gamestate->animation_granularity = 4;
	gamestate->gfx_state = &gfx_state;

	// Default config:
	gfx_options_t gfx_options;

#ifdef CUSTOM_GRAPHICS_OPTIONS
	gfx_options.buffer_pics_nr = 0;
	gfx_options.pic0_unscaled = 1;
	gfx_options.pic0_dither_mode = GFXR_DITHER_MODE_D256;
	gfx_options.pic0_dither_pattern = GFXR_DITHER_PATTERN_SCALED;
	gfx_options.pic0_brush_mode = GFX_BRUSH_MODE_RANDOM_ELLIPSES;
	gfx_options.pic0_line_mode = GFX_LINE_MODE_CORRECT;
	gfx_options.cursor_xlate_filter = GFX_XLATE_FILTER_NONE;
	gfx_options.view_xlate_filter = GFX_XLATE_FILTER_NONE;
	gfx_options.pic_xlate_filter = GFX_XLATE_FILTER_NONE;
	gfx_options.text_xlate_filter = GFX_XLATE_FILTER_NONE;
	gfx_options.dirty_frames = GFXOP_DIRTY_FRAMES_CLUSTERS;
	for (int i = 0; i < GFX_RESOURCE_TYPES_NR; i++) {
		gfx_options.res_conf.assign[i] = NULL;
		gfx_options.res_conf.mod[i] = NULL;
	}
	gfx_options.workarounds = 0;
	// Default config ends
#endif

	bool isVGA = _resmgr->_sciVersion >= SCI_VERSION_01_VGA && !(getFlags() & GF_SCI1_EGA);
	if (gfxop_init(_resmgr->_sciVersion, isVGA, &gfx_state, &gfx_options, _resmgr)) {
		fprintf(stderr, "Graphics initialization failed. Aborting...\n");
		return Common::kUnknownError;
	}

	if (game_init_graphics(gamestate)) { // Init interpreter graphics
		fprintf(stderr, "Game initialization failed: Error in GFX subsystem. Aborting...\n");
		return Common::kUnknownError;
	}

	if (game_init_sound(gamestate, 0)) {
		fprintf(stderr, "Game initialization failed: Error in sound subsystem. Aborting...\n");
		return Common::kUnknownError;
	}

	printf("Emulating SCI version %s\n", versionNames[version]);

	game_run(&gamestate); // Run the game

	game_exit(gamestate);
	script_free_engine(gamestate); // Uninitialize game state
	script_free_breakpoints(gamestate);

	delete gamestate;

	delete _resmgr;

	gfxop_exit(&gfx_state);

	return Common::kNoError;
}

GUI::Debugger *SciEngine::getDebugger() {
	return _console;
}

const char* SciEngine::getGameID() const {
	return _gameDescription->desc.gameid;
}

int SciEngine::getVersion() const {
	return _gameDescription->version;
}

int SciEngine::getResourceVersion() const {
	return _gameDescription->res_version;
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
	char extension[6];
	snprintf(extension, sizeof(extension), ".%03d", nr);
	return _targetName + extension;
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

} // End of namespace Sci
