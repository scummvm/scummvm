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

#include "crab/ui/PauseMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void PauseMenu::load(rapidxml::xml_node<char> *node) {
	_menu.load(node->first_node("menu"));
	_save.load(node->first_node("save"));

	if (nodeValid("bg", node))
		_bg.load(node->first_node("bg"));
}

bool PauseMenu::draw(Button &back) {
	switch (_state) {
	case STATE_NORMAL:
		_bg.draw();
		_menu.draw();
		break;
	case STATE_OPTION:
		g_engine->_optionMenu->draw(back);
		return true;
	default:
		break;
	}

	return false;
}

PauseSignal PauseMenu::handleEvents(const Common::Event &event, Button &back) {
	switch (_state) {
	case STATE_NORMAL:
		_choice = _menu.handleEvents(event);
		if (_choice == -1) {
			if (back._hotkey.handleEvents(event))
				return PS_RESUME;
		} else {
			switch (_choice) {
			case 0:
				_state = STATE_NORMAL;
				return PS_RESUME;
			case 1:
				if (g_engine->saveGameDialog()) {
					_state = STATE_NORMAL;
					return PS_SAVE;
				} else
					_state = STATE_NORMAL;
				break;
			case 2:
				if (g_engine->loadGameDialog()) {
					_state = STATE_NORMAL;
					return PS_LOAD;
				} else
					_state = STATE_NORMAL;
				break;
			case 3:
				_state = STATE_OPTION;
				break;
			case 4:
				return PS_HELP;
			case 5:
				return PS_QUIT_MENU;
			case 6:
				return PS_QUIT_GAME;
			default:
				break;
			}
		}
		break;
	case STATE_OPTION:
		if (g_engine->_optionMenu->handleEvents(back, event)) {
			g_engine->_optionMenu->reset();
			_state = STATE_NORMAL;
		}
		break;
	case STATE_LOAD:
		if (g_engine->loadGameDialog())
			return PS_LOAD;
		else
			_state = STATE_NORMAL;
		break;
	default:
		break;
	}

	return PS_NONE;
}

#if 0
PauseSignal PauseMenu::handleEvents(const SDL_Event &Event, Button &back) {
	switch (state) {
	case STATE_NORMAL:
		choice = menu.handleEvents(Event);
		if (choice == -1) {
			if (back.hotkey.handleEvents(Event))
				return PS_RESUME;
		} else {
			switch (choice) {
			case 0:
				state = STATE_NORMAL;
				return PS_RESUME;
			case 1:
				state = STATE_SAVE;
				save.ScanDir();
				break;
			case 2:
				state = STATE_LOAD;
				g_engine->_loadMenu->ScanDir();
				break;
			case 3:
				state = STATE_OPTION;
				break;
			case 4:
				return PS_HELP;
			case 5:
				return PS_QUIT_MENU;
			case 6:
				return PS_QUIT_GAME;
			default:
				break;
			}
		}
		break;
	case STATE_SAVE:
		if (save.handleEvents(Event)) {
			state = STATE_NORMAL;
			return PS_SAVE;
		} else if (back.handleEvents(Event) == BUAC_LCLICK && !save.DisableHotkeys())
			state = STATE_NORMAL;
		break;
	case STATE_OPTION:
		if (g_engine->_optionMenu->handleEvents(back, Event)) {
			g_engine->_optionMenu->reset();
			state = STATE_NORMAL;
		}
		break;
	case STATE_LOAD:
		if (g_engine->_loadMenu->handleEvents(Event))
			return PS_LOAD;
		else if (back.handleEvents(Event) == BUAC_LCLICK)
			state = STATE_NORMAL;
		break;
	default:
		break;
	}

	return PS_NONE;
}
#endif

bool PauseMenu::disableHotkeys() {
	return (_state == STATE_SAVE && _save.disableHotkeys()) || (_state == STATE_OPTION && g_engine->_optionMenu->disableHotkeys());
}

void PauseMenu::setUI() {
	_bg.setUI();
	_menu.setUI();
	_save.setUI();
}

} // End of namespace Crab
