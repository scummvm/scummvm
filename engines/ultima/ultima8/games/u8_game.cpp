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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/games/u8_game.h"

#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/graphics/fade_to_modal_process.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/xform_blend.h"
#include "ultima/ultima8/filesys/u8_save_file.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/gumps/movie_gump.h"
#include "ultima/ultima8/filesys/raw_archive.h"
#include "ultima/ultima8/gumps/credits_gump.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/games/start_u8_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

U8Game::U8Game() : Game() {
	// Set some defaults for gameplay-related settings
	SettingManager *settingman = SettingManager::get_instance();
	settingman->setDefault("skipstart", false);
	settingman->setDefault("endgame", false);
	settingman->setDefault("quotes", false);
	settingman->setDefault("footsteps", true);
	settingman->setDefault("targetedjump", true);

	const GameInfo *info = Ultima8Engine::get_instance()->getGameInfo();
	if (info->_language == GameInfo::GAMELANG_JAPANESE) {
		settingman->setDefault("textdelay", 20);
	} else {
		settingman->setDefault("textdelay", 8);
	}
}

U8Game::~U8Game() {
}

bool U8Game::loadFiles() {
	// Load palette
	pout << "Load Palette" << Std::endl;
	Common::SeekableReadStream *pf = FileSystem::get_instance()->ReadFile("@game/static/u8pal.pal");
	if (!pf) {
		perr << "Unable to load static/u8pal.pal." << Std::endl;
		return false;
	}
	pf->seek(4); // seek past header

	Common::MemoryReadStream xfds(U8XFormPal, 1024);
	PaletteManager::get_instance()->load(PaletteManager::Pal_Game, *pf, xfds);
	delete pf;

	pout << "Load GameData" << Std::endl;
	GameData::get_instance()->loadU8Data();

	return true;
}

bool U8Game::startGame() {
	// NOTE: assumes the entire engine has been reset!

	pout << "Starting new Ultima 8 game." << Std::endl;

	ObjectManager *objman = ObjectManager::get_instance();

	// reserve a number of objids just in case we'll need them sometime
	for (uint16 i = 384; i < 512; ++i)
		objman->reserveObjId(i);

	// reserve ObjId 666 for the Guardian Bark hack
	objman->reserveObjId(666);

	Common::SeekableReadStream *savers = FileSystem::get_instance()->ReadFile("@game/savegame/u8save.000");
	if (!savers) {
		perr << "Unable to load savegame/u8save.000." << Std::endl;
		return false;
	}
	U8SaveFile *u8save = new U8SaveFile(savers);

	Common::SeekableReadStream *nfd = u8save->getDataSource("NONFIXED.DAT");
	if (!nfd) {
		perr << "Unable to load savegame/u8save.000/NONFIXED.DAT." << Std::endl;
		return false;
	}
	World::get_instance()->loadNonFixed(nfd); // deletes nfd

	Common::SeekableReadStream *icd = u8save->getDataSource("ITEMCACH.DAT");
	if (!icd) {
		perr << "Unable to load savegame/u8save.000/ITEMCACH.DAT." << Std::endl;
		return false;
	}
	Common::SeekableReadStream *npcd = u8save->getDataSource("NPCDATA.DAT");
	if (!npcd) {
		perr << "Unable to load savegame/u8save.000/NPCDATA.DAT." << Std::endl;
		delete icd;
		return false;
	}

	World::get_instance()->loadItemCachNPCData(icd, npcd); // deletes icd, npcd
	delete u8save;

	MainActor *av = getMainActor();
	assert(av);

	av->setName("Avatar"); // default name

	// avatar needs a backpack ... CONSTANTs and all that
	Item *backpack = ItemFactory::createItem(529, 0, 0, 0, 0, 0, 0, true);
	backpack->moveToContainer(av);

	World::get_instance()->switchMap(av->getMapNum());

	Ultima8Engine::get_instance()->setAvatarInStasis(true);

	return true;
}

bool U8Game::startInitialUsecode(int saveSlot) {
	Process *proc = new StartU8Process(saveSlot);
	Kernel::get_instance()->addProcess(proc);

	return true;
}


ProcId U8Game::playIntroMovie(bool fade) {
	const GameInfo *gameinfo = CoreApp::get_instance()->getGameInfo();
	char langletter = gameinfo->getLanguageFileLetter();
	if (!langletter) {
		perr << "U8Game::playIntro: Unknown language." << Std::endl;
		return 0;
	}

	Std::string filename = "@game/static/";
	filename += langletter;
	filename += "intro.skf";

	FileSystem *filesys = FileSystem::get_instance();
	Common::SeekableReadStream *skf = filesys->ReadFile(filename);
	if (!skf) {
		pout << "U8Game::playIntro: movie not found." << Std::endl;
		return 0;
	}

	return MovieGump::U8MovieViewer(skf, fade, true);
}

