/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
#include "sky/sky.h"
#include "sky/skydefs.h" //game specific defines
#include "sky/compact.h"
#include "sky/logic.h"
#include "sky/debug.h"
#include "sky/mouse.h"
#include "common/file.h"
#include "common/gameDetector.h"
#include <errno.h>
#include <time.h>

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

static const VersionSettings sky_settings[] = {
	/* Beneath a Steel Sky */
	{"sky", "Beneath a Steel Sky", GID_SKY_FIRST, 99, VersionSettings::ADLIB_DONT_CARE, 0, "sky.dsk" },
	{NULL, NULL, 0, 0, VersionSettings::ADLIB_DONT_CARE, 0, NULL}
};

const VersionSettings *Engine_SKY_targetList() {
	return sky_settings;
}

Engine *Engine_SKY_create(GameDetector *detector, OSystem *syst) {
	return new SkyState(detector, syst);
}

void **SkyState::_itemList[300];

SystemVars SkyState::_systemVars = {0, 0, 0, 0, 4316, 0, false};

SkyState::SkyState(GameDetector *detector, OSystem *syst)
	: Engine(detector, syst) {
	
	_game = detector->_game.id;

	if (!_mixer->bindToSystem(syst))
		warning("Sound initialisation failed.");

	_mixer->setVolume(detector->_sfx_volume); //unnecessary?
	
	_debugMode = detector->_debugMode;
	_debugLevel = detector->_debugLevel;
	_systemVars.language = detector->_language;
	_detector = detector;

	_floppyIntro = detector->_floppyIntro;

	_introTextSpace = 0;
	_introTextSave = 0;
}

SkyState::~SkyState() {

	delete _skyLogic;
	delete _skySound;
	delete _skyMusic;
	delete _skyText;
	delete _skyMouse;
	delete _skyScreen;
}

void SkyState::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void SkyState::go() {

	if (!_dump_file)
		_dump_file = stdout;

	initialise();

	bool introSkipped;
	if (!isDemo() || isCDVersion())
		introSkipped = !intro();
	else introSkipped = false;

	_skyDisk->flushPrefetched();

	loadBase0();

	_paintGrid = false;

	/*if (introSkipped)
		_skyControl->restartGame();*/

	while (1) {
		delay(_systemVars.gameSpeed);
		if (_key_pressed == 'r') {
			warning("loading grid");
			_skyLogic->_skyGrid->loadGrids();
			_key_pressed = 0;
		}
		if (_key_pressed == 'g') {
			_paintGrid = !_paintGrid;
			warning("Grid paint: %s",(_paintGrid)?("ON"):("OFF"));
			if (!_paintGrid)
				_skyScreen->forceRefresh();
			_key_pressed = 0;
		}
		
		if (_key_pressed == 63) {
			_key_pressed = 0;
			_skyControl->doControlPanel();
		}			
		/*if ((_key_pressed == 27) && (!_systemVars.pastIntro))
			_skyControl->restartGame();*/

		_skySound->checkFxQueue();
		_skyMouse->mouseEngine((uint16)_sdl_mouse_x, (uint16)_sdl_mouse_y);
		_skyLogic->engine();
		if (!_skyLogic->checkProtection()) { // don't let copy prot. screen flash up
			if (_paintGrid)
				_skyScreen->forceRefresh();
			_skyScreen->recreate();
			_skyScreen->spriteEngine();
			_skyScreen->flip();
			if (_paintGrid)
				_skyScreen->showGrid(_skyLogic->_skyGrid->giveGrid(SkyLogic::_scriptVariables[SCREEN]));
			_system->update_screen();
		}
	}
}

