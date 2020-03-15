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
#include "ultima/ultima8/world/actors/combat_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/weapon_info.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/world/actors/monster_info.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/actors/loiter_process.h"
#include "ultima/ultima8/world/actors/ambush_process.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CombatProcess, Process)

CombatProcess::CombatProcess() : Process() {

}

CombatProcess::CombatProcess(Actor *actor_) {
	assert(actor_);
	_itemNum = actor_->getObjId();

	_type = 0x00F2; // CONSTANT !
	_target = 0;
	_fixedTarget = 0;
	_combatMode = CM_WAITING;
}

void CombatProcess::terminate() {
	Actor *a = getActor(_itemNum);
	if (a)
		a->clearActorFlag(Actor::ACT_INCOMBAT);

	Process::terminate();
}

void CombatProcess::run() {
	// TODO: handle invisible targets.
	// Monsters should attack you only if you are standing directly
	// next to them, or maybe only when you are attacking them.
	// They should not try to approach.

	Actor *a = getActor(_itemNum);
	if (!(a->getFlags() & Item::FLG_FASTAREA))
		return;

	Actor *t = getActor(_target);

	if (!t || !isValidTarget(t)) {
		// no _target? try to find one

		_target = seekTarget();

		if (!_target) {
			waitForTarget();
			return;
		}

		pout << "[COMBAT " << _itemNum << "] _target found: "
		     << _target << Std::endl;
		_combatMode = CM_WAITING;
	}

	int targetdir = getTargetDirection();
	if (a->getDir() != targetdir) {
		turnToDirection(targetdir);
		return;
	}

	if (inAttackRange()) {
		_combatMode = CM_ATTACKING;

		pout << "[COMBAT " << _itemNum << "] _target (" << _target
		     << ") in range" << Std::endl;

		bool hasidle1 = a->hasAnim(Animation::idle1);
		bool hasidle2 = a->hasAnim(Animation::idle2);

		if ((hasidle1 || hasidle2) && (getRandom() % 5) == 0) {
			// every once in a while, act threatening instead of attacking
			// TODO: maybe make frequency depend on monster type
			Animation::Sequence idleanim;

			if (!hasidle1) {
				idleanim = Animation::idle2;
			} else if (!hasidle2) {
				idleanim = Animation::idle1;
			} else {
				if (getRandom() % 2)
					idleanim = Animation::idle1;
				else
					idleanim = Animation::idle2;
			}
			uint16 idlepid = a->doAnim(idleanim, 8);
			waitFor(idlepid);
		} else {

			// attack
			ProcId attackanim = a->doAnim(Animation::attack, 8);

			// wait a while, depending on dexterity, before attacking again
			int dex = a->getDex();
			if (dex < 25) {
				int recoverytime = 3 * (25 - dex);
				Process *waitproc = new DelayProcess(recoverytime);
				ProcId waitpid = Kernel::get_instance()->addProcess(waitproc);
				waitproc->waitFor(attackanim);
				waitFor(waitpid);
			} else {
				waitFor(attackanim);
			}
		}

		return;
	} else if (_combatMode != CM_PATHFINDING) {
		// not in range? See if we can get in range

		Process *pfproc = new PathfinderProcess(a, _target, true);

		waitFor(Kernel::get_instance()->addProcess(pfproc));
		_combatMode = CM_PATHFINDING;
		return;
	}

	_combatMode = CM_WAITING;
	waitForTarget();
}

ObjId CombatProcess::getTarget() {
	Actor *t = getActor(_target);

	if (!t || !isValidTarget(t))
		_target = 0;

	return _target;
}

void CombatProcess::setTarget(ObjId newtarget) {
	if (_fixedTarget == 0) {
		_fixedTarget = newtarget; // want to prevent seekTarget from changing it
	}

	_target = newtarget;
}

bool CombatProcess::isValidTarget(Actor *target_) {
	assert(target_);
	Actor *a = getActor(_itemNum);
	if (!a) return false; // uh oh

	// don't target_ self
	if (target_ == a) return false;

	// not in the fastarea
	if (!(target_->getFlags() & Item::FLG_FASTAREA)) return false;

	// dead actors don't make good targets
	if (target_->isDead()) return false;

	// feign death only works on undead and demons
	if (target_->getActorFlags() & Actor::ACT_FEIGNDEATH) {

		if ((a->getDefenseType() & WeaponInfo::DMG_UNDEAD) ||
		        (a->getShape() == 96)) return false; // CONSTANT!
	}

	// otherwise, ok
	return true;
}

