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
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/anim_dat.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"
#include "ultima/ultima8/world/actors/anim_action.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/world/actors/npc_dat.h"
#include "ultima/ultima8/world/actors/pathfinder.h"
#include "ultima/ultima8/world/actors/resurrection_process.h"
#include "ultima/ultima8/world/actors/clear_feign_death_process.h"
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/world/actors/loiter_process.h"
#include "ultima/ultima8/world/actors/guard_process.h"
#include "ultima/ultima8/world/actors/combat_process.h"
#include "ultima/ultima8/world/actors/attack_process.h"
#include "ultima/ultima8/world/actors/pace_process.h"
#include "ultima/ultima8/world/actors/surrender_process.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/destroy_item_process.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/world/target_reticle_process.h"
#include "ultima/ultima8/world/item_selection_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/fire_type.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/audio/music_process.h"

namespace Ultima {
namespace Ultima8 {

static const unsigned int BACKPACK_SHAPE = 529;

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Actor)

Actor::Actor() : _strength(0), _dexterity(0), _intelligence(0),
		_hitPoints(0), _mana(0), _alignment(0), _enemyAlignment(0),
		_lastAnim(Animation::stand), _animFrame(0), _direction(dir_north),
		_fallStart(0), _unkByte(0), _actorFlags(0), _combatTactic(0),
		_homeX(0), _homeY(0), _homeZ(0), _currentActivityNo(0),
		_lastActivityNo(0), _activeWeapon(0), _lastTimeWasHit(0) {
	_defaultActivity[0] = 0;
	_defaultActivity[1] = 0;
	_defaultActivity[2] = 0;
}

Actor::~Actor() {
}

uint16 Actor::assignObjId() {
	if (_objId == 0xFFFF)
		_objId = ObjectManager::get_instance()->assignActorObjId(this);

	Std::list<Item *>::iterator iter;
	for (iter = _contents.begin(); iter != _contents.end(); ++iter) {
		(*iter)->assignObjId();
		(*iter)->setParent(_objId);
	}

	return _objId;
}

int16 Actor::getMaxMana() const {
	return static_cast<int16>(2 * getInt());
}

uint16 Actor::getMaxHP() const {
	return static_cast<uint16>(2 * getStr());
}

bool Actor::loadMonsterStats() {
	if (GAME_IS_CRUSADER)
		return loadMonsterStatsCru();
	else
		return loadMonsterStatsU8();
}

bool Actor::loadMonsterStatsCru() {
	const NPCDat *npcData = GameData::get_instance()->getNPCDataForShape(getShape());

	if (!npcData)
		return false;

	setStr(npcData->getMaxHp() / 2);
	setHP(npcData->getMaxHp());
	_defaultActivity[0] = npcData->getDefaultActivity(0);
	_defaultActivity[1] = npcData->getDefaultActivity(1);
	_defaultActivity[2] = npcData->getDefaultActivity(2);

	// TODO: Give them the default weapon for their type here.

	return true;
}

bool Actor::loadMonsterStatsU8() {

	const ShapeInfo *shapeinfo = getShapeInfo();
	const MonsterInfo *mi = nullptr;
	if (shapeinfo) mi = shapeinfo->_monsterInfo;
	if (!mi)
		return false;

	uint16 hp;
	if (mi->_maxHp <= mi->_minHp)
		hp = mi->_minHp;
	else
		hp = mi->_minHp + getRandom() % (mi->_maxHp - mi->_minHp);
	setHP(hp);

	uint16 dex;
	if (mi->_maxDex <= mi->_minDex)
		dex = mi->_minDex;
	else
		dex = mi->_minDex + getRandom() % (mi->_maxDex - mi->_minDex);
	setDex(dex);

	uint8 new_alignment = mi->_alignment;
	setAlignment(new_alignment & 0x0F);
	setEnemyAlignment((new_alignment & 0xF0) >> 4); // !! CHECKME

	return true;
}

bool Actor::giveTreasure() {
	MainShapeArchive *mainshapes = GameData::get_instance()->getMainShapes();
	const ShapeInfo *shapeinfo = getShapeInfo();
	const MonsterInfo *mi = nullptr;
	if (shapeinfo) mi = shapeinfo->_monsterInfo;
	if (!mi)
		return false;

	const Std::vector<TreasureInfo> &treasure = mi->_treasure;

	for (unsigned int i = 0; i < treasure.size(); ++i) {
		const TreasureInfo &ti = treasure[i];
		Item *item;

		// check map
		int currentmap = World::get_instance()->getCurrentMap()->getNum();
		if (ti._map != 0 && ((ti._map > 0 && ti._map != currentmap) ||
		                    (ti._map < 0 && -ti._map == currentmap))) {
			continue;
		}

		// check chance
		if (ti._chance < 0.999 &&
		        (static_cast<double>(getRandom()) / RAND_MAX) > ti._chance) {
			continue;
		}

		// determine count/quantity
		int count;
		if (ti._minCount >= ti._maxCount)
			count = ti._minCount;
		else
			count = ti._minCount + (getRandom() % (ti._maxCount - ti._minCount));

		if (!ti._special.empty()) {
			if (ti._special == "weapon") {

				// NB: this is rather biased towards weapons with low _shapes...
				for (unsigned int s = 0; s < mainshapes->getCount(); ++s) {
					const ShapeInfo *si = mainshapes->getShapeInfo(s);
					if (!si->_weaponInfo) continue;

					int chance = si->_weaponInfo->_treasureChance;
					if (!chance) continue;

					int r = getRandom() % 100;
#if 0
					pout << "weapon (" << s << ") chance: " << r << "/"
					     << chance << Std::endl;
#endif
					if (r >= chance) continue;

					// create the weapon
					item = ItemFactory::createItem(s,
					                               0, // frame
					                               count, // quality
					                               Item::FLG_DISPOSABLE,//flags
					                               0, // npcnum,
					                               0, // mapnum
					                               0, true); // ext.flags,_objId
					item->moveToContainer(this);
					item->randomGumpLocation();
					break;
				}
			} else if (ti._special == "sorcfocus") {
				// CONSTANTS! (and lots of them...)
				int shapeNum = 397;
				int frameNum;
				uint16 qualityNum;

				if (getRandom() % 10 < 8) {
					// wand
					if (getRandom() % 10 < 4) {
						// charged
						frameNum = 0;
						qualityNum = 3 + (getRandom() % 4) + // charges
						          ((1 + (getRandom() % 4)) << 8); // spell
					} else {
						frameNum = 15;
						qualityNum = 0;
					}

					item = ItemFactory::createItem(shapeNum, frameNum, qualityNum,
					                               Item::FLG_DISPOSABLE,
					                               0, 0, 0, true);
					item->moveToContainer(this);
					item->randomGumpLocation();
				}

				if (getRandom() % 10 < 6) {
					// rod
					if (getRandom() % 10 < 2) {
						// charged
						frameNum = 3;
						qualityNum = 3 + (getRandom() % 4) + // charges
						          ((1 + (getRandom() % 7)) << 8); // spell
					} else {
						frameNum = 16;
						qualityNum = 0;
					}

					item = ItemFactory::createItem(shapeNum, frameNum, qualityNum,
					                               Item::FLG_DISPOSABLE,
					                               0, 0, 0, true);
					item->moveToContainer(this);
					item->randomGumpLocation();
				}

				if (getRandom() % 10 < 5) {
					// symbol
					if (getRandom() % 10 < 5) {
						// charged
						frameNum = 12;
						uint8 spell = 1 + (getRandom() % 11);
						qualityNum = spell << 8;
						if (spell < 4) {
							qualityNum += 3 + (getRandom() % 4);
						} else {
							// symbol can only have one charge of anything
							// other than ignite/extinguish
							qualityNum += 1;
						}
					} else {
						frameNum = 19;
						qualityNum = 0;
					}

					item = ItemFactory::createItem(shapeNum, frameNum, qualityNum,
					                               Item::FLG_DISPOSABLE,
					                               0, 0, 0, true);
					item->moveToContainer(this);
					item->randomGumpLocation();
				}

				if (getRandom() % 10 < 2) {
					// demon talisman
					if (getRandom() % 10 < 2) {
						// charged
						frameNum = 9;
						qualityNum = 1 + (getRandom() % 2) +  // charges
						          ((10 + (getRandom() % 2)) << 8); // spell
					} else {
						frameNum = 18;
						qualityNum = 0;
					}

					item = ItemFactory::createItem(shapeNum, frameNum, qualityNum,
					                               Item::FLG_DISPOSABLE,
					                               0, 0, 0, true);
					item->moveToContainer(this);
					item->randomGumpLocation();
				}

			} else {
				pout << "Unhandled special treasure: " << ti._special
				     << Std::endl;
			}
			continue;
		}

		// if _shapes.size() == 1 and the given shape is SF_QUANTITY,
		// then produce a stack of that shape (ignoring frame)

		if (ti._shapes.size() == 1) {
			uint32 shapeNum = ti._shapes[0];
			const ShapeInfo *si = mainshapes->getShapeInfo(shapeNum);
			if (!si) {
				perr << "Trying to create treasure with an invalid shapeNum ("
				     << shapeNum << ")" << Std::endl;
				continue;
			}
			if (si->hasQuantity()) {
				// CHECKME: which flags?
				item = ItemFactory::createItem(shapeNum,
				                               0, // frame
				                               count, // quality
				                               Item::FLG_DISPOSABLE, // flags
				                               0, // npcnum,
				                               0, // mapnum
				                               0, true); // ext. flags, _objId
				item->moveToContainer(this);
				item->randomGumpLocation();
				item->callUsecodeEvent_combine(); // this sets the right frame
				continue;
			}
		}

		if (ti._shapes.empty() || ti._frames.empty()) {
			perr << "No shape/frame set in treasure" << Std::endl;
			continue;
		}

		// we need to produce a number of items
		for (int j = 0; (int)j < count; ++j) {
			// pick shape
			int n = getRandom() % ti._shapes.size();
			uint32 shapeNum = ti._shapes[n];

			// pick frame
			n = getRandom() % ti._frames.size();
			uint32 frameNum = ti._frames[n];

			const ShapeInfo *si = GameData::get_instance()->getMainShapes()->
			                getShapeInfo(shapeNum);
			if (!si) {
				perr << "Trying to create treasure with an invalid shapeNum ("
				     << shapeNum << ")" << Std::endl;
				continue;
			}
			uint16 qual = 0;
			if (si->hasQuantity())
				qual = 1;

			// CHECKME: flags?
			item = ItemFactory::createItem(shapeNum,
			                               frameNum, // frameNum
			                               qual, // quality
			                               Item::FLG_DISPOSABLE, // flags
			                               0, // npcnum,
			                               0, // mapnum
			                               0, true); // ext. flags, _objId
			item->moveToContainer(this);
			item->randomGumpLocation();
		}
	}

	return true;
}

bool Actor::removeItem(Item *item) {
	if (!Container::removeItem(item)) return false;

	item->clearFlag(FLG_EQUIPPED); // unequip if necessary

	return true;
}

bool Actor::setEquip(Item *item, bool checkwghtvol) {
	uint32 equiptype = item->getShapeInfo()->_equipType;
	bool backpack = (item->getShape() == BACKPACK_SHAPE);

	// valid item type?
	if (equiptype == ShapeInfo::SE_NONE && !backpack) return false;

	// now check 'equipment slots'
	// we can have one item of each equipment type, plus one backpack
	for (Std::list<Item *>::const_iterator iter = _contents.begin(); iter != _contents.end(); ++iter) {
		if ((*iter)->getObjId() == item->getObjId()) continue;

		uint32 cet = (*iter)->getShapeInfo()->_equipType;
		bool cbackpack = ((*iter)->getShape() == BACKPACK_SHAPE);

		// already have an item with the same equiptype
		if (cet == equiptype || (cbackpack && backpack)) return false;
	}

	if (!item->moveToContainer(this, checkwghtvol)) return false;
	item->clearFlag(FLG_CONTAINED);
	item->setFlag(FLG_EQUIPPED);
	item->setZ(equiptype);

	return true;
}

uint16 Actor::getEquip(uint32 type) const {
	Std::list<Item *>::const_iterator iter;
	for (iter = _contents.begin(); iter != _contents.end(); ++iter) {
		uint32 cet = (*iter)->getShapeInfo()->_equipType;
		bool cbackpack = ((*iter)->getShape() == BACKPACK_SHAPE);

		if ((*iter)->hasFlags(FLG_EQUIPPED) &&
		        (cet == type || (cbackpack && type == 7))) { // !! constant
			return (*iter)->getObjId();
		}
	}

	return 0;
}

void Actor::teleport(int newmap, int32 newx, int32 newy, int32 newz) {
	uint16 newmapnum = static_cast<uint16>(newmap);

	// Set the mapnum
	setMapNum(newmapnum);

	// Put it in the void
	moveToEtherealVoid();

	// Move it to this map
	if (newmapnum == World::get_instance()->getCurrentMap()->getNum()) {
#ifdef DEBUG
		perr << "Actor::teleport: " << getObjId() << " to " << newmap << ","
		     << newx << "," << newy << "," << newz << Std::endl;
#endif
		move(newx, newy, newz);
	}
	// Move it to another map
	else {
		World::get_instance()->etherealRemove(_objId);
		_x = newx;
		_y = newy;
		_z = newz;
	}
}

uint16 Actor::doAnim(Animation::Sequence anim, Direction dir, unsigned int steps) {
	if (dir < 0 || dir > 16) {
		perr << "Actor::doAnim: Invalid _direction (" << dir << ")" << Std::endl;
		return 0;
	}

	if (dir == dir_current)
		dir = getDir();

#if 0
	if (tryAnim(anim, dir)) {
		perr << "Actor::doAnim: tryAnim = Ok!" << Std::endl;
	} else {
		perr << "Actor::doAnim: tryAnim = bad!" << Std::endl;
	}
#endif

	if (GAME_IS_CRUSADER) {
		// Crusader sets some flags on animation start
		// Small hack: When switching from 16-dir to 8-dir, fix the direction
		if (animDirMode(anim) == dirmode_8dirs)
			dir = static_cast<Direction>(dir - (static_cast<uint32>(dir) % 2));
		else if (anim == Animation::readyWeapon)
			setActorFlag(ACT_WEAPONREADY);
		else if (anim == Animation::unreadyWeapon)
			clearActorFlag(ACT_WEAPONREADY);
		else if (anim == Animation::startKneeling || anim == Animation::kneelAndFire ||
				 anim == Animation::kneelAndFireSmallWeapon ||
				 anim == Animation::kneelAndFireLargeWeapon)
			setActorFlag(ACT_KNEELING);
		else if (anim == Animation::stopKneeling)
			clearActorFlag(ACT_KNEELING);
	}

#if 0
	if (_objId == 1) {
		int32 x, y, z;
		getLocation(x, y, z);
		int32 actionno = AnimDat::getActionNumberForSequence(anim, this);
		const AnimAction *action = GameData::get_instance()->getMainShapes()->getAnim(getShape(), actionno);
		debug(6, "Actir::doAnim(%d, %d, %d) from (%d, %d, %d) frame repeat %d", anim, dir, steps, x, y, z, action->getFrameRepeat());
	}
#endif

	Process *p = new ActorAnimProcess(this, anim, dir, steps);

	return Kernel::get_instance()->addProcess(p);
}

bool Actor::hasAnim(Animation::Sequence anim) {
	AnimationTracker tracker;

	return tracker.init(this, anim, dir_north);
}

Animation::Result Actor::tryAnim(Animation::Sequence anim, Direction dir,
                                 unsigned int steps, PathfindingState *state) {
	if (dir < 0 || dir > 16) return Animation::FAILURE;

	if (dir == dir_current)
		dir = getDir();

	AnimationTracker tracker;
	if (!tracker.init(this, anim, dir, state))
		return Animation::FAILURE;

	const AnimAction *animaction = tracker.getAnimAction();

	if (!animaction) return Animation::FAILURE;

	unsigned int curstep = 0;

	while (tracker.step() && (!steps || curstep >= steps)) {
		curstep++;
	}

	if (tracker.isBlocked() &&
	        !animaction->hasFlags(AnimAction::AAF_UNSTOPPABLE)) {
		return Animation::FAILURE;
	}

	if (state) {
		tracker.updateState(*state);
		state->_lastAnim = anim;
		state->_direction = dir;
	}


	if (tracker.isUnsupported()) {
		return Animation::END_OFF_LAND;
	}

	// isUnsupported only checks for AFF_ONGROUND, we need either
	int32 end[3], dims[3];
	getFootpadWorld(dims[0], dims[1], dims[2]);
	tracker.getPosition(end[0], end[1], end[2]);

	CurrentMap *cm = World::get_instance()->getCurrentMap();

	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
	cm->surfaceSearch(&uclist, script, sizeof(script),
	                  getObjId(), end, dims,
	                  false, true, false);
	for (uint32 i = 0; i < uclist.getSize(); i++) {
		Item *item = getItem(uclist.getuint16(i));
		if (item->getShapeInfo()->is_land())
			return Animation::SUCCESS;
	}

	return Animation::END_OFF_LAND;
}

DirectionMode Actor::animDirMode(Animation::Sequence anim) const {
	int32 actionno = AnimDat::getActionNumberForSequence(anim, this);
	const AnimAction *action = GameData::get_instance()->getMainShapes()->getAnim(getShape(), actionno);

	if (!action)
		return dirmode_8dirs;
	return action->getDirCount() == 8 ? dirmode_8dirs : dirmode_16dirs;
}

uint16 Actor::turnTowardDir(Direction targetdir) {
	bool combatRun = hasActorFlags(Actor::ACT_COMBATRUN);
	Direction curdir = getDir();
	bool combat = isInCombat() && !combatRun;
	Animation::Sequence standanim = Animation::stand;
	bool surrendered = hasActorFlags(Actor::ACT_SURRENDERED);

	int stepDelta = Direction_GetShorterTurnDelta(curdir, targetdir);
	Animation::Sequence turnanim = Animation::stand;
	if (GAME_IS_U8) {
		if (stepDelta == -1) {
			turnanim = Animation::lookLeft;
		} else {
			turnanim = Animation::lookRight;
		}
	}

	if (combat) {
		turnanim = Animation::combatStand;
		standanim = Animation::combatStand;
	} else if (surrendered) {
		turnanim = Animation::surrenderStand;
		standanim = Animation::surrenderStand;
	}

	ProcId animpid = 0;
	ProcId prevpid = 0;

	// Create a sequence of turn animations from
	// our current direction to the new one
	DirectionMode mode = animDirMode(turnanim);

	// slight hack - avoid making 8-step turns if we need to swap
	// to/from a 16-step direction - we'll never get to the right
	// direction that way.
	if (static_cast<uint32>(targetdir) % 2 !=
		static_cast<uint32>(curdir) % 2) {
		mode = dirmode_16dirs;
	}

	bool done = false;
	for (Direction dir = curdir; !done; dir = Direction_TurnByDelta(dir, stepDelta, mode)) {
		Animation::Sequence nextanim = turnanim;
		if (dir == targetdir) {
			nextanim = standanim;
			done = true;
		}

		animpid = doAnim(nextanim, dir);
		if (prevpid) {
			Process *proc = Kernel::get_instance()->getProcess(animpid);
			assert(proc);
			proc->waitFor(prevpid);
		}

		prevpid = animpid;
	}

	return animpid;
}

uint16 Actor::setActivity(int activity) {
	if (GAME_IS_CRUSADER)
		return setActivityCru(activity);
	else
		return setActivityU8(activity);
}

uint16 Actor::setActivityU8(int activity) {
	switch (activity) {
	case 0: // loiter
		Kernel::get_instance()->addProcess(new LoiterProcess(this));
		return Kernel::get_instance()->addProcess(new DelayProcess(1));
		break;
	case 1: // combat
		setInCombatU8();
		return 0;
	case 2: // stand
		// NOTE: temporary fall-throughs!
		return doAnim(Animation::stand, dir_current);

	default:
		perr << "Actor::setActivityU8: invalid activity (" << activity << ")"
		     << Std::endl;
	}

	return 0;
}

uint16 Actor::setActivityCru(int activity) {
	if (isDead())
		return 0;

	_lastActivityNo = _currentActivityNo;
	_currentActivityNo = activity;

	switch (activity) {
	case 1: // stand
		return doAnim(Animation::stand, dir_current);
	case 3: // pace
		return Kernel::get_instance()->addProcess(new PaceProcess(this));
	case 2: // loiter
		Kernel::get_instance()->addProcess(new LoiterProcess(this));
		return Kernel::get_instance()->addProcess(new DelayProcess(1));
	case 4:
	case 6:
	    // Does nothing in game..
	    break;
	case 7:
		return Kernel::get_instance()->addProcess(new SurrenderProcess(this));
	    break;
	case 8:
		return Kernel::get_instance()->addProcess(new GuardProcess(this));
	    break;
	case 5:
	case 9:
	case 0xa:
	case 0xb:
	case 0xc:
		// attack
		setInCombatCru(activity);
		return 0;
	case 0xd:
		// Only in No Regret
		perr << "Actor::setActivityCru: TODO: RollingThunderProcess (" << activity << ")";
		return doAnim(Animation::stand, dir_current);
	case 0x70:
		return setActivity(getDefaultActivity(0));
	case 0x71:
		return setActivity(getDefaultActivity(1));
	case 0x72:
		return setActivity(getDefaultActivity(2));
	default:
		perr << "Actor::setActivityCru: invalid activity (" << activity << ")"
		     << Std::endl;
		return doAnim(Animation::stand, dir_current);
	}

	return 0;
}


uint32 Actor::getArmourClass() const {
	const ShapeInfo *si = getShapeInfo();
	if (si->_monsterInfo)
		return si->_monsterInfo->_armourClass;
	else
		return 0;
}

uint16 Actor::getDefenseType() const {
	const ShapeInfo *si = getShapeInfo();
	if (si->_monsterInfo)
		return si->_monsterInfo->_defenseType;
	else
		return 0;
}

int16 Actor::getDefendingDex() const {
	return getDex();
}

int16 Actor::getAttackingDex() const {
	return getDex();
}

uint16 Actor::getDamageType() const {
	const ShapeInfo *si = getShapeInfo();
	if (si->_monsterInfo)
		return si->_monsterInfo->_damageType;
	else
		return WeaponInfo::DMG_NORMAL;
}


int Actor::getDamageAmount() const {
	const ShapeInfo *si = getShapeInfo();
	if (si->_monsterInfo) {

		int min = static_cast<int>(si->_monsterInfo->_minDmg);
		int max = static_cast<int>(si->_monsterInfo->_maxDmg);

		int damage = (getRandom() % (max - min + 1)) + min;

		return damage;
	} else {
		return 1;
	}
}

void Actor::setDefaultActivity(int no, uint16 activity) {
	assert(no >= 0 && no < 3);
	_defaultActivity[no] = activity;
}

uint16 Actor::getDefaultActivity(int no) const {
	assert(no >= 0 && no < 3);
	return _defaultActivity[no];
}

void Actor::setHomePosition(int32 x, int32 y, int32 z) {
	_homeX = x;
	_homeY = y;
	_homeZ = z;
}

void Actor::getHomePosition(int32 &x, int32 &y, int32 &z) const {
	x = _homeX;
	y = _homeY;
	z = _homeZ;
}


void Actor::receiveHit(uint16 other, Direction dir, int damage, uint16 damage_type) {
	if (GAME_IS_U8) {
		receiveHitU8(other, dir, damage, damage_type);
	} else {
		receiveHitCru(other, dir, damage, damage_type);
	}
}

void Actor::receiveHitCru(uint16 other, Direction dir, int damage, uint16 damage_type) {
	//
	// This is a big stack of constants and hard-coded things.
	// It's like that in the original game.
	//
	Actor *attacker = getActor(other);
	AudioProcess *audio = AudioProcess::get_instance();
	Kernel *kernel = Kernel::get_instance();
	uint32 shape = getShape();

	if (shape == 0x3ac && _hitPoints > 0) {
		// TODO: Finish special case for Vargas.  Should not do any damage
		// if there is a particular anim process running.  Also, check if the
		// same special case exists in REGRET.
		doAnim(Animation::teleportOutReplacement, dir_current);
		doAnim(Animation::teleportInReplacement, dir_current);
		_hitPoints -= damage;
		return;
	}

	if (isDead())
		return;

	_lastTimeWasHit = Kernel::get_instance()->getTickNum();

	if (shape != 1 && this != getControlledActor()) {
		Actor *controlled = getControlledActor();
		if (!isInCombat()) {
			setActivity(getDefaultActivity(2)); // get activity from field 0xA
			if (!isInCombat()) {
				setInCombatCru(5);
				CombatProcess *combat = getCombatProcess();
				if (combat && controlled) {
					combat->setTarget(controlled->getObjId());
				}
			}
		} else {
			if (getCurrentActivityNo() == 8) {
				setActivity(5);
			}
			setInCombatCru(5);
			CombatProcess *combat = getCombatProcess();
			if (combat && controlled) {
				combat->setTarget(controlled->getObjId());
			}
		}

		// If the attacker is the controlled npc and this actor is not pathfinding
		if (attacker && attacker == getControlledActor() &&
			kernel->findProcess(_objId, PathfinderProcess::PATHFINDER_PROC_TYPE) != nullptr) {
			int32 x, y, z;
			int32 ox, oy, oz;
			getLocation(x, y, z);
			attacker->getLocation(ox, oy, oz);
			int32 maxdiff = MAX(MAX(abs(x - ox), abs(y - oy)), abs(z - oz));
			if (maxdiff < 641 && isOnScreen()) {
				// TODO: implement the equivalent of this function.  For now, we always
				// cancel pathfinding for the NPC.
				// uint32 direction = static_cast<uint32>(Get_WorldDirection(y - oy, x - ox));
				// int result = FUN_1128_1755(this, attacker, direction, 0, 0, 0);
				// if (result) {
					kernel->killProcesses(_objId, PathfinderProcess::PATHFINDER_PROC_TYPE, true);
				// }
			}
		}
	} else {
		damage = receiveShieldHit(damage, damage_type);
	}

	if (hasActorFlags(ACT_IMMORTAL))
		damage = 0;

	if (damage > _hitPoints)
		damage = _hitPoints;

	setHP(static_cast<uint16>(_hitPoints - damage));

	if (_hitPoints == 0) {
		// Die!
		die(damage_type);
	} else if (damage) {
		// Not dead yet.
		if (!isRobotCru()) {
			uint16 sfxno;
			if (hasExtFlags(EXT_FEMALE)) {
				sfxno = 0xd8; // female scream
			} else {
				sfxno = 0x8f; // male scream
			}
			if (audio && !audio->isSFXPlayingForObject(sfxno, other)) {
				audio->playSFX(sfxno, 0x10, other, 1, false);
			}
		}
		if (damage_type == 0xf || damage_type == 7) {
			if (shape == 1) {
				kernel->killProcesses(_objId, 0x204, true);
				doAnim(static_cast<Animation::Sequence>(0x37), dir_current);
			} else if (shape == 0x4e6 || shape == 0x338 || shape == 0x385 || shape == 899) {
				if (!(getRandom() % 3)) {
					// Randomly stun the NPC for these damage types.
					// CHECK ME: is this time accurate?
					Process *attack = kernel->findProcess(_objId, 0x259);
					uint stun = ((getRandom() % 10) + 8) * 60;
					if (attack && stun) {
						Process *delay = new DelayProcess(stun);
						kernel->addProcess(delay);
						attack->waitFor(delay);
					}
				}
			}
		}
	}
}

void Actor::tookHitCru() {
	Animation::Sequence lastanim = getLastAnim();
	if (lastanim == Animation::unknownAnim30 || lastanim == Animation::startRunWithLargeWeapon) {
		//uint16 controllednpc = World::get_instance()->getControlledNPCNum();
		bool canseecontrolled = true; //this->canSee(controllednpc);
		if (canseecontrolled) {
			if (getRandom() % 4)
				setActivity(5);
			else
				setActivity(10);
		}
	} else {
		uint32 shape = getShape();
		if (shape != 0x576) { // 0x576 = flaming guy
			if (shape < 0x577) {
				if (shape == 0x385 || shape == 0x4e6) {
				   explode(2, 0);
				   clearFlag(FLG_IN_NPC_LIST);
				   clearFlag(FLG_GUMP_OPEN);
			   }
			   return;
		   }
		   if (shape != 0x596) {
			   return;
		   }
		}

		bool violence = true; // Game::I_isViolenceEnabled
		if (!violence)
			return;

		static const uint16 FEMALE_SCREAMS[] = {0xb, 0xa};
		static const uint16 MALE_SCREAMS[] = {0x65, 0x66, 0x67};
		int nsounds;
		const uint16 *sounds;
		if (hasExtFlags(EXT_FEMALE)) {
			nsounds = ARRAYSIZE(FEMALE_SCREAMS);
			sounds = FEMALE_SCREAMS;
		} else {
			nsounds = ARRAYSIZE(MALE_SCREAMS);
			sounds = MALE_SCREAMS;
		}
		AudioProcess *audio = AudioProcess::get_instance();
		if (!audio)
			return;
		for (int i = 0; i < nsounds; i++) {
			if (audio->isSFXPlayingForObject(sounds[i], _objId))
				return;
		}
		audio->playSFX(sounds[getRandom() % nsounds], 0x80, _objId, 1);
	}
}

void Actor::receiveHitU8(uint16 other, Direction dir, int damage, uint16 damage_type) {
	if (isDead())
		return; // already dead, so don't bother

	Item *hitter = getItem(other);
	Actor *attacker = getActor(other);

	if (damage == 0 && attacker) {
		damage = attacker->getDamageAmount();
	}

	if (damage_type == 0 && hitter) {
		damage_type = hitter->getDamageType();
	}

	if (other == 1 && attacker && attacker->getLastAnim() != Animation::kick) {
		// _strength for kicks is accumulated in AvatarMoverProcess
		MainActor *av = getMainActor();
		av->accumulateStr(damage / 4);
	}

	pout << "Actor " << getObjId() << " received hit from " << other
	     << " (dmg=" << damage << ",type=" << Std::hex << damage_type
	     << Std::dec << "). ";

	damage = calculateAttackDamage(other, damage, damage_type);

	if (!damage) {
		pout << "No damage." << Std::endl;
	} else {
		pout << "Damage: " << damage << Std::endl;
	}

	if (damage >= 4 && _objId == 1 && attacker) {
		// play blood sprite
		int start = 0, end = 12;
		if (dir > dir_east) {
			start = 13;
			end = 25;
		}

		int32 xv, yv, zv;
		getLocation(xv, yv, zv);
		zv += (getRandom() % 24);
		Process *sp = new SpriteProcess(620, start, end, 1, 1, xv, yv, zv);
		Kernel::get_instance()->addProcess(sp);
	}

	if (damage > 0 && !hasActorFlags(ACT_IMMORTAL | ACT_INVINCIBLE)) {
		if (damage >= _hitPoints) {
			// we're dead

			if (hasActorFlags(ACT_WITHSTANDDEATH)) {
				// or maybe not...

				setHP(getMaxHP());
				AudioProcess *audioproc = AudioProcess::get_instance();
				if (audioproc) audioproc->playSFX(59, 0x60, _objId, 0);
				clearActorFlag(ACT_WITHSTANDDEATH);
			} else {
				die(damage_type);
			}
			return;
		}

		// not dead yet
		setHP(static_cast<uint16>(_hitPoints - damage));
	}

	ProcId fallingprocid = 0;
	if (_objId == 1 && damage > 0) {
		if ((damage_type & WeaponInfo::DMG_FALLING) && damage >= 6) {
			// high falling damage knocks you down
			doAnim(Animation::fallBackwards, dir_current);

			// TODO: shake head after getting back up when not in combat
			return;
		}

		// got hit, so abort current animation
		fallingprocid = killAllButFallAnims(false);
	}

	// if avatar was blocking; do a quick stopBlock/startBlock and play SFX
	if (_objId == 1 && getLastAnim() == Animation::startBlock) {
		ProcId anim1pid = doAnim(Animation::stopBlock, dir_current);
		ProcId anim2pid = doAnim(Animation::startBlock, dir_current);

		Process *anim1proc = Kernel::get_instance()->getProcess(anim1pid);
		Process *anim2proc = Kernel::get_instance()->getProcess(anim2pid);
		assert(anim1proc);
		assert(anim2proc);
		anim2proc->waitFor(anim1proc);

		int sfx;
		if (damage)
			sfx = 50 + (getRandom() % 2); // constants!
		else
			sfx = 20 + (getRandom() % 3); // constants!
		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc) audioproc->playSFX(sfx, 0x60, _objId, 0);
		return;
	}

