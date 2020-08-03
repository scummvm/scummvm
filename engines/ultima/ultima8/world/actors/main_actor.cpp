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
#include "ultima/ultima8/world/target_reticle_process.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/actors/avatar_death_process.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/anim_dat.h"
#include "ultima/ultima8/graphics/wpn_ovlay_dat.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/gumps/cru_pickup_area_gump.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/fire_type.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/world/actors/avatar_gravity_process.h"
#include "ultima/ultima8/audio/music_process.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MainActor)

MainActor::MainActor() : _justTeleported(false), _accumStr(0), _accumDex(0),
	_accumInt(0), _cruBatteryType(ChemicalBattery), _keycards(0),
	_activeInvItem(0), _shieldType(0), _shieldSpriteProc(0) {
}

MainActor::~MainActor() {
}

GravityProcess *MainActor::ensureGravityProcess() {
	AvatarGravityProcess *p;
	if (_gravityPid) {
		p = dynamic_cast<AvatarGravityProcess *>(
		        Kernel::get_instance()->getProcess(_gravityPid));
	} else {
		p = new AvatarGravityProcess(this, 0);
		Kernel::get_instance()->addProcess(p);
		p->init();
	}
	assert(p);
	return p;
}

bool MainActor::CanAddItem(Item *item, bool checkwghtvol) {

	if (!Actor::CanAddItem(item, checkwghtvol)) return false;
	if (item->getParent() == _objId) return true; // already in here

	// now check 'equipment slots'
	// we can have one item of each equipment type, plus one backpack

	if (GAME_IS_U8) {
		const unsigned int backpack_shape = 529; //!! *cough* constant
		uint32 equiptype = item->getShapeInfo()->_equipType;
		bool backpack = (item->getShape() == backpack_shape);

		// valid item type?
		if (equiptype == ShapeInfo::SE_NONE && !backpack) return false;

		Std::list<Item *>::iterator iter;
		for (iter = _contents.begin(); iter != _contents.end(); ++iter) {
			uint32 cet = (*iter)->getShapeInfo()->_equipType;
			bool cbackpack = ((*iter)->getShape() == backpack_shape);

			// already have an item with the same equiptype
			if (cet == equiptype || (cbackpack && backpack)) return false;
		}
	} else if (GAME_IS_CRUSADER) {
		// TODO: Enforce the number of slots by family here (weapon / ammo / other)
		// For now just enforce no limit.
		return true;
	}

	return true;
}

bool MainActor::addItem(Item *item, bool checkwghtvol) {
	if (!Actor::addItem(item, checkwghtvol)) return false;

	item->setFlag(FLG_EQUIPPED);

	uint32 equiptype = item->getShapeInfo()->_equipType;
	item->setZ(equiptype);

	return true;
}

