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

#include "ultima/ultima8/world/actors/rolling_thunder_process.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/pathfinder.h"
#include "ultima/ultima8/world/actors/anim_action.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
#include "ultima/ultima8/gfx/anim_dat.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/misc/direction_util.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(RollingThunderProcess)

static const uint16 CRUSPID = 0x584;
static const uint16 BULLET_SPLASH_SHAPE = 0x1d9;

RollingThunderProcess::RollingThunderProcess() : Process(), _target(0), _timer(0) {
}

RollingThunderProcess::RollingThunderProcess(Actor *actor) : _target(0), _timer(0) {
	assert(actor);
	_itemNum = actor->getObjId();

	_type = 0x263; // CONSTANT!
}

void RollingThunderProcess::run() {
	Actor *actor = getActor(_itemNum);

	if (!actor || actor->isDead()) {
		// gone! maybe dead..
		terminate();
		return;
	}

	if (actor->isBusy()) {
		sleepFor60Ticks();
		return;
	}

	uint16 controllednpc = World::get_instance()->getControlledNPCNum();
	const Item *target = getItem(_target);

	// Target the controlled npc, unless our current target is a spider bomb
	if (_target != controllednpc && (!target || target->getShape() != CRUSPID)) {
		_target = controllednpc ? controllednpc : 1;
		target = getItem(_target);
	}

	const Actor *targeta = dynamic_cast<const Actor *>(target);
	if (targeta && targeta->isDead()) {
		_target = controllednpc;
		sleepFor60Ticks();
		return;
	}

	if (!actor->isPartlyOnScreen()) {
		sleepFor60Ticks();
		return;
	}

	// Should end up with some target here??
	if (!target)
		return;

	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	Animation::Sequence anim = rs.getRandomBit() ? Animation::combatRollLeft : Animation::combatRollRight;

	Direction actordir = actor->getDir();
	Direction outdir = actordir;
	bool canroll = checkDir(anim, outdir);

	if (!canroll) {
		// try the other way
		if (anim == Animation::combatRollLeft)
			anim = Animation::combatRollRight;
		else
			anim = Animation::combatRollLeft;

		canroll = checkDir(anim, outdir);
	}

	if (!canroll) {
		Point3 pta = actor->getLocation();
		Point3 ptt = target->getLocation();
		Direction dirtotarget = Direction_GetWorldDir(ptt.y - pta.y, ptt.x - pta.x, dirmode_16dirs);

		if (dirtotarget == actordir) {
			uint32 now = Kernel::get_instance()->getTickNum();
			if (now - actor->getLastTickWasHit() >= 120) {
				if (actor->fireDistance(target, dirtotarget, 0, 0, 0)) {
					actor->doAnim(Animation::attack, dir_current);
					return;
				}
			}
			checkForSpiderBomb();
		} else {
			uint16 turnproc = actor->turnTowardDir(dirtotarget);
			waitFor(turnproc);
		}
	} else {
		uint16 animpid = actor->doAnim(anim, dir_current);
		if (outdir != actordir) {
			animpid = actor->turnTowardDir(outdir, animpid);
		}
		int attackcount = rs.getRandomNumberRng(1, 3);
		for (int i = 0; i < attackcount; i++) {
			animpid = actor->doAnimAfter(Animation::attack, outdir, animpid);
		}
		Animation::Sequence rollback;
		if (anim == Animation::combatRollLeft) {
			rollback = Animation::combatRollRight;
		} else {
			rollback = Animation::combatRollLeft;
		}
		animpid = actor->doAnimAfter(rollback, dir_current, animpid);
		waitFor(animpid);
	}
}


bool RollingThunderProcess::checkDir(Animation::Sequence anim, Direction &outdir) const {
	Actor *actor = getActor(_itemNum);
	Direction curdir = actor->getDir();
	if (!actor->isPartlyOnScreen())
		return false;

	const Item *target = getItem(_target);
	if (!target)
		return false;

	PathfindingState state;
	state.load(actor);

	// Check if the anim is blocked or would take the actor off-screen.
	Animation::Result animresult = actor->tryAnim(anim, dir_current, 0, &state);

	if (animresult == Animation::FAILURE || !actor->isPartlyOnScreen())
		return false;

	// check if the dir to the target is within 2 direction steps of the current dir
	Point3 pt = target->getLocation();
	Direction dirtotarget = Direction_GetWorldDir(pt.y - state._point.y, pt.x - state._point.x, dirmode_16dirs);

	static const int DIROFFSETS[] = {0, -1, 1, -2, 2};

	outdir = dirtotarget;

	// Check that the target is in a nearby direction
	bool nearby = false;
	for (int i = 0; i < ARRAYSIZE(DIROFFSETS); i++) {
		Direction dir = Direction_TurnByDelta(dirtotarget, DIROFFSETS[i], dirmode_16dirs);
		if (curdir == dir) {
			nearby = true;
			break;
		}
	}
	if (!nearby)
		return false;

	// Check whether we can fire in that direction and hit the target
	for (int i = 0; i < ARRAYSIZE(DIROFFSETS); i++) {
		Direction dir = Direction_TurnByDelta(dirtotarget, DIROFFSETS[i], dirmode_16dirs);
		if (fireDistance(dir, state._point.x, state._point.y, state._point.z))
			return true;
	}

	return false;
}


