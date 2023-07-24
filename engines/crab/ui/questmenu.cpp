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
	_selQuest = -1;
	_selPage = -1;
	_selBu = -1;
	_align = ALIGN_LEFT;
	_colN = 0;
	_colS = 0;
	_unread = false;
	_font = 0;
}

//------------------------------------------------------------------------
// Purpose: Load layout from file
//------------------------------------------------------------------------
void QuestMenu::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		if (nodeValid("menu", node))
			_menu.load(node->first_node("menu"));

		if (nodeValid("tab", node)) {
			rapidxml::xml_node<char> *tabnode = node->first_node("tab");
			loadNum(_font, "font", tabnode);
			loadAlign(_align, tabnode);
			_offTitle.load(tabnode);
			_offUnread.load(tabnode->first_node("unread"));

			if (nodeValid("normal", tabnode)) {
				rapidxml::xml_node<char> *nornode = tabnode->first_node("normal");
				_imgN.load(nornode);
				//loadColor(col_n, nornode);
			}

			if (nodeValid("select", tabnode)) {
				rapidxml::xml_node<char> *selnode = tabnode->first_node("select");
				_imgS.load(selnode);
				//loadColor(col_s, selnode);
			}
		}

		if (nodeValid("text", node))
			_text.load(node->first_node("text"));
	}
}

//------------------------------------------------------------------------
// Purpose: Add an entry to the menu
//------------------------------------------------------------------------
void QuestMenu::add(const Common::String &title, const Common::String &txt) {
	for (auto i = _quest.begin(); i != _quest.end(); ++i)
		if (i->_title == title) // We already have the quest entry
		{
			i->_text.insert_at(0, txt); // Just add the new string to the start of the quest messages and return
			i->_unread = true;
			return;
		}

	Quest q(title, txt, true, false);
	_quest.insert_at(0, q);
	_menu.add();
	_unread = true;
}

void QuestMenu::add(const pyrodactyl::event::Quest &q) {
	_quest.insert_at(0, q);
	_menu.add();
}

//------------------------------------------------------------------------
// Purpose: Remove an entry from the menu
//------------------------------------------------------------------------
void QuestMenu::erase(const int &index) {
	_quest.erase(_quest.begin() + index);
	_menu.erase();
}

//------------------------------------------------------------------------
// Purpose: Indicate that this quest has an associated map marker in world map
//------------------------------------------------------------------------
void QuestMenu::marker(const Common::String &title, const bool &val) {
	for (auto i = _quest.begin(); i != _quest.end(); ++i)
		if (i->_title == title)
			i->_marker = val;
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void QuestMenu::draw(Button &buMap) {
	_menu.draw();

	using namespace pyrodactyl::text;
	for (auto i = _menu.index(), count = 0u; i < _menu.indexPlusOne() && i < _quest.size(); i++, count++) {
		auto base_x = _menu.baseX(count), base_y = _menu.baseY(count);

		// Only draw in _s color if we are on the same button and page
		if ((unsigned int)_selBu == count && (unsigned int)_selPage == _menu.currentPage())
			g_engine->_textManager->draw(base_x + _offTitle.x, base_y + _offTitle.y, _quest[i]._title, _colS, _font, _align);
		else
			g_engine->_textManager->draw(base_x + _offTitle.x, base_y + _offTitle.y, _quest[i]._title, _colN, _font, _align);

		if (_quest[i]._unread) {
			using namespace pyrodactyl::image;
			g_engine->_imageManager->draw(base_x + _offUnread.x, base_y + _offUnread.y, g_engine->_imageManager->_notify);
		}
	}

	if (_selQuest >= 0 && (unsigned int)_selQuest < _quest.size()) {
		_text.draw(_quest[_selQuest]);

		if (_quest[_selQuest]._marker)
			buMap.draw();
	}
}

//------------------------------------------------------------------------
// Purpose: Handle user input
//------------------------------------------------------------------------
bool QuestMenu::handleEvents(Button &buMap, Common::String &mapTitle, const Common::Event &event) {
	int res = _menu.handleEvents(event);
	if (res != -1) {
		if (_selBu >= 0 && _selPage >= 0)
			_menu.image(_selBu, _selPage, _imgN);

		_selBu = res;
		_selPage = _menu.currentPage();
		_selQuest = _menu.index() + _selBu;

		_quest[_selQuest]._unread = false;
		_text.reset();

		_menu.image(_selBu, _selPage, _imgS);
	}

	if (_selQuest >= 0 && (unsigned int)_selQuest < _quest.size()) {
		if (_quest[_selQuest]._marker)
			if (buMap.handleEvents(event) == BUAC_LCLICK) {
				// The title of the quest selected by the "show in map" button
				mapTitle = _quest[_selQuest]._title;
				return true;
			}

		_text.handleEvents(_quest[_selQuest], event);
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Select an entry
//------------------------------------------------------------------------
void QuestMenu::select(const int &questIndex) {
	if (questIndex >= 0 && (unsigned int)questIndex < _quest.size()) {
		if (_selBu >= 0 && _selPage >= 0)
			_menu.image(_selBu, _selPage, _imgN);

		_selQuest = questIndex;

		_selPage = questIndex / _menu.elementsPerPage();
		_menu.currentPage(_selPage);
		_menu.updateInfo();

		_selBu = questIndex % _menu.elementsPerPage();

		_quest[questIndex]._unread = false;
		_text.reset();

		_menu.image(_selBu, _selPage, _imgS);
	}
}

//------------------------------------------------------------------------
// Purpose: Save state to file
//------------------------------------------------------------------------
void QuestMenu::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);

	saveBool(_unread, "unread", doc, child);

	for (auto q = _quest.begin(); q != _quest.end(); ++q)
		q->saveState(doc, child);

	root->append_node(child);
}

//------------------------------------------------------------------------
// Purpose: Load state from file
//------------------------------------------------------------------------
void QuestMenu::loadState(rapidxml::xml_node<char> *node) {
	loadBool(_unread, "unread", node);

	_quest.clear();
	for (auto n = node->first_node("quest"); n != NULL; n = n->next_sibling("quest")) {
		Quest q;
		q.loadState(n);
		_quest.push_back(q);
		_menu.add();
	}
}

//------------------------------------------------------------------------
// Purpose: Reposition UI elements
//------------------------------------------------------------------------
void QuestMenu::setUI() {
	_menu.setUI();
	_text.setUI();
}

} // End of namespace Crab
