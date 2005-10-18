/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "sword1/sword1.h"

#include "backends/fs/fs.h"

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/timer.h"
#include "common/system.h"

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
	int i;
	DetectedGameList detectedGames;
	bool filesFound[NUM_FILES_TO_CHECK];
	for (i = 0; i < NUM_FILES_TO_CHECK; i++)
		filesFound[i] = false;

	Sword1CheckDirectory(fslist, filesFound);
	bool mainFilesFound = true;
	for (i = 0; i < NUM_FILES_TO_CHECK -1; i++)
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

REGISTER_PLUGIN(SWORD1, "Broken Sword")

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
	Common::File::addDefaultDirectory(_gameDataPath + "CLUSTERS/");
	Common::File::addDefaultDirectory(_gameDataPath + "MUSIC/");
	Common::File::addDefaultDirectory(_gameDataPath + "SPEECH/");
	Common::File::addDefaultDirectory(_gameDataPath + "VIDEO/");
	Common::File::addDefaultDirectory(_gameDataPath + "clusters/");
	Common::File::addDefaultDirectory(_gameDataPath + "music/");
	Common::File::addDefaultDirectory(_gameDataPath + "speech/");
	Common::File::addDefaultDirectory(_gameDataPath + "video/");
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

	checkCdFiles();

	debug(5, "Starting resource manager");
	_resMan = new ResMan("swordres.rif");
	debug(5, "Starting object manager");
	_objectMan = new ObjectMan(_resMan);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, Audio::Mixer::kMaxMixerVolume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, Audio::Mixer::kMaxMixerVolume);
	_mouse = new Mouse(_system, _resMan, _objectMan);
	_screen = new Screen(_system, _resMan, _objectMan);
	_music = new Music(_mixer);
	_sound = new Sound("", _mixer, _resMan);
	_menu = new Menu(_screen, _mouse);
	_logic = new Logic(_objectMan, _resMan, _screen, _mouse, _sound, _music, _menu, _system, _mixer);
	_mouse->useLogicAndMenu(_logic, _menu);

	uint musicVol = ConfMan.getInt("music_volume");
	uint speechVol = ConfMan.getInt("speech_volume");
	uint sfxVol = ConfMan.getInt("sfx_volume");
	if (musicVol > 255)
		musicVol = 255;
	if (speechVol > 255)
		speechVol = 255;
	if (sfxVol > 255)
		sfxVol = 255;

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

void SwordEngine::flagsToBool(bool *dest, uint8 flags) {
	uint8 bitPos = 0;
	while (flags) {
		if (flags & 1)
			dest[bitPos] = true;
		flags >>= 1;
		bitPos++;
	}
}

static const char *errorMsgs[] = {
	"The file \"%s\" is missing and the game doesn't work without it.\n"
	"Please copy it from CD %d and try starting the game again.\n"
	"The Readme file also contains further information.",

	"%d important files are missing, the game can't start without them.\n"
	"Please copy these files from their corresponding CDs:\n",

    "The file \"%s\" is missing.\n"
	"Even though the game may initially seem to\n"
	"work fine, it will crash when it needs the\n"
	"data from this file and you will be thrown back to your last savegame.\n"
	"Please copy the file from CD %d and start the game again.",

	"%d files are missing.\n"
	"Even though the game may initially seem to\n"
	"work fine, it will crash when it needs the\n"
	"data from these files and you will be thrown back to your last savegame.\n"
	"Please copy these files from their corresponding CDs:\n"
};

