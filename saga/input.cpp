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
#include "saga/script.h"

namespace Saga {

int SagaEngine::processInput() {
	OSystem::Event event;

	Point imousePt;

	while (g_system->pollEvent(event)) {
		switch (event.event_code) {
		case OSystem::EVENT_KEYDOWN:
			if (event.kbd.flags == OSystem::KBD_CTRL) {
				if (event.kbd.keycode == 'd')
					_console->attach();
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
			case 282: // F1
				_render->toggleFlag(RF_SHOW_FPS);
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
			case 9: // Tab
				_script->SThreadDebugStep();
				break;

			// Actual game keys
			case 32: // space
				_actor->skipDialogue();
				break;
			case 19:  // pause
			case 'p':
				_render->toggleFlag(RF_RENDERPAUSE);
				break;
			case 27: // Esc
				// Skip to next scene skip target
				if (!_interface->getMode() == kPanelNone) // FIXME: hack
					_script->SThreadAbortAll();
				else
					_scene->skipScene();
				break;
			default:
				break;
			}
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			_mousePos.x = event.mouse.x;
			_mousePos.y = event.mouse.y;
			imousePt = _mousePos;
			_interface->update(imousePt, UPDATE_MOUSECLICK);
			break;
		case OSystem::EVENT_MOUSEMOVE:
			_mousePos.x = event.mouse.x;
			_mousePos.y = event.mouse.y;
			imousePt = _mousePos;
			break;
		case OSystem::EVENT_QUIT:
			g_system->quit();
			break;
		default:
			break;
		}
	}

	return SUCCESS;
}

Point SagaEngine::getMousePos() {
	return _mousePos;
}

} // End of namespace Saga

