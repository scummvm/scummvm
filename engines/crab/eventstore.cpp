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

#include "eventstore.h"

namespace pyrodactyl {
namespace event {
GameEventStore gEventStore;
}
} // End of namespace pyrodactyl

using namespace pyrodactyl::event;

void GameEventStore::AddConv(rapidxml::xml_node<char> *node, unsigned int &index) {
	ConversationData c;
	if (NodeValid("talk", node))
		c.Load(node->first_node("talk"));

	index = con.size();
	con.push_back(c);
}

void GameEventStore::Load(const std::string &filename) {
	// Request current user stats from Steam
	// m_pSteamUserStats = SteamUserStats();

	// if (m_pSteamUserStats != nullptr)
	// m_pSteamUserStats->RequestCurrentStats();

	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("store");

		if (NodeValid("animations", node)) {
			rapidxml::xml_node<char> *animnode = node->first_node("animations");
			for (auto n = animnode->first_node("anim"); n != NULL; n = n->next_sibling("anim"))
				anim.push_back(n);
		}

		if (NodeValid("tones", node)) {
			rapidxml::xml_node<char> *tonenode = node->first_node("tones");
			for (auto n = tonenode->first_node("tone"); n != NULL; n = n->next_sibling("tone")) {
				ToneData dat;
				LoadStr(dat.text, "text", n);
				tone.push_back(dat);
			}
		}

		if (NodeValid("images", node)) {
			rapidxml::xml_node<char> *imgnode = node->first_node("images");
			for (auto n = imgnode->first_node("img"); n != NULL; n = n->next_sibling("img"))
				img.push_back(n);
		}

		if (NodeValid("traits", node)) {
			rapidxml::xml_node<char> *traitnode = node->first_node("traits");
			for (auto n = traitnode->first_node("trait"); n != NULL; n = n->next_sibling("trait"))
				trait.push_back(n);
		}
	}
}

void GameEventStore::SetAchievement(const int &id) {
	/*
	if (m_pSteamUserStats != nullptr)
	{
		if (id == 0)
			m_pSteamUserStats->SetAchievement("a0");
		else
			m_pSteamUserStats->SetAchievement(trait.at(id).id_str.c_str());

		m_pSteamUserStats->StoreStats();
	}
	*/
}
