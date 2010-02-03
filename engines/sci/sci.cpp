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
#include "sci/event.h"

#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"	// for script_adjust_opcode_formats

#include "sci/sound/audio.h"
#include "sci/sound/soundcmd.h"
#include "sci/graphics/gui.h"
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
	Common::addDebugChannel(kDebugLevelOnStartup, "OnStartup", "Enter debugger at start of game");

	_gamestate = 0;

	SearchMan.addSubDirectoryMatching(_gameDataDir, "actors");	// KQ6 hi-res portraits
	SearchMan.addSubDirectoryMatching(_gameDataDir, "aud");	// resource.aud and audio files
	SearchMan.addSubDirectoryMatching(_gameDataDir, "avi");	// AVI movie files for Windows versions
	//SearchMan.addSubDirectoryMatching(_gameDataDir, "patches");	// resource patches
	SearchMan.addSubDirectoryMatching(_gameDataDir, "seq");	// SEQ movie files for DOS versions
	SearchMan.addSubDirectoryMatching(_gameDataDir, "wav");	// speech files in WAV format
	SearchMan.addSubDirectoryMatching(_gameDataDir, "sfx");	// music/sound files in WAV format
	SearchMan.addSubDirectoryMatching(_gameDataDir, "robot");	// robot files
}

SciEngine::~SciEngine() {
	// Remove all of our debug levels here
	Common::clearAllDebugChannels();

	delete _audio;
	delete _kernel;
	delete _vocabulary;
	delete _console;
	delete _resMan;
}

Common::Error SciEngine::run() {
	// FIXME/TODO: Move some of the stuff below to init()

	// Assign default values to the config manager, in case settings are missing
	ConfMan.registerDefault("undither", "true");
	ConfMan.registerDefault("enable_fb01", "false");

	_resMan = new ResourceManager();

	if (!_resMan) {
		warning("No resources found, aborting");
		return Common::kNoGameDataFoundError;
	}

	// Scale the screen, if needed
	bool upscaledHires = false;

	// King's Quest 6 and Gabriel Knight 1 have hires content, gk1/cd was able to provide that under DOS as well, but as
	//  gk1/floppy does support upscaled hires scriptswise, but doesn't actually have the hires content we need to limit
	//  it to platform windows.
	if (getPlatform() == Common::kPlatformWindows) {
		if (!strcmp(getGameID(), "kq6"))
			upscaledHires = true;
#ifdef ENABLE_SCI32
		if (!strcmp(getGameID(), "gk1"))
			upscaledHires = true;
#endif
	}

	// Japanese versions of games use hi-res font on upscaled version of the game
	if ((getLanguage() == Common::JA_JPN) && (getSciVersion() <= SCI_VERSION_1_1))
		upscaledHires = true;

	// Initialize graphics-related parts
	GfxScreen *screen = 0;

	// invokes initGraphics()
	if (_resMan->detectHires())
		screen = new GfxScreen(_resMan, 640, 480, false);
	else
		screen = new GfxScreen(_resMan, 320, 200, upscaledHires);

	GfxPalette *palette = new GfxPalette(_resMan, screen);
	GfxCache *cache = new GfxCache(_resMan, screen, palette);
	Cursor *cursor = new Cursor(_resMan, palette, screen);

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);

	_kernel = new Kernel(_resMan);
	// Only SCI0 and SCI01 games used a parser
	_vocabulary = (getSciVersion() <= SCI_VERSION_1_EGA) ? new Vocabulary(_resMan) : NULL;
	_audio = new AudioPlayer(_resMan);

	SegManager *segMan = new SegManager(_resMan);

	// We'll set the GUI below
	_gamestate = new EngineState(_resMan, _kernel, _vocabulary, segMan, NULL, _audio);
	_gamestate->_event = new SciEvent();

	if (script_init_engine(_gamestate))
		return Common::kUnknownError;

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		_gamestate->_gfxAnimate = 0;
		_gamestate->_gfxControls = 0;
		_gamestate->_gfxMenu = 0;
		_gamestate->_gfxPorts = 0;
		_gamestate->_gui = 0;
		_gamestate->_gui32 = new SciGui32(_gamestate, screen, palette, cache, cursor);
	} else {
#endif
		_gamestate->_gfxPorts = new GfxPorts(segMan, screen);
		_gamestate->_gui = new SciGui(_gamestate, screen, palette, cache, cursor, _gamestate->_gfxPorts, _audio);
#ifdef ENABLE_SCI32
		_gamestate->_gui32 = 0;
	}
#endif

	_gamestate->_gfxPalette = palette;
	_gamestate->_gfxScreen = screen;
	_gamestate->_gfxCache = cache;

	if (game_init(_gamestate)) { /* Initialize */
		warning("Game initialization failed: Aborting...");
		// TODO: Add an "init failed" error?
		return Common::kUnknownError;
	}

	// Add the after market GM patches for the specified game, if they exist
	_resMan->addNewGMPatch(_gamestate->_gameId);

	script_adjust_opcode_formats(_gamestate);
	_kernel->loadKernelNames(getGameID(), _gamestate);

	// Set the savegame dir (actually, we set it to a fake value,
	// since we cannot let the game control where saves are stored)
	assert(_gamestate->sys_strings->_strings[SYS_STRING_SAVEDIR]._value != 0);
	strcpy(_gamestate->sys_strings->_strings[SYS_STRING_SAVEDIR]._value, "");

	_gamestate->_features->setGameInfo(_gamestate->_gameObj, _gamestate->_gameId);

	SciVersion soundVersion = _gamestate->_features->detectDoSoundType();

	_gamestate->_soundCmd = new SoundCommandParser(_resMan, segMan, _kernel, _audio, soundVersion);

	screen->unditherSetState(ConfMan.getBool("undither"));

#ifdef USE_OLD_MUSIC_FUNCTIONS
	if (game_init_sound(_gamestate, 0, soundVersion)) {
		warning("Game initialization failed: Error in sound subsystem. Aborting...");
		return Common::kUnknownError;
	}
#endif

	syncSoundSettings();

#ifdef ENABLE_SCI32
	if (_gamestate->_gui32)
		_gamestate->_gui32->init();
	else
#endif
		_gamestate->_gui->init(_gamestate->_features->usesOldGfxFunctions());

	debug("Emulating SCI version %s\n", getSciVersionDesc(getSciVersion()).c_str());

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
	delete _gamestate->_gui;
	delete _gamestate->_event;
	delete segMan;
	delete cursor;
	delete palette;
	delete screen;
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
