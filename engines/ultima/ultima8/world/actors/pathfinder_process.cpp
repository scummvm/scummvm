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
#include "ultima/ultima8/world/actors/pathfinder_process.h"

#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/pathfinder.h"
#include "ultima/ultima8/world/get_object.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

static const unsigned int PATH_OK = 1;
static const unsigned int PATH_FAILED = 0;

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(PathfinderProcess, Process)

PathfinderProcess::PathfinderProcess() : Process() {

}

PathfinderProcess::PathfinderProcess(Actor *actor_, ObjId item_, bool hit) {
	assert(actor_);
	_itemNum = actor_->getObjId();
	_type = 0x0204; // CONSTANT !


	Item *item = getItem(item_);
	if (!item) {
		perr << "PathfinderProcess: non-existent target" << Std::endl;
		// can't get there...
		_result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	_currentStep = 0;
	_targetItem = item_;
	_hitMode = hit;
	assert(_targetItem);

	item->getLocation(_targetX, _targetY, _targetZ);

	Pathfinder pf;
	pf.init(actor_);
	pf.setTarget(item, hit);

	bool ok = pf.pathfind(_path);

	if (!ok) {
		// can't get there...
		debug(MM_INFO, "PathfinderProcess: actor %d failed to find path", _itemNum);
		_result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	// TODO: check if flag already set? kill other pathfinders?
	actor_->setActorFlag(Actor::ACT_PATHFINDING);
}

PathfinderProcess::PathfinderProcess(Actor *actor_,
                                     int32 x, int32 y, int32 z) {
	assert(actor_);
	_itemNum = actor_->getObjId();

	_targetX = x;
	_targetY = y;
	_targetZ = z;
	_targetItem = 0;

	_currentStep = 0;

	Pathfinder pf;
	pf.init(actor_);
	pf.setTarget(_targetX, _targetY, _targetZ);

	bool ok = pf.pathfind(_path);

	if (!ok) {
		// can't get there...
		debug(MM_INFO, "PathfinderProcess: actor %d failed to find path", _itemNum);
		_result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	// TODO: check if flag already set? kill other pathfinders?
	actor_->setActorFlag(Actor::ACT_PATHFINDING);
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
	if (!(actor->getFlags() & Item::FLG_FASTAREA)) return;


	bool ok = true;

	if (_targetItem) {
		int32 curx, cury, curz;
		Item *item = getItem(_targetItem);
		if (!item) {
			perr << "PathfinderProcess: target missing" << Std::endl;
			_result = PATH_FAILED;
			terminate();
			return;
		}

		item->getLocation(curx, cury, curz);
		if (ABS(curx - _targetX) >= 32 || ABS(cury - _targetY) >= 32 ||
		        ABS(curz - _targetZ) >= 8) {
			// target moved
			ok = false;
		}
	}

	if (ok && _currentStep >= _path.size()) {
		// done
#if 0
		pout << "PathfinderProcess: done" << Std::endl;
#endif
		_result = PATH_OK;
		terminate();
		return;
	}

	// try to take the next step

#if 0
	pout << "PathfinderProcess: trying step" << Std::endl;
#endif

	// if actor is still animating for whatever reason, wait until he stopped
	// FIXME: this should happen before the pathfinder is actually called,
	// since the running animation may move the actor, which could break
	// the found _path.
	if (actor->getActorFlags() & Actor::ACT_ANIMLOCK) {
		perr << "PathfinderProcess: ANIMLOCK, waiting" << Std::endl;
		return;
	}

	if (ok) {
		ok = actor->tryAnim(_path[_currentStep]._action,
		                    _path[_currentStep]._direction,
		                    _path[_currentStep]._steps) == Animation::SUCCESS;
	}

	if (!ok) {
#if 0
		pout << "PathfinderProcess: recalculating _path" << Std::endl;
#endif

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
				item->getLocation(_targetX, _targetY, _targetZ);
			}
		} else {
			pf.setTarget(_targetX, _targetY, _targetZ);
		}
		if (ok)
			ok = pf.pathfind(_path);

		_currentStep = 0;
		if (!ok) {
			// can't get there anymore
			debug(MM_INFO, "PathfinderProcess: actor %d failed to find path", _itemNum);
			_result = PATH_FAILED;
			terminate();
			return;
		}
	}

	if (_currentStep >= _path.size()) {
#if 0
		pout << "PathfinderProcess: done" << Std::endl;
#endif
		// done
		_result = PATH_OK;
		terminate();
		return;
	}

	uint16 animpid = actor->doAnim(_path[_currentStep]._action,
	                               _path[_currentStep]._direction,
	                               _path[_currentStep]._steps);
#if 0
	pout << "PathfinderProcess(" << getPid() << "): taking step "
	     << _path[_currentStep].action << "," << _path[_currentStep].direction
	     << " (animpid=" << animpid << ")" << Std::endl;
#endif
	_currentStep++;

	waitFor(animpid);
}

void PathfinderProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write2(_targetItem);
	ods->write2(static_cast<uint16>(_targetX));
	ods->write2(static_cast<uint16>(_targetY));
	ods->write2(static_cast<uint16>(_targetZ));
	ods->write1(_hitMode ? 1 : 0);
	ods->write2(static_cast<uint16>(_currentStep));

	ods->write2(static_cast<uint16>(_path.size()));
	for (unsigned int i = 0; i < _path.size(); ++i) {
		ods->write2(static_cast<uint16>(_path[i]._action));
		ods->write2(static_cast<uint16>(_path[i]._direction));
	}
}

bool PathfinderProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	_targetItem = ids->read2();
	_targetX = ids->read2();
	_targetY = ids->read2();
	_targetZ = ids->read2();
	_hitMode = (ids->read1() != 0);
	_currentStep = ids->read2();

	unsigned int pathsize = ids->read2();
	_path.resize(pathsize);
	for (unsigned int i = 0; i < pathsize; ++i) {
		_path[i]._action = static_cast<Animation::Sequence>(ids->read2());
		_path[i]._direction = ids->read2();
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
