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

#include "crab/people/person.h"

namespace Crab {

using namespace pyrodactyl::stat;
using namespace pyrodactyl::people;

Person::Person() {
	type = PE_NEUTRAL;
	state = PST_NORMAL;
	alt_journal_name = false;
	trig.clear();
}

void Person::Load(rapidxml::xml_node<char> *node, const pyrodactyl::stat::StatTemplates &stem) {
	if (nodeValid(node)) {
		loadStr(id, "id", node);
		loadStr(name, "name", node);
		// loadImgKey(pic, "img", node);

		if (nodeValid("opinion", node))
			opinion.Load(node->first_node("opinion"));

		if (node->first_attribute("type") != NULL) {
			Common::String t;
			loadStr(t, "type", node);
			type = StringToPersonType(t);
		} else
			type = PE_NEUTRAL;

		if (node->first_attribute("state") != NULL) {
			Common::String s;
			loadStr(s, "state", node);
			state = StringToPersonState(s);
		} else
			state = PST_NORMAL;

		if (node->first_attribute("journal_name") != NULL) {
			loadStr(journal_name, "journal_name", node);
			alt_journal_name = true;
		} else
			alt_journal_name = false;

		if (nodeValid("stats", node)) {
			rapidxml::xml_node<char> *statnode = node->first_node("stats");
			if (statnode->first_attribute("template") == NULL) {
				stat.Load(statnode);
			} else {
				int index = 0;
				loadNum(index, "template", statnode);
				if (index >= 0 && (unsigned int)index < stem.collection.size())
					for (int i = 0; i < STAT_TOTAL; i++)
						stat.val[i] = stem.collection[index].val[i];
			}
		}

		if (nodeValid("traits", node, false)) {
			rapidxml::xml_node<char> *traitnode = node->first_node("traits");
			for (auto n = traitnode->first_node("trait"); n != NULL; n = n->next_sibling("trait"))
				trait.push_back(n);
		}
	}
}

void Person::Reset() {
	for (auto i = 0; i < STAT_TOTAL; ++i)
		stat.val[i].Reset();
}

void Person::Validate() {
	for (int i = 0; i < STAT_TOTAL; ++i)
		stat.val[i].Validate();
}

void Person::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "object");
	child->append_attribute(doc.allocate_attribute("id", id.c_str()));
	child->append_attribute(doc.allocate_attribute("name", name.c_str()));

	unsigned int val = static_cast<unsigned int>(state);
	child->append_attribute(doc.allocate_attribute("state", gStrPool->Get(val)));

	opinion.SaveState(doc, child);

	rapidxml::xml_node<char> *child_s = doc.allocate_node(rapidxml::node_element, "stats");
	stat.val[STAT_HEALTH].SaveState(doc, child_s, STATNAME_HEALTH);
	stat.val[STAT_ATTACK].SaveState(doc, child_s, STATNAME_ATTACK);
	stat.val[STAT_DEFENSE].SaveState(doc, child_s, STATNAME_DEFENSE);
	stat.val[STAT_SPEED].SaveState(doc, child_s, STATNAME_SPEED);
	/*stat.val[STAT_CHARISMA].SaveState(doc, child_s, STATNAME_CHARISMA);
	stat.val[STAT_INTELLIGENCE].SaveState(doc, child_s, STATNAME_INTELLIGENCE);*/
	child->append_node(child_s);

	rapidxml::xml_node<char> *child_t = doc.allocate_node(rapidxml::node_element, "traits");
	for (auto &i : trait)
		i.SaveState(doc, child_t, "trait");
	child->append_node(child_t);

	root->append_node(child);
}

void Person::LoadState(rapidxml::xml_node<char> *node) {
	loadStr(id, "id", node);
	loadStr(name, "name", node);
	loadEnum(state, "state", node);

	if (nodeValid("opinion", node))
		opinion.Load(node->first_node("opinion"));

	if (nodeValid("stats", node))
		stat.Load(node->first_node("stats"));

	if (nodeValid("traits", node, false)) {
		rapidxml::xml_node<char> *traitnode = node->first_node("traits");

		trait.clear();
		for (auto n = traitnode->first_node("trait"); n != NULL; n = n->next_sibling("trait"))
			trait.push_back(n);
	}
}

} // End of namespace Crab