const CdFile SwordEngine::_cdFileList[] = {
	{ "paris2.clu", FLAG_CD1 },
	{ "ireland.clu", FLAG_CD2 },
	{ "paris3.clu", FLAG_CD1 },
	{ "paris4.clu", FLAG_CD1 },
	{ "scotland.clu", FLAG_CD2 },
	{ "spain.clu", FLAG_CD2 },
	{ "syria.clu", FLAG_CD2 },
	{ "train.clu", FLAG_CD2 },
	{ "compacts.clu", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "general.clu", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "maps.clu", FLAG_CD1 | FLAG_DEMO },
	{ "paris1.clu", FLAG_CD1 | FLAG_DEMO },
	{ "scripts.clu", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "swordres.rif", FLAG_CD1 | FLAG_DEMO | FLAG_IMMED },
	{ "text.clu", FLAG_CD1 | FLAG_DEMO },
	{ "cows.mad", FLAG_DEMO },
	{ "speech1.clu", FLAG_SPEECH1 },
	 { "speech2.clu", FLAG_SPEECH2 }
#ifdef USE_MAD
	,{ "speech1.cl3", FLAG_SPEECH1 },
	 { "speech2.cl3", FLAG_SPEECH2 }
#endif
#ifdef USE_VORBIS
	,{ "speech1.clv", FLAG_SPEECH1 },
	 { "speech2.clv", FLAG_SPEECH2 }
#endif
};

void SwordEngine::showFileErrorMsg(uint8 type, bool *fileExists) {
	char msg[1024];
	int missCnt = 0, missNum = 0;
	for (int i = 0; i < ARRAYSIZE(_cdFileList); i++)
		if (!fileExists[i]) {
			missCnt++;
			missNum = i;
		}
	assert(missCnt > 0); // this function shouldn't get called if there's nothing missing.
	warning("%d files missing", missCnt);
	int msgId = (type == TYPE_IMMED) ? 0 : 2;
	if (missCnt == 1) {
		sprintf(msg, errorMsgs[msgId],
				_cdFileList[missNum].name, (_cdFileList[missNum].flags & FLAG_CD2) ? 2 : 1);
		warning(msg);
	} else {
		char *pos = msg + sprintf(msg, errorMsgs[msgId + 1], missCnt);
		warning(msg);
		for (int i = 0; i < ARRAYSIZE(_cdFileList); i++)
			if (!fileExists[i]) {
				warning("\"%s\" (CD %d)", _cdFileList[i].name, (_cdFileList[i].flags & FLAG_CD2) ? 2 : 1);
				pos += sprintf(pos, "\"%s\" (CD %d)\n", _cdFileList[i].name, (_cdFileList[i].flags & FLAG_CD2) ? 2 : 1);
			}
	}
	GUI::MessageDialog dialog(msg);
	dialog.runModal();
	if (type == TYPE_IMMED) // we can't start without this file, so error() out.
		error(msg);
}

