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
#include "queen/queen.h"
#include "queen/cutaway.h"
#include "queen/talk.h"
#include "queen/walk.h"
#include "queen/graphics.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "queen/display.h"
#include "queen/graphics.h"

extern uint16 _debugLevel;

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;

#endif

static const GameSettings queen_settings[] = {
	/* Flight of the Amazon Queen */
	{ "queen", "Flight of the Amazon Queen", GID_QUEEN_FIRST, 99, MDT_ADLIB | MDT_NATIVE | MDT_PREFER_NATIVE, 0, "queen.1" },
	{ NULL, NULL, 0, 0, MDT_NONE, 0, NULL} 
};

const GameSettings *Engine_QUEEN_targetList() {
	return queen_settings;
}

Engine *Engine_QUEEN_create(GameDetector *detector, OSystem *syst) {
	return new Queen::QueenEngine(detector, syst);
}

REGISTER_PLUGIN("Flight of the Amazon Queen", Engine_QUEEN_targetList, Engine_QUEEN_create);

namespace Queen {

QueenEngine::QueenEngine(GameDetector *detector, OSystem *syst)
	: Engine(detector, syst) {
	
	_game = detector->_game.id;

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
	delete _resource;
	delete _display;
	delete _graphics;
	delete _logic;
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
		// XXX R_MAP();
		// XXX fadeout(0,223);
	}
	else if (_logic->currentRoom() == 95 && _logic->gameState(117) == 0) {
		char nextFilename[20];

		_logic->roomDisplay(_logic->roomName(_logic->currentRoom()), RDM_FADE_NOJOE, 100, 2, true);

		if (_resource->isDemo()) {
			if (_resource->exists("pclogo.cut"))
				Cutaway::run("pclogo.cut", nextFilename, _graphics, _logic, _resource);
			else
				Cutaway::run("clogo.cut",  nextFilename, _graphics, _logic, _resource);
		}
		else {
			Cutaway::run("copy.cut",  nextFilename, _graphics, _logic, _resource);
			Cutaway::run("clogo.cut", nextFilename, _graphics, _logic, _resource);

			// TODO enable talking for talkie version

			Cutaway::run("cdint.cut", nextFilename, _graphics, _logic, _resource);

			// XXX _graphics->panelLoad();
			
			Cutaway::run("cred.cut",  nextFilename, _graphics, _logic, _resource);
		}

		_logic->currentRoom(73);
		// XXX _entryObj = 584;

		_logic->roomDisplay(_logic->roomName(_logic->currentRoom()), RDM_FADE_JOE, 100, 2, true);
		Cutaway::run("c70d.cut", nextFilename, _graphics, _logic, _resource);

		_logic->gameState(117, 1);

		// XXX setupItems();
		// XXX inventory();
	}
	else {
		_logic->roomDisplay(_logic->roomName(_logic->currentRoom()), RDM_FADE_JOE, 100, 1, false);
	}
	// XXX _drawMouseFlag = 1;
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
			_graphics->update();
			_logic->oldRoom(_logic->currentRoom());
			_logic->currentRoom(_logic->newRoom());
			roomChanged();
			// XXX _logic->fullScreen(false);
			if (_logic->currentRoom() == _logic->newRoom())
				_logic->newRoom(0);
		}
		else {
			if (_logic->joeWalk() == 2) {
				_logic->joeWalk(0);
				// XXX executeAction(yes);
			}
			else {
				// XXX if (_parse == 1)
				// XXX 	clearCommand(1);
				_logic->joeWalk(0);
				// XXX checkPlayer();
			}
		}

		break; // XXX don't loop yet
	}

	while (1) { //main loop
		delay(1000);
	}
}

void QueenEngine::initialise(void) {
	_resource = new Resource(_gameDataPath);
	_display = new Display(_system);
	_graphics = new Graphics(_display, _resource);
	_logic = new Logic(_resource, _graphics, _display);
	//_sound = new Sound(_mixer, _detector->_sfx_volume);
}

void QueenEngine::delay(uint amount) { 

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
					_sdl_mouse_x = event.mouse.x;
					_sdl_mouse_y = event.mouse.y;
					
					break;

				case OSystem::EVENT_LBUTTONDOWN:
#ifdef _WIN32_WCE
					_sdl_mouse_x = event.mouse.x;
					_sdl_mouse_y = event.mouse.y;
#endif
					break;

				case OSystem::EVENT_RBUTTONDOWN:
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
			if (this_delay > amount)
				this_delay = amount;
			_system->delay_msecs(this_delay);
		}
		cur = _system->get_msecs();
	} while (cur < start + amount);
}

} // End of namespace Queen
