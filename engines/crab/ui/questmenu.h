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
	PageButtonMenu menu;

	// The currently selected quest for reading
	int sel_quest;

	// The currently selected page
	int sel_page;

	// The currently selected button in the page menu
	int sel_bu;

	// For drawing quest text
	QuestText text;

	// For drawing quest tabs
	FontKey font;
	Align align;
	int col_n, col_s;
	ButtonImage img_n, img_s;
	Vector2i off_title, off_unread;

public:
	// All the quests currently in this menu
	std::deque<pyrodactyl::event::Quest> quest;

	// Keep track of unread notifications for each category button
	bool unread;

	QuestMenu();
	~QuestMenu() {}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(Button &bu_map);

	bool HandleEvents(Button &bu_map, Common::String &map_title, const Common::Event &Event);
#if 0
	bool HandleEvents(Button &bu_map, Common::String &map_title, const SDL_Event &Event);
#endif

	void UseKeyboard(const bool &val) { menu.UseKeyboard(val); }
	void AssignPaths() { menu.AssignPaths(); }

	void Marker(const Common::String &title, const bool &val);

	void Add(const Common::String &title, const Common::String &txt);
	void Add(const pyrodactyl::event::Quest &q);
	void Erase(const int &index);

	void Select(const int &quest_index);

	void SaveState(rapidxml::xml_document<char> &doc, rapidxml::xml_node<char> *root, const char *name);
	void LoadState(rapidxml::xml_node<char> *node);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_QUESTMENU_H