	// TODO: target needs to stumble/fall/call for help/...(?)

	if (_objId != 1) {
		ObjId target = 1;
		if (attacker)
			target = attacker->getObjId();
		if (!isInCombat())
			setInCombatU8();

		CombatProcess *cp = getCombatProcess();
		assert(cp);
		cp->setTarget(target);

		if (target == 1) {
			// call for help
		}
	}

	// FIXME: What are the equivalent Crusader animations here?
	if (damage && !fallingprocid && GAME_IS_U8) {
		ProcId anim1pid = doAnim(Animation::stumbleBackwards, dir);
		ProcId anim2pid;
		if (isInCombat())
			// not doing this would cause you to re-draw your weapon when hit
			anim2pid = doAnim(Animation::combatStand, dir);
		else
			anim2pid = doAnim(Animation::stand, dir);
		Process *anim1proc = Kernel::get_instance()->getProcess(anim1pid);
		Process *anim2proc = Kernel::get_instance()->getProcess(anim2pid);
		assert(anim1proc);
		assert(anim2proc);
		anim2proc->waitFor(anim1proc);
	}
}

ProcId Actor::die(uint16 damageType) {
	setHP(0);
	setActorFlag(ACT_DEAD);
	setFlag(FLG_BROKEN);
	clearActorFlag(ACT_INCOMBAT);

	ProcId animprocid = 0;
#if 1
	animprocid = killAllButFallAnims(true);
#else
	Kernel::get_instance()->killProcesses(getObjId(), 6, true); // CONSTANT!
#endif

	// TODO: In Crusader, this should default to 0x12, but randomly choose anim 0x14
	// if it's available.
	if (!animprocid)
		animprocid = doAnim(Animation::die, dir_current);

	MainActor *avatar = getMainActor();
	// if hostile to avatar
	if (GAME_IS_U8 && (getEnemyAlignment() & avatar->getAlignment())) {
		if (avatar->isInCombat()) {
			// play victory fanfare
			MusicProcess::get_instance()->playCombatMusic(109);
			// and resume combat music afterwards
			MusicProcess::get_instance()->queueMusic(98);
		}
	} else if (GAME_IS_CRUSADER) {
		if (!isRobotCru()) {
			uint16 sfxno;
			static const uint16 FADING_SCREAM_SFX[] = { 0xD9, 0xDA };
			static const uint16 MALE_DEATH_SFX[] = { 0x88, 0x8C, 0x8F };
			static const uint16 FEMALE_DEATH_SFX[] = { 0xD8, 0x10 };
			if (damageType == 0xf) {
				sfxno = FADING_SCREAM_SFX[getRandom() % 2];
			} else {
				if (hasExtFlags(EXT_FEMALE)) {
					sfxno = FEMALE_DEATH_SFX[getRandom() % 2];
				} else {
					sfxno = MALE_DEATH_SFX[getRandom() % 3];
				}
			}
			AudioProcess::get_instance()->playSFX(sfxno, 0x10, _objId, 0, true);
		}
	}

	destroyContents();
	giveTreasure();

	const ShapeInfo *shapeinfo = getShapeInfo();
	const MonsterInfo *mi = nullptr;
	if (shapeinfo) mi = shapeinfo->_monsterInfo;

	if (mi && mi->_resurrection && !(damageType & WeaponInfo::DMG_FIRE)) {
		// this monster will be resurrected after a while

		pout << "Actor::die: scheduling resurrection" << Std::endl;

		int timeout = ((getRandom() % 25) + 5) * 30; // 5-30 seconds

		Process *resproc = new ResurrectionProcess(this);
		Kernel::get_instance()->addProcess(resproc);

		Process *delayproc = new DelayProcess(timeout);
		Kernel::get_instance()->addProcess(delayproc);

		ProcId animpid = doAnim(Animation::standUp, dir_current);
		Process *animproc = Kernel::get_instance()->getProcess(animpid);
		assert(animproc);

		resproc->waitFor(delayproc);
		animproc->waitFor(resproc);
	}

	if (mi && mi->_explode) {
		// this monster explodes when it dies

		pout << "Actor::die: exploding" << Std::endl;

		int count = 5;
		Shape *explosionshape = GameData::get_instance()->getMainShapes()
		                        ->getShape(mi->_explode);
		assert(explosionshape);
		unsigned int framecount = explosionshape->frameCount();

		for (int i = 0; i < count; ++i) {
			Item *piece = ItemFactory::createItem(mi->_explode,
			                                      getRandom() % framecount,
			                                      0, // qual
			                                      Item::FLG_FAST_ONLY, //flags,
			                                      0, // npcnum
			                                      0, // mapnum
			                                      0, true // ext. flags, _objId
			                                     );
			piece->move(_x - 128 + 32 * (getRandom() % 6),
			            _y - 128 + 32 * (getRandom() % 6),
			            _z + getRandom() % 8); // move to near actor's position
			piece->hurl(-25 + (getRandom() % 50),
			            -25 + (getRandom() % 50),
			            10 + (getRandom() % 10),
			            4); // (wrong?) CONSTANTS!
		}
	}

	return animprocid;
}

