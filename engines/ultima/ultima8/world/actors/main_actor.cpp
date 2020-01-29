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

#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/teleport_egg.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/teleport_to_egg_process.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/actors/avatar_death_process.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/wpn_ovlay_dat.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/actors/avatar_gravity_process.h"
#include "ultima/ultima8/audio/music_process.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MainActor, Actor)

MainActor::MainActor() : justTeleported(false), accumStr(0), accumDex(0),
	accumInt(0) {

}

MainActor::~MainActor() {

}

GravityProcess *MainActor::ensureGravityProcess() {
	AvatarGravityProcess *p = 0;
	if (gravitypid) {
		p = p_dynamic_cast<AvatarGravityProcess *>(
		        Kernel::get_instance()->getProcess(gravitypid));
	} else {
		p = new AvatarGravityProcess(this, 0);
		Kernel::get_instance()->addProcess(p);
		p->init();
	}
	assert(p);
	return p;
}

bool MainActor::CanAddItem(Item *item, bool checkwghtvol) {
	const unsigned int backpack_shape = 529; //!! *cough* constant

	if (!Actor::CanAddItem(item, checkwghtvol)) return false;
	if (item->getParent() == objid) return true; // already in here

	// now check 'equipment slots'
	// we can have one item of each equipment type, plus one backpack

	uint32 equiptype = item->getShapeInfo()->equiptype;
	bool backpack = (item->getShape() == backpack_shape);

	// valid item type?
	if (equiptype == ShapeInfo::SE_NONE && !backpack) return false;

	Std::list<Item *>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		uint32 cet = (*iter)->getShapeInfo()->equiptype;
		bool cbackpack = ((*iter)->getShape() == backpack_shape);

		// already have an item with the same equiptype
		if (cet == equiptype || (cbackpack && backpack)) return false;
	}

	return true;
}

bool MainActor::addItem(Item *item, bool checkwghtvol) {
	if (!Actor::addItem(item, checkwghtvol)) return false;

	item->setFlag(FLG_EQUIPPED);

	uint32 equiptype = item->getShapeInfo()->equiptype;
	item->setZ(equiptype);

	return true;
}

void MainActor::teleport(int mapNum_, int32 x_, int32 y_, int32 z_) {
	World *world = World::get_instance();

	// (attempt to) load the new map
	if (!world->switchMap(mapNum_)) {
		perr << "MainActor::teleport(): switchMap() failed!" << Std::endl;
		return;
	}

	Actor::teleport(mapNum_, x_, y_, z_);
	justTeleported = true;
}

// teleport to TeleportEgg
// NB: be careful when calling this from a process, as it might kill
// all running processes
void MainActor::teleport(int mapNum_, int teleport_id) {
	int oldmap = getMapNum();
	int32 oldx, oldy, oldz;
	getLocation(oldx, oldy, oldz);

	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();

	pout << "MainActor::teleport(): teleporting to map " << mapNum_
	     << ", egg " << teleport_id << Std::endl;

	setMapNum(mapNum_);

	// (attempt to) load the new map
	if (!world->switchMap(mapNum_)) {
		perr << "MainActor::teleport(): switchMap() failed!" << Std::endl;
		setMapNum(oldmap);
		return;
	}

	// find destination
	TeleportEgg *egg = currentmap->findDestination(teleport_id);
	if (!egg) {
		perr << "MainActor::teleport(): destination egg not found!"
		     << Std::endl;
		teleport(oldmap, oldx, oldy, oldz);
		return;
	}
	int32 xv, yv, zv;
	egg->getLocation(xv, yv, zv);

	pout << "Found destination: " << xv << "," << yv << "," << zv << Std::endl;
	egg->dumpInfo();

	Actor::teleport(mapNum_, xv, yv, zv);
	justTeleported = true;
}

uint16 MainActor::getDefenseType() {
	uint16 type = 0;

	Std::list<Item *>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		uint32 frameNum = (*iter)->getFrame();
		ShapeInfo *si = (*iter)->getShapeInfo();
		if (si->armourinfo) {
			type |= si->armourinfo[frameNum].defense_type;
		}
	}

	return type;
}

uint32 MainActor::getArmourClass() {
	uint32 armour = 0;

	Std::list<Item *>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		uint32 frameNum = (*iter)->getFrame();
		ShapeInfo *si = (*iter)->getShapeInfo();
		if (si->armourinfo) {
			armour += si->armourinfo[frameNum].armour_class;
		}
		if (si->weaponinfo) {
			armour += si->weaponinfo->armour_bonus;
		}
	}

	return armour;
}

int16 MainActor::getDefendingDex() {
	int16 dex = getDex();

	Item *weapon = getItem(getEquip(ShapeInfo::SE_WEAPON));
	if (weapon) {
		ShapeInfo *si = weapon->getShapeInfo();
		assert(si->weaponinfo);
		dex += si->weaponinfo->dex_defend_bonus;
	}

	if (dex <= 0) dex = 1;

	return dex;
}

int16 MainActor::getAttackingDex() {
	int16 dex = getDex();

	Item *weapon = getItem(getEquip(ShapeInfo::SE_WEAPON));
	if (weapon) {
		ShapeInfo *si = weapon->getShapeInfo();
		assert(si->weaponinfo);
		dex += si->weaponinfo->dex_attack_bonus;
	}

	return dex;
}

uint16 MainActor::getDamageType() {
	Item *weapon = getItem(getEquip(ShapeInfo::SE_WEAPON));

	if (weapon) {
		// weapon equipped?

		ShapeInfo *si = weapon->getShapeInfo();
		assert(si->weaponinfo);

		return si->weaponinfo->damage_type;
	}

	return Actor::getDamageType();
}

int MainActor::getDamageAmount() {
	int damage = 0;

	if (getLastAnim() == Animation::kick) {
		// kick

		int kick_bonus = 0;
		Item *legs = getItem(getEquip(ShapeInfo::SE_LEGS));
		if (legs) {
			ShapeInfo *si = legs->getShapeInfo();
			assert(si->armourinfo);
			kick_bonus = si->armourinfo[legs->getFrame()].kick_attack_bonus;
		}

		damage = (getRandom() % (getStr() / 2 + 1)) + kick_bonus;

		return damage;

	}

	Item *weapon = getItem(getEquip(ShapeInfo::SE_WEAPON));


	if (weapon) {
		// weapon equipped?

		ShapeInfo *si = weapon->getShapeInfo();
		assert(si->weaponinfo);

		int base = si->weaponinfo->base_damage;
		int mod = si->weaponinfo->damage_modifier;

		damage = (getRandom() % (mod + 1)) + base + getStr() / 5;

		return damage;
	}

	// no weapon?

	damage = (getRandom() % (getStr() / 2 + 1)) + 1;

	return damage;
}

void MainActor::setInCombat() {
	setActorFlag(ACT_INCOMBAT);
	MusicProcess::get_instance()->playCombatMusic(98); // CONSTANT!
}

void MainActor::clearInCombat() {
	clearActorFlag(ACT_INCOMBAT);
	MusicProcess::get_instance()->restoreMusic();
}

ProcId MainActor::die(uint16 damageType) {
	ProcId animprocid = Actor::die(damageType);

	Ultima8Engine *app = p_dynamic_cast<Ultima8Engine *>(Ultima8Engine::get_instance());
	assert(app);

	app->setAvatarInStasis(true);

	Process *deathproc = new AvatarDeathProcess();
	Kernel::get_instance()->addProcess(deathproc);

	Process *delayproc = new DelayProcess(30 * 5); // 5 seconds
	Kernel::get_instance()->addProcess(delayproc);

	Process *animproc = Kernel::get_instance()->getProcess(animprocid);

	if (animproc)
		delayproc->waitFor(animproc);

	deathproc->waitFor(delayproc);

	MusicProcess *music = MusicProcess::get_instance();
	if (music) {
		music->unqueueMusic();
		music->playCombatMusic(44); // CONSTANT!!
	};

	return animprocid;
}


