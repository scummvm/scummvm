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

#include "ultima8/misc/pent_include.h"
#include "AnimationTracker.h"

#include "ultima8/games/game_data.h"
#include "ultima8/world/actors/actor.h"
#include "ultima8/world/world.h"
#include "ultima8/world/current_map.h"
#include "MainShapeArchive.h"
#include "ultima8/world/actors/AnimAction.h"
#include "ultima8/misc/direction.h"
#include "ultima8/graphics/shape_info.h"
#include "ultima8/usecode/UCList.h"
#include "LoopScript.h"
#include "ultima8/world/get_object.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"
#include "ultima8/kernel/core_app.h"

namespace Ultima8 {

//#define WATCHACTOR 1

#ifdef WATCHACTOR
static const int watchactor = WATCHACTOR;
#endif

AnimationTracker::AnimationTracker() {

}

AnimationTracker::~AnimationTracker() {

}

bool AnimationTracker::init(Actor *actor_, Animation::Sequence action_,
                            uint32 dir_, PathfindingState *state_) {
	assert(actor_);
	actor = actor_->getObjId();
	uint32 shape = actor_->getShape();
	animaction = GameData::get_instance()->getMainShapes()->
	             getAnim(shape, action_);
	if (!animaction) return false;

	dir = dir_;

	if (state_ == 0) {
		animaction->getAnimRange(actor_, dir, startframe, endframe);
		actor_->getLocation(x, y, z);
		flipped = (actor_->getFlags() & Item::FLG_FLIPPED) != 0;
		firststep = (actor_->getActorFlags() & Actor::ACT_FIRSTSTEP) != 0;
	} else {
		animaction->getAnimRange(state_->lastanim, state_->direction,
		                         state_->firststep, dir, startframe, endframe);
		flipped = state_->flipped;
		firststep = state_->firststep;
		x = state_->x;
		y = state_->y;
		z = state_->z;
	}
	startx = x;
	starty = y;
	startz = z;

#ifdef WATCHACTOR
	if (actor_ && actor_->getObjId() == watchactor) {
		pout << "AnimationTracker: playing " << startframe << "-" << endframe
		     << " (animaction flags: " << std::hex << animaction->flags
		     << std::dec << ")" << std::endl;

	}
#endif

	firstframe = true;

	done = false;
	blocked = false;
	unsupported = false;
	hitobject = 0;
	mode = NormalMode;

	return true;
}

unsigned int AnimationTracker::getNextFrame(unsigned int frame) {
	frame++;

	if (frame == endframe)
		return endframe;

	// loop if necessary
	if (frame >= animaction->size) {
		if (animaction->flags & (AnimAction::AAF_LOOPING |
		                         AnimAction::AAF_LOOPING2)) {
			// CHECKME: unknown flag
			frame = 1;
		} else {
			frame = 0;
		}
	}

	return frame;
}

bool AnimationTracker::stepFrom(int32 x_, int32 y_, int32 z_) {
	x = x_;
	y = y_;
	z = z_;

	return step();
}

void AnimationTracker::evaluateMaxAnimTravel(int32 &max_endx, int32 &max_endy, uint32 dir) {
	max_endx = x;
	max_endy = y;

	if (done) return;

	Actor *a = getActor(actor);
	assert(a);

	unsigned int testframe;
	if (firstframe)
		testframe = startframe;
	else
		testframe = getNextFrame(currentframe);

	for (;;) {
		AnimFrame &f = animaction->frames[dir][testframe];
		// determine movement for this frame
		int32 dx = 4 * x_fact[dir] * f.deltadir;
		int32 dy = 4 * y_fact[dir] * f.deltadir;
		max_endx += dx;
		max_endy += dy;
		testframe = getNextFrame(testframe);
		if (testframe == endframe)
			return;
	}
}

bool AnimationTracker::step() {
	if (done) return false;

	Actor *a = getActor(actor);
	assert(a);

	if (firstframe)
		currentframe = startframe;
	else
		currentframe = getNextFrame(currentframe);

	if (currentframe == endframe) {
		done = true;

		// toggle ACT_FIRSTSTEP flag if necessary
		if (animaction->flags & AnimAction::AAF_TWOSTEP)
			firststep = !firststep;
		else
			firststep = true;

		return false;
	}

	prevx = x;
	prevy = y;
	prevz = z;

	// reset status flags
	unsupported = false;
	blocked = false;


	firstframe = false;

	AnimFrame &f = animaction->frames[dir][currentframe];

	shapeframe = f.frame;
	flipped = f.is_flipped();

	// determine movement for this frame
	int32 dx = 4 * x_fact[dir] * f.deltadir;
	int32 dy = 4 * y_fact[dir] * f.deltadir;
	int32 dz = f.deltaz;

	if (mode == TargetMode && !(f.flags & AnimFrame::AFF_ONGROUND)) {
		dx += target_dx / target_offground_left;
		dy += target_dy / target_offground_left;
		dz += target_dz / target_offground_left;

		target_dx -= target_dx / target_offground_left;
		target_dy -= target_dy / target_offground_left;
		target_dz -= target_dz / target_offground_left;

		--target_offground_left;
	}

	// determine footpad
	bool actorflipped = (a->getFlags() & Item::FLG_FLIPPED) != 0;
	int32 xd, yd, zd;
	a->getFootpadWorld(xd, yd, zd);
	if (actorflipped != flipped) {
		int32 t = xd;
		xd = yd;
		yd = t;
	}
	CurrentMap *cm = World::get_instance()->getCurrentMap();

	// TODO: check if this step is allowed
	// * can move?
	//   if not:
	//     - try to step up a bit
	//     - try to shift left/right a bit
	//     CHECKME: how often can we do these minor adjustments?
	//     CHECKME: for which animation types can we do them?
	//   if still fails: blocked
	// * if ONGROUND
	//     - is supported if ONGROUND?
	//       if not:
	//         * try to step down a bit
	//         * try to shift left/right a bit
	//       if still fails: unsupported
	//     - if supported by non-land item: unsupported

	// It might be worth it creating a 'scanForValidPosition' function
	// (in CurrentMap maybe) that scans a small area around the given
	// coordinates for a valid position (with 'must be supported' as a flag).
	// Note that it should only check in directions orthogonal to the movement
	// direction (to prevent it becoming impossible to step off a ledge).

	// I seem to recall that the teleporter from the Upper Catacombs teleporter
	// to the Upper Catacombs places you inside the floor. Using this
	// scanForValidPosition after a teleport would work around that problem.

	int32 tx, ty, tz;
	tx = x + dx;
	ty = y + dy;
	tz = z + dz;

	// Only for particularly large steps we do a full sweepTest
	if (abs(dx) >= xd - 8 || abs(dy) >= yd - 8 || abs(dz) >= zd - 8) {

		int32 start[3] = { x, y, z };
		int32 end[3] = { tx, ty, tz };
		int32 dims[3] = { xd, yd, zd };

		// Do the sweep test
		std::list<CurrentMap::SweepItem> collisions;
		std::list<CurrentMap::SweepItem>::iterator it;
		cm->sweepTest(start, end, dims, a->getShapeInfo()->flags, a->getObjId(),
		              false, &collisions);


		for (it = collisions.begin(); it != collisions.end(); it++) {
			// hit something, can't move
			if (!it->touching && it->blocking) {
#ifdef WATCHACTOR
				if (a->getObjId() == watchactor) {
					pout << "AnimationTracker: did sweepTest for large step; "
					     << "collision at time " << it->hit_time << std::endl;
				}
#endif
				blocked = true;
				it->GetInterpolatedCoords(end, start, end);
				x = end[0];
				y = end[1];
				z = end[2];
				return false;
			}
		}

		// If it succeeded, we proceed as usual
	}

	Item *support;
	bool targetok = cm->isValidPosition(tx, ty, tz,
	                                    startx, starty, startz,
	                                    xd, yd, zd,
	                                    a->getShapeInfo()->flags,
	                                    actor, &support, 0);

	if (GAME_IS_U8 && targetok && support) {
		// Might need to check for bridge traversal adjustments
		uint32 supportshape = support->getShape();
		if (supportshape >= 675 && supportshape <= 681) {
			// Could be a sloping portion of a bridge.  For a bridge along the
			// X axis, positive descent delta is a positive change in Y when
			// moving to higher X (left to right).  Units are 60x the needed
			// dy/dx
			int descentdelta = 0;
			if (supportshape == 675)
				descentdelta = -20;         // Descend
			else if (supportshape == 676)
				descentdelta = 12;          // Ascend
			else if (supportshape == 681)
				descentdelta = -20;         // Descend

			if (descentdelta) {
				if (dy == 0 && dx != 0 && !(support->getFlags() & Item::FLG_FLIPPED)) {
					// Moving left or right on horizontal bridge
					// descentdelta = 60*dy/dx
					// 60*dy = descentdelta * dx
					// dy = descentdelta * dx / 60;
					ty += descentdelta * dx / 60;
				} else if (dx == 0 && dy != 0 && (support->getFlags() & Item::FLG_FLIPPED)) {
					// Moving up or down on vertical bridge
					tx += descentdelta * dy / 60;
				}
			}
		}
	}

	if (!targetok || ((f.flags & AnimFrame::AFF_ONGROUND) && !support)) {

		// If on ground, try to adjust properly
		// TODO: Profile the effect of disabling this for pathfinding.
		//       It shouldn't be necessary in that case, and may provide a
		//       worthwhile speed-up.
		if ((f.flags & AnimFrame::AFF_ONGROUND) && zd > 8) {
			targetok = cm->scanForValidPosition(tx, ty, tz, a, dir,
			                                    true, tx, ty, tz);

			if (!targetok) {
				blocked = true;
				return false;
			} else {
#ifdef WATCHACTOR
				if (a->getObjId() == watchactor) {
					pout << "AnimationTracker: adjusted step: "
					     << tx - (x + dx) << "," << ty - (y + dy) << "," << tz - (z + dz)
					     << std::endl;
				}
#endif
			}
		} else {
			if (!targetok) {
				blocked = true;
				return false;
			}
		}
	}

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor) {
		pout << "AnimationTracker: step (" << tx - x << "," << ty - y
		     << "," << tz - z << ")" << std::endl;
	}
#endif

