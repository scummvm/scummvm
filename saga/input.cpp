/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "saga/script.h"
#include "saga/isomap.h"

#include "common/system.h"

namespace Saga {

int SagaEngine::processInput() {
	OSystem::Event event;

//	Point imousePt;

	while (g_system->pollEvent(event)) {
		switch (event.type) {
		case OSystem::EVENT_KEYDOWN:
			if (event.kbd.flags == OSystem::KBD_CTRL) {
				if (event.kbd.keycode == 'd')
					_console->attach();
			}
			if (_interface->_textInput || _interface->_statusTextInput) {
				_interface->processAscii(event.kbd.ascii);
				return SUCCESS;
			}

			switch (event.kbd.keycode) {
			case '#':
			case '`':
			case '~':
				_console->attach();
				break;
			case 'r':
				_interface->draw();
				break;

#if 0
			case 269:
			case 270:
			case 273:
			case 274:
			case 275:
			case 276:
				if (_vm->_scene->getFlags() & kSceneFlagISO) {
					_vm->_isoMap->_viewDiff += (event.kbd.keycode == 270) - (event.kbd.keycode == 269);
					_vm->_isoMap->_viewScroll.y += (_vm->_isoMap->_viewDiff * (event.kbd.keycode == 274) - _vm->_isoMap->_viewDiff * (event.kbd.keycode == 273));
					_vm->_isoMap->_viewScroll.x += (_vm->_isoMap->_viewDiff * (event.kbd.keycode == 275) - _vm->_isoMap->_viewDiff * (event.kbd.keycode == 276));
				}
				break;
#endif
			case 282: // F1
				_render->toggleFlag(RF_SHOW_FPS);
				_actor->_handleActionDiv = (_actor->_handleActionDiv == 15) ? 50 : 15;
				break;
			case 283: // F2
				_render->toggleFlag(RF_PALETTE_TEST);
				break;
			case 284: // F3
				_render->toggleFlag(RF_TEXT_TEST);
				break;
			case 285: // F4
				_render->toggleFlag(RF_OBJECTMAP_TEST);
				break;
			case 286: // F5
				if (_interface->getSaveReminderState() > 0)
					_interface->setMode(kPanelOption);
				break;
			case 287: // F6
				_render->toggleFlag(RF_ACTOR_PATH_TEST);
				break;
			case 288: // F7
				break;
			case 289: // F8
				break;
			case 290: // F9
				_interface->keyBoss();
				break;

			// Actual game keys
			case 32: // space
				_actor->abortSpeech();
				break;
			case 19:  // pause
			case 'z':
				_render->toggleFlag(RF_RENDERPAUSE);
				break;
			default:
				_interface->processAscii(event.kbd.ascii);
				break;
			}
			break;
		case OSystem::EVENT_KEYUP:
			_interface->processKeyUp(event.kbd.ascii);
			break;
		case OSystem::EVENT_LBUTTONUP:
			_leftMouseButtonPressed = false;
			break;
		case OSystem::EVENT_RBUTTONUP:
			_rightMouseButtonPressed = false;
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			_leftMouseButtonPressed = true;
			_mousePos = event.mouse;
			_interface->update(_mousePos, UPDATE_LEFTBUTTONCLICK);
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			_rightMouseButtonPressed = true;
			_mousePos = event.mouse;
			_interface->update(_mousePos, UPDATE_RIGHTBUTTONCLICK);
			break;
		case OSystem::EVENT_WHEELUP:
			_interface->update(_mousePos, UPDATE_WHEELUP);
			break;
		case OSystem::EVENT_WHEELDOWN:
			_interface->update(_mousePos, UPDATE_WHEELDOWN);
			break;
		case OSystem::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
		case OSystem::EVENT_QUIT:
			shutDown();
			break;
		default:
			break;
		}
	}

	return SUCCESS;
}


} // End of namespace Saga

