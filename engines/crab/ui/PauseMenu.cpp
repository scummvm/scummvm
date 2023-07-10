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
	menu.load(node->first_node("menu"));
	save.load(node->first_node("save"));

	if (nodeValid("bg", node))
		bg.load(node->first_node("bg"));
}

bool PauseMenu::draw(Button &back) {
	switch (state) {
	case STATE_NORMAL:
		bg.draw();
		menu.draw();
		break;
	case STATE_OPTION:
		g_engine->_optionMenu->draw(back);
		return true;
	default:
		break;
	}

	return false;
}

PauseSignal PauseMenu::HandleEvents(const Common::Event &Event, Button &back) {
	switch (state) {
	case STATE_NORMAL:
		choice = menu.HandleEvents(Event);
		if (choice == -1) {
			if (back.hotkey.HandleEvents(Event))
				return PS_RESUME;
		} else {
			switch (choice) {
			case 0:
				state = STATE_NORMAL;
				return PS_RESUME;
			case 1:
				if (g_engine->saveGameDialog()) {
					state = STATE_NORMAL;
					return PS_SAVE;
				} else
					state = STATE_NORMAL;
				break;
			case 2:
				if (g_engine->loadGameDialog()) {
					state = STATE_NORMAL;
					return PS_LOAD;
				} else
					state = STATE_NORMAL;
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
	case STATE_OPTION:
		if (g_engine->_optionMenu->HandleEvents(back, Event)) {
			g_engine->_optionMenu->reset();
			state = STATE_NORMAL;
		}
		break;
	case STATE_LOAD:
		if (g_engine->loadGameDialog())
			return PS_LOAD;
		else
			state = STATE_NORMAL;
		break;
	default:
		break;
	}

	return PS_NONE;
}

#if 0
PauseSignal PauseMenu::HandleEvents(const SDL_Event &Event, Button &back) {
	switch (state) {
	case STATE_NORMAL:
		choice = menu.HandleEvents(Event);
		if (choice == -1) {
			if (back.hotkey.HandleEvents(Event))
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
		if (save.HandleEvents(Event)) {
			state = STATE_NORMAL;
			return PS_SAVE;
		} else if (back.HandleEvents(Event) == BUAC_LCLICK && !save.DisableHotkeys())
			state = STATE_NORMAL;
		break;
	case STATE_OPTION:
		if (g_engine->_optionMenu->HandleEvents(back, Event)) {
			g_engine->_optionMenu->reset();
			state = STATE_NORMAL;
		}
		break;
	case STATE_LOAD:
		if (g_engine->_loadMenu->HandleEvents(Event))
			return PS_LOAD;
		else if (back.HandleEvents(Event) == BUAC_LCLICK)
			state = STATE_NORMAL;
		break;
	default:
		break;
	}

	return PS_NONE;
}
#endif

bool PauseMenu::DisableHotkeys() {
	return (state == STATE_SAVE && save.DisableHotkeys()) || (state == STATE_OPTION && g_engine->_optionMenu->DisableHotkeys());
}

void PauseMenu::setUI() {
	bg.setUI();
	menu.setUI();
	save.setUI();
}

} // End of namespace Crab
