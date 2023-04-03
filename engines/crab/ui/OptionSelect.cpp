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

void OptionSelect::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		option.data.Load(node);
		prev.Load(node->first_node("prev"));
		next.Load(node->first_node("next"));
		cur = 0;

		option.text.clear();
		for (auto n = node->first_node("option"); n != NULL; n = n->next_sibling("option")) {
			std::string s;
			LoadStr(s, "name", n);
			option.text.push_back(s);
		}

		LoadBool(usekeyboard, "keyboard", node, false);
	}
}

void OptionSelect::Draw() {
	option.Draw(cur);

	if (cur > 0)
		prev.Draw();

	if (cur < option.text.size() - 1)
		next.Draw();
}

bool OptionSelect::HandleEvents(const Common::Event &Event) {
	using namespace pyrodactyl::input;

	if (cur > 0) {
		// Don't check for keyboard inputs for now
		if (prev.HandleEvents(Event) == BUAC_LCLICK) {
			cur--;
			return true;
		}
	}

	if (cur < option.text.size() - 1) {

		// Don't check for keyboard inputs for now
		if (next.HandleEvents(Event) == BUAC_LCLICK) {
			cur++;
			return true;
		}
	}

	return false;
}

#if 0
bool OptionSelect::HandleEvents(const SDL_Event &Event) {
	using namespace pyrodactyl::input;

	if (cur > 0) {
		if (prev.HandleEvents(Event) == BUAC_LCLICK || (usekeyboard && gInput.Equals(IU_LEFT, Event) == SDL_PRESSED)) {
			cur--;
			return true;
		}
	}

	if (cur < option.text.size() - 1) {
		if (next.HandleEvents(Event) == BUAC_LCLICK || (usekeyboard && gInput.Equals(IU_RIGHT, Event) == SDL_PRESSED)) {
			cur++;
			return true;
		}
	}

	return false;
}
#endif

void OptionSelect::SetUI() {
	option.data.SetUI();
	prev.SetUI();
	next.SetUI();
}

} // End of namespace Crab
