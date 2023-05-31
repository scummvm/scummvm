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
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "crab/ui/SlideShow.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

void SlideShow::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		if (NodeValid("pos", node))
			pos.Load(node->first_node("pos"));

		if (NodeValid("bg", node))
			bg.Load(node->first_node("bg"));

		if (NodeValid("prev", node)) {
			prev.Load(node->first_node("prev"));
			prev.hotkey.Set(IU_PREV);
		}

		if (NodeValid("next", node)) {
			next.Load(node->first_node("next"));
			next.hotkey.Set(IU_NEXT);
		}

		path.clear();
		for (auto n = node->first_node("slide"); n != NULL; n = n->next_sibling("slide")) {
			Common::String p;
			LoadStr(p, "path", n);
			path.push_back(p);
		}

		index = 0;

		LoadBool(usekeyboard, "keyboard", node, false);
	}
}

void SlideShow::Draw() {
	bg.Draw();
	img.Draw(pos.x, pos.y);

	if (index > 0)
		prev.Draw();

	if (index < path.size() - 1)
		next.Draw();
}

void SlideShow::HandleEvents(const Common::Event &Event) {
	using namespace pyrodactyl::input;

	if (index > 0)
		if (prev.HandleEvents(Event) == BUAC_LCLICK) {
			index--;
			Refresh();
		}

	if (index < path.size() - 1)
		if (next.HandleEvents(Event) == BUAC_LCLICK) {
			index++;
			Refresh();
		}
}

#if 0
void SlideShow::HandleEvents(const SDL_Event &Event) {
	using namespace pyrodactyl::input;

	if (index > 0)
		if (prev.HandleEvents(Event) == BUAC_LCLICK || (usekeyboard && g_engine->_inputManager->Equals(IU_LEFT, Event) == SDL_PRESSED)) {
			index--;
			Refresh();
		}

	if (index < path.size() - 1)
		if (next.HandleEvents(Event) == BUAC_LCLICK || (usekeyboard && g_engine->_inputManager->Equals(IU_RIGHT, Event) == SDL_PRESSED)) {
			index++;
			Refresh();
		}
}
#endif

void SlideShow::Refresh() {
	warning("SlideShow::Refresh()");

#if 0
	img.Delete();

	if (index >= 0 && index < path.size())
		img.Load(path[index]);
#endif
}

void SlideShow::SetUI() {
	pos.SetUI();
	bg.SetUI();
	prev.SetUI();
	next.SetUI();
}

} // End of namespace Crab
