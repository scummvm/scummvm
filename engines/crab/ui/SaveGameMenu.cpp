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
		ta_name.load(node->first_node("name"));

	FileMenu<SaveFileData>::load(node);
}

void GameSaveMenu::AddButton(const Common::String &p, unsigned int &slot_index, unsigned int &menu_index) {
	slot_info.push_back(SaveFileData(p));
	menu.Add(slot_index, menu_index);
}

void GameSaveMenu::ScanDir() {
	Common::String res = "CRAB_*";
	res += g_engine->_filePath->save_ext;
	Common::StringArray saves = g_engine->getSaveFileManager()->listSavefiles(res);

	slot_info.clear();
	menu.Clear();

	unsigned int count_slot = 0, count_menu = 0;

	// For the save menu, the first slot is a "blank" slot - to create a new save file
	AddButton("CRAB_New Save" + g_engine->_filePath->save_ext, count_menu, count_slot);

	for (const Common::String& save : saves) {
		AddButton(save, count_menu, count_slot);
	}

	menu.AssignPaths();
}

bool GameSaveMenu::HandleEvents(const Common::Event &Event) {
	int choice = -1;
	switch (state) {
	case STATE_NORMAL:
		choice = menu.HandleEvents(Event);
		if (choice >= 0) {
			ta_name.x = menu.CurX(choice) + td_b[DATA_SAVENAME].x;
			ta_name.y = menu.CurY(choice) + td_b[DATA_SAVENAME].y;

			index = menu.Index() + choice;

			if (index != 0)
				ta_name.text = slot_info[index].name;
			else
				ta_name.text = "";

			state = STATE_NAME;
		}
		break;
	case STATE_NAME:
		if (g_engine->_inputManager->GetKeyBindingMode() != input::KBM_UI)
			g_engine->_inputManager->SetKeyBindingMode(KBM_UI);

		if (ta_name.HandleEvents(Event)) {
			if (index <= (int)slot_info.size() && index != 0)
				g_engine->getSaveFileManager()->removeSavefile(slot_info[index].path);

			selected = ta_name.text;
			state = STATE_NORMAL;
			reset();
			g_engine->_inputManager->SetKeyBindingMode(KBM_GAME);
			return true;
		}

		if (g_engine->_inputManager->State(IU_BACK)) {
			ta_name.text = "New Save";
			state = STATE_NORMAL;
		}
	default:
		break;
	}

	return false;
}

void GameSaveMenu::draw() {
	bg.draw();
	menu.draw();
	for (auto i = menu.Index(), count = 0u; i < menu.IndexPlusOne() && i < slot_info.size(); i++, count++) {
		float base_x = menu.BaseX(count), base_y = menu.BaseY(count);
		td_b[DATA_LASTMODIFIED].draw(slot_info[i].last_modified, base_x, base_y);

		if (i == (unsigned int)index && state == STATE_NAME)
			ta_name.draw();
		else
			td_b[DATA_SAVENAME].draw(slot_info[i].name, base_x, base_y);
	}

	DrawHover();
}

void GameSaveMenu::SetUI() {
	FileMenu<SaveFileData>::SetUI();
	ta_name.SetUI();
}

} // End of namespace Crab
