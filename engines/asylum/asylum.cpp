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
#include "asylum/shared.h"

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
	
	Common::File::addDefaultDirectory(_gameDataDir.getChild("Data"));
	Common::File::addDefaultDirectory(_gameDataDir.getChild("Vids"));
	Common::File::addDefaultDirectory(_gameDataDir.getChild("Music"));

	g_eventRec.registerRandomSource(_rnd, "asylum");
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

	_screen		= new Screen(_system);
	_sound		= new Sound(_mixer);
	_video		= new Video(_mixer);
	_console	= new Console(this);
	_mainMenu	= 0;
	_scene		= 0;

	Shared.setScreen(_screen);
	Shared.setSound(_sound);
	Shared.setVideo(_video);

	_introPlaying = false;

	return Common::kNoError;
}

Common::Error AsylumEngine::go() {
	// TODO: save dialogue key codes into sntrm_k.txt (need to figure out why they use such thing)
	// TODO: load startup configurations (address 0041A970)
	// TODO: init unknown game stuffs (address 0040F430)
	// TODO: if savegame exists on folder, than start NewGame()

	// Set up the game's main scene
	_scene = new Scene(5);
	Shared.setScene(_scene);

	// XXX This is just here for testing purposes. It is also defined
	// in the processActionList() method when the necessary action is fired.
	// Once the blowup puzzle testing is removed from checkForEvent(), this
	// can be removed as well.
    _scene->setBlowUpPuzzle(new BlowUpPuzzleVCR()); // this will be done by a Script command

	// XXX This can probably also be rolled into the scene constructor.
	// Investigate if this will fuck up the execution sequence though :P
	ScriptMan.setScript(_scene->getDefaultActionList());

	// Set up main menu
	_mainMenu = new MainMenu();

	// XXX Testing
	_encounter = new Encounter(_scene);

	// TODO you should be able to skip this if you want. The original
	// allows this through the /SKIP command line argument.
	// Also, this routine is used to set game flags 4 and 12, so if we're
	// skipping the intro, but not loading a save file, those flags
	// need to be set somewhere else.
	//playIntro();

	// Enter first scene
	ScriptMan.setGameFlag(4);
	ScriptMan.setGameFlag(12);
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
		ScriptMan.processActionList();
	}
}

void AsylumEngine::playIntro() {
	_video->playVideo(1, kSubtitlesOn);
	if (_scene->getResources()->getWorldStats()->musicCurrentResId != 0xFFFFFD66)
		_sound->playMusic(_scene->getMusicPack(),
						  _scene->getResources()->getWorldStats()->musicCurrentResId);

	_screen->clearScreen();

	ScriptMan.setGameFlag(4);
	ScriptMan.setGameFlag(12);

	ResourcePack *introRes = new ResourcePack(18);

	_sound->playSfx(introRes, 7);

	_introPlaying = true;

	delete introRes;
}

void AsylumEngine::checkForEvent(bool doUpdate) {

	// NOTE
	// In the original version of Sanitarium, the control loop for the sound
	// effect that played after the intro video involved a while loop that
	// executed until the sound handle was released.
	// This caused the application to be locked until the while loop's execution
	// completed successfully. Our implementation circumvents this issue
	// by moving the logic to the event loop and checking whether a flag is
	// set to determine if control should be returned to the engine.
	if (_introPlaying) {
		if (!_sound->isSfxActive()) {
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
	int videoIdx = ScriptMan.getDelayedVideoIndex();
	if (videoIdx >= 0) {
		_sound->stopMusic();
		_sound->stopSfx();
		_video->playVideo(videoIdx, kSubtitlesOn);
		ScriptMan.setDelayedVideoIndex(-1);

		if (_mainMenu->isActive())
			_mainMenu->openMenu();
		else if (_scene->isActive())
			_scene->enterScene();
	}

	// check for a delayed scene change
	int sceneIdx = ScriptMan.getDelayedSceneIndex();
	if (sceneIdx >=0 && !ScriptMan.isProcessing()) {
		_sound->stopMusic();
		_sound->stopSfx();
		
		if (_scene)
			delete _scene;

		_scene = new Scene(sceneIdx);
		Shared.setScene(_scene);
		_scene->enterScene();

		ScriptMan.setDelayedSceneIndex(-1);
		ScriptMan.setScript(_scene->getDefaultActionList());
	}
}

} // namespace Asylum
