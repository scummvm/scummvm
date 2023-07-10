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


#include "crab/ui/GameOverMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void GameOverMenu::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		if (nodeValid("bg", node))
			bg.load(node->first_node("bg"));

		if (nodeValid("title", node)) {
			rapidxml::xml_node<char> *tinode = node->first_node("title");
			title.load(tinode);

			for (auto n = tinode->first_node("quote"); n != NULL; n = n->next_sibling("quote")) {
				Common::String str;
				loadStr(str, "text", n);
				quote.push_back(str);
			}
		}

		menu.load(node->first_node("menu"));
	}
}

int GameOverMenu::HandleEvents(const Common::Event &Event) {
	return menu.HandleEvents(Event);
}

#if 0
int GameOverMenu::HandleEvents(const SDL_Event &Event) {
	return menu.HandleEvents(Event);
}
#endif

void GameOverMenu::Draw() {
	bg.Draw();
	if (cur < quote.size())
		title.Draw(quote[cur]);

	menu.Draw();
}

void GameOverMenu::SetUI() {
	bg.SetUI();
	title.SetUI();
	menu.SetUI();
}

} // End of namespace Crab