int16 MainActor::addItemCru(Item *item, bool showtoast) {
	// This code is a little ugly, it's a somewhat close
	// re-implementation of the original and could do
	// with some cleanup.

	if (!item || !item->getShape())
		return 0;

	int shapeno = item->getShape();
	int32 x, y, z;
	getLocation(x, y, z);

	CruPickupAreaGump *pickupArea = CruPickupAreaGump::get_instance();
	assert(pickupArea);

	if (shapeno == 0x4ed) {
		Item *credits = getFirstItemWithShape(shapeno, true);
		if (credits) {
			uint16 q = item->getQuality();
			uint32 newq = credits->getQuality() + q;
			if (newq > 64000)
				newq = 64000;
			credits->setQuality(newq);
			credits->callUsecodeEvent_combine();
			if (showtoast)
				pickupArea->addPickup(item);
			item->destroy();
		} else {
			item->setFrame(0);
			item->moveToContainer(this);
			if (!_activeInvItem)
				_activeInvItem = item->getObjId();
			if (showtoast)
				pickupArea->addPickup(item);
		}
		return 1;
	}

	switch (static_cast<ShapeInfo::SFamily>(item->getShapeInfo()->_family)) {
	case ShapeInfo::SF_CRUWEAPON: {	// 0xa
		Item *weapon = getFirstItemWithShape(shapeno, true);
		if (!weapon) {
			// New weapon. Add it.
			const WeaponInfo *winfo = item->getShapeInfo()->_weaponInfo;
			assert(winfo);
			if (winfo->_ammoType == 0) {
				item->setQuality(0);
				item->callUsecodeEvent_combine();
			} else {
				warning("TODO: Get default count for ammo type %d", winfo->_ammoType);
				item->setQuality(100);
			}
			item->setLocation(x, y, z);
			item->moveToContainer(this);
			if (!_activeWeapon)
				_activeWeapon = item->getObjId();
			if (showtoast)
				pickupArea->addPickup(item);
		}
		break;
	}
	case ShapeInfo::SF_CRUAMMO:	{	// 0xb
		Item *ammo = getFirstItemWithShape(shapeno, true);
		if (!ammo) {
			// don't have this ammo yet, add it
			item->setQuality(1);
			item->callUsecodeEvent_combine();
			item->moveToContainer(this);
			if (showtoast)
				pickupArea->addPickup(item);
			return 1;
		} else {
			// already have this, add some ammo.
			uint16 q = ammo->getQuality();
			if (q < 0x14) {
				ammo->setQuality(q + 1);
				ammo->callUsecodeEvent_combine();
				if (showtoast)
					pickupArea->addPickup(item);
				item->destroy();
				return 1;
			}
		}
		break;
	}
	case ShapeInfo::SF_CRUBOMB:		// 0xc
	case ShapeInfo::SF_CRUINVITEM:	// 0xd
		if (shapeno == 0x111) {
			addKeycard(item->getQuality() & 0xff);
			if (showtoast) {
				pickupArea->addPickup(item);
			}
			item->destroy();
			return 1;
		} else if ((shapeno == 0x3a2) || (shapeno == 0x3a3) || (shapeno == 0x3a4)) {
			// Batteries
			if (showtoast)
				pickupArea->addPickup(item);
			item->destroy();
			int plusenergy = 0;
			CruBatteryType oldbattery = _cruBatteryType;
			if (shapeno == 0x3a2) {
				if (oldbattery == NoBattery) {
					setBatteryType(ChemicalBattery);
				} else {
					plusenergy = 0x9c4;
				}
			} else if (shapeno == 0x3a4) {
				if (oldbattery < FusionBattery) {
					setBatteryType(FusionBattery);
				} else {
					plusenergy = 5000;
				}
			} else if (shapeno == 0x3a3) {
				if (oldbattery < FissionBattery) {
					setBatteryType(FissionBattery);
				} else {
					plusenergy = 10000;
				}
			}
			if (plusenergy) {
				int newenergy = getMana() + plusenergy;
				if (newenergy > getMaxEnergy())
					newenergy = getMaxEnergy();
				setMana(newenergy);
			}
			return 1;
		} else {
			Item *existing = getFirstItemWithShape(shapeno, true);
			if (!existing) {
				if ((shapeno == 0x52e) || (shapeno == 0x52f) || (shapeno == 0x530)) {
					warning("TODO: Properly handle giving avatar a shield 0x%x", shapeno);
					return 0;
				} else {
					item->setFrame(0);
					item->setQuality(1);
					item->callUsecodeEvent_combine();
					item->moveToContainer(this);
					if (showtoast)
						pickupArea->addPickup(item);
					if (!_activeInvItem)
						_activeInvItem = item->getObjId();
					return 1;
				}
			} else {
				// Already have this item..
				if ((shapeno == 0x52e) || (shapeno == 0x52f) || (shapeno == 0x530)) {
					// shields, already have one, destroy the new one.
					item->destroy();
					return 1;
				} else if (shapeno == 0x560) {
					uint16 q = existing->getQuality();
					if (q < 0x14) {
						existing->setQuality(q + 1);
						existing->callUsecodeEvent_combine();
						if (showtoast)
							pickupArea->addPickup(item);
						item->destroy();
						return 1;
					}
				} else {
					uint16 q = existing->getQuality();
					if (q < 10) {
						existing->setQuality(q + 1);
						existing->callUsecodeEvent_combine();
						if (showtoast)
							pickupArea->addPickup(item);
						item->destroy();
						return 1;
					}
				}
			}
		}
		break;
	default:
		break;
	}

	return 0;
}