void MainActor::ConCmd_teleport(const Console::ArgvType &argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		pout << "Cheats are disabled" << Std::endl;
		return;
	}
	MainActor *mainactor = getMainActor();
	int curmap = mainactor->getMapNum();

	switch (argv.size() - 1) {
	case 1:
		mainactor->teleport(curmap,
		                    strtol(argv[1].c_str(), 0, 0));
		break;
	case 2:
		mainactor->teleport(strtol(argv[1].c_str(), 0, 0),
		                    strtol(argv[2].c_str(), 0, 0));
		break;
	case 3:
		mainactor->teleport(curmap,
		                    strtol(argv[1].c_str(), 0, 0),
		                    strtol(argv[2].c_str(), 0, 0),
		                    strtol(argv[3].c_str(), 0, 0));
		break;
	case 4:
		mainactor->teleport(strtol(argv[1].c_str(), 0, 0),
		                    strtol(argv[2].c_str(), 0, 0),
		                    strtol(argv[3].c_str(), 0, 0),
		                    strtol(argv[4].c_str(), 0, 0));
		break;
	default:
		pout << "teleport usage:" << Std::endl;
		pout << "teleport <mapnum> <x> <y> <z>: teleport to (x,y,z) on map mapnum" << Std::endl;
		pout << "teleport <x> <y> <z>: teleport to (x,y,z) on current map" << Std::endl;
		pout << "teleport <mapnum> <eggnum>: teleport to target egg eggnum on map mapnum" << Std::endl;
		pout << "teleport <eggnum>: teleport to target egg eggnum on current map" << Std::endl;
		break;
	}
}

void MainActor::ConCmd_mark(const Console::ArgvType &argv) {
	if (argv.size() == 1) {
		pout << "Usage: mark <mark>: set named mark to this location" << Std::endl;
		return;
	}

	SettingManager *settings = SettingManager::get_instance();
	MainActor *mainactor = getMainActor();
	int curmap = mainactor->getMapNum();
	int32 x, y, z;
	mainactor->getLocation(x, y, z);

	Pentagram::istring confkey = "marks/" + argv[1];
	char buf[100]; // large enough for 4 ints
	sprintf(buf, "%d %d %d %d", curmap, x, y, z);

	settings->set(confkey, buf);
	settings->write(); //!! FIXME: clean this up

	pout << "Set mark \"" << argv[1].c_str() << "\" to " << buf << Std::endl;
}

void MainActor::ConCmd_recall(const Console::ArgvType &argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		pout << "Cheats are disabled" << Std::endl;
		return;
	}
	if (argv.size() == 1) {
		pout << "Usage: recall <mark>: recall to named mark" << Std::endl;
		return;
	}

	SettingManager *settings = SettingManager::get_instance();
	MainActor *mainactor = getMainActor();
	Pentagram::istring confkey = "marks/" + argv[1];
	Std::string target;
	if (!settings->get(confkey, target)) {
		pout << "recall: no such mark" << Std::endl;
		return;
	}

	int t[4];
	int n = sscanf(target.c_str(), "%d%d%d%d", &t[0], &t[1], &t[2], &t[3]);
	if (n != 4) {
		pout << "recall: invalid mark" << Std::endl;
		return;
	}

	mainactor->teleport(t[0], t[1], t[2], t[3]);
}

void MainActor::ConCmd_listmarks(const Console::ArgvType &argv) {
	SettingManager *settings = SettingManager::get_instance();
	Std::vector<Pentagram::istring> marks;
	marks = settings->listDataKeys("marks");
	for (Std::vector<Pentagram::istring>::iterator iter = marks.begin();
	        iter != marks.end(); ++iter) {
		pout << (*iter) << Std::endl;
	}
}

