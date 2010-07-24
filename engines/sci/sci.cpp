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
#include "sci/engine/message.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"	// for script_adjust_opcode_formats
#include "sci/engine/selector.h"	// for SELECTOR

#include "sci/sound/audio.h"
#include "sci/sound/soundcmd.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/controls.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/maciconbar.h"
#include "sci/graphics/menu.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/transitions.h"

#ifdef ENABLE_SCI32
#include "sci/graphics/frameout.h"
#endif

namespace Sci {

SciEngine *g_sci = 0;


class GfxDriver;

SciEngine::SciEngine(OSystem *syst, const ADGameDescription *desc, SciGameId gameId)
		: Engine(syst), _gameDescription(desc), _gameId(gameId) {

	assert(g_sci == 0);
	g_sci = this;

	_gfxMacIconBar = 0;

	_audio = 0;
	_features = 0;
	_resMan = 0;
	_gamestate = 0;
	_kernel = 0;
	_vocabulary = 0;
	_vocabularyLanguage = 1; // we load english vocabulary on startup
	_eventMan = 0;
	_console = 0;

	// Set up the engine specific debug levels
	DebugMan.addDebugChannel(kDebugLevelError, "Error", "Script error debugging");
	DebugMan.addDebugChannel(kDebugLevelNodes, "Lists", "Lists and nodes debugging");
	DebugMan.addDebugChannel(kDebugLevelGraphics, "Graphics", "Graphics debugging");
	DebugMan.addDebugChannel(kDebugLevelStrings, "Strings", "Strings debugging");
	DebugMan.addDebugChannel(kDebugLevelMemory, "Memory", "Memory debugging");
	DebugMan.addDebugChannel(kDebugLevelFuncCheck, "Func", "Function parameter debugging");
	DebugMan.addDebugChannel(kDebugLevelBresen, "Bresenham", "Bresenham algorithms debugging");
	DebugMan.addDebugChannel(kDebugLevelSound, "Sound", "Sound debugging");
	DebugMan.addDebugChannel(kDebugLevelBaseSetter, "Base", "Base Setter debugging");
	DebugMan.addDebugChannel(kDebugLevelParser, "Parser", "Parser debugging");
	DebugMan.addDebugChannel(kDebugLevelSaid, "Said", "Said specs debugging");
	DebugMan.addDebugChannel(kDebugLevelFile, "File", "File I/O debugging");
	DebugMan.addDebugChannel(kDebugLevelTime, "Time", "Time debugging");
	DebugMan.addDebugChannel(kDebugLevelRoom, "Room", "Room number debugging");
	DebugMan.addDebugChannel(kDebugLevelAvoidPath, "Pathfinding", "Pathfinding debugging");
	DebugMan.addDebugChannel(kDebugLevelDclInflate, "DCL", "DCL inflate debugging");
	DebugMan.addDebugChannel(kDebugLevelVM, "VM", "VM debugging");
	DebugMan.addDebugChannel(kDebugLevelScripts, "Scripts", "Notifies when scripts are unloaded");
	DebugMan.addDebugChannel(kDebugLevelGC, "GC", "Garbage Collector debugging");
	DebugMan.addDebugChannel(kDebugLevelResMan, "ResMan", "Resource manager debugging");
	DebugMan.addDebugChannel(kDebugLevelOnStartup, "OnStartup", "Enter debugger at start of game");

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "actors");	// KQ6 hi-res portraits
	SearchMan.addSubDirectoryMatching(gameDataDir, "aud");	// resource.aud and audio files
	SearchMan.addSubDirectoryMatching(gameDataDir, "wav");	// speech files in WAV format
	SearchMan.addSubDirectoryMatching(gameDataDir, "sfx");	// music/sound files in WAV format
	SearchMan.addSubDirectoryMatching(gameDataDir, "avi");	// AVI movie files for Windows versions
	SearchMan.addSubDirectoryMatching(gameDataDir, "seq");	// SEQ movie files for DOS versions
	SearchMan.addSubDirectoryMatching(gameDataDir, "robot");	// robot movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "movies");	// vmd movie files
	SearchMan.addSubDirectoryMatching(gameDataDir, "vmd");	// vmd movie files

	// Add the patches directory, except for KQ6CD; The patches folder in some versions of KQ6CD
	// is for the demo of Phantasmagoria, included in the disk
	if (_gameId != GID_KQ6)
		SearchMan.addSubDirectoryMatching(gameDataDir, "patches");	// resource patches
}

SciEngine::~SciEngine() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

#ifdef ENABLE_SCI32
	delete _gfxFrameout;
#endif
	delete _gfxMenu;
	delete _gfxControls;
	delete _gfxText16;
	delete _gfxAnimate;
	delete _gfxPaint;
	delete _gfxTransitions;
	delete _gfxCompare;
	delete _gfxCoordAdjuster;
	delete _gfxPorts;
	delete _gfxCache;
	delete _gfxPalette;
	delete _gfxCursor;
	delete _gfxScreen;

	delete _audio;
	delete _soundCmd;
	delete _kernel;
	delete _vocabulary;
	delete _console;
	delete _features;
	delete _gfxMacIconBar;

	delete _eventMan;
	delete _gamestate->_segMan;
	delete _gamestate;
	delete _resMan;	// should be deleted last
	g_sci = 0;
}

Common::Error SciEngine::run() {
	// Assign default values to the config manager, in case settings are missing
	ConfMan.registerDefault("undither", "true");
	ConfMan.registerDefault("enable_fb01", "false");

	_resMan = new ResourceManager();
	assert(_resMan);
	_resMan->addAppropriateSources();
	_resMan->init();

	// TODO: Add error handling. Check return values of addAppropriateSources
	// and init. We first have to *add* sensible return values, though ;).
/*
	if (!_resMan) {
		warning("No resources found, aborting");
		return Common::kNoGameDataFoundError;
	}
*/

	// Reset, so that error()s before SoundCommandParser is initialized wont cause a crash
	_soundCmd = NULL;

	// Add the after market GM patches for the specified game, if they exist
	_resMan->addNewGMPatch(_gameId);
	_gameObj = _resMan->findGameObject();

	SegManager *segMan = new SegManager(_resMan);

	// Initialize the game screen
	_gfxScreen = new GfxScreen(_resMan);
	_gfxScreen->debugUnditherSetState(ConfMan.getBool("undither"));

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);
	_kernel = new Kernel(_resMan, segMan);
	_features = new GameFeatures(segMan, _kernel);
	// Only SCI0 and SCI01 games used a parser
	_vocabulary = (getSciVersion() <= SCI_VERSION_1_EGA) ? new Vocabulary(_resMan, false) : NULL;
	_audio = new AudioPlayer(_resMan);
	_gamestate = new EngineState(segMan);
	_eventMan = new EventManager(_resMan->detectFontExtended());

	// The game needs to be initialized before the graphics system is initialized, as
	// the graphics code checks parts of the seg manager upon initialization (e.g. for
	// the presence of the fastCast object)
	if (!initGame()) { /* Initialize */
		warning("Game initialization failed: Aborting...");
		// TODO: Add an "init failed" error?
		return Common::kUnknownError;
	}

	script_adjust_opcode_formats();

	// Must be called after game_init(), as they use _features
	_kernel->loadKernelNames(_features);
	_soundCmd = new SoundCommandParser(_resMan, segMan, _kernel, _audio, _features->detectDoSoundType());

	syncSoundSettings();

	// Initialize all graphics related subsystems
	initGraphics();

	debug("Emulating SCI version %s\n", getSciVersionDesc(getSciVersion()));

	// Check whether loading a savestate was requested
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) {
		reg_t restoreArgv[2] = { NULL_REG, make_reg(0, saveSlot) };	// special call (argv[0] is NULL)
		kRestoreGame(_gamestate, 2, restoreArgv);

		// Initialize the game menu, if there is one.
		// This is not done when loading, so we must do it manually.
		reg_t menuBarObj = _gamestate->_segMan->findObjectByName("MenuBar");
		if (menuBarObj.isNull())
			menuBarObj = _gamestate->_segMan->findObjectByName("TheMenuBar");	// LSL2
		if (menuBarObj.isNull())
			menuBarObj = _gamestate->_segMan->findObjectByName("menuBar");	// LSL6
		if (!menuBarObj.isNull()) {
			// Reset abortScriptProcessing before initializing the game menu, so that the
			// VM call performed by invokeSelector will actually run.
			_gamestate->abortScriptProcessing = kAbortNone;
			Object *menuBar = _gamestate->_segMan->getObject(menuBarObj);
			// Invoke the first method (init) of the menuBar object
			invokeSelector(_gamestate, menuBarObj, menuBar->getFuncSelector(0), 0, _gamestate->stack_base);
			_gamestate->abortScriptProcessing = kAbortLoadGame;
		}
	}

	runGame();

	ConfMan.flushToDisk();

	return Common::kNoError;
}

