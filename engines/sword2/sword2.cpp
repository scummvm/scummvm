/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/console.h"
#include "sword2/controls.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/screen.h"
#include "sword2/sound.h"

#ifdef _WIN32_WCE
extern bool isSmartphone();
#endif

struct Sword2GameSettings {
	const char *gameid;
	const char *description;
	uint32 features;
	const char *detectname;
};

static const Sword2GameSettings sword2_settings[] = {
	/* Broken Sword 2 */
	{"sword2", "Broken Sword 2: The Smoking Mirror", 0, "players.clu" },
	{"sword2alt", "Broken Sword 2: The Smoking Mirror (alt)", 0, "r2ctlns.ocx" },
	{"sword2demo", "Broken Sword 2: The Smoking Mirror (Demo)", Sword2::GF_DEMO, "players.clu" },
	{NULL, NULL, 0, NULL}
};

GameList Engine_SWORD2_gameIDList() {
	const Sword2GameSettings *g = sword2_settings;
	GameList games;
	while (g->gameid) {
		games.push_back(toGameSettings(*g));
		g++;
	}
	return games;
}

GameSettings Engine_SWORD2_findGameID(const char *gameid) {
	const Sword2GameSettings *g = sword2_settings;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			break;
		g++;
	}
	return toGameSettings(*g);
}

DetectedGameList Engine_SWORD2_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const Sword2GameSettings *g;

	// TODO: It would be nice if we had code here which distinguishes
	// between the 'sword2' and 'sword2demo' targets. The current code
	// can't do that since they use the same detectname.

	for (g = sword2_settings; g->gameid; ++g) {
		// Iterate over all files in the given directory
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				const char *gameName = file->displayName().c_str();

				if (0 == scumm_stricmp(g->detectname, gameName)) {
					// Match found, add to list of candidates, then abort inner loop.
					detectedGames.push_back(toGameSettings(*g));
					break;
				}
			}
		}
	}
	return detectedGames;
}

Engine *Engine_SWORD2_create(GameDetector *detector, OSystem *syst) {
	return new Sword2::Sword2Engine(detector, syst);
}

REGISTER_PLUGIN(SWORD2, "Broken Sword 2")

namespace Sword2 {

Sword2Engine::Sword2Engine(GameDetector *detector, OSystem *syst) : Engine(syst) {
	// Add default file directories
	Common::File::addDefaultDirectory(_gameDataPath + "CLUSTERS/");
	Common::File::addDefaultDirectory(_gameDataPath + "SWORD2/");
	Common::File::addDefaultDirectory(_gameDataPath + "VIDEO/");
	Common::File::addDefaultDirectory(_gameDataPath + "clusters/");
	Common::File::addDefaultDirectory(_gameDataPath + "sword2/");
	Common::File::addDefaultDirectory(_gameDataPath + "video/");

	if (0 == scumm_stricmp(detector->_gameid.c_str(), "sword2demo"))
		_features = GF_DEMO;
	else
		_features = 0;
	
	_targetName = detector->_targetName;

	_bootParam = ConfMan.getInt("boot_param");
	_saveSlot = ConfMan.getInt("save_slot");

	_memory = NULL;
	_resman = NULL;
	_sound = NULL;
	_screen = NULL;
	_mouse = NULL;
	_logic = NULL;
	_fontRenderer = NULL;
	_debugger = NULL;

	_keyboardEvent.pending = false;
	_keyboardEvent.repeat = 0;
	_mouseEvent.pending = false;

	_wantSfxDebug = false;

#ifdef SWORD2_DEBUG
	_stepOneCycle = false;
	_renderSkip = false;
#endif

	_gamePaused = false;
	_graphicsLevelFudged = false;

	_gameCycle = 0;

	_quit = false;
}

Sword2Engine::~Sword2Engine() {
	delete _debugger;
	delete _sound;
	delete _fontRenderer;
	delete _screen;
	delete _mouse;
	delete _logic;
	delete _resman;
	delete _memory;
}

void Sword2Engine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);

#ifdef _WIN32_WCE
	if (isSmartphone())
		return;
#endif

	// Unless an error -originated- within the debugger, spawn the
	// debugger. Otherwise exit out normally.
	if (_debugger && !_debugger->isAttached()) {
		// (Print it again in case debugger segfaults)
		printf("%s\n", buf2);
		_debugger->attach(buf2);
		_debugger->onFrame();
	}
}

void Sword2Engine::registerDefaultSettings() {
	ConfMan.registerDefault("music_mute", false);
	ConfMan.registerDefault("speech_mute", false);
	ConfMan.registerDefault("sfx_mute", false);
	ConfMan.registerDefault("gfx_details", 2);
	ConfMan.registerDefault("subtitles", false);
	ConfMan.registerDefault("reverse_stereo", false);
}

