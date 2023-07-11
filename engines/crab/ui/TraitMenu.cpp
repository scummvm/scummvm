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
#include "crab/ui/TraitMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;

void TraitMenu::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("dim", node)) {
		rapidxml::xml_node<char> *dimnode = node->first_node("dim");
		loadNum(rows, "rows", dimnode);
		loadNum(cols, "cols", dimnode);
		size = rows * cols;
	}

	if (nodeValid("ref", node))
		ref.load(node->first_node("ref"));

	if (nodeValid("inc", node))
		inc.load(node->first_node("inc"));

	if (nodeValid("desc", node))
		desc.load(node->first_node("desc"));

	for (unsigned int i = 0; i < size; ++i) {
		TraitButton b;
		b.Init(ref, inc.x * (i % cols), inc.y * (i / cols));
		menu.element.push_back(b);
	}

	bool usekey = false;
	loadBool(usekey, "keyboard", node);
	menu.UseKeyboard(usekey);

	menu.AssignPaths();
}

void TraitMenu::draw(const pyrodactyl::people::Person *obj) {
	if (obj != nullptr) {
		auto i = menu.element.begin();
		for (auto t = obj->trait.begin(); t != obj->trait.end() && i != menu.element.end(); ++t, ++i) {
			i->draw();
			if (t->unread)
				g_engine->_imageManager->NotifyDraw(i->x + i->w, i->y);
		}

		for (; i != menu.element.end(); ++i)
			i->draw();

		if (select > -1 && (unsigned int)select < obj->trait.size())
			desc.draw(obj->trait[select].desc);
	} else
		for (auto &i : menu.element)
			i.draw();
}

void TraitMenu::handleEvents(pyrodactyl::people::Person *obj, const Common::Event &Event) {
	int choice = menu.handleEvents(Event);
	if (choice >= 0) {
		for (auto i = menu.element.begin(); i != menu.element.end(); ++i)
			i->State(false);

		menu.element[choice].State(true);
		select = choice;

		if (obj != nullptr) {
			if (select > -1 && (unsigned int)select < obj->trait.size())
				obj->trait[select].unread = false;
		}
	}
}

#if 0
void TraitMenu::handleEvents(pyrodactyl::people::Person *obj, const SDL_Event &Event) {
	int choice = menu.handleEvents(Event);
	if (choice >= 0) {
		for (auto i = menu.element.begin(); i != menu.element.end(); ++i)
			i->State(false);

		menu.element[choice].State(true);
		select = choice;

		if (obj != nullptr) {
			if (select > -1 && select < obj->trait.size())
				obj->trait[select].unread = false;
		}
	}
}
#endif

void TraitMenu::Cache(const pyrodactyl::people::Person &obj) {
	auto e = menu.element.begin();

	for (auto i = obj.trait.begin(); i != obj.trait.end() && e != menu.element.end(); ++i, ++e)
		e->Cache(*i);

	for (; e != menu.element.end(); ++e)
		e->Empty();
}

void TraitMenu::Clear() {
	for (auto e = menu.element.begin(); e != menu.element.end(); ++e)
		e->Empty();
}

void TraitMenu::setUI() {
	ref.setUI();
	desc.setUI();
	menu.setUI();
}

} // End of namespace Crab
