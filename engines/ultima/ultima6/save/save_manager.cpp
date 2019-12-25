/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima6/core/nuvie_defs.h"
#include "ultima/ultima6/conf/configuration.h"
#include "ultima/ultima6/actors/actor.h"
#include "ultima/ultima6/actors/actor_manager.h"
#include "ultima/ultima6/misc/u6_misc.h"
#include "ultima/ultima6/misc/u6_llist.h"
#include "ultima/ultima6/save/save_manager.h"
#include "ultima/ultima6/files/nuvie_io_file.h"
#include "ultima/ultima6/files/nuvie_file_list.h"

#include "ultima/ultima6/gui/gui.h"
//#include "ultima/ultima6/core/game.h"
#include "ultima/ultima6/core/event.h"
#include "ultima/ultima6/core/console.h"
#include "ultima/ultima6/save/save_dialog.h"
#include "ultima/ultima6/save/save_slot.h"
#include "ultima/ultima6/save/save_game.h"
#include "ultima/ultima6/core/msg_scroll.h"
#include "ultima/ultima6/files/utils.h"
////#include <direct.h>

#ifdef HAVE_JOYSTICK_SUPPORT
#include "ultima/ultima6/keybinding/keys.h"
#endif

#if defined __linux__ || MACOSX
//#include <sys/stat.h>
#endif