void MainActor::teleport(int mapNum_, int32 x_, int32 y_, int32 z_) {
	World *world = World::get_instance();

	// (attempt to) load the new map
	if (!world->switchMap(mapNum_)) {
		perr << "MainActor::teleport(): switchMap() failed!" << Std::endl;
		return;
	}

	Actor::teleport(mapNum_, x_, y_, z_);
	_justTeleported = true;
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
	_justTeleported = true;
}

uint16 MainActor::getDefenseType() const {
	uint16 type = 0;

	Std::list<Item *>::const_iterator iter;
	for (iter = _contents.begin(); iter != _contents.end(); ++iter) {
		uint32 frameNum = (*iter)->getFrame();
		const ShapeInfo *si = (*iter)->getShapeInfo();
		if (si->_armourInfo) {
			type |= si->_armourInfo[frameNum]._defenseType;
		}
	}

	return type;
}

uint32 MainActor::getArmourClass() const {
	uint32 armour = 0;

	Std::list<Item *>::const_iterator iter;
	for (iter = _contents.begin(); iter != _contents.end(); ++iter) {
		uint32 frameNum = (*iter)->getFrame();
		const ShapeInfo *si = (*iter)->getShapeInfo();
		if (si->_armourInfo) {
			armour += si->_armourInfo[frameNum]._armourClass;
		}
		if (si->_weaponInfo) {
			armour += si->_weaponInfo->_armourBonus;
		}
	}

	return armour;
}

int16 MainActor::getDefendingDex() const {
	int16 dex = getDex();

	Item *weapon = getItem(getEquip(ShapeInfo::SE_WEAPON));
	if (weapon) {
		const ShapeInfo *si = weapon->getShapeInfo();
		assert(si->_weaponInfo);
		dex += si->_weaponInfo->_dexDefendBonus;
	}

	if (dex <= 0) dex = 1;

	return dex;
}

int16 MainActor::getAttackingDex() const {
	int16 dex = getDex();

	Item *weapon = getItem(getEquip(ShapeInfo::SE_WEAPON));
	if (weapon) {
		const ShapeInfo *si = weapon->getShapeInfo();
		assert(si->_weaponInfo);
		dex += si->_weaponInfo->_dexAttackBonus;
	}

	return dex;
}

uint16 MainActor::getDamageType() const {
	Item *weapon = getItem(getEquip(ShapeInfo::SE_WEAPON));

	if (weapon) {
		// weapon equipped?

		const ShapeInfo *si = weapon->getShapeInfo();
		assert(si->_weaponInfo);

		return si->_weaponInfo->_damageType;
	}

	return Actor::getDamageType();
}

int MainActor::getDamageAmount() const {
	int damage = 0;

	if (getLastAnim() == Animation::kick) {
		// kick

		int kick_bonus = 0;
		Item *legs = getItem(getEquip(ShapeInfo::SE_LEGS));
		if (legs) {
			const ShapeInfo *si = legs->getShapeInfo();
			assert(si->_armourInfo);
			kick_bonus = si->_armourInfo[legs->getFrame()]._kickAttackBonus;
		}

		damage = (getRandom() % (getStr() / 2 + 1)) + kick_bonus;

		return damage;

	}

	Item *weapon = getItem(getEquip(ShapeInfo::SE_WEAPON));


	if (weapon) {
		// weapon equipped?

		const ShapeInfo *si = weapon->getShapeInfo();
		assert(si->_weaponInfo);

		int base = si->_weaponInfo->_baseDamage;
		int mod = si->_weaponInfo->_damageModifier;

		damage = (getRandom() % (mod + 1)) + base + getStr() / 5;

		return damage;
	}

	// no weapon?

	damage = (getRandom() % (getStr() / 2 + 1)) + 1;

	return damage;
}

