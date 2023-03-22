#pragma once

#include "FileData.h"
#include "ImageData.h"
#include "PageMenu.h"
#include "TextData.h"
#include "common_header.h"

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
	std::string selected;

	// The extension and directory used by this menu
	std::string extension, directory;

	// The save information for each slot
	std::vector<FileType> slot_info;
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
		std::string no_preview_path;

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

	std::string SelectedPath() { return selected; }
	void SelectedPath(const std::string &val) { selected = val; }

	void ScanDir() {
		using namespace boost::filesystem;

		slot_info.clear();
		menu.Clear();

		path filedir(directory);
		if (exists(filedir) && is_directory(filedir)) {
			directory_iterator dir_it(filedir);
			std::vector<path> file_in_dir;

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
	}

	void Load(rapidxml::xml_node<char> *node) {
		if (NodeValid("bg", node))
			bg.Load(node->first_node("bg"));

		if (NodeValid("menu", node))
			menu.Load(node->first_node("menu"));

		if (NodeValid("preview", node)) {
			auto prnode = node->first_node("preview");
			img.pos.Load(prnode);
			LoadStr(img.no_preview_path, "path", prnode);
		}

		if (NodeValid("offset", node)) {
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

		extension = gFilePath.save_ext;
		directory = gFilePath.appdata + gFilePath.save_dir;
		ScanDir();
	}

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

extern FileMenu<SaveFileData> gLoadMenu;
} // End of namespace ui
} // End of namespace pyrodactyl