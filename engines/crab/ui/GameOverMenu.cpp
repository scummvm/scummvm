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
			_bg.load(node->first_node("bg"));

		if (nodeValid("title", node)) {
			rapidxml::xml_node<char> *tinode = node->first_node("title");
			_title.load(tinode);

			for (auto n = tinode->first_node("quote"); n != nullptr; n = n->next_sibling("quote")) {
				Common::String str;
				loadStr(str, "text", n);
				_quote.push_back(str);
			}
		}

		_menu.load(node->first_node("menu"));
	}
}

int GameOverMenu::handleEvents(const Common::Event &event) {
	return _menu.handleEvents(event);
}

void GameOverMenu::draw() {
	_bg.draw();
	if (_cur < _quote.size())
		_title.draw(_quote[_cur]);

	_menu.draw();
}

void GameOverMenu::setUI() {
	_bg.setUI();
	_title.setUI();
	_menu.setUI();
}

} // End of namespace Crab