//
// This is practically a copy of Item::fireDistance, but with some changes
// to measure from the hypothetical position of the actor after rolling.
//
// Ideally it would be refactored, but for now copy it with changes just like
// the game does.
//
bool RollingThunderProcess::fireDistance(Direction dir, int32 x, int32 y, int32 z) const {
	int32 xoff = 0;
	int32 yoff = 0;
	int32 zoff = 0;
	int32 xoff2 = 0;
	int32 yoff2 = 0;
	int32 zoff2 = 0;

	const Actor *actor = getActor(_itemNum);
	const Item *target = getItem(_target);

	if (!actor || !target)
		return 0;

	Point3 pt = target->getLocation();

	uint16 shapeno = actor->getShape();
	uint32 actionno = AnimDat::getActionNumberForSequence(Animation::attack, actor);
	const AnimAction *animaction = GameData::get_instance()->getMainShapes()->getAnim(shapeno, actionno);

	CurrentMap *cm = World::get_instance()->getCurrentMap();

	bool other_offsets = false;
	bool first_offsets = false;
	int nframes = animaction->getSize();
	for (int frameno = 0; frameno < nframes; frameno++) {
		const AnimFrame &frame = animaction->getFrame(dir, frameno);
		if (frame.is_cruattack()) {
			if (!first_offsets) {
				xoff = frame.cru_attackx();
				yoff = frame.cru_attacky();
				zoff = frame.cru_attackz();
				first_offsets = true;
			} else {
				xoff2 = frame.cru_attackx();
				yoff2 = frame.cru_attacky();
				zoff2 = frame.cru_attackz();
				other_offsets = true;
				break;
			}
		}
	}

	if (!first_offsets)
		return 0;

	int dist = 0;
	for (int i = 0; i < (other_offsets ? 2 : 1) && dist == 0; i++) {
		int32 cx = x + (i == 0 ? xoff : xoff2);
		int32 cy = y + (i == 0 ? yoff : yoff2);
		int32 cz = z + (i == 0 ? zoff : zoff2);

		PositionInfo info = cm->getPositionInfo(cx, cy, cz, BULLET_SPLASH_SHAPE, _itemNum);
		if (!info.valid && info.blocker) {
			if (info.blocker->getObjId() == target->getObjId())
				dist = MAX(abs(x - pt.x), abs(y - pt.y));
		} else {
			Point3 oc = target->getCentre();
			oc.z = target->getTargetZRelativeToAttackerZ(z);
			const Point3 start(cx, cy, cz);
			const Point3 end = oc;
			const int32 dims[3] = {2, 2, 2};

			Std::list<CurrentMap::SweepItem> collisions;
			cm->sweepTest(start, end, dims, ShapeInfo::SI_SOLID,
						 _itemNum, false, &collisions);
			for (const auto &collision : collisions) {
				if (collision._item == _itemNum)
					continue;
				if (collision._item != target->getObjId())
					break;
				Point3 out = collision.GetInterpolatedCoords(start, end);
				dist = MAX(abs(x - out.x), abs(y - out.y));
				break;
			}
		}
	}
	return dist;
}

bool RollingThunderProcess::checkForSpiderBomb() {
	const Item *target = getItem(_target);
	const Actor *actor = getActor(_itemNum);

	if (target && target->getShape() == CRUSPID)
		return false;
	if (!checkTimer())
		return false;

	CurrentMap *currentmap = World::get_instance()->getCurrentMap();
	UCList spiderlist(2);
	LOOPSCRIPT(script, LS_SHAPE_EQUAL(CRUSPID));
	currentmap->areaSearch(&spiderlist, script, sizeof(script), actor, 800, false);

	for (unsigned int i = 0; i < spiderlist.getSize(); ++i) {
		const Item *spider = getItem(spiderlist.getuint16(i));
		if (!spider)
			continue;
		Point3 pta = actor->getLocation();
		Point3 pts = spider->getLocation();
		Direction dirtospider = Direction_GetWorldDir(pts.y - pta.y, pts.x - pta.x, dirmode_16dirs);
		uint16 dist = actor->fireDistance(spider, dirtospider, 0, 0, 0);
		if (dist > 0) {
			_target = spider->getObjId();
			return true;
		}
	}

	return false;
}

bool RollingThunderProcess::checkTimer() {
	uint32 ticksnow = Kernel::get_instance()->getTickNum();
	if (ticksnow > _timer + 90) {
		_timer = ticksnow;
		return true;
	}
	return false;
}

void RollingThunderProcess::sleepFor60Ticks() {
	Process *wait = new DelayProcess(60);
	Kernel::get_instance()->addProcess(wait);
	waitFor(wait);
}

void RollingThunderProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
	ws->writeUint16LE(_target);
	ws->writeUint32LE(_timer);
}

bool RollingThunderProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;
	_target = rs->readUint16LE();
	_timer = rs->readUint32LE();
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
