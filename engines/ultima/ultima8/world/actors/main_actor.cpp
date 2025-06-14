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

#include "ultima/ultima.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/teleport_egg.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/teleport_to_egg_process.h"
#include "ultima/ultima8/world/target_reticle_process.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/gfx/shape_info.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/actors/avatar_death_process.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/anim_dat.h"
#include "ultima/ultima8/gfx/wpn_ovlay_dat.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
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
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/world/actors/anim_action.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MainActor)

ShapeInfo *MainActor::_kneelingShapeInfo = nullptr;

MainActor::MainActor() : _justTeleported(false), _accumStr(0), _accumDex(0),
	_accumInt(0), _cruBatteryType(ChemicalBattery), _keycards(0),
	_activeInvItem(0), _shieldType(0), _shieldSpriteProc(0) {
}

MainActor::~MainActor() {
	if (_kneelingShapeInfo) {
		delete _kneelingShapeInfo;
		_kneelingShapeInfo = nullptr;
	}
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

		for (const auto *i : _contents) {
			uint32 cet = i->getShapeInfo()->_equipType;
			bool cbackpack = (i->getShape() == backpack_shape);

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
	Point3 pt = getLocation();

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
				pickupArea->addPickup(item, true);
			item->destroy();
		} else {
			item->setFrame(0);
			item->moveToContainer(this);
			if (!_activeInvItem)
				_activeInvItem = item->getObjId();
			if (showtoast)
				pickupArea->addPickup(item, true);
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
				item->setQuality(winfo->_clipSize);
			}
			item->setLocation(pt);
			item->moveToContainer(this);
			if (!_activeWeapon)
				_activeWeapon = item->getObjId();
			if (showtoast)
				pickupArea->addPickup(item, false);
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
				pickupArea->addPickup(item, false);
			return 1;
		} else {
			// already have this, add some ammo.
			uint16 q = ammo->getQuality();
			if (q < 0x14) {
				ammo->setQuality(q + 1);
				ammo->callUsecodeEvent_combine();
				if (showtoast)
					pickupArea->addPickup(item, false);
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
				pickupArea->addPickup(item, false);
			}
			item->destroy();
			return 1;
		} else if ((shapeno == 0x3a2) || (shapeno == 0x3a3) || (shapeno == 0x3a4)) {
			// Batteries
			if (showtoast)
				pickupArea->addPickup(item, false);
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
				if (oldbattery < FissionBattery) {
					setBatteryType(FissionBattery);
				} else {
					plusenergy = 5000;
				}
			} else if (shapeno == 0x3a3) {
				if (oldbattery < FusionBattery) {
					setBatteryType(FusionBattery);
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
				// Shields. Note, these are the same in Remorse and Regret.
				if ((shapeno == 0x52e) || (shapeno == 0x52f) || (shapeno == 0x530)) {
					uint16 shieldtype;
					switch (shapeno) {
						default:
						case 0x52e:
							shieldtype = 1;
							break;
						case 0x52f:
							shieldtype = 2;
							break;
						case 0x530:
							shieldtype = 3;
							break;
					}
					if (_shieldType < shieldtype) {
						_shieldType = shieldtype;
					}
					if (showtoast)
						pickupArea->addPickup(item, false);
					item->destroy();
					return 1;
				} else {
					item->setFrame(0);
					item->setQuality(1);
					item->callUsecodeEvent_combine();
					item->moveToContainer(this);
					if (showtoast)
						pickupArea->addPickup(item, true);
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
						item->setQuality(1); // Count of picked up item is always 1
						if (showtoast)
							pickupArea->addPickup(item, true);
						item->destroy();
						return 1;
					}
				} else {
					uint16 q = existing->getQuality();
					if (q < 10) {
						existing->setQuality(q + 1);
						existing->callUsecodeEvent_combine();
						item->setQuality(1); // Count of picked up item is always 1
						if (showtoast)
							pickupArea->addPickup(item, true);
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

bool MainActor::removeItemCru(Item *item) {
	warning("TODO: Implement MainActor::removeItemCru");
	return false;
}

const ShapeInfo *MainActor::getShapeInfoFromGameInstance() const {
	const ShapeInfo *info = Item::getShapeInfoFromGameInstance();

	if (!(_actorFlags & ACT_KNEELING) || GAME_IS_U8)
		return info;

	// When kneeling in Crusader, return a modified shape with a lower height.
	if (!_kneelingShapeInfo) {
		_kneelingShapeInfo = new ShapeInfo();
		// Not great coupling here, we know most fields don't need filling out..
		_kneelingShapeInfo->_flags = info->_flags;
		_kneelingShapeInfo->_x = info->_x;
		_kneelingShapeInfo->_y = info->_y;
		_kneelingShapeInfo->_weight = info->_weight;
		_kneelingShapeInfo->_volume = info->_volume;
		_kneelingShapeInfo->_family = info->_family;
		_kneelingShapeInfo->_z = info->_z - 4;
	}

	return _kneelingShapeInfo;
}

void MainActor::move(int32 x, int32 y, int32 z) {
	Actor::move(x, y, z);
	if (_shieldSpriteProc != 0) {
		SpriteProcess *sprite = dynamic_cast<SpriteProcess *>(
			Kernel::get_instance()->getProcess(_shieldSpriteProc));
		if (sprite) {
			sprite->move(x, y, z);
		}
	}
}

void MainActor::teleport(int mapNum, int32 x, int32 y, int32 z) {
	World *world = World::get_instance();

	uint16 oldmap = getMapNum();

	// (attempt to) load the new map
	if (!world->switchMap(mapNum)) {
		warning("MainActor::teleport(): switchMap(%d) failed", mapNum);
		return;
	}

	Actor::teleport(mapNum, x, y, z);

	if (GAME_IS_CRUSADER && (x || y) && oldmap == mapNum) {
		// Keep the camera on the avatar (the snap process will update on next move)
		CameraProcess::GetCameraProcess()->moveToLocation(x, y, z);
	}

	_justTeleported = true;
}

// teleport to TeleportEgg
// NB: be careful when calling this from a process, as it might kill
// all running processes
void MainActor::teleport(int mapNum, int teleport_id) {
	int oldmap = getMapNum();
	Point3 old = getLocation();

	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();

	debugC(kDebugActor, "MainActor::teleport(): teleporting to map %d, egg %d", mapNum, teleport_id);

	setMapNum(mapNum);

	// (attempt to) load the new map
	if (!world->switchMap(mapNum)) {
		warning("MainActor::teleport(): switchMap() failed");
		setMapNum(oldmap);
		return;
	}

	// find destination
	TeleportEgg *egg = currentmap->findDestination(teleport_id);
	if (!egg) {
		warning("MainActor::teleport(): destination egg not found");
		teleport(oldmap, old.x, old.y, old.z);
		return;
	}
	Point3 pt = egg->getLocation();

	debugC(kDebugActor, "Found destination: %d, %d, %d", pt.x, pt.y, pt.z);
	debugC(kDebugActor, "%s", egg->dumpInfo().c_str());

	if (GAME_IS_CRUSADER) {
		// Keep the camera on the avatar (the snap process will update on next move)
		// We don't add a new camera process here, as that would update the fast area
		// before the cachein calls above have run.
		CameraProcess::GetCameraProcess()->moveToLocation(pt);
	}

	Actor::teleport(mapNum, pt.x, pt.y, pt.z);

	_justTeleported = true;
}

uint16 MainActor::getDefenseType() const {
	uint16 type = 0;

	for (const auto *i : _contents) {
		uint32 frameNum = i->getFrame();
		const ShapeInfo *si = i->getShapeInfo();
		if (si->_armourInfo) {
			type |= si->_armourInfo[frameNum]._defenseType;
		}
	}

	return type;
}

uint32 MainActor::getArmourClass() const {
	uint32 armour = 0;

	for (const auto *i : _contents) {
		uint32 frameNum = i->getFrame();
		const ShapeInfo *si = i->getShapeInfo();
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
	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
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

		damage = rs.getRandomNumber(getStr() / 2) + kick_bonus;

		return damage;

	}

	Item *weapon = getItem(getEquip(ShapeInfo::SE_WEAPON));


	if (weapon) {
		// weapon equipped?

		const ShapeInfo *si = weapon->getShapeInfo();
		assert(si->_weaponInfo);

		int base = si->_weaponInfo->_baseDamage;
		int mod = si->_weaponInfo->_damageModifier;

		damage = rs.getRandomNumber(mod) + base + getStr() / 5;

		return damage;
	}

	// no weapon?

	damage = rs.getRandomNumber(getStr() / 2) + 1;

	return damage;
}

void MainActor::setInCombat(int activity) {
	setActorFlag(ACT_INCOMBAT);
	if (GAME_IS_U8)
		MusicProcess::get_instance()->playCombatMusic(98); // CONSTANT!
}

void MainActor::clearInCombat() {
	clearActorFlag(ACT_INCOMBAT);
	if (GAME_IS_U8)
		MusicProcess::get_instance()->restoreMusic();
}

ProcId MainActor::die(uint16 damageType, uint16 damagePts, Direction srcDir) {
	ProcId animprocid = Actor::die(damageType, damagePts, srcDir);

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

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();

	_accumStr += n;
	if (_accumStr >= 650 || rs.getRandomNumber(650 - _accumStr) == 0) { //!! constant
		_strength++;
		_accumStr = 0;
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
		debugC(kDebugActor, "Gained _strength!");
	}
}

void MainActor::accumulateDex(int n) {
	// already max?
	if (_dexterity == 25) return; //!! constant

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();

	_accumDex += n;
	if (_accumDex >= 650 || rs.getRandomNumber(650 - _accumDex) == 0) { //!! constant
		_dexterity++;
		_accumDex = 0;
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
		debugC(kDebugActor, "Gained _dexterity!");
	}
}

void MainActor::accumulateInt(int n) {
	// already max?
	if (_intelligence == 25) return; //!! constant

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();

	_accumInt += n;
	if (_accumInt >= 650 || rs.getRandomNumber(650 - _accumInt) == 0) { //!! constant
		_intelligence++;
		_accumInt = 0;
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
		debugC(kDebugActor, "Gained _intelligence!");
	}
}

void MainActor::getWeaponOverlay(const WeaponOverlayFrame *&frame, uint32 &shape) {
	shape = 0;
	frame = 0;

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

	const WeaponInfo *weaponinfo = shapeinfo->_weaponInfo;
	if (!weaponinfo) return;

	shape = weaponinfo->_overlayShape;

	const WpnOvlayDat *wpnovlay = GameData::get_instance()->getWeaponOverlay();
	frame = wpnovlay->getOverlayFrame(action, weaponinfo->_overlayType,
	                                  _direction, _animFrame);

	if (frame == nullptr)
		shape = 0;
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

	// Update combat stance in case we switched big/small weapon.
	if (_lastAnim == Animation::combatStand) {
		if (isBusy()) {
			// Corner case - need to stop active "stand"
			// animation to correct it.
			Kernel::get_instance()->killProcesses(_objId, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE, true);
		}
		doAnim(Animation::combatStand, dir_current);
	}
}

void MainActor::dropWeapon() {
	uint16 oldweapon = _activeWeapon;
	Item *wpn = getItem(oldweapon);
	if (!wpn || (wpn->getShape() == 0x32e && World::get_instance()->getGameDifficulty() < 2))
		return;

	nextWeapon();
	removeItem(wpn);
	wpn->move(_x, _y, _z);
}

void MainActor::nextInvItem() {
	Std::vector<Item *> items;
	getItemsWithShapeFamily(items, ShapeInfo::SF_CRUINVITEM, true);
	getItemsWithShapeFamily(items, ShapeInfo::SF_CRUBOMB, true);
	if (GAME_IS_REMORSE) {
		Item *credits = getFirstItemWithShape(0x4ed, true);
		if (credits)
			items.push_back(credits);
	}
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
	_name.assign(namelength, ' ');
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
	ARG_NULL16(); // TODO: put_in_stasis, 0/1

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
	// Note: only happens in U8, so activity num is not important.
	av->setInCombat(0);

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

uint32 MainActor::I_switchMap(const uint8 *args,
									   unsigned int /*argsize*/) {
	ARG_UINT16(mapnum);
	MainActor *av = getMainActor();
	if (av) {
		av->teleport(mapnum, 0x1e); // CONSTANT
	}
	return 0;
}

uint32 MainActor::I_removeItemCru(const uint8 *args,
								 unsigned int /*argsize*/) {
	MainActor *av = getMainActor();
	ARG_ITEM_FROM_ID(item);

	if (!av || !item)
		return 0;

	if (av->removeItemCru(item))
		return 1;

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
		debugC(kDebugActor, "Can't use item: avatarInStasis");
		return;
	}
	const int32 shapenum = item->getShape();
	if (shapenum == 0x4ed && GAME_IS_CRUSADER) {
		// Do nothing for Credits
		return;
	}
	item->callUsecodeEvent_use();

	// 0x4d4 = datalink, 0x52d = scanner, 0x52e = ionic,
	// 0x52f = plasma, 0x530 = graviton
	// Note: These are the same in Remorse and Regret.
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
	uint16 shieldtype = getShieldType();
	if (shieldtype == 3) {
		shieldtype = 4;
	}

	const FireType *firetype = GameData::get_instance()->getFireType(damage_type);
	int energy = getMana();
	Kernel *kernel = Kernel::get_instance();

	if (shieldtype && firetype && firetype->getShieldCost() && (firetype->getShieldMask() & shieldtype)
			&& firetype->getShieldCost() <= energy) {
		setMana(energy - firetype->getShieldCost());
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
			Point3 pt;

			switch (shieldtype) {
			case 1:
				shieldsprite = 0x5a9;
				shieldstartframe = 7;
				shieldendframe = 0xd;
				remembersprite = false;
				// NOTE: In the game, this is put in the location of the
				// hit.  For now just put in centre.
				pt = getCentre();
				break;
			case 2:
				shieldsprite = 0x5a9;
				shieldstartframe = 0;
				shieldendframe = 6;
				remembersprite = false;
				pt = getCentre();
				break;
			default:
				shieldsprite = 0x52b;
				shieldstartframe = 0;
				shieldendframe = 8;
				pt = getLocation();
				pt.x += 0x10;
				pt.y += 0x18;
				remembersprite = true;
				break;
			}
			Process *p = new SpriteProcess(shieldsprite, shieldstartframe,
										   shieldendframe, 1, 4, pt.x, pt.y, pt.z);
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

void MainActor::detonateBomb() {
	// search area for shape 0x55F (1375) - DETPAC
	UCList uclist(2);
	LOOPSCRIPT(script, LS_SHAPE_EQUAL(0x55F));
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	currentmap->areaSearch(&uclist, script, sizeof(script), nullptr,
							0x800, true, _x, _y);
	for (unsigned int i = 0; i < uclist.getSize(); ++i) {
		Item *founditem = getItem(uclist.getuint16(i));
		if (founditem->hasFlags(FLG_CONTAINED))
			continue;
		founditem->callUsecodeEvent_use();
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
