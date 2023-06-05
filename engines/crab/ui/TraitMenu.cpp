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

void TraitMenu::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid("dim", node)) {
		rapidxml::xml_node<char> *dimnode = node->first_node("dim");
		LoadNum(rows, "rows", dimnode);
		LoadNum(cols, "cols", dimnode);
		size = rows * cols;
	}

	if (NodeValid("ref", node))
		ref.Load(node->first_node("ref"));

	if (NodeValid("inc", node))
		inc.Load(node->first_node("inc"));

	if (NodeValid("desc", node))
		desc.Load(node->first_node("desc"));

	for (unsigned int i = 0; i < size; ++i) {
		TraitButton b;
		b.Init(ref, inc.x * (i % cols), inc.y * (i / cols));
		menu.element.push_back(b);
	}

	bool usekey = false;
	LoadBool(usekey, "keyboard", node);
	menu.UseKeyboard(usekey);

	menu.AssignPaths();
}

void TraitMenu::Draw(const pyrodactyl::people::Person *obj) {
	if (obj != nullptr) {
		auto i = menu.element.begin();
		for (auto t = obj->trait.begin(); t != obj->trait.end() && i != menu.element.end(); ++t, ++i) {
			i->Draw();
			if (t->unread)
				g_engine->_imageManager->NotifyDraw(i->x + i->w, i->y);
		}

		for (; i != menu.element.end(); ++i)
			i->Draw();

		if (select > -1 && (unsigned int)select < obj->trait.size())
			desc.Draw(obj->trait[select].desc);
	} else
		for (auto &i : menu.element)
			i.Draw();
}

void TraitMenu::HandleEvents(pyrodactyl::people::Person *obj, const Common::Event &Event) {
	int choice = menu.HandleEvents(Event);
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
void TraitMenu::HandleEvents(pyrodactyl::people::Person *obj, const SDL_Event &Event) {
	int choice = menu.HandleEvents(Event);
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

void TraitMenu::SetUI() {
	ref.SetUI();
	desc.SetUI();
	menu.SetUI();
}

} // End of namespace Crab
