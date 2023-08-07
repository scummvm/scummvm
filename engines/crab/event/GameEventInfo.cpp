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

#include "crab/event/GameEventInfo.h"
#include "crab/event/eventstore.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
bool IsChar(char c) {
	if (c >= '0' && c <= '9')
		return false;
	return true;
}
} // End of namespace event
} // End of namespace pyrodactyl

using namespace pyrodactyl::people;
using namespace pyrodactyl::event;

//------------------------------------------------------------------------
// Purpose: Load from xml
//------------------------------------------------------------------------
void Info::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("people", node)) {
		rapidxml::xml_node<char> *pnode = node->first_node("people");

		_stem.load(pnode->first_attribute("templates")->value());

		XMLDoc conf(pnode->first_attribute("list")->value());
		if (conf.ready()) {
			rapidxml::xml_node<char> *cnode = conf.doc()->first_node("characters");
			if (nodeValid(cnode)) {
				loadNum(OPINION_MIN, "op_min", cnode);
				loadNum(OPINION_MAX, "op_max", cnode);

				for (auto n = cnode->first_node("group"); n != nullptr; n = n->next_sibling("group"))
					loadPeople(n->value());
			}
		}
	}

	if (nodeValid("objective", node))
		_journal.load(node->first_node("objective")->first_attribute("layout")->value());

	if (nodeValid("inventory", node)) {
		rapidxml::xml_node<char> *inode = node->first_node("inventory");
		_inv.load(inode->first_attribute("layout")->value());
	}

	curLocID(node->first_node("level")->first_attribute("start")->value());
	_inv.itemFile(node->first_node("item")->first_attribute("list")->value());
}

