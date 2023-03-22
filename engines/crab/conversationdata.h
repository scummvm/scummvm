#pragma once

#include "changeval.h"
#include "common_header.h"
#include "triggerset.h"

namespace pyrodactyl {
namespace event {
struct ReplyChoice {
	// The text for the reply
	std::string text;

	// The tone of the response
	unsigned int tone;

	// The next event id
	unsigned int nextid;

	// The conditions to unlock this choice
	TriggerSet unlock;

	// The changes to opinion that are possible for this reply
	// All NPCs affected by this conversation
	std::vector<ChangeVal> change;

	ReplyChoice() {
		tone = 0;
		nextid = 0;
	}
	ReplyChoice(rapidxml::xml_node<char> *node) : ReplyChoice() { Load(node); }

	void Load(rapidxml::xml_node<char> *node);
};

struct ConversationData {
	// The set of replies
	std::vector<ReplyChoice> reply;

	ConversationData() {}

	void Load(rapidxml::xml_node<char> *node);
};
} // End of namespace event
} // End of namespace pyrodactyl