bool SciEngine::initGame() {
	// Script 0 needs to be allocated here before anything else!
	int script0Segment = _gamestate->_segMan->getScriptSegment(0, SCRIPT_GET_LOCK);
	DataStack *stack = _gamestate->_segMan->allocateStack(VM_STACK_SIZE, NULL);

	_gamestate->_msgState = new MessageState(_gamestate->_segMan);
	_gamestate->gcCountDown = GC_INTERVAL - 1;

	// Script 0 should always be at segment 1
	if (script0Segment != 1) {
		debug(2, "Failed to instantiate script.000");
		return false;
	}

	_gamestate->initGlobals();

	if (_gamestate->abortScriptProcessing == kAbortRestartGame && _gfxMenu)
		_gfxMenu->reset();

	_gamestate->_segMan->initSysStrings();

	_gamestate->r_acc = _gamestate->r_prev = NULL_REG;

	_gamestate->_executionStack.clear();    // Start without any execution stack
	_gamestate->executionStackBase = -1; // No vm is running yet
	_gamestate->_executionStackPosChanged = false;

	_gamestate->abortScriptProcessing = kAbortNone;
	_gamestate->gameWasRestarted = false;

	_gamestate->stack_base = stack->_entries;
	_gamestate->stack_top = stack->_entries + stack->_capacity;

	if (!_gamestate->_segMan->instantiateScript(0)) {
		error("initGame(): Could not instantiate script 0");
		return false;
	}

	// Reset parser
	if (_vocabulary) {
		_vocabulary->reset();
	}

	_gamestate->gameStartTime = _gamestate->lastWaitTime = _gamestate->_screenUpdateTime = g_system->getMillis();

	srand(g_system->getMillis()); // Initialize random number generator

	// Load game language into printLang property of game object
	setSciLanguage();

	return true;
}