void Info::loadPeople(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("people");
		if (nodeValid(node)) {
			for (auto n = node->first_node(); n != nullptr; n = n->next_sibling()) {
				Common::String str;
				loadStr(str, "id", n);
				_people[str].load(n, _stem);
			}
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Get/Set information about object type
//------------------------------------------------------------------------
void Info::type(const Common::String &id, const PersonType &val) {
	if (_people.contains(id))
		_people[id]._type = val;
}

PersonType Info::type(const Common::String &id) {
	if (_people.contains(id))
		return _people[id]._type;

	return PE_NEUTRAL;
}

//------------------------------------------------------------------------
// Purpose: Get/Set information about object state
//------------------------------------------------------------------------
void Info::state(const Common::String &id, const PersonState &val) {
	if (_people.contains(id))
		_people[id]._state = val;
}

PersonState Info::state(const Common::String &id) {
	if (_people.contains(id))
		return _people[id]._state;

	return PST_NORMAL;
}

//------------------------------------------------------------------------
// Purpose: Get/Set information about variables
//------------------------------------------------------------------------
bool Info::varGet(const Common::String &name, int &val) {
	if (!_var.contains(name))
		return false;
	else
		val = _var[name];
	return true;
}

void Info::varSet(const Common::String &name, const Common::String &val) {
	int varVal = 0;
	bool assignToVar = Common::find_if(val.begin(), val.end(), IsChar) != val.end();

	if (assignToVar)
		varGet(val, varVal);
	else
		varVal = stringToNumber<int>(val);

	_var[name] = varVal;
}

void Info::varAdd(const Common::String &name, const int &val) {
	if (!_var.contains(name))
		varSet(name, 0);

	_var[name] += val;
}

void Info::varSub(const Common::String &name, const int &val) {
	if (!_var.contains(name))
		varSet(name, 0);

	_var[name] -= val;
}

void Info::varMul(const Common::String &name, const int &val) {
	if (!_var.contains(name))
		varSet(name, 0);

	_var[name] *= val;
}

void Info::varDiv(const Common::String &name, const int &val) {
	if (!_var.contains(name))
		varSet(name, 0);

	_var[name] /= val;
}

void Info::varDel(const Common::String &name) {
	_var.erase(name);
}

//------------------------------------------------------------------------
// Purpose: Get/Set person traits
//------------------------------------------------------------------------
void Info::traitAdd(const Common::String &perId, const int &traitId) {
	if (personValid(perId)) { // Valid person id
		if (traitId >= 0 && (uint)traitId < g_engine->_eventStore->_trait.size()) { // Valid trait id
			// Check for duplicate traits, DONT award anything if duplicate found
			Person *p = &personGet(perId);

			for (auto i = p->_trait.begin(); i != p->_trait.end(); ++i)
				if (i->_id == traitId)
					return;

			p->_trait.push_back(g_engine->_eventStore->_trait[traitId]);

			g_engine->_eventStore->setAchievement(g_engine->_eventStore->_trait[traitId]._id);
		}
	}
}

void Info::traitDel(const Common::String &perId, const int &traitId) {
	if (personValid(perId)) { // Valid person id
		if (traitId > 0 && (uint)traitId < g_engine->_eventStore->_trait.size()) { // Valid trait id
			Person *p = &personGet(perId);

			for (auto j = p->_trait.begin(); j != p->_trait.end(); ++j) {
				if (j->_id == traitId) {
					p->_trait.erase(j);
					break;
				}
			}
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Get/Set information about object opinion
//------------------------------------------------------------------------
bool Info::opinionGet(const Common::String &name, const pyrodactyl::people::OpinionType &type, int &val) {
	if (!_people.contains(name))
		return false;

	val = _people[name]._opinion._val[type];
	return true;
}

void Info::opinionChange(const Common::String &name, const pyrodactyl::people::OpinionType &type, int val) {
	if (_people.contains(name))
		_people[name]._opinion.change(type, val);
}

void Info::opinionSet(const Common::String &name, const pyrodactyl::people::OpinionType &type, int val) {
	if (_people.contains(name))
		_people[name]._opinion.set(type, val);
}

//------------------------------------------------------------------------
// Purpose: Get/Set information about object stats
//------------------------------------------------------------------------
bool Info::statGet(const Common::String &name, const pyrodactyl::stat::StatType &type, int &num) {
	if (!_people.contains(name))
		return false;

	num = _people[name]._stat._val[type]._cur;
	return true;
}

void Info::statSet(const Common::String &name, const pyrodactyl::stat::StatType &type, const int &num) {
	if (_people.contains(name))
		_people[name]._stat.set(type, num);
}

void Info::statChange(const Common::String &name, const pyrodactyl::stat::StatType &type, const int &num) {
	if (_people.contains(name))
		_people[name]._stat.change(type, num);
}

//------------------------------------------------------------------------
// Purpose: Get person object
//------------------------------------------------------------------------
bool Info::personGet(const Common::String &id, pyrodactyl::people::Person &p) {
	if (!_people.contains(id))
		return false;

	p = _people[id];
	return true;
}

bool Info::personValid(const Common::String &id) {
	return _people.contains(id);
}

pyrodactyl::people::Person &Info::personGet(const Common::String &id) {
	// Make sure to check PersonValid before doing this!
	// Only use this to change parts of an object
	return _people[id];
}

bool Info::collideWithTrigger(const Common::String &id, int rectIndex) {
	if (_people.contains(id)) {
		for (auto i = _people[id]._trig.begin(); i != _people[id]._trig.end(); ++i)
			if (*i == rectIndex)
				return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Replace all #values with their appropriate names in a string
//------------------------------------------------------------------------
void Info::insertName(Common::String &msg) {
	// We scan the dialog for #id values, which we convert to actual NPC names
	for (uint i = 0; i < msg.size(); ++i) {
		// The # symbol indicates that the next string until an end character needs to be replaced by the name
		if (msg[i] == '#') {
			// The position we want to start from, and the length of the substring
			uint start = i, end = i + 1, len = 0;

			// First make sure # wasn't the end of the string
			for (; end < msg.size(); ++end, ++len)
				if (msg[end] == ',' || msg[end] == '.' || msg[end] == '!' || msg[end] == ' ' ||
					msg[end] == '?' || msg[end] == '-' || msg[end] == '\'' || msg[end] == '\"')
					break;

			if (end < msg.size()) {
				// We use start+1 here because # isn't part of the id
				Common::String s = msg.substr(start + 1, len);

				// We use length+1 here because otherwise it lets the last character stay in dialog
				if (personValid(s))
					msg.replace(start, len + 1, personGet(s)._name);
			}
		}
	}
}

Common::String Info::getName(const Common::String &id) {
	if (personValid(id))
		return personGet(id)._name;

	return id;
}

//------------------------------------------------------------------------
// Purpose: Save and load object state
//------------------------------------------------------------------------
void Info::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	for (auto v = _var.begin(); v != _var.end(); ++v) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "var");
		child->append_attribute(doc.allocate_attribute("id", v->_key.c_str()));
		child->append_attribute(doc.allocate_attribute("val", g_engine->_stringPool->Get(v->_value)));
		root->append_node(child);
	}

	for (auto p = _people.begin(); p != _people.end(); ++p)
		p->_value.saveState(doc, root);

	rapidxml::xml_node<char> *childUnr = doc.allocate_node(rapidxml::node_element, "unread");

	saveBool(_unread._inventory, "inventory", doc, childUnr);
	saveBool(_unread._journal, "journal", doc, childUnr);
	saveBool(_unread._trait, "trait", doc, childUnr);
	saveBool(_unread._map, "map", doc, childUnr);

	root->append_node(childUnr);

	rapidxml::xml_node<char> *child_img = doc.allocate_node(rapidxml::node_element, "img");
	child_img->append_attribute(doc.allocate_attribute("index", g_engine->_stringPool->Get(_playerImg)));
	root->append_node(child_img);

	rapidxml::xml_node<char> *child_money = doc.allocate_node(rapidxml::node_element, "money");
	child_money->append_attribute(doc.allocate_attribute("var", _moneyVar.c_str()));
	root->append_node(child_money);

	_journal.saveState(doc, root);
	_inv.saveState(doc, root);
}

void Info::loadState(rapidxml::xml_node<char> *node) {
	for (rapidxml::xml_node<char> *v = node->first_node("var"); v != nullptr; v = v->next_sibling("var"))
		_var[v->first_attribute("id")->value()] = stringToNumber<int>(v->first_attribute("val")->value());

	for (rapidxml::xml_node<char> *p = node->first_node("object"); p != nullptr; p = p->next_sibling("object")) {
		Common::String id;
		loadStr(id, "id", p);
		_people[id].loadState(p);
	}

	if (nodeValid("unread", node)) {
		rapidxml::xml_node<char> *unrnode = node->first_node("unread");
		loadBool(_unread._inventory, "inventory", unrnode);
		loadBool(_unread._journal, "journal", unrnode);
		loadBool(_unread._trait, "trait", unrnode);
		loadBool(_unread._map, "map", unrnode);
	}

	if (nodeValid("img", node))
		loadNum(_playerImg, "index", node->first_node("img"));

	if (nodeValid("money", node))
		loadStr(_moneyVar, "var", node->first_node("money"));

	_journal.loadState(node);
	_inv.loadState(node);
}

//------------------------------------------------------------------------
// Purpose: Calculate UI positions after change in screen size
//------------------------------------------------------------------------
void Info::setUI() {
	_journal.setUI();
	_inv.setUI();
}

} // End of namespace Crab
