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

#ifndef ULTIMA6_SAVE_SAVE_MANAGER_H
#define ULTIMA6_SAVE_SAVE_MANAGER_H

#include "ultima/shared/std/string.h"
#include <list>

#include "GUI_CallBack.h"

namespace Ultima {
namespace Ultima6 {

#define QUICK_LOAD true
#define QUICK_SAVE false

class Configuration;


class SaveDialog;
class SaveGame;
class SaveSlot;

class SaveManager : public GUI_CallBack {
	Configuration *config;
	ActorManager *actor_manager;
	ObjManager *obj_manager;

	int game_type;

	SaveGame *savegame;

	std::string savedir;
	std::string search_prefix; //eg. nuvieU6, nuvieMD or nuvieSE
// gui widgets;

	SaveDialog *dialog;

public:

	SaveManager(Configuration *cfg);
	virtual ~SaveManager();

	bool init();

	bool load_save();
	bool load_latest_save();

	void create_dialog();
	SaveDialog *get_dialog() {
		return dialog;
	}

	bool load(SaveSlot *save_slot);
	bool save(SaveSlot *save_slot);
	bool quick_save(int save_num, bool load);

	std::string get_new_savefilename();
	std::string get_savegame_directory() {
		return savedir;
	}

	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data);
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
