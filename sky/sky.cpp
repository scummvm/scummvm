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

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/timer.h"

#include "sky/compact.h"
#include "sky/control.h"
#include "sky/debug.h"
#include "sky/disk.h"
#include "sky/intro.h"
#include "sky/logic.h"
#include "sky/mouse.h"
#include "sky/music/adlibmusic.h"
#include "sky/music/gmmusic.h"
#include "sky/music/mt32music.h"
#include "sky/music/musicbase.h"
#include "sky/screen.h"
#include "sky/sky.h"
#include "sky/skydefs.h"
#include "sky/sound.h"
#include "sky/text.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;

#endif

/*
 At the beginning the reverse engineers were happy, and did rejoice at
 their task, for the engine before them did shineth and was full of
 promise. But then they did look closer and see'th the aweful truth;
 it's code was assembly and messy (rareth was its comments). And so large
 were it's includes that did at first seem small; queereth also was its
 compact(s). Then they did findeth another version, and this was slightly 
 different from the first. Then a third, and this was different again.
 All different, but not really better, for all were not really compatible.
 But, eventually, it did come to pass that Steel Sky was implemented on
 a modern platform. And the programmers looked and saw that it was indeed a
 miracle. But they were not joyous and instead did weep for nobody knew
 just what had been done. Except people who read the source. Hello.

 With apologies to the CD32 SteelSky file.
*/

#undef WITH_DEBUG_CHEATS

static const GameSettings skySetting =
	{"sky", "Beneath a Steel Sky", 0 };

GameList Engine_SKY_gameList() {
	GameList games;
	games.push_back(skySetting);
	return games;
}

DetectedGameList Engine_SKY_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	// Iterate over all files in the given directory
	for (FSList::ConstIterator file = fslist.begin(); file != fslist.end(); ++file) {
		const char *fileName = file->displayName().c_str();

		if (0 == scumm_stricmp("sky.dsk", fileName)) {
			// Match found, add to list of candidates, then abort inner loop.
			detectedGames.push_back(skySetting);
			break;
		}
	}
	return detectedGames;
}

Engine *Engine_SKY_create(GameDetector *detector, OSystem *syst) {
	return new Sky::SkyEngine(detector, syst);
}

REGISTER_PLUGIN("Beneath a Steel Sky", Engine_SKY_gameList, Engine_SKY_create, Engine_SKY_detectGames)