	x = tx;
	y = ty;
	z = tz;


	// if attack animation, see if we hit something
	if ((animaction->flags & AnimAction::AAF_ATTACK) &&
	        (hitobject == 0) && f.attack_range() > 0) {
		checkWeaponHit();
	}

	if (f.flags & AnimFrame::AFF_ONGROUND) {
		// needs support

		/*bool targetok = */ cm->isValidPosition(tx, ty, tz,
		        startx, starty, startz,
		        xd, yd, zd,
		        a->getShapeInfo()->flags,
		        actor, &support, 0);


		if (!support) {
			unsupported = true;
			return false;
		} else {
#if 0
			// This check causes really weird behaviour when fall()
			// doesn't make things fall off non-land items, so disabled for now

			Item *supportitem = getItem(support);
			assert(supportitem);
			if (!supportitem->getShapeInfo()->is_land()) {
//				pout << "Not land: "; supportitem->dumpInfo();
				// invalid support
				unsupported = true;
				return false;
			}
#endif
		}
	}

	return true;
}

AnimFrame *AnimationTracker::getAnimFrame() {
	return &animaction->frames[dir][currentframe];
}

void AnimationTracker::setTargetedMode(int32 x_, int32 y_, int32 z_) {
	unsigned int i;
	int totaldir = 0;
	int totalz = 0;
	int offGround = 0;
	int32 end_dx, end_dy, end_dz;

	for (i = startframe; i != endframe; i = getNextFrame(i)) {
		AnimFrame &f = animaction->frames[dir][i];
		totaldir += f.deltadir;  // This line sometimes seg faults.. ????
		totalz += f.deltaz;
		if (!(f.flags & AnimFrame::AFF_ONGROUND))
			++offGround;
	}

	end_dx = 4 * x_fact[dir] * totaldir;
	end_dy = 4 * y_fact[dir] * totaldir;
	end_dz = totalz;

	if (offGround) {
		mode = TargetMode;
		target_offground_left = offGround;
		target_dx = x_ - x - end_dx;
		target_dy = y_ - y - end_dy;
		target_dz = z_ - z - end_dz;

		// Don't allow large changes in Z
		if (target_dz > 16)
			target_dz = 16;
		if (target_dz < -16)
			target_dz = -16;

	}

}

