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

#include "conversationdata.h"

using namespace pyrodactyl::event;

void ReplyChoice::Load(rapidxml::xml_node<char> *node) {
	LoadStr(text, "text", node);
	LoadNum(tone, "tone", node);
	LoadNum(nextid, "next", node);

	if (NodeValid("unlock", node, false))
		unlock.Load(node->first_node("unlock"));

	if (NodeValid("change", node, false))
		for (auto n = node->first_node("change"); n != NULL; n = n->next_sibling("change"))
			change.push_back(n);
}

void ConversationData::Load(rapidxml::xml_node<char> *node) {
	for (auto n = node->first_node("reply"); n != NULL; n = n->next_sibling("reply"))
		reply.push_back(n);
}
