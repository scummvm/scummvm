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

#ifndef ULTIMA6_SAVE_SAVE_GAME_H
#define ULTIMA6_SAVE_SAVE_GAME_H

#define NUVIE_SAVE_VERSION_MAJOR 0
#define NUVIE_SAVE_VERSION_MINOR 3

#define NUVIE_SAVE_VERSION       NUVIE_SAVE_VERSION_MAJOR * 256 + NUVIE_SAVE_VERSION_MINOR

#define MAX_SAVE_DESC_LENGTH    52

#include "ultima/shared/std/string.h"
#include "SDL.h"

namespace Ultima {
namespace Ultima6 {

class Configuration;
class ActorManager;
class ObjManager;
class Actor;
class Map;
class NuvieIO;
class NuvieIOFileWrite;

struct SaveHeader {
	uint16 num_saves;
	uint32 actual_play_time; //total play time for this save game in minutes.
	uint32 game_play_time; //total play time for this save game measured in game turns.
	std::string save_description;
	std::string player_name;
	uint8 player_gender;
	uint8 str, intelligence, dex;
	uint16 level;
	uint16 exp;

	SDL_Surface *thumbnail;
	unsigned char *thumbnail_data;

	SaveHeader() {
		num_saves = 0;
		actual_play_time = 0;
		game_play_time = 0;
		player_gender = 0;
		str = 0;
		intelligence = 0;
		dex = 0;
		level = 0;
		exp = 0;
		thumbnail = NULL;
		thumbnail_data = NULL;
	};
};

class SaveGame {
	Configuration *config;

	SaveHeader header;

	NuvieIOBuffer objlist;

public:

	SaveGame(Configuration *cfg);
	~SaveGame();

	void init(ObjManager *obj_manager);

	bool load_new();
	bool load_original();
	SaveHeader *load_info(NuvieIOFileRead *loadfile);
	bool load(const char *filename);

	bool check_version(NuvieIOFileRead *loadfile);

	bool save(const char *filename, std::string *save_description);


	uint16 get_num_saves() {
		return header.num_saves;
	};

protected:

	bool load_objlist();
	bool save_objlist();
	bool save_thumbnail(NuvieIOFileWrite *savefile);

	void clean_up();

	void update_objlist_for_new_game();
	void update_objlist_for_new_game_u6();
	void update_objlist_for_new_game_se();
	void update_objlist_for_new_game_md();
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
