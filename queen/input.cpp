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
#include "queen/input.h"
#include "common/system.h"
#include "common/util.h"

namespace Queen {


Input::Input(OSystem *system) : 
	_system(system), _fastMode(false), _keyVerb(VERB_NONE), 
	_cutawayRunning(false), _cutQuit(false), _talkQuit(false) {
}

void Input::delay() {
	delay(_fastMode ? DELAY_SHORT : DELAY_NORMAL);
}

void Input::delay(uint amount) { 

	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;
	_key_pressed = 0;	//reset

	do {
		while (_system->poll_event(&event)) {
			switch (event.event_code) {
				case OSystem::EVENT_KEYDOWN:
#if 0
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
#endif

					debug(1, "event.kbd.keycode = %i (%c)", 
							event.kbd.keycode,
							isprint(event.kbd.keycode) ? event.kbd.keycode : '.');

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

void Input::checkKeys() {
}

  
} // End of namespace Queen

