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
#include "sword1.h"

#include "backends/fs/fs.h"

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/timer.h"

#include "memman.h"
#include "resman.h"
#include "objectman.h"
#include "mouse.h"
#include "logic.h"
#include "sound.h"
#include "screen.h"
#include "swordres.h"
#include "menu.h"
#include "music.h"

// taken from Sword2.cpp
struct Sword1GameSettings {
	const char *name;
	const char *description;
	uint32 features;
	const char *detectname;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};

static const Sword1GameSettings sword1_settings[] = {
	/* Broken Sword 1 */
	{"sword1", "Broken Sword I", GF_DEFAULT_TO_1X_SCALER, "swordres.rif" },
	{NULL, NULL, 0, NULL}
};

GameList Engine_SWORD1_gameList() {
	const Sword1GameSettings *g = sword1_settings;
	GameList games;
	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}
	return games;
}

GameList Engine_SWORD1_detectGames(const FSList &fslist) {
	GameList detectedGames;
	const Sword1GameSettings *g;

	for (g = sword1_settings; g->name; ++g) {
		// Iterate over all files in the given directory
		for (FSList::ConstIterator file = fslist.begin(); file != fslist.end(); ++file) {
			const char *gameName = file->displayName().c_str();

			if (0 == scumm_stricmp(g->detectname, gameName)) {
				// Match found, add to list of candidates, then abort inner loop.
				detectedGames.push_back(g->toGameSettings());
				break;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_SWORD1_create(GameDetector *detector, OSystem *syst) {
	return new SwordEngine(detector, syst);
}

REGISTER_PLUGIN("Broken Sword", Engine_SWORD1_gameList, Engine_SWORD1_create, Engine_SWORD1_detectGames)

SystemVars SwordEngine::_systemVars;

void SwordEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

extern uint16 _debugLevel;

SwordEngine::SwordEngine(GameDetector *detector, OSystem *syst)
	: Engine(syst) {

	_detector = detector;
	_system = syst;
	_debugLevel = ConfMan.getInt("debuglevel");
	
	if (!_mixer->bindToSystem(syst))
		warning("Sound initialization failed");
}

SwordEngine::~SwordEngine() {
}

void SwordEngine::initialize(void) {
	_system->init_size(640, 480);
	debug(5, "Starting memory manager");
	_memMan = new MemMan();
	debug(5, "Starting resource manager");
	_resMan = new ResMan("swordres.rif", _memMan);
	debug(5, "Starting object manager");
	_objectMan = new ObjectMan(_resMan);
	_mixer->setVolume(255);
	_mouse = new SwordMouse(_system, _resMan, _objectMan);
	_screen = new SwordScreen(_system, _resMan, _objectMan);
	_music = new SwordMusic(_system, _mixer);
	_sound = new SwordSound("", _mixer, _resMan);
	_menu = new SwordMenu(_screen, _mouse);
	_logic = new SwordLogic(_objectMan, _resMan, _screen, _mouse, _sound, _music, _menu);
	_mouse->useLogic(_logic);

	_systemVars.justRestoredGame = _systemVars.currentCD = 
		_systemVars.gamePaused = _systemVars.saveGameFlag = 
		_systemVars.deathScreenFlag = _systemVars.currentMusic = 0;
	_systemVars.snrStatus = 0;
	_systemVars.rate = 8;

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
	// TODO add czech option
	default:
		_systemVars.language = BS1_ENGLISH;
	}
	
	_systemVars.playSpeech = 1;
	//- start.c:
	// todo: move these to somewhere else
	SwordLogic::_scriptVars[GEORGE_CDT_FLAG] = GEO_TLK_TABLE;
	SwordLogic::_scriptVars[CHANGE_X] = 481;
	SwordLogic::_scriptVars[CHANGE_Y] = 413;
	SwordLogic::_scriptVars[CHANGE_DIR] = DOWN;
	SwordLogic::_scriptVars[CHANGE_PLACE] = FLOOR_1;
	
	SwordLogic::_scriptVars[NEW_SCREEN] = 1;
	_objectMan->fetchObject(PLAYER)->o_screen = 1;
	_objectMan->megaEntering(1);

	SwordLogic::_scriptVars[CHANGE_STANCE] = STAND;
	_mouseState = 0;
}

void SwordEngine::go(void) {
	
	initialize();
	// check if we have savegames. if we do, show control panel, else start intro.
	do {
        mainLoop();
		// mainLoop was left, show control panel
	} while (true);
}

void SwordEngine::mainLoop(void) {
	uint32 newTime, frameTime;
	do {
		// do we need the section45-hack from sword.c here?
		// todo: ensure right cd is inserted
		_screen->newScreen(SwordLogic::_scriptVars[NEW_SCREEN]);
		_logic->newScreen(SwordLogic::_scriptVars[NEW_SCREEN]);
		SwordLogic::_scriptVars[SCREEN] = SwordLogic::_scriptVars[NEW_SCREEN];
		//		 let swordSound start room sfx
		do {
			_music->stream();
			frameTime = _system->get_msecs();
			_systemVars.saveGameFlag = 0;
			debug(5, "\n\nNext logic cycle");
			_logic->engine();
			_logic->updateScreenParams(); // sets scrolling

			_screen->recreate();
			_screen->spritesAndParallax();
			_mouse->animate();

			newTime = _system->get_msecs();
			/*if ((newTime - frameTime < 50) && (!SwordLogic::_scriptVars[NEW_PALETTE])) {
				RenderScreenGDK();
				BlitMenusGDK();
				BlitMousePm();
				if (newTime - frameTime < 40)
					_system->delay_msecs(40 - (newTime - frameTime));
				FlipScreens();
			}*/

			_sound->engine();
			_screen->updateScreen();
		//-
			_menu->refresh(MENU_TOP);
			_menu->refresh(MENU_BOT);

			newTime = _system->get_msecs();
			if (newTime - frameTime < 80)
				delay(80 - (newTime - frameTime));
			else
				delay(0);

			/*FlipScreens(); this is done in SwordScreen::updateScreen() now.
			if (SwordLogic::_scriptVars[NEW_PALETTE]) {
				SwordLogic::_scriptVars[NEW_PALETTE] = 0;
				startFadePaletteUp();
			}*/

			_mouse->engine( _mouseX, _mouseY, _mouseState);
			_mouseState = 0;
			// do something smart here to implement pausing the game. If we even want that, that is.
		} while ((SwordLogic::_scriptVars[SCREEN] == SwordLogic::_scriptVars[NEW_SCREEN]) &&
			(_systemVars.saveGameFlag < 2));	// change screen

		if (SwordLogic::_scriptVars[SCREEN] != 53)	// don't fade down after syria pan
			_screen->fadeDownPalette();
		while (_screen->stillFading()) {
			_screen->updateScreen();
			delay(1000/12);
			// todo: fade sfx?
		}

		_screen->quitScreen(); // close graphic resources
		_objectMan->closeSection(SwordLogic::_scriptVars[SCREEN]); // close the section that PLAYER has just left, if it's empty now
        // todo: stop sfx, clear sfx queue, free sfx memory
	} while (_systemVars.saveGameFlag < 2);
}

void SwordEngine::delay(uint amount) { //copied and mutilated from sky.cpp

	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;
	uint16 _key_pressed = 0;	//reset

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
				break;
			case OSystem::EVENT_QUIT:
				_system->quit();
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
