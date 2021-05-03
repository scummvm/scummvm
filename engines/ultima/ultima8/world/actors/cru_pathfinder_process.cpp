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

#include "ultima/ultima8/world/actors/cru_pathfinder_process.h"
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/world/actors/attack_process.h"

#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/actors/pathfinder.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/kernel/kernel.h"

namespace Ultima {
namespace Ultima8 {

static const int8 PATHFIND_DIR_OFFSETS_1[8] = {0,  2, -2,  4, -4,  6, -6, -8};
static const int8 PATHFIND_DIR_OFFSETS_2[8] = {0, -2,  2, -4,  4, -6,  6, -8};

DEFINE_RUNTIME_CLASSTYPE_CODE(CruPathfinderProcess)

CruPathfinderProcess::CruPathfinderProcess() : Process(),
		_currentStep(0), _targetItem(0), _currentDistance(0),
		_targetX(0), _targetY(0), _targetZ(0), _randomFlag(false),
		_nextTurn(false), _turnAtEnd(false), _lastDir(dir_current),
		_nextDir(dir_current), _nextDir2(dir_current),
		_solidObject(true), _directPathBlocked(false), _noShotAvailable(true),
		_dir16Flag(false), _stopDistance(0)
{
}

CruPathfinderProcess::CruPathfinderProcess(Actor *actor, Item *target, int maxsteps, int stopdistance, bool turnatend) :
		_currentStep(0), _currentDistance(0), _targetX(0), _targetY(0), _targetZ(0),
		_maxSteps(maxsteps), _stopDistance(stopdistance), _nextTurn(false), _turnAtEnd(turnatend),
		_lastDir(dir_current), _nextDir(dir_current), _nextDir2(dir_current),
		_directPathBlocked(false), _noShotAvailable(true), _dir16Flag(false) {
	assert(actor && target);
	_itemNum = actor->getObjId();
	_type = PathfinderProcess::PATHFINDER_PROC_TYPE;
	_randomFlag = (getRandom() % 2) != 0;
	_targetItem = target->getObjId();
	target->getLocation(_targetX, _targetY, _targetZ);

	int32 ax, ay, az;
	actor->getLocation(ax, ay, az);
	_currentDistance = MAX(abs(ax - _targetX), abs(ay - _targetY));

	const ShapeInfo *si = actor->getShapeInfo();
	_solidObject = (si->_flags & ShapeInfo::SI_SOLID) && si->_z > 0;

	// TODO: check if flag already set? kill other pathfinders?
	assert(!actor->hasActorFlags(Actor::ACT_PATHFINDING));
	actor->setActorFlag(Actor::ACT_PATHFINDING);
}

CruPathfinderProcess::CruPathfinderProcess(Actor *actor, int32 x, int32 y, int32 z, int maxsteps, int stopdistance, bool turnatend) :
		_targetX(x), _targetY(y), _targetZ(z), _targetItem(0), _currentStep(0),
		_maxSteps(maxsteps), _stopDistance(stopdistance), _nextTurn(false), _turnAtEnd(turnatend),
		_lastDir(dir_current), _nextDir(dir_current), _nextDir2(dir_current),
		_directPathBlocked(false), _noShotAvailable(true), _dir16Flag(false) {
	assert(actor);
	_itemNum = actor->getObjId();
	_type = PathfinderProcess::PATHFINDER_PROC_TYPE;
	_randomFlag = (getRandom() % 2) != 0;

	int32 ax, ay, az;
	actor->getLocation(ax, ay, az);
	_currentDistance = MAX(abs(ax - _targetX), abs(ay - _targetY));

	const ShapeInfo *si = actor->getShapeInfo();
	_solidObject = (si->_flags & ShapeInfo::SI_SOLID) && si->_z > 0;

	// TODO: check if flag already set? kill other pathfinders?
	assert(!actor->hasActorFlags(Actor::ACT_PATHFINDING));
	actor->setActorFlag(Actor::ACT_PATHFINDING);
}

CruPathfinderProcess::~CruPathfinderProcess() {
}

void CruPathfinderProcess::terminate() {
	Actor *actor = getActor(_itemNum);
	if (actor && !actor->isDead()) {
		// TODO: only clear if it was set by us?
		// (slightly more complicated if we kill other pathfinders on startup)
		actor->clearActorFlag(Actor::ACT_PATHFINDING);

		uint16 turnproc = 0;
		if (_turnAtEnd) {
			Direction destdir = dir_current;
			// TODO: this logic can be cleaned up a bit by just updating targetx/y?
			int32 ix, iy, iz;
			actor->getLocationAbsolute(ix, iy, iz);
			if (_targetItem == 0) {
				destdir = Direction_GetWorldDir(_targetY - iy, _targetX - ix, dirmode_8dirs);
			} else {
				Item *target = getItem(_targetItem);
				if (target) {
					int32 tx, ty, tz;
					target->getLocationAbsolute(tx, ty, tz);
					destdir = Direction_GetWorldDir(ty - iy, tx - ix, dirmode_8dirs);
				}
			}
			if (destdir != dir_current)
				turnproc = actor->turnTowardDir(destdir);
		}
		if (!turnproc && _noShotAvailable) {
			Animation::Sequence standanim = (actor->isInCombat() ? Animation::combatStandSmallWeapon : Animation::stand);
			actor->doAnim(standanim, dir_current);
		}
	}

	Process::terminate();
}

Direction CruPathfinderProcess::nextDirFromPoint(struct Point3 &npcpt) {
	const Direction dirtotarget = Direction_GetWorldDir(_targetY - npcpt.y, _targetX - npcpt.x, dirmode_8dirs);
	Actor *npc = getActor(_itemNum);

	//assert(npc);

	const int maxdiffxy = MAX(abs(npcpt.x - _targetX), abs(npcpt.y - _targetY));
	if (maxdiffxy < _currentDistance) {
		// each time we get closer, check again if we can walk toward the target.
		_currentDistance = maxdiffxy;
		PathfindingState state;
		state._x = npcpt.x;
		state._y = npcpt.y;
		state._z = npcpt.z;
		state._direction = dirtotarget;
		state._combat = npc->isInCombat();
		Animation::Sequence anim = npc->isInCombat() ? Animation::walk : Animation::advanceSmallWeapon;
		Animation::Result result = npc->tryAnim(anim, dirtotarget, 0, &state);
		if (result == Animation::SUCCESS) {
			_directPathBlocked = false;
		}
	}

	int startoff = 0;
	Direction dirtable[8];
	Direction nextdir_table[8];

	if (!_directPathBlocked) {
		for (int i = 0; i < 8; i++) {
			if (_randomFlag) {
				dirtable[i] = Direction_TurnByDelta(dirtotarget, PATHFIND_DIR_OFFSETS_1[i], dirmode_16dirs);
			} else {
				dirtable[i] = Direction_TurnByDelta(dirtotarget, PATHFIND_DIR_OFFSETS_2[i], dirmode_16dirs);
			}
		}

	} else {

		int diroffset;
		if (_randomFlag) {
			diroffset = (_nextTurn ? 2 : -2);
		} else {
			diroffset = (_nextTurn ? -2 : 2);
		}
		nextdir_table[0] = Direction_TurnByDelta(_nextDir, diroffset + 8, dirmode_16dirs);

		for (int i = 1; i < 8; i++) {
			if (_randomFlag) {
				diroffset = (_nextTurn ? 2 : -2);
			} else {
				diroffset = (_nextTurn ? -2 : 2);
			}
			nextdir_table[i] = Direction_TurnByDelta(nextdir_table[i - 1], diroffset, dirmode_16dirs);
		}
		startoff = 1;
	}

	PathfindingState state;
	Animation::Result animresult = Animation::SUCCESS;
	int i;
	for (i = startoff; i < 8; i++) {
		// TODO: double-check these in disasm
		if (_directPathBlocked && i == 2)
			continue;
		if (_directPathBlocked && i == 7)
			break;

		if (_directPathBlocked)
			_nextDir2 = nextdir_table[i];
		else
			_nextDir2 = dirtable[i];

		// LAB_1110_0c26:
		Animation::Sequence anim = npc->isInCombat() ? Animation::walk : Animation::advanceSmallWeapon;
		state._x = npcpt.x;
		state._y = npcpt.y;
		state._z = npcpt.z;
		state._direction = _nextDir2;
		state._combat = npc->isInCombat();
		animresult = npc->tryAnim(anim, _nextDir2, 0, &state);

		// Note: this will never trigger in our code -
		// "tryAnim" code seems to behave differently in original?
		/*if (_solidObject && ((_result >> 1) & 1)) {
			_turnAtEnd = true;
			return dir_invalid;
		}*/

		if (_stopDistance && (MAX(abs(_targetX - state._x), abs(_targetY - state._y)) <= _stopDistance)) {
			_turnAtEnd = true;
			return dir_invalid;
		}

		if (animresult == Animation::SUCCESS)
			break;
	}

	// LAB_1110_0dd5:
	if (animresult != Animation::SUCCESS)
		return dir_current;

	if ((_nextDir2 != dirtotarget) && !_directPathBlocked) {
		_directPathBlocked = true;
		_nextTurn = (i % 2);
	}

	npcpt.x = state._x;
	npcpt.y = state._y;
	npcpt.z = state._z;
	bool is_controlled = World::get_instance()->getControlledNPCNum() == _itemNum;
	if (npc->isInCombat() && !is_controlled) {
		AttackProcess *attackproc = dynamic_cast<AttackProcess *>
				(Kernel::get_instance()->findProcess(_itemNum, AttackProcess::ATTACK_PROCESS_TYPE));
		if (attackproc) {
			const Actor *target = getActor(attackproc->getTarget());
			if (target && npc->isOnScreen() && npc->fireDistance(target, dirtotarget, 0, 0, 0)) {
				npc->doAnim(Animation::stand, dir_current);
				attackproc->setField7F();
				_noShotAvailable = false;
				_turnAtEnd = true;
				return dir_invalid;
			}
		}
	}
	return _nextDir2;
}


void CruPathfinderProcess::run() {
	Actor *npc = getActor(_itemNum);
	if (!npc || !npc->hasFlags(Item::FLG_FASTAREA))
		return;

	if (npc->isDead()) {
		terminate();
		return;
	}

	if (_dir16Flag) {
		terminate(); // terminate 1
		return;
	}

	if (_targetItem != 0 && _solidObject) {
		Item *target = getItem(_targetItem);
		if (target)
			target->getLocation(_targetX, _targetY, _targetZ);
	}

	Point3 npcpt;
	npc->getLocation(npcpt);

	if (_targetX == npcpt.x && _targetY == npcpt.y) {
		terminate(); // _destpt.z != npcpt.z
		return;
	}
	const Direction lastdir = _nextDir;
	_nextDir = nextDirFromPoint(npcpt);
	_lastDir = lastdir;
	if (_nextDir == dir_current) {
	   terminate(); //0
		return;
	}

	if (_nextDir == dir_invalid) {
		_dir16Flag = true;
	} else {
	   if (_currentStep == _maxSteps) {
			terminate(); //0
			return;
		}
	}

	Direction newdir;
	if (!_dir16Flag) {
		newdir = _nextDir;
	} else {
		newdir = _nextDir2;
	}

	uint16 turnpid = npc->turnTowardDir(newdir);
	Animation::Sequence anim = npc->isInCombat() ? Animation::advanceSmallWeapon : Animation::walk;
	uint16 animpid = npc->doAnim(anim, newdir);
	if (turnpid)
		Kernel::get_instance()->getProcess(animpid)->waitFor(turnpid);
	waitFor(animpid);
	_currentStep += 1;
}

void CruPathfinderProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint16LE(_targetItem);
	ws->writeUint16LE(static_cast<uint16>(_targetX));
	ws->writeUint16LE(static_cast<uint16>(_targetY));
	ws->writeUint16LE(static_cast<uint16>(_targetZ));
	ws->writeUint16LE(static_cast<uint16>(_currentDistance));
	ws->writeByte(_randomFlag ? 1 : 0);
	ws->writeByte(_nextTurn ? 1 : 0);
	ws->writeByte(_turnAtEnd ? 1 : 0);
	ws->writeByte(_lastDir);
	ws->writeByte(_nextDir);
	ws->writeByte(_nextDir2);
	ws->writeByte(_solidObject ? 1 : 0);
	ws->writeByte(_directPathBlocked ? 1 : 0);
	ws->writeByte(_noShotAvailable ? 1 : 0);
	ws->writeByte(_dir16Flag ? 1 : 0);
	ws->writeUint16LE(static_cast<uint16>(_currentStep));
	ws->writeUint16LE(static_cast<uint16>(_maxSteps));
	ws->writeUint16LE(static_cast<uint16>(_stopDistance));
}

bool CruPathfinderProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_targetItem = rs->readUint16LE();
	_targetX = rs->readUint16LE();
	_targetY = rs->readUint16LE();
	_targetZ = rs->readUint16LE();
	_currentDistance = rs->readUint16LE();
	_randomFlag = rs->readByte();
	_nextTurn = rs->readByte();
	_turnAtEnd = rs->readByte();
	_lastDir = static_cast<Direction>(rs->readByte());
	_nextDir = static_cast<Direction>(rs->readByte());
	_nextDir2 = static_cast<Direction>(rs->readByte());
	_solidObject = rs->readByte();
	_directPathBlocked = rs->readByte();
	_noShotAvailable = rs->readByte();
	_dir16Flag = rs->readByte();
	_currentStep = rs->readUint16LE();
	_maxSteps = rs->readUint16LE();
	_stopDistance = rs->readUint16LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
