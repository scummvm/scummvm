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

#ifndef CRAB_GAMEEVENT_H
#define CRAB_GAMEEVENT_H

#include "crab/image/ImageManager.h"
#include "crab/event/effect.h"
#include "crab/event/eventstore.h"
#include "crab/people/person.h"
#include "crab/people/personbase.h"
#include "crab/event/triggerset.h"

namespace Crab {

// An uinteger is our event id format
typedef uint EventID;

// Just map loading function to number load
#define LoadEventID loadNum

namespace pyrodactyl {
namespace event {
enum EventType {
	EVENT_DIALOG,
	EVENT_REPLY,
	EVENT_TEXT,
	EVENT_ANIM,
	EVENT_SILENT,
	EVENT_SPLASH
};

struct GameEvent {
	// The event identifier
	EventID _id;

	// The heading for the dialog spoken
	Common::String _title;

	// The dialog spoken in the event
	Common::String _dialog;

	// The state decides which animation is drawn in the dialog box
	pyrodactyl::people::PersonState _state;

	// Event type and related data index
	EventType _type;
	uint _special;

	// The variables changed/added in the event
	Common::Array<Effect> _effect;

	// The triggers for the event
	TriggerSet _trig;

	// The id of the next event
	Common::Array<EventID> _next;

	GameEvent();

	GameEvent(rapidxml::xml_node<char> *node) : GameEvent() {
		load(node);
	}

	~GameEvent() {}

	void load(rapidxml::xml_node<char> *node);
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_GAMEEVENT_H
