/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "sword1.h"

#include "backends/fs/fs.h"

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/timer.h"

#include "sword1/resman.h"
#include "sword1/objectman.h"
#include "sword1/mouse.h"
#include "sword1/logic.h"
#include "sword1/sound.h"
#include "sword1/screen.h"
#include "sword1/swordres.h"
#include "sword1/menu.h"
#include "sword1/music.h"
#include "sword1/control.h"

#include "gui/message.h"
#include "gui/newgui.h"

using namespace Sword1;

/* Broken Sword 1 */
static const GameSettings sword1FullSettings =
	{"sword1", "Broken Sword I", GF_DEFAULT_TO_1X_SCALER};
static const GameSettings sword1DemoSettings = 
	{"sword1demo", "Broken Sword I (Demo)", GF_DEFAULT_TO_1X_SCALER | Sword1::GF_DEMO };

// check these subdirectories (if present)
static const char *g_dirNames[] = {	"clusters",	"speech" };

#define NUM_FILES_TO_CHECK 5
static const char *g_filesToCheck[NUM_FILES_TO_CHECK] = { // these files have to be found
	"swordres.rif",
	"general.clu",
	"compacts.clu",
	"scripts.clu",
	"cows.mad",	// this one should only exist in the demo version
	// the engine needs several more files to work, but checking these should be sufficient
};

GameList Engine_SWORD1_gameList() {
	GameList games;
	games.push_back(sword1FullSettings);
	games.push_back(sword1DemoSettings);
	return games;
}

void Sword1CheckDirectory(const FSList &fslist, bool *filesFound) {
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			const char *fileName = file->displayName().c_str();
			for (int cnt = 0; cnt < NUM_FILES_TO_CHECK; cnt++)
				if (scumm_stricmp(fileName, g_filesToCheck[cnt]) == 0)
					filesFound[cnt] = true;
		} else {
			for (int cnt = 0; cnt < ARRAYSIZE(g_dirNames); cnt++)
				if (scumm_stricmp(file->displayName().c_str(), g_dirNames[cnt]) == 0)
					Sword1CheckDirectory(file->listDir(AbstractFilesystemNode::kListFilesOnly), filesFound);
		}
	}
}

DetectedGameList Engine_SWORD1_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	bool filesFound[NUM_FILES_TO_CHECK];
	for (int i = 0; i < NUM_FILES_TO_CHECK; i++)
		filesFound[i] = false;

	Sword1CheckDirectory(fslist, filesFound);
	bool mainFilesFound = true;
	for (int i = 0; i < NUM_FILES_TO_CHECK -1; i++)
		if (!filesFound[i])
			mainFilesFound = false;

	if (mainFilesFound && filesFound[NUM_FILES_TO_CHECK - 1])
		detectedGames.push_back(sword1DemoSettings);
	else if (mainFilesFound)
		detectedGames.push_back(sword1FullSettings);

	return detectedGames;
}

Engine *Engine_SWORD1_create(GameDetector *detector, OSystem *syst) {
	return new SwordEngine(detector, syst);
}

REGISTER_PLUGIN("Broken Sword", Engine_SWORD1_gameList, Engine_SWORD1_create, Engine_SWORD1_detectGames)

namespace Sword1 {

SystemVars SwordEngine::_systemVars;

void SwordEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

SwordEngine::SwordEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	_features = detector->_game.features;

	if (!_mixer->isReady())
		warning("Sound initialization failed");

	// Add default file directories
	File::addDefaultDirectory(_gameDataPath + "CLUSTERS/");
	File::addDefaultDirectory(_gameDataPath + "MUSIC/");
	File::addDefaultDirectory(_gameDataPath + "SPEECH/");
	File::addDefaultDirectory(_gameDataPath + "VIDEO/");
	File::addDefaultDirectory(_gameDataPath + "clusters/");
	File::addDefaultDirectory(_gameDataPath + "music/");
	File::addDefaultDirectory(_gameDataPath + "speech/");
	File::addDefaultDirectory(_gameDataPath + "video/");
}

