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

#ifndef CRAB_QUESTMENU_H
#define CRAB_QUESTMENU_H

#include "crab/ui/PageMenu.h"
#include "crab/ui/QuestText.h"
#include "crab/ui/StateButton.h"
#include "crab/common_header.h"
#include "crab/event/quest.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class QuestMenu {
	// The collection of quest pages
	PageButtonMenu _menu;

	// The currently selected quest for reading
	int _selQuest;

	// The currently selected page
	int _selPage;

	// The currently selected button in the page menu
	int _selBu;

	// For drawing quest text
	QuestText _text;

	// For drawing quest tabs
	FontKey _font;
	Align _align;
	int _colN, _colS;
	ButtonImage _imgN, _imgS;
	Vector2i _offTitle, _offUnread;

public:
	// All the quests currently in this menu
	Common::Array<pyrodactyl::event::Quest> _quest;

	// Keep track of unread notifications for each category button
	bool _unread;

	QuestMenu();
	~QuestMenu() {}

	void load(rapidxml::xml_node<char> *node);
	void draw(Button &buMap);

	bool handleEvents(Button &bu_map, Common::String &map_title, const Common::Event &Event);

	void useKeyboard(const bool &val) {
		_menu.useKeyboard(val);
	}

	void assignPaths() {
		_menu.assignPaths();
	}

	void marker(const Common::String &title, const bool &val);

	void add(const Common::String &title, const Common::String &txt);
	void add(const pyrodactyl::event::Quest &q);
	void erase(const int &index);

	void select(const int &questIndex);

	void saveState(rapidxml::xml_document<char> &doc, rapidxml::xml_node<char> *root, const char *name);
	void loadState(rapidxml::xml_node<char> *node);

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_QUESTMENU_H
