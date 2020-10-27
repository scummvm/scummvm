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
 * Copyright 2020 Google
 *
 */
#ifndef HADESCH_EVENT_H
#define HADESCH_EVENT_H

namespace Hadesch {

class EventHandler {
public:
	virtual void operator()() = 0;
	virtual ~EventHandler() {}
};

class EventHandlerWrapper {
public:
	void operator()() const;
	bool operator==(int eventId) const;

	EventHandlerWrapper(int eventId) {
		_eventId = eventId;
	}

	EventHandlerWrapper() {
		_eventId = -1;
	}

	EventHandlerWrapper(Common::SharedPtr<EventHandler> handler) {
		_handler = handler;
		_eventId = -1;
	}

	Common::String getDebugString() const {
		return Common::String::format("eventid=%d, handler is %s", _eventId,
					      !!_handler ? "valid" : "null");
	}

private:
	Common::SharedPtr<EventHandler> _handler;
	int _eventId;
};
}

#endif
