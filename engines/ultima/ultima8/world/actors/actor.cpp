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
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/world/actors/anim_action.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/actors/pathfinder.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/actors/resurrection_process.h"
#include "ultima/ultima8/world/destroy_item_process.h"
#include "ultima/ultima8/world/actors/clear_feign_death_process.h"
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/world/actors/loiter_process.h"
#include "ultima/ultima8/world/actors/combat_process.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Actor, Container)

Actor::Actor() : _strength(0), _dexterity(0), _intelligence(0),
	  _hitPoints(0), _mana(0), _alignment(0), _enemyAlignment(0),
	  _lastAnim(Animation::walk), _animFrame(0), _direction(0),
		_fallStart(0), _unk0C(0), _actorFlags(0) {
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
	ShapeInfo *shapeinfo = getShapeInfo();
	MonsterInfo *mi = nullptr;
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
	ShapeInfo *shapeinfo = getShapeInfo();
	MonsterInfo *mi = nullptr;
	if (shapeinfo) mi = shapeinfo->_monsterInfo;
	if (!mi)
		return false;

	Std::vector<TreasureInfo> &treasure = mi->_treasure;

	for (unsigned int i = 0; i < treasure.size(); ++i) {
		TreasureInfo &ti = treasure[i];
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
					ShapeInfo *si = mainshapes->getShapeInfo(s);
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
				pout << "Unhandled _special treasure: " << ti._special
				     << Std::endl;
			}
			continue;
		}

		// if _shapes.size() == 1 and the given shape is SF_QUANTITY,
		// then produce a stack of that shape (ignoring frame)

		if (ti._shapes.size() == 1) {
			uint32 shapeNum = ti._shapes[0];
			ShapeInfo *si = mainshapes->getShapeInfo(shapeNum);
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
		for (i = 0; (int)i < count; ++i) {
			// pick shape
			int n = getRandom() % ti._shapes.size();
			uint32 shapeNum = ti._shapes[n];

			// pick frame
			n = getRandom() % ti._frames.size();
			uint32 frameNum = ti._frames[n];

			ShapeInfo *si = GameData::get_instance()->getMainShapes()->
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
	const unsigned int backpack_shape = 529; //!! *cough* constant
	uint32 equiptype = item->getShapeInfo()->_equipType;
	bool backpack = (item->getShape() == backpack_shape);

	// valid item type?
	if (equiptype == ShapeInfo::SE_NONE && !backpack) return false;

	// now check 'equipment slots'
	// we can have one item of each equipment type, plus one backpack
	Std::list<Item *>::iterator iter;
	for (iter = _contents.begin(); iter != _contents.end(); ++iter) {
		if ((*iter)->getObjId() == item->getObjId()) continue;

		uint32 cet = (*iter)->getShapeInfo()->_equipType;
		bool cbackpack = ((*iter)->getShape() == backpack_shape);

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
	const unsigned int backpack_shape = 529; //!! *cough* constant

	Std::list<Item *>::const_iterator iter;
	for (iter = _contents.begin(); iter != _contents.end(); ++iter) {
		uint32 cet = (*iter)->getShapeInfo()->_equipType;
		bool cbackpack = ((*iter)->getShape() == backpack_shape);

		if (((*iter)->getFlags() & FLG_EQUIPPED) &&
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

uint16 Actor::doAnim(Animation::Sequence anim, int dir, unsigned int steps) {
	if (dir < 0 || dir > 8) {
		perr << "Actor::doAnim: Invalid _direction (" << dir << ")" << Std::endl;
		return 0;
	}

#if 0
	if (tryAnim(anim, dir)) {
		perr << "Actor::doAnim: tryAnim = Ok!" << Std::endl;
	} else {
		perr << "Actor::doAnim: tryAnim = bad!" << Std::endl;
	}
#endif

	Process *p = new ActorAnimProcess(this, anim, dir, steps);

	return Kernel::get_instance()->addProcess(p);
}

bool Actor::hasAnim(Animation::Sequence anim) {
	AnimationTracker tracker;

	return tracker.init(this, anim, 0);
}

Animation::Result Actor::tryAnim(Animation::Sequence anim, int dir,
                                 unsigned int steps, PathfindingState *state) {
	if (dir < 0 || dir > 8) return Animation::FAILURE;

	if (dir == 8) dir = getDir();

	AnimationTracker tracker;
	if (!tracker.init(this, anim, dir, state))
		return Animation::FAILURE;

	AnimAction *animaction = tracker.getAnimAction();

	if (!animaction) return Animation::FAILURE;

	unsigned int curstep = 0;

	while (tracker.step() && (!steps || curstep >= steps)) {
		curstep++;
	}

	if (tracker.isBlocked() &&
	        !(animaction->_flags & AnimAction::AAF_UNSTOPPABLE)) {
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

uint16 Actor::cSetActivity(int activity) {
	switch (activity) {
	case 0: // loiter
		Kernel::get_instance()->addProcess(new LoiterProcess(this));
		return Kernel::get_instance()->addProcess(new DelayProcess(1));
		break;
	case 1: // combat
		setInCombat();
		return 0;
	case 2: // stand
		// NOTE: temporary fall-throughs!
		return doAnim(Animation::stand, 8);

	default:
		perr << "Actor::cSetActivity: invalid activity (" << activity << ")"
		     << Std::endl;
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
	ShapeInfo *si = getShapeInfo();
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


void Actor::receiveHit(uint16 other, int dir, int damage, uint16 damage_type) {
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

	if (other == 1 && attacker->getLastAnim() != Animation::kick) {
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
		if (dir > 2) {
			start = 13;
			end = 25;
		}

		int32 xv, yv, zv;
		getLocation(xv, yv, zv);
		zv += (getRandom() % 24);
		Process *sp = new SpriteProcess(620, start, end, 1, 1, xv, yv, zv);
		Kernel::get_instance()->addProcess(sp);
	}

	if (damage > 0 && !(getActorFlags() & (ACT_IMMORTAL | ACT_INVINCIBLE))) {
		if (damage >= _hitPoints) {
			// we're dead

			if (getActorFlags() & ACT_WITHSTANDDEATH) {
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
			doAnim(Animation::fallBackwards, 8);

			// TODO: shake head after getting back up when not in combat
			return;
		}

		// got hit, so abort current animation
		fallingprocid = killAllButFallAnims(false);
	}

	// if avatar was blocking; do a quick stopBlock/startBlock and play SFX
	if (_objId == 1 && getLastAnim() == Animation::startBlock) {
		ProcId anim1pid = doAnim(Animation::stopBlock, 8);
		ProcId anim2pid = doAnim(Animation::startBlock, 8);

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
			setInCombat();

		CombatProcess *cp = getCombatProcess();
		assert(cp);
		cp->setTarget(target);

		if (target == 1) {
			// call for help
		}
	}

	if (damage && !fallingprocid) {
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
	clearActorFlag(ACT_INCOMBAT);

	ProcId animprocid = 0;
#if 1
	animprocid = killAllButFallAnims(true);
#else
	Kernel::get_instance()->killProcesses(getObjId(), 6, true); // CONSTANT!
#endif

	if (!animprocid)
		animprocid = doAnim(Animation::die, getDir());


	MainActor *avatar = getMainActor();
	// if hostile to avatar
	if (getEnemyAlignment() & avatar->getAlignment()) {
		if (avatar->isInCombat()) {
			// play victory fanfare
			MusicProcess::get_instance()->playCombatMusic(109);
			// and resume combat music afterwards
			MusicProcess::get_instance()->queueMusic(98);
		}
	}


	destroyContents();
	giveTreasure();

	ShapeInfo *shapeinfo = getShapeInfo();
	MonsterInfo *mi = nullptr;
	if (shapeinfo) mi = shapeinfo->_monsterInfo;

	if (mi && mi->_resurrection && !(damageType & WeaponInfo::DMG_FIRE)) {
		// this monster will be resurrected after a while

		pout << "Actor::die: scheduling resurrection" << Std::endl;

		int timeout = ((getRandom() % 25) + 5) * 30; // 5-30 seconds

		Process *resproc = new ResurrectionProcess(this);
		Kernel::get_instance()->addProcess(resproc);

		Process *delayproc = new DelayProcess(timeout);
		Kernel::get_instance()->addProcess(delayproc);

		ProcId animpid = doAnim(Animation::standUp, 8);
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
		ActorAnimProcess *p = p_dynamic_cast<ActorAnimProcess *>(*iter);
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
		        !(getActorFlags() & ACT_STUNNED)) {
			damage -= getStr() / 5;
		}

		int ACmod = 3 * getArmourClass();
		if (damage_type & WeaponInfo::DMG_FIRE)
			ACmod /= 2; // armour doesn't protect from fire as well

		if (getActorFlags() & ACT_STUNNED)
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

		if ((getActorFlags() & ACT_STUNNED) ||
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
	CombatProcess *cp = p_dynamic_cast<CombatProcess *>(p);
	assert(cp);

	return cp;
}

void Actor::setInCombat() {
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

void Actor::clearInCombat() {
	if ((_actorFlags & ACT_INCOMBAT) == 0) return;

	CombatProcess *cp = getCombatProcess();
	cp->terminate();

	clearActorFlag(ACT_INCOMBAT);
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

		if (npc->getActorFlags() & (ACT_DEAD | ACT_FEIGNDEATH)) continue;
		if (!(npc->getActorFlags() & ACT_INCOMBAT)) continue;

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

void Actor::saveData(ODataSource *ods) {
	Container::saveData(ods);
	ods->write2(_strength);
	ods->write2(_dexterity);
	ods->write2(_intelligence);
	ods->write2(_hitPoints);
	ods->write2(_mana);
	ods->write2(_alignment);
	ods->write2(_enemyAlignment);
	ods->write2(_lastAnim);
	ods->write2(_animFrame);
	ods->write2(_direction);
	ods->write4(_fallStart);
	ods->write4(_actorFlags);
	ods->write1(_unk0C);
}

bool Actor::loadData(IDataSource *ids, uint32 version) {
	if (!Container::loadData(ids, version)) return false;

	_strength = static_cast<int16>(ids->read2());
	_dexterity = static_cast<int16>(ids->read2());
	_intelligence = static_cast<int16>(ids->read2());
	_hitPoints = ids->read2();
	_mana = static_cast<int16>(ids->read2());
	_alignment = ids->read2();
	_enemyAlignment = ids->read2();
	_lastAnim = static_cast<Animation::Sequence>(ids->read2());
	_animFrame = ids->read2();
	_direction = ids->read2();
	_fallStart = ids->read4();
	_actorFlags = ids->read4();
	_unk0C = ids->read1();

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

	return actor->doAnim(static_cast<Animation::Sequence>(anim), dir);
}

uint32 Actor::I_getDir(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getDir();
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

	actor->setInCombat();

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

	CombatProcess *cp = actor->getCombatProcess();
	if (!cp) {
		actor->setInCombat();
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

	return 0;
}

uint32 Actor::I_clrDead(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->clearActorFlag(ACT_DEAD);

	return 0;
}

uint32 Actor::I_isImmortal(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->getActorFlags() & ACT_IMMORTAL)
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

	if (actor->getActorFlags() & ACT_WITHSTANDDEATH)
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

	if (actor->getActorFlags() & ACT_FEIGNDEATH)
		return 1;
	else
		return 0;
}

uint32 Actor::I_setFeignDeath(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->getActorFlags() & ACT_FEIGNDEATH)
		return 0;

	actor->setActorFlag(ACT_FEIGNDEATH);

	ProcId animfallpid = actor->doAnim(Animation::die, 8);
	Process *animfallproc = Kernel::get_instance()->getProcess(animfallpid);
	assert(animfallproc);

	ProcId animstandpid = actor->doAnim(Animation::standUp, 8);
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

	uint32 count = Kernel::get_instance()->getNumProcesses(id, 0x00F0);
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

uint32 Actor::I_cSetActivity(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(activity);
	if (!actor) return 0;

	return actor->cSetActivity(activity);
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

	if (actor->getActorFlags() & ACT_AIRWALK)
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
	assert(item->getZ() == type + 1 || (item->getShape() == 529 && type == 6));

	return 1;
}

} // End of namespace Ultima8
} // End of namespace Ultima
