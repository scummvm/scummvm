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


#include "crab/ui/OptionSelect.h"

namespace Crab {

using namespace pyrodactyl::text;
using namespace pyrodactyl::ui;

void OptionSelect::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		option.data.load(node);
		prev.load(node->first_node("prev"));
		next.load(node->first_node("next"));
		cur = 0;

		option.text.clear();
		for (auto n = node->first_node("option"); n != NULL; n = n->next_sibling("option")) {
			Common::String s;
			loadStr(s, "name", n);
			option.text.push_back(s);
		}

		loadBool(usekeyboard, "keyboard", node, false);
	}
}

void OptionSelect::draw() {
	option.draw(cur);

	if (cur > 0)
		prev.draw();

	if ((unsigned int)cur < option.text.size() - 1)
		next.draw();
}

bool OptionSelect::handleEvents(const Common::Event &Event) {
	using namespace pyrodactyl::input;

	if (cur > 0) {
		// Don't check for keyboard inputs for now
		if (prev.handleEvents(Event) == BUAC_LCLICK) {
			cur--;
			return true;
		}
	}

	if ((unsigned int)cur < option.text.size() - 1) {

		// Don't check for keyboard inputs for now
		if (next.handleEvents(Event) == BUAC_LCLICK) {
			cur++;
			return true;
		}
	}

	return false;
}

#if 0
bool OptionSelect::handleEvents(const SDL_Event &Event) {
	using namespace pyrodactyl::input;

	if (cur > 0) {
		if (prev.handleEvents(Event) == BUAC_LCLICK || (usekeyboard && g_engine->_inputManager->Equals(IU_LEFT, Event) == SDL_PRESSED)) {
			cur--;
			return true;
		}
	}

	if (cur < option.text.size() - 1) {
		if (next.handleEvents(Event) == BUAC_LCLICK || (usekeyboard && g_engine->_inputManager->Equals(IU_RIGHT, Event) == SDL_PRESSED)) {
			cur++;
			return true;
		}
	}

	return false;
}
#endif

void OptionSelect::setUI() {
	option.data.setUI();
	prev.setUI();
	next.setUI();
}

} // End of namespace Crab