ProcId U8Game::playEndgameMovie(bool fade) {
	static const Std::string filename = "@game/static/endgame.skf";
	FileSystem *filesys = FileSystem::get_instance();
	Common::SeekableReadStream *skf = filesys->ReadFile(filename);
	if (!skf) {
		pout << "U8Game::playEndgame: movie not found." << Std::endl;
		return 0;
	}

	return MovieGump::U8MovieViewer(skf, fade);
}

void U8Game::playCredits() {
	const GameInfo *gameinfo = CoreApp::get_instance()->getGameInfo();
	char langletter = gameinfo->getLanguageFileLetter();
	if (!langletter) {
		perr << "U8Game::playCredits: Unknown language." << Std::endl;
		return;
	}
	Std::string filename = "@game/static/";
	filename += langletter;
	filename += "credits.dat";

	Common::SeekableReadStream *rs = FileSystem::get_instance()->ReadFile(filename);
	if (!rs) {
		perr << "U8Game::playCredits: error opening credits file: "
		     << filename << Std::endl;
		return;
	}
	Std::string text = getCreditText(rs);
	delete rs;

	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(51); // CONSTANT!

	CreditsGump *gump = new CreditsGump(text);
	gump->SetFlagWhenFinished("quotes");
	FadeToModalProcess *p = new FadeToModalProcess(gump);
	Kernel::get_instance()->addProcess(p);
}

void U8Game::playQuotes() {
	static const Std::string filename = "@game/static/quotes.dat";

	Common::SeekableReadStream *rs = FileSystem::get_instance()->ReadFile(filename);
	if (!rs) {
		perr << "U8Game::playCredits: error opening credits file: "
		     << filename << Std::endl;
		return;
	}
	const Std::string text = getCreditText(rs);
	delete rs;

	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(113); // CONSTANT!

	CreditsGump *gump = new CreditsGump(text, 80);
	FadeToModalProcess *p = new FadeToModalProcess(gump);
	Kernel::get_instance()->addProcess(p);
}


void U8Game::writeSaveInfo(Common::WriteStream *ws) {
	MainActor *av = getMainActor();
	int32 x, y, z;

	const Std::string &avname = av->getName();
	const uint8 namelength = static_cast<uint8>(avname.size());
	ws->writeByte(namelength);
	for (unsigned int i = 0; i < namelength; ++i)
		ws->writeByte(static_cast<uint8>(avname[i]));

	av->getLocation(x, y, z);
	ws->writeUint16LE(av->getMapNum());
	ws->writeUint32LE(static_cast<uint32>(x));
	ws->writeUint32LE(static_cast<uint32>(y));
	ws->writeUint32LE(static_cast<uint32>(z));

	ws->writeUint16LE(av->getStr());
	ws->writeUint16LE(av->getInt());
	ws->writeUint16LE(av->getDex());
	ws->writeUint16LE(av->getHP());
	ws->writeUint16LE(av->getMaxHP());
	ws->writeUint16LE(av->getMana());
	ws->writeUint16LE(av->getMaxMana());
	ws->writeUint16LE(av->getArmourClass());
	ws->writeUint16LE(av->getTotalWeight());

	for (unsigned int i = 1; i <= 6; i++) {
		uint16 objid = av->getEquip(i);
		Item *item = getItem(objid);
		if (item) {
			ws->writeUint32LE(item->getShape());
			ws->writeUint32LE(item->getFrame());
		} else {
			ws->writeUint32LE(0);
			ws->writeUint32LE(0);
		}
	}
}

Std::string U8Game::getCreditText(Common::SeekableReadStream *rs) {
	Std::string text;
	unsigned int size = rs->size();
	text.resize(size);
	for (unsigned int i = 0; i < size; ++i) {
		uint8 c = rs->readByte();
		int x;
		switch (i) {
		case 0:
		case 1:
			x = 0;
			break;
		case 2:
			x = 0xE1;
			break;
		default:
			x = 0x20 * (i + 1) + (i >> 1);
			x += (i % 0x40) * ((i & 0xC0) >> 6) * 0x40;
			break;
		}
		char d = (c ^ x) & 0xFF;
		if (d == 0) d = '\n';
		text[i] = d;
	}

	return text;
}

} // End of namespace Ultima8
} // End of namespace Ultima
