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
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/map.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/filesys/flex_file.h"
#include "ultima/ultima8/filesys/raw_archive.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/misc/id_man.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/camera_process.h" // for resetting the camera
#include "ultima/ultima8/gumps/gump.h" // For CloseItemDependents notification
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/usecode/intrinsics.h"

namespace Ultima {
namespace Ultima8 {

//#define DUMP_ITEMS

World *World::_world = nullptr;

World::World() : _currentMap(nullptr), _alertActive(false), _difficulty(1),
				 _controlledNPCNum(1) {
	debugN(MM_INFO, "Creating World...\n");

	_world = this;
}


World::~World() {
	debugN(MM_INFO, "Destroying World...\n");
	clear();

	_world = nullptr;
}


void World::clear() {
	unsigned int i;

	for (i = 0; i < _maps.size(); ++i) {
		delete _maps[i];
	}
	_maps.clear();

	while (!_ethereal.empty())
		_ethereal.pop_front();

	if (_currentMap)
		delete _currentMap;
	_currentMap = nullptr;
}

void World::reset() {
	debugN(MM_INFO, "Resetting World...\n");

	clear();

	initMaps();
}

void World::initMaps() {
	// Q: How do we determine which Maps to create? Only create those
	// with non-zero size in fixed.dat?

	_maps.resize(256);
	for (unsigned int i = 0; i < 256; ++i) {
		_maps[i] = new Map(i);
	}

	_currentMap = new CurrentMap();
}

bool World::switchMap(uint32 newmap) {
	assert(_currentMap);

	if (_currentMap->getNum() == newmap)
		return true;

	if (newmap >= _maps.size() || _maps[newmap] == nullptr)
		return false; // no such map

	// Map switching procedure:

	// get rid of camera
	// stop all sound effects (except speech, such as Guardian barks)
	// notify all gumps of a map change
	// delete any _ethereal objects
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

	// get rid of any remaining _ethereal items
	while (!_ethereal.empty()) {
		uint16 eth = _ethereal.front();
		_ethereal.pop_front();
		Item *i = getItem(eth);
		if (i) i->destroy();
	}

	uint32 oldmap = _currentMap->getNum();
	if (oldmap != 0) {
		perr << "Unloading map " << oldmap << Std::endl;

		assert(oldmap < _maps.size() && _maps[oldmap] != nullptr);

		_currentMap->writeback();

		perr << "Unloading Fixed items from map " << oldmap << Std::endl;

		_maps[oldmap]->unloadFixed();
	}

	// Kill any processes that need killing (those with type != 1 && item != 0)
	Kernel::get_instance()->killProcessesNotOfType(0, 1, true);

	pout << "Loading Fixed items in map " << newmap << Std::endl;
	Common::SeekableReadStream *items = GameData::get_instance()->getFixed()
	                     ->get_datasource(newmap);
	_maps[newmap]->loadFixed(items);
	delete items;

	_currentMap->loadMap(_maps[newmap]);

	// reset camera
	CameraProcess::SetCameraProcess(new CameraProcess(1));
	CameraProcess::SetEarthquake(0);

	return true;
}

void World::loadNonFixed(Common::SeekableReadStream *rs) {
	FlexFile *f = new FlexFile(rs);

	pout << "Loading NonFixed items" << Std::endl;

	for (unsigned int i = 0; i < f->getCount(); ++i) {

		// items in this map?
		if (f->getSize(i) > 0) {
			assert(_maps.size() > i);
			assert(_maps[i] != nullptr);

			Common::SeekableReadStream *items = f->getDataSource(i);

			_maps[i]->loadNonFixed(items);

			delete items;

		}
	}

	delete f;
}

void World::loadItemCachNPCData(Common::SeekableReadStream *itemcach, Common::SeekableReadStream *npcdata) {
	FlexFile *itemcachflex = new FlexFile(itemcach);
	FlexFile *npcdataflex = new FlexFile(npcdata);

	Common::SeekableReadStream *itemds = itemcachflex->getDataSource(0);
	Common::SeekableReadStream *npcds = npcdataflex->getDataSource(0);

	delete itemcachflex;
	delete npcdataflex;

	pout << "Loading NPCs" << Std::endl;

	for (uint32 i = 1; i < 256; ++i) { // Get rid of constants?
		// These are ALL unsigned on disk
		itemds->seek(0x00000 + i * 2);
		int32 x = static_cast<int32>(itemds->readUint16LE());
		itemds->seek(0x04800 + i * 2);
		int32 y = static_cast<int32>(itemds->readUint16LE());
		itemds->seek(0x09000 + i * 1);
		int32 z = static_cast<int32>(itemds->readByte());

		itemds->seek(0x0B400 + i * 2);
		uint32 shape = itemds->readUint16LE();
		itemds->seek(0x0FC00 + i * 1);
		uint32 frame = itemds->readByte();
		itemds->seek(0x12000 + i * 2);
		uint16 flags = itemds->readUint16LE();
		itemds->seek(0x16800 + i * 2);
		uint16 quality = itemds->readUint16LE();
		itemds->seek(0x1B000 + i * 1);
		uint16 npcnum = static_cast<uint8>(itemds->readByte());
		itemds->seek(0x1D400 + i * 1);
		uint16 mapnum = static_cast<uint8>(itemds->readByte());
		itemds->seek(0x1F800 + i * 2);
		//uint16 next;
		(void)itemds->readUint16LE();

		// half the frame number is stored in npcdata.dat
		npcds->seek(7 + i * 0x31);
		frame += npcds->readByte() << 8;

		if (shape == 0) {
			// U8's itemcach has a lot of garbage in it.
			// Ignore it.
			continue;
		}

#ifdef DUMP_ITEMS
		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << Std::hex << flags << Std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << Std::endl;
#endif

		Actor *actor = ItemFactory::createActor(shape, frame, quality,
		                                        flags | Item::FLG_IN_NPC_LIST,
		                                        npcnum, mapnum,
		                                        Item::EXT_PERMANENT_NPC, false);
		if (!actor) {
#ifdef DUMP_ITEMS
			pout << "Couldn't create actor" << Std::endl;
#endif
			continue;
		}
		ObjectManager::get_instance()->assignActorObjId(actor, i);

		actor->setLocation(x, y, z);

		// read npcdata:
		npcds->seek(i * 0x31);
		actor->setStr(npcds->readByte()); // 0x00: strength
		actor->setDex(npcds->readByte()); // 0x01: dexterity
		actor->setInt(npcds->readByte()); // 0x02: intelligence
		actor->setHP(npcds->readByte());  // 0x03: hitpoints
		actor->setDir(Direction_FromUsecodeDir(npcds->readByte())); // 0x04: direction
		uint16 la = npcds->readUint16LE();    // 0x05,0x06: last anim
		actor->setLastAnim(static_cast<Animation::Sequence>(la));
		npcds->skip(1); // 0x07: high byte of framenum
		npcds->skip(1); // 0x08: current anim frame
		npcds->skip(1); // 0x09: start Z of current fall
		npcds->skip(1); // 0x0A: unknown, always zero
		uint8 align = npcds->readByte(); // 0x0B: alignments
		actor->setAlignment(align & 0x0F);
		actor->setEnemyAlignment(align & 0xF0);
		actor->setUnkByte(npcds->readByte()); // 0x0C: unknown;
		// 0x0C is almost always zero, except for
		// the avatar (0xC0) and
		// Malchir, Vardion, Gorgrond, Beren (0xE0)
		npcds->skip(14); // 0x0D-0x1A: unknown, always zero
		actor->clearActorFlag(0xFF);
		actor->setActorFlag(npcds->readByte()); // 0x1B: flags
		npcds->skip(1);  // 0x1C: unknown, always zero
		npcds->skip(16); // 0x1D-0x2C: equipment
		int16 mana = static_cast<int16>(npcds->readUint16LE()); // 0x2D,0x2E: mana
		actor->setMana(mana);
		actor->clearActorFlag(0xFFFF00);
		uint32 flags2F = npcds->readByte(); // 0x2F: flags
		actor->setActorFlag(flags2F << 8);
		uint32 flags30 = npcds->readByte(); // 0x30: flags
		actor->setActorFlag(flags30 << 16);
	}

	delete itemds;
	delete npcds;
}


void World::worldStats() const {
	unsigned int i, mapcount = 0;

	for (i = 0; i < _maps.size(); i++) {
		if (_maps[i] != nullptr && !_maps[i]->isEmpty())
			mapcount++;
	}

	g_debugger->debugPrintf("World memory stats:\n");
	g_debugger->debugPrintf("Maps       : %u/256\n", mapcount);

	const Actor *av = getMainActor();
	g_debugger->debugPrintf("Avatar pos.: ");
	if (av) {
		g_debugger->debugPrintf("map %d, (", av->getMapNum());
		int32 x, y, z;
		av->getLocation(x, y, z);
		g_debugger->debugPrintf("%d,%d,%d)\n", x, y, z);
	} else {
		g_debugger->debugPrintf("missing (null)\n");
	}
}

void World::save(Common::WriteStream *ws) {
	ws->writeUint32LE(_currentMap->getNum());

	ws->writeUint16LE(_currentMap->_eggHatcher);

	if (GAME_IS_CRUSADER) {
		ws->writeByte(_alertActive ? 0 : 1);
		ws->writeByte(_difficulty);
	}

	uint16 es = static_cast<uint16>(_ethereal.size());
	ws->writeUint32LE(es);

	// empty stack and refill it again
	uint16 *e = new uint16[es];
	Std::list<ObjId>::const_iterator it = _ethereal.begin();
	unsigned int i;
	for (i = 0; i < es; ++i) {
		e[es - i] = *it;
		++it;
	}

	for (i = 0; i < es; ++i) {
		ws->writeUint16LE(e[i]);
	}
	delete[] e;
}

// load items
bool World::load(Common::ReadStream *rs, uint32 version) {
	uint16 curmapnum = rs->readUint32LE();
	_currentMap->setMap(_maps[curmapnum]);

	_currentMap->_eggHatcher = rs->readUint16LE();

	if (GAME_IS_CRUSADER) {
		_alertActive = (rs->readByte() != 0);
		_difficulty = rs->readByte();
	}

	uint32 etherealcount = rs->readUint32LE();
	for (unsigned int i = 0; i < etherealcount; ++i) {
		_ethereal.push_front(rs->readUint16LE());
	}

	return true;
}

void World::saveMaps(Common::WriteStream *ws) {
	ws->writeUint32LE(static_cast<uint32>(_maps.size()));
	for (unsigned int i = 0; i < _maps.size(); ++i) {
		_maps[i]->save(ws);
	}
}

bool World::loadMaps(Common::ReadStream *rs, uint32 version) {
	uint32 mapcount = rs->readUint32LE();

	// Map objects have already been created by reset()
	for (unsigned int i = 0; i < mapcount; ++i) {
		bool res = _maps[i]->load(rs, version);
		if (!res) return false;
	}

	return true;
}

void World::setAlertActive(bool active)
{
	assert(GAME_IS_CRUSADER);
    _alertActive = active;

	// Replicate the behavior of the original game.
	LOOPSCRIPT(script,
		LS_OR(
			LS_OR(
				LS_OR(
					LS_OR(LS_SHAPE_EQUAL(0x49), LS_SHAPE_EQUAL(0x21)),
					LS_SHAPE_EQUAL(0x174)),
				LS_SHAPE_EQUAL(0x271)),
			  LS_SHAPE_EQUAL(0x477))
	);

	UCList itemlist(2);
	_world->getCurrentMap()->areaSearch(&itemlist, script, sizeof(script),
										nullptr, 0xffff, false);
	for (uint32 i = 0; i < itemlist.getSize(); i++) {
		uint16 itemid = itemlist.getuint16(i);
		Item *item = getItem(itemid);
		int frame = item->getFrame();
		if (_alertActive) {
			if (item->getShape() == 0x477) {
				if (frame < 2)
					item->setFrame(frame + 2);
			} else if (frame == 0) {
				item->setFrame(1);
			}
		} else {
			if (item->getShape() == 0x477) {
				if (frame > 1)
					item->setFrame(frame - 2);
			} else if (frame == 1) {
				item->setFrame(0);
			}
		}
	}
}

void World::setControlledNPCNum(uint16 num) {
	warning("TODO: World::setControlledNPCNum(%d): IMPLEMENT ME", num);
}

uint32 World::I_getAlertActive(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	return get_instance()->_world->isAlertActive() ? 1 : 0;
}

uint32 World::I_setAlertActive(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	get_instance()->_world->setAlertActive(true);
	return 0;
}

uint32 World::I_clrAlertActive(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	get_instance()->_world->setAlertActive(false);
	return 0;
}

uint32 World::I_gameDifficulty(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	return get_instance()->_world->getGameDifficulty();
}

uint32 World::I_getControlledNPCNum(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	return get_instance()->_world->getControlledNPCNum();
}

uint32 World::I_setControlledNPCNum(const uint8 *args,
	unsigned int /*argsize*/) {
	ARG_UINT16(num);
	get_instance()->_world->setControlledNPCNum(num);
	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
