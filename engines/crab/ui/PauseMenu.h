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

#ifndef CRAB_PAUSEMENU_H
#define CRAB_PAUSEMENU_H

#include "crab/ui/FileMenu.h"
#include "crab/event/GameEventInfo.h"
#include "crab/ui/OptionMenu.h"
#include "crab/ui/SaveGameMenu.h"
#include "crab/ui/SlideShow.h"
#include "crab/ui/menu.h"
#include "crab/ui/slider.h"
#include "crab/ui/textarea.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
enum PauseSignal {
	PS_NONE,
	PS_RESUME,
	PS_SAVE,
	PS_LOAD,
	PS_HELP,
	PS_QUIT_MENU,
	PS_QUIT_GAME
};

class PauseMenu {
	enum PauseState {
		STATE_NORMAL,
		STATE_SAVE,
		STATE_OPTION,
		STATE_LOAD
	} _state;

	// The pause menu background
	ImageData _bg;

	// The buttons in the menu
	ButtonMenu _menu;

	// Save game menu
	GameSaveMenu _save;

	// The selected main menu button
	int _choice;

public:
	PauseMenu() {
		_state = STATE_NORMAL;
		_choice = -1;
	}

	~PauseMenu() {}

	void updateMode(const bool &ironman) {
		_menu._element[PS_SAVE - 1]._visible = !ironman;
		_menu._element[PS_LOAD - 1]._visible = !ironman;
	}

	void load(rapidxml::xml_node<char> *node);

	PauseSignal handleEvents(const Common::Event &event, Button &back);

	// Returns true if inside options menu, false otherwise
	bool draw(Button &back);

	void reset() {
		_state = STATE_NORMAL;
	}

	void scanDir() {
		_save.scanDir();
	}

	Common::String saveFile() {
		return _save.selectedPath();
	}

	bool disableHotkeys();

	// Should we allow the pause key(default escape) to quit to main menu?
	// This is done because esc is both the "go back on menu level" and the pause key
	bool showLevel() {
		return _state == STATE_NORMAL;
	}

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_PAUSEMENU_H