void MainActor::setInCombat() {
	setActorFlag(ACT_INCOMBAT);
	if (GAME_IS_U8)
		MusicProcess::get_instance()->playCombatMusic(98); // CONSTANT!
}

void MainActor::clearInCombat() {
	clearActorFlag(ACT_INCOMBAT);
	if (GAME_IS_U8)
		MusicProcess::get_instance()->restoreMusic();
}

ProcId MainActor::die(uint16 damageType) {
	ProcId animprocid = Actor::die(damageType);

	Ultima8Engine *app = Ultima8Engine::get_instance();
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
	if (GAME_IS_U8 && music) {
		music->unqueueMusic();
		music->playCombatMusic(44); // CONSTANT!!
	}

	if (GAME_IS_CRUSADER) {
		// Force a reticle update
		TargetReticleProcess::get_instance()->avatarMoved();
	}

	return animprocid;
}

void MainActor::accumulateStr(int n) {
	// already max?
	if (_strength == 25) return; //!! constant

	_accumStr += n;
	if (_accumStr >= 650 || getRandom() % (650 - _accumStr) == 0) { //!! constant
		_strength++;
		_accumStr = 0;
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
		pout << "Gained _strength!" << Std::endl;
	}
}

void MainActor::accumulateDex(int n) {
	// already max?
	if (_dexterity == 25) return; //!! constant

	_accumDex += n;
	if (_accumDex >= 650 || getRandom() % (650 - _accumDex) == 0) { //!! constant
		_dexterity++;
		_accumDex = 0;
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
		pout << "Gained _dexterity!" << Std::endl;
	}
}

void MainActor::accumulateInt(int n) {
	// already max?
	if (_intelligence == 25) return; //!! constant

	_accumInt += n;
	if (_accumInt >= 650 || getRandom() % (650 - _accumInt) == 0) { //!! constant
		_intelligence++;
		_accumInt = 0;
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
		pout << "Gained _intelligence!" << Std::endl;
	}
}

void MainActor::getWeaponOverlay(const WeaponOverlayFrame *&frame_, uint32 &shape_) {
	shape_ = 0;
	frame_ = 0;

	if (!isInCombat() && _lastAnim != Animation::unreadyWeapon) return;

	uint32 action = AnimDat::getActionNumberForSequence(_lastAnim, this);

	ObjId weaponid;
	if (GAME_IS_U8)
		weaponid = getEquip(ShapeInfo::SE_WEAPON);
	else
		weaponid = getActiveWeapon();

	Item *weapon = getItem(weaponid);
	if (!weapon) return;

	const ShapeInfo *shapeinfo = weapon->getShapeInfo();
	if (!shapeinfo) return;

	WeaponInfo *weaponinfo = shapeinfo->_weaponInfo;
	if (!weaponinfo) return;

	shape_ = weaponinfo->_overlayShape;

	WpnOvlayDat *wpnovlay = GameData::get_instance()->getWeaponOverlay();
	frame_ = wpnovlay->getOverlayFrame(action, weaponinfo->_overlayType,
	                                  _direction, _animFrame);

	if (frame_ == 0) shape_ = 0;
}

int16 MainActor::getMaxEnergy() {
	switch (_cruBatteryType) {
		case ChemicalBattery:
			return 0x9c4; // docs say 2500, code says otherwise..
		case FissionBattery:
			return 5000;
		case FusionBattery:
			return 10000;
		default:
			return 0;
	}
}

bool MainActor::hasKeycard(int num) const {
	if (num > 31)
		return 0;

	return _keycards & (1 << num);
}

