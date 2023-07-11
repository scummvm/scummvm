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

#ifndef CRAB_CONVERSATIONDATA_H
#define CRAB_CONVERSATIONDATA_H

#include "crab/event/changeval.h"
#include "crab/common_header.h"
#include "crab/event/triggerset.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
struct ReplyChoice {
	// The text for the reply
	Common::String _text;

	// The tone of the response
	unsigned int _tone;

	// The next event id
	unsigned int _nextid;

	// The conditions to unlock this choice
	TriggerSet _unlock;

	// The changes to opinion that are possible for this reply
	// All NPCs affected by this conversation
	Common::Array<ChangeVal> _change;

	ReplyChoice() {
		_tone = 0;
		_nextid = 0;
	}

	ReplyChoice(rapidxml::xml_node<char> *node) : ReplyChoice() {
		load(node);
	}

	void load(rapidxml::xml_node<char> *node);
};

struct ConversationData {
	// The set of replies
	Common::Array<ReplyChoice> _reply;

	ConversationData() {}

	void load(rapidxml::xml_node<char> *node);
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_CONVERSATIONDATA_H
