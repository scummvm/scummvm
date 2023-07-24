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

#ifndef CRAB_SAVEGAMEMENU_H
#define CRAB_SAVEGAMEMENU_H

#include "crab/ui/FileMenu.h"
#include "crab/common_header.h"
#include "crab/ui/textarea.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class GameSaveMenu : public FileMenu<SaveFileData> {
	enum State {
		STATE_NORMAL,
		STATE_NAME
	} _state;

	// This stores the name of the save slot
	TextArea _taName;

	// The index of the selected button
	int _index;

	void addButton(const Common::String &p, unsigned int &slotIndex, unsigned int &menuIndex);

public:
	GameSaveMenu() {
		_state = STATE_NORMAL;
		_index = 0;
	}

	~GameSaveMenu() {}

	void scanDir();
	bool disableHotkeys() {
		return _state == STATE_NAME;
	}

	void load(rapidxml::xml_node<char> *node);

	bool handleEvents(const Common::Event &Event);

	void draw();

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_SAVEGAMEMENU_H