void Sword2Engine::readSettings() {
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	setSubtitles(ConfMan.getBool("subtitles"));
	_sound->muteMusic(ConfMan.getBool("music_mute"));
	_sound->muteSpeech(ConfMan.getBool("speech_mute"));
	_sound->muteFx(ConfMan.getBool("sfx_mute"));
	_sound->setReverseStereo(ConfMan.getBool("reverse_stereo"));
	_mouse->setObjectLabels(ConfMan.getBool("object_labels"));
	_screen->setRenderLevel(ConfMan.getInt("gfx_details"));
}

void Sword2Engine::writeSettings() {
	ConfMan.set("music_volume", _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType));
	ConfMan.set("speech_volume", _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType));
	ConfMan.set("sfx_volume", _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType));
	ConfMan.set("music_mute", _sound->isMusicMute());
	ConfMan.set("speech_mute", _sound->isSpeechMute());
	ConfMan.set("sfx_mute", _sound->isFxMute());
	ConfMan.set("gfx_details", _screen->getRenderLevel());
	ConfMan.set("subtitles", getSubtitles());
	ConfMan.set("object_labels", _mouse->getObjectLabels());
	ConfMan.set("reverse_stereo", _sound->isReverseStereo());

	ConfMan.flushToDisk();
}

/**
 * The global script variables and player object should be kept open throughout
 * the game, so that they are never expelled by the resource manager.
 */

void Sword2Engine::setupPersistentResources() {
	_logic->_scriptVars = _resman->openResource(1) + ResHeader::size();
	_resman->openResource(CUR_PLAYER_ID);
}

int Sword2Engine::init(GameDetector &detector) {
	// Get some falling RAM and put it in your pocket, never let it slip
	// away

	_system->beginGFXTransaction();
		initCommonGFX(detector, true);
		_screen = new Screen(this, 640, 480);
	_system->endGFXTransaction();

	// Create the debugger as early as possible (but not before the
	// screen object!) so that errors can be displayed in it. In
	// particular, we want errors about missing files to be clearly
	// visible to the user.

	_debugger = new Debugger(this);

	_memory = new MemoryManager(this);
	_resman = new ResourceManager(this);
	_logic = new Logic(this);
	_fontRenderer = new FontRenderer(this);
	_sound = new Sound(this);
	_mouse = new Mouse(this);

	// Setup mixer
	if (!_mixer->isReady())
		warning("Sound initialization failed");

	registerDefaultSettings();
	readSettings();

	initStartMenu();

	// During normal gameplay, we care neither about mouse button releases
	// nor the scroll wheel.
	setInputEventFilter(RD_LEFTBUTTONUP | RD_RIGHTBUTTONUP | RD_WHEELUP | RD_WHEELDOWN);

	setupPersistentResources();
	initialiseFontResourceFlags();

	if (_features & GF_DEMO)
		_logic->writeVar(DEMO, 1);
	else
		_logic->writeVar(DEMO, 0);

	if (_saveSlot != -1) {
		if (saveExists(_saveSlot))
			restoreGame(_saveSlot);
		else {
			RestoreDialog dialog(this);
			if (!dialog.runModal())
				startGame();
		}
	} else if (!_bootParam && saveExists()) {
		int32 pars[2] = { 221, FX_LOOP };
		bool result;

		_mouse->setMouse(NORMAL_MOUSE_ID);
		_logic->fnPlayMusic(pars);

		StartDialog dialog(this);

		result = (dialog.runModal() != 0);

		// If the game is started from the beginning, the cutscene
		// player will kill the music for us. Otherwise, the restore
		// will either have killed the music, or done a crossfade.

		if (_quit)
			return 0;

		if (result)
			startGame();
	} else
		startGame();

	_screen->initialiseRenderCycle();

	return 0;
}

int Sword2Engine::go() {
	while (1) {
		if (_debugger->isAttached())
			_debugger->onFrame();

#ifdef SWORD2_DEBUG
		if (_stepOneCycle) {
			pauseGame();
			_stepOneCycle = false;
		}
#endif

		KeyboardEvent *ke = keyboardEvent();

		if (ke) {
			if ((ke->modifiers == OSystem::KBD_CTRL && ke->keycode == 'd') || ke->ascii == '#' || ke->ascii == '~') {
				_debugger->attach();
			} else if (ke->modifiers == 0 || ke->modifiers == OSystem::KBD_SHIFT) {
				switch (ke->keycode) {
				case 'p':
					if (_gamePaused)
						unpauseGame();
					else
						pauseGame();
					break;
				case 'c':
					if (!_logic->readVar(DEMO) && !_mouse->isChoosing()) {
						ScreenInfo *screenInfo = _screen->getScreenInfo();
						_logic->fnPlayCredits(NULL);
						screenInfo->new_palette = 99;
					}
					break;
#ifdef SWORD2_DEBUG
				case ' ':
					if (_gamePaused) {
						_stepOneCycle = true;
						unpauseGame();
					}
					break;
				case 's':
					_renderSkip = !_renderSkip;
					break;
#endif
				default:
					break;
				}
			}
		}

		// skip GameCycle if we're paused
		if (!_gamePaused) {
			_gameCycle++;
			gameCycle();
		}

		// We can't use this as termination condition for the loop,
		// because we want the break to happen before updating the
		// screen again.

		if (_quit)
			break;

		// creates the debug text blocks
		_debugger->buildDebugText();

#ifdef SWORD2_DEBUG
		// if not in console & '_renderSkip' is set, only render
		// display once every 4 game-cycles

		if (!_renderSkip || (_gameCycle % 4) == 0)
			_screen->buildDisplay();
#else
		_screen->buildDisplay();
#endif
	}

	return 0;
}

