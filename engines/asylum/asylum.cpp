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

#include "asylum/asylum.h"

#include "asylum/resources/actionlist.h"
#include "asylum/resources/encounters.h"

#include "asylum/system/config.h"
#include "asylum/system/screen.h"
#include "asylum/system/sound.h"
#include "asylum/system/text.h"
#include "asylum/system/video.h"

#include "asylum/views/scene.h"
#include "asylum/views/menu.h"

#include "asylum/console.h"
#include "asylum/respack.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/EventRecorder.h"

#include "engines/util.h"

namespace Asylum {

AsylumEngine::AsylumEngine(OSystem *system, const ADGameDescription *gd) : Engine(system), _gameDescription(gd),
	_console(NULL), _encounter(NULL), _resource(NULL), _mainMenu(NULL), _scene(NULL), _screen(NULL),
	_sound(NULL), _text(NULL), _video(NULL) {

	// Add default search directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(gameDataDir, "vids");
	SearchMan.addSubDirectoryMatching(gameDataDir, "music");

	// Initialize custom debug levels
	DebugMan.addDebugChannel(kDebugLevelMain,      "Main",      "Generic debug level");
	DebugMan.addDebugChannel(kDebugLevelResources, "Resources", "Resources debugging");
	DebugMan.addDebugChannel(kDebugLevelSprites,   "Sprites",   "Sprites debugging");
	DebugMan.addDebugChannel(kDebugLevelInput,     "Input",     "Input events debugging");
	DebugMan.addDebugChannel(kDebugLevelMenu,      "Menu",      "Menu debugging");
	DebugMan.addDebugChannel(kDebugLevelScripts,   "Scripts",   "Scripts debugging");
	DebugMan.addDebugChannel(kDebugLevelSound,     "Sound",     "Sound debugging");
	DebugMan.addDebugChannel(kDebugLevelSavegame,  "Savegame",  "Saving & restoring game debugging");
	DebugMan.addDebugChannel(kDebugLevelScene,     "Scene",     "Scene process and draw debugging");
	DebugMan.addDebugChannel(kDebugLevelObjects,   "Objects",   "Debug Object Objects");

	// Initialize random number source
	g_eventRec.registerRandomSource(_rnd, "asylum");
}

AsylumEngine::~AsylumEngine() {
	delete _console;
	delete _encounter;
	delete _resource;
	delete _mainMenu;
	delete _scene;
	delete _screen;
	delete _sound;
	delete _text;
	delete _video;

	// Zero passed pointers
	_gameDescription = NULL;
}

Common::Error AsylumEngine::run() {
	// Initialize the graphics
	initGraphics(640, 480, true);

	// Create debugger. It requires GFX to be initialized
	_console   = new Console(this);

	// Create all manager classes
	_resource  = new ResourceManager();
	_screen    = new Screen(this);
	_sound     = new Sound(this, _mixer);
	_video     = new Video(this, _mixer);
	_text      = new Text(this);
	_scene     = NULL;
	_encounter = NULL;

	_introPlaying = false;

	memset(_gameFlags, 0, 1512);

	// Start the game
    g_system->showMouse(true);

	// TODO: save dialogue key codes into sntrm_k.txt (need to figure out why they use such thing) (address 00411CD0)
    // load startup configurations (.text:0041A970)
    Config.read();
	// TODO: init unknown game stuffs (.text:0040F430)

	if (Config.showIntro)
		_video->playVideo(0, Config.showMovieSubtitles);

	// Set up main menu
	_mainMenu = new MainMenu(this);

	// TODO: if savegame not exists on folder, than start game()
	//if(0) { //SearchMan.hasArchive
		startGame();
	//} else {
	//    _mainMenu->openMenu();
	//}

	while (!shouldQuit()) {
		handleEvents(true);
		waitForTimer(55);
	}

	return Common::kNoError;
}

void AsylumEngine::waitForTimer(int msec_delay) {
	uint32 start_time = _system->getMillis();

	while (_system->getMillis() < start_time + msec_delay) {
		handleEvents(false);
		if (_scene) {
			processDelayedEvents();
		}
		_system->updateScreen();
	}
}

void AsylumEngine::startGame() {
	// TODO: reset what need to be reset for a new game
	if (_scene)
		delete _scene;

	_scene = new Scene(kResourcePackTowerCells, this);

	if (Config.showIntro)
		playIntro();

	_scene->initialize();

	// FIXME This is just here for testing purposes. It is also defined
	// in the processActionList() method when the necessary action is fired.
	// Once the blowup puzzle testing is removed from checkForEvent(), this
	// can be removed as well.
	//_scene->setBlowUpPuzzle(new BlowUpPuzzleVCR(_scene));

	// XXX Testing
	_encounter = new Encounter(_scene);

	// Enter first scene
	if(!_introPlaying)
	{
		setGameFlag(kGameFlag4);
		setGameFlag(kGameFlag12);
		_scene->enterScene();
	}
}

void AsylumEngine::playIntro() {
	_introPlaying = true;
	g_system->showMouse(false);

	_video->playVideo(1, Config.showMovieSubtitles);

	/*if (_scene->worldstats()->musicCurrentResourceId != kResourceMusic_FFFFFD66)
		_sound->playMusic(_scene->getResourcePack(), _scene->worldstats()->musicCurrentResourceId);*/

	_screen->clearScreen();

	setGameFlag(kGameFlag4);
	setGameFlag(kGameFlag12);

	// Play the intro sound sample (the screen is blacked out, you hear
	// an alarm sounding and men talking about.
	_sound->playSound(MAKE_RESOURCE(kResourcePackSound, 7));
}

void AsylumEngine::handleEvents(bool doUpdate) { // k_sub_40AE30 (0040AE30)
	// Make sure the debugger is present
	if (!_console)
		error("AsylumEngine::handleEvents: called before the required subsystems have been initialized!");

	// Show the debugger if required
	_console->onFrame();

	// NOTE
	// In the original version of Sanitarium, the control loop for the sound
	// effect that played after the intro video involved a while loop that
	// executed until the sound handle was released.
	// This caused the application to be locked until the while loop's execution
	// completed successfully. Our implementation circumvents this issue
	// by moving the logic to the event loop and checking whether a flag is
	// set to determine if control should be returned to the engine.
	if (_introPlaying) {
		if (!_sound->isPlaying(MAKE_RESOURCE(kResourcePackSound, 7))) {
			_introPlaying = false;

			// TODO Since we've currently only got one sfx handle to play with in
			// the sound class, entering the scene overwrites the "alarm" loop.
			// This sound is technically supposed to play until the actor disables
			// the alarm by flipping the switch. The sound class needs to be extended
			// to be able to handle multiple handles.
			// The currently active sound resources can probably also be buffered into
			// the scene's soundResourceId[] array (seems that's the way the original worked,
			// especially when you examine isSoundinList() or isSoundPlaying())

			_scene->enterScene();
		}
	}

	Common::Event ev;

	if (_eventMan->pollEvent(ev)) {
		switch (ev.type) {
		default:
			break;

		case Common::EVENT_KEYDOWN:
			if ((ev.kbd.flags & Common::KBD_CTRL) && ev.kbd.keycode == Common::KEYCODE_d)
				_console->attach();

			if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
				// Toggle menu
				if (_mainMenu->isActive()) {
					if (_scene) {
						_mainMenu->closeMenu();
						_scene->enterScene();
					}
				} else if (_scene && _scene->isActive()) {
					_mainMenu->openMenu();
				}
				/* FIXME
				} else if (_scene && _scene->getBlowUpPuzzle()->isActive()) {
					_scene->getBlowUpPuzzle()->closeBlowUp();
					_scene->enterScene();
				}
				*/
				return;
			}

			// XXX: TEST ONLY
			/*
			if (ev.kbd.keycode == Common::KEYCODE_b) {
				if (_scene) {
					_scene->getBlowUpPuzzle()->openBlowUp();
				}
			}
			*/
			break;
		}
	}

