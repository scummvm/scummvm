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

#include "crab/crab.h"
#include "crab/people/person.h"

namespace Crab {

using namespace pyrodactyl::stat;
using namespace pyrodactyl::people;

Person::Person() {
	_type = PE_NEUTRAL;
	_state = PST_NORMAL;
	_altJournalName = false;
	_trig.clear();
}

void Person::load(rapidxml::xml_node<char> *node, const pyrodactyl::stat::StatTemplates &stem) {
	if (nodeValid(node)) {
		loadStr(_id, "id", node);
		loadStr(_name, "name", node);
		// loadImgKey(pic, "img", node);

		if (nodeValid("opinion", node))
			_opinion.load(node->first_node("opinion"));

		if (node->first_attribute("type") != nullptr) {
			Common::String t;
			loadStr(t, "type", node);
			_type = stringToPersonType(t);
		} else
			_type = PE_NEUTRAL;

		if (node->first_attribute("state") != nullptr) {
			Common::String s;
			loadStr(s, "state", node);
			_state = stringToPersonState(s);
		} else
			_state = PST_NORMAL;

		if (node->first_attribute("journal_name") != nullptr) {
			loadStr(_journalName, "journal_name", node);
			_altJournalName = true;
		} else
			_altJournalName = false;

		if (nodeValid("stats", node)) {
			rapidxml::xml_node<char> *statnode = node->first_node("stats");
			if (statnode->first_attribute("template") == nullptr) {
				_stat.load(statnode);
			} else {
				int index = 0;
				loadNum(index, "template", statnode);
				if (index >= 0 && (uint)index < stem._collection.size())
					for (int i = 0; i < STAT_TOTAL; i++)
						_stat._val[i] = stem._collection[index]._val[i];
			}
		}

		if (nodeValid("traits", node, false)) {
			rapidxml::xml_node<char> *traitnode = node->first_node("traits");
			for (auto n = traitnode->first_node("trait"); n != nullptr; n = n->next_sibling("trait"))
				_trait.push_back(n);
		}
	}
}

void Person::reset() {
	for (auto i = 0; i < STAT_TOTAL; ++i)
		_stat._val[i].reset();
}

void Person::validate() {
	for (int i = 0; i < STAT_TOTAL; ++i)
		_stat._val[i].validate();
}

void Person::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "object");
	child->append_attribute(doc.allocate_attribute("id", _id.c_str()));
	child->append_attribute(doc.allocate_attribute("name", _name.c_str()));

	uint val = static_cast<uint>(_state);
	child->append_attribute(doc.allocate_attribute("state", g_engine->_stringPool->Get(val)));

	_opinion.saveState(doc, child);

	rapidxml::xml_node<char> *child_s = doc.allocate_node(rapidxml::node_element, "stats");
	_stat._val[STAT_HEALTH].saveState(doc, child_s, STATNAME_HEALTH);
	_stat._val[STAT_ATTACK].saveState(doc, child_s, STATNAME_ATTACK);
	_stat._val[STAT_DEFENSE].saveState(doc, child_s, STATNAME_DEFENSE);
	_stat._val[STAT_SPEED].saveState(doc, child_s, STATNAME_SPEED);
	/*stat.val[STAT_CHARISMA].saveState(doc, child_s, STATNAME_CHARISMA);
	stat.val[STAT_INTELLIGENCE].saveState(doc, child_s, STATNAME_INTELLIGENCE);*/
	child->append_node(child_s);

	rapidxml::xml_node<char> *child_t = doc.allocate_node(rapidxml::node_element, "traits");
	for (auto &i : _trait)
		i.saveState(doc, child_t, "trait");
	child->append_node(child_t);

	root->append_node(child);
}

void Person::loadState(rapidxml::xml_node<char> *node) {
	loadStr(_id, "id", node);
	loadStr(_name, "name", node);
	loadEnum(_state, "state", node);

	if (nodeValid("opinion", node))
		_opinion.load(node->first_node("opinion"));

	if (nodeValid("stats", node))
		_stat.load(node->first_node("stats"));

	if (nodeValid("traits", node, false)) {
		rapidxml::xml_node<char> *traitnode = node->first_node("traits");

		_trait.clear();
		for (auto n = traitnode->first_node("trait"); n != nullptr; n = n->next_sibling("trait"))
			_trait.push_back(n);
	}
}

} // End of namespace Crab
