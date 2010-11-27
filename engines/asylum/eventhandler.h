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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_EVENTHANDLER_H
#define ASYLUM_EVENTHANDLER_H

#include "common/func.h"
#include "common/events.h"

namespace Asylum {

enum AsylumEventType {
	EVENT_ASYLUM_INIT   = 0xFFF0,
	EVENT_ASYLUM_DEINIT = 0xFFF1,
	EVENT_ASYLUM_UPDATE = 0xFFF2
};

struct AsylumEvent : public Common::Event {
	// Since we don't feed any custom message into the event manager,
	// we can safely use our own custom event type.
	AsylumEvent(AsylumEventType msgType) {
		type = (Common::EventType)msgType;
	}
};

typedef Common::Functor1<const AsylumEvent &, void> MessageHandler;

#define MESSAGE_HANDLER(class, name, inst) \
	Common::Functor1Mem<const AsylumEvent&, void, class>(inst, &class::name)

} // End of namespace Asylum

#endif // ASYLUM_EVENTHANDLER_H