void Sword2Engine::closeGame() {
	_quit = true;
}

void Sword2Engine::restartGame() {
	ScreenInfo *screenInfo = _screen->getScreenInfo();
	uint32 temp_demo_flag;

	_mouse->closeMenuImmediately();

	// Restart the game. To do this, we must...

	// Stop music instantly!
	_sound->stopMusic(true);

	// In case we were dead - well we're not anymore!
	_logic->writeVar(DEAD, 0);

	// Restart the game. Clear all memory and reset the globals
	temp_demo_flag = _logic->readVar(DEMO);

	// Remove all resources from memory, including player object and
	// global variables
	_resman->removeAll();

	// Reopen global variables resource and player object
	setupPersistentResources();

	_logic->writeVar(DEMO, temp_demo_flag);

	// Free all the route memory blocks from previous game
	_logic->_router->freeAllRouteMem();

	// Call the same function that first started us up
	startGame();

	// Prime system with a game cycle

	// Reset the graphic 'BuildUnit' list before a new logic list
	// (see fnRegisterFrame)
	_screen->resetRenderLists();

	// Reset the mouse hot-spot list (see fnRegisterMouse and
	// fnRegisterFrame)
	_mouse->resetMouseList();

	_mouse->closeMenuImmediately();

	// FOR THE DEMO - FORCE THE SCROLLING TO BE RESET!
	// - this is taken from fnInitBackground
	// switch on scrolling (2 means first time on screen)
	screenInfo->scroll_flag = 2;

	if (_logic->processSession())
		error("restart 1st cycle failed??");

	// So palette not restored immediately after control panel - we want
	// to fade up instead!
	screenInfo->new_palette = 99;
}

bool Sword2Engine::checkForMouseEvents() {
	return _mouseEvent.pending;
}

MouseEvent *Sword2Engine::mouseEvent() {
	if (!_mouseEvent.pending)
		return NULL;

	_mouseEvent.pending = false;
	return &_mouseEvent;
}

KeyboardEvent *Sword2Engine::keyboardEvent() {
	if (!_keyboardEvent.pending)
		return NULL;

	_keyboardEvent.pending = false;
	return &_keyboardEvent;
}

uint32 Sword2Engine::setInputEventFilter(uint32 filter) {
	uint32 oldFilter = _inputEventFilter;

	_inputEventFilter = filter;
	return oldFilter;
}

/**
 * Clear the input events. This is so that we won't get any keyboard repeat
 * right after using the debugging console.
 */

void Sword2Engine::clearInputEvents() {
	_keyboardEvent.pending = false;
	_keyboardEvent.repeat = 0;
	_mouseEvent.pending = false;
}

/**
 * OSystem Event Handler. Full of cross platform goodness and 99% fat free!
 */

