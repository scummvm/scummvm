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
#include "crab/ui/SlideShow.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

void SlideShow::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		if (nodeValid("pos", node))
			pos.load(node->first_node("pos"));

		if (nodeValid("bg", node))
			bg.load(node->first_node("bg"));

		if (nodeValid("prev", node)) {
			prev.load(node->first_node("prev"));
			prev.hotkey.Set(IU_PREV);
		}

		if (nodeValid("next", node)) {
			next.load(node->first_node("next"));
			next.hotkey.Set(IU_NEXT);
		}

		path.clear();
		for (auto n = node->first_node("slide"); n != NULL; n = n->next_sibling("slide")) {
			Common::String p;
			loadStr(p, "path", n);
			path.push_back(p);
		}

		index = 0;

		loadBool(usekeyboard, "keyboard", node, false);
	}
}

void SlideShow::draw() {
	bg.draw();
	img.draw(pos.x, pos.y);

	if (index > 0)
		prev.draw();

	if (index < path.size() - 1)
		next.draw();
}

void SlideShow::handleEvents(const Common::Event &Event) {
	using namespace pyrodactyl::input;

	if (index > 0)
		if (prev.handleEvents(Event) == BUAC_LCLICK) {
			index--;
			Refresh();
		}

	if (index < path.size() - 1)
		if (next.handleEvents(Event) == BUAC_LCLICK) {
			index++;
			Refresh();
		}
}

#if 0
void SlideShow::handleEvents(const SDL_Event &Event) {
	using namespace pyrodactyl::input;

	if (index > 0)
		if (prev.handleEvents(Event) == BUAC_LCLICK || (usekeyboard && g_engine->_inputManager->Equals(IU_LEFT, Event) == SDL_PRESSED)) {
			index--;
			Refresh();
		}

	if (index < path.size() - 1)
		if (next.handleEvents(Event) == BUAC_LCLICK || (usekeyboard && g_engine->_inputManager->Equals(IU_RIGHT, Event) == SDL_PRESSED)) {
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
		img.load(path[index]);
#endif
}

void SlideShow::setUI() {
	pos.setUI();
	bg.setUI();
	prev.setUI();
	next.setUI();
}

} // End of namespace Crab