void SciEngine::initGraphics() {

	// Reset all graphics objects
	_gfxAnimate = 0;
	_gfxCache = 0;
	_gfxCompare = 0;
	_gfxControls = 0;
	_gfxCoordAdjuster = 0;
	_gfxCursor = 0;
	_gfxMacIconBar = 0;
	_gfxMenu = 0;
	_gfxPaint = 0;
	_gfxPaint16 = 0;
	_gfxPalette = 0;
	_gfxPorts = 0;
	_gfxText16 = 0;
	_gfxTransitions = 0;
#ifdef ENABLE_SCI32
	_gfxFrameout = 0;
	_gfxPaint32 = 0;
#endif

	if (_resMan->isSci11Mac() && getSciVersion() == SCI_VERSION_1_1)
		_gfxMacIconBar = new GfxMacIconBar();

	bool paletteMerging = true;
	if (getSciVersion() >= SCI_VERSION_1_1) {
		// there are some games that use inbetween SCI1.1 interpreter, so we have to detect if it's merging or copying
		if (getSciVersion() == SCI_VERSION_1_1)
			paletteMerging = _resMan->detectForPaletteMergingForSci11();
		else
			paletteMerging = false;
	}

	_gfxPalette = new GfxPalette(_resMan, _gfxScreen, paletteMerging);
	_gfxCache = new GfxCache(_resMan, _gfxScreen, _gfxPalette);
	_gfxCursor = new GfxCursor(_resMan, _gfxPalette, _gfxScreen);

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		// SCI32 graphic objects creation
		_gfxCoordAdjuster = new GfxCoordAdjuster32(_gamestate->_segMan);
		_gfxCursor->init(_gfxCoordAdjuster, _eventMan);
		_gfxCompare = new GfxCompare(_gamestate->_segMan, g_sci->getKernel(), _gfxCache, _gfxScreen, _gfxCoordAdjuster);
		_gfxPaint32 = new GfxPaint32(g_sci->getResMan(), _gamestate->_segMan, g_sci->getKernel(), _gfxCoordAdjuster, _gfxCache, _gfxScreen, _gfxPalette);
		_gfxPaint = _gfxPaint32;
		_gfxFrameout = new GfxFrameout(_gamestate->_segMan, g_sci->getResMan(), _gfxCoordAdjuster, _gfxCache, _gfxScreen, _gfxPalette, _gfxPaint32);
	} else {
#endif
		// SCI0-SCI1.1 graphic objects creation
		_gfxPorts = new GfxPorts(_gamestate->_segMan, _gfxScreen);
		_gfxCoordAdjuster = new GfxCoordAdjuster16(_gfxPorts);
		_gfxCursor->init(_gfxCoordAdjuster, g_sci->getEventManager());
		_gfxCompare = new GfxCompare(_gamestate->_segMan, g_sci->getKernel(), _gfxCache, _gfxScreen, _gfxCoordAdjuster);
		_gfxTransitions = new GfxTransitions(_gfxScreen, _gfxPalette, g_sci->getResMan()->isVGA());
		_gfxPaint16 = new GfxPaint16(g_sci->getResMan(), _gamestate->_segMan, g_sci->getKernel(), _gfxCache, _gfxPorts, _gfxCoordAdjuster, _gfxScreen, _gfxPalette, _gfxTransitions, _audio);
		_gfxPaint = _gfxPaint16;
		_gfxAnimate = new GfxAnimate(_gamestate, _gfxCache, _gfxPorts, _gfxPaint16, _gfxScreen, _gfxPalette, _gfxCursor, _gfxTransitions);
		_gfxText16 = new GfxText16(g_sci->getResMan(), _gfxCache, _gfxPorts, _gfxPaint16, _gfxScreen);
		_gfxControls = new GfxControls(_gamestate->_segMan, _gfxPorts, _gfxPaint16, _gfxText16, _gfxScreen);
		_gfxMenu = new GfxMenu(g_sci->getEventManager(), _gamestate->_segMan, _gfxPorts, _gfxPaint16, _gfxText16, _gfxScreen, _gfxCursor);

		_gfxMenu->reset();
#ifdef ENABLE_SCI32
	}
#endif

	if (_gfxPorts) {
		_gfxPorts->init(_features->usesOldGfxFunctions(), _gfxPaint16, _gfxText16);
		_gfxPaint16->init(_gfxAnimate, _gfxText16);
	}
	// Set default (EGA, amiga or resource 999) palette
	_gfxPalette->setDefault();
}

void SciEngine::initStackBaseWithSelector(Selector selector) {
	_gamestate->stack_base[0] = make_reg(0, (uint16)selector);
	_gamestate->stack_base[1] = NULL_REG;

	// Register the first element on the execution stack
	if (!send_selector(_gamestate, _gameObj, _gameObj, _gamestate->stack_base, 2, _gamestate->stack_base)) {
		_console->printObject(_gameObj);
		error("initStackBaseWithSelector: error while registering the first selector in the call stack");
	}

}

