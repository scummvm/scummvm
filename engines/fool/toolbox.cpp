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
	Common::EventManager *manager = g_system->getEventManager();
	while (manager->pollEvent(event)) {
		EventRecord newRecord;
		newRecord.when = this->TickCount();
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_mouse = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			_mouse = event.mouse;
			_modifiers &= ~kModMouseButtonUp;
			newRecord.what = kMouseDown;
			newRecord.where = _mouse;
			newRecord.modifiers = _modifiers;
			_events.push_back(newRecord);
			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_mouse = event.mouse;
			_modifiers |= kModMouseButtonUp;
			newRecord.what = kMouseUp;
			newRecord.where = _mouse;
			newRecord.modifiers = _modifiers;
			_events.push_back(newRecord);
			break;
		case Common::EVENT_QUIT:
			manager->resetQuit();
			newRecord.what = kScummVMQuitEvt;
			newRecord.where = _mouse;
			newRecord.modifiers = _modifiers;
			_events.push_back(newRecord);
			break;
		case Common::EVENT_RETURN_TO_LAUNCHER:
			manager->resetReturnToLauncher();
			newRecord.what = kScummVMReturnToLauncherEvt;
			newRecord.where = _mouse;
			newRecord.modifiers = _modifiers;
			_events.push_back(newRecord);
			break;
		default:
			break;
		}
	}
	// erase the older events if they aren't consumed in time
	while (_events.size() > 2048) {
		_events.pop_front();
	}
}

void Toolbox::_updateScreen() {
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

void Toolbox::FlushEvents(uint32 eventMask, uint32 stopMask) {
	if ((eventMask == 0xffffffff) && (stopMask == 0)) {
		_events.clear();
		//_modifiers = kModMouseButtonUp;
		return;
	}
	for (auto it = _events.begin(); it != _events.end(); ) {
		if ((1 << it->what) & stopMask) {
			return;
		} else if ((1 << it->what) & eventMask) {
			it = _events.erase(it);
		} else {
			++it;
		}
	}
	//if (eventMask & 0x6) {
	//	  _modifiers = kModMouseButtonUp;
	//}
}

bool Toolbox::GetNextEvent(uint32 eventMask, EventRecord &theEvent) {
	//warning("STUB: Toolbox::GetNextEvent");
	_pumpEvents();
	if (!_events.empty() && eventMask) {
		for (auto it = _events.begin(); it != _events.end(); ++it) {
			if ((1 << it->what) & eventMask) {
				theEvent = Common::move(*it);
				_events.erase(it);
				return true;
			}
		}
	}
	theEvent.what = kNullEvent;
	theEvent.where = _mouse;
	theEvent.modifiers = _modifiers;
	return false;
}

uint32 Toolbox::TickCount() {
	return (uint32)(g_system->getMillis() * 60 / 1000);
}


} // namespace Fool
