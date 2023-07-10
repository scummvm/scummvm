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

#ifndef CRAB_FILEMENU_H
#define CRAB_FILEMENU_H

#include "common/savefile.h"

#include "crab/crab.h"
#include "crab/ui/FileData.h"
#include "crab/ui/ImageData.h"
#include "crab/ui/PageMenu.h"
#include "crab/ui/TextData.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
// Used for menus that are responsible for reading multiple files from disk
template<typename FileType>
class FileMenu {
protected:
	// The background of the menu
	ImageData bg;

	// The collection of buttons
	PageButtonMenu menu;

	// The final filename that is selected
	Common::String selected;

	// The extension and directory used by this menu
	Common::String extension, directory;

	// The save information for each slot
	Common::Array<FileType> slot_info;
	TextData td_b[DATA_BUTTON_TOTAL];

	// The titles for loc_name, difficulty, time_played and player_name
	HoverInfo hov[DATA_HOVER_TOTAL];
	TextData td_h[DATA_HOVER_TOTAL];

	// The preview picture details
	struct
	{
		// We load only the current preview image instead of all of them
		pyrodactyl::image::Image preview;

		// Fallback path if there is no preview image or if we fail to load it
		Common::String no_preview_path;

		// Position of image
		Element pos;

		// Is the image loaded
		bool loaded;
	} img;

	// Are we hovering over a button right now?
	bool hover;

	// The previously hover button
	int prev_hover;

public:
	FileMenu() {
		img.loaded = false;
		hover = false;
		prev_hover = -1;
	}

	~FileMenu() {
		if (img.loaded)
			img.preview.Delete();
	}
	void Reset() {
		if (img.loaded)
			img.preview.Delete();
		img.loaded = false;
		hover = false;
	}

	Common::String SelectedPath() { return selected; }
	void SelectedPath(const Common::String &val) { selected = val; }

	void ScanDir() {
		warning("STUB: FileMenu::ScanDir()");

		Common::String res = "CRAB_*";
		res += g_engine->_filePath->save_ext;
		Common::StringArray saves = g_engine->getSaveFileManager()->listSavefiles(res);

		slot_info.clear();
		menu.Clear();

		unsigned int count_slot = 0, count_menu = 0;
		for (const Common::String& save : saves) {
			slot_info.push_back(FileType(save));
			menu.Add(count_slot, count_menu);
		}

		menu.AssignPaths();

#if 0
		using namespace boost::filesystem;

		slot_info.clear();
		menu.Clear();

		path filedir(directory);
		if (exists(filedir) && is_directory(filedir)) {
			directory_iterator dir_it(filedir);
			Common::Array<path> file_in_dir;

			std::copy(directory_iterator(filedir), directory_iterator(), std::back_inserter(file_in_dir));
			std::sort(file_in_dir.begin(), file_in_dir.end(), PathCompare);

			// First, we must load all the files with the same extension as our save file
			unsigned int count_slot = 0, count_menu = 0;
			for (auto i = file_in_dir.begin(); i != file_in_dir.end(); ++i)
				if (is_regular_file(*i) && i->extension().string() == extension) {
					slot_info.push_back(FileType(*i));
					menu.Add(count_slot, count_menu);
				}
		} else
			create_directories(filedir);

		menu.AssignPaths();
#endif
	}

	void Load(rapidxml::xml_node<char> *node) {
		if (nodeValid("bg", node))
			bg.Load(node->first_node("bg"));

		if (nodeValid("menu", node))
			menu.Load(node->first_node("menu"));

		if (nodeValid("preview", node)) {
			auto prnode = node->first_node("preview");
			img.pos.Load(prnode);
			loadStr(img.no_preview_path, "path", prnode);
		}

		if (nodeValid("offset", node)) {
			rapidxml::xml_node<char> *offnode = node->first_node("offset");

			// Stuff displayed on the slot button
			td_b[DATA_SAVENAME].Load(offnode->first_node("save_name"));
			td_b[DATA_LASTMODIFIED].Load(offnode->first_node("last_modified"));

			// Stuff displayed when you hover over a slot button
			td_h[DATA_LOCNAME].Load(offnode->first_node("loc_name"));
			td_h[DATA_DIFFICULTY].Load(offnode->first_node("difficulty"));
			td_h[DATA_TIMEPLAYED].Load(offnode->first_node("time_played"));
			td_h[DATA_PLAYERNAME].Load(offnode->first_node("player_name"));

			// Titles for the stuff displayed when you hover over a slot button
			hov[DATA_LOCNAME].Load(offnode->first_node("loc_name_title"));
			hov[DATA_DIFFICULTY].Load(offnode->first_node("difficulty_title"));
			hov[DATA_TIMEPLAYED].Load(offnode->first_node("time_played_title"));
			hov[DATA_PLAYERNAME].Load(offnode->first_node("player_name_title"));
		}

		extension = g_engine->_filePath->save_ext;
		directory = (g_engine->_filePath->appdata + g_engine->_filePath->save_dir);
		ScanDir();
	}

	bool HandleEvents(const Common::Event &Event) {
		int choice = menu.HandleEvents(Event);
		if (choice >= 0) {
			menu.Reset();
			selected = slot_info[menu.Index() + choice].path;
			Reset();
			return true;
		}

		return false;
	}

#if 0
	bool HandleEvents(const SDL_Event &Event) {
		int choice = menu.HandleEvents(Event);
		if (choice >= 0) {
			menu.Reset();
			selected = slot_info[menu.Index() + choice].path;
			Reset();
			return true;
		}

		return false;
	}
#endif

	void Draw() {
		bg.Draw();
		menu.Draw();
		for (auto i = menu.Index(), count = 0u; i < menu.IndexPlusOne() && i < slot_info.size(); i++, count++) {
			auto base_x = menu.BaseX(count), base_y = menu.BaseY(count);
			td_b[DATA_SAVENAME].Draw(slot_info[i].name, base_x, base_y);
			td_b[DATA_LASTMODIFIED].Draw(slot_info[i].last_modified, base_x, base_y);
		}

		DrawHover();
	}

	void DrawHover() {
		if (menu.HoverIndex() >= 0) {
			int i = menu.HoverIndex();

			if (!img.loaded || prev_hover != i) {
				img.loaded = true;
				prev_hover = i;
				if (!img.preview.Load(slot_info[i].preview))
					img.preview.Load(img.no_preview_path);
			}

			hover = true;
			img.preview.Draw(img.pos.x, img.pos.y);

			td_h[DATA_LOCNAME].Draw(slot_info[i].loc_name);
			td_h[DATA_DIFFICULTY].Draw(slot_info[i].diff);
			td_h[DATA_TIMEPLAYED].Draw(slot_info[i].time);
			td_h[DATA_PLAYERNAME].Draw(slot_info[i].char_name);

			for (int num = 0; num < DATA_HOVER_TOTAL; ++num)
				hov[num].Draw();
		} else if (hover)
			Reset();
	}

	bool Empty() {
		ScanDir();
		return slot_info.empty();
	}

	bool SelectNewestFile() {
		if (slot_info.size() > 0) {
			selected = slot_info[0].path;
			return true;
		}

		return false;
	}

	void SetUI() {
		bg.SetUI();
		menu.SetUI();
		ScanDir();
		img.pos.SetUI();

		for (int i = 0; i < DATA_BUTTON_TOTAL; ++i)
			td_b[i].SetUI();

		for (int i = 0; i < DATA_HOVER_TOTAL; ++i) {
			td_h[i].SetUI();
			hov[i].SetUI();
		}
	}
};

} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_FILEMENU_H
