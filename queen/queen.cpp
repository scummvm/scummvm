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
#include "common/config-manager.h"
#include "common/file.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "queen/graphics.h"

extern uint16 _debugLevel;

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;

#endif

static const TargetSettings queen_settings[] = {
	/* Flight of the Amazon Queen */
	{ "queen", "Flight of the Amazon Queen", GID_QUEEN_FIRST, 99, MDT_ADLIB | MDT_NATIVE | MDT_PREFER_NATIVE, 0, "queen.1" },
	{ NULL, NULL, 0, 0, MDT_NONE, 0, NULL} 
};

const TargetSettings *Engine_QUEEN_targetList() {
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
	
	_debugMode = detector->_debugMode;
	_debugLevel = ConfMan.getInt("debuglevel");
	_detector = detector;

	_fastMode = 0;

	_system->init_size(320, 200);
}

QueenEngine::~QueenEngine() {
	delete _graphics;
	delete _logic;
	delete _resource;
	//delete _queenDisplay;
}

void QueenEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

void QueenEngine::go() {

	if (!_dump_file)
		_dump_file = stdout;

	initialise();
	
	while (1) { //main loop
		delay(1000);
	}
}

void QueenEngine::initialise(void) {
	_resource = new Resource(_gameDataPath);
	_graphics = new Graphics(_resource);
	_logic = new Logic(_resource, _graphics);
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