void MainActor::ConCmd_maxstats(const Console::ArgvType &argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		pout << "Cheats are disabled" << Std::endl;
		return;
	}
	MainActor *mainactor = getMainActor();

	// constants!!
	mainactor->setStr(25);
	mainactor->setDex(25);
	mainactor->setInt(25);
	mainactor->setHP(mainactor->getMaxHP());
	mainactor->setMana(mainactor->getMaxMana());

	AudioProcess *audioproc = AudioProcess::get_instance();
	if (audioproc) audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
}

void MainActor::ConCmd_heal(const Console::ArgvType &argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		pout << "Cheats are disabled" << Std::endl;
		return;
	}
	MainActor *mainactor = getMainActor();

	mainactor->setHP(mainactor->getMaxHP());
	mainactor->setMana(mainactor->getMaxMana());
}


void MainActor::accumulateStr(int n) {
	// already max?
	if (strength == 25) return; //!! constant

	accumStr += n;
	if (accumStr >= 650 || getRandom() % (650 - accumStr) == 0) { //!! constant
		strength++;
		accumStr = 0;
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
		pout << "Gained strength!" << Std::endl;
	}
}

void MainActor::accumulateDex(int n) {
	// already max?
	if (dexterity == 25) return; //!! constant

	accumDex += n;
	if (accumDex >= 650 || getRandom() % (650 - accumDex) == 0) { //!! constant
		dexterity++;
		accumDex = 0;
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
		pout << "Gained dexterity!" << Std::endl;
	}
}

void MainActor::accumulateInt(int n) {
	// already max?
	if (intelligence == 25) return; //!! constant

	accumInt += n;
	if (accumInt >= 650 || getRandom() % (650 - accumInt) == 0) { //!! constant
		intelligence++;
		accumInt = 0;
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
		pout << "Gained intelligence!" << Std::endl;
	}
}

void MainActor::getWeaponOverlay(const WeaponOverlayFrame *&frame_, uint32 &shape_) {
	shape_ = 0;
	frame_ = 0;

	if (!isInCombat() && lastanim != Animation::unreadyWeapon) return;

	ObjId weaponid = getEquip(ShapeInfo::SE_WEAPON);
	Item *weapon = getItem(weaponid);
	if (!weapon) return;

	ShapeInfo *shapeinfo = weapon->getShapeInfo();
	if (!shapeinfo) return;

	WeaponInfo *weaponinfo = shapeinfo->weaponinfo;
	if (!weaponinfo) return;

	shape_ = weaponinfo->overlay_shape;

	WpnOvlayDat *wpnovlay = GameData::get_instance()->getWeaponOverlay();
	frame_ = wpnovlay->getOverlayFrame(lastanim, weaponinfo->overlay_type,
	                                  direction, animframe);

	if (frame_ == 0) shape_ = 0;
}

void MainActor::saveData(ODataSource *ods) {
	Actor::saveData(ods);
	uint8 jt = justTeleported ? 1 : 0;
	ods->write1(jt);
	ods->write4(accumStr);
	ods->write4(accumDex);
	ods->write4(accumInt);
	uint8 namelength = static_cast<uint8>(name.size());
	ods->write1(namelength);
	for (unsigned int i = 0; i < namelength; ++i)
		ods->write1(static_cast<uint8>(name[i]));

}

bool MainActor::loadData(IDataSource *ids, uint32 version) {
	if (!Actor::loadData(ids, version)) return false;

	justTeleported = (ids->read1() != 0);
	accumStr = static_cast<int32>(ids->read4());
	accumDex = static_cast<int32>(ids->read4());
	accumInt = static_cast<int32>(ids->read4());

	uint8 namelength = ids->read1();
	name.resize(namelength);
	for (unsigned int i = 0; i < namelength; ++i)
		name[i] = ids->read1();

	return true;
}

uint32 MainActor::I_teleportToEgg(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(mapnum);
	ARG_UINT16(teleport_id);
	ARG_UINT16(unknown); // 0/1

	return Kernel::get_instance()->addProcess(
	           new TeleportToEggProcess(mapnum, teleport_id));
}