void Actor::killAllButCombatProcesses() {
	// loop over all processes, keeping only the relevant ones
	ProcessIter iter = Kernel::get_instance()->getProcessBeginIterator();
	ProcessIter endproc = Kernel::get_instance()->getProcessEndIterator();
	for (; iter != endproc; ++iter) {
		Process *p = *iter;
		if (!p) continue;
		if (p->getItemNum() != _objId) continue;
		if (p->is_terminated()) continue;

		uint16 type = p->getType();

		if (type != 0xF0 && type != 0xF2 && type != 0x208 && type != 0x21D &&
		        type != 0x220 && type != 0x238 && type != 0x243) {
			p->fail();
		}
	}
}

ProcId Actor::killAllButFallAnims(bool death) {
	ProcId fallproc = 0;

	Kernel *kernel = Kernel::get_instance();

	if (death) {
		// if dead, we want to kill everything but animations
		kernel->killProcessesNotOfType(_objId, 0xF0, true);
	} else {
		// otherwise, need to focus on combat, so kill everything else
		killAllButCombatProcesses();
	}

	// loop over all animation processes, keeping only the relevant ones
	ProcessIter iter = Kernel::get_instance()->getProcessBeginIterator();
	ProcessIter endproc = Kernel::get_instance()->getProcessEndIterator();
	for (; iter != endproc; ++iter) {
		ActorAnimProcess *p = dynamic_cast<ActorAnimProcess *>(*iter);
		if (!p) continue;
		if (p->getItemNum() != _objId) continue;
		if (p->is_terminated()) continue;

		Animation::Sequence action = p->getAction();

		if (action == Animation::die) {
			fallproc = p->getPid();
			continue;
		}

		if (!death && action == Animation::standUp) {
			fallproc = p->getPid();
		} else {
			p->fail();
		}
	}

	return fallproc;
}

