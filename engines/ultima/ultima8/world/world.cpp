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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/map.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/filesys/flex_file.h"
#include "ultima/ultima8/filesys/raw_archive.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/actors/scheduler_process.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/world/camera_process.h" // for resetting the camera
#include "ultima/ultima8/gumps/gump.h" // For CloseItemDependents notification
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/target_reticle_process.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/snap_process.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"

namespace Ultima {
namespace Ultima8 {

//#define DUMP_ITEMS

World *World::_world = nullptr;

World::World() : _currentMap(nullptr), _alertActive(false), _difficulty(3),
				 _controlledNPCNum(1), _vargasShield(5000) {
	debug(1, "Creating World...");

	_world = this;
}


World::~World() {
	debug(1, "Destroying World...");
	clear();

	_world = nullptr;
}


void World::clear() {
	unsigned int i;

	for (i = 0; i < _maps.size(); ++i) {
		delete _maps[i];
	}
	_maps.clear();

	_ethereal.clear();

	if (_currentMap)
		delete _currentMap;
	_currentMap = nullptr;

	_alertActive = false;
	_controlledNPCNum = 1;
	_vargasShield = 5000;
}

void World::reset() {
	debug(1, "Resetting World...");

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
	// update camera if needed

	AudioProcess *ap = AudioProcess::get_instance();
	if (ap) ap->stopAllExceptSpeech();

	// Notify all the gumps of the mapchange
	Ultima8Engine *gui = Ultima8Engine::get_instance();
	if (gui) {
		Gump *desktop = gui->getDesktopGump();
		if (desktop) desktop->CloseItemDependents();
	}

	// get rid of any remaining ethereal items
	while (!_ethereal.empty()) {
		uint16 eth = _ethereal.front();
		_ethereal.pop_front();
		Item *i = getItem(eth);
		if (i) {
			if (i->getFlags() & Item::FLG_ETHEREAL)
				i->destroy();
			else
				warning("Not destroying ethereal item %d - it doesn't think it's ethereal", eth);
		}
	}

	uint32 oldmap = _currentMap->getNum();
	if (oldmap != 0) {
		debug(1, "Unloading map %u", oldmap);

		assert(oldmap < _maps.size() && _maps[oldmap] != nullptr);

		_currentMap->writeback();

		debug(1, "Unloading Fixed items from map %u", oldmap);

		_maps[oldmap]->unloadFixed();
	}

	// Kill any processes that need killing
	if (GAME_IS_U8) {
		// U8 doesn't kill processes of object 0 *or* type 1 when changing map.
		Kernel::get_instance()->killProcessesNotOfType(0, 1, true);
	} else {
		// Crusader kills processes even for object 0 when switching.
		SnapProcess::get_instance()->clearEggs();
		CameraProcess::ResetCameraProcess();
		Kernel::get_instance()->killAllProcessesNotOfTypeExcludeCurrent(1, true);
		Kernel::get_instance()->addProcess(new SchedulerProcess());
	}

	debug(1, "Loading Fixed items in map %u", newmap);
	Common::SeekableReadStream *items = GameData::get_instance()->getFixed()
	                     ->get_datasource(newmap);
	_maps[newmap]->loadFixed(items);
	delete items;

	_currentMap->loadMap(_maps[newmap]);

	// Update camera
	if (GAME_IS_U8) {
		// TODO: This may not even be needed for U8, but reset in case camera
		// was looking at something other than the avatar during teleport.
		CameraProcess *camera = CameraProcess::GetCameraProcess();
		if (camera && camera->getItemNum() != kMainActorId) {
			CameraProcess::SetCameraProcess(new CameraProcess(kMainActorId));
		}
		CameraProcess::SetEarthquake(0);
	} else {
		// In Crusader, snap the camera to the avatar.  The snap process will
		// then find the right snap egg in the next frame.
		CameraProcess::SetCameraProcess(new CameraProcess(kMainActorId));
	}

	return true;
}

void World::loadNonFixed(Common::SeekableReadStream *rs) {
	FlexFile *f = new FlexFile(rs);

	debug(1, "Loading NonFixed items");

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

	debug(1, "Loading NPCs");

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
		debugC(kDebugObject, "%u,%u:\t(%d, %d, %d),\t%04X, %u, %u, u",
			shape, frame, x, y, z, flags, quality, npcnum, mapnum);
#endif

		Actor *actor = ItemFactory::createActor(shape, frame, quality,
		                                        flags | Item::FLG_IN_NPC_LIST,
		                                        npcnum, mapnum,
		                                        Item::EXT_PERMANENT_NPC, false);
		if (!actor) {
			warning("Couldn't create actor");
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
		Point3 pt = av->getLocation();
		g_debugger->debugPrintf("%d,%d,%d)\n", pt.x, pt.y, pt.z);
	} else {
		g_debugger->debugPrintf("missing (null)\n");
	}
}

void World::save(Common::WriteStream *ws) {
	ws->writeUint32LE(_currentMap->getNum());

	ws->writeUint16LE(_currentMap->_eggHatcher);

	if (GAME_IS_CRUSADER) {
		ws->writeByte(_alertActive ? 1 : 0);
		ws->writeByte(_difficulty);
		ws->writeUint16LE(_controlledNPCNum);
		ws->writeUint32LE(_vargasShield);
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
		_controlledNPCNum = rs->readUint16LE();
		_vargasShield = rs->readUint32LE();
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

	// Integrity check
	if (mapcount > _maps.size()) {
		warning("Invalid mapcount in save: %d.  Corrupt save?", mapcount);
		return false;
	}

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

	if (GAME_IS_REMORSE) {
		setAlertActiveRemorse(active);
	} else {
		setAlertActiveRegret(active);
	}
}

void World::setAlertActiveRemorse(bool active)
{
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
		assert(item);
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

void World::setAlertActiveRegret(bool active)
{
	setAlertActiveRemorse(active);

	LOOPSCRIPT(offscript, LS_OR(LS_SHAPE_EQUAL(0x660), LS_SHAPE_EQUAL(0x661)));
	LOOPSCRIPT(onscript, LS_OR(LS_SHAPE_EQUAL(0x662), LS_SHAPE_EQUAL(0x663)));

	const uint8 *script = active ? onscript : offscript;
	// note: size should be the same, but just to be explicit.
	int scriptlen = active ? sizeof(onscript) : sizeof(offscript);

	UCList itemlist(2);
	_world->getCurrentMap()->areaSearch(&itemlist, script, scriptlen,
										nullptr, 0xffff, false);
	for (uint32 i = 0; i < itemlist.getSize(); i++) {
		uint16 itemid = itemlist.getuint16(i);
		Item *item = getItem(itemid);
		assert(item);
		switch (item->getShape()) {
			case 0x660:
				item->setShape(0x663);
				break;
			case 0x661:
				item->setShape(0x662);
				break;
			case 0x662:
				item->setShape(0x661);
				break;
			case 0x663:
				item->setShape(0x660);
				break;
			default:
				warning("unexpected shape %d returned from search", item->getShape());
				break;
		}
		item->setFrame(0);
	}
}

void World::setGameDifficulty(uint8 difficulty) {
   _difficulty = difficulty;
   if (GAME_IS_REMORSE) {
	   // HACK: Set ammo data for BA-40 in higher 2 difficulty levels
	   // This would be better handled in the ini file somehow?
	   const ShapeInfo *si = GameData::get_instance()->getMainShapes()->getShapeInfo(0x32E);
	   if (si && si->_weaponInfo) {
		   WeaponInfo *wi = si->_weaponInfo;
		   wi->_clipSize = 20;
		   if (difficulty > 1) {
			   wi->_ammoShape = 0x33D;
			   wi->_ammoType = 1;
		   } else {
			   wi->_ammoShape = 0;
			   wi->_ammoType = 0;
		   }
	   }
   }
}


void World::setControlledNPCNum(uint16 num) {
	uint16 oldnpc = _controlledNPCNum;
	_controlledNPCNum = num;
	Actor *previous = getActor(oldnpc);
	if (previous && !previous->isDead() && previous->isInCombat()) {
		previous->clearInCombat();
	}

	Actor *controlled = getActor(num);
	if (controlled) {
		if (num != 1) {
			Kernel::get_instance()->killProcesses(num, Kernel::PROC_TYPE_ALL, true);
			if (controlled->isInCombat())
				controlled->clearInCombat();
		}
		Point3 pt = controlled->getCentre();
		CameraProcess::SetCameraProcess(new CameraProcess(pt));
	}

	TargetReticleProcess *t = TargetReticleProcess::get_instance();
	if (t) {
		t->avatarMoved();
	}
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

uint32 World::I_resetVargasShield(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	get_instance()->setVargasShield(500);
	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
