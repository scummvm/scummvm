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
#include "common/debug-channels.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "sci/sci.h"
#include "sci/debug.h"
#include "sci/console.h"
#include "sci/event.h"

#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"	// for script_adjust_opcode_formats

#include "sci/sound/audio.h"
#include "sci/sound/soundcmd.h"
#include "sci/graphics/gui.h"
#include "sci/graphics/maciconbar.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/cache.h"

#ifdef ENABLE_SCI32
#include "sci/graphics/gui32.h"
#endif

namespace Sci {

extern int g_loadFromLauncher;

SciEngine *g_sci = 0;


class GfxDriver;

SciEngine::SciEngine(OSystem *syst, const ADGameDescription *desc)
		: Engine(syst), _gameDescription(desc), _system(syst) {
	_console = NULL;

	assert(g_sci == 0);
	g_sci = this;
	_features = 0;

	// Set up the engine specific debug levels
	DebugMan.addDebugChannel(kDebugLevelError, "Error", "Script error debugging");
	DebugMan.addDebugChannel(kDebugLevelNodes, "Lists", "Lists and nodes debugging");
	DebugMan.addDebugChannel(kDebugLevelGraphics, "Graphics", "Graphics debugging");
	DebugMan.addDebugChannel(kDebugLevelStrings, "Strings", "Strings debugging");
	DebugMan.addDebugChannel(kDebugLevelMemory, "Memory", "Memory debugging");
	DebugMan.addDebugChannel(kDebugLevelFuncCheck, "Func", "Function parameter debugging");
	DebugMan.addDebugChannel(kDebugLevelBresen, "Bresenham", "Bresenham algorithms debugging");
	DebugMan.addDebugChannel(kDebugLevelSound, "Sound", "Sound debugging");
	DebugMan.addDebugChannel(kDebugLevelGfxDriver, "Gfxdriver", "Gfx driver debugging");
	DebugMan.addDebugChannel(kDebugLevelBaseSetter, "Base", "Base Setter debugging");
	DebugMan.addDebugChannel(kDebugLevelParser, "Parser", "Parser debugging");
	DebugMan.addDebugChannel(kDebugLevelMenu, "Menu", "Menu handling debugging");
	DebugMan.addDebugChannel(kDebugLevelSaid, "Said", "Said specs debugging");
	DebugMan.addDebugChannel(kDebugLevelFile, "File", "File I/O debugging");
	DebugMan.addDebugChannel(kDebugLevelTime, "Time", "Time debugging");
	DebugMan.addDebugChannel(kDebugLevelRoom, "Room", "Room number debugging");
	DebugMan.addDebugChannel(kDebugLevelAvoidPath, "Pathfinding", "Pathfinding debugging");
	DebugMan.addDebugChannel(kDebugLevelDclInflate, "DCL", "DCL inflate debugging");
	DebugMan.addDebugChannel(kDebugLevelVM, "VM", "VM debugging");
	DebugMan.addDebugChannel(kDebugLevelScripts, "Scripts", "Notifies when scripts are unloaded");
	DebugMan.addDebugChannel(kDebugLevelGC, "GC", "Garbage Collector debugging");
	DebugMan.addDebugChannel(kDebugLevelSci0Pic, "Sci0Pic", "SCI0 pic drawing debugging");
	DebugMan.addDebugChannel(kDebugLevelResMan, "ResMan", "Resource manager debugging");
	DebugMan.addDebugChannel(kDebugLevelOnStartup, "OnStartup", "Enter debugger at start of game");

	_gamestate = 0;
	_gfxMacIconBar = 0;

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "actors");	// KQ6 hi-res portraits
	SearchMan.addSubDirectoryMatching(gameDataDir, "aud");	// resource.aud and audio files
	SearchMan.addSubDirectoryMatching(gameDataDir, "avi");	// AVI movie files for Windows versions
	SearchMan.addSubDirectoryMatching(gameDataDir, "seq");	// SEQ movie files for DOS versions
	SearchMan.addSubDirectoryMatching(gameDataDir, "wav");	// speech files in WAV format
	SearchMan.addSubDirectoryMatching(gameDataDir, "sfx");	// music/sound files in WAV format
	SearchMan.addSubDirectoryMatching(gameDataDir, "robot");	// robot files