SwordEngine::~SwordEngine() {
	delete _control;
	delete _logic;
	delete _menu;
	delete _sound;
	delete _music;
	delete _screen;
	delete _mouse;
	delete _objectMan;
	delete _resMan;
}

int SwordEngine::init(GameDetector &detector) {

	_system->beginGFXTransaction();
		initCommonGFX(detector);
		_system->initSize(640, 480);
	_system->endGFXTransaction();

	debug(5, "Starting resource manager");
	_resMan = new ResMan("swordres.rif");
	debug(5, "Starting object manager");
	_objectMan = new ObjectMan(_resMan);
	_mixer->setVolume(255);
	_mixer->setMusicVolume(256);
	_mouse = new Mouse(_system, _resMan, _objectMan);
	_screen = new Screen(_system, _resMan, _objectMan);
	_music = new Music(_system, _mixer);
	_sound = new Sound("", _mixer, _resMan);
	_menu = new Menu(_screen, _mouse);
	_logic = new Logic(_objectMan, _resMan, _screen, _mouse, _sound, _music, _menu, _system, _mixer);
	_mouse->useLogicAndMenu(_logic, _menu);

	ConfMan.registerDefault("speech_volume", 192);

	uint8 musicVol = (uint8)ConfMan.getInt("music_volume");
	uint8 speechVol = (uint8)ConfMan.getInt("speech_volume");
	uint8 sfxVol = (uint8)ConfMan.getInt("sfx_volume");

	_music->setVolume(musicVol, musicVol);      // these routines expect left and right volume,
	_sound->setSpeechVol(speechVol, speechVol); // but our config manager doesn't support it.
	_sound->setSfxVol(sfxVol, sfxVol);

	_systemVars.justRestoredGame = 0;
	_systemVars.currentCD = 0;
	_systemVars.controlPanelMode = CP_NEWGAME;
	_systemVars.forceRestart = false;
	_systemVars.wantFade = true;
	_systemVars.engineQuit = false;

	switch (Common::parseLanguage(ConfMan.get("language"))) {
	case Common::DE_DEU:
		_systemVars.language = BS1_GERMAN;
		break;
	case Common::FR_FRA:
		_systemVars.language = BS1_FRENCH;
		break;
	case Common::IT_ITA:
		_systemVars.language = BS1_ITALIAN;
		break;
	case Common::ES_ESP:
		_systemVars.language = BS1_SPANISH;
		break;
	case Common::PT_BRA:
		_systemVars.language = BS1_PORT;
		break;
	case Common::CZ_CZE:
		_systemVars.language = BS1_CZECH;
		break;
	default:
		_systemVars.language = BS1_ENGLISH;
	}

	_systemVars.showText = ConfMan.getBool("subtitles");
	
	_systemVars.playSpeech = 1;
	_mouseState = 0;

	_logic->initialize();
	_objectMan->initialize();
	_mouse->initialize();
	_control = new Control(_saveFileMan, _resMan, _objectMan, _system, _mouse, _sound, _music);
	
	return 0;
}

void SwordEngine::reinitialize(void) {
	_resMan->flush(); // free everything that's currently alloced and opened. (*evil*)

	_logic->initialize();     // now reinitialize these objects as they (may) have locked
	_objectMan->initialize(); // resources which have just been wiped.
	_mouse->initialize();
	_system->warpMouse(320, 240);
	_systemVars.wantFade = true;
}