int Actor::calculateAttackDamage(uint16 other, int damage, uint16 damage_type) {
	Actor *attacker = getActor(other);

	uint16 defense_type = getDefenseType();

	// most damage types are blocked straight away by defense types
	damage_type &= ~(defense_type & ~(WeaponInfo::DMG_MAGIC  |
	                                  WeaponInfo::DMG_UNDEAD |
	                                  WeaponInfo::DMG_PIERCE));

	// immunity to non-magical weapons
	if ((defense_type & WeaponInfo::DMG_MAGIC) &&
	        !(damage_type & WeaponInfo::DMG_MAGIC)) {
		damage = 0;
	}

	bool slayer = false;

	// _special attacks
	if (damage && damage_type) {
		if (damage_type & WeaponInfo::DMG_SLAYER) {
			if (getRandom() % 10 == 0) {
				slayer = true;
				damage = 255; // instant kill
			}
		}

		if ((damage_type & WeaponInfo::DMG_UNDEAD) &&
		        (defense_type & WeaponInfo::DMG_UNDEAD)) {
			damage *= 2; // double damage against undead
		}

		if ((defense_type & WeaponInfo::DMG_PIERCE) &&
		        !(damage_type & (WeaponInfo::DMG_BLADE |
		                         WeaponInfo::DMG_FIRE  |
		                         WeaponInfo::DMG_PIERCE))) {
			damage /= 2; // resistance to blunt damage
		}
	} else {
		damage = 0;
	}

	// armour
	if (damage && !(damage_type & WeaponInfo::DMG_PIERCE) && !slayer) {
		// blocking?
		if ((getLastAnim() == Animation::startBlock ||
		        getLastAnim() == Animation::stopBlock) &&
		        !hasActorFlags(ACT_STUNNED)) {
			damage -= getStr() / 5;
		}

		int ACmod = 3 * getArmourClass();
		if (damage_type & WeaponInfo::DMG_FIRE)
			ACmod /= 2; // armour doesn't protect from fire as well

		if (hasActorFlags(ACT_STUNNED))
			ACmod /= 2; // stunned?

		if (ACmod > 100) ACmod = 100;

		// TODO: replace rounding bias by something random
		damage = ((100 - ACmod) * damage) / 100;

		if (damage < 0) damage = 0;
	}

	// to-hit
	if (damage && !(damage_type & WeaponInfo::DMG_PIERCE) && attacker) {
		bool hit = false;
		int16 attackdex = attacker->getAttackingDex();
		int16 defenddex = getDefendingDex();
		if (attackdex < 0) attackdex = 0;
		if (defenddex <= 0) defenddex = 1;

		if (hasActorFlags(ACT_STUNNED) ||
		        (getRandom() % (attackdex + 3) > getRandom() % defenddex)) {
			hit = true;
		}

		// TODO: give avatar an extra chance to hit monsters
		//       with defense_type DMG_PIERCE

		if (hit && other == 1) {
			MainActor *av = getMainActor();
			if (attackdex > defenddex)
				av->accumulateDex(2 * (attackdex - defenddex));
			else
				av->accumulateDex(2);
		}

		if (!hit) {
			damage = 0;
		}
	}

	return damage;
}