bool CombatProcess::isEnemy(Actor *target_) {
	assert(target_);

	Actor *a = getActor(_itemNum);
	if (!a) return false; // uh oh

	return ((a->getEnemyAlignment() & target_->getAlignment()) != 0);
}

ObjId CombatProcess::seekTarget() {
	Actor *a = getActor(_itemNum);
	if (!a) return 0; // uh oh

	if (_fixedTarget) {
		Actor *t = getActor(_fixedTarget);
		if (t && isValidTarget(t))
			return _fixedTarget; // no need to search
	}

	UCList itemlist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE);
	CurrentMap *cm = World::get_instance()->getCurrentMap();
	cm->areaSearch(&itemlist, script, sizeof(script), a, 768, false);

	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		Actor *t = getActor(itemlist.getuint16(i));

		if (t && isValidTarget(t) && isEnemy(t)) {
			// found _target
			return itemlist.getuint16(i);
		}
	}

	// no suitable targets
	return 0;
}

int CombatProcess::getTargetDirection() {
	Actor *a = getActor(_itemNum);
	Actor *t = getActor(_target);

	return a->getDirToItemCentre(*t);
}

void CombatProcess::turnToDirection(int direction) {
	Actor *a = getActor(_itemNum);
	int curdir = a->getDir();
	int step = 1;
	if ((curdir - direction + 8) % 8 < 4) step = -1;
	Animation::Sequence turnanim = Animation::combatStand;

	ProcId prevpid = 0;
	bool done = false;

	for (int dir = curdir; !done;) {
		ProcId animpid = a->doAnim(turnanim, dir);

		if (dir == direction) done = true;

		if (prevpid) {
			Process *proc = Kernel::get_instance()->getProcess(animpid);
			assert(proc);
			proc->waitFor(prevpid);
		}

		prevpid = animpid;

		dir = (dir + step + 8) % 8;
	}

	if (prevpid) waitFor(prevpid);
}

bool CombatProcess::inAttackRange() {
	Actor *a = getActor(_itemNum);
	ShapeInfo *shapeinfo = a->getShapeInfo();
	MonsterInfo *mi = nullptr;
	if (shapeinfo) mi = shapeinfo->_monsterInfo;

	if (mi && mi->_ranged)
		return true; // ranged attacks (ghost's fireball) always in range

	AnimationTracker tracker;
	if (!tracker.init(a, Animation::attack, a->getDir(), 0))
		return false;

	while (tracker.step()) {
		if (tracker.hitSomething()) break;
	}

	ObjId hit = tracker.hitSomething();
	if (hit == _target) return true;

	return false;
}

void CombatProcess::waitForTarget() {
	Actor *a = getActor(_itemNum);
	ShapeInfo *shapeinfo = a->getShapeInfo();
	MonsterInfo *mi = nullptr;
	if (shapeinfo) mi = shapeinfo->_monsterInfo;

	if (mi && mi->_shifter && a->getMapNum() != 43 && (getRandom() % 2) == 0) {
		// changelings (except the ones at the U8 endgame pentagram)

		// shift into a tree if nobody is around

		ProcId shift1pid = a->doAnim(static_cast<Animation::Sequence>(20), 8);
		Process *ambushproc = new AmbushProcess(a);
		ProcId ambushpid = Kernel::get_instance()->addProcess(ambushproc);
		ProcId shift2pid = a->doAnim(static_cast<Animation::Sequence>(21), 8);
		Process *shift2proc = Kernel::get_instance()->getProcess(shift2pid);

		ambushproc->waitFor(shift1pid);
		shift2proc->waitFor(ambushpid);
		waitFor(shift2proc);

	} else {
		waitFor(Kernel::get_instance()->addProcess(new LoiterProcess(a, 1)));
	}
}

void CombatProcess::dumpInfo() const {
	Process::dumpInfo();
	pout << "Target: " << _target << Std::endl;
}

void CombatProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write2(_target);
	ods->write2(_fixedTarget);
	ods->write1(static_cast<uint8>(_combatMode));
}

bool CombatProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	_target = ids->read2();
	_fixedTarget = ids->read2();
	_combatMode = static_cast<CombatMode>(ids->read1());

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