void MainActor::addKeycard(int bitno) {
	if (bitno > 31 || bitno < 0)
		return;
	_keycards |= (1 << bitno);
}

static uint16 getIdOfNextItemInList(const Std::vector<Item *> &items, uint16 current) {
	const int n = items.size();
	if (n == 0)
		return 0;
	if (n == 1)
		return items[0]->getObjId();

	int i;
	for (i = 0; i < n; i++) {
		if (items[i]->getObjId() == current) {
			i++;
			break;
		}
	}
	return items[i % n]->getObjId();
}

void MainActor::nextWeapon() {
	Std::vector<Item *> weapons;
	getItemsWithShapeFamily(weapons, ShapeInfo::SF_CRUWEAPON, true);
	_activeWeapon = getIdOfNextItemInList(weapons, _activeWeapon);
}

void MainActor::nextInvItem() {
	Std::vector<Item *> items;
	getItemsWithShapeFamily(items, ShapeInfo::SF_CRUINVITEM, true);
	_activeInvItem = getIdOfNextItemInList(items, _activeInvItem);
}


void MainActor::saveData(Common::WriteStream *ws) {
	Actor::saveData(ws);
	uint8 jt = _justTeleported ? 1 : 0;
	ws->writeByte(jt);
	ws->writeUint32LE(_accumStr);
	ws->writeUint32LE(_accumDex);
	ws->writeUint32LE(_accumInt);

	if (GAME_IS_CRUSADER) {
		ws->writeByte(static_cast<byte>(_cruBatteryType));
		ws->writeUint32LE(_keycards);
		ws->writeUint16LE(_activeInvItem);
		ws->writeUint16LE(_shieldType);
		ws->writeUint16LE(_shieldSpriteProc);
	}

	uint8 namelength = static_cast<uint8>(_name.size());
	ws->writeByte(namelength);
	for (unsigned int i = 0; i < namelength; ++i)
		ws->writeByte(static_cast<uint8>(_name[i]));

}

bool MainActor::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Actor::loadData(rs, version)) return false;

	_justTeleported = (rs->readByte() != 0);
	_accumStr = static_cast<int32>(rs->readUint32LE());
	_accumDex = static_cast<int32>(rs->readUint32LE());
	_accumInt = static_cast<int32>(rs->readUint32LE());

	if (GAME_IS_CRUSADER) {
		_cruBatteryType = static_cast<CruBatteryType>(rs->readByte());
		_keycards = rs->readUint32LE();
		_activeInvItem = rs->readUint16LE();
		_shieldType = rs->readUint16LE();
		_shieldSpriteProc = rs->readUint16LE();
	}

	uint8 namelength = rs->readByte();
	_name.resize(namelength);
	for (unsigned int i = 0; i < namelength; ++i)
		_name[i] = rs->readByte();

	return true;
}

