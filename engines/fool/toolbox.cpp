/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/events.h"
#include "common/system.h"

#include "fool/fool.h"
#include "fool/toolbox.h"

namespace Fool {

Toolbox::Toolbox() {
	_frameLimiter = new Graphics::FrameLimiter(g_system, 60);
}

Toolbox::~Toolbox() {
	delete _frameLimiter;
}

void Toolbox::_pumpEvents() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		EventRecord newRecord;
		newRecord.when = this->TickCount();
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			newRecord.what = kMouseDown;
			newRecord.where = Common::Point(event.mouse.x, event.mouse.y);
			_events.push(newRecord);
			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			newRecord.what = kMouseUp;
			newRecord.where = Common::Point(event.mouse.x, event.mouse.y);
			_events.push(newRecord);
			break;
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
		default:
			break;
		}
	}
}

void Toolbox::_updateScreen() {
	_defaultWindow->getWindowSurface()->copyFrom(*_defaultBits);
	g_engine->_wm.draw();
	_frameLimiter->delayBeforeSwap();
	g_system->updateScreen();
	_frameLimiter->startFrame();
}

uint32 Toolbox::Delay(uint32 numTicks) {
	uint32 target = g_system->getMillis() + (numTicks * 1000 / 60);
	do  {
		_pumpEvents();
		_updateScreen();
	} while (g_system->getMillis() < target);
	return (uint32)(g_system->getMillis() * 60 / 1000);
}

bool Toolbox::GetNextEvent(uint16 eventMask, EventRecord &theEvent) {
	warning("STUB: Toolbox::GetNextEvent");
	_pumpEvents();
	if (!_events.empty()) {
		theEvent = _events.pop();
	} else {
		theEvent.what = kNullEvent;
		// pretend mouse button is up
		theEvent.modifiers = 0x80;
	}
	return true;
}

uint32 Toolbox::TickCount() {
	return (uint32)(g_system->getMillis() * 60 / 1000);
}


} // namespace Fool