uint32 MainActor::I_accumulateStrength(const uint8 *args,
                                       unsigned int /*argsize*/) {
	ARG_SINT16(n);
	MainActor *av = getMainActor();
	av->accumulateStr(n);

	return 0;
}

uint32 MainActor::I_accumulateDexterity(const uint8 *args,
                                        unsigned int /*argsize*/) {
	ARG_SINT16(n);
	MainActor *av = getMainActor();
	av->accumulateDex(n);

	return 0;
}

uint32 MainActor::I_accumulateIntelligence(const uint8 *args,
        unsigned int /*argsize*/) {
	ARG_SINT16(n);
	MainActor *av = getMainActor();
	av->accumulateInt(n);

	return 0;
}

uint32 MainActor::I_clrAvatarInCombat(const uint8 * /*args*/,
                                      unsigned int /*argsize*/) {
	MainActor *av = getMainActor();
	av->clearInCombat();

	return 0;
}

uint32 MainActor::I_setAvatarInCombat(const uint8 * /*args*/,
                                      unsigned int /*argsize*/) {
	MainActor *av = getMainActor();
	av->setInCombat();

	return 0;
}

uint32 MainActor::I_isAvatarInCombat(const uint8 * /*args*/,
                                     unsigned int /*argsize*/) {
	MainActor *av = getMainActor();
	if (av->isInCombat())
		return 1;
	else
		return 0;
}

void MainActor::ConCmd_name(const Console::ArgvType &argv) {
	MainActor *av = getMainActor();
	if (argv.size() > 1)
		av->setName(argv[1]);

	pout << "MainActor::name = \"" << av->getName() << "\"" << Std::endl;
}

void MainActor::ConCmd_useBackpack(const Console::ArgvType &argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		pout << "Can't: avatarInStasis" << Std::endl;
		return;
	}
	MainActor *av = getMainActor();
	Item *backpack = getItem(av->getEquip(7));
	if (backpack)
		backpack->callUsecodeEvent_use();
}

void MainActor::ConCmd_useInventory(const Console::ArgvType &argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		pout << "Can't: avatarInStasis" << Std::endl;
		return;
	}
	MainActor *av = getMainActor();
	av->callUsecodeEvent_use();
}

void MainActor::useInventoryItem(uint32 shapenum) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		pout << "Can't: avatarInStasis" << Std::endl;
		return;
	}
	LOOPSCRIPT(script, LS_SHAPE_EQUAL(shapenum));
	UCList uclist(2);
	this->containerSearch(&uclist, script, sizeof(script), true);
	if (uclist.getSize() < 1)
		return;

	uint16 oId = uclist.getuint16(0);
	Item *item = getItem(oId);
	item->callUsecodeEvent_use();

}

void MainActor::ConCmd_useRecall(const Console::ArgvType &argv) {
	MainActor *av = getMainActor();
	av->useInventoryItem(833);
}

void MainActor::ConCmd_useBedroll(const Console::ArgvType &argv) {
	MainActor *av = getMainActor();
	av->useInventoryItem(534);
}

void MainActor::ConCmd_useKeyring(const Console::ArgvType &argv) {
	MainActor *av = getMainActor();
	av->useInventoryItem(79);
}

void MainActor::ConCmd_toggleCombat(const Console::ArgvType &argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		pout << "Can't: avatarInStasis" << Std::endl;
		return;
	}
	MainActor *av = getMainActor();
	av->toggleInCombat();
}

void MainActor::ConCmd_toggleInvincibility(const Console::ArgvType &argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		pout << "Cheats are disabled" << Std::endl;
		return;
	}
	MainActor *av = getMainActor();

	if (av->getActorFlags() & Actor::ACT_INVINCIBLE) {

		av->clearActorFlag(Actor::ACT_INVINCIBLE);
		pout << "Avatar is no longer invincible." << Std::endl;


	} else {

		av->setActorFlag(Actor::ACT_INVINCIBLE);
		pout << "Avatar invincible." << Std::endl;

	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
