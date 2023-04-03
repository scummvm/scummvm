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

#include "crab/ui/journal.h"

namespace Crab {

using namespace pyrodactyl::image;
using namespace pyrodactyl::ui;

//------------------------------------------------------------------------
// Purpose: Load game
//------------------------------------------------------------------------
void Journal::Load(const std::string &filename) {
	XMLDoc conf(filename.c_str());
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("objectives");
		if (NodeValid(node)) {
			if (NodeValid("bg", node))
				bg.Load(node->first_node("bg"));

			if (NodeValid("map", node))
				bu_map.Load(node->first_node("map"));

			if (NodeValid("category", node))
				category.Load(node->first_node("category"));

			if (NodeValid("quest_list", node))
				ref.Load(node->first_node("quest_list"));

			category.UseKeyboard(true);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Prepare a new character's journal
//------------------------------------------------------------------------
void Journal::Init(const std::string &id) {
	int found = false;

	for (auto &i : journal)
		if (i.id == id) {
			found = true;
			break;
		}

	if (!found) {
		Group g;
		g.id = id;
		for (int i = 0; i < JE_TOTAL; ++i) {
			g.menu[i] = ref;
			g.menu[i].UseKeyboard(true);
			g.menu[i].AssignPaths();
		}
		journal.push_back(g);
	}
}

//------------------------------------------------------------------------
// Purpose: Select a category
//------------------------------------------------------------------------
void Journal::Select(const std::string &id, const int &choice) {
	for (unsigned int i = 0; i < category.element.size(); ++i)
		category.element.at(i).State(false);

	category.element.at(choice).State(true);
	select = choice;

	// Always find valid journal group first
	for (auto &jo : journal)
		if (jo.id == id) {
			jo.menu[choice].unread = false;
			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Draw stuff
//------------------------------------------------------------------------
void Journal::Draw(const std::string &id) {
	bg.Draw();
	category.Draw();

	// Always find valid journal group first
	for (auto &jo : journal)
		if (jo.id == id) {
			int count = 0;
			for (auto i = category.element.begin(); i != category.element.end() && count < JE_TOTAL; ++i, ++count)
				if (jo.menu[count].unread)
					gImageManager.NotifyDraw(i->x + i->w, i->y);

			if (select >= 0 && select < JE_TOTAL)
				jo.menu[select].Draw(bu_map);

			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Handle user input
//------------------------------------------------------------------------
bool Journal::HandleEvents(const std::string &id, const Common::Event &Event) {
	int choice = category.HandleEvents(Event);
	if (choice >= 0 && choice < category.element.size())
		Select(id, choice);

	// Check if select is valid
	if (select >= 0 && select < JE_TOTAL) {
		// Always find valid journal group first
		for (auto &jo : journal)
			if (jo.id == id)
				return jo.menu[select].HandleEvents(bu_map, marker_title, Event);
	}

	return false;
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle user input
//------------------------------------------------------------------------
bool Journal::HandleEvents(const std::string &id, const SDL_Event &Event) {
	int choice = category.HandleEvents(Event);
	if (choice >= 0 && choice < category.element.size())
		Select(id, choice);

	// Check if select is valid
	if (select >= 0 && select < JE_TOTAL) {
		// Always find valid journal group first
		for (auto &jo : journal)
			if (jo.id == id)
				return jo.menu[select].HandleEvents(bu_map, marker_title, Event);
	}

	return false;
}
#endif

//------------------------------------------------------------------------
// Purpose: Add an entry to journal
//------------------------------------------------------------------------
void Journal::Add(const std::string &id, const std::string &Category, const std::string &Title, const std::string &Text) {
	// Always find valid journal group first
	for (auto &jo : journal)
		if (jo.id == id) {
			if (Category == JE_CUR_NAME) {
				jo.menu[JE_CUR].Add(Title, Text);
			} else if (Category == JE_DONE_NAME) {
				jo.menu[JE_DONE].Add(Title, Text);
			} else if (Category == JE_PEOPLE_NAME) {
				jo.menu[JE_PEOPLE].Add(Title, Text);
			} else if (Category == JE_LOCATION_NAME) {
				jo.menu[JE_LOCATION].Add(Title, Text);
			} else if (Category == JE_HISTORY_NAME) {
				jo.menu[JE_HISTORY].Add(Title, Text);
			}

			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Set the marker of a quest
//------------------------------------------------------------------------
void Journal::Marker(const std::string &id, const std::string &Title, const bool &val) {
	// Always find valid journal group first
	for (auto &jo : journal)
		if (jo.id == id) {
			jo.menu[JE_CUR].Marker(Title, val);
			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Move an entry from one category to another
//------------------------------------------------------------------------
void Journal::Move(const std::string &id, const std::string &Title, const bool &completed) {
	JournalCategory source, destination;
	if (completed) {
		source = JE_CUR;
		destination = JE_DONE;
	} else {
		source = JE_DONE;
		destination = JE_CUR;
	}

	// Always find valid journal group first
	for (auto &jo : journal)
		if (jo.id == id) {
			// Find the quest chain in the source menu
			unsigned int index = 0;
			for (auto i = jo.menu[source].quest.begin(); i != jo.menu[source].quest.end(); ++i, ++index)
				if (i->title == Title)
					break;

			if (index < jo.menu[source].quest.size()) {
				jo.menu[destination].Add(jo.menu[source].quest.at(index));
				jo.menu[source].Erase(index);
			}

			break;
		}
}

//------------------------------------------------------------------------
// Purpose: Open a specific entry in the journal
//------------------------------------------------------------------------
void Journal::Open(const std::string &id, const JournalCategory &Category, const std::string &Title) {
	// Always find valid journal group first
	for (auto &jo : journal)
		if (jo.id == id) {
			if (Category >= 0 && Category < category.element.size()) {
				// If category passes the valid check, select it
				Select(id, Category);

				// Perform validity check on select, just in case
				if (select > 0 && select < JE_TOTAL) {
					// Search for the title with same name
					for (unsigned int num = 0; num < jo.menu[select].quest.size(); ++num)
						if (jo.menu[select].quest[num].title == Title) {
							// Found it, switch to this
							jo.menu[select].Select(num);
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
void Journal::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	for (auto &m : journal) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "journal");
		child->append_attribute(doc.allocate_attribute("id", m.id.c_str()));

		m.menu[JE_CUR].SaveState(doc, child, JE_CUR_NAME);
		m.menu[JE_DONE].SaveState(doc, child, JE_DONE_NAME);
		m.menu[JE_PEOPLE].SaveState(doc, child, JE_PEOPLE_NAME);
		m.menu[JE_LOCATION].SaveState(doc, child, JE_LOCATION_NAME);
		m.menu[JE_HISTORY].SaveState(doc, child, JE_HISTORY_NAME);
		root->append_node(child);
	}
}

void Journal::LoadState(rapidxml::xml_node<char> *node) {
	for (rapidxml::xml_node<char> *n = node->first_node("journal"); n != NULL; n = n->next_sibling("journal")) {
		std::string id;
		LoadStr(id, "id", n);

		Init(id);

		for (auto &i : journal)
			if (i.id == id) {
				i.menu[JE_CUR].LoadState(n->first_node(JE_CUR_NAME));
				i.menu[JE_DONE].LoadState(n->first_node(JE_DONE_NAME));
				i.menu[JE_PEOPLE].LoadState(n->first_node(JE_PEOPLE_NAME));
				i.menu[JE_LOCATION].LoadState(n->first_node(JE_LOCATION_NAME));
				i.menu[JE_HISTORY].LoadState(n->first_node(JE_HISTORY_NAME));
			}
	}
}

//------------------------------------------------------------------------
// Purpose: Adjust UI elements
//------------------------------------------------------------------------
void Journal::SetUI() {
	bg.SetUI();
	category.SetUI();
	ref.SetUI();

	for (auto &m : journal)
		for (auto i = 0; i < JE_TOTAL; ++i)
			m.menu[i].SetUI();
}

} // End of namespace Crab
