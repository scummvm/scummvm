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
