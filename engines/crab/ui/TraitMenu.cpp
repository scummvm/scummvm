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
		loadNum(_rows, "rows", dimnode);
		loadNum(_cols, "cols", dimnode);
		_size = _rows * _cols;
	}

	if (nodeValid("ref", node))
		_ref.load(node->first_node("ref"));

	if (nodeValid("inc", node))
		_inc.load(node->first_node("inc"));

	if (nodeValid("desc", node))
		_desc.load(node->first_node("desc"));

	for (uint i = 0; i < _size; ++i) {
		TraitButton b;
		b.init(_ref, _inc.x * (i % _cols), _inc.y * (i / _cols));
		_menu._element.push_back(b);
	}

	bool usekey = false;
	loadBool(usekey, "keyboard", node);
	_menu.useKeyboard(usekey);

	_menu.assignPaths();
}

void TraitMenu::draw(const pyrodactyl::people::Person *obj) {
	if (obj != nullptr) {
		auto i = _menu._element.begin();
		for (auto t = obj->_trait.begin(); t != obj->_trait.end() && i != _menu._element.end(); ++t, ++i) {
			i->draw();
			if (t->_unread)
				g_engine->_imageManager->notifyDraw(i->x + i->w, i->y);
		}

		for (; i != _menu._element.end(); ++i)
			i->draw();

		if (_select > -1 && (uint)_select < obj->_trait.size())
			_desc.draw(obj->_trait[_select]._desc);
	} else
		for (auto &i : _menu._element)
			i.draw();
}

void TraitMenu::handleEvents(pyrodactyl::people::Person *obj, const Common::Event &event) {
	int choice = _menu.handleEvents(event);
	if (choice >= 0) {
		for (auto i = _menu._element.begin(); i != _menu._element.end(); ++i)
			i->state(false);

		_menu._element[choice].state(true);
		_select = choice;

		if (obj != nullptr && (uint)_select < obj->_trait.size())
			obj->_trait[_select]._unread = false;
	}
}

void TraitMenu::cache(const pyrodactyl::people::Person &obj) {
	auto e = _menu._element.begin();

	for (auto i = obj._trait.begin(); i != obj._trait.end() && e != _menu._element.end(); ++i, ++e)
		e->cache(*i);

	for (; e != _menu._element.end(); ++e)
		e->empty();
}

void TraitMenu::clear() {
	for (auto &e : _menu._element)
		e.empty();
}

void TraitMenu::setUI() {
	_ref.setUI();
	_desc.setUI();
	_menu.setUI();
}

} // End of namespace Crab
