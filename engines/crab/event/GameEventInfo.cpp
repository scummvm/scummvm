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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "crab/event/GameEventInfo.h"
#include "crab/event/eventstore.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
bool IsChar(char c) {
	return !isdigit(c);
}
} // End of namespace event
} // End of namespace pyrodactyl

using namespace pyrodactyl::people;
using namespace pyrodactyl::event;

//------------------------------------------------------------------------
// Purpose: Load from xml
//------------------------------------------------------------------------
void Info::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid("people", node)) {
		rapidxml::xml_node<char> *pnode = node->first_node("people");

		stem.Load(pnode->first_attribute("templates")->value());

		XMLDoc conf(pnode->first_attribute("list")->value());
		if (conf.ready()) {
			rapidxml::xml_node<char> *cnode = conf.Doc()->first_node("characters");
			if (NodeValid(cnode)) {
				LoadNum(OPINION_MIN, "op_min", cnode);
				LoadNum(OPINION_MAX, "op_max", cnode);

				for (auto n = cnode->first_node("group"); n != NULL; n = n->next_sibling("group"))
					LoadPeople(n->value());
			}
		}
	}

	if (NodeValid("objective", node))
		journal.Load(node->first_node("objective")->first_attribute("layout")->value());

	if (NodeValid("inventory", node)) {
		rapidxml::xml_node<char> *inode = node->first_node("inventory");
		inv.Load(inode->first_attribute("layout")->value());
	}

	CurLocID(node->first_node("level")->first_attribute("start")->value());
	inv.ItemFile(node->first_node("item")->first_attribute("list")->value());
}