CombatProcess *Actor::getCombatProcess() {
	Process *p = Kernel::get_instance()->findProcess(_objId, 0xF2); // CONSTANT!
	if (!p)
		return nullptr;
	CombatProcess *cp = dynamic_cast<CombatProcess *>(p);
	assert(cp);

	return cp;
}

AttackProcess *Actor::getAttackProcess() {
	Process *p = Kernel::get_instance()->findProcess(_objId, 0x259); // CONSTANT!
	if (!p)
		return nullptr;
	AttackProcess *ap = dynamic_cast<AttackProcess *>(p);
	assert(ap);

	return ap;
}

void Actor::setInCombat(int activity) {
	if (GAME_IS_U8)
		setInCombatU8();
	else
		setInCombatCru(activity);
}

void Actor::setInCombatU8() {
	if ((_actorFlags & ACT_INCOMBAT) != 0) return;

	assert(getCombatProcess() == nullptr);

	// kill any processes belonging to this actor
	Kernel::get_instance()->killProcesses(getObjId(), 6, true);

	// perform _special actions
	ProcId castproc = callUsecodeEvent_cast(0);

	CombatProcess *cp = new CombatProcess(this);
	Kernel::get_instance()->addProcess(cp);

	// wait for any _special actions to finish before starting to fight
	if (castproc)
		cp->waitFor(castproc);

	setActorFlag(ACT_INCOMBAT);
}

