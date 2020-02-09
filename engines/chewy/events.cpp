/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/events.h"

#include "chewy/chewy.h"
#include "chewy/console.h"
#include "chewy/cursor.h"
#include "chewy/events.h"
#include "chewy/graphics.h"
#include "chewy/scene.h"

namespace Chewy {

Events::Events(ChewyEngine *vm) : _vm(vm) {

	_eventManager = g_system->getEventManager();
}

void Events::processEvents() {
	while (_eventManager->pollEvent(_event)) {
		if (_event.type == Common::EVENT_KEYDOWN) {
			switch (_event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				_vm->quitGame();
				break;
			case Common::KEYCODE_SPACE:
				_vm->_cursor->nextCursor();
				break;
			default:
				break;
			}
		} else if (_event.type == Common::EVENT_LBUTTONUP) {
			_vm->_scene->mouseClick(_event.mouse);
		} else if (_event.type == Common::EVENT_RBUTTONUP) {
			_vm->_cursor->nextCursor();
		} else if (_event.type == Common::EVENT_MOUSEMOVE) {
			_vm->_scene->updateMouse(_event.mouse);
		}
	}
}

} // End of namespace Chewy
