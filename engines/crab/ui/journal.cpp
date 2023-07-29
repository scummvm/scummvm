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
#include "crab/ui/journal.h"

namespace Crab {

using namespace pyrodactyl::image;
using namespace pyrodactyl::ui;

//------------------------------------------------------------------------
// Purpose: Load game
//------------------------------------------------------------------------
void Journal::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("objectives");
		if (nodeValid(node)) {
			if (nodeValid("bg", node))
				_bg.load(node->first_node("bg"));

			if (nodeValid("map", node))
				_bu_map.load(node->first_node("map"));

			if (nodeValid("category", node))
				_category.load(node->first_node("category"));

			if (nodeValid("quest_list", node))
				_ref.load(node->first_node("quest_list"));

			_category.useKeyboard(true);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Prepare a new character's journal
//------------------------------------------------------------------------
void Journal::init(const Common::String &id) {
	int found = false;

	for (auto &i : _journal)
		if (i._id == id) {
			found = true;
			break;
		}

	if (!found) {
		Group g;
		g._id = id;
		for (int i = 0; i < JE_TOTAL; ++i) {
			g._menu[i] = _ref;
			g._menu[i].useKeyboard(true);
			g._menu[i].assignPaths();
		}
		_journal.push_back(g);
	}
}

//------------------------------------------------------------------------
// Purpose: Select a category
//------------------------------------------------------------------------
void Journal::select(const Common::String &id, const int &choice) {
	for (unsigned int i = 0; i < _category._element.size(); ++i)
		_category._element[i].state(false);

	_category._element[choice].state(true);
	_select = choice;

	// Always find valid journal group first
	for (auto &jo : _journal)
		if (jo._id == id) {
			jo._menu[choice]._unread = false;
			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Draw stuff
//------------------------------------------------------------------------
void Journal::draw(const Common::String &id) {
	_bg.draw();
	_category.draw();

	// Always find valid journal group first
	for (auto &jo : _journal)
		if (jo._id == id) {
			int count = 0;
			for (auto i = _category._element.begin(); i != _category._element.end() && count < JE_TOTAL; ++i, ++count)
				if (jo._menu[count]._unread)
					g_engine->_imageManager->notifyDraw(i->x + i->w, i->y);

			if (_select >= 0 && _select < JE_TOTAL)
				jo._menu[_select].draw(_bu_map);

			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Handle user input
//------------------------------------------------------------------------
bool Journal::handleEvents(const Common::String &id, const Common::Event &event) {
	int choice = _category.handleEvents(event);
	if (choice >= 0 && (unsigned int)choice < _category._element.size())
		select(id, choice);

	// Check if select is valid
	if (_select >= 0 && _select < JE_TOTAL) {
		// Always find valid journal group first
		for (auto &jo : _journal)
			if (jo._id == id)
				return jo._menu[_select].handleEvents(_bu_map, _markerTitle, event);
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Add an entry to journal
//------------------------------------------------------------------------
void Journal::add(const Common::String &id, const Common::String &category, const Common::String &title, const Common::String &text) {
	// Always find valid journal group first
	for (auto &jo : _journal)
		if (jo._id == id) {
			if (category == JE_CUR_NAME) {
				jo._menu[JE_CUR].add(title, text);
			} else if (category == JE_DONE_NAME) {
				jo._menu[JE_DONE].add(title, text);
			} else if (category == JE_PEOPLE_NAME) {
				jo._menu[JE_PEOPLE].add(title, text);
			} else if (category == JE_LOCATION_NAME) {
				jo._menu[JE_LOCATION].add(title, text);
			} else if (category == JE_HISTORY_NAME) {
				jo._menu[JE_HISTORY].add(title, text);
			}

			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Set the marker of a quest
//------------------------------------------------------------------------
void Journal::marker(const Common::String &id, const Common::String &title, const bool &val) {
	// Always find valid journal group first
	for (auto &jo : _journal)
		if (jo._id == id) {
			jo._menu[JE_CUR].marker(title, val);
			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Move an entry from one category to another
//------------------------------------------------------------------------
void Journal::move(const Common::String &id, const Common::String &title, const bool &completed) {
	JournalCategory source, destination;
	if (completed) {
		source = JE_CUR;
		destination = JE_DONE;
	} else {
		source = JE_DONE;
		destination = JE_CUR;
	}

	// Always find valid journal group first
	for (auto &jo : _journal)
		if (jo._id == id) {
			// Find the quest chain in the source menu
			unsigned int index = 0;
			for (auto i = jo._menu[source]._quest.begin(); i != jo._menu[source]._quest.end(); ++i, ++index)
				if (i->_title == title)
					break;

			if (index < jo._menu[source]._quest.size()) {
				jo._menu[destination].add(jo._menu[source]._quest[index]);
				jo._menu[source].erase(index);
			}

			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Open a specific entry in the journal
//------------------------------------------------------------------------
void Journal::open(const Common::String &id, const JournalCategory &category, const Common::String &title) {
	// Always find valid journal group first
	for (auto &jo : _journal)
		if (jo._id == id) {
			if (category >= 0 && category < (int)_category._element.size()) {
				// If category passes the valid check, select it
				select(id, category);

				// Perform validity check on select, just in case
				if (_select > 0 && _select < JE_TOTAL) {
					// Search for the title with same name
					for (unsigned int num = 0; num < jo._menu[_select]._quest.size(); ++num)
						if (jo._menu[_select]._quest[num]._title == title) {
							// Found it, switch to this
							jo._menu[_select].select(num);
							break;
						}
				}
			}

			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Load save game stuff
//------------------------------------------------------------------------
void Journal::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	for (auto &m : _journal) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "journal");
		child->append_attribute(doc.allocate_attribute("id", m._id.c_str()));

		m._menu[JE_CUR].saveState(doc, child, JE_CUR_NAME);
		m._menu[JE_DONE].saveState(doc, child, JE_DONE_NAME);
		m._menu[JE_PEOPLE].saveState(doc, child, JE_PEOPLE_NAME);
		m._menu[JE_LOCATION].saveState(doc, child, JE_LOCATION_NAME);
		m._menu[JE_HISTORY].saveState(doc, child, JE_HISTORY_NAME);
		root->append_node(child);
	}
}

void Journal::loadState(rapidxml::xml_node<char> *node) {
	for (rapidxml::xml_node<char> *n = node->first_node("journal"); n != NULL; n = n->next_sibling("journal")) {
		Common::String id;
		loadStr(id, "id", n);

		init(id);

		for (auto &i : _journal)
			if (i._id == id) {
				i._menu[JE_CUR].loadState(n->first_node(JE_CUR_NAME));
				i._menu[JE_DONE].loadState(n->first_node(JE_DONE_NAME));
				i._menu[JE_PEOPLE].loadState(n->first_node(JE_PEOPLE_NAME));
				i._menu[JE_LOCATION].loadState(n->first_node(JE_LOCATION_NAME));
				i._menu[JE_HISTORY].loadState(n->first_node(JE_HISTORY_NAME));
			}
	}
}

//------------------------------------------------------------------------
// Purpose: Adjust UI elements
//------------------------------------------------------------------------
void Journal::setUI() {
	_bg.setUI();
	_category.setUI();
	_ref.setUI();

	for (auto &m : _journal)
		for (auto i = 0; i < JE_TOTAL; ++i)
			m._menu[i].setUI();
}

} // End of namespace Crab
