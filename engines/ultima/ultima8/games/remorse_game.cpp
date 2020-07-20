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
#include "ultima/ultima8/games/remorse_game.h"
#include "ultima/ultima8/games/start_crusader_process.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/gumps/movie_gump.h"
#include "ultima/ultima8/gumps/cru_status_gump.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/graphics/xform_blend.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/filesys/raw_archive.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

RemorseGame::RemorseGame() : Game() {
	// Set some defaults for gameplay-related settings
	SettingManager *settingman = SettingManager::get_instance();
	settingman->setDefault("skipstart", false);
	settingman->setDefault("endgame", false);
	settingman->setDefault("footsteps", true);
	settingman->setDefault("textdelay", 5);
}

RemorseGame::~RemorseGame() {

}

static bool loadPalette(const char *path, PaletteManager::PalIndex index) {
	Common::SeekableReadStream *pf = FileSystem::get_instance()->ReadFile(path);
	if (!pf) {
		perr << "Unable to load static/*.pal." << Std::endl;
		return false;
	}

	Common::MemoryReadStream xfds(CruXFormPal, 1024);
	PaletteManager::get_instance()->load(index, *pf, xfds);
	delete pf;

	return true;
}

bool RemorseGame::loadFiles() {
	// Load palette
	pout << "Load Palettes" << Std::endl;

	if (!loadPalette("@game/static/gamepal.pal", PaletteManager::Pal_Game))
		return false;
	if (GAME_IS_REGRET) {
		if (!loadPalette("@game/static/cred.pal", PaletteManager::Pal_Cred))
			return false;
	}
	if (!loadPalette("@game/static/diff.pal", PaletteManager::Pal_Diff))
		return false;
	if (!loadPalette("@game/static/misc.pal", PaletteManager::Pal_Misc))
		return false;
	if (!loadPalette("@game/static/misc2.pal", PaletteManager::Pal_Misc2))
		return false;
	if (!loadPalette("@game/static/star.pal", PaletteManager::Pal_Star))
		return false;

	pout << "Load GameData" << Std::endl;
	GameData::get_instance()->loadRemorseData();

	return true;
}

bool RemorseGame::startGame() {
	// NOTE: assumes the entire engine has been reset!

	pout << "Starting new Crusader: No Remorse game." << Std::endl;

	ObjectManager *objman = ObjectManager::get_instance();

	// reserve a number of objids just in case we'll need them sometime
	for (uint16 i = 384; i < 512; ++i)
		objman->reserveObjId(i);

	// FIXME: fix flags and such
	Actor *actor = ItemFactory::createActor(1, 0, 0, Item::FLG_IN_NPC_LIST,
	                                        1, 1, Item::EXT_PERMANENT_NPC, false);
	if (!actor)
		error("Couldn't create MainActor");

	// TODO: these should be read from DTable data.
	actor->setStr(75);
	actor->setHP(150);
	actor->setInt(5000);
	actor->setMana(2500);

	ObjectManager::get_instance()->assignActorObjId(actor, 1);

	if (GAME_IS_REMORSE) {
		// Some useful points to warp into for testing..
		actor->setLocation(0, 0, 0); // Map 1 (mission 1)
		//actor->setLocation(60716, 59400, 16); // Map 1 (mission 1)
		//actor->setLocation(42493, 26621, 16); // Map 2 (mission 1 / level 4)
		//actor->setLocation(34302, 32254, 16); // Map 3 (mission 2)
		//actor->setLocation(34813, 33789, 16); // Map 4
		//actor->setLocation(37373, 30205, 16); // Map 5
		//actor->setLocation(37373, 30205, 16); // Map 6
		//actor->setLocation(35070, 26142, 96); // Map 7
		//actor->setLocation(29693, 32253, 0); // Map 8 - unfinished area?
		//actor->setLocation(2046, 2046, 0); // Map 9
		//actor->setLocation(14845, 6141, 0); // Map 22 - debugging map
		//actor->setLocation(34302, 32254, 16); // Map 40 (Rebel base)
	} else {
		actor->setLocation(58174, 56606, 16);
	}

	World::get_instance()->switchMap(0);

	return true;
}

bool RemorseGame::startInitialUsecode(int saveSlot) {
	Process* proc = new StartCrusaderProcess();
	Kernel::get_instance()->addProcess(proc);
	return true;
}


static ProcId playMovie(const char *movieID, bool fade) {
	const Std::string filename = Std::string::format("@game/flics/%s.avi", movieID);
	FileSystem *filesys = FileSystem::get_instance();
	Common::SeekableReadStream *rs = filesys->ReadFile(filename);
	if (!rs) {
		pout << "RemorseGame::playIntro: movie not found." << Std::endl;
		return 0;
	}
	// TODO: Add support for subtitles (.txt file).  The format is very simple.
	return MovieGump::U8MovieViewer(rs, fade);
}

ProcId RemorseGame::playIntroMovie(bool fade) {
	return playMovie("T01", fade);
}

ProcId RemorseGame::playIntroMovie2(bool fade) {
	return playMovie("T02", fade);
}


ProcId RemorseGame::playEndgameMovie(bool fade) {
	return playMovie("O01", fade);
}

void RemorseGame::playCredits() {
	warning("TODO: RemorseGame::playCredits: Implement Crusader credits");
}

void RemorseGame::writeSaveInfo(Common::WriteStream *ws) {
#if 0
	MainActor *av = getMainActor();
	int32 x, y, z;

	const Std::string &avname = av->getName();
	uint8 namelength = static_cast<uint8>(avname.size());
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
		const uint16 objid = av->getEquip(i);
		const Item *item = getItem(objid);
		if (item) {
			ws->writeUint32LE(item->getShape());
			ws->writeUint32LE(item->getFrame());
		} else {
			ws->writeUint32LE(0);
			ws->writeUint32LE(0);
		}
	}
#endif
}

} // End of namespace Ultima8
} // End of namespace Ultima
