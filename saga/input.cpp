/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
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
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/actor.h"
#include "saga/console.h"
#include "saga/interface.h"
#include "saga/render.h"
#include "saga/scene.h"
#include "saga/script_mod.h"

namespace Saga {

static R_POINT _mousePos;

int SYSINPUT_ProcessInput() {
	OSystem::Event event;

	R_POINT imouse_pt;

	while (g_system->poll_event(&event)) {
		int in_char;

		switch (event.event_code) {
		case OSystem::EVENT_KEYDOWN:
			if (_vm->_console->isActive()) {
				in_char = event.kbd.ascii;
				switch (event.kbd.keycode) {
				case 96: // backquote
					_vm->_console->deactivate();
					break;
				case 280: // page up
					_vm->_console->pageUp();
					break;
				case 281: // page down
					_vm->_console->pageDown();
					break;
				case 273: // up
				case 264: // keypad up
					_vm->_console->cmdUp();
					break;
				case 274: // down
				case 258: // keypad down
					_vm->_console->cmdDown();
					break;
				default:
					if (in_char) {
						_vm->_console->type(in_char);
					}
					break;
				}
				break;
			}

			switch (event.kbd.keycode) {
			case 96: // back quote
				_vm->_console->activate();
				break;
			case 114: // r
				_vm->_interface->draw();
				break;
			case 282: // F1
				_vm->_render->toggleFlag(RF_SHOW_FPS);
				break;
			case 283: // F2
				_vm->_render->toggleFlag(RF_PALETTE_TEST);
				break;
			case 284: // F3
				_vm->_render->toggleFlag(RF_TEXT_TEST);
				break;
			case 285: // F4
				_vm->_render->toggleFlag(RF_OBJECTMAP_TEST);
				break;
			case 9: // Tab
				STHREAD_DebugStep();
				break;

			// Actual game keys
			case 32: // space
				_vm->_actor->skipDialogue();
				break;
			case 19:  // pause
			case 112: // p
				_vm->_render->toggleFlag(RF_RENDERPAUSE);
				break;
			case 27: // Esc
				// Skip to next scene skip target
				_vm->_scene->skipScene();
				break;
			default:
				break;
			}
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			_vm->_interface->update(&imouse_pt, UPDATE_MOUSECLICK);
			break;
		case OSystem::EVENT_MOUSEMOVE:
			_mousePos.x = event.mouse.x;
			_mousePos.y = event.mouse.y;
			imouse_pt = _mousePos;
			break;
		case OSystem::EVENT_QUIT:
			g_system->quit();
			break;
		default:
			break;
		}
	}

	return R_SUCCESS;
}

R_POINT SYSINPUT_GetMousePos() {
	return _mousePos;
}

} // End of namespace Saga

