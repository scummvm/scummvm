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

void GameSaveMenu::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid("name", node))
		ta_name.Load(node->first_node("name"));

	FileMenu<SaveFileData>::Load(node);
}

#if 0
void GameSaveMenu::AddButton(boost::filesystem::path p, unsigned int &slot_index, unsigned int &menu_index) {
	slot_info.push_back(SaveFileData(p));
	menu.Add(slot_index, menu_index);
}

void GameSaveMenu::ScanDir() {
	using namespace boost::filesystem;

	slot_info.clear();
	menu.Clear();

	path savedir(directory);

	if (!exists(savedir))
		create_directories(savedir);

	if (exists(savedir) && is_directory(savedir)) {
		directory_iterator dir_it(savedir);
		std::vector<boost::filesystem::path> file_in_dir;
		file_in_dir.clear();

		// Find all files in the save directory, sort them according to last modified
		std::copy(directory_iterator(savedir), directory_iterator(), std::back_inserter(file_in_dir));
		std::sort(file_in_dir.begin(), file_in_dir.end(), PathCompare);

		unsigned int count_slot = 0, count_menu = 0;

		// For the save menu, the first slot is a "blank" slot - to create a new save file
		AddButton(gFilePath.save_dir + "New Save" + gFilePath.save_ext, count_menu, count_slot);

		// Next, we must load all the files with the same extension as our save file
		for (auto i = file_in_dir.begin(); i != file_in_dir.end(); ++i)
			if (is_regular_file(*i) && i->extension().string() == gFilePath.save_ext)
				AddButton(*i, count_menu, count_slot);
	}

	menu.AssignPaths();
}

bool GameSaveMenu::HandleEvents(const SDL_Event &Event) {
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
		if (ta_name.HandleEvents(Event)) {
			if (index <= slot_info.size() && index != 0)
				boost::filesystem::remove(slot_info[index].path);
			selected = ta_name.text;
			state = STATE_NORMAL;
			Reset();
			return true;
		}

		if (gInput.Equals(IU_BACK, Event) == SDL_RELEASED) {
			ta_name.text = "New Save";
			state = STATE_NORMAL;
		}
	default:
		break;
	}

	return false;
}
#endif

bool GameSaveMenu::HandleEvents(const Common::Event &Event) {
	warning("STUB: GameSaveMenu::HandleEvents()");
	return false;
}

void GameSaveMenu::Draw() {
	bg.Draw();
	menu.Draw();
	for (auto i = menu.Index(), count = 0u; i < menu.IndexPlusOne() && i < slot_info.size(); i++, count++) {
		float base_x = menu.BaseX(count), base_y = menu.BaseY(count);
		td_b[DATA_LASTMODIFIED].Draw(slot_info[i].last_modified, base_x, base_y);

		if (i == index && state == STATE_NAME)
			ta_name.Draw();
		else
			td_b[DATA_SAVENAME].Draw(slot_info[i].name, base_x, base_y);
	}

	DrawHover();
}

void GameSaveMenu::SetUI() {
	FileMenu<SaveFileData>::SetUI();
	ta_name.SetUI();
}

} // End of namespace Crab
