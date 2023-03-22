#pragma once

#include "ImageManager.h"
#include "common_header.h"
#include "effect.h"
#include "eventstore.h"
#include "person.h"
#include "personbase.h"
#include "triggerset.h"

// An unsigned integer is our event id format
typedef unsigned int EventID;

// Just map loading function to number load
#define LoadEventID LoadNum

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
	EventID id;

	// The heading for the dialog spoken
	std::string title;

	// The dialog spoken in the event
	std::string dialog;

	// The state decides which animation is drawn in the dialog box
	pyrodactyl::people::PersonState state;

	// Event type and related data index
	EventType type;
	unsigned int special;

	// The variables changed/added in the event
	std::vector<Effect> effect;

	// The triggers for the event
	TriggerSet trig;

	// The id of the next event
	std::vector<EventID> next;

	GameEvent();
	GameEvent(rapidxml::xml_node<char> *node) { Load(node); }
	~GameEvent() {}

	void Load(rapidxml::xml_node<char> *node);
};
} // End of namespace event
} // End of namespace pyrodactyl