void SwordEngine::checkCdFiles(void) { // check if we're running from cd, hdd or what...
	const char *speechFiles[] = {
#ifdef USE_MAD
		"SPEECH%d.CL3",
#endif
#ifdef USE_VORBIS
		"SPEECH%d.CLV",
#endif
		"SPEECH%d.CLU"
	};
	bool fileFound[2] = { false, false };
	File test;

	_systemVars.playSpeech = true;
	_systemVars.isDemo = false;

	for (int i = 1; i <= 2; i++) {
		for (int j = 0; j < ARRAYSIZE(speechFiles); j++) {
			char fileName[12];
			sprintf(fileName, speechFiles[j], i);
			if (test.open(fileName)) {
				test.close();
				fileFound[i - 1] = true;
				break;
			}
		}
	}

	if (fileFound[0] && fileFound[1]) {
		// both files exist, assume running from HDD and everything's fine.
		_systemVars.runningFromCd = false;
		_systemVars.playSpeech = true;
	} else { // speech1 & speech2 not present. are we running from cd?
		if (test.open("cows.mad")) {
			_systemVars.isDemo = true;
			Logic::_scriptVars[PLAYINGDEMO] = 1;
			test.close();
		}
		if (test.open("cd1.id")) {
			_systemVars.runningFromCd = true;
			_systemVars.currentCD = 1;
			test.close();
		} else if (test.open("cd2.id")) {
			_systemVars.runningFromCd = true;
			_systemVars.currentCD = 2;
			test.close();
		} else {
			const char msg[] = "Unable to find the game files.\nPlease read the ScummVM documentation";
			GUI::MessageDialog dialog(msg);
			dialog.runModal();
			error(msg);
		}
	}

	// check cutscene pack version
	_systemVars.cutscenePackVersion = 0;
#ifdef USE_MPEG2
	if (test.open("intro.snd")) {
		_systemVars.cutscenePackVersion = 1;
		test.close();
	}
#endif
}

int SwordEngine::go() {
	
	checkCdFiles();

	uint16 startPos = ConfMan.getInt("boot_param");
	if (startPos)
		_logic->startPositions(startPos);
	else {
		if (_control->savegamesExist()) {
			_systemVars.controlPanelMode = CP_NEWGAME;
			if (_control->runPanel() == CONTROL_GAME_RESTORED)
				_control->doRestore();
			else if (!_systemVars.engineQuit)
				_logic->startPositions(0);
		} else // no savegames, start new game.
			_logic->startPositions(0);
	}
	_systemVars.controlPanelMode = CP_NORMAL;

	while (!_systemVars.engineQuit) {
		uint8 action = mainLoop();

		if (!_systemVars.engineQuit) {
			// the mainloop was left, we have to reinitialize.
			reinitialize();
			if (action == CONTROL_GAME_RESTORED)
				_control->doRestore();
			else if (action == CONTROL_RESTART_GAME)
				_logic->startPositions(1);
			_systemVars.forceRestart = false;
			_systemVars.controlPanelMode = CP_NORMAL;
		}
	}
	
	return 0;
}

void SwordEngine::checkCd(void) {
	uint8 needCd = _cdList[Logic::_scriptVars[NEW_SCREEN]];
	if (_systemVars.runningFromCd) { // are we running from cd?
		if (needCd == 0) { // needCd == 0 means we can use either CD1 or CD2.
			if (_systemVars.currentCD == 0) {
				_systemVars.currentCD = 1; // if there is no CD currently inserted, ask for CD1.
				_control->askForCd();
			} // else: there is already a cd inserted and we don't care if it's cd1 or cd2.
		} else if (needCd != _systemVars.currentCD) { // we need a different CD than the one in drive.
			_music->startMusic(0, 0); // 
			_sound->closeCowSystem(); // close music and sound files before changing CDs
			_systemVars.currentCD = needCd; // askForCd will ask the player to insert _systemVars.currentCd,
			_control->askForCd();		    // so it has to be updated before calling it.
		}
	} else {		// we're running from HDD, we don't have to care about music files and Sound will take care of
		if (needCd) // switching sound.clu files on Sound::newScreen by itself, so there's nothing to be done.
			_systemVars.currentCD = needCd;
		else if (_systemVars.currentCD == 0)
			_systemVars.currentCD = 1;
	}
}

