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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_EVENTHANDLER_H
#define ASYLUM_EVENTHANDLER_H

#include "common/func.h"
#include "common/events.h"

namespace Asylum {

enum AsylumEventType {
	EVENT_ASYLUM_MIDI     = 953,
	EVENT_ASYLUM_CHANGECD = 5120,
	EVENT_ASYLUM_INIT     = 5122,
	EVENT_ASYLUM_UPDATE   = 5121,
	EVENT_ASYLUM_DEINIT   = 5123,
	EVENT_ASYLUM_CURSOR   = 5124,
	EVENT_ASYLUM_SUBTITLE = 5125,
	EVENT_ASYLUM_MUSIC    = 5376,

	EVENT_ASYLUM_ACTIVATE = 0xFFF0
};

struct AsylumEvent : public Common::Event {
	AsylumEvent() : Event() {
		param1 = 0;
		param2 = 0;
	}

	// Since we don't feed any custom message into the event manager,
	// we can safely use our own custom event type.
	AsylumEvent(AsylumEventType msgType, int32 p1 = 0, int32 p2 = 0) : Event() {
		type = (Common::EventType)msgType;
		param1 = p1;
		param2 = p2;
	}

	int32 param1;
	int32 param2;
};

class EventHandler {
public:
	virtual ~EventHandler() {}

	virtual bool handleEvent(const AsylumEvent &ev) = 0;
};

} // End of namespace Asylum

#endif // ASYLUM_EVENTHANDLER_H