namespace Ultima {
namespace Ultima6 {

SaveManager::SaveManager(Configuration *cfg) {
	config = cfg;
	dialog = NULL;
	actor_manager = NULL;
	obj_manager = NULL;
	savegame = NULL;
	game_type = 0;
}

// setup the savedir variable.
// throws if the directory is not found or inaccessable
bool SaveManager::init() {
	std::string savedir_key;

	config->value("config/GameType", game_type);

	search_prefix.assign("nuvie");
	search_prefix.append(get_game_tag(game_type));

	savegame = new SaveGame(config);

	savedir_key = config_get_game_key(config);

	savedir_key.append("/savedir");

	config->value(savedir_key, savedir);

	if (savedir.size() == 0) {
		DEBUG(0, LEVEL_WARNING, "savedir config variable not found. Using current directory for saves!\n");
		savedir.assign("");
	}

	/*
	   if(directory_exists(savedir.c_str()) == false && !savedir.empty())
	       // try to create the save dir if it doesn't exist
	       mkdir(savedir.c_str());
	*/

	if (directory_exists(savedir.c_str()) == false) {
		//DEBUG(0,LEVEL_NOTIFICATION,"creating directory %s\n", savedir.c_str());
		ConsoleAddInfo("Creating savegame directory: \"%s\"", savedir.c_str());
		if (mkdir_recursive(savedir.c_str(), 0700) != 0) {
			DEBUG(0, LEVEL_ERROR, "savedir '%s' either not found or not accessible!\n", savedir.c_str());
			ConsoleAddError("Setting Save Directory!");
			return false;
		}
	}

	ConsoleAddInfo("Save dir: \"%s\"", savedir.c_str());

	return true;
}

SaveManager::~SaveManager() {
	delete savegame;
}

bool SaveManager::load_save() {
	bool newsave = false;

	config->value("config/newgame", newsave, false);

	if (newsave) {
		return savegame->load_new();
	}

	return load_latest_save();
}

bool SaveManager::load_latest_save() {
	NuvieFileList filelist;
	std::string *filename;
	std::string fullpath;

	if (filelist.open(savedir.c_str(), search_prefix.c_str(), NUVIE_SORT_TIME_DESC) == false) {
		ConsoleAddError("Opening " + savedir);
		return false;
	}
	filename = filelist.get_latest();

	filelist.close();

	if (filename != NULL)
		build_path(savedir, filename->c_str(), fullpath);

	if (!filename || savegame->load(fullpath.c_str()) == false) { //try to load the latest save
		if (savegame->load_original() == false) {       // fall back to savegame/ if no nuvie savegames exist.
			return savegame->load_new();                 // if all else fails try to load a new game.
		}
	}

	return true;
}

bool SaveManager::quick_save(int save_num, bool load) {
	if (save_num < 0 || save_num > 999)
		return false;
	Event *event = Game::get_game()->get_event();

	if (event->get_mode() == EQUIP_MODE)
		event->set_mode(MOVE_MODE);
	if (event->get_mode() != MOVE_MODE || Game::get_game()->user_paused())
		return false;

	char buf[6];
	std::string text;
	MsgScroll *scroll = Game::get_game()->get_scroll();
	if (load) {
		text = "loading quick save ";
		event->close_gumps();
	} else {
		if (Game::get_game()->is_armageddon()) {
			scroll->message("Can't save. You killed everyone!\n\n");
			return false;
		} else if (event->using_control_cheat()) {
			scroll->message(" Can't save while using control cheat\n\n");
			return false;
		}
		text = "saving quick save ";
	}
	snprintf(buf, 6, "%03d\n", save_num);
	text.append(buf);
	scroll->display_string(text);

	char end_buf[8]; // 000.sav\0
	snprintf(end_buf, 8, "%03d.sav", save_num);
	std::string save_name = "nuvie";

	save_name.append(get_game_tag(game_type));
	save_name.append("qs");
	save_name.append(end_buf);

	std::string fullpath;
	build_path(savedir, save_name.c_str(), fullpath);
	const char *fullpath_char = fullpath.c_str();

	if (load) {
		if (fileExists(fullpath_char)) {
			if (savegame->load(fullpath_char)) {
				return true;
			} else {
				scroll->message("\nfailed!\n\n");
				return false;
			}
		} else {
			scroll->display_string(save_name);
			scroll->message(" not found!\n\n");
			return false;
		}
	}

	return savegame->save(fullpath_char, &save_name); // always true
}

void SaveManager::create_dialog() {
	GUI *gui = GUI::get_gui();

	if (dialog == NULL) {
		dialog = new SaveDialog((GUI_CallBack *)this);
		dialog->init(savedir.c_str(), search_prefix.c_str());
		dialog->grab_focus();
		gui->AddWidget(dialog);
		gui->lock_input(dialog);
	}

	return;
}

bool SaveManager::load(SaveSlot *save_slot) {
	std::string save_filename;

	if (save_slot->get_filename()->size() == 0) {
		if (!save_slot->is_new_save() && savegame->load_original())
			return true;
		else
			return savegame->load_new();
	}

	build_path(savedir, save_slot->get_filename()->c_str(), save_filename);

	return savegame->load(save_filename.c_str());
}

bool SaveManager::save(SaveSlot *save_slot) {
	std::string save_filename;
	std::string save_fullpath;
	std::string save_desc;



	save_filename.assign(save_slot->get_filename()->c_str());

	if (save_filename.size() == 0)
		save_filename = get_new_savefilename();

	build_path(savedir, save_filename, save_fullpath);

	save_desc = save_slot->get_save_description();
	return savegame->save(save_fullpath.c_str(), &save_desc);
}

std::string SaveManager::get_new_savefilename() {
	uint32 max_count;
	uint32 count;
	std::string *filename;
	std::string new_filename;
	std::string searchPrefix;
	std::string num_str;
	char end_buf[8]; // 000.sav\0
	NuvieFileList filelist;

	max_count = 0;

	searchPrefix = "nuvie";
	searchPrefix.append(get_game_tag(game_type));

	new_filename = searchPrefix;

	filelist.open(savedir.c_str(), searchPrefix.c_str(), NUVIE_SORT_TIME_DESC);

	for (; (filename = filelist.next());) {
		// search for highest save number here.
		num_str = filename->substr(7, 3); //extract the number.
		count = atoi(num_str.c_str());
		if (count > max_count)
			max_count = count;
	}

	filelist.close();

	max_count++;

	snprintf(end_buf, 8, "%03d.sav", max_count);

	new_filename.append(end_buf);

	return new_filename;
}

GUI_status SaveManager::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	SaveSlot *save_slot;

	if (caller == dialog) {
		GUI_Dialog *gamemenu_dialog = Game::get_game()->get_event()->get_gamemenu_dialog();
		switch (msg) {
		case SAVEDIALOG_CB_DELETE :
			dialog = NULL;
			break;

		case SAVEDIALOG_CB_LOAD :
			save_slot = (SaveSlot *)data;
			if (load(save_slot) == false)
				return GUI_PASS;
#ifdef HAVE_JOYSTICK_SUPPORT
			Game::get_game()->get_keybinder()->set_enable_joy_repeat(true);
#endif
			if (gamemenu_dialog != NULL)
				gamemenu_dialog->callback(0, (GUI_CallBack *)gamemenu_dialog, (void *)gamemenu_dialog);
			//callback_object->callback(GameplayDialog_CB_DELETE, this, this);
			break;

		case SAVEDIALOG_CB_SAVE :
			save_slot = (SaveSlot *)data;
			if (save(save_slot) == false)
				return GUI_PASS;
			if (gamemenu_dialog != NULL)
				GUI::get_gui()->lock_input(gamemenu_dialog);
			break;
		default :
			break;
		}
	}

	return GUI_YUM;
}

} // End of namespace Ultima6
} // End of namespace Ultima