void SkyState::initialise(void) {

	_skyDisk = new SkyDisk(_gameDataPath);
	_skySound = new SkySound(_mixer, _skyDisk);
	
	_systemVars.gameVersion = _skyDisk->determineGameVersion();

	if (_detector->getMidiDriverType() == MD_ADLIB) {
		_systemVars.systemFlags |= SF_SBLASTER;
		_skyMusic = new SkyAdlibMusic(_mixer, _skyDisk, _system);
	} else {
		_systemVars.systemFlags |= SF_ROLAND;
		if (_detector->_native_mt32)
			_skyMusic = new SkyMT32Music(_detector->createMidi(), _skyDisk, _system);
		else
			_skyMusic = new SkyGmMusic(_detector->createMidi(), _skyDisk, _system);
	}

        // Override global fullscreen setting with any game-specific define
        if (g_config->getBool("fullscreen", false)) {
                if (!_system->property(OSystem::PROP_GET_FULLSCREEN, 0))
                        _system->property(OSystem::PROP_TOGGLE_FULLSCREEN, 0);
        }

	if (isCDVersion())
		_systemVars.systemFlags |= SF_ALLOW_SPEECH | SF_ALLOW_TEXT;
	else
		_systemVars.systemFlags |= SF_ALLOW_TEXT;

	_systemVars.systemFlags |= SF_PLAY_VOCS;
	_systemVars.gameSpeed = 50;

	_skyText = new SkyText(_skyDisk);
	_skyMouse = new SkyMouse(_system, _skyDisk);
	_skyScreen = new SkyScreen(_system, _skyDisk);

	initVirgin();
	initItemList();
	loadFixedItems();
	_skyLogic = new SkyLogic(_skyScreen, _skyDisk, _skyText, _skyMusic, _skyMouse, _skySound);
	_skyMouse->useLogicInstance(_skyLogic);
	
	_timer = Engine::_timer; // initialize timer *after* _skyScreen has been initialized.
	_timer->installProcedure(&timerHandler, 1000000 / 50); //call 50 times per second

	_skyControl = new SkyControl(_skyScreen, _skyDisk, _skyMouse, _skyText, _skyMusic, _skyLogic, _skySound, _system, getSavePath());
	_skyLogic->useControlInstance(_skyControl);

	if (_systemVars.gameVersion == 288)
		SkyCompact::patchFor288();
}

void SkyState::initItemList() {
	
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

void SkyState::loadBase0(void) {

	_skyLogic->fnEnterSection(0, 0, 0);
	_skyMusic->startMusic(2);
}

void SkyState::loadFixedItems(void) {

	if (!isDemo())
		_itemList[36] = (void **)_skyDisk->loadFile(26, NULL);

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

void **SkyState::fetchItem(uint32 num) {

	return _itemList[num];
}

void SkyState::timerHandler(void *ptr) {

	((SkyState*)ptr)->gotTimerTick();
}

void SkyState::gotTimerTick(void) {

	_skyScreen->handleTimer();
}

Compact *SkyState::fetchCompact(uint32 a) {
	SkyDebug::fetchCompact(a);
	uint32 sectionNum = (a & 0xf000) >> 12;
	uint32 compactNum = (a & 0x0fff);

	return (Compact *)(_itemList[119 + sectionNum][compactNum]);
}

void SkyState::delay(uint amount) { //copied and mutilated from Simon.cpp

	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;
	_key_pressed = 0;	//reset

	do {
		while (_system->poll_event(&event)) {
			switch (event.event_code) {
				case OSystem::EVENT_KEYDOWN:
					// Make sure backspace works right (this fixes a small issue on OS X)
					if (event.kbd.keycode == 8)
						_key_pressed = 8;
					else
						_key_pressed = (byte)event.kbd.ascii;
					break;

				case OSystem::EVENT_MOUSEMOVE:
					_sdl_mouse_x = event.mouse.x;
					_sdl_mouse_y = event.mouse.y;
					_system->set_mouse_pos(_sdl_mouse_x, _sdl_mouse_y);
					break;

				case OSystem::EVENT_LBUTTONDOWN:
					_skyMouse->buttonPressed(2);
#ifdef _WIN32_WCE
					_sdl_mouse_x = event.mouse.x;
					_sdl_mouse_y = event.mouse.y;
#endif
					break;

				case OSystem::EVENT_RBUTTONDOWN:
					_skyMouse->buttonPressed(1);					
					break;

				case OSystem::EVENT_QUIT:
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
			if (this_delay > amount)
				this_delay = amount;
			_system->delay_msecs(this_delay);
		}
		cur = _system->get_msecs();
	} while (cur < start + amount);
}

bool SkyState::isDemo(void) {
	switch (_systemVars.gameVersion) {
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
		error("Unknown game version");
	}
}

bool SkyState::isCDVersion(void) {

	switch (_systemVars.gameVersion) {
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
		error("Unknown game version");
	}
}