	if (doUpdate) {
		if (_mainMenu->isActive() ||
			(_scene && _scene->isActive())) //|| (_scene && _scene->getBlowUpPuzzle()->isActive()))
			// Copy background image
			_screen->copyBackBufferToScreen();
	}


	if (_mainMenu->isActive())
		// Main menu active, pass events to it
		_mainMenu->handleEvent(&ev, doUpdate);
	else if (_scene && _scene->isActive())
		// Pass events to the game
		_scene->handleEvent(&ev, doUpdate);
	/* FIXME reimplement
	else if (_scene && _scene->getBlowUpPuzzle()->isActive())
		// Pass events to BlowUp Puzzles
		_scene->getBlowUpPuzzle()->handleEvent(&ev, doUpdate);
	*/
}

void AsylumEngine::processDelayedEvents() {
	// check for a delayed video
	int videoIdx = _scene->actions()->getDelayedVideoIndex();
	if (videoIdx >= 0) {
		_sound->stopMusic();
		_sound->stopAllSounds();
		_video->playVideo(videoIdx, true);
		_scene->actions()->setDelayedVideoIndex(-1);

		if (_mainMenu->isActive())
			_mainMenu->openMenu();
		else if (_scene->isActive())
			_scene->enterScene();
	}

	// check for a delayed scene change
	ResourcePackId packId = _scene->actions()->getDelayedSceneIndex();
	// XXX Flag 183 indicates whether the actionlist is currently
	// processing
	if (packId >= 0 && isGameFlagNotSet(kGameFlagScriptProcessing)) {

		// Reset delayed scene
		_scene->actions()->setDelayedSceneIndex(kResourcePackInvalid);

		_sound->stopMusic();
		_sound->stopAllSounds();

		if (_scene)
			delete _scene;

		_scene = new Scene(packId, this);
		_scene->initialize();
		_scene->enterScene();
	}
}

