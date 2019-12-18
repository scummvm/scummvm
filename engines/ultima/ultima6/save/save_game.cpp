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

//#include <list>
//#include <cassert>



#include "ultima/ultima6/core/nuvie_defs.h"
#include "ultima/ultima6/misc/u6_misc.h"
#include "ultima/ultima6/files/nuvie_io.h"
#include "ultima/ultima6/files/nuvie_io_file.h"
#include "ultima/ultima6/files/u6_lzw.h"
#include "ultima/ultima6/gui/gui.h"
#include "ultima/ultima6/core/console.h"
#include "ultima/ultima6/save/save_game.h"
#include "ultima/ultima6/conf/configuration.h"
#include "ultima/ultima6/core/game.h"
#include "ultima/ultima6/core/obj_manager.h"
#include "ultima/ultima6/save/obj_list.h"
#include "ultima/ultima6/actors/actor_manager.h"
#include "ultima/ultima6/core/egg_manager.h"
#include "ultima/ultima6/actors/actor.h"
#include "ultima/ultima6/views/view_manager.h"
#include "ultima/ultima6/core/map_window.h"
#include "ultima/ultima6/core/msg_scroll.h"
#include "ultima/ultima6/core/party.h"
#include "ultima/ultima6/core/player.h"
#include "ultima/ultima6/portraits/portrait.h"
#include "ultima/ultima6/core/game_clock.h"
#include "ultima/ultima6/core/command_bar.h"
#include "ultima/ultima6/core/weather.h"
#include "ultima/ultima6/script/script.h"
#include "ultima/ultima6/core/event.h"

