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

#include "crab/event/conversationdata.h"

namespace Crab {

using namespace pyrodactyl::event;

void ReplyChoice::load(rapidxml::xml_node<char> *node) {
	loadStr(text, "text", node);
	loadNum(tone, "tone", node);
	loadNum(nextid, "next", node);

	if (nodeValid("unlock", node, false))
		unlock.load(node->first_node("unlock"));

	if (nodeValid("change", node, false))
		for (auto n = node->first_node("change"); n != NULL; n = n->next_sibling("change"))
			change.push_back(n);
}

void ConversationData::load(rapidxml::xml_node<char> *node) {
	for (auto n = node->first_node("reply"); n != NULL; n = n->next_sibling("reply"))
		reply.push_back(n);
}

} // End of namespace Crab
