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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/events.h"
#include "engines/util.h"
#include "mads/mads.h"
#include "mads/events.h"

namespace MADS {

EventsManager::EventsManager(MADSEngine *vm) {
	_vm = vm;
	_cursorSprites = nullptr;
}

EventsManager::~EventsManager() {
	delete _cursorSprites;
}

void EventsManager::loadCursors(const Common::String &spritesName) {
	error("TODO: load SpriteSet");
}

void EventsManager::setCursor(CursorType cursorId) {
	_cursorId = cursorId;
	_newCursorId = cursorId;
	changeCursor();
}

void EventsManager::changeCursor() {

}

void EventsManager::handleEvents() {
	Common::Event e;
	while (!_vm->shouldQuit()) {
		g_system->getEventManager()->pollEvent(e);
		g_system->delayMillis(10);
	}
}

} // End of namespace MADS