void AnimationTracker::checkWeaponHit() {
	int range = animaction->frames[dir][currentframe].attack_range();

	Actor *a = getActor(actor);
	assert(a);


	Pentagram::Box abox = a->getWorldBox();
	abox.MoveAbs(x, y, z);
	abox.MoveRel(x_fact[dir] * 32 * range, y_fact[dir] * 32 * range, 0);

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor) {
		pout << "AnimationTracker: Checking hit, range " << range << ", box "
		     << abox.x << "," << abox.y << "," << abox.z << "," << abox.xd
		     << "," << abox.yd << "," << abox.zd << ": ";
	}
#endif

	CurrentMap *cm = World::get_instance()->getCurrentMap();

	UCList itemlist(2);
	LOOPSCRIPT(script, LS_TOKEN_END);

	cm->areaSearch(&itemlist, script, sizeof(script), 0, 320, false, x, y);

	ObjId hit = 0;
	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		ObjId itemid = itemlist.getuint16(i);
		if (itemid == actor) continue; // don't want to hit self

		Actor *item = getActor(itemid);
		if (!item) continue;

		Pentagram::Box ibox = item->getWorldBox();

		if (abox.Overlaps(ibox)) {
			hit = itemid;
#ifdef WATCHACTOR
			if (a->getObjId() == watchactor) {
				pout << "hit: ";
				item->dumpInfo();
			}
#endif
			break;
		}
	}

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor && !hit) {
		pout << "nothing" << std::endl;
	}
#endif

	hitobject = hit;
}

void AnimationTracker::updateState(PathfindingState &state) {
	state.x = x;
	state.y = y;
	state.z = z;
	state.flipped = flipped;
	state.firststep = firststep;
}