	// Add the patches directory, except for KQ6CD; The patches folder in some versions of KQ6CD
	// is for the demo of Phantasmagoria, included in the disk
	if (strcmp(getGameID(), "kq6"))
		SearchMan.addSubDirectoryMatching(gameDataDir, "patches");	// resource patches
}

SciEngine::~SciEngine() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

	delete _audio;
	delete _kernel;
	delete _vocabulary;
	delete _console;
	delete _resMan;
	delete _features;
	delete _macIconBar;

	g_sci = 0;
}

Common::Error SciEngine::run() {
	// Assign default values to the config manager, in case settings are missing
	ConfMan.registerDefault("undither", "true");
	ConfMan.registerDefault("enable_fb01", "false");

	_resMan = new ResourceManager();

	if (!_resMan) {
		warning("No resources found, aborting");
		return Common::kNoGameDataFoundError;
	}

	SegManager *segMan = new SegManager(_resMan);

	// Scale the screen, if needed
	int upscaledHires = GFX_SCREEN_UPSCALED_DISABLED;

	// King's Quest 6 and Gabriel Knight 1 have hires content, gk1/cd was able to provide that under DOS as well, but as
	//  gk1/floppy does support upscaled hires scriptswise, but doesn't actually have the hires content we need to limit
	//  it to platform windows.
	if (getPlatform() == Common::kPlatformWindows) {
		if (!strcmp(getGameID(), "kq6"))
			upscaledHires = GFX_SCREEN_UPSCALED_640x440;
#ifdef ENABLE_SCI32
		if (!strcmp(getGameID(), "gk1"))
			upscaledHires = GFX_SCREEN_UPSCALED_640x480;
#endif
	}

	// Japanese versions of games use hi-res font on upscaled version of the game
	if ((getLanguage() == Common::JA_JPN) && (getSciVersion() <= SCI_VERSION_1_1))
		upscaledHires = GFX_SCREEN_UPSCALED_640x400;

	// Initialize graphics-related parts
	GfxScreen *screen = 0;

	// invokes initGraphics()
	if (_resMan->detectHires())
		screen = new GfxScreen(_resMan, 640, 480);
	else
		screen = new GfxScreen(_resMan, 320, 200, upscaledHires);

	if (_resMan->isSci11Mac() && getSciVersion() == SCI_VERSION_1_1)
		_gfxMacIconBar = new GfxMacIconBar();

	GfxPalette *palette = new GfxPalette(_resMan, screen);
	GfxCache *cache = new GfxCache(_resMan, screen, palette);
	GfxCursor *cursor = new GfxCursor(_resMan, palette, screen);

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);

	_kernel = new Kernel(_resMan, segMan);
	// Only SCI0 and SCI01 games used a parser
	_vocabulary = (getSciVersion() <= SCI_VERSION_1_EGA) ? new Vocabulary(_resMan) : NULL;
	_audio = new AudioPlayer(_resMan);

	_features = new GameFeatures(segMan, _kernel);

	_gamestate = new EngineState(_vocabulary, segMan);

	_gamestate->_event = new SciEvent(_resMan);

	if (script_init_engine(_gamestate))
		return Common::kUnknownError;

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		_gfxAnimate = 0;
		_gfxControls = 0;
		_gfxMenu = 0;
		_gfxPaint16 = 0;
		_gfxPorts = 0;
		_gui = 0;
		_gui32 = new SciGui32(_gamestate->_segMan, _gamestate->_event, screen, palette, cache, cursor);
	} else {
#endif
		_gfxPorts = new GfxPorts(segMan, screen);
		_gui = new SciGui(_gamestate, screen, palette, cache, cursor, _gfxPorts, _audio);
#ifdef ENABLE_SCI32
		_gui32 = 0;
		_gfxFrameout = 0;
	}
