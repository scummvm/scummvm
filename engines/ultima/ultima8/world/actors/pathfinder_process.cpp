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
	item_num = actor_->getObjId();
	type = 0x0204; // CONSTANT !


	Item *item = getItem(item_);
	if (!item) {
		perr << "PathfinderProcess: non-existent target" << std::endl;
		// can't get there...
		result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	currentstep = 0;
	targetitem = item_;
	hitmode = hit;
	assert(targetitem);

	item->getLocation(targetx, targety, targetz);

	Pathfinder pf;
	pf.init(actor_);
	pf.setTarget(item, hit);

	bool ok = pf.pathfind(path);

	if (!ok) {
		perr << "PathfinderProcess: actor " << item_num
		     << " failed to find path" << std::endl;
		// can't get there...
		result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	// TODO: check if flag already set? kill other pathfinders?
	actor_->setActorFlag(Actor::ACT_PATHFINDING);
}

PathfinderProcess::PathfinderProcess(Actor *actor_,
                                     int32 x, int32 y, int32 z) {
	assert(actor_);
	item_num = actor_->getObjId();

	targetx = x;
	targety = y;
	targetz = z;
	targetitem = 0;

	currentstep = 0;

	Pathfinder pf;
	pf.init(actor_);
	pf.setTarget(targetx, targety, targetz);

	bool ok = pf.pathfind(path);

	if (!ok) {
		perr << "PathfinderProcess: actor " << item_num
		     << " failed to find path" << std::endl;
		// can't get there...
		result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	// TODO: check if flag already set? kill other pathfinders?
	actor_->setActorFlag(Actor::ACT_PATHFINDING);
}

PathfinderProcess::~PathfinderProcess() {

}

void PathfinderProcess::terminate() {
	Actor *actor = getActor(item_num);
	if (actor) {
		// TODO: only clear if it was set by us?
		// (slightly more complicated if we kill other pathfinders on startup)
		actor->clearActorFlag(Actor::ACT_PATHFINDING);
	}

	Process::terminate();
}

void PathfinderProcess::run() {
	Actor *actor = getActor(item_num);
	assert(actor);
	// if not in the fastarea, do nothing
	if (!(actor->getFlags() & Item::FLG_FASTAREA)) return;


	bool ok = true;

	if (targetitem) {
		int32 curx, cury, curz;
		Item *item = getItem(targetitem);
		if (!item) {
			perr << "PathfinderProcess: target missing" << std::endl;
			result = PATH_FAILED;
			terminate();
			return;
		}

		item->getLocation(curx, cury, curz);
		if (ABS(curx - targetx) >= 32 || ABS(cury - targety) >= 32 ||
		        ABS(curz - targetz) >= 8) {
			// target moved
			ok = false;
		}
	}

	if (ok && currentstep >= path.size()) {
		// done
#if 0
		pout << "PathfinderProcess: done" << std::endl;
#endif
		result = PATH_OK;
		terminate();
		return;
	}

	// try to take the next step

#if 0
	pout << "PathfinderProcess: trying step" << std::endl;
#endif

	// if actor is still animating for whatever reason, wait until he stopped
	// FIXME: this should happen before the pathfinder is actually called,
	// since the running animation may move the actor, which could break
	// the found path.
	if (actor->getActorFlags() & Actor::ACT_ANIMLOCK) {
		perr << "PathfinderProcess: ANIMLOCK, waiting" << std::endl;
		return;
	}

	if (ok) {
		ok = actor->tryAnim(path[currentstep].action,
		                    path[currentstep].direction,
		                    path[currentstep].steps) == Animation::SUCCESS;
	}

	if (!ok) {
#if 0
		pout << "PathfinderProcess: recalculating path" << std::endl;
#endif

		// need to redetermine path
		ok = true;
		Pathfinder pf;
		pf.init(actor);
		if (targetitem) {
			Item *item = getItem(targetitem);
			if (!item)
				ok = false;
			else {
				if (hitmode && !actor->isInCombat()) {
					// Actor exited combat mode
					hitmode = false;
				}
				pf.setTarget(item, hitmode);
				item->getLocation(targetx, targety, targetz);
			}
		} else {
			pf.setTarget(targetx, targety, targetz);
		}
		if (ok)
			ok = pf.pathfind(path);

		currentstep = 0;
		if (!ok) {
			perr << "PathfinderProcess: actor " << item_num
			     << " failed to find path" << std::endl;
			// can't get there anymore
			result = PATH_FAILED;
			terminate();
			return;
		}
	}

	if (currentstep >= path.size()) {
#if 0
		pout << "PathfinderProcess: done" << std::endl;
#endif
		// done
		result = PATH_OK;
		terminate();
		return;
	}

	uint16 animpid = actor->doAnim(path[currentstep].action,
	                               path[currentstep].direction,
	                               path[currentstep].steps);
#if 0
	pout << "PathfinderProcess(" << getPid() << "): taking step "
	     << path[currentstep].action << "," << path[currentstep].direction
	     << " (animpid=" << animpid << ")" << std::endl;
#endif
	currentstep++;

	waitFor(animpid);
}

void PathfinderProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write2(targetitem);
	ods->write2(static_cast<uint16>(targetx));
	ods->write2(static_cast<uint16>(targety));
	ods->write2(static_cast<uint16>(targetz));
	ods->write1(hitmode ? 1 : 0);
	ods->write2(static_cast<uint16>(currentstep));

	ods->write2(static_cast<uint16>(path.size()));
	for (unsigned int i = 0; i < path.size(); ++i) {
		ods->write2(static_cast<uint16>(path[i].action));
		ods->write2(static_cast<uint16>(path[i].direction));
	}
}

bool PathfinderProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	targetitem = ids->read2();
	targetx = ids->read2();
	targety = ids->read2();
	targetz = ids->read2();
	hitmode = (ids->read1() != 0);
	currentstep = ids->read2();

	unsigned int pathsize = ids->read2();
	path.resize(pathsize);
	for (unsigned int i = 0; i < pathsize; ++i) {
		path[i].action = static_cast<Animation::Sequence>(ids->read2());
		path[i].direction = ids->read2();
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