namespace Ultima {
namespace Ultima6 {

#ifdef WIN32
#define OBJLIST_FILENAME "savegame\\objlist"
#define OBJBLK_FILENAME  "savegame\\objblkxx"
#else
#define OBJLIST_FILENAME "savegame/objlist"
#define OBJBLK_FILENAME  "savegame/objblkxx"
#endif

SaveGame::SaveGame(Configuration *cfg) {
	config = cfg;
	init(NULL); //we don't need ObjManager here as there will be nothing to clean at this stage. :-)
}

SaveGame::~SaveGame() {
	objlist.close();
	clean_up();
}

void SaveGame::init(ObjManager *obj_manager) {
	header.save_description.assign("");

	if (objlist.get_size() > 0)
		objlist.close();

	if (obj_manager)
		obj_manager->clean();

	return;
}


bool SaveGame::load_new() {
	std::string filename;
	U6Lzw lzw;
	NuvieIOBuffer buf;
	unsigned char *data;
	uint32 decomp_size;
	ObjManager *obj_manager;
	uint8 i;
	uint32 pos;

	obj_manager = Game::get_game()->get_obj_manager();

	init(obj_manager);

// load surface chunks

	config_get_path(config, "lzobjblk", filename);
	data = lzw.decompress_file(filename, decomp_size);

	buf.open(data, decomp_size, NUVIE_BUF_NOCOPY);

	for (i = 0; i < 64; i++)
		obj_manager->load_super_chunk(&buf, 0, i);

	buf.close();
	free(data);

// load dungeon chunks

	config_get_path(config, "lzdngblk", filename);
	data = lzw.decompress_file(filename, decomp_size);

	buf.open(data, decomp_size, NUVIE_BUF_NOCOPY);

	for (i = 0; i < 5; i++)
		obj_manager->load_super_chunk(&buf, i, 0);

	pos = buf.position();
	buf.close();

// load objlist

	objlist.open(&data[pos], decomp_size - pos, NUVIE_BUF_COPY);

	update_objlist_for_new_game();

	load_objlist();

	Actor *player = Game::get_game()->get_player()->get_actor();
	Game::get_game()->get_egg_manager()->spawn_eggs(player->get_x(), player->get_y(), player->get_z(), true);

	free(data);

	return true;
}

bool SaveGame::load_original() {
	std::string path, objlist_filename, objblk_filename;
	unsigned char *data;
//char *filename;
	char x, y;
	uint16 len;
	uint8 i;
	NuvieIOFileRead *objblk_file;
	NuvieIOFileRead objlist_file;
	ObjManager *obj_manager;

	objblk_file = new NuvieIOFileRead();

	obj_manager = Game::get_game()->get_obj_manager();

	init(obj_manager);

	objblk_filename = OBJBLK_FILENAME;
	len = objblk_filename.length();

	i = 0;

	for (y = 'a'; y < 'i'; y++) {
		for (x = 'a'; x < 'i'; x++) {
			objblk_filename[len - 1] = y;
			objblk_filename[len - 2] = x;
			ConsoleAddInfo("Loading file: %s", objblk_filename.c_str());
			config_get_path(config, objblk_filename, path);
			if (objblk_file->open(path) == false) {
				delete objblk_file;
				return false;
			}

			if (obj_manager->load_super_chunk((NuvieIO *)objblk_file, 0, i) == false) {
				delete objblk_file;
				return false;
			}
			i++;
			objblk_file->close();
		}
	}

	objblk_filename[len - 1] = 'i';

	for (i = 0, x = 'a'; x < 'f'; x++, i++) { //Load dungeons
		objblk_filename[len - 2] = x;
		config_get_path(config, objblk_filename, path);
		objblk_file->open(path);

		if (obj_manager->load_super_chunk((NuvieIO *)objblk_file, i, 0) == false) {
			delete objblk_file;
			return false;
		}

		objblk_file->close();
	}

	delete objblk_file;

//print_egg_list();
	config_get_path(config, OBJLIST_FILENAME, objlist_filename);
	if (objlist_file.open(objlist_filename) == false)
		return false;

	data = objlist_file.readAll();

	objlist.open(data, objlist_file.get_size(), NUVIE_BUF_COPY);
	free(data);

	load_objlist();

	return true;
}


bool SaveGame::load_objlist() {
	Game *game;
	GameClock *clock;
	ActorManager *actor_manager;
	ObjManager *obj_manager;
	ViewManager *view_manager;
	MapWindow *map_window;
	MsgScroll *scroll;
	CommandBar *command_bar;
	Player *player;
	Party *party;
	Portrait *portrait;
	Weather *weather;
	uint16 px, py;
	uint8 pz;

	game = Game::get_game();

	clock = game->get_clock();
	actor_manager = game->get_actor_manager();
	obj_manager = game->get_obj_manager();
	scroll = game->get_scroll();
	map_window = game->get_map_window();
	command_bar = game->get_command_bar();

	player = game->get_player();
	party = game->get_party();
	portrait = game->get_portrait();
	view_manager = game->get_view_manager();
	weather = game->get_weather();

	portrait->load(&objlist); //load avatar portrait number.

	clock->load(&objlist);
	game->set_ethereal(false); // needs to go before actor_manager->load(&objlist);
	actor_manager->load(&objlist);

	party->load(&objlist);
	player->load(&objlist);

	weather->load(&objlist);

	command_bar->set_combat_mode(party->is_in_combat_mode()); // update CommandBar
	command_bar->load(&objlist);

	view_manager->reload();

	game->get_script()->call_load_game(&objlist);
	game->get_event()->set_control_cheat(false);

	player->get_location(&px, &py, &pz);
	obj_manager->update(px, py, pz); // spawn eggs.

	map_window->centerMapOnActor(player->get_actor());

	scroll->display_string("\nGame Loaded\n\n");

	scroll->init((char *)player->get_name());

	scroll->display_prompt();

	return true;
}

SaveHeader *SaveGame::load_info(NuvieIOFileRead *loadfile) {
#ifdef TODO
	uint32 rmask, gmask, bmask;
	unsigned char save_desc[MAX_SAVE_DESC_LENGTH + 1];
	unsigned char player_name[14];

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0x00ff0000;
	gmask = 0x0000ff00;
	bmask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
#endif

	clean_up();

	loadfile->seek(15); //skip version, textual id string and game tag

	header.num_saves = loadfile->read2();

	loadfile->readToBuf(save_desc, MAX_SAVE_DESC_LENGTH);
	save_desc[MAX_SAVE_DESC_LENGTH] = '\0';
	header.save_description.assign((const char *)save_desc);

	loadfile->readToBuf(player_name, 14);
	header.player_name.assign((const char *)player_name);

	header.player_gender = loadfile->read1();

	header.level = loadfile->read1();
	header.str = loadfile->read1();
	header.dex = loadfile->read1();
	header.intelligence = loadfile->read1();
	header.exp = loadfile->read2();

//should we load the thumbnail here!?

	header.thumbnail_data = new unsigned char[MAPWINDOW_THUMBNAIL_SIZE * MAPWINDOW_THUMBNAIL_SIZE * 3];

	loadfile->readToBuf(header.thumbnail_data, MAPWINDOW_THUMBNAIL_SIZE * MAPWINDOW_THUMBNAIL_SIZE * 3); //seek past thumbnail data.

	header.thumbnail = SDL_CreateRGBSurfaceFrom(header.thumbnail_data, MAPWINDOW_THUMBNAIL_SIZE, MAPWINDOW_THUMBNAIL_SIZE, 24, MAPWINDOW_THUMBNAIL_SIZE * 3, rmask, gmask, bmask, 0);

	return &header;
#else
	return nullptr;
#endif
}

bool SaveGame::check_version(NuvieIOFileRead *loadfile) {
	uint16 version;

	loadfile->seekStart();

	version = loadfile->read2();
	if (version != NUVIE_SAVE_VERSION) {
		DEBUG(0, LEVEL_ERROR, "Incompatible savegame version. Savegame version '%d', current system version '%d'\n", version, NUVIE_SAVE_VERSION);
		return false;
	}

	return true;
}

bool SaveGame::load(const char *filename) {
	uint8 i;
	uint32 objlist_size;
	uint32 bytes_read;
	NuvieIOFileRead *loadfile;
	unsigned char *data;
	int game_type;
//char game_tag[3];
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();

	config->value("config/GameType", game_type);

	loadfile = new NuvieIOFileRead();

	if (loadfile->open(filename) == false) {
		delete loadfile;
		return false;
	}

	ConsoleAddInfo("Loading Game: %s", filename);
	DEBUG(0, LEVEL_NOTIFICATION, "Loading Game: %s\n", filename);

	if (!check_version(loadfile)) {
		DEBUG(0, LEVEL_NOTIFICATION, "version incorrect\n");
		delete loadfile;
		return false;
	}

	init(obj_manager); // needs to come after checking for failure

	load_info(loadfile); //load header info

// load actor inventories
	obj_manager->load_super_chunk((NuvieIO *)loadfile, 0, 0);

// load eggs
	obj_manager->load_super_chunk((NuvieIO *)loadfile, 0, 0);

// load surface objects
	for (i = 0; i < 64; i++) {
		ConsoleAddInfo("Loading super chunk %d of 64", i + 1);
		obj_manager->load_super_chunk((NuvieIO *)loadfile, 0, i);
	}

// load dungeon objects
	for (i = 0; i < 5; i++) {
		obj_manager->load_super_chunk((NuvieIO *)loadfile, i + 1, 0);
	}

	objlist_size = loadfile->get_size() - loadfile->position();

	data = loadfile->readBuf(objlist_size, &bytes_read);

	objlist.open(data, objlist_size, NUVIE_BUF_COPY);

	free(data);
	loadfile->close();
	delete loadfile;

	load_objlist();

	return true;
}

bool SaveGame::save(const char *filename, std::string *save_description) {
	uint8 i;
	NuvieIOFileWrite *savefile;
	int game_type;
	char game_tag[3];
	unsigned char player_name[14];
	unsigned char save_desc[MAX_SAVE_DESC_LENGTH + 1];
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();
	Player *player = Game::get_game()->get_player();
	Actor *avatar = Game::get_game()->get_actor_manager()->get_actor(1); // get the avatar actor.

	config->value("config/GameType", game_type);
	bool newgame;
	config->value("config/newgame", newgame, false);
	if (newgame) {
		config->set("config/newgame", false);
		config->write();
	}

	savefile = new NuvieIOFileWrite();

	savefile->open(filename);

	savefile->write2(NUVIE_SAVE_VERSION);
	savefile->writeBuf((const unsigned char *)"Nuvie Save", 11);


	switch (game_type) {
	case NUVIE_GAME_U6 :
		strcpy(game_tag, "U6");
		break;

	case NUVIE_GAME_MD :
		strcpy(game_tag, "MD");
		break;

	case NUVIE_GAME_SE :
		strcpy(game_tag, "SE");
		break;
	}

	savefile->writeBuf((const unsigned char *)game_tag, 2);

	header.num_saves++;
	savefile->write2(header.num_saves);

	memset(save_desc, 0, MAX_SAVE_DESC_LENGTH);
	strncpy((char *)save_desc, save_description->c_str(), MAX_SAVE_DESC_LENGTH);
	save_desc[MAX_SAVE_DESC_LENGTH] = '\0';
	savefile->writeBuf(save_desc, MAX_SAVE_DESC_LENGTH);

	memset(player_name, 0, 14);
	strcpy((char *)player_name, (const char *)player->get_name());
	savefile->writeBuf((const unsigned char *)player_name, 14);

	savefile->write1(player->get_gender());

	savefile->write1(avatar->get_level());
	savefile->write1(avatar->get_strength());
	savefile->write1(avatar->get_dexterity());
	savefile->write1(avatar->get_intelligence());
	savefile->write2(avatar->get_exp());

	save_thumbnail(savefile);

	obj_manager->save_inventories(savefile);

	obj_manager->save_eggs(savefile);

// save surface objects
	for (i = 0; i < 64; i++)
		obj_manager->save_super_chunk(savefile, 0, i);

// save dungeon objects
	for (i = 0; i < 5; i++)
		obj_manager->save_super_chunk(savefile, i + 1, 0);

	save_objlist();

	savefile->writeBuf(objlist.get_raw_data(), objlist.get_size());

	savefile->close();

	delete savefile;

	return true;
}

bool SaveGame::save_objlist() {
	Game *game;
	GameClock *clock;
	ActorManager *actor_manager;
	Player *player;
	Party *party;
	MsgScroll *scroll;
	Weather *weather;

	game = Game::get_game();

	clock = game->get_clock();
	actor_manager = game->get_actor_manager();

	player = game->get_player();
	party = game->get_party();
	scroll = game->get_scroll();
	weather = game->get_weather();

	clock->save(&objlist);
	actor_manager->save(&objlist);

	player->save(&objlist);
	party->save(&objlist);

	weather->save(&objlist);
	game->get_command_bar()->save(&objlist);

	game->get_script()->call_save_game(&objlist);

	scroll->display_string("\nGame Saved\n\n");
	scroll->display_prompt();

	return true;
}

bool SaveGame::save_thumbnail(NuvieIOFileWrite *savefile) {
	unsigned char *thumbnail;

	MapWindow *map_window = Game::get_game()->get_map_window();

	thumbnail = map_window->make_thumbnail();

	savefile->writeBuf(thumbnail, MAPWINDOW_THUMBNAIL_SIZE * MAPWINDOW_THUMBNAIL_SIZE * 3);
	map_window->free_thumbnail();

	return true;
}

void SaveGame::clean_up() {
//clean up old header if required
	if (header.thumbnail) {
		SDL_FreeSurface(header.thumbnail);
		delete[] header.thumbnail_data;

		header.thumbnail = NULL;
		header.thumbnail_data = NULL;
	}

	return;
}

void SaveGame::update_objlist_for_new_game() {
	nuvie_game_t type = Game::get_game()->get_game_type();
	if (type == NUVIE_GAME_SE) {
		update_objlist_for_new_game_se();
	} else if (type == NUVIE_GAME_MD) {
		update_objlist_for_new_game_md();
	} else {
		update_objlist_for_new_game_u6();
	}

}

void SaveGame::update_objlist_for_new_game_u6() {
	std::string name = "";

	config->value("config/newgamedata/name", name, "Avatar");
	objlist.seek(0xf00);
	int len = name.length();
	if (len > 13)
		len = 13;

	objlist.writeBuf((unsigned char *)name.c_str(), len + 1);

	objlist.seek(0x1c71); //gender

	int gender;
	config->value("config/newgamedata/gender", gender, 0);

	objlist.write1(gender == 1 ? 0 : 1);

	int portrait;
	config->value("config/newgamedata/portrait", portrait, 0);

	objlist.write1((uint8)(gender * 6 + portrait) + 1);

	int stat;
	config->value("config/newgamedata/str", stat, 0xf);
	objlist.seek(0x901);
	objlist.write1(stat);

	config->value("config/newgamedata/dex", stat, 0xf);
	objlist.seek(0xa01);
	objlist.write1(stat);

	config->value("config/newgamedata/int", stat, 0xf);
	objlist.seek(0xb01);
	objlist.write1(stat);

	objlist.seek(0xc02);
	objlist.write2(0x172); //experience

	objlist.seek(0x13f2);
	objlist.write1(stat * 2); //magic

	objlist.seek(0xff2);
	objlist.write1(3); //level
}

void SaveGame::update_objlist_for_new_game_se() {
	std::string name = "";

	config->value("config/newgamedata/name", name, "Avatar");
	objlist.seek(0xf00);
	int len = name.length();
	if (len > 13)
		len = 13;

	objlist.writeBuf((unsigned char *)name.c_str(), len + 1);

	int str, dex, intelligence;
	config->value("config/newgamedata/str", str, 0xf);
	objlist.seek(0x901);
	objlist.write1(str);

	config->value("config/newgamedata/dex", dex, 0xf);
	objlist.seek(0xa01);
	objlist.write1(dex);

	config->value("config/newgamedata/int", intelligence, 0xf);
	objlist.seek(0xb01);
	objlist.write1(intelligence);

	objlist.seek(0xe01);
	objlist.write1(str * 2 + intelligence); //hp

	objlist.seek(0x14f2);
	objlist.write1(dex); //movement points
}

void SaveGame::update_objlist_for_new_game_md() {
	std::string name = "";

	int gender;
	config->value("config/newgamedata/gender", gender, 0);

	config->value("config/newgamedata/name", name, "Avatar");
	objlist.seek(0xf00);
	int len = name.length();
	if (len > 13)
		len = 13;

	objlist.writeBuf((unsigned char *)name.c_str(), len + 1);

	int str, dex, intelligence;
	config->value("config/newgamedata/str", str, 0xf);
	objlist.seek(0x901);
	objlist.write1(str);

	config->value("config/newgamedata/dex", dex, 0xf);
	objlist.seek(0xa01);
	objlist.write1(dex);

	config->value("config/newgamedata/int", intelligence, 0xf);
	objlist.seek(0xb01);
	objlist.write1(intelligence);

	objlist.seek(0xc02);
	objlist.write2(600); //experience

	objlist.seek(0xe01);
	objlist.write1(str * 2 + (4 * 24)); //hp = strength * 2 + level * 24

	objlist.seek(0xff2);
	objlist.write1(4); //level

	//FIXME movement points where are they?
	objlist.seek(0x14f2);
	objlist.write1(dex); //movement points

	objlist.seek(0x15f2);
	objlist.write1(20); //movement points

	objlist.seek(OBJLIST_OFFSET_MD_GENDER);
	objlist.write1(gender);

	//read in avatar's obj_n + frame_n data
	objlist.seek(0x403);
	uint8 b2 = objlist.read1();
	uint16 obj_n = gender == 0 ? 343 : 344;
	uint8 frame_n = (b2 & 0xfc) >> 2;

	//write out adjusted avatar gender obj_n + frame_n
	objlist.seek(0x402);
	objlist.write1(obj_n & 0xff);
	b2 = obj_n >> 8;
	b2 += frame_n << 2;
	objlist.write1(b2);

	//also write to the old obj_n + frame_n data.
	objlist.seek(0x16f3);
	objlist.write1(obj_n & 0xff);
	b2 = obj_n >> 8;
	b2 += frame_n << 2;
	objlist.write1(b2);
}

} // End of namespace Ultima6
} // End of namespace Ultima
