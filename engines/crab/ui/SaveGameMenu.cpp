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

#include "crab/ui/SaveGameMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::input;

void GameSaveMenu::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("name", node))
		_taName.load(node->first_node("name"));

	FileMenu<SaveFileData>::load(node);
}

void GameSaveMenu::addButton(const Common::String &p, unsigned int &slotIndex, unsigned int &menuIndex) {
	_slotInfo.push_back(SaveFileData(p));
	_menu.add(slotIndex, menuIndex);
}

void GameSaveMenu::scanDir() {
	Common::String res = "CRAB_*";
	res += g_engine->_filePath->save_ext;
	Common::StringArray saves = g_engine->getSaveFileManager()->listSavefiles(res);

	_slotInfo.clear();
	_menu.clear();

	unsigned int countSlot = 0, countMenu = 0;

	// For the save menu, the first slot is a "blank" slot - to create a new save file
	addButton("CRAB_New Save" + g_engine->_filePath->save_ext, countMenu, countSlot);

	for (const Common::String& save : saves) {
		addButton(save, countMenu, countSlot);
	}

	_menu.assignPaths();
}

bool GameSaveMenu::handleEvents(const Common::Event &Event) {
	int choice = -1;
	switch (_state) {
	case STATE_NORMAL:
		choice = _menu.handleEvents(Event);
		if (choice >= 0) {
			_taName.x = _menu.curX(choice) + tdB[DATA_SAVENAME].x;
			_taName.y = _menu.curY(choice) + tdB[DATA_SAVENAME].y;

			_index = _menu.index() + choice;

			if (_index != 0)
				_taName._text = _slotInfo[_index]._name;
			else
				_taName._text = "";

			_state = STATE_NAME;
		}
		break;
	case STATE_NAME:
		if (g_engine->_inputManager->getKeyBindingMode() != input::KBM_UI)
			g_engine->_inputManager->setKeyBindingMode(KBM_UI);

		if (_taName.handleEvents(Event)) {
			if (_index <= (int)_slotInfo.size() && _index != 0)
				g_engine->getSaveFileManager()->removeSavefile(_slotInfo[_index]._path);

			_selected = _taName._text;
			_state = STATE_NORMAL;
			reset();
			g_engine->_inputManager->setKeyBindingMode(KBM_GAME);
			return true;
		}

		if (g_engine->_inputManager->state(IU_BACK)) {
			_taName._text = "New Save";
			_state = STATE_NORMAL;
		}
	default:
		break;
	}

	return false;
}

void GameSaveMenu::draw() {
	_bg.draw();
	_menu.draw();
	for (auto i = _menu.index(), count = 0u; i < _menu.indexPlusOne() && i < _slotInfo.size(); i++, count++) {
		float base_x = _menu.baseX(count), base_y = _menu.baseY(count);
		tdB[DATA_LASTMODIFIED].draw(_slotInfo[i]._lastModified, base_x, base_y);

		if (i == (unsigned int)_index && _state == STATE_NAME)
			_taName.draw();
		else
			tdB[DATA_SAVENAME].draw(_slotInfo[i]._name, base_x, base_y);
	}

	DrawHover();
}

void GameSaveMenu::setUI() {
	FileMenu<SaveFileData>::setUI();
	_taName.setUI();
}

} // End of namespace Crab