uint32 MainActor::I_teleportToEgg(const uint8 *args, unsigned int argsize) {
	uint16 mapnum;
	if (argsize == 6) {
		ARG_UINT16(map);
		mapnum = map;
	} else {
		// Crusader teleport intrinsic 096 uses main actor map.
		// Intrinsic 079 provides a map argument.
		assert(argsize == 4);
		MainActor *av = getMainActor();
		mapnum = av->getMapNum();
	}

	ARG_UINT16(teleport_id);
	ARG_UINT16(put_in_stasis); // 0/1

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

uint32 MainActor::I_getMaxEnergy(const uint8 *args,
								 unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	MainActor *av = getMainActor();
	if (!av || actor != av) {
		return 0;
	}
	return av->getMaxEnergy();
}

uint32 MainActor::I_hasKeycard(const uint8 *args,
								 unsigned int /*argsize*/) {
	ARG_UINT16(num);
	MainActor *av = getMainActor();
	if (!av)
		return 0;
	return av->hasKeycard(num);
}

uint32 MainActor::I_clrKeycards(const uint8 *args,
								 unsigned int /*argsize*/) {
	MainActor *av = getMainActor();
	if (!av)
		return 0;
	av->clrKeycards();
	return 0;
}

uint32 MainActor::I_addItemCru(const uint8 *args,
								 unsigned int /*argsize*/) {
	MainActor *av = getMainActor();
	ARG_ITEM_FROM_ID(item);
	ARG_UINT16(showtoast);

	if (!av || !item)
		return 0;

	if (av->addItemCru(item, showtoast != 0))
		return 1;

	return 0;
}

uint32 MainActor::I_getNumberOfCredits(const uint8 *args,
unsigned int /*argsize*/) {
	MainActor *av = getMainActor();
	if (av) {
		Item *item = av->getFirstItemWithShape(0x4ed, true);
		if (item)
			return item->getQuality();
	}
	return 0;
}

void MainActor::useInventoryItem(uint32 shapenum) {
	Item *item = getFirstItemWithShape(shapenum, true);
	useInventoryItem(item);
}

void MainActor::useInventoryItem(Item *item) {
	if (!item)
		return;
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		pout << "Can't use item: avatarInStasis" << Std::endl;
		return;
	}
	const int32 shapenum = item->getShape();
	if (shapenum == 0x4ed && GAME_IS_CRUSADER) {
		// Do nothing for Credits
		return;
	}
	item->callUsecodeEvent_use();

	if (GAME_IS_CRUSADER && (shapenum != 0x4d4 && shapenum != 0x52d &&
							 shapenum != 0x530 && shapenum != 0x52f &&
							 shapenum != 0x52e)) {
		uint16 q = item->getQuality();
		item->setQuality(q - 1);
		item->callUsecodeEvent_combine();
		q = item->getQuality();
		if (q == 0) {
			const ObjId id = item->getObjId();
			item->destroy();
			if (id == _activeInvItem)
				nextInvItem();
		}
	}
}

int MainActor::receiveShieldHit(int damage, uint16 damage_type) {
	uint8 shieldtype = getShieldType();
	if (shieldtype == 3) {
		shieldtype = 4;
	}

	const FireType *firetype = GameData::get_instance()->getFireType(damage_type);
	int energy = getMana();
	Kernel *kernel = Kernel::get_instance();

	if (shieldtype && firetype && firetype->getShieldCost() && (firetype->getShieldMask() & shieldtype) && damage < energy) {
		setMana(energy - damage);
		damage = 0;
		AudioProcess *audio = AudioProcess::get_instance();
		audio->playSFX(0x48, 0x10, _objId, 1, true);

		// If there's no active shield sprite, create a new one.
		if (!_shieldSpriteProc || kernel->getProcess(_shieldSpriteProc) == nullptr) {
			// Create the shield damage sprite
			uint16 shieldsprite;
			uint16 shieldstartframe;
			uint16 shieldendframe;
			bool remembersprite;
			int32 x, y, z;

			switch (shieldtype) {
			case 1:
				shieldsprite = 0x5a9;
				shieldstartframe = 7;
				shieldendframe = 0xd;
				remembersprite = false;
				// NOTE: In the game, this is put in the location of the
				// hit.  For now just put in centre.
				getCentre(x, y, z);
				break;
			case 2:
				shieldsprite = 0x5a9;
				shieldstartframe = 0;
				shieldendframe = 6;
				remembersprite = false;
				getCentre(x, y, z);
				break;
			default:
				shieldsprite = 0x52b;
				shieldstartframe = 0;
				shieldendframe = 8;
				getLocation(x, y, z);
				x += 0x10;
				y += 0x18;
				remembersprite = true;
				break;
			}
			Process *p = new SpriteProcess(shieldsprite, shieldstartframe,
										   shieldendframe, 1, 4, x, y, z);
			kernel->addProcess(p);
			if (remembersprite) {
				_shieldSpriteProc = p->getPid();
			} else {
				_shieldSpriteProc = 0;
			}
		}
	}
	return damage;
}


} // End of namespace Ultima8
} // End of namespace Ultima
