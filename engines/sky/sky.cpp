/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/timer.h"

#include "sky/control.h"
#include "sky/debug.h"
#include "sky/disk.h"
#include "sky/grid.h"
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
#include "sky/compact.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;
extern bool isSmartphone(void);
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

static const GameSettings skySetting =
	{"sky", "Beneath a Steel Sky" };

GameList Engine_SKY_gameList() {
	GameList games;
	games.push_back(skySetting);
	return games;
}

DetectedGameList Engine_SKY_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	// Iterate over all files in the given directory
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			const char *fileName = file->displayName().c_str();

			if (0 == scumm_stricmp("sky.dsk", fileName)) {
				// Match found, add to list of candidates, then abort inner loop.
				// The game detector uses US English by default. We want British
				// English to match the recorded voices better.
				detectedGames.push_back(DetectedGame(skySetting, Common::EN_GRB, Common::kPlatformUnknown));
				break;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_SKY_create(GameDetector *detector, OSystem *syst) {
	return new Sky::SkyEngine(syst);
}

REGISTER_PLUGIN(SKY, "Beneath a Steel Sky")


namespace Sky {

void *SkyEngine::_itemList[300];

SystemVars SkyEngine::_systemVars = {0, 0, 0, 0, 4316, 0, 0, false, false, false };

SkyEngine::SkyEngine(OSystem *syst)
	: Engine(syst), _fastMode(0), _debugger(0) {
}

SkyEngine::~SkyEngine() {

	_timer->removeTimerProc(&timerHandler);

	delete _skyLogic;
	delete _skySound;
	delete _skyMusic;
	delete _skyText;
	delete _skyMouse;
	delete _skyScreen;
	delete _debugger;
	delete _skyDisk;
	delete _skyControl;
	delete _skyCompact;

	for (int i = 0; i < 300; i++)
		if (_itemList[i])
			free(_itemList[i]);
}

void SkyEngine::errorString(const char *buf1, char *buf2) {
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

void SkyEngine::initVirgin() {

	_skyScreen->setPalette(60111);
	_skyScreen->showScreen(60110);
}

void SkyEngine::handleKey(void) {

	if (_keyPressed && _systemVars.paused) {
		_skySound->fnUnPauseFx();
		_systemVars.paused = false;
		_skyScreen->setPaletteEndian((uint8 *)_skyCompact->fetchCpt(SkyEngine::_systemVars.currentPalette));
		_keyFlags = _keyPressed = 0;
		return;
	}

	if (_keyFlags == OSystem::KBD_CTRL) {
		if (_keyPressed == 'f')
			_fastMode ^= 1;
		else if (_keyPressed == 'g')
			_fastMode ^= 2;
		else if (_keyPressed == 'd')
			_debugger->attach();
	} else {
		switch (_keyPressed) {
		case '`':
		case '~':
		case '#':
			_debugger->attach();
			break;
		case 63:
			_skyControl->doControlPanel();
			break;

		case 27:
			if (!_systemVars.pastIntro)
				_skyControl->restartGame();
			break;

		case '.':
			_skyMouse->logicClick();
			break;

		case 'p':
			_skyScreen->halvePalette();
			_skySound->fnPauseFx();
			_systemVars.paused = true;
			break;

		}
	}
	_keyFlags = _keyPressed = 0;
}

int SkyEngine::go() {

	_systemVars.quitGame = false;

	_mouseX = GAME_SCREEN_WIDTH / 2;
	_mouseY = GAME_SCREEN_HEIGHT / 2;
	_keyFlags = _keyPressed = 0;

	uint16 result = 0;
	if (ConfMan.hasKey("save_slot") && ConfMan.getInt("save_slot") >= 0)
		result = _skyControl->quickXRestore(ConfMan.getInt("save_slot"));

	if (result != GAME_RESTORED) {
		bool introSkipped = false;
		if (_systemVars.gameVersion > 267) { // don't do intro for floppydemos
			_skyIntro = new Intro(_skyDisk, _skyScreen, _skyMusic, _skySound, _skyText, _mixer, _system);
			introSkipped = !_skyIntro->doIntro(_floppyIntro);
			_systemVars.quitGame = _skyIntro->_quitProg;

			delete _skyIntro;
		}

		if (!_systemVars.quitGame) {
			_skyLogic->initScreen0();
			if (introSkipped)
				_skyControl->restartGame();
		}
	}

	_lastSaveTime = _system->getMillis();

	uint32 delayCount = _system->getMillis();
	while (!_systemVars.quitGame) {
		if (_debugger->isAttached())
			_debugger->onFrame();

		if (shouldPerformAutoSave(_lastSaveTime)) {
			if (_skyControl->loadSaveAllowed()) {
				_lastSaveTime = _system->getMillis();
				_skyControl->doAutoSave();
			} else
				_lastSaveTime += 30 * 1000; // try again in 30 secs
		}
		_skySound->checkFxQueue();
		_skyMouse->mouseEngine((uint16)_mouseX, (uint16)_mouseY);
		handleKey();
		if (_systemVars.paused) {
			do {
				_system->updateScreen();
				delay(50);
				handleKey();
			} while (_systemVars.paused);
			delayCount = _system->getMillis();
		}

		_skyLogic->engine();
		_skyScreen->recreate();
		_skyScreen->spriteEngine();
		if (_debugger->showGrid()) {
			_skyScreen->showGrid(_skyLogic->_skyGrid->giveGrid(Logic::_scriptVariables[SCREEN]));
			_skyScreen->forceRefresh();
		}
		_skyScreen->flip();

		if (_fastMode & 2)
			delay(0);
		else if (_fastMode & 1)
			delay(10);
		else {
			delayCount += _systemVars.gameSpeed;
			int needDelay = delayCount - (int)_system->getMillis();
			if ((needDelay < 0) || (needDelay > 4 * _systemVars.gameSpeed)) {
				needDelay = 0;
				delayCount = _system->getMillis();
			}
			delay(needDelay);
		}
	}

	_skyControl->showGameQuitMsg();
	_skyMusic->stopMusic();
	ConfMan.flushToDisk();
	delay(1500);
	return 0;
}

int SkyEngine::init(GameDetector &detector) {
	_system->beginGFXTransaction();
		initCommonGFX(detector, false);
		_system->initSize(320, 200);
	_system->endGFXTransaction();

	if (!_mixer->isReady())
		warning("Sound initialisation failed");

	if (ConfMan.getBool("sfx_mute")) {
		SkyEngine::_systemVars.systemFlags |= SF_FX_OFF;
	}
	 _mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	 _mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_floppyIntro = ConfMan.getBool("alt_intro");

	_skyDisk = new Disk(_gameDataPath);
	_skySound = new Sound(_mixer, _skyDisk, ConfMan.getInt("sfx_volume"));

	_systemVars.gameVersion = _skyDisk->determineGameVersion();

	int midiDriver = MidiDriver::detectMusicDriver(MDT_ADLIB | MDT_MIDI | MDT_PREFER_MIDI);
	if (midiDriver == MD_ADLIB) {
		_systemVars.systemFlags |= SF_SBLASTER;
		_skyMusic = new AdlibMusic(_mixer, _skyDisk);
	} else {
		_systemVars.systemFlags |= SF_ROLAND;
		if ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"))
			_skyMusic = new MT32Music(MidiDriver::createMidi(midiDriver), _skyDisk);
		else
			_skyMusic = new GmMusic(MidiDriver::createMidi(midiDriver), _skyDisk);
	}

	if (isCDVersion()) {
		if (ConfMan.hasKey("nosubtitles")) {
			warning("Configuration key 'nosubtitles' is deprecated. Use 'subtitles' instead");
			if (!ConfMan.getBool("nosubtitles"))
				_systemVars.systemFlags |= SF_ALLOW_TEXT;
		}

		if (ConfMan.getBool("subtitles"))
			_systemVars.systemFlags |= SF_ALLOW_TEXT;

		if (!ConfMan.getBool("speech_mute"))
			_systemVars.systemFlags |= SF_ALLOW_SPEECH;

	} else
		_systemVars.systemFlags |= SF_ALLOW_TEXT;

	_systemVars.systemFlags |= SF_PLAY_VOCS;
	_systemVars.gameSpeed = 50;

	_skyCompact = new SkyCompact();
	_skyText = new Text(_skyDisk, _skyCompact);
	_skyMouse = new Mouse(_system, _skyDisk, _skyCompact);
	_skyScreen = new Screen(_system, _skyDisk, _skyCompact);

	initVirgin();
	initItemList();
	loadFixedItems();
	_skyLogic = new Logic(_skyCompact, _skyScreen, _skyDisk, _skyText, _skyMusic, _skyMouse, _skySound);
	_skyMouse->useLogicInstance(_skyLogic);

	// initialize timer *after* _skyScreen has been initialized.
	_timer->installTimerProc(&timerHandler, 1000000 / 50, this); //call 50 times per second

	_skyControl = new Control(_saveFileMan, _skyScreen, _skyDisk, _skyMouse, _skyText, _skyMusic, _skyLogic, _skySound, _skyCompact, _system);
	_skyLogic->useControlInstance(_skyControl);

	switch (Common::parseLanguage(ConfMan.get("language"))) {
	case Common::EN_USA:
		_systemVars.language = SKY_USA;
		break;
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
		_systemVars.language = SKY_ENGLISH;
		break;
	}

	if (!_skyDisk->fileExists(60600 + SkyEngine::_systemVars.language * 8)) {
		warning("The language you selected does not exist in your BASS version.");
		if (_skyDisk->fileExists(60600))
			SkyEngine::_systemVars.language = SKY_ENGLISH; // default to GB english if it exists..
		else if (_skyDisk->fileExists(60600 + SKY_USA * 8))
			SkyEngine::_systemVars.language = SKY_USA;		// try US english...
		else
			for (uint8 cnt = SKY_ENGLISH; cnt <= SKY_SPANISH; cnt++)
				if (_skyDisk->fileExists(60600 + cnt * 8)) {	// pick the first language we can find
					SkyEngine::_systemVars.language = cnt;
					break;
				}
	}

	_skyMusic->setVolume(ConfMan.getInt("music_volume") >> 1);

	_debugger = new Debugger(_skyLogic, _skyMouse, _skyScreen, _skyCompact);
	return 0;
}

void SkyEngine::initItemList() {

	//See List.asm for (cryptic) item # descriptions

	for (int i = 0; i < 300; i++)
		_itemList[i] = NULL;
}

void SkyEngine::loadFixedItems(void) {

	_itemList[49] = _skyDisk->loadFile(49);
	_itemList[50] = _skyDisk->loadFile(50);
	_itemList[73] = _skyDisk->loadFile(73);
	_itemList[262] = _skyDisk->loadFile(262);

	if (!isDemo()) {
		_itemList[36] = _skyDisk->loadFile(36);
		_itemList[263] = _skyDisk->loadFile(263);
		_itemList[264] = _skyDisk->loadFile(264);
		_itemList[265] = _skyDisk->loadFile(265);
		_itemList[266] = _skyDisk->loadFile(266);
		_itemList[267] = _skyDisk->loadFile(267);
		_itemList[269] = _skyDisk->loadFile(269);
		_itemList[271] = _skyDisk->loadFile(271);
		_itemList[272] = _skyDisk->loadFile(272);
	}
}

void *SkyEngine::fetchItem(uint32 num) {

	return _itemList[num];
}

void SkyEngine::timerHandler(void *refCon) {

	((SkyEngine *)refCon)->gotTimerTick();
}

void SkyEngine::gotTimerTick(void) {

	_skyScreen->handleTimer();
}

void SkyEngine::delay(int32 amount) {

	OSystem::Event event;

	uint32 start = _system->getMillis();
	_keyFlags = _keyPressed = 0;	//reset

	if (amount < 0)
		amount = 0;

	do {
		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				_keyFlags = event.kbd.flags;
				if (_keyFlags == OSystem::KBD_CTRL)
					_keyPressed = event.kbd.keycode;
				else
					_keyPressed = (byte)event.kbd.ascii;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				if (!(_systemVars.systemFlags & SF_MOUSE_LOCKED)) {
					_mouseX = event.mouse.x;
					_mouseY = event.mouse.y;
				}
				break;
			case OSystem::EVENT_LBUTTONDOWN:
#ifdef PALMOS_MODE
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
#endif
				_skyMouse->buttonPressed(2);
				break;
			case OSystem::EVENT_RBUTTONDOWN:
#ifdef PALMOS_MODE
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
#endif
				_skyMouse->buttonPressed(1);
				break;
			case OSystem::EVENT_QUIT:
				_systemVars.quitGame = true;
				break;
			default:
				break;
			}
		}

		if (amount > 0)
			_system->delayMillis((amount > 10) ? 10 : amount);

	} while (_system->getMillis() < start + amount);
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