void SwordEngine::checkCdFiles(void) { // check if we're running from cd, hdd or what...
	Common::File test;
	bool fileExists[30];
	bool isFullVersion = false; // default to demo version
	bool missingTypes[8] = { false, false, false, false, false, false, false, false };
	bool foundTypes[8] = { false, false, false, false, false, false, false, false };
	bool cd2FilesFound = false;
	_systemVars.runningFromCd = false;
	_systemVars.playSpeech = true;

	// check all files and look out if we can find a file that wouldn't exist if this was the demo version
	for (int fcnt = 0; fcnt < ARRAYSIZE(_cdFileList); fcnt++) {
		if (test.open(_cdFileList[fcnt].name)) {
			test.close();
			fileExists[fcnt] = true;
			flagsToBool(foundTypes, _cdFileList[fcnt].flags);
			if (!(_cdFileList[fcnt].flags & FLAG_DEMO))
				isFullVersion = true;
			if (_cdFileList[fcnt].flags & FLAG_CD2)
				cd2FilesFound = true;
		} else {
			flagsToBool(missingTypes, _cdFileList[fcnt].flags);
			fileExists[fcnt] = false;
		}
	}

	if (((_features & GF_DEMO) == 0) != isFullVersion) // shouldn't happen...
		warning("Your Broken Sword 1 version looks like a %s version but you are starting it as a %s version", isFullVersion ? "full" : "demo", (_features & GF_DEMO) ? "demo" : "full");

	if (foundTypes[TYPE_SPEECH1]) // we found some kind of speech1 file (.clu, .cl3, .clv)
		missingTypes[TYPE_SPEECH1] = false; // so we don't care if there's a different kind missing
	if (foundTypes[TYPE_SPEECH2]) // same for speech2
		missingTypes[TYPE_SPEECH2] = false;

	if (isFullVersion)					 // if this is the full version...
		missingTypes[TYPE_DEMO] = false; // then we don't need demo files...
	else								 // and vice versa
		missingTypes[TYPE_SPEECH1] = missingTypes[TYPE_SPEECH2] = missingTypes[TYPE_CD1] = missingTypes[TYPE_CD2] = false;

	bool somethingMissing = false;
	for (int i = 0; i < 8; i++)
		somethingMissing |= missingTypes[i];
	if (somethingMissing) { // okay, there *are* files missing
		// first, update the fileExists[] array depending on our changed missingTypes
		for (int fileCnt = 0; fileCnt < ARRAYSIZE(_cdFileList); fileCnt++)
			if (!fileExists[fileCnt]) {
				fileExists[fileCnt] = true;
				for (int flagCnt = 0; flagCnt < 8; flagCnt++)
					if (missingTypes[flagCnt] && ((_cdFileList[fileCnt].flags & (1 << flagCnt)) != 0))
						fileExists[fileCnt] = false; // this is one of the files we were looking for
			}
		if (missingTypes[TYPE_IMMED]) {
			// important files missing, can't start the game without them
			showFileErrorMsg(TYPE_IMMED, fileExists);
		} else if ((!missingTypes[TYPE_CD1]) && !cd2FilesFound) {
			/* we have all the data from cd one, but not a single one from CD2.
				I'm not sure how we should handle this, for now I'll just assume that the
				user has set up the extrapath correctly and copied the necessary files to HDD.
				A quite optimistic assumption, I'd say. Maybe we should change this for the release
				to warn the user? */
			warning("CD2 data files not found. I hope you know what you're doing and that\n"
					"you have set up the extrapath and additional data correctly.\n"
					"If you didn't, you should better read the ScummVM readme file");
			_systemVars.runningFromCd = true;
			_systemVars.playSpeech = true;
		} else if (missingTypes[TYPE_CD1] || missingTypes[TYPE_CD2]) {
			// several files from CD1 both CDs are missing. we can probably start, but it'll crash sooner or later
			showFileErrorMsg(TYPE_CD1, fileExists);
		} else if (missingTypes[TYPE_SPEECH1] || missingTypes[TYPE_SPEECH2]) {
			// not so important, but there won't be any voices
			if (missingTypes[TYPE_SPEECH1] && missingTypes[TYPE_SPEECH2])
				warning("Unable to find the speech files. The game will work, but you won't hear any voice output.\n"
						"Please copy the SPEECH.CLU files from both CDs and rename them to SPEECH1.CLU and SPEECH2.CLU,\n"
						"corresponding to the CD number.\n"
						"Please read the ScummVM Readme file for more information");
			else
				warning("Unable to find the speech file from CD %d.\n"
						"You won't hear any voice output in that part of the game.\n"
						"Please read the ScummVM Readme file for more information", missingTypes[TYPE_SPEECH1] ? 1 : 2);
		} else if (missingTypes[TYPE_DEMO]) {
			// for the demo version, we simply expect to have all files immediately
			showFileErrorMsg(TYPE_IMMED, fileExists);
		}
	} // everything's fine, let's play.
	/*if (!isFullVersion)
		_systemVars.isDemo = true;
	*/
	// make the demo flag depend on the Gamesettings for now, and not on what the datafiles look like
	_systemVars.isDemo = (_features & GF_DEMO) != 0;
	_systemVars.cutscenePackVersion = 0;
#ifdef USE_MPEG2
	if (test.open("intro.snd")) {
		_systemVars.cutscenePackVersion = 1;
		test.close();
	}
#endif
}

int SwordEngine::go() {

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
#if defined(_WIN32_WCE) || defined(__PALM_OS__)
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
#endif
				break;
			case OSystem::EVENT_RBUTTONDOWN:
				_mouseState |= BS1R_BUTTON_DOWN;
#if defined(_WIN32_WCE) || defined(__PALM_OS__)
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
