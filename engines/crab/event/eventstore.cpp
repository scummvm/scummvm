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

#include "crab/event/eventstore.h"

namespace Crab {

using namespace pyrodactyl::event;

void GameEventStore::addConv(rapidxml::xml_node<char> *node, uint &index) {
	ConversationData c;
	if (nodeValid("talk", node))
		c.load(node->first_node("talk"));

	index = _con.size();
	_con.push_back(c);
}

void GameEventStore::load(const Common::String &filename) {

	// Request current user stats from Steam
	// m_pSteamUserStats = SteamUserStats();

	// if (m_pSteamUserStats != nullptr)
	// m_pSteamUserStats->RequestCurrentStats();

	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("store");

		if (nodeValid("animations", node)) {
			rapidxml::xml_node<char> *animnode = node->first_node("animations");
			for (auto n = animnode->first_node("anim"); n != nullptr; n = n->next_sibling("anim"))
				_anim.push_back(n);
		}

		if (nodeValid("tones", node)) {
			rapidxml::xml_node<char> *tonenode = node->first_node("tones");
			for (auto n = tonenode->first_node("tone"); n != nullptr; n = n->next_sibling("tone")) {
				ToneData dat;
				loadStr(dat._text, "text", n);
				_tone.push_back(dat);
			}
		}

		if (nodeValid("images", node)) {
			rapidxml::xml_node<char> *imgnode = node->first_node("images");
			for (auto n = imgnode->first_node("img"); n != nullptr; n = n->next_sibling("img"))
				_img.push_back(n);
		}

		if (nodeValid("traits", node)) {
			rapidxml::xml_node<char> *traitnode = node->first_node("traits");
			for (auto n = traitnode->first_node("trait"); n != nullptr; n = n->next_sibling("trait"))
				_trait.push_back(n);
		}
	}
}

void GameEventStore::setAchievement(const int &id) {
	/*
	if (m_pSteamUserStats != nullptr)
	{
		if (id == 0)
			m_pSteamUserStats->SetAchievement("a0");
		else
			m_pSteamUserStats->SetAchievement(trait[id].id_str.c_str());

		m_pSteamUserStats->StoreStats();
	}
	*/
}

} // End of namespace Crab