void AnimationTracker::updateActorFlags() {
	Actor *a = getActor(actor);
	assert(a);

	if (flipped)
		a->setFlag(Item::FLG_FLIPPED);
	else
		a->clearFlag(Item::FLG_FLIPPED);

	if (firststep)
		a->setActorFlag(Actor::ACT_FIRSTSTEP);
	else
		a->clearActorFlag(Actor::ACT_FIRSTSTEP);

	if (animaction) {
		bool hanging = (animaction->flags & AnimAction::AAF_HANGING) != 0;
		if (hanging)
			a->setFlag(Item::FLG_HANGING);
		else
			a->clearFlag(Item::FLG_HANGING);
	}

	if (currentframe != endframe)
		a->animframe = currentframe;
}

void AnimationTracker::getInterpolatedPosition(int32 &x_, int32 &y_,
        int32 &z_, int fc) {
	int32 dx = x - prevx;
	int32 dy = y - prevy;
	int32 dz = z - prevz;

	x_ = prevx + (dx * fc) / (animaction->framerepeat + 1);
	y_ = prevy + (dy * fc) / (animaction->framerepeat + 1);
	z_ = prevz + (dz * fc) / (animaction->framerepeat + 1);
}

void AnimationTracker::getSpeed(int32 &dx, int32 &dy, int32 &dz) {
	dx = x - prevx;
	dy = y - prevy;
	dz = z - prevz;
}


void AnimationTracker::save(ODataSource *ods) {
	ods->write4(startframe);
	ods->write4(endframe);
	uint8 ff = firstframe ? 1 : 0;
	ods->write1(ff);
	ods->write4(currentframe);

	ods->write2(actor);
	ods->write1(static_cast<uint8>(dir));

	if (animaction) {
		ods->write4(animaction->shapenum);
		ods->write4(animaction->action);
	} else {
		ods->write4(0);
		ods->write4(0);
	}

	ods->write4(static_cast<uint32>(prevx));
	ods->write4(static_cast<uint32>(prevy));
	ods->write4(static_cast<uint32>(prevz));
	ods->write4(static_cast<uint32>(x));
	ods->write4(static_cast<uint32>(y));
	ods->write4(static_cast<uint32>(z));

	ods->write2(static_cast<uint16>(mode));
	if (mode == TargetMode) {
		ods->write4(static_cast<uint32>(target_dx));
		ods->write4(static_cast<uint32>(target_dy));
		ods->write4(static_cast<uint32>(target_dz));
		ods->write4(static_cast<uint32>(target_offground_left));
	}
	uint8 fs = firststep ? 1 : 0;
	ods->write1(fs);
	uint8 fl = flipped ? 1 : 0;
	ods->write1(fl);
	ods->write4(shapeframe);

	uint8 flag = done ? 1 : 0;
	ods->write1(flag);
	flag = blocked ? 1 : 0;
	ods->write1(flag);
	flag = unsupported ? 1 : 0;
	ods->write1(flag);
	ods->write2(hitobject);
}

bool AnimationTracker::load(IDataSource *ids, uint32 version) {
	startframe = ids->read4();
	endframe = ids->read4();
	firstframe = (ids->read1() != 0);
	currentframe = ids->read4();

	actor = ids->read2();
	dir = ids->read1();

	uint32 shapenum = ids->read4();
	uint32 action = ids->read4();
	if (shapenum == 0) {
		animaction = 0;
	} else {
		animaction = GameData::get_instance()->getMainShapes()->
		             getAnim(shapenum, action);
		assert(animaction);
	}

	prevx = ids->read4();
	prevy = ids->read4();
	prevz = ids->read4();
	x = ids->read4();
	y = ids->read4();
	z = ids->read4();

	mode = static_cast<Mode>(ids->read2());
	if (mode == TargetMode) {
		target_dx = ids->read4();
		target_dy = ids->read4();
		if (version >= 5) {
			target_dz = ids->read4();
			target_offground_left = ids->read4();
		} else {
			// Versions before 5 stored the only x,y adjustment
			// to be made per frame. This is less accurate and ignores z.

			target_offground_left = 0;
			unsigned int i = currentframe;
			if (!firstframe) i = getNextFrame(i);

			for (; i != endframe; i = getNextFrame(i)) {
				AnimFrame &f = animaction->frames[dir][i];
				if (!(f.flags & AnimFrame::AFF_ONGROUND))
					++target_offground_left;
			}

			target_dx *= target_offground_left;
			target_dy *= target_offground_left;
			target_dz = 0;
		}
	}

	firststep = (ids->read1() != 0);
	flipped = (ids->read1() != 0);
	shapeframe = ids->read4();

	done = (ids->read1() != 0);
	blocked = (ids->read1() != 0);
	unsupported = (ids->read1() != 0);
	hitobject = ids->read2();

	return true;
}

} // End of namespace Ultima8
