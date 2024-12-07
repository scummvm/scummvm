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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/widgets/console.h"
#include "ultima/nuvie/save/save_game.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/save/obj_list.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/core/egg_manager.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/portraits/portrait.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/gui/widgets/command_bar.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/nuvie.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "gui/browser.h"

namespace Ultima {
namespace Nuvie {

#define OBJLIST_FILENAME "savegame/objlist"
#define OBJBLK_FILENAME  "savegame/objblkxx"
#define GAME_ID(GT) ((GT == GAME_SAVAGE_EMPIRE) ? MKTAG16('S', 'E') : \
	((GT == GAME_MARTIAN_DREAMS) ? MKTAG16('M', 'D') : MKTAG16('U', '6')))

SaveGame::SaveGame(Configuration *cfg) : config(cfg) {
	// We don't need ObjManager here as there will be nothing to clean at this stage
	init(nullptr);
}

SaveGame::~SaveGame() {
	objlist.close();
}

void SaveGame::init(ObjManager *obj_manager) {
	if (objlist.get_size() > 0)
		objlist.close();

	if (obj_manager)
		obj_manager->clean();

	return;
}


bool SaveGame::load_new() {
	Common::Path filename;
	U6Lzw lzw;
	NuvieIOBuffer buf;
	uint32 decomp_size;
	uint8 i;
	uint32 pos;

	ObjManager *obj_manager = Game::get_game()->get_obj_manager();

	init(obj_manager);

	// Load surface chunks
	config_get_path(config, "lzobjblk", filename);
	unsigned char *data = lzw.decompress_file(filename, decomp_size);
	if (!data)
		return false;

	buf.open(data, decomp_size, NUVIE_BUF_NOCOPY);

	for (i = 0; i < 64; i++)
		obj_manager->load_super_chunk(&buf, 0, i);

	buf.close();
	free(data);

	// Load dungeon chunks
	config_get_path(config, "lzdngblk", filename);
	data = lzw.decompress_file(filename, decomp_size);
	if (!data)
		return false;

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
	Std::string objblk_filename;
	Common::Path path, objlist_filename;
	char x, y;
	uint16 len;
	NuvieIOFileRead objlist_file;

	NuvieIOFileRead *objblk_file = new NuvieIOFileRead();
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();

	init(obj_manager);

	objblk_filename = OBJBLK_FILENAME;
	len = objblk_filename.length();

	uint8 i = 0;

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

			if (obj_manager->load_super_chunk(objblk_file, 0, i) == false) {
				delete objblk_file;
				return false;
			}
			i++;
			objblk_file->close();
		}
	}

	objblk_filename[len - 1] = 'i';

	for (i = 0, x = 'a'; x < 'f'; x++, i++) { // Load dungeons
		objblk_filename[len - 2] = x;
		config_get_path(config, objblk_filename, path);
		objblk_file->open(path);

		if (obj_manager->load_super_chunk(objblk_file, i, 0) == false) {
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

	unsigned char *data = objlist_file.readAll();

	objlist.open(data, objlist_file.get_size(), NUVIE_BUF_COPY);
	free(data);

	load_objlist();

	return true;
}

bool SaveGame::transfer_character() {
	::GUI::BrowserDialog dialog(_("Transfer Character"), true);
	if (!dialog.runModal())
		return false;

	Common::FSNode folder = dialog.getResult();

	// TODO: Load in character data from given folder and start new game
	g_engine->GUIError(Common::String::format("Load party file from folder - %s", folder.getPath().toString(Common::Path::kNativeSeparator).c_str()));

	return false;
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

	portrait->load(&objlist); // load avatar portrait number.

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

	scroll->init(player->get_name());

	scroll->display_prompt();

	return true;
}

bool SaveGame::check_version(NuvieIOFileRead *loadfile, uint16 gameType) {
	uint16 version, gt;

	loadfile->seekStart();
	version = loadfile->read2();
	gt = loadfile->read2();

	if (version != SAVE_VERSION) {
		DEBUG(0, LEVEL_ERROR, "Incompatible savegame version. Savegame version '%d', current system version '%d'\n", version, SAVE_VERSION);
		return false;
	}

	if (gt != gameType) {
		DEBUG(0, LEVEL_ERROR, "Incorrect game type\n");
		return false;
	}

	return true;
}

bool SaveGame::load(const Common::String &filename) {
	uint8 i;
	uint32 bytes_read;
	NuvieIOFileRead loadFile;
	GameId gameType = g_engine->getGameId();
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();

	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(filename);
	if (!saveFile || !loadFile.open(saveFile))
		return false;

	ConsoleAddInfo("Loading Game: %s", filename.c_str());
	DEBUG(0, LEVEL_NOTIFICATION, "Loading Game: %s\n", filename.c_str());

	if (!check_version(&loadFile, GAME_ID(gameType))) {
		DEBUG(0, LEVEL_NOTIFICATION, "version incorrect\n");
		return false;
	}

	init(obj_manager); // needs to come after checking for failure

	// Load actor inventories
	obj_manager->load_super_chunk(&loadFile, 0, 0);

	// Load eggs
	obj_manager->load_super_chunk(&loadFile, 0, 0);

	// Load surface objects
	for (i = 0; i < 64; i++) {
		ConsoleAddInfo("Loading super chunk %d of 64", i + 1);
		obj_manager->load_super_chunk(&loadFile, 0, i);
	}

	// Load dungeon objects
	for (i = 0; i < 5; i++) {
		obj_manager->load_super_chunk(&loadFile, i + 1, 0);
	}

	uint32 objlist_size = loadFile.read4();
	unsigned char *data = loadFile.readBuf(objlist_size, &bytes_read);

	objlist.open(data, objlist_size, NUVIE_BUF_COPY);

	free(data);
	loadFile.close();

	load_objlist();

	delete saveFile;
	return true;
}

bool SaveGame::save(const Common::String &filename, const Common::String &save_description, bool isAutosave) {
	NuvieIOFileWrite saveFile;
	GameId gameType = g_engine->getGameId();
	ObjManager *obj_manager = Game::get_game()->get_obj_manager();

	bool newgame;
	config->value("config/newgame", newgame, false);
	if (newgame) {
		config->set("config/newgame", false);
		config->write();
	}

	saveFile.open(filename, isAutosave);

	saveFile.write2(SAVE_VERSION);
	saveFile.write2(GAME_ID(gameType));

	obj_manager->save_inventories(&saveFile);

	obj_manager->save_eggs(&saveFile);

	// Save surface objects
	for (uint8 i = 0; i < 64; i++)
		obj_manager->save_super_chunk(&saveFile, 0, i);

	// Save dungeon objects
	for (uint8 i = 0; i < 5; i++)
		obj_manager->save_super_chunk(&saveFile, i + 1, 0);

	save_objlist();

	saveFile.write4(objlist.get_size());
	saveFile.writeBuf(objlist.get_raw_data(), objlist.get_size());

	saveFile.writeDesc(save_description);
	saveFile.close();

	return true;
}

bool SaveGame::save_objlist() {
	Game *game;
	GameClock *clock;
	ActorManager *actor_manager;
	Player *player;
	Party *party;
	Weather *weather;

	game = Game::get_game();

	clock = game->get_clock();
	actor_manager = game->get_actor_manager();

	player = game->get_player();
	party = game->get_party();
	weather = game->get_weather();

	clock->save(&objlist);
	actor_manager->save(&objlist);

	player->save(&objlist);
	party->save(&objlist);

	weather->save(&objlist);
	game->get_command_bar()->save(&objlist);

	game->get_script()->call_save_game(&objlist);

	return true;
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
	Std::string name = "";

	config->value("config/newgamedata/name", name, "Avatar");
	objlist.seek(0xf00);
	int len = name.length();
	if (len > 13)
		len = 13;

	objlist.writeBuf((const unsigned char *)name.c_str(), len + 1);

	objlist.seek(0x1c71); // gender

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

	int intelligence;
	config->value("config/newgamedata/int", intelligence, 0xf);
	objlist.seek(0xb01);
	objlist.write1(intelligence);

	config->value("config/newgamedata/exp", stat, 0x172);
	objlist.seek(0xc02);
	objlist.write2(stat); // experience

	config->value("config/newgamedata/magic", stat, intelligence * 2);
	objlist.seek(0x13f2);
	objlist.write1(stat); // magic

	config->value("config/newgamedata/level", stat, 3);
	objlist.seek(0xff2);
	objlist.write1(stat); //level
}

void SaveGame::update_objlist_for_new_game_se() {
	Std::string name = "";

	config->value("config/newgamedata/name", name, "Avatar");
	objlist.seek(0xf00);
	int len = name.length();
	if (len > 13)
		len = 13;

	objlist.writeBuf((const unsigned char *)name.c_str(), len + 1);

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
	objlist.write1(dex); // movement points
}

void SaveGame::update_objlist_for_new_game_md() {
	Std::string name = "";

	int gender;
	config->value("config/newgamedata/gender", gender, 0);

	config->value("config/newgamedata/name", name, "Avatar");
	objlist.seek(0xf00);
	int len = name.length();
	if (len > 13)
		len = 13;

	objlist.writeBuf((const unsigned char *)name.c_str(), len + 1);

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
	objlist.write2(600); // experience

	objlist.seek(0xe01);
	objlist.write1(str * 2 + (4 * 24)); //hp = strength * 2 + level * 24

	objlist.seek(0xff2);
	objlist.write1(4); // level

	// FIXME: movement points where are they?
	objlist.seek(0x14f2);
	objlist.write1(dex); // movement points

	objlist.seek(0x15f2);
	objlist.write1(20); // movement points

	objlist.seek(OBJLIST_OFFSET_MD_GENDER);
	objlist.write1(gender);

	// Read in avatar's obj_n + frame_n data
	objlist.seek(0x403);
	uint8 b2 = objlist.read1();
	uint16 obj_n = gender == 0 ? 343 : 344;
	uint8 frame_n = (b2 & 0xfc) >> 2;

	// Write out adjusted avatar gender obj_n + frame_n
	objlist.seek(0x402);
	objlist.write1(obj_n & 0xff);
	b2 = obj_n >> 8;
	b2 += frame_n << 2;
	objlist.write1(b2);

	// Also write to the old obj_n + frame_n data.
	objlist.seek(0x16f3);
	objlist.write1(obj_n & 0xff);
	b2 = obj_n >> 8;
	b2 += frame_n << 2;
	objlist.write1(b2);
}

} // End of namespace Nuvie
} // End of namespace Ultima
