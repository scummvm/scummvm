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
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/misc/direction_util.h"

namespace Ultima {
namespace Ultima8 {

static const unsigned int PATH_OK = 1;
static const unsigned int PATH_FAILED = 0;

DEFINE_RUNTIME_CLASSTYPE_CODE(PathfinderProcess)

PathfinderProcess::PathfinderProcess() : Process(),
		_currentStep(0), _targetItem(0), _hitMode(false),
		_target() {
}

PathfinderProcess::PathfinderProcess(Actor *actor, ObjId itemid, bool hit) :
		_currentStep(0), _targetItem(itemid), _hitMode(hit),
		_target() {
	assert(actor);
	_itemNum = actor->getObjId();
	_type = PATHFINDER_PROC_TYPE;

	Item *item = getItem(itemid);
	if (!item) {
		warning("PathfinderProcess: non-existent target");
		// can't get there...
		_result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	assert(_targetItem);

	_target = item->getLocation();

	Pathfinder pf;
	pf.init(actor);
	pf.setTarget(item, hit);

	bool ok = pf.pathfind(_path);

	if (!ok) {
		// can't get there...
		debugC(kDebugPath, "PathfinderProcess: actor %d failed to find path", _itemNum);
		_result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	// TODO: check if flag already set? kill other pathfinders?
	actor->setActorFlag(Actor::ACT_PATHFINDING);
}

PathfinderProcess::PathfinderProcess(Actor *actor, const Point3 &target) :
		_target(target), _targetItem(0), _currentStep(0),
		_hitMode(false) {
	assert(actor);
	_itemNum = actor->getObjId();
	_type = PATHFINDER_PROC_TYPE;

	Pathfinder pf;
	pf.init(actor);
	pf.setTarget(_target);

	bool ok = pf.pathfind(_path);

	if (!ok) {
		// can't get there...
		debugC(kDebugPath, "PathfinderProcess: actor %d failed to find path", _itemNum);
		_result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	// TODO: check if flag already set? kill other pathfinders?
	actor->setActorFlag(Actor::ACT_PATHFINDING);
}

PathfinderProcess::~PathfinderProcess() {
}

void PathfinderProcess::terminate() {
	Actor *actor = getActor(_itemNum);
	if (actor) {
		// TODO: only clear if it was set by us?
		// (slightly more complicated if we kill other pathfinders on startup)
		actor->clearActorFlag(Actor::ACT_PATHFINDING);
	}

	Process::terminate();
}

void PathfinderProcess::run() {
	Actor *actor = getActor(_itemNum);
	assert(actor);
	// if not in the fastarea, do nothing
	if (!actor->hasFlags(Item::FLG_FASTAREA)) return;


	bool ok = true;

	if (_targetItem) {
		Item *item = getItem(_targetItem);
		if (!item) {
			warning("PathfinderProcess: target missing");
			_result = PATH_FAILED;
			terminate();
			return;
		}

		Point3 cur = item->getLocation();
		if (ABS(cur.x - _target.x) >= 32 || ABS(cur.y - _target.y) >= 32 ||
		        ABS(cur.z - _target.z) >= 8) {
			// target moved
			ok = false;
		}
	}

	if (ok && _currentStep >= _path.size()) {
		// done
		debugC(kDebugPath, "PathfinderProcess: done");
		_result = PATH_OK;
		terminate();
		return;
	}

	// try to take the next step
	debugC(kDebugPath, "PathfinderProcess: trying step");

	// if actor is still animating for whatever reason, wait until he stopped
	// FIXME: this should happen before the pathfinder is actually called,
	// since the running animation may move the actor, which could break
	// the found _path.
	if (actor->hasActorFlags(Actor::ACT_ANIMLOCK)) {
		debugC(kDebugPath, "PathfinderProcess: ANIMLOCK, waiting");
		return;
	}

	if (ok) {
		ok = actor->tryAnim(_path[_currentStep]._action,
		                    _path[_currentStep]._direction,
		                    _path[_currentStep]._steps) == Animation::SUCCESS;
	}

	if (!ok) {
		debugC(kDebugPath, "PathfinderProcess: recalculating _path");

		// need to redetermine _path
		ok = true;
		Pathfinder pf;
		pf.init(actor);
		if (_targetItem) {
			Item *item = getItem(_targetItem);
			if (!item)
				ok = false;
			else {
				if (_hitMode && !actor->isInCombat()) {
					// Actor exited combat mode
					_hitMode = false;
				}
				pf.setTarget(item, _hitMode);
				_target = item->getLocation();
			}
		} else {
			pf.setTarget(_target);
		}
		if (ok)
			ok = pf.pathfind(_path);

		_currentStep = 0;
		if (!ok) {
			// can't get there anymore
			debugC(kDebugPath, "PathfinderProcess: actor %d failed to find path", _itemNum);
			_result = PATH_FAILED;
			terminate();
			return;
		}
	}

	if (_currentStep >= _path.size()) {
		debugC(kDebugPath, "PathfinderProcess: done");
		// done
		_result = PATH_OK;
		terminate();
		return;
	}

	uint16 animpid = actor->doAnim(_path[_currentStep]._action,
	                               _path[_currentStep]._direction,
	                               _path[_currentStep]._steps);

	debugC(kDebugPath, "PathfinderProcess(%u): taking step %d, %d (animpid=%u)",
		getPid(), _path[_currentStep]._action, _path[_currentStep]._direction, animpid);

	_currentStep++;

	waitFor(animpid);
}

void PathfinderProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint16LE(_targetItem);
	ws->writeUint16LE(static_cast<uint16>(_target.x));
	ws->writeUint16LE(static_cast<uint16>(_target.y));
	ws->writeUint16LE(static_cast<uint16>(_target.z));
	ws->writeByte(_hitMode ? 1 : 0);
	ws->writeUint16LE(static_cast<uint16>(_currentStep));

	ws->writeUint16LE(static_cast<uint16>(_path.size()));
	for (unsigned int i = 0; i < _path.size(); ++i) {
		ws->writeUint16LE(static_cast<uint16>(_path[i]._action));
		ws->writeUint16LE(static_cast<uint16>(Direction_ToUsecodeDir(_path[i]._direction)));
	}
}

bool PathfinderProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	// Type previously missing from constructor
	if (_type == 0) {
		_type = PATHFINDER_PROC_TYPE;
	}

	_targetItem = rs->readUint16LE();
	_target.x = rs->readUint16LE();
	_target.y = rs->readUint16LE();
	_target.z = rs->readUint16LE();
	_hitMode = (rs->readByte() != 0);
	_currentStep = rs->readUint16LE();

	unsigned int pathsize = rs->readUint16LE();
	_path.resize(pathsize);
	for (unsigned int i = 0; i < pathsize; ++i) {
		_path[i]._action = static_cast<Animation::Sequence>(rs->readUint16LE());
		_path[i]._direction = Direction_FromUsecodeDir(rs->readUint16LE());
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