void Info::LoadPeople(const std::string &filename) {
	XMLDoc conf(filename.c_str());
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("people");
		if (NodeValid(node)) {
			for (auto n = node->first_node(); n != NULL; n = n->next_sibling()) {
				std::string str;
				LoadStr(str, "id", n);
				people[str].Load(n, stem);
			}
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Get/Set information about object type
//------------------------------------------------------------------------
void Info::Type(const std::string &id, const PersonType &val) {
	if (people.count(id) > 0)
		people.at(id).type = val;
}

PersonType Info::Type(const std::string &id) {
	if (people.count(id) > 0)
		return people.at(id).type;

	return PE_NEUTRAL;
}

//------------------------------------------------------------------------
// Purpose: Get/Set information about object state
//------------------------------------------------------------------------
void Info::State(const std::string &id, const PersonState &val) {
	if (people.count(id) > 0)
		people.at(id).state = val;
}

PersonState Info::State(const std::string &id) {
	if (people.count(id) > 0)
		return people.at(id).state;

	return PST_NORMAL;
}

//------------------------------------------------------------------------
// Purpose: Get/Set information about variables
//------------------------------------------------------------------------
bool Info::VarGet(const std::string &name, int &val) {
	if (var.count(name) == 0)
		return false;
	else
		val = var.at(name);
	return true;
}

void Info::VarSet(const std::string &name, const std::string &val) {
	int var_val = 0;
	bool assign_to_var = std::find_if(val.begin(), val.end(), IsChar) != val.end();

	if (assign_to_var)
		VarGet(val, var_val);
	else
		var_val = StringToNumber<int>(val);

	var[name] = var_val;
}

void Info::VarAdd(const std::string &name, const int &val) {
	if (var.count(name) == 0)
		VarSet(name, 0);

	var.at(name) += val;
}

void Info::VarSub(const std::string &name, const int &val) {
	if (var.count(name) == 0)
		VarSet(name, 0);

	var.at(name) -= val;
}

void Info::VarMul(const std::string &name, const int &val) {
	if (var.count(name) == 0)
		VarSet(name, 0);

	var.at(name) *= val;
}

void Info::VarDiv(const std::string &name, const int &val) {
	if (var.count(name) == 0)
		VarSet(name, 0);

	var.at(name) /= val;
}

void Info::VarDel(const std::string &name) {
	var.erase(name);
}

//------------------------------------------------------------------------
// Purpose: Get/Set person traits
//------------------------------------------------------------------------
void Info::TraitAdd(const std::string &per_id, const int &trait_id) {
	if (PersonValid(per_id)) // Valid person id
	{
		if (trait_id >= 0 && trait_id < gEventStore.trait.size()) // Valid trait id
		{
			// Check for duplicate traits, DONT award anything if duplicate found
			Person *p = &PersonGet(per_id);

			for (auto i = p->trait.begin(); i != p->trait.end(); ++i)
				if (i->id == trait_id)
					return;

			p->trait.push_back(gEventStore.trait.at(trait_id));

			gEventStore.SetAchievement(gEventStore.trait.at(trait_id).id);
		}
	}
}

void Info::TraitDel(const std::string &per_id, const int &trait_id) {
	if (PersonValid(per_id)) // Valid person id
	{
		if (trait_id > 0 && trait_id < gEventStore.trait.size()) // Valid trait id
		{
			Person *p = &PersonGet(per_id);

			for (auto j = p->trait.begin(); j != p->trait.end(); ++j) {
				if (j->id == trait_id) {
					p->trait.erase(j);
					break;
				}
			}
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Get/Set information about object opinion
//------------------------------------------------------------------------
bool Info::OpinionGet(const std::string &name, const pyrodactyl::people::OpinionType &type, int &val) {
	if (people.count(name) == 0)
		return false;

	val = people.at(name).opinion.val[type];
	return true;
}

void Info::OpinionChange(const std::string &name, const pyrodactyl::people::OpinionType &type, int val) {
	if (people.count(name) > 0)
		people.at(name).opinion.Change(type, val);
}

void Info::OpinionSet(const std::string &name, const pyrodactyl::people::OpinionType &type, int val) {
	if (people.count(name) > 0)
		people.at(name).opinion.Set(type, val);
}

//------------------------------------------------------------------------
// Purpose: Get/Set information about object stats
//------------------------------------------------------------------------
bool Info::StatGet(const std::string &name, const pyrodactyl::stat::StatType &type, int &num) {
	if (people.count(name) == 0)
		return false;

	num = people.at(name).stat.val[type].cur;
	return true;
}

void Info::StatSet(const std::string &name, const pyrodactyl::stat::StatType &type, const int &num) {
	if (people.count(name) > 0)
		people.at(name).stat.Set(type, num);
}

void Info::StatChange(const std::string &name, const pyrodactyl::stat::StatType &type, const int &num) {
	if (people.count(name) > 0)
		people.at(name).stat.Change(type, num);
}

//------------------------------------------------------------------------
// Purpose: Get person object
//------------------------------------------------------------------------
bool Info::PersonGet(const std::string &id, pyrodactyl::people::Person &p) {
	if (people.count(id) == 0)
		return false;

	p = people.at(id);
	return true;
}

bool Info::PersonValid(const std::string &id) {
	return people.count(id) > 0;
}

pyrodactyl::people::Person &Info::PersonGet(const std::string &id) {
	// Make sure to check PersonValid before doing this!
	// Only use this to change parts of an object
	return people.at(id);
}

bool Info::CollideWithTrigger(const std::string &id, int rect_index) {
	if (people.count(id) > 0) {
		for (auto i = people.at(id).trig.begin(); i != people.at(id).trig.end(); ++i)
			if (*i == rect_index)
				return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Replace all #values with their appropriate names in a string
//------------------------------------------------------------------------
void Info::InsertName(std::string &msg) {
	// We scan the dialog for #id values, which we convert to actual NPC names
	for (unsigned int i = 0; i < msg.size(); ++i) {
		// The # symbol indicates that the next string until an end character needs to be replaced by the name
		if (msg.at(i) == '#') {
			// The position we want to start from, and the length of the substring
			unsigned int start = i, end = i + 1, len = 0;

			// First make sure # wasn't the end of the string
			for (; end < msg.size(); ++end, ++len)
				if (msg.at(end) == ',' || msg.at(end) == '.' || msg.at(end) == '!' || msg.at(end) == ' ' ||
					msg.at(end) == '?' || msg.at(end) == '-' || msg.at(end) == '\'' || msg.at(end) == '\"')
					break;

			if (end < msg.size()) {
				// We use start+1 here because # isn't part of the id
				std::string s = msg.substr(start + 1, len);

				// We use length+1 here because otherwise it lets the last character stay in dialog
				if (PersonValid(s))
					msg.replace(start, len + 1, PersonGet(s).name);
			}
		}
	}
}

std::string Info::GetName(const std::string &id) {
	if (PersonValid(id))
		return PersonGet(id).name;

	return id;
}

//------------------------------------------------------------------------
// Purpose: Save and load object state
//------------------------------------------------------------------------
void Info::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	warning("Info::SaveState()");

#if 0
	for (auto v = var.begin(); v != var.end(); ++v) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "var");
		child->append_attribute(doc.allocate_attribute("id", v->first.c_str()));
		child->append_attribute(doc.allocate_attribute("val", gStrPool.Get(v->second)));
		root->append_node(child);
	}

	for (auto p = people.begin(); p != people.end(); ++p)
		p->second.SaveState(doc, root);

	rapidxml::xml_node<char> *child_unr = doc.allocate_node(rapidxml::node_element, "unread");

	SaveBool(unread.inventory, "inventory", doc, child_unr);
	SaveBool(unread.journal, "journal", doc, child_unr);
	SaveBool(unread.trait, "trait", doc, child_unr);
	SaveBool(unread.map, "map", doc, child_unr);

	root->append_node(child_unr);

	rapidxml::xml_node<char> *child_img = doc.allocate_node(rapidxml::node_element, "img");
	child_img->append_attribute(doc.allocate_attribute("index", gStrPool.Get(player_img)));
	root->append_node(child_img);

	rapidxml::xml_node<char> *child_money = doc.allocate_node(rapidxml::node_element, "money");
	child_money->append_attribute(doc.allocate_attribute("var", money_var.c_str()));
	root->append_node(child_money);

	journal.SaveState(doc, root);
	inv.SaveState(doc, root);
#endif
}

void Info::LoadState(rapidxml::xml_node<char> *node) {
	for (rapidxml::xml_node<char> *v = node->first_node("var"); v != NULL; v = v->next_sibling("var"))
		var[v->first_attribute("id")->value()] = StringToNumber<int>(v->first_attribute("val")->value());

	for (rapidxml::xml_node<char> *p = node->first_node("object"); p != NULL; p = p->next_sibling("object")) {
		std::string id;
		LoadStr(id, "id", p);
		people[id].LoadState(p);
	}

	if (NodeValid("unread", node)) {
		rapidxml::xml_node<char> *unrnode = node->first_node("unread");
		LoadBool(unread.inventory, "inventory", unrnode);
		LoadBool(unread.journal, "journal", unrnode);
		LoadBool(unread.trait, "trait", unrnode);
		LoadBool(unread.map, "map", unrnode);
	}

	if (NodeValid("img", node))
		LoadNum(player_img, "index", node->first_node("img"));

	if (NodeValid("money", node))
		LoadStr(money_var, "var", node->first_node("money"));

	journal.LoadState(node);
	inv.LoadState(node);
}

//------------------------------------------------------------------------
// Purpose: Calculate UI positions after change in screen size
//------------------------------------------------------------------------
void Info::SetUI() {
	journal.SetUI();
	inv.SetUI();
}

} // End of namespace Crab