void Actor::setInCombatCru(int activity) {
	if ((_actorFlags & ACT_INCOMBAT) != 0) return;

	assert(getAttackProcess() == nullptr);

	setActorFlag(ACT_INCOMBAT);

	AttackProcess *ap = new AttackProcess(this);
	Kernel::get_instance()->addProcess(ap);

	if (getCurrentActivityNo() == 8) {
		// Guard process.. set some flag in ap
		ap->setField97();
	}
	if (activity == 0xc) {
		ap->setTimer3();
		// This sets fields 0x77 and 0x79 of the attack process
		// to some random timer value in the future
		//ap->AttackProcess_1108_1485();
	}

	uint16 animproc = 0;
	if (activity == 9 || activity == 0xb) {
		ap->setIsActivity9OrB();
		animproc = doAnim(Animation::readyWeapon, dir_current);
	} else {
		animproc = doAnim(Animation::stand, dir_current);
	}
	if (animproc) {
		// Do the animation first
		ap->waitFor(animproc);
	}

	if (activity == 0xa || activity == 0xb) {
		ap->setIsActivityAOrB();
	}
}

void Actor::clearInCombat() {
	if ((_actorFlags & ACT_INCOMBAT) == 0) return;

	Process *p;
	if (GAME_IS_U8) {
		p = getCombatProcess();
	} else {
		p = getAttackProcess();
	}
	if (p)
		p->terminate();

	clearActorFlag(ACT_INCOMBAT);
}

int32 Actor::collideMove(int32 x, int32 y, int32 z, bool teleports, bool force,
						 ObjId *hititem, uint8 *dirs) {
	int32 result = Item::collideMove(x, y, z, teleports, force, hititem, dirs);
	if (this == getControlledActor() && GAME_IS_CRUSADER) {
		TargetReticleProcess::get_instance()->avatarMoved();
		ItemSelectionProcess::get_instance()->avatarMoved();
	}
	return result;
}

bool Actor::activeWeaponIsSmall() const {
	const Item *wpn = getItem(_activeWeapon);
	if (wpn) {
		const WeaponInfo *wi = wpn->getShapeInfo()->_weaponInfo;
		return wi && (wi->_small != 0);
	}
	return false;
}


bool Actor::areEnemiesNear() {
	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	currentmap->areaSearch(&uclist, script, sizeof(script), this, 0x800, false);

	for (unsigned int i = 0; i < uclist.getSize(); ++i) {
		Actor *npc = getActor(uclist.getuint16(i));
		if (!npc) continue;
		if (npc == this) continue;

		if (npc->hasActorFlags(ACT_DEAD | ACT_FEIGNDEATH)) continue;
		if (!npc->hasActorFlags(ACT_INCOMBAT)) continue;

		// TODO: check if hostile.
		// Might not be strictly necessary, though. This function is only
		// used on the avatar, and any NPCs in combat mode around the avatar
		// are most likely hostile... (and if they're not hostile, they're
		// probably in combat mode because something hostile _is_ nearby)

		return true;
	}

	return false;
}

uint16 Actor::schedule(uint32 time) {
	if (isDead())
		return 0;

	uint32 ret = callUsecodeEvent_schedule(time);

	return static_cast<uint16>(ret);
}

//static
Actor *Actor::createActor(uint32 shape, uint32 frame) {
	Actor *newactor = ItemFactory::createActor(shape, frame, 0,
	                  Item::FLG_IN_NPC_LIST,
	                  0, 0, 0, true);
	if (!newactor)
		return nullptr;
	uint16 objID = newactor->getObjId();

	// set stats
	if (!newactor->loadMonsterStats()) {
		perr << "I_createActor failed to set stats for actor (" << shape
		     << ")." << Std::endl;
	}

	Actor *av = getMainActor();
	newactor->setMapNum(av->getMapNum());
	newactor->setNpcNum(objID);
	newactor->setFlag(FLG_ETHEREAL);
	World::get_instance()->etherealPush(objID);

	return newactor;
}

void Actor::dumpInfo() const {
	Container::dumpInfo();

	pout << "hp: " << _hitPoints << ", mp: " << _mana << ", str: " << _strength
	     << ", dex: " << _dexterity << ", int: " << _intelligence
	     << ", ac: " << getArmourClass() << ", defense: " << Std::hex
	     << getDefenseType() << " align: " << getAlignment() << " enemy: "
	     << getEnemyAlignment() << ", flags: " << _actorFlags
	     << Std::dec << Std::endl;
}

void Actor::saveData(Common::WriteStream *ws) {
	Container::saveData(ws);
	ws->writeUint16LE(_strength);
	ws->writeUint16LE(_dexterity);
	ws->writeUint16LE(_intelligence);
	ws->writeUint16LE(_hitPoints);
	ws->writeUint16LE(_mana);
	ws->writeUint16LE(_alignment);
	ws->writeUint16LE(_enemyAlignment);
	ws->writeUint16LE(_lastAnim);
	ws->writeUint16LE(_animFrame);
	ws->writeUint16LE(Direction_ToUsecodeDir(_direction));
	ws->writeUint32LE(_fallStart);
	ws->writeUint32LE(_actorFlags);
	ws->writeByte(_unkByte);

	if (GAME_IS_CRUSADER) {
		ws->writeUint16LE(_defaultActivity[0]);
		ws->writeUint16LE(_defaultActivity[1]);
		ws->writeUint16LE(_defaultActivity[2]);
		ws->writeUint16LE(_combatTactic);
		ws->writeUint32LE(_homeX);
		ws->writeUint32LE(_homeY);
		ws->writeUint32LE(_homeZ);
		ws->writeUint16LE(_currentActivityNo);
		ws->writeUint16LE(_lastActivityNo);
		ws->writeUint16LE(_activeWeapon);
		ws->writeSint32LE(_lastTimeWasHit);
	}
}

