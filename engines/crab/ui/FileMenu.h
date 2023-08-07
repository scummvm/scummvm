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

namespace Crab {

namespace pyrodactyl {
namespace ui {
// Used for menus that are responsible for reading multiple files from disk
template<typename FileType>
class FileMenu {
protected:
	// The background of the menu
	ImageData _bg;

	// The collection of buttons
	PageButtonMenu _menu;

	// The final filename that is selected
	Common::String _selected;

	// The extension and directory used by this menu
	Common::String _extension, _directory;

	// The save information for each slot
	Common::Array<FileType> _slotInfo;
	TextData tdB[DATA_BUTTON_TOTAL];

	// The titles for loc_name, difficulty, time_played and player_name
	HoverInfo hov[DATA_HOVER_TOTAL];
	TextData tdH[DATA_HOVER_TOTAL];

	// The preview picture details
	struct
	{
		// We load only the current preview image instead of all of them
		pyrodactyl::image::Image _preview;

		// Fallback path if there is no preview image or if we fail to load it
		Common::String _noPreviewPath;

		// Position of image
		Element _pos;

		// Is the image loaded
		bool _loaded;
	} _img;

	// Are we hovering over a button right now?
	bool _hover;

	// The previously hover button
	int _prevHover;

public:
	FileMenu() {
		_img._loaded = false;
		_hover = false;
		_prevHover = -1;
	}

	~FileMenu() {
		if (_img._loaded)
			_img._preview.deleteImage();
	}
	void reset() {
		if (_img._loaded)
			_img._preview.deleteImage();
		_img._loaded = false;
		_hover = false;
	}

	Common::String selectedPath() {
		return _selected;
	}

	void selectedPath(const Common::String &val) {
		_selected = val;
	}

	void scanDir() {
		warning("STUB: FileMenu::scanDir()");

		Common::String res = "CRAB_*";
		res += g_engine->_filePath->_saveExt;
		Common::StringArray saves = g_engine->getSaveFileManager()->listSavefiles(res);

		_slotInfo.clear();
		_menu.clear();

		uint countSlot = 0, countMenu = 0;
		for (const Common::String& save : saves) {
			_slotInfo.push_back(FileType(save));
			_menu.add(countSlot, countMenu);
		}

		_menu.assignPaths();

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
			uint count_slot = 0, count_menu = 0;
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

	void load(rapidxml::xml_node<char> *node) {
		if (nodeValid("bg", node))
			_bg.load(node->first_node("bg"));

		if (nodeValid("menu", node))
			_menu.load(node->first_node("menu"));

		if (nodeValid("preview", node)) {
			auto prnode = node->first_node("preview");
			_img._pos.load(prnode);
			loadStr(_img._noPreviewPath, "path", prnode);
		}

		if (nodeValid("offset", node)) {
			rapidxml::xml_node<char> *offnode = node->first_node("offset");

			// Stuff displayed on the slot button
			tdB[DATA_SAVENAME].load(offnode->first_node("save_name"));
			tdB[DATA_LASTMODIFIED].load(offnode->first_node("last_modified"));

			// Stuff displayed when you hover over a slot button
			tdH[DATA_LOCNAME].load(offnode->first_node("loc_name"));
			tdH[DATA_DIFFICULTY].load(offnode->first_node("difficulty"));
			tdH[DATA_TIMEPLAYED].load(offnode->first_node("time_played"));
			tdH[DATA_PLAYERNAME].load(offnode->first_node("player_name"));

			// Titles for the stuff displayed when you hover over a slot button
			hov[DATA_LOCNAME].load(offnode->first_node("loc_name_title"));
			hov[DATA_DIFFICULTY].load(offnode->first_node("difficulty_title"));
			hov[DATA_TIMEPLAYED].load(offnode->first_node("time_played_title"));
			hov[DATA_PLAYERNAME].load(offnode->first_node("player_name_title"));
		}

		_extension = g_engine->_filePath->_saveExt;
		_directory = (g_engine->_filePath->_appdata + g_engine->_filePath->_saveDir);
		scanDir();
	}

	bool handleEvents(const Common::Event &event) {
		int choice = _menu.handleEvents(event);
		if (choice >= 0) {
			_menu.reset();
			_selected = _slotInfo[_menu.index() + choice]._path;
			reset();
			return true;
		}

		return false;
	}

	void draw() {
		_bg.draw();
		_menu.draw();
		for (auto i = _menu.index(), count = 0u; i < _menu.indexPlusOne() && i < _slotInfo.size(); i++, count++) {
			auto base_x = _menu.baseX(count), base_y = _menu.baseY(count);
			tdB[DATA_SAVENAME].draw(_slotInfo[i]._name, base_x, base_y);
			tdB[DATA_LASTMODIFIED].draw(_slotInfo[i]._lastModified, base_x, base_y);
		}

		DrawHover();
	}

	void DrawHover() {
		if (_menu.hoverIndex() >= 0) {
			int i = _menu.hoverIndex();

			if (!_img._loaded || _prevHover != i) {
				_img._loaded = true;
				_prevHover = i;
				if (!_img._preview.load(_slotInfo[i]._preview))
					_img._preview.load(_img._noPreviewPath);
			}

			_hover = true;
			_img._preview.draw(_img._pos.x, _img._pos.y);

			tdH[DATA_LOCNAME].draw(_slotInfo[i]._locName);
			tdH[DATA_DIFFICULTY].draw(_slotInfo[i]._diff);
			tdH[DATA_TIMEPLAYED].draw(_slotInfo[i]._time);
			tdH[DATA_PLAYERNAME].draw(_slotInfo[i]._charName);

			for (int num = 0; num < DATA_HOVER_TOTAL; ++num)
				hov[num].draw();
		} else if (_hover)
			reset();
	}

	bool empty() {
		scanDir();
		return _slotInfo.empty();
	}

	bool selectNewestFile() {
		if (_slotInfo.size() > 0) {
			_selected = _slotInfo[0]._path;
			return true;
		}

		return false;
	}

	void setUI() {
		_bg.setUI();
		_menu.setUI();
		scanDir();
		_img._pos.setUI();

		for (int i = 0; i < DATA_BUTTON_TOTAL; ++i)
			tdB[i].setUI();

		for (int i = 0; i < DATA_HOVER_TOTAL; ++i) {
			tdH[i].setUI();
			hov[i].setUI();
		}
	}
};

} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_FILEMENU_H