//////////////////////////////////////////////////////////////////////////
// Message handlers
//////////////////////////////////////////////////////////////////////////
void AsylumEngine::switchMessageHandler(MessageHandler *handler) {
	error("[AsylumEngine::switchMessageHandler] not implemented");
}

AsylumEngine::MessageHandler *AsylumEngine::getMessageHandler(uint32 index) {
	error("[AsylumEngine::getMessageHandler] not implemented");
}

//////////////////////////////////////////////////////////////////////////
// Game flags
//////////////////////////////////////////////////////////////////////////
void AsylumEngine::setGameFlag(GameFlag flag) {
	_gameFlags[flag / 32] |= 1 << flag % -32;
}

void AsylumEngine::clearGameFlag(GameFlag flag) {
	_gameFlags[flag / 32] &= ~(1 << flag % -32);
}

void AsylumEngine::toggleGameFlag(GameFlag flag) {
	_gameFlags[flag / 32] ^= 1 << flag % -32;
}

bool AsylumEngine::isGameFlagSet(GameFlag flag) {
	return ((1 << flag % -32) & (unsigned int)_gameFlags[flag / 32]) >> flag % -32 != 0;
}

bool AsylumEngine::isGameFlagNotSet(GameFlag flag) {
	return ((1 << flag % -32) & (unsigned int)_gameFlags[flag / 32]) >> flag % -32 == 0;
}

void AsylumEngine::setFlag(FlagType flag, bool isSet) {
	if (flag > ARRAYSIZE(_flags))
		error("[AsylumEngine::setFlag] Invalid flag type (was: %d, max: %d", flag, ARRAYSIZE(_flags));

	_flags[flag] = isSet;
}

} // namespace Asylum
