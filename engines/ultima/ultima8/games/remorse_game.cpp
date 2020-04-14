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
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/kernel/object_manager.h"
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

bool RemorseGame::loadFiles() {
	// Load palette
	pout << "Load Palette" << Std::endl;
	Common::SeekableReadStream *pf = FileSystem::get_instance()->ReadFile("@game/static/gamepal.pal");
	if (!pf) {
		perr << "Unable to load static/gamepal.pal." << Std::endl;
		return false;
	}

	Common::MemoryReadStream xfds(U8XFormPal, 1024);
	PaletteManager::get_instance()->load(PaletteManager::Pal_Game, *pf, xfds);
	delete pf;

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

	ObjectManager::get_instance()->assignActorObjId(actor, 1);

	actor->setLocation(60700, 59420, 16);


	World::get_instance()->switchMap(1);

	Ultima8Engine::get_instance()->setAvatarInStasis(true);

	return true;
}

bool RemorseGame::startInitialUsecode(int saveSlot) {
//	Process* proc = new StartU8Process();
//	Kernel::get_instance()->addProcess(proc);

	return true;
}


ProcId RemorseGame::playIntroMovie(bool fade) {
	return 0;
}

ProcId RemorseGame::playEndgameMovie(bool fade) {
	return 0;
}

void RemorseGame::playCredits() {

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
