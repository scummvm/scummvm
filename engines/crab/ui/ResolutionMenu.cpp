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
#include "crab/ui/ResolutionMenu.h"

namespace Crab {

using namespace pyrodactyl::image;
using namespace pyrodactyl::text;
using namespace pyrodactyl::ui;

void ResolutionMenu::load(rapidxml::xml_node<char> *node) {
	_cancel.load(node->first_node("cancel"));
	_change.load(node->first_node("change"));
	_custom.load(node->first_node("custom"));

	_info.load(node->first_node("info"));
	_defInfo = _info._text;

	if (nodeValid("reference", node))
		_ref.load(node->first_node("reference"));

	if (nodeValid("inc", node)) {
		_inc.load(node->first_node("inc"));
		loadNum(_columns, "columns", node->first_node("inc"));
	}

	if (nodeValid("options", node)) {
		int countSlot = 0;
		rapidxml::xml_node<char> *resnode = node->first_node("options");
		for (auto n = resnode->first_node("res"); n != NULL; n = n->next_sibling("res"), countSlot++) {
			Dimension d;
			loadNum(d.w, "x", n);
			loadNum(d.h, "y", n);

			if (g_engine->_screenSettings->validDimension(d)) {
				_dim.push_back(d);
				Button b;
				b.init(_ref, _inc.x * (countSlot % _columns), _inc.y * (countSlot / _columns));
				b._caption._text = numberToString(d.w);
				b._caption._text += " x ";
				b._caption._text += numberToString(d.h);
				_element.push_back(b);
			}
		}
	}

	setInfo();

	loadBool(_useKeyboard, "keyboard", node, false);
	assignPaths();
}

void ResolutionMenu::draw() {
	_info.draw();

	switch (_state) {
	case STATE_NORMAL:
		_change.draw();
		break;
	case STATE_CHANGE:
		Menu::draw();
		_cancel.draw();
		_custom.draw();
		break;
	default:
		break;
	}
}

int ResolutionMenu::handleEvents(const Common::Event &event) {
	switch (_state) {
	case STATE_NORMAL:
		if (_change.handleEvents(event) == BUAC_LCLICK)
			_state = STATE_CHANGE;
		break;
	case STATE_CHANGE: {
		int choice = Menu::handleEvents(event);
		if (choice >= 0) {
			g_engine->_screenSettings->_cur = _dim[choice];
			_state = STATE_NORMAL;
			return 1;
		}

		if (_custom.handleEvents(event) == BUAC_LCLICK) {
			_state = STATE_NORMAL;
			return 2;
		}

		if (_cancel.handleEvents(event) == BUAC_LCLICK)
			_state = STATE_NORMAL;
	} break;
	default:
		break;
	}

	return 0;
}

void ResolutionMenu::setInfo() {
	_info._text = _defInfo;
	_info._text += numberToString(g_engine->_screenSettings->_cur.w);
	_info._text += " x ";
	_info._text += numberToString(g_engine->_screenSettings->_cur.h);
}

void ResolutionMenu::setUI() {
	_cancel.setUI();
	_change.setUI();
	_custom.setUI();

	_info.setUI();
	_ref.setUI();
	ButtonMenu::setUI();
}

} // End of namespace Crab
