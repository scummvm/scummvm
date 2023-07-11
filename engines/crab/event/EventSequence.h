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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_EVENTSEQUENCE_H
#define CRAB_EVENTSEQUENCE_H
#include "crab/common_header.h"

#include "crab/event/GameEventInfo.h"
#include "crab/image/ImageManager.h"
#include "crab/music/MusicManager.h"
//#include "crab/ui/Inventory.h"
#include "crab/event/gameevent.h"
//#include "crab/ui/journal.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
class EventSequence {
	Common::Array<GameEvent> _events;
	bool _eventInProgress;

	// The event currently in execution - updated only when all trigger conditions are met in InternalEvents
	unsigned int _cur;

	// The events that can happen next - these are updated when the cur event is over
	// This means cur and next operate in an alternating way
	// scan next until find event, make it cur, end cur and update next, repeat
	Common::Array<unsigned int> _next;

public:
	EventSequence() {
		_eventInProgress = false;
		_next.push_back(0);
		_cur = 0;
	}

	~EventSequence() {}

	GameEvent *currentEvent() {
		 return &_events[_cur];
	}

	// See if we should trigger any event
	void internalEvents(pyrodactyl::event::Info &info);
	void nextEvent(Info &info, const Common::String &playerId, Common::Array<EventResult> &result,
				   Common::Array<EventSeqInfo> &endSeq, int nextEventChoice = -1);

	bool eventInProgress() {
		return _eventInProgress;
	}

	void eventInProgress(bool val) {
		_eventInProgress = val;
	}

	// Load and save
	void load(const Common::String &filename);

	void saveState(rapidxml::xml_document<char> &doc, rapidxml::xml_node<char> *root, const char *name);
	void loadState(rapidxml::xml_node<char> *node);
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_EVENTSEQUENCE_H
