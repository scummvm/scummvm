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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/file.h"
#include "common/EventRecorder.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

namespace Asylum {

AsylumEngine::AsylumEngine(OSystem *system, Common::Language language)
		: Engine(system) {

	Common::addDebugChannel(kDebugLevelMain, "Main", "Generic debug level");
	Common::addDebugChannel(kDebugLevelResources, "Resources", "Resources debugging");
	Common::addDebugChannel(kDebugLevelSprites, "Sprites", "Sprites debugging");
	Common::addDebugChannel(kDebugLevelInput, "Input", "Input events debugging");
	Common::addDebugChannel(kDebugLevelMenu, "Menu", "Menu debugging");
	Common::addDebugChannel(kDebugLevelScripts, "Scripts", "Scripts debugging");
	Common::addDebugChannel(kDebugLevelSound, "Sound", "Sound debugging");
	Common::addDebugChannel(kDebugLevelSavegame, "Savegame", "Saving & restoring game debugging");
	Common::addDebugChannel(kDebugLevelScene, "Scene", "Scene process and draw debugging");

	SearchMan.addSubDirectoryMatching(_gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(_gameDataDir, "vids");
	SearchMan.addSubDirectoryMatching(_gameDataDir, "music");

	g_eventRec.registerRandomSource(_rnd, "asylum");

	memset(_gameFlags, 0, 1512);

	// initialize default configuration
	_ambientVolume = 0;
	_soundVolume   = 0;
}

AsylumEngine::~AsylumEngine() {
	Common::clearAllDebugChannels();

	delete _console;
	delete _scene;
	delete _mainMenu;
	delete _video;
	delete _sound;
	delete _screen;
	delete _encounter;
	delete _text;
}

Common::Error AsylumEngine::run() {
	Common::Error err;
	err = init();
	if (err != Common::kNoError)
		return err;

	return go();
}

// Will do the same as subroutine at address 0041A500
Common::Error AsylumEngine::init() {
	initGraphics(640, 480, true);

	_screen   = new Screen(this);
	_sound    = new Sound(_mixer);
	_video    = new Video(_mixer);
	_console  = new Console(this);
	_text     = new Text(_screen);
	_mainMenu = 0;
	_scene    = 0;

	_introPlaying = false;

	return Common::kNoError;
}

Common::Error AsylumEngine::go() {
	// TODO: save dialogue key codes into sntrm_k.txt (need to figure out why they use such thing)
	// TODO: load startup configurations (address 0041A970)
	// TODO: init unknown game stuffs (address 0040F430)
	// TODO: if savegame exists on folder, than start NewGame()

	// FIXME The scene shouldn't be created here, as we don't technically know
	// how the scene is starting. This was put in for testing, but will eventually
	// have to be removed once saveload is implemented
	_scene = new Scene(5, this);

	// FIXME This is just here for testing purposes. It is also defined
	// in the processActionList() method when the necessary action is fired.
	// Once the blowup puzzle testing is removed from checkForEvent(), this
	// can be removed as well.
	_scene->setBlowUpPuzzle(new BlowUpPuzzleVCR(_scene));

	// FIXME This can probably also be rolled into the scene constructor.
	// Investigate if this will fuck up the execution sequence though :P
	_scene->actions()->setScriptByIndex(_scene->worldstats()->actionListIdx);

	// Set up main menu
	_mainMenu = new MainMenu(this);

	// XXX Testing
	_encounter = new Encounter(_scene);

	// TODO you should be able to skip this if you want. The original
	// allows this through the /SKIP command line argument.
	// Also, this routine is used to set game flags 4 and 12, so if we're
	// skipping the intro, but not loading a save file, those flags
	// need to be set somewhere else.
	//playIntro();

	// Enter first scene
	setGameFlag(4);
	setGameFlag(12);
	_scene->enterScene();

	while (!shouldQuit()) {
		checkForEvent(true);
		waitForTimer(55);
	}

	return Common::kNoError;
}

void AsylumEngine::waitForTimer(int msec_delay) {
	uint32 start_time = _system->getMillis();

	while (_system->getMillis() < start_time + msec_delay) {
		checkForEvent(false);
		processDelayedEvents();
		_system->updateScreen();
	}
}

void AsylumEngine::playIntro() {
	_video->playVideo(1, kSubtitlesOn);

	if (_scene->worldstats()->musicCurrentResId != 0xFFFFFD66)
		_sound->playMusic(_scene->worldstats()->musicCurrentResId);

	_screen->clearScreen();

	setGameFlag(4);
	setGameFlag(12);

	// Play the intro sound sample (the screen is blacked out, you hear
	// an alarm sounding and men talking about.
	_sound->playSound(0x8012007, false, _soundVolume, 0);
}

void AsylumEngine::checkForEvent(bool doUpdate) { // k_sub_40AE30 (0040AE30)

	// NOTE
	// In the original version of Sanitarium, the control loop for the sound
	// effect that played after the intro video involved a while loop that
	// executed until the sound handle was released.
	// This caused the application to be locked until the while loop's execution
	// completed successfully. Our implementation circumvents this issue
	// by moving the logic to the event loop and checking whether a flag is
	// set to determine if control should be returned to the engine.
	if (_introPlaying) {
		if (!_sound->isPlaying(0x8012007)) {
			_introPlaying = false;

			// TODO Since we've currently only got one sfx handle to play with in
			// the sound class, entering the scene overwrites the "alarm" loop.
			// This sound is technically supposed to play until the actor disables
			// the alarm by flipping the switch. The sound class needs to be extended
			// to be able to handle multiple handles.
			// The currently active sound resources can probably also be buffered into
			// the scene's soundResId[] array (seems that's the way the original worked,
			// especially when you examine isSoundinList() or isSoundPlaying())

			_scene->enterScene();
		} else {
			return;
		}
	}

	Common::Event ev;

	if (_system->getEventManager()->pollEvent(ev)) {
		if (ev.type == Common::EVENT_KEYDOWN) {
			if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
				// Toggle menu
				if (_mainMenu->isActive()) {
					_mainMenu->closeMenu();
					_scene->enterScene();
				} else if (_scene->isActive()) {
					_mainMenu->openMenu();
				} else if (_scene->getBlowUpPuzzle()->isActive()) {
					_scene->getBlowUpPuzzle()->closeBlowUp();
					_scene->enterScene();
				}

				return;
			}

			// XXX: TEST ONLY
			if (ev.kbd.keycode == Common::KEYCODE_b) {
				//_mainMenu->closeMenu();
				_scene->getBlowUpPuzzle()->openBlowUp();
			}

			if (ev.kbd.flags == Common::KBD_CTRL) {
				if (ev.kbd.keycode == Common::KEYCODE_d)
					_console->attach();
			}

		}
	}

	if (doUpdate) {
		if (_mainMenu->isActive() || _scene->isActive() || _scene->getBlowUpPuzzle()->isActive())
			// Copy background image
			_screen->copyBackBufferToScreen();

		if (_console->isAttached())
			_console->onFrame();
	}

	if (_mainMenu->isActive())
		// Main menu active, pass events to it
		_mainMenu->handleEvent(&ev, doUpdate);
	else if (_scene->isActive())
		// Pass events to the game
		_scene->handleEvent(&ev, doUpdate);
	else if (_scene->getBlowUpPuzzle()->isActive())
		// Pass events to BlowUp Puzzles
		_scene->getBlowUpPuzzle()->handleEvent(&ev, doUpdate);

	if (_introPlaying) {

	}
}

void AsylumEngine::processDelayedEvents() {
	// check for a delayed video
	int videoIdx = _scene->actions()->delayedVideoIndex;
	if (videoIdx >= 0) {
		_sound->stopMusic();
		_sound->stopAllSounds();
		_video->playVideo(videoIdx, kSubtitlesOn);
		_scene->actions()->delayedVideoIndex = -1;

		if (_mainMenu->isActive())
			_mainMenu->openMenu();
		else if (_scene->isActive())
			_scene->enterScene();
	}

	// check for a delayed scene change
	int sceneIdx = _scene->actions()->delayedSceneIndex;
	if (sceneIdx >= 0 && !_scene->actions()->processing) {
		_sound->stopMusic();
		_sound->stopAllSounds();

		if (_scene)
			delete _scene;

		_scene = new Scene(sceneIdx, this);
		_scene->enterScene();

		_scene->actions()->delayedSceneIndex = -1;
		_scene->actions()->setScriptByIndex(_scene->worldstats()->actionListIdx);
	}
}

void AsylumEngine::setGameFlag(int flag) {
	_gameFlags[flag / 32] |= 1 << flag % -32;
}

void AsylumEngine::clearGameFlag(int flag) {
	_gameFlags[flag / 32] &= ~(1 << flag % -32);
}

void AsylumEngine::toggleGameFlag(int flag) {
	_gameFlags[flag / 32] ^= 1 << flag % -32;
}

bool AsylumEngine::isGameFlagSet(int flag) {
	return ((1 << flag % -32) & (unsigned int)_gameFlags[flag / 32]) >> flag % -32 != 0;
}

bool AsylumEngine::isGameFlagNotSet(int flag) {
	return ((1 << flag % -32) & (unsigned int)_gameFlags[flag / 32]) >> flag % -32 == 0;
}


} // namespace Asylum
