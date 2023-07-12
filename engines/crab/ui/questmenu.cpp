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
#include "crab/ui/questmenu.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::event;

QuestMenu::QuestMenu() {
	sel_quest = -1;
	sel_page = -1;
	sel_bu = -1;
	align = ALIGN_LEFT;
	col_n = 0;
	col_s = 0;
	unread = false;
	font = 0;
}

//------------------------------------------------------------------------
// Purpose: Load layout from file
//------------------------------------------------------------------------
void QuestMenu::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		if (nodeValid("menu", node))
			menu.load(node->first_node("menu"));

		if (nodeValid("tab", node)) {
			rapidxml::xml_node<char> *tabnode = node->first_node("tab");
			loadNum(font, "font", tabnode);
			loadAlign(align, tabnode);
			off_title.load(tabnode);
			off_unread.load(tabnode->first_node("unread"));

			if (nodeValid("normal", tabnode)) {
				rapidxml::xml_node<char> *nornode = tabnode->first_node("normal");
				img_n.load(nornode);
				//loadColor(col_n, nornode);
			}

			if (nodeValid("select", tabnode)) {
				rapidxml::xml_node<char> *selnode = tabnode->first_node("select");
				img_s.load(selnode);
				//loadColor(col_s, selnode);
			}
		}

		if (nodeValid("text", node))
			text.load(node->first_node("text"));
	}
}

//------------------------------------------------------------------------
// Purpose: Add an entry to the menu
//------------------------------------------------------------------------
void QuestMenu::Add(const Common::String &title, const Common::String &txt) {
	for (auto i = quest.begin(); i != quest.end(); ++i)
		if (i->_title == title) // We already have the quest entry
		{
			i->_text.insert_at(0, txt); // Just add the new string to the start of the quest messages and return
			i->_unread = true;
			return;
		}

	Quest q(title, txt, true, false);
	quest.insert_at(0, q);
	menu.Add();
	unread = true;
}

void QuestMenu::Add(const pyrodactyl::event::Quest &q) {
	quest.insert_at(0, q);
	menu.Add();
}

//------------------------------------------------------------------------
// Purpose: Remove an entry from the menu
//------------------------------------------------------------------------
void QuestMenu::Erase(const int &index) {
	quest.erase(quest.begin() + index);
	menu.Erase();
}

//------------------------------------------------------------------------
// Purpose: Indicate that this quest has an associated map marker in world map
//------------------------------------------------------------------------
void QuestMenu::Marker(const Common::String &title, const bool &val) {
	for (auto i = quest.begin(); i != quest.end(); ++i)
		if (i->_title == title)
			i->_marker = val;
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void QuestMenu::draw(Button &bu_map) {
	menu.draw();

	using namespace pyrodactyl::text;
	for (auto i = menu.Index(), count = 0u; i < menu.IndexPlusOne() && i < quest.size(); i++, count++) {
		auto base_x = menu.BaseX(count), base_y = menu.BaseY(count);

		// Only draw in _s color if we are on the same button and page
		if ((unsigned int)sel_bu == count && (unsigned int)sel_page == menu.CurrentPage())
			g_engine->_textManager->draw(base_x + off_title.x, base_y + off_title.y, quest[i]._title, col_s, font, align);
		else
			g_engine->_textManager->draw(base_x + off_title.x, base_y + off_title.y, quest[i]._title, col_n, font, align);

		if (quest[i]._unread) {
			using namespace pyrodactyl::image;
			g_engine->_imageManager->draw(base_x + off_unread.x, base_y + off_unread.y, g_engine->_imageManager->notify);
		}
	}

	if (sel_quest >= 0 && (unsigned int)sel_quest < quest.size()) {
		text.draw(quest[sel_quest]);

		if (quest[sel_quest]._marker)
			bu_map.draw();
	}
}

//------------------------------------------------------------------------
// Purpose: Handle user input
//------------------------------------------------------------------------
bool QuestMenu::handleEvents(Button &bu_map, Common::String &map_title, const Common::Event &Event) {
	int res = menu.handleEvents(Event);
	if (res != -1) {
		if (sel_bu >= 0 && sel_page >= 0)
			menu.Image(sel_bu, sel_page, img_n);

		sel_bu = res;
		sel_page = menu.CurrentPage();
		sel_quest = menu.Index() + sel_bu;

		quest[sel_quest]._unread = false;
		text.reset();

		menu.Image(sel_bu, sel_page, img_s);
	}

	if (sel_quest >= 0 && (unsigned int)sel_quest < quest.size()) {
		if (quest[sel_quest]._marker)
			if (bu_map.handleEvents(Event) == BUAC_LCLICK) {
				// The title of the quest selected by the "show in map" button
				map_title = quest[sel_quest]._title;
				return true;
			}

		text.handleEvents(quest[sel_quest], Event);
	}

	return false;
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle user input
//------------------------------------------------------------------------
bool QuestMenu::handleEvents(Button &bu_map, Common::String &map_title, const SDL_Event &Event) {
	int res = menu.handleEvents(Event);
	if (res != -1) {
		if (sel_bu >= 0 && sel_page >= 0)
			menu.Image(sel_bu, sel_page, img_n);

		sel_bu = res;
		sel_page = menu.CurrentPage();
		sel_quest = menu.Index() + sel_bu;

		quest[sel_quest].unread = false;
		text.reset();

		menu.Image(sel_bu, sel_page, img_s);
	}

	if (sel_quest >= 0 && sel_quest < quest.size()) {
		if (quest[sel_quest].marker)
			if (bu_map.handleEvents(Event) == BUAC_LCLICK) {
				// The title of the quest selected by the "show in map" button
				map_title = quest[sel_quest].title;
				return true;
			}

		text.handleEvents(quest[sel_quest], Event);
	}

	return false;
}
#endif

//------------------------------------------------------------------------
// Purpose: Select an entry
//------------------------------------------------------------------------
void QuestMenu::Select(const int &quest_index) {
	if (quest_index >= 0 && (unsigned int)quest_index < quest.size()) {
		if (sel_bu >= 0 && sel_page >= 0)
			menu.Image(sel_bu, sel_page, img_n);

		sel_quest = quest_index;

		sel_page = quest_index / menu.ElementsPerPage();
		menu.CurrentPage(sel_page);
		menu.UpdateInfo();

		sel_bu = quest_index % menu.ElementsPerPage();

		quest[quest_index]._unread = false;
		text.reset();

		menu.Image(sel_bu, sel_page, img_s);
	}
}

//------------------------------------------------------------------------
// Purpose: Save state to file
//------------------------------------------------------------------------
void QuestMenu::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);

	saveBool(unread, "unread", doc, child);

	for (auto q = quest.begin(); q != quest.end(); ++q)
		q->saveState(doc, child);

	root->append_node(child);
}

//------------------------------------------------------------------------
// Purpose: Load state from file
//------------------------------------------------------------------------
void QuestMenu::loadState(rapidxml::xml_node<char> *node) {
	loadBool(unread, "unread", node);

	quest.clear();
	for (auto n = node->first_node("quest"); n != NULL; n = n->next_sibling("quest")) {
		Quest q;
		q.loadState(n);
		quest.push_back(q);
		menu.Add();
	}
}

//------------------------------------------------------------------------
// Purpose: Reposition UI elements
//------------------------------------------------------------------------
void QuestMenu::setUI() {
	menu.setUI();
	text.setUI();
}

} // End of namespace Crab