bool Actor::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Container::loadData(rs, version)) return false;

	_strength = static_cast<int16>(rs->readUint16LE());
	_dexterity = static_cast<int16>(rs->readUint16LE());
	_intelligence = static_cast<int16>(rs->readUint16LE());
	_hitPoints = rs->readUint16LE();
	_mana = static_cast<int16>(rs->readUint16LE());
	_alignment = rs->readUint16LE();
	_enemyAlignment = rs->readUint16LE();
	_lastAnim = static_cast<Animation::Sequence>(rs->readUint16LE());
	_animFrame = rs->readUint16LE();
	_direction = Direction_FromUsecodeDir(rs->readUint16LE());
	_fallStart = rs->readUint32LE();
	_actorFlags = rs->readUint32LE();
	_unkByte = rs->readByte();

	if (GAME_IS_CRUSADER) {
		_defaultActivity[0] = rs->readUint16LE();
		_defaultActivity[1] = rs->readUint16LE();
		_defaultActivity[2] = rs->readUint16LE();
		_combatTactic = rs->readUint16LE();
		_homeX = rs->readUint32LE();
		_homeY = rs->readUint32LE();
		_homeZ = rs->readUint32LE();
		_currentActivityNo = rs->readUint16LE();
		_lastActivityNo = rs->readUint16LE();
		_activeWeapon = rs->readUint16LE();
		_lastTimeWasHit = rs->readSint32LE();
	}

	return true;
}


uint32 Actor::I_isNPC(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;
	return 1;
}

uint32 Actor::I_getMap(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getMapNum();
}

uint32 Actor::I_teleport(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(newx);
	ARG_UINT16(newy);
	ARG_UINT16(newz);
	ARG_UINT16(newmap);
	if (!actor) return 0;

	if (GAME_IS_CRUSADER) {
		newx *= 2;
		newy *= 2;
	}

	actor->teleport(newmap, newx, newy, newz);
	return 0;
}

uint32 Actor::I_doAnim(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(anim);
	ARG_UINT16(dir); // seems to be 0-8
	ARG_UINT16(unk1); // this is almost always 10000 in U8.Maybe speed-related?
	ARG_UINT16(unk2); // appears to be 0 or 1. Some flag?

	if (!actor) return 0;

	//
	// HACK: In Crusader, we do translation on the animations so we want to remap
	// most of them, but for direct commands from the usecode we add a bitflag for
	// no remapping
	//
	if (GAME_IS_CRUSADER) {
		anim |= Animation::crusaderAbsoluteAnimFlag;
	}

	return actor->doAnim(static_cast<Animation::Sequence>(anim), Direction_FromUsecodeDir(dir));
}

uint32 Actor::I_getDir(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return Direction_ToUsecodeDir(actor->getDir());
}

uint32 Actor::I_getLastAnimSet(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getLastAnim();
}

uint32 Actor::I_getStr(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getStr();
}

uint32 Actor::I_getDex(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getDex();
}

uint32 Actor::I_getInt(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getInt();
}

uint32 Actor::I_getHp(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getHP();
}

uint32 Actor::I_getMana(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getMana();
}

uint32 Actor::I_getAlignment(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getAlignment();
}

uint32 Actor::I_getEnemyAlignment(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getEnemyAlignment();
}

uint32 Actor::I_setStr(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_SINT16(str);
	if (!actor) return 0;

	actor->setStr(str);
	return 0;
}

uint32 Actor::I_setDex(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_SINT16(dex);
	if (!actor) return 0;

	actor->setDex(dex);
	return 0;
}

uint32 Actor::I_setInt(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_SINT16(int_);
	if (!actor) return 0;

	actor->setStr(int_);
	return 0;
}

uint32 Actor::I_setHp(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(hp);
	if (!actor) return 0;

	actor->setHP(hp);
	return 0;
}

uint32 Actor::I_addHp(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(hp);

	if (actor) {
		int max = actor->getMaxHP();
		int cur = actor->getHP();
		if (cur < max) {
			actor->setHP(MIN(max, cur + hp));
			return 1;
		}
	}
	return 0;
}

uint32 Actor::I_setMana(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_SINT16(mp);
	if (!actor) return 0;

	actor->setMana(mp);
	return 0;
}

uint32 Actor::I_setAlignment(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(a);
	if (!actor) return 0;

	actor->setAlignment(a);
	return 0;
}

uint32 Actor::I_setEnemyAlignment(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(a);
	if (!actor) return 0;

	actor->setEnemyAlignment(a);
	return 0;
}

uint32 Actor::I_isInCombat(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->isInCombat())
		return 1;
	else
		return 0;
}

uint32 Actor::I_setInCombat(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	assert(GAME_IS_U8);
	actor->setInCombatU8();

	return 0;
}

uint32 Actor::I_clrInCombat(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->clearInCombat();

	return 0;
}

uint32 Actor::I_setTarget(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(target);
	if (!actor) return 0;

	assert(GAME_IS_U8);

	CombatProcess *cp = actor->getCombatProcess();
	if (!cp) {
		actor->setInCombatU8();
		cp = actor->getCombatProcess();
	}
	if (!cp) {
		perr << "Actor::I_setTarget: failed to enter combat mode"
		     << Std::endl;
		return 0;
	}

	cp->setTarget(target);

	return 0;
}

uint32 Actor::I_getTarget(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	CombatProcess *cp = actor->getCombatProcess();

	if (!cp) return 0;

	return static_cast<uint32>(cp->getTarget());
}


uint32 Actor::I_isEnemy(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_ACTOR_FROM_ID(other);
	if (!actor) return 0;
	if (!other) return 0;

	if (actor->getEnemyAlignment() & other->getAlignment())
		return 1;
	else
		return 0;
}

uint32 Actor::I_isDead(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->isDead())
		return 1;
	else
		return 0;
}

uint32 Actor::I_setDead(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->setActorFlag(ACT_DEAD);
	if (GAME_IS_CRUSADER) {
		actor->setFlag(FLG_BROKEN);
		World::get_instance()->getCurrentMap()->removeTargetItem(actor);
	}

	return 0;
}

uint32 Actor::I_clrDead(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->clearActorFlag(ACT_DEAD);
	if (GAME_IS_CRUSADER) {
		actor->clearFlag(FLG_BROKEN);
		World::get_instance()->getCurrentMap()->addTargetItem(actor);
	}

	return 0;
}

uint32 Actor::I_isImmortal(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->hasActorFlags(ACT_IMMORTAL))
		return 1;
	else
		return 0;
}

uint32 Actor::I_setImmortal(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->setActorFlag(ACT_IMMORTAL);
	actor->clearActorFlag(ACT_INVINCIBLE);

	return 0;
}

uint32 Actor::I_clrImmortal(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->clearActorFlag(ACT_IMMORTAL);

	return 0;
}

uint32 Actor::I_isWithstandDeath(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->hasActorFlags(ACT_WITHSTANDDEATH))
		return 1;
	else
		return 0;
}

uint32 Actor::I_setWithstandDeath(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->setActorFlag(ACT_WITHSTANDDEATH);

	return 0;
}

uint32 Actor::I_clrWithstandDeath(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->clearActorFlag(ACT_WITHSTANDDEATH);

	return 0;
}

uint32 Actor::I_isFeignDeath(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->hasActorFlags(ACT_FEIGNDEATH))
		return 1;
	else
		return 0;
}

uint32 Actor::I_setFeignDeath(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->hasActorFlags(ACT_FEIGNDEATH))
		return 0;

	actor->setActorFlag(ACT_FEIGNDEATH);

	ProcId animfallpid = actor->doAnim(Animation::die, dir_current);
	Process *animfallproc = Kernel::get_instance()->getProcess(animfallpid);
	assert(animfallproc);

	ProcId animstandpid = actor->doAnim(Animation::standUp, dir_current);
	Process *animstandproc = Kernel::get_instance()->getProcess(animstandpid);
	assert(animstandproc);

	Process *delayproc = new DelayProcess(900); // 30 seconds
	Kernel::get_instance()->addProcess(delayproc);

	Process *clearproc = new ClearFeignDeathProcess(actor);
	Kernel::get_instance()->addProcess(clearproc);

	// do them in order (fall, stand, wait, clear)

	clearproc->waitFor(delayproc);
	delayproc->waitFor(animstandproc);
	animstandproc->waitFor(animfallproc);

	return 0;
}