namespace Sky {

void **SkyEngine::_itemList[300];

SystemVars SkyEngine::_systemVars = {0, 0, 0, 0, 4316, 0, 0, false, false };

SkyEngine::SkyEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {
	
	if (!_mixer->isReady())
		warning("Sound initialisation failed.");

	_mixer->setVolume(ConfMan.getInt("sfx_volume")); //unnecessary?
	
	_debugMode = ConfMan.hasKey("debuglevel");

	_floppyIntro = ConfMan.getBool("floppy_intro");

	_fastMode = 0;

	_system->init_size(320, 200);
}

SkyEngine::~SkyEngine() {

	delete _skyLogic;
	delete _skySound;
	delete _skyMusic;
	delete _skyText;
	delete _skyMouse;
	delete _skyScreen;
}

void SkyEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void SkyEngine::initVirgin() {
	
	_skyScreen->setPalette(60111);
	_skyScreen->showScreen(60110);
}

void SkyEngine::doCheat(uint8 num) {

	switch(num) {
	case 1: warning("executed cheat: get jammer");
		Logic::_scriptVariables[258] = 42; // got_jammer
		Logic::_scriptVariables[240] = 69; // got_sponsor
		break;
	case 2: warning("executed cheat: computer room");
		Logic::_scriptVariables[479] = 2; // card_status
		Logic::_scriptVariables[480] = 1; // card_fix
		break;
	case 3: warning("executed cheat: get to burke");
		Logic::_scriptVariables[190] = 42; // knows_port
		break;
	case 4: warning("executed cheat: get to reactor section");
		Logic::_scriptVariables[451] = 42; // foreman_friend
		_skyLogic->fnSendSync(8484, 1, 0); // send sync to RAD suit (put in locker)
		_skyLogic->fnKillId(ID_ANITA_SPY, 0, 0); // stop anita from getting to you
		break;
	default: warning("unknown cheat: %d", num);
		break;
	}
}

void SkyEngine::handleKey(void) {

	if (_key_pressed == 63)
		_skyControl->doControlPanel();

	if ((_key_pressed == 27) && (!_systemVars.pastIntro))
		_skyControl->restartGame();
#ifdef WITH_DEBUG_CHEATS
	if ((_key_pressed >= '0') && (_key_pressed <= '9'))
		doCheat(_key_pressed - '0');

	if (_key_pressed == 'r') {
		warning("loading grid");
		_skyLogic->_skyGrid->loadGrids();
	}
#endif
	if (_key_pressed == '.')
		_skyMouse->logicClick();
	_key_pressed = 0;
}

void SkyEngine::go() {

	if (!_dump_file)
		_dump_file = stdout;

	initialise();

	bool introSkipped = false;
	if (!_quickLaunch) {
		if (_systemVars.gameVersion > 267) {// don't do intro for floppydemos
			_skyIntro = new Intro(_skyDisk, _skyScreen, _skyMusic, _skySound, _skyText, _mixer, _system);
			introSkipped = !_skyIntro->doIntro(_floppyIntro);
			if (_skyIntro->_quitProg) {
				delete _skyIntro;
				_skyControl->showGameQuitMsg();
			}
			delete _skyIntro;
		}
		loadBase0();
	}

	if (introSkipped)
		_skyControl->restartGame();
	
	_lastSaveTime = _system->get_msecs();

	while (1) {
		if (_fastMode & 2)
			delay(0);
		else if (_fastMode & 1)
			delay(10);
		else
			delay(_systemVars.gameSpeed);

		if (_system->get_msecs() - _lastSaveTime > 5 * 60 * 1000) {
			if (_skyControl->loadSaveAllowed()) {
				_lastSaveTime = _system->get_msecs();
				_skyControl->doAutoSave();
			} else
				_lastSaveTime += 30 * 1000; // try again in 30 secs
		}
		_skySound->checkFxQueue();
		_skyMouse->mouseEngine((uint16)_sdl_mouse_x, (uint16)_sdl_mouse_y);
		if (_key_pressed)
			handleKey();
		_skyLogic->engine();
		if (!_skyLogic->checkProtection()) { // don't let copy prot. screen flash up
			_skyScreen->recreate();
			_skyScreen->spriteEngine();
			_skyScreen->flip();
		}
	}
}

void SkyEngine::initialise(void) {
	_skyDisk = new Disk(_gameDataPath);
	_skySound = new Sound(_mixer, _skyDisk, ConfMan.getInt("sfx_volume"));
	
	_systemVars.gameVersion = _skyDisk->determineGameVersion();

	int midiDriver = GameDetector::detectMusicDriver(MDT_ADLIB | MDT_NATIVE | MDT_PREFER_NATIVE);
	if (midiDriver == MD_ADLIB) {
		_systemVars.systemFlags |= SF_SBLASTER;
		_skyMusic = new AdlibMusic(_mixer, _skyDisk, _system);
	} else {
		_systemVars.systemFlags |= SF_ROLAND;
		if (ConfMan.getBool("native_mt32"))
			_skyMusic = new MT32Music(GameDetector::createMidi(midiDriver), _skyDisk, _system);
		else
			_skyMusic = new GmMusic(GameDetector::createMidi(midiDriver), _skyDisk, _system);
	}

	if (isCDVersion()) {
		_systemVars.systemFlags |= SF_ALLOW_SPEECH;
		if (ConfMan.hasKey("nosubtitles")) {
			warning("Configuration key 'nosubtitles' is deprecated. Use 'subtitles' instead");
			if (!ConfMan.getBool("nosubtitles"))
				_systemVars.systemFlags |= SF_ALLOW_TEXT;
		}

		if (ConfMan.getBool("subtitles"))
			_systemVars.systemFlags |= SF_ALLOW_TEXT;
	} else
		_systemVars.systemFlags |= SF_ALLOW_TEXT;

	_systemVars.systemFlags |= SF_PLAY_VOCS;
	_systemVars.gameSpeed = 50;

	_skyText = new Text(_skyDisk);
	_skyMouse = new Mouse(_system, _skyDisk);
	_skyScreen = new Screen(_system, _skyDisk);

	initVirgin();
	initItemList();
	loadFixedItems();
	_skyLogic = new Logic(_skyScreen, _skyDisk, _skyText, _skyMusic, _skyMouse, _skySound);
	_skyMouse->useLogicInstance(_skyLogic);
	
	// initialize timer *after* _skyScreen has been initialized.
	_timer->installTimerProc(&timerHandler, 1000000 / 50, this); //call 50 times per second

	_skyControl = new Control(_skyScreen, _skyDisk, _skyMouse, _skyText, _skyMusic, _skyLogic, _skySound, _system, getSavePath());
	_skyLogic->useControlInstance(_skyControl);

	if (_systemVars.gameVersion == 288)
		SkyCompact::patchFor288();

	switch (Common::parseLanguage(ConfMan.get("language"))) {
	case Common::DE_DEU:
		_systemVars.language = SKY_GERMAN;
		break;
	case Common::FR_FRA:
		_systemVars.language = SKY_FRENCH;
		break;
	case Common::IT_ITA:
		_systemVars.language = SKY_ITALIAN;
		break;
	case Common::PT_BRA:
		_systemVars.language = SKY_PORTUGUESE;
		break;
	case Common::ES_ESP:
		_systemVars.language = SKY_SPANISH;
		break;
	case Common::SE_SWE:
		_systemVars.language = SKY_SWEDISH;
		break;
	case Common::EN_GRB:
		_systemVars.language = SKY_ENGLISH;
		break;
	default:
		_systemVars.language = SKY_USA;
		break;
	}

	if (!_skyDisk->fileExists(60600 + SkyEngine::_systemVars.language * 8)) {
		warning("The language you selected does not exist in your BASS version.");
		if (_skyDisk->fileExists(60600))
			SkyEngine::_systemVars.language = SKY_ENGLISH;
		else if (_skyDisk->fileExists(60600 + SKY_USA * 8))
			SkyEngine::_systemVars.language = SKY_USA;
		else
			for (uint8 cnt = SKY_ENGLISH; cnt <= SKY_SPANISH; cnt++)
				if (_skyDisk->fileExists(60600 + cnt * 8)) {
					SkyEngine::_systemVars.language = cnt;
					break;
				}
	}

	uint16 result = 0;
	if (ConfMan.hasKey("save_slot") && ConfMan.getInt("save_slot") >= 0)
		result = _skyControl->quickXRestore(ConfMan.getInt("save_slot"));

	if (result == GAME_RESTORED)
		_quickLaunch = true;
	else
		_quickLaunch = false;

	_skyMusic->setVolume(ConfMan.getInt("music_volume") >> 1);
}

void SkyEngine::initItemList() {
	
	//See List.asm for (cryptic) item # descriptions

	for (int i = 0; i < 300; i++)
		_itemList[i] = (void **)NULL;

	//init the non-null items
	_itemList[119] = (void **)SkyCompact::data_0; // Compacts - Section 0
	_itemList[120] = (void **)SkyCompact::data_1; // Compacts - Section 1
	
	if (isDemo()) {
		_itemList[121] = _itemList[122] = _itemList[123] = _itemList[124] = _itemList[125] = (void **)SkyCompact::data_0;
	} else {
		_itemList[121] = (void **)SkyCompact::data_2; // Compacts - Section 2
		_itemList[122] = (void **)SkyCompact::data_3; // Compacts - Section 3
		_itemList[123] = (void **)SkyCompact::data_4; // Compacts - Section 4
		_itemList[124] = (void **)SkyCompact::data_5; // Compacts - Section 5
		_itemList[125] = (void **)SkyCompact::data_6; // Compacts - Section 6
	}
}

void SkyEngine::loadBase0(void) {

	_skyLogic->fnEnterSection(0, 0, 0);
	_skyMusic->startMusic(2);
	_systemVars.currentMusic = 2;
}

void SkyEngine::loadFixedItems(void) {

	if (!isDemo())
		_itemList[36] = (void **)_skyDisk->loadFile(36, NULL);

	_itemList[49] = (void **)_skyDisk->loadFile(49, NULL);
	_itemList[50] = (void **)_skyDisk->loadFile(50, NULL);
	_itemList[73] = (void **)_skyDisk->loadFile(73, NULL);
	_itemList[262] = (void **)_skyDisk->loadFile(262, NULL);

	if (isDemo()) 
		return;
	
	_itemList[263] = (void **)_skyDisk->loadFile(263, NULL);
	_itemList[264] = (void **)_skyDisk->loadFile(264, NULL);
	_itemList[265] = (void **)_skyDisk->loadFile(265, NULL);
	_itemList[266] = (void **)_skyDisk->loadFile(266, NULL);
	_itemList[267] = (void **)_skyDisk->loadFile(267, NULL);
	_itemList[269] = (void **)_skyDisk->loadFile(269, NULL);
	_itemList[271] = (void **)_skyDisk->loadFile(271, NULL);
	_itemList[272] = (void **)_skyDisk->loadFile(272, NULL);
		
}

void **SkyEngine::fetchItem(uint32 num) {

	return _itemList[num];
}

void SkyEngine::timerHandler(void *refCon) {

	((SkyEngine *)refCon)->gotTimerTick();
}

void SkyEngine::gotTimerTick(void) {

	_skyScreen->handleTimer();
}

Compact *SkyEngine::fetchCompact(uint32 a) {
	Debug::fetchCompact(a);
	uint32 sectionNum = (a & 0xf000) >> 12;
	uint32 compactNum = (a & 0x0fff);

	return (Compact *)(_itemList[119 + sectionNum][compactNum]);
}

void SkyEngine::delay(uint amount) { //copied and mutilated from Simon.cpp

	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;
	_key_pressed = 0;	//reset

	do {
		while (_system->poll_event(&event)) {
			switch (event.event_code) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.flags == OSystem::KBD_CTRL) {
					if (event.kbd.keycode == 'f') {
						_fastMode ^= 1;
						break;
					}
					if (event.kbd.keycode == 'g') {
						_fastMode ^= 2;
						break;
					}
				}

				// Make sure backspace works right (this fixes a small issue on OS X)
				if (event.kbd.keycode == 8)
					_key_pressed = 8;
				else
					_key_pressed = (byte)event.kbd.ascii;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				if (!(_systemVars.systemFlags & SF_MOUSE_LOCKED)) {
					_sdl_mouse_x = event.mouse.x;
					_sdl_mouse_y = event.mouse.y;
				}
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				_skyMouse->buttonPressed(2);
				break;
			case OSystem::EVENT_RBUTTONDOWN:
				_skyMouse->buttonPressed(1);
				break;
			case OSystem::EVENT_QUIT:
				if (!SkyEngine::_systemVars.quitting)
					_skyControl->showGameQuitMsg(); // will call _system->quit()
				break;
			default:
				break;
			}
		}

		if (amount == 0)
			break;

		{
			uint this_delay = 20; // 1?
#ifdef _WIN32_WCE
			this_delay = 10;
#endif
			if (this_delay > amount)
				this_delay = amount;
			_system->delay_msecs(this_delay);
		}
		cur = _system->get_msecs();
	} while (cur < start + amount);
}

bool SkyEngine::isDemo(void) {
	switch (_systemVars.gameVersion) {
	case 109: // pc gamer demo
	case 267: // floppy demo
	case 365: // cd demo
		return true;
	case 288:
	case 303:
	case 331:
	case 348:
	case 368:
	case 372:
		return false;
	default:
		error("Unknown game version %d", _systemVars.gameVersion);
	}
}

bool SkyEngine::isCDVersion(void) {

	switch (_systemVars.gameVersion) {
	case 109:
	case 267:
	case 288:
	case 303:
	case 331:
	case 348:
		return false;
	case 365:
	case 368:
	case 372:
		return true;
	default:
		error("Unknown game version %d", _systemVars.gameVersion);
	}
}

} // End of namespace Sky