void SciEngine::runGame() {
	initStackBaseWithSelector(SELECTOR(play)); // Call the play selector

	// Attach the debug console on game startup, if requested
	if (DebugMan.isDebugChannelEnabled(kDebugLevelOnStartup))
		_console->attach();

	do {
		_gamestate->_executionStackPosChanged = false;
		run_vm(_gamestate);
		exitGame();

		if (_gamestate->abortScriptProcessing == kAbortRestartGame) {
			_gamestate->abortScriptProcessing = kAbortNone;
			_gamestate->_segMan->resetSegMan();
			initGame();
			initStackBaseWithSelector(SELECTOR(play));
			_gamestate->gameWasRestarted = true;
		} else if (_gamestate->abortScriptProcessing == kAbortLoadGame) {
			_gamestate->abortScriptProcessing = kAbortNone;
			_gamestate->_executionStack.clear();
			initStackBaseWithSelector(SELECTOR(replay));
			_gamestate->shrinkStackToBase();
		} else {
			break;	// exit loop
		}
	} while (true);
}

void SciEngine::exitGame() {
	if (_gamestate->abortScriptProcessing != kAbortLoadGame) {
		_gamestate->_executionStack.clear();
		_audio->stopAllAudio();
		g_sci->_soundCmd->clearPlayList();
	}

	// TODO Free parser segment here

	// TODO Free scripts here

	// Close all opened file handles
	_gamestate->_fileHandles.clear();
	_gamestate->_fileHandles.resize(5);
}

// Invoked by error() when a severe error occurs
GUI::Debugger *SciEngine::getDebugger() {
	if (_gamestate) {
		ExecStack *xs = &(_gamestate->_executionStack.back());
		xs->addr.pc.offset = _debugState.old_pc_offset;
		xs->sp = _debugState.old_sp;
	}

	_debugState.runningStep = 0; // Stop multiple execution
	_debugState.seeking = kDebugSeekNothing; // Stop special seeks

	return _console;
}

// Used to obtain the engine's console in order to print messages to it
Console *SciEngine::getSciDebugger() {
	return _console;
}

const char *SciEngine::getGameIdStr() const {
	return _gameDescription->gameid;
}

Common::Language SciEngine::getLanguage() const {
	return _gameDescription->language;
}

Common::Platform SciEngine::getPlatform() const {
	return _gameDescription->platform;
}

bool SciEngine::isDemo() const {
	return _gameDescription->flags & ADGF_DEMO;
}

Common::String SciEngine::getSavegameName(int nr) const {
	return _targetName + Common::String::printf(".%03d", nr);
}

Common::String SciEngine::getSavegamePattern() const {
	return _targetName + ".???";
}

Common::String SciEngine::getFilePrefix() const {
	if (_gameId == GID_QFG2) {
		// Quest for Glory 2 wants to read files from Quest for Glory 1 (EGA/VGA) to import character data
		if (_gamestate->currentRoomNumber() == 805)
			return "qfg1";
		// TODO: Include import-room for qfg1vga
	} else if (_gameId == GID_QFG3) {
		// Quest for Glory 3 wants to read files from Quest for Glory 2 to import character data
		if (_gamestate->currentRoomNumber() == 54)
			return "qfg2";
	} else if (_gameId == GID_QFG4) {
		// Quest for Glory 4 wants to read files from Quest for Glory 3 to import character data
		if (_gamestate->currentRoomNumber() == 54)
			return "qfg3";
	}
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
	_mixer->pauseAll(pause);
}

void SciEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	int soundVolumeMusic = (mute ? 0 : ConfMan.getInt("music_volume"));

	if (_gamestate && g_sci->_soundCmd) {
		int vol =  (soundVolumeMusic + 1) * SoundCommandParser::kMaxSciVolume / Audio::Mixer::kMaxMixerVolume;
		g_sci->_soundCmd->setMasterVolume(vol);
	}
}

} // End of namespace Sci
