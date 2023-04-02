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

void PauseMenu::Load(rapidxml::xml_node<char> *node) {
	menu.Load(node->first_node("menu"));
	save.Load(node->first_node("save"));

	if (NodeValid("bg", node))
		bg.Load(node->first_node("bg"));
}

bool PauseMenu::Draw(Button &back) {
	switch (state) {
	case STATE_NORMAL:
		bg.Draw();
		menu.Draw();
		break;
	case STATE_SAVE:
		save.Draw();
		back.Draw();
		break;
	case STATE_LOAD:
		gLoadMenu.Draw();
		back.Draw();
		break;
	case STATE_OPTION:
		gOptionMenu.Draw(back);
		return true;
	default:
		break;
	}

	return false;
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
				gLoadMenu.ScanDir();
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
		if (gOptionMenu.HandleEvents(back, Event)) {
			gOptionMenu.Reset();
			state = STATE_NORMAL;
		}
		break;
	case STATE_LOAD:
		if (gLoadMenu.HandleEvents(Event))
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
	return (state == STATE_SAVE && save.DisableHotkeys()) || (state == STATE_OPTION && gOptionMenu.DisableHotkeys());
}

void PauseMenu::SetUI() {
	bg.SetUI();
	menu.SetUI();
	save.SetUI();
}

} // End of namespace Crab
