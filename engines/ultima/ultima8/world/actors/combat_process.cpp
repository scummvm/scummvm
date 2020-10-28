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
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/actors/loiter_process.h"
#include "ultima/ultima8/world/actors/ambush_process.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CombatProcess)

CombatProcess::CombatProcess() : Process(), _target(0), _fixedTarget(0), _combatMode(CM_WAITING) {

}

CombatProcess::CombatProcess(Actor *actor) : _target(0), _fixedTarget(0), _combatMode(CM_WAITING) {
	assert(actor);
	_itemNum = actor->getObjId();

	_type = 0x00F2; // CONSTANT !
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
	if (!a || !a->hasFlags(Item::FLG_FASTAREA))
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

	Direction targetdir = getTargetDirection();
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
			uint16 idlepid = a->doAnim(idleanim, dir_current);
			waitFor(idlepid);
		} else {

			// attack
			ProcId attackanim = a->doAnim(Animation::attack, dir_current);

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
	const Actor *t = getActor(_target);

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

bool CombatProcess::isValidTarget(const Actor *target) const {
	assert(target);
	const Actor *a = getActor(_itemNum);
	if (!a) return false; // uh oh

	// don't target_ self
	if (target == a) return false;

	// not in the fastarea
	if (!target->hasFlags(Item::FLG_FASTAREA)) return false;

	// dead actors don't make good targets
	if (target->isDead()) return false;

	// feign death only works on undead and demons
	if (target->hasActorFlags(Actor::ACT_FEIGNDEATH)) {

		if ((a->getDefenseType() & WeaponInfo::DMG_UNDEAD) ||
		        (a->getShape() == 96)) return false; // CONSTANT!
	}

	// otherwise, ok
	return true;
}

bool CombatProcess::isEnemy(const Actor *target) const {
	assert(target);

	const Actor *a = getActor(_itemNum);
	if (!a) return false; // uh oh

	return ((a->getEnemyAlignment() & target->getAlignment()) != 0);
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
		const Actor *t = getActor(itemlist.getuint16(i));

		if (t && isValidTarget(t) && isEnemy(t)) {
			// found _target
			return itemlist.getuint16(i);
		}
	}

	// no suitable targets
	return 0;
}

Direction CombatProcess::getTargetDirection() const {
	const Actor *a = getActor(_itemNum);
	const Actor *t = getActor(_target);
	if (!a || !t)
		return dir_north; // shouldn't happen

	return a->getDirToItemCentre(*t);
}

void CombatProcess::turnToDirection(Direction direction) {
	Actor *a = getActor(_itemNum);
	if (!a)
		return;
	assert(a->isInCombat());
	uint16 waitpid = a->turnTowardDir(direction);
	if (waitpid)
		waitFor(waitpid);
}

bool CombatProcess::inAttackRange() const {
	const Actor *a = getActor(_itemNum);
	if (!a)
		return false; // shouldn't happen
	const ShapeInfo *shapeinfo = a->getShapeInfo();
	const MonsterInfo *mi = nullptr;
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
	if (!a)
		return; // shouldn't happen
	const ShapeInfo *shapeinfo = a->getShapeInfo();
	const MonsterInfo *mi = nullptr;
	if (shapeinfo) mi = shapeinfo->_monsterInfo;

	if (mi && mi->_shifter && a->getMapNum() != 43 && (getRandom() % 2) == 0) {
		// changelings (except the ones at the U8 endgame pentagram)

		// shift into a tree if nobody is around

		ProcId shift1pid = a->doAnim(static_cast<Animation::Sequence>(20), dir_current);
		Process *ambushproc = new AmbushProcess(a);
		ProcId ambushpid = Kernel::get_instance()->addProcess(ambushproc);
		ProcId shift2pid = a->doAnim(static_cast<Animation::Sequence>(21), dir_current);
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

void CombatProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint16LE(_target);
	ws->writeUint16LE(_fixedTarget);
	ws->writeByte(static_cast<uint8>(_combatMode));
}

bool CombatProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_target = rs->readUint16LE();
	_fixedTarget = rs->readUint16LE();
	_combatMode = static_cast<CombatMode>(rs->readByte());

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
