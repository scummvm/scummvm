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
#include "backends/fs/fs.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "queen/command.h"
#include "queen/cutaway.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/input.h"
#include "queen/queen.h"
#include "queen/sound.h"
#include "queen/talk.h"
#include "queen/walk.h"

extern uint16 _debugLevel;

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;

#endif

static const GameSettings queen_settings[] = {
	/* Flight of the Amazon Queen */
	{ "queen", "Flight of the Amazon Queen", MDT_ADLIB | MDT_NATIVE | MDT_PREFER_NATIVE, 0, "queen.1" },
	{ "queencomp", "Flight of the Amazon Queen", MDT_ADLIB | MDT_NATIVE | MDT_PREFER_NATIVE, 0, "queen.1c" },
	{ NULL, NULL, MDT_NONE, 0, NULL} 
};

GameList Engine_QUEEN_gameList() {
	const GameSettings *g = queen_settings;
	GameList games;
	while (g->gameName)
		games.push_back(*g++);
	return games;
}

GameList Engine_QUEEN_detectGames(const FSList &fslist) {
	GameList detectedGames;
	const GameSettings *g = &queen_settings[0];

	while(g->detectname) {
		// Iterate over all files in the given directory
		for (FSList::ConstIterator file = fslist.begin(); file != fslist.end(); ++file) {
			const char *gameName = file->displayName().c_str();

			if (0 == scumm_stricmp(g->detectname, gameName)) {
				// Match found, add to list of candidates, then abort inner loop.
				detectedGames.push_back(*g);
				break;
			}
		}
		g++;
	}
	return detectedGames;
}

Engine *Engine_QUEEN_create(GameDetector *detector, OSystem *syst) {
	return new Queen::QueenEngine(detector, syst);
}

REGISTER_PLUGIN("Flight of the Amazon Queen", Engine_QUEEN_gameList, Engine_QUEEN_create, Engine_QUEEN_detectGames);

namespace Queen {

QueenEngine::QueenEngine(GameDetector *detector, OSystem *syst)
	: Engine(detector, syst) {

	if (!_mixer->bindToSystem(syst))
		warning("Sound initialisation failed.");

	_mixer->setVolume(ConfMan.getInt("sfx_volume"));
	
	_debugMode = ConfMan.hasKey("debuglevel");
	_debugLevel = ConfMan.getInt("debuglevel");
	_detector = detector;

	_fastMode = 0;

	_system->init_size(320, 200);
}

QueenEngine::~QueenEngine() {

	_timer->removeTimerProc(&timerHandler);
	delete _resource;
	delete _display;
	delete _logic;
	delete _graphics;
	delete _input;
}

void QueenEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void QueenEngine::roomChanged() {
	// queen.c function SETUP_ROOM, lines 398-428

	// This function uses lots of variables in logic, but we can't move it to
	// logic because that would cause a circular dependency between Cutaway and
	// Logic... :-(

	if (_logic->currentRoom() == 7) {
		warning("Room 7 not yet handled!");
		_logic->handlePinnacleRoom();
		// XXX fadeout(0,223);
	}
	else if (_logic->currentRoom() == FOTAQ_LOGO && _logic->gameState(VAR_INTRO_PLAYED) == 0) {
#if 0
		bool pcGamesDemo = _resource->isDemo() && !_resource->exists("pclogo.cut");

		if (pcGamesDemo) {
			_logic->currentRoom(79);
		}
		_logic->roomDisplay(_logic->roomName(_logic->currentRoom()), RDM_FADE_NOJOE, 100, 2, true);

		if (_resource->isDemo()) {
			if (pcGamesDemo) {
				_logic->playCutaway("clogo.cut");
			}
			else {
				_logic->playCutaway("pclogo.cut");
			}
		}
		else {
			_logic->playCutaway("copy.cut");
			_logic->playCutaway("clogo.cut");

			// TODO enable talking for talkie version

			_logic->playCutaway("cdint.cut");

			// restore palette colors ranging from 144 to 256
			_graphics->loadPanel();
			
			_logic->playCutaway("cred.cut");
		}
#endif

		_logic->currentRoom(ROOM_HOTEL_LOBBY);
		_logic->entryObj(584);

		_logic->roomDisplay(_logic->roomName(_logic->currentRoom()), RDM_FADE_JOE, 100, 2, true);
		_logic->playCutaway("c70d.cut");

		_logic->gameState(VAR_INTRO_PLAYED, 1);

		_logic->inventorySetup();
		_logic->inventoryRefresh();
	}
	else {
		_logic->roomDisplay(_logic->roomName(_logic->currentRoom()), RDM_FADE_JOE, 100, 1, false);
	}
	_display->mouseCursorShow(true); // _drawMouseFlag = 1;
}


void QueenEngine::go() {

	if (!_dump_file)
		_dump_file = stdout;

	initialise();

	_logic->oldRoom(0);
	_logic->newRoom(_logic->currentRoom());

	for (;;) {
		// queen.c lines 4080-4104
		if (_logic->newRoom() > 0) {
			_graphics->textClear(151, 151);
			_logic->update();
			_logic->oldRoom(_logic->currentRoom());
			_logic->currentRoom(_logic->newRoom());
			roomChanged();
			_display->fullscreen(false);
			if (_logic->currentRoom() == _logic->newRoom()) {
				_logic->newRoom(0);
			}
		}
		else {
			if (_logic->joeWalk() == 2) {
				_logic->joeWalk(0);
				_logic->command()->executeCurrentAction(true);
			}
			else {
				if (_logic->command()->parse()) {
					_logic->command()->clear(true);
				}
				_logic->joeWalk(0);
				_logic->checkPlayer();
			}
		}
	}
}

void QueenEngine::initialise(void) {
	_resource = new Resource(_gameDataPath, _detector->_game.detectname);
	_input = new Input(_resource->getLanguage(), _system);
	_display = new Display(_system, _input);
	_graphics = new Graphics(_display, _input, _resource);
	_sound = Sound::giveSound(_mixer, _input, _resource, _resource->compression());
	_logic = new Logic(_resource, _graphics, _display, _input, _sound);
	_timer->installTimerProc(&timerHandler, 1000000 / 50, this); //call 50 times per second
}


void QueenEngine::timerHandler(void *ptr) {

	((QueenEngine *)ptr)->gotTimerTick();
}


void QueenEngine::gotTimerTick() {

	_display->handleTimer();
}

} // End of namespace Queen