void Sword2Engine::parseInputEvents() {
	OSystem::Event event;

	uint32 now = _system->getMillis();

	while (_system->pollEvent(event)) {
		switch (event.type) {
		case OSystem::EVENT_KEYDOWN:
			if (!(_inputEventFilter & RD_KEYDOWN)) {
				_keyboardEvent.pending = true;
				_keyboardEvent.repeat = now + 400;
				_keyboardEvent.ascii = event.kbd.ascii;
				_keyboardEvent.keycode = event.kbd.keycode;
				_keyboardEvent.modifiers = event.kbd.flags;
			}
			break;
		case OSystem::EVENT_KEYUP:
			_keyboardEvent.repeat = 0;
			break;
		case OSystem::EVENT_MOUSEMOVE:
			if (!(_inputEventFilter & RD_KEYDOWN)) {
				_mouse->setPos(event.mouse.x, event.mouse.y - MENUDEEP);
			}
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			if (!(_inputEventFilter & RD_LEFTBUTTONDOWN)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_LEFTBUTTONDOWN;
			}
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			if (!(_inputEventFilter & RD_RIGHTBUTTONDOWN)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_RIGHTBUTTONDOWN;
			}
			break;
		case OSystem::EVENT_LBUTTONUP:
			if (!(_inputEventFilter & RD_LEFTBUTTONUP)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_LEFTBUTTONUP;
			}
			break;
		case OSystem::EVENT_RBUTTONUP:
			if (!(_inputEventFilter & RD_RIGHTBUTTONUP)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_RIGHTBUTTONUP;
			}
			break;
		case OSystem::EVENT_WHEELUP:
			if (!(_inputEventFilter & RD_WHEELUP)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_WHEELUP;
			}
			break;
		case OSystem::EVENT_WHEELDOWN:
			if (!(_inputEventFilter & RD_WHEELDOWN)) {
				_mouseEvent.pending = true;
				_mouseEvent.buttons = RD_WHEELDOWN;
			}
			break;
		case OSystem::EVENT_QUIT:
			closeGame();
			break;
		default:
			break;
		}
	}

	// Handle keyboard auto-repeat
	if (!_keyboardEvent.pending && _keyboardEvent.repeat && now >= _keyboardEvent.repeat) {
		_keyboardEvent.pending = true;
		_keyboardEvent.repeat = now + 100;
	}
}

void Sword2Engine::gameCycle() {
	// Do one game cycle, that is run the logic session until a full loop
	// has been performed.

	if (_logic->getRunList()) {
		do {
			// Reset the 'BuildUnit' and mouse hot-spot lists
			// before each new logic list. The service scripts
			// will fill thrm through fnRegisterFrame() and
			// fnRegisterMouse().

			_screen->resetRenderLists();
			_mouse->resetMouseList();

			// Keep going as long as new lists keep getting put in
			// - i.e. screen changes.
		} while (_logic->processSession());
	} else {
		// Start the console and print the start options perhaps?
		_debugger->attach("AWAITING START COMMAND: (Enter 's 1' then 'q' to start from beginning)");
	}

	// If this screen is wide, recompute the scroll offsets every cycle
	ScreenInfo *screenInfo = _screen->getScreenInfo();

	if (screenInfo->scroll_flag)
		_screen->setScrolling();

	_mouse->mouseEngine();
	_sound->processFxQueue();
}

void Sword2Engine::startGame() {
	// Boot the game straight into a start script. It's always George's
	// script #1, but with different ScreenManager objects depending on
	// if it's the demo or the full game, or if we're using a boot param.

	int screen_manager_id = 0;

	debug(5, "startGame() STARTING:");

	if (!_bootParam) {
		if (_logic->readVar(DEMO))
			screen_manager_id = 19;		// DOCKS SECTION START
		else
			screen_manager_id = 949;	// INTRO & PARIS START
	} else {
		// FIXME this could be validated against startup.inf for valid
		// numbers to stop people shooting themselves in the foot

		if (_bootParam != 0)
			screen_manager_id = _bootParam;
	}

	_logic->runResObjScript(screen_manager_id, CUR_PLAYER_ID, 1);
}

// FIXME: Move this to some better place?

void Sword2Engine::sleepUntil(uint32 time) {
	while (getMillis() < time) {
		// Make sure menu animations and fades don't suffer, but don't
		// redraw the entire scene.
		_mouse->processMenu();
		_screen->updateDisplay(false);
		_system->delayMillis(10);
	}
}

void Sword2Engine::pauseGame() {
	// Don't allow Pause while screen fading or while black
	if (_screen->getFadeStatus() != RDFADE_NONE)
		return;

	_sound->pauseAllSound();
	_mouse->pauseGame();

	// If render level is at max, turn it down because palette-matching
	// won't work when the palette is dimmed.

	if (_screen->getRenderLevel() == 3) {
		_screen->setRenderLevel(2);
		_graphicsLevelFudged = true;
	}

#ifdef SWORD2_DEBUG
	// Don't dim it if we're single-stepping through frames
	// dim the palette during the pause

	if (!_stepOneCycle)
		_screen->dimPalette();
#else
	_screen->dimPalette();
#endif

	_gamePaused = true;
}

void Sword2Engine::unpauseGame() {
	_mouse->unpauseGame();
	_sound->unpauseAllSound();

	// Put back game screen palette; see screen.cpp
	_screen->setFullPalette(-1);

	// If graphics level at max, turn up again
	if (_graphicsLevelFudged) {
		_screen->setRenderLevel(3);
		_graphicsLevelFudged = false;
	}

	_gamePaused = false;

	// If mouse is about or we're in a chooser menu
	if (!_mouse->getMouseStatus() || _mouse->isChoosing())
		_mouse->setMouse(NORMAL_MOUSE_ID);
}

uint32 Sword2Engine::getMillis() {
	return _system->getMillis();
}

} // End of namespace Sword2
