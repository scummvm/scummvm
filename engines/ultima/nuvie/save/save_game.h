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

#ifndef NUVIE_SAVE_SAVE_GAME_H
#define NUVIE_SAVE_SAVE_GAME_H

#define SAVE_VERSION_MAJOR 1
#define SAVE_VERSION_MINOR 0
#define SAVE_VERSION (SAVE_VERSION_MAJOR * 256 + SAVE_VERSION_MINOR)

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/files/nuvie_io_file.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class ActorManager;
class ObjManager;
class Actor;
class Map;
class NuvieIO;
class NuvieIOFileWrite;

class SaveGame {
private:
	Configuration *config;
	NuvieIOBuffer objlist;
protected:
	bool load_objlist();
	bool save_objlist();

	void update_objlist_for_new_game();
	void update_objlist_for_new_game_u6();
	void update_objlist_for_new_game_se();
	void update_objlist_for_new_game_md();
public:
	SaveGame(Configuration *cfg);
	~SaveGame();

	void init(ObjManager *obj_manager);

	/**
	 * Sets up a new game
	 */
	bool load_new();

	/**
	 * Loads in the savegame created by the original Ultima 6
	 */
	bool load_original();

	/**
	 * Transfers in a character from a previous Ultima game
	 */
	bool transfer_character();

	/**
	 * Loads the savegame with the given name
	 */
	bool load(const Common::String &filename);

	bool check_version(NuvieIOFileRead *loadfile, uint16 gameType);

	bool save(const Common::String &filename, const Common::String &save_description, bool isAutosave);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
