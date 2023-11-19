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
#include "crab/ui/KeyBindMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::text;
using namespace pyrodactyl::input;

void KeyBindMenu::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		if (nodeValid("menu", node)) {
			rapidxml::xml_node<char> *menode = node->first_node("menu");

			if (nodeValid("primary", menode))
				_prim.load(menode->first_node("primary"));

			if (nodeValid("alt", menode))
				_alt.load(menode->first_node("alt"));

			if (nodeValid("prompt", menode))
				_prompt.load(menode->first_node("prompt"));

			if (nodeValid("inc", menode))
				_inc.load(menode->first_node("inc"));

			if (nodeValid("dim", menode))
				_dim.load(menode->first_node("dim"));

			if (nodeValid("divide", menode))
				_divide.load(menode->first_node("divide"));

			if (nodeValid("desc", menode))
				_desc.load(menode->first_node("desc"));
		}

		// Initialize the menus
		initMenu(CON_GAME);
		initMenu(CON_UI);

		if (nodeValid("controls", node))
			_selControls.load(node->first_node("controls"));
	}
}

void KeyBindMenu::startAndSize(const int &type, int &start, int &size) {
	switch (type) {
	case CON_GAME:
		start = IG_START;
		size = IG_SIZE;
		break;
	case CON_UI:
		start = IU_START;
		size = IU_SIZE;
		break;
	default:
		break;
	}
}

void KeyBindMenu::initMenu(const int &type) {
	int start = 0, size = 0;
	startAndSize(type, start, size);

	// Initialize the menu
	_menu[type]._element.resize(size);
	for (int i = 0; i < size; i++) {
		int xoffset = _inc.x * i + _divide.x * (i / _dim.x);
		int yoffset = _inc.y * (i % _dim.x) + _divide.y * (i / _dim.x);

		_menu[type]._element[i].init(_prim, xoffset, yoffset);
		_menu[type]._element[i]._caption._text = g_engine->_inputManager->getAssociatedKey((InputType)(start + i));

		//_menu[type]._element[i + 1].init(_alt, xoffset, yoffset);
		//_menu[type]._element[i + 1]._caption._text = SDL_GetScancodeName(g_engine->_inputManager->iv[start + (i / 2)].alt);
	}
}

void KeyBindMenu::drawDesc(const int &type) {
	int start = 0, size = 0;
	startAndSize(type, start, size);

	for (int i = 0; i < size; i++) {
		int xoffset = _inc.x * i + _divide.x * (i / _dim.x);
		int yoffset = _inc.y * (i % _dim.x) + _divide.y * (i / _dim.x);

		_desc.draw(g_engine->_inputManager->_iv[i + start], xoffset, yoffset);
	}
}

void KeyBindMenu::draw() {
	_selControls.draw();

	_menu[_selControls._cur].draw();
	drawDesc(_selControls._cur);
}

void KeyBindMenu::setCaption() {
	int start = 0, size = 0;
	startAndSize(_selControls._cur, start, size);

	for (int i = 0; i < size; i++)
		_menu[_selControls._cur]._element[i]._caption._text = g_engine->_inputManager->getAssociatedKey((InputType)(start + i));
}

void KeyBindMenu::handleEvents(const Common::Event &event) {
	if (_selControls.handleEvents(event))
		setCaption();

	switch (_state) {
	case STATE_NORMAL:
		_choice = _menu[_selControls._cur].handleEvents(event);
		if (_choice >= 0) {
			_prompt.swap(_menu[_selControls._cur]._element[_choice]._caption);
			_state = STATE_KEY;
			g_system->getEventManager()->getKeymapper()->setEnabled(false);
			break;
		}

		break;
	case STATE_KEY:
		if (setKey(event)) { // if key remapped successfully
			g_engine->_inputManager->populateKeyTable(); // repopulate key table
			g_system->getEventManager()->getKeymapper()->setEnabled(true);

			setCaption();
			_menu[_selControls._cur]._element[_choice]._caption._col = _prompt._colPrev;
			_state = STATE_NORMAL;
		}

		break;
	default:
		break;
	}
}

bool KeyBindMenu::setKey(const Common::Event &event) {
	Common::HardwareInput hwInput = g_system->getEventManager()->getKeymapper()->findHardwareInput(event);
	if (hwInput.type != Common::kHardwareInputTypeInvalid) {
		int ch = _choice;
		if (_selControls._cur == CON_UI)
			ch += IG_SIZE;

		Common::KeymapArray keymapArr = g_system->getEventManager()->getKeymapper()->getKeymaps();
		for (Common::Keymap *keymap : keymapArr) {
			if (keymap->getType() != Common::Keymap::kKeymapTypeGame)
				continue;

			const Common::Keymap::ActionArray actions = keymap->getActions();
			for (Common::Action *action : actions) {
				if ((int)action->event.customType == ch) {
					keymap->unregisterMapping(action);
					keymap->registerMapping(action, hwInput);
					return true;
				}
			}
		}
	}

	return false;
}

#if 0
void KeyBindMenu::SwapKey(const SDL_Scancode &find) {
	int start = 0, size = 0;
	StartAndSize(sel_controls.cur, start, size);
	int pos = start + (choice / 2);

	for (int i = start; i < start + size; ++i) {
		if (g_engine->_inputManager->iv[i].key == find) {
			g_engine->_inputManager->iv[i].key = g_engine->_inputManager->iv[pos].key;
			break;
		} else if (g_engine->_inputManager->iv[i].alt == find) {
			g_engine->_inputManager->iv[i].alt = g_engine->_inputManager->iv[pos].key;
			break;
		}
	}

	if (choice % 2 == 0)
		g_engine->_inputManager->iv[pos].key = find;
	else
		g_engine->_inputManager->iv[pos].alt = find;
}
#endif

void KeyBindMenu::setUI() {
	_menu[CON_GAME].clear();
	_menu[CON_UI].clear();

	// Initialize the menus
	_prim.setUI();
	_alt.setUI();
	initMenu(CON_GAME);
	initMenu(CON_UI);

	_desc.setUI();
	_selControls.setUI();
}

} // End of namespace Crab
