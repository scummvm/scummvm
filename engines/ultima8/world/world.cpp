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

#include "ultima8/misc/pent_include.h"

#include "ultima8/world/world.h"
#include "ultima8/world/map.h"
#include "ultima8/world/current_map.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"
#include "ultima8/filesys/flex_file.h"
#include "ultima8/filesys/raw_archive.h"
#include "ultima8/world/item_factory.h"
#include "ultima8/world/actors/actor.h"
#include "ultima8/world/actors/main_actor.h"
#include "ultima8/misc/id_man.h"
#include "ultima8/games/game_data.h"
#include "ultima8/kernel/kernel.h"
#include "ultima8/kernel/object_manager.h"
#include "ultima8/ultima8.h"
#include "ultima8/world/camera_process.h" // for resetting the camera
#include "ultima8/gumps/gump.h" // For CloseItemDependents notification
#include "ultima8/world/actors/animation.h"
#include "ultima8/world/get_object.h"
#include "ultima8/kernel/memory_manager.h"
#include "ultima8/audio/audio_process.h"

namespace Ultima8 {

//#define DUMP_ITEMS

World *World::world = 0;

World::World()
	: currentmap(0) {
	con->Print(MM_INFO, "Creating World...\n");
	assert(world == 0);

	world = this;
}


World::~World() {
	con->Print(MM_INFO, "Destroying World...\n");
	clear();

	world = 0;
}


void World::clear() {
	unsigned int i;

	for (i = 0; i < maps.size(); ++i) {
		delete maps[i];
	}
	maps.clear();

	while (!ethereal.empty())
		ethereal.pop_front();

	if (currentmap)
		delete currentmap;
	currentmap = 0;
}

void World::reset() {
	con->Print(MM_INFO, "Resetting World...\n");

	clear();

	initMaps();
}

void World::initMaps() {
	// Q: How do we determine which Maps to create? Only create those
	// with non-zero size in fixed.dat?

	maps.resize(256);
	for (unsigned int i = 0; i < 256; ++i) {
		maps[i] = new Map(i);
	}

	currentmap = new CurrentMap();
}

bool World::switchMap(uint32 newmap) {
	assert(currentmap);

	if (currentmap->getNum() == newmap)
		return true;

	if (newmap >= maps.size() || maps[newmap] == 0)
		return false; // no such map

	// Map switching procedure:

	// get rid of camera
	// stop all sound effects (except speech, such as Guardian barks)
	// notify all gumps of a map change
	// delete any ethereal objects
	// write back CurrentMap to the old map, which
	//   deletes all disposable items
	//   deletes the EggHatcher
	//   resets all eggs
	// swap out fixed items in old map
	// kill all processes (except those of type 1 or of item 0)
	// load fixed items in new map
	// load new map into CurrentMap, which also
	//   assigns objIDs to fixed items
	//   assigns objIDs to nonfixed items
	//   creates an EggHatcher and notifies it of all eggs
	//   sets up all NPCs in the new map
	// reset camera


	// kill camera
	CameraProcess::ResetCameraProcess();

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->stopAllExceptSpeech();

	// Notify all the gumps of the mapchange
	Ultima8Engine *gui = Ultima8Engine::get_instance();
	if (gui) {
		Gump *desktop = gui->getDesktopGump();
		if (desktop) desktop->CloseItemDependents();
	}

	// get rid of any remaining ethereal items
	while (!ethereal.empty()) {
		uint16 eth = ethereal.front();
		ethereal.pop_front();
		Item *i = getItem(eth);
		if (i) i->destroy();
	}

	uint32 oldmap = currentmap->getNum();
	if (oldmap != 0) {
		perr << "Unloading map " << oldmap << std::endl;

		assert(oldmap < maps.size() && maps[oldmap] != 0);

		currentmap->writeback();

		perr << "Unloading Fixed items from map " << oldmap << std::endl;

		maps[oldmap]->unloadFixed();
	}

	// Kill any processes that need killing (those with type != 1 && item != 0)
	Kernel::get_instance()->killProcessesNotOfType(0, 1, true);

	pout << "Loading Fixed items in map " << newmap << std::endl;
	IDataSource *items = GameData::get_instance()->getFixed()
	                     ->get_datasource(newmap);
	maps[newmap]->loadFixed(items);
	delete items;

	currentmap->loadMap(maps[newmap]);

	// reset camera
	CameraProcess::SetCameraProcess(new CameraProcess(1));
	CameraProcess::SetEarthquake(0);

	MemoryManager::get_instance()->freeResources();

	return true;
}

void World::loadNonFixed(IDataSource *ds) {
	FlexFile *f = new FlexFile(ds);

	pout << "Loading NonFixed items" << std::endl;

	for (unsigned int i = 0; i < f->getCount(); ++i) {

		// items in this map?
		if (f->getSize(i) > 0) {
			assert(maps.size() > i);
			assert(maps[i] != 0);

			IDataSource *items = f->getDataSource(i);

			maps[i]->loadNonFixed(items);

			delete items;

		}
	}

	delete f;
}

void World::loadItemCachNPCData(IDataSource *itemcach, IDataSource *npcdata) {
	FlexFile *itemcachflex = new FlexFile(itemcach);
	FlexFile *npcdataflex = new FlexFile(npcdata);

	IDataSource *itemds = itemcachflex->getDataSource(0);
	IDataSource *npcds = npcdataflex->getDataSource(0);

	delete itemcachflex;
	delete npcdataflex;

	pout << "Loading NPCs" << std::endl;

	for (uint32 i = 1; i < 256; ++i) { // Get rid of constants?
		// These are ALL unsigned on disk
		itemds->seek(0x00000 + i * 2);
		int32 x = static_cast<int32>(itemds->readX(2));
		itemds->seek(0x04800 + i * 2);
		int32 y = static_cast<int32>(itemds->readX(2));
		itemds->seek(0x09000 + i * 1);
		int32 z = static_cast<int32>(itemds->readX(1));

		itemds->seek(0x0B400 + i * 2);
		uint32 shape = itemds->read2();
		itemds->seek(0x0FC00 + i * 1);
		uint32 frame = itemds->read1();
		itemds->seek(0x12000 + i * 2);
		uint16 flags = itemds->read2();
		itemds->seek(0x16800 + i * 2);
		uint16 quality = itemds->read2();
		itemds->seek(0x1B000 + i * 1);
		uint16 npcnum = static_cast<uint8>(itemds->read1());
		itemds->seek(0x1D400 + i * 1);
		uint16 mapnum = static_cast<uint8>(itemds->read1());
		itemds->seek(0x1F800 + i * 2);
		//uint16 next;
		(void)itemds->read2();

		// half the frame number is stored in npcdata.dat
		npcds->seek(7 + i * 0x31);
		frame += npcds->read1() << 8;

		if (shape == 0) {
			// U8's itemcach has a lot of garbage in it.
			// Ignore it.
			continue;
		}

#ifdef DUMP_ITEMS
		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << std::endl;
#endif

		Actor *actor = ItemFactory::createActor(shape, frame, quality,
		                                        flags | Item::FLG_IN_NPC_LIST,
		                                        npcnum, mapnum,
		                                        Item::EXT_PERMANENT_NPC, false);
		if (!actor) {
#ifdef DUMP_ITEMS
			pout << "Couldn't create actor" << std::endl;
#endif
			continue;
		}
		ObjectManager::get_instance()->assignActorObjId(actor, i);

		actor->setLocation(x, y, z);

		// read npcdata:
		npcds->seek(i * 0x31);
		actor->setStr(npcds->read1()); // 0x00: strength
		actor->setDex(npcds->read1()); // 0x01: dexterity
		actor->setInt(npcds->read1()); // 0x02: intelligence
		actor->setHP(npcds->read1());  // 0x03: hitpoints
		actor->setDir(npcds->read1()); // 0x04: direction
		uint16 la = npcds->read2();    // 0x05,0x06: last anim
		actor->setLastAnim(static_cast<Animation::Sequence>(la));
		npcds->skip(1); // 0x07: high byte of framenum
		npcds->skip(1); // 0x08: current anim frame
		npcds->skip(1); // 0x09: start Z of current fall
		npcds->skip(1); // 0x0A: unknown, always zero
		uint8 align = npcds->read1(); // 0x0B: alignments
		actor->setAlignment(align & 0x0F);
		actor->setEnemyAlignment(align & 0xF0);
		actor->setUnk0C(npcds->read1()); // 0x0C: unknown;
		// 0x0C is almost always zero, except for
		// the avatar (0xC0) and
		// Malchir, Vardion, Gorgrond, Beren (0xE0)
		npcds->skip(14); // 0x0D-0x1A: unknown, always zero
		actor->clearActorFlag(0xFF);
		actor->setActorFlag(npcds->read1()); // 0x1B: flags
		npcds->skip(1);  // 0x1C: unknown, always zero
		npcds->skip(16); // 0x1D-0x2C: equipment
		int16 mana = static_cast<int16>(npcds->read2()); // 0x2D,0x2E: mana
		actor->setMana(mana);
		actor->clearActorFlag(0xFFFF00);
		uint32 flags2F = npcds->read1(); // 0x2F: flags
		actor->setActorFlag(flags2F << 8);
		uint32 flags30 = npcds->read1(); // 0x30: flags
		actor->setActorFlag(flags30 << 16);
	}

	delete itemds;
	delete npcds;
}


void World::worldStats() {
	unsigned int i, mapcount = 0;

	for (i = 0; i < maps.size(); i++) {
		if (maps[i] != 0 && !maps[i]->isEmpty())
			mapcount++;
	}

	pout << "World memory stats:" << std::endl;
	pout << "Maps       : " << mapcount << "/256" << std::endl;
	Actor *av = getMainActor();
	pout << "Avatar pos.: ";
	if (av) {
		pout << "map " << av->getMapNum() << ", (";
		int32 x, y, z;
		av->getLocation(x, y, z);
		pout << x << "," << y << "," << z << ")" << std::endl;
	} else {
		pout << "missing (null)" << std::endl;
	}
}

void World::save(ODataSource *ods) {
	ods->write4(currentmap->getNum());

	ods->write2(currentmap->egghatcher);

	uint16 es = static_cast<uint16>(ethereal.size());
	ods->write4(es);

	// empty stack and refill it again
	uint16 *e = new uint16[es];
	std::list<ObjId>::iterator it = ethereal.begin();
	unsigned int i;
	for (i = 0; i < es; ++i) {
		e[es - i] = *it;
		++it;
	}

	for (i = 0; i < es; ++i) {
		ods->write2(e[i]);
	}
	delete[] e;
}

// load items
bool World::load(IDataSource *ids, uint32 version) {
	uint16 curmapnum = ids->read4();
	currentmap->setMap(maps[curmapnum]);

	currentmap->egghatcher = ids->read2();

	uint32 etherealcount = ids->read4();
	for (unsigned int i = 0; i < etherealcount; ++i) {
		ethereal.push_front(ids->read2());
	}

	return true;
}

void World::saveMaps(ODataSource *ods) {
	ods->write4(static_cast<uint32>(maps.size()));
	for (unsigned int i = 0; i < maps.size(); ++i) {
		maps[i]->save(ods);
	}
}


bool World::loadMaps(IDataSource *ids, uint32 version) {
	uint32 mapcount = ids->read4();

	// Map objects have already been created by reset()
	for (unsigned int i = 0; i < mapcount; ++i) {
		bool res = maps[i]->load(ids, version);
		if (!res) return false;
	}

	return true;
}

} // End of namespace Ultima8