uint32 Actor::I_clrFeignDeath(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->clearActorFlag(ACT_FEIGNDEATH);

	return 0;
}

uint32 Actor::I_pathfindToItem(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_OBJID(id2);
	Item *item = getItem(id2);
	if (!actor) return 0;
	if (!item) return 0;

	return Kernel::get_instance()->addProcess(
	           new PathfinderProcess(actor, id2));
}

uint32 Actor::I_pathfindToPoint(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);
	ARG_NULL16(); // unknown. Only one instance of this in U8, value is 5.
	if (!actor) return 0;

	if (GAME_IS_CRUSADER) {
		x *= 2;
		y *= 2;
	}

	return Kernel::get_instance()->addProcess(
	           new PathfinderProcess(actor, x, y, z));
}

uint32 Actor::I_areEnemiesNear(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->areEnemiesNear())
		return 1;
	else
		return 0;
}

uint32 Actor::I_isBusy(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UC_PTR(ptr);
	uint16 id = UCMachine::ptrToObject(ptr);

	uint32 count = Kernel::get_instance()->getNumProcesses(id, ActorAnimProcess::ACTOR_ANIM_PROC_TYPE);
	if (count > 0)
		return 1;
	else
		return 0;
}

uint32 Actor::I_createActor(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UC_PTR(ptr);
	ARG_UINT16(shape);
	ARG_UINT16(frame);

	//!! do we need to flag actor as temporary?

	Actor *newactor = createActor(shape, frame);
	if (!newactor) {
		perr << "I_createActor failed to create actor (" << shape
		     << ")." << Std::endl;
		return 0;
	}
	uint16 objID = newactor->getObjId();

	uint8 buf[2];
	buf[0] = static_cast<uint8>(objID);
	buf[1] = static_cast<uint8>(objID >> 8);
	UCMachine::get_instance()->assignPointer(ptr, buf, 2);

#if 0
	perr << "I_createActor: created actor #" << objID << " with shape " << shape << Std::endl;
#endif

	return objID;
}

uint32 Actor::I_createActorCru(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(other);

	if (!item || !other)
		return 0;

	const int gameDifficulty = World::get_instance()->getGameDifficulty();
	int npcDifficulty = (item->getMapNum() & 3) + 1;

	if (gameDifficulty < npcDifficulty)
		return 0;

	uint16 dtableidx = other->getNpcNum();

	const NPCDat *npcData = GameData::get_instance()->getNPCData(dtableidx);
	if (!npcData)
		return 0;

	int dir = item->getNpcNum() & 0xf;
	int frame = (dir * 2 + 4) & 0xf;
	uint16 shape = npcData->getShapeNo();

	enum extflags ext = static_cast<extflags>(0);
	if (shape == 0x597 || shape == 0x3ac)
		ext = EXT_FEMALE;

	Actor *newactor = ItemFactory::createActor(shape, frame, 0,
	                  Item::FLG_IN_NPC_LIST | Item::FLG_DISPOSABLE,
	                  0, 0, ext, true);
	if (!newactor) {
		perr << "I_createActorCru failed to create actor ("
			 << npcData->getShapeNo() << ")." << Std::endl;
		return 0;
	}

	// Most of these will be overwritten below, but this is cleaner..
	bool loaded = newactor->loadMonsterStats();
	if (!loaded) {
		perr << "I_createActorCru failed to load monster stats ("
			 << npcData->getShapeNo() << ")." << Std::endl;
		return 0;
	}

	newactor->setDir(Direction_FromUsecodeDir(dir));

	int32 x, y, z;
	item->getLocation(x, y, z);
	newactor->move(x, y, z);

	newactor->setDefaultActivity(0, other->getQuality() >> 8);
	newactor->setDefaultActivity(1, item->getQuality() >> 8);
	newactor->setDefaultActivity(2, other->getMapNum());

	newactor->setUnkByte(item->getQuality() & 0xff);

	bool wpnflag = (item->getMapNum() & 4);
	uint16 wpntype = npcData->getWpnType();
	uint16 wpntype2 = npcData->getWpnType2();

	if (World::get_instance()->getGameDifficulty() == 4) {
	   wpntype = NPCDat::randomlyGetStrongerWeaponTypes(shape);
	}

	if ((!wpntype || !wpnflag) && wpntype2) {
		wpntype = wpntype2;
	}

	if (wpntype) {
		// TODO: Nasty hard coded list.. use the ini file for this.
		static const int WPNSHAPES[] = {0, 0x032E, 0x032F, 0x0330, 0x038C, 0x0332, 0x0333,
			0x0334, 0x038E, 0x0388, 0x038A, 0x038D, 0x038B, 0x0386};
		// wpntype is an offset into wpn table
		Item *weapon = ItemFactory::createItem(WPNSHAPES[wpntype], 0, 0, 0, 0, newactor->getMapNum(), 0, true);
		if (weapon) {
			weapon->moveToContainer(newactor, false);
			newactor->_activeWeapon = weapon->getObjId();
		}
	}

	newactor->setCombatTactic(0);
	newactor->setHomePosition(x, y, z);

	/*
	 TODO: once I know what this field is.. seems to never be used in game?
	 newactor->setField0x12(item->getNpcNum() >> 4);
	 */

	return newactor->getObjId();
}

uint32 Actor::I_setUnkByte(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(value);
	if (actor)
		actor->setUnkByte(static_cast<uint8>(value & 0xff));
	return 0;
}

uint32 Actor::I_getUnkByte(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getUnkByte();
}

uint32 Actor::I_setActivity(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(activity);
	if (!actor) return 0;

	return actor->setActivity(activity);
}

uint32 Actor::I_setAirWalkEnabled(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(enabled);
	if (!actor) return 0;

	if (enabled)
		actor->setActorFlag(ACT_AIRWALK);
	else
		actor->clearActorFlag(ACT_AIRWALK);

	return 0;
}


uint32 Actor::I_getAirWalkEnabled(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->hasActorFlags(ACT_AIRWALK))
		return 1;
	else
		return 0;
}

uint32 Actor::I_schedule(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT32(time);
	if (!actor) return 0;

	return actor->schedule(time);
}


uint32 Actor::I_getEquip(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(type);
	if (!actor) return 0;

	return actor->getEquip(type + 1);
}

uint32 Actor::I_setEquip(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(type);
	ARG_ITEM_FROM_ID(item);
	if (!actor) return 0;
	if (!item) return 0;

	if (!actor->setEquip(item, false))
		return 0;

	// check it was added to the right slot
	assert(item->getZ() == type + 1 || (item->getShape() == BACKPACK_SHAPE && type == 6));

	return 1;
}

uint32 Actor::I_setDefaultActivity0(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(activity);
	if (!actor) return 0;

	actor->setDefaultActivity(0, activity);
	return 0;
}

uint32 Actor::I_setDefaultActivity1(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(activity);
	if (!actor) return 0;

	actor->setDefaultActivity(1, activity);
	return 0;
}

uint32 Actor::I_setDefaultActivity2(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(activity);
	if (!actor) return 0;

	actor->setDefaultActivity(2, activity);
	return 0;
}

uint32 Actor::I_getDefaultActivity0(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getDefaultActivity(0);
}

uint32 Actor::I_getDefaultActivity1(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getDefaultActivity(1);
}

uint32 Actor::I_getDefaultActivity2(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getDefaultActivity(2);
}

uint32 Actor::I_setCombatTactic(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;
	ARG_UINT16(tactic);

	actor->setCombatTactic(tactic);
	return 0;
}

uint32 Actor::I_getCurrentActivityNo(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getCurrentActivityNo();
}

uint32 Actor::I_getLastActivityNo(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getLastActivityNo();
}

uint32 Actor::I_turnToward(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	ARG_UINT16(dir);
	ARG_UINT16(dir16);

	Direction newdir = Direction_FromUsecodeDir(dir);
	Direction curdir = actor->getDir();
	Direction oneleft = Direction_OneLeft(curdir, dirmode_16dirs);
	Direction oneright = Direction_OneRight(curdir, dirmode_16dirs);

	if (curdir == newdir ||
		(!dir16 && (newdir == oneleft || newdir == oneright)))
		return 0;

	return actor->turnTowardDir(newdir);
}

uint32 Actor::I_isKneeling(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->hasFlags(ACT_KNEELING) ? 1 : 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
