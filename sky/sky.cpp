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
#include "common/file.h"
#include "common/gameDetector.h"
#include <errno.h>
#include <time.h>

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;

#endif

static const VersionSettings sky_settings[] = {
	/* Beneath a Steel Sky */
	{"sky", "Beneath a Steel Sky", GID_SKY_FIRST, 99, 99, 99, 0, "sky.dsk" },
	{NULL, NULL, 0, 0, 0, 0, 0, NULL}
};

const VersionSettings *Engine_SKY_targetList() {
	return sky_settings;
}

Engine *Engine_SKY_create(GameDetector *detector, OSystem *syst) {
	return new SkyState(detector, syst);
}

SkyState::SkyState(GameDetector *detector, OSystem *syst)
	: Engine(detector, syst) {
	
	_game = detector->_gameId;

	if (!_mixer->bindToSystem(syst))
		warning("Sound initialisation failed.");

	_mixer->setVolume(detector->_sfx_volume); //unnecessary?
	
	_debugMode = detector->_debugMode;
	_debugLevel = detector->_debugLevel;
	_language = detector->_language;
}

SkyState::~SkyState() {
	
}

void SkyState::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void SkyState::pollMouseXY() {

	_mouse_x = _sdl_mouse_x;
	_mouse_y = _sdl_mouse_y;
}

void SkyState::go() {

	if (!_dump_file)
		_dump_file = stdout;

	initialise();
	if (!isDemo(_gameVersion) || isCDVersion(_gameVersion))
		intro();
	
	while (1) {
		delay(100);
	}
}

void SkyState::initialise(void) {

	//initialise_memory();
	initTimer();
	//init_music(); 

	_sound = new SkySound(_mixer);

	_skyDisk = new SkyDisk(_gameDataPath);
	_gameVersion = _skyDisk->determineGameVersion();
	_skyText = getSkyText();
	
	initialiseScreen();
	initVirgin();
	//initMouse();
	initItemList();
	//initScript();
	initialiseGrids();
	//initialiseRouter();
}

void SkyState::initItemList() {
	
	//See List.asm for (cryptic) item # descriptions

	for (int i = 0; i < 300; i++)
		_itemList[i] = (void*)NULL;

	//init the non-null items
	/*
	_itemList[119] = (void*)data_0; // Compacts - Section 0
	_itemList[120] = (void*)data_1; // Compacts - Section 1
	
	if (isDemo(_gameVersion)) {
		_itemList[121] = _itemList[122] = _itemList[123] = _itemList[124] = _itemList[125] = (void*)data_0;
	} else {
		_itemList[121] = (void*)data_2; // Compacts - Section 2
		_itemList[122] = (void*)data_3; // Compacts - Section 3
		_itemList[123] = (void*)data_4; // Compacts - Section 4
		_itemList[124] = (void*)data_5; // Compacts - Section 5
		_itemList[125] = (void*)data_6; // Compacts - Section 6
	}
	*/
}

void SkyState::delay(uint amount) { //copied and mutilated from Simon.cpp

	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;

	_rnd.getRandomNumber(2);

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
					_mouse_pos_changed = true;
					break;

					case OSystem::EVENT_LBUTTONDOWN:
					_left_button_down++;
#ifdef _WIN32_WCE
					_sdl_mouse_x = event.mouse.x;
					_sdl_mouse_y = event.mouse.y;
#endif
					break;

				case OSystem::EVENT_RBUTTONDOWN:
					
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

SkyText *SkyState::getSkyText() {

	switch (_gameVersion) {
	case 267:
		//floppy demo
		return new SkyText_v00267(_skyDisk, _gameVersion);
	case 288:
		//floppy version
		return new SkyText_v00288(_skyDisk, _gameVersion);
	case 303:
		//floppy version
		return new SkyText_v00303(_skyDisk, _gameVersion);
	case 331:
		//floppy version
		return new SkyText_v00331(_skyDisk, _gameVersion);
	case 365:
		//cd demo, uses a slightly modified version of v00372
	case 368:
		//cd version, uses a slightly modified version of v00372
	case 372:
		//cd version
		return new SkyText_v00372(_skyDisk, _gameVersion);
	default:
		error("Unknown game version");
	}
}

bool SkyState::isDemo(uint32 version) {
	switch (version) {
	case 267:
		return true;
	case 288:
		return false;
	case 303:
		return false;
	case 331:
		return false;
	case 365:
		return true;
	case 368:
		return false;
	case 372:
		return false;
	default:
		error("Unknown game version");
	}
}

bool SkyState::isCDVersion(uint32 version) {
	switch (version) {
	case 267:
		return false;
	case 288:
		return false;
	case 303:
		return false;
	case 331:
		return false;
	case 365:
		return true;
	case 368:
		return true;
	case 372:
		return true;
	default:
		error("Unknown game version");
	}
}