uint8 SwordEngine::mainLoop(void) {
	uint8 retCode = 0;
	_keyPressed = 0;

	while ((retCode == 0) && (!_systemVars.engineQuit)) {
		// do we need the section45-hack from sword.c here?
		checkCd();

		_screen->newScreen(Logic::_scriptVars[NEW_SCREEN]);
		_logic->newScreen(Logic::_scriptVars[NEW_SCREEN]);
		_sound->newScreen(Logic::_scriptVars[NEW_SCREEN]);
		Logic::_scriptVars[SCREEN] = Logic::_scriptVars[NEW_SCREEN];
		
		do {
			uint32 newTime;
			bool scrollFrameShown = false;

			uint32 frameTime = _system->getMillis();
			_logic->engine();
			_logic->updateScreenParams(); // sets scrolling

			_screen->draw();
			_mouse->animate();
			_sound->engine();
			_menu->refresh(MENU_TOP);
			_menu->refresh(MENU_BOT);

			newTime = _system->getMillis();
			if (newTime - frameTime < 1000 / FRAME_RATE) {
				scrollFrameShown = _screen->showScrollFrame();
				delay((1000 / (FRAME_RATE * 2)) - (_system->getMillis() - frameTime));
			}

			newTime = _system->getMillis();
			if ((newTime - frameTime < 1000 / FRAME_RATE) || (!scrollFrameShown))
				_screen->updateScreen();
			delay((1000 / FRAME_RATE) - (_system->getMillis() - frameTime));			

			_mouse->engine( _mouseX, _mouseY, _mouseState);

			if (_systemVars.forceRestart)
				retCode = CONTROL_RESTART_GAME;

			// The control panel is triggered by F5 or ESC.
			// FIXME: This is a very strange way of detecting F5...
			else if (((_keyPressed == 63 || _keyPressed == 27) && (Logic::_scriptVars[MOUSE_STATUS] & 1)) || (_systemVars.controlPanelMode)) {
				retCode = _control->runPanel();
				if (!retCode)
					_screen->fullRefresh();
			}
			_mouseState = _keyPressed = 0;
		} while ((Logic::_scriptVars[SCREEN] == Logic::_scriptVars[NEW_SCREEN]) && (retCode == 0) && (!_systemVars.engineQuit));

		if ((retCode == 0) && (Logic::_scriptVars[SCREEN] != 53) && _systemVars.wantFade && (!_systemVars.engineQuit)) {
			_screen->fadeDownPalette();
			int32 relDelay = (int32)_system->getMillis();
			while (_screen->stillFading()) {
				relDelay += (1000 / FRAME_RATE);
				_screen->updateScreen();
				delay(relDelay - (int32)_system->getMillis());
			}
		}

		_sound->quitScreen();
		_screen->quitScreen(); // close graphic resources
		_objectMan->closeSection(Logic::_scriptVars[SCREEN]); // close the section that PLAYER has just left, if it's empty now
	}
	return retCode;
}

void SwordEngine::delay(int32 amount) { //copied and mutilated from sky.cpp

	OSystem::Event event;
	uint32 start = _system->getMillis();

	do {
		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				// Make sure backspace works right (this fixes a small issue on OS X)
				if (event.kbd.keycode == 8)
					_keyPressed = 8;
				else
					_keyPressed = (uint8)event.kbd.ascii;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				_mouseState |= BS1L_BUTTON_DOWN;
#ifdef _WIN32_WCE
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
#endif
				break;
			case OSystem::EVENT_RBUTTONDOWN:
				_mouseState |= BS1R_BUTTON_DOWN;
#ifdef _WIN32_WCE
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
#endif
				break;
			case OSystem::EVENT_LBUTTONUP:
				_mouseState |= BS1L_BUTTON_UP;
				break;
			case OSystem::EVENT_RBUTTONUP:
				_mouseState |= BS1R_BUTTON_UP;
				break;
			case OSystem::EVENT_QUIT:
				_systemVars.engineQuit = true;
				break;
			default:
				break;
			}
		}
		if (amount > 0)
			_system->delayMillis(10);
	} while (_system->getMillis() < start + amount);
}

} // End of namespace Sword1