#endif

	_gfxPalette = palette;
	_gfxScreen = screen;
	_gfxCache = cache;
	_gfxCursor = cursor;

	if (game_init(_gamestate)) { /* Initialize */
		warning("Game initialization failed: Aborting...");
		// TODO: Add an "init failed" error?
		return Common::kUnknownError;
	}

	// Add the after market GM patches for the specified game, if they exist
	_resMan->addNewGMPatch(getGameID());

	script_adjust_opcode_formats(_gamestate);
	_kernel->loadKernelNames(getGameID());

	// Set the savegame dir (actually, we set it to a fake value,
	// since we cannot let the game control where saves are stored)
	assert(_gamestate->sys_strings->_strings[SYS_STRING_SAVEDIR]._value != 0);
	strcpy(_gamestate->sys_strings->_strings[SYS_STRING_SAVEDIR]._value, "");

	SciVersion soundVersion = _features->detectDoSoundType();

	_gamestate->_soundCmd = new SoundCommandParser(_resMan, segMan, _kernel, _audio, soundVersion);

	screen->debugUnditherSetState(ConfMan.getBool("undither"));

#ifdef USE_OLD_MUSIC_FUNCTIONS
	if (game_init_sound(_gamestate, 0, soundVersion)) {
		warning("Game initialization failed: Error in sound subsystem. Aborting...");
		return Common::kUnknownError;
	}
#endif

	syncSoundSettings();

#ifdef ENABLE_SCI32
	if (_gui32)
		_gui32->init();
	else
#endif
		_gui->init(_features->usesOldGfxFunctions());

	debug("Emulating SCI version %s\n", getSciVersionDesc(getSciVersion()));

	// Check whether loading a savestate was requested
	if (ConfMan.hasKey("save_slot")) {
		g_loadFromLauncher = ConfMan.getInt("save_slot");
	} else {
		g_loadFromLauncher = -1;
	}

	game_run(&_gamestate); // Run the game

	game_exit(_gamestate);

	ConfMan.flushToDisk();

	delete _gamestate->_soundCmd;
	delete _gui;
#ifdef ENABLE_SCI32
	delete _gui32;
#endif
	delete _gfxPorts;
	delete _gfxCache;
	delete _gfxPalette;
	delete cursor;
	delete _gfxScreen;
	delete _gamestate->_event;
	delete segMan;
	delete _gamestate;

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

bool SciEngine::isDemo() const {
	return getFlags() & ADGF_DEMO;
}

Common::String SciEngine::getSavegameName(int nr) const {
	return _targetName + Common::String::printf(".%03d", nr);
}

Common::String SciEngine::getSavegamePattern() const {
	return _targetName + ".???";
}

Common::String SciEngine::getFilePrefix() const {
	const char* gameID = getGameID();
	if (!strcmp(gameID, "qfg2")) {
		// Quest for Glory 2 wants to read files from Quest for Glory 1 (EGA/VGA) to import character data
		if (_gamestate->currentRoomNumber() == 805)
			return "qfg1";
		// TODO: Include import-room for qfg1vga
	}
	if (!strcmp(gameID, "qfg3")) {
		// Quest for Glory 3 wants to read files from Quest for Glory 2 to import character data
		if (_gamestate->currentRoomNumber() == 54)
			return "qfg2";
	}
	// TODO: Implement the same for qfg4, when sci32 is good enough
	return _targetName;
}

Common::String SciEngine::wrapFilename(const Common::String &name) const {
	return getFilePrefix() + "-" + name;
}

Common::String SciEngine::unwrapFilename(const Common::String &name) const {
	Common::String prefix = getFilePrefix() + "-";
	if (name.hasPrefix(prefix.c_str()))
		return Common::String(name.c_str() + prefix.size());
	return name;
}

void SciEngine::pauseEngineIntern(bool pause) {
#ifdef USE_OLD_MUSIC_FUNCTIONS
	_gamestate->_sound.sfx_suspend(pause);
#endif
	_mixer->pauseAll(pause);
}

void SciEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

#ifndef USE_OLD_MUSIC_FUNCTIONS
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	int soundVolumeMusic = (mute ? 0 : ConfMan.getInt("music_volume"));

	if (_gamestate && _gamestate->_soundCmd) {
		int vol =  (soundVolumeMusic + 1) * SoundCommandParser::kMaxSciVolume / Audio::Mixer::kMaxMixerVolume;
		_gamestate->_soundCmd->setMasterVolume(vol);
	}
#endif
}

} // End of namespace Sci
