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
#include "ultima/ultima8/world/actors/animation_tracker.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/world/actors/anim_action.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/kernel/core_app.h"

namespace Ultima {
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
	_actor = actor_->getObjId();
	uint32 shape = actor_->getShape();
	_animAction = GameData::get_instance()->getMainShapes()->
	             getAnim(shape, action_);
	if (!_animAction) return false;

	_dir = dir_;

	if (state_ == 0) {
		_animAction->getAnimRange(actor_, _dir, _startFrame, _endFrame);
		actor_->getLocation(_x, _y, _z);
		_flipped = (actor_->getFlags() & Item::FLG_FLIPPED) != 0;
		_firstStep = (actor_->getActorFlags() & Actor::ACT_FIRSTSTEP) != 0;
	} else {
		_animAction->getAnimRange(state_->_lastAnim, state_->_direction,
		                         state_->_firstStep, _dir, _startFrame, _endFrame);
		_flipped = state_->_flipped;
		_firstStep = state_->_firstStep;
		_x = state_->_x;
		_y = state_->_y;
		_z = state_->_z;
	}
	_startX = _x;
	_startY = _y;
	_startZ = _z;

#ifdef WATCHACTOR
	if (actor_ && actor_->getObjId() == watchactor) {
		pout << "AnimationTracker: playing " << _startFrame << "-" << _endFrame
		     << " (_animAction flags: " << Std::hex << _animAction->flags
		     << Std::dec << ")" << Std::endl;

	}
#endif

	_firstFrame = true;

	_done = false;
	_blocked = false;
	_unsupported = false;
	_hitObject = 0;
	_mode = NormalMode;

	return true;
}

unsigned int AnimationTracker::getNextFrame(unsigned int frame) const {
	frame++;

	if (frame == _endFrame)
		return _endFrame;

	// loop if necessary
	if (frame >= _animAction->_size) {
		if (_animAction->_flags & (AnimAction::AAF_LOOPING |
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
	_x = x_;
	_y = y_;
	_z = z_;

	return step();
}

void AnimationTracker::evaluateMaxAnimTravel(int32 &max_endx, int32 &max_endy, uint32 dir_) {
	max_endx = _x;
	max_endy = _y;

	if (_done) return;

	Actor *a = getActor(_actor);
	assert(a);

	unsigned int testframe;
	if (_firstFrame)
		testframe = _startFrame;
	else
		testframe = getNextFrame(_currentFrame);

	for (;;) {
		AnimFrame &f = _animAction->frames[dir_][testframe];
		// determine movement for this frame
		int32 dx = 4 * x_fact[dir_] * f._deltaDir;
		int32 dy = 4 * y_fact[dir_] * f._deltaDir;
		max_endx += dx;
		max_endy += dy;
		testframe = getNextFrame(testframe);
		if (testframe == _endFrame)
			return;
	}
}

bool AnimationTracker::step() {
	if (_done) return false;

	Actor *a = getActor(_actor);
	assert(a);

	if (_firstFrame)
		_currentFrame = _startFrame;
	else
		_currentFrame = getNextFrame(_currentFrame);

	if (_currentFrame == _endFrame) {
		_done = true;

		// toggle ACT_FIRSTSTEP flag if necessary
		if (_animAction->_flags & AnimAction::AAF_TWOSTEP)
			_firstStep = !_firstStep;
		else
			_firstStep = true;

		return false;
	}

	_prevX = _x;
	_prevY = _y;
	_prevZ = _z;

	// reset status flags
	_unsupported = false;
	_blocked = false;


	_firstFrame = false;

	AnimFrame &f = _animAction->frames[_dir][_currentFrame];

	_shapeFrame = f._frame;
	_flipped = f.is_flipped();

	// determine movement for this frame
	int32 dx = 4 * x_fact[_dir] * f._deltaDir;
	int32 dy = 4 * y_fact[_dir] * f._deltaDir;
	int32 dz = f._deltaZ;

	if (_mode == TargetMode && !(f._flags & AnimFrame::AFF_ONGROUND)) {
		dx += _targetDx / _targetOffGroundLeft;
		dy += _targetDy / _targetOffGroundLeft;
		dz += _targetDz / _targetOffGroundLeft;

		_targetDx -= _targetDx / _targetOffGroundLeft;
		_targetDy -= _targetDy / _targetOffGroundLeft;
		_targetDz -= _targetDz / _targetOffGroundLeft;

		--_targetOffGroundLeft;
	}

	// determine footpad
	bool actorflipped = (a->getFlags() & Item::FLG_FLIPPED) != 0;
	int32 xd, yd, zd;
	a->getFootpadWorld(xd, yd, zd);
	if (actorflipped != _flipped) {
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
	//   if still fails: _blocked
	// * if ONGROUND
	//     - is supported if ONGROUND?
	//       if not:
	//         * try to step down a bit
	//         * try to shift left/right a bit
	//       if still fails: _unsupported
	//     - if supported by non-land item: _unsupported

	// It might be worth it creating a 'scanForValidPosition' function
	// (in CurrentMap maybe) that scans a small area around the given
	// coordinates for a valid position (with 'must be supported' as a flag).
	// Note that it should only check in directions orthogonal to the movement
	// _direction (to prevent it becoming impossible to step off a ledge).

	// I seem to recall that the teleporter from the Upper Catacombs teleporter
	// to the Upper Catacombs places you inside the floor. Using this
	// scanForValidPosition after a teleport would work around that problem.

	int32 tx, ty, tz;
	tx = _x + dx;
	ty = _y + dy;
	tz = _z + dz;

	// Only for particularly large steps we do a full sweepTest
	if (ABS(dx) >= xd - 8 || ABS(dy) >= yd - 8 || ABS(dz) >= zd - 8) {

		int32 start[3] = { _x, _y, _z };
		int32 end[3] = { tx, ty, tz };
		int32 dims[3] = { xd, yd, zd };

		// Do the sweep test
		Std::list<CurrentMap::SweepItem> collisions;
		Std::list<CurrentMap::SweepItem>::iterator it;
		cm->sweepTest(start, end, dims, a->getShapeInfo()->_flags, a->getObjId(),
		              false, &collisions);


		for (it = collisions.begin(); it != collisions.end(); it++) {
			// hit something, can't move
			if (!it->_touching && it->_blocking) {
#ifdef WATCHACTOR
				if (a->getObjId() == watchactor) {
					pout << "AnimationTracker: did sweepTest for large step; "
					     << "collision at time " << it->hit_time << Std::endl;
				}
#endif
				_blocked = true;
				it->GetInterpolatedCoords(end, start, end);
				_x = end[0];
				_y = end[1];
				_z = end[2];
				return false;
			}
		}

		// If it succeeded, we proceed as usual
	}

	const Item *support;
	bool targetok = cm->isValidPosition(tx, ty, tz,
	                                    _startX, _startY, _startZ,
	                                    xd, yd, zd,
	                                    a->getShapeInfo()->_flags,
	                                    _actor, &support, 0);

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

	if (!targetok || ((f._flags & AnimFrame::AFF_ONGROUND) && !support)) {

		// If on ground, try to adjust properly
		// TODO: Profile the effect of disabling this for pathfinding.
		//       It shouldn't be necessary in that case, and may provide a
		//       worthwhile speed-up.
		if ((f._flags & AnimFrame::AFF_ONGROUND) && zd > 8) {
			targetok = cm->scanForValidPosition(tx, ty, tz, a, _dir,
			                                    true, tx, ty, tz);

			if (!targetok) {
				_blocked = true;
				return false;
			} else {
#ifdef WATCHACTOR
				if (a->getObjId() == watchactor) {
					pout << "AnimationTracker: adjusted step: "
					     << tx - (_x + dx) << "," << ty - (_y + dy) << "," << tz - (_z + dz)
					     << Std::endl;
				}
#endif
			}
		} else {
			if (!targetok) {
				_blocked = true;
				return false;
			}
		}
	}

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor) {
		pout << "AnimationTracker: step (" << tx - _x << "," << ty - _y
		     << "," << tz - _z << ")" << Std::endl;
	}
#endif

	_x = tx;
	_y = ty;
	_z = tz;


	// if attack animation, see if we hit something
	if ((_animAction->_flags & AnimAction::AAF_ATTACK) &&
	        (_hitObject == 0) && f.attack_range() > 0) {
		checkWeaponHit();
	}

	if (f._flags & AnimFrame::AFF_ONGROUND) {
		// needs support

		/*bool targetok = */ cm->isValidPosition(tx, ty, tz,
		        _startX, _startY, _startZ,
		        xd, yd, zd,
		        a->getShapeInfo()->_flags,
		        _actor, &support, 0);


		if (!support) {
			_unsupported = true;
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
				_unsupported = true;
				return false;
			}
#endif
		}
	}

	return true;
}

AnimFrame *AnimationTracker::getAnimFrame() const {
	return &_animAction->frames[_dir][_currentFrame];
}

void AnimationTracker::setTargetedMode(int32 x_, int32 y_, int32 z_) {
	unsigned int i;
	int totaldir = 0;
	int totalz = 0;
	int offGround = 0;
	int32 end_dx, end_dy, end_dz;

	for (i = _startFrame; i != _endFrame; i = getNextFrame(i)) {
		AnimFrame &f = _animAction->frames[_dir][i];
		totaldir += f._deltaDir;  // This line sometimes seg faults.. ????
		totalz += f._deltaZ;
		if (!(f._flags & AnimFrame::AFF_ONGROUND))
			++offGround;
	}

	end_dx = 4 * x_fact[_dir] * totaldir;
	end_dy = 4 * y_fact[_dir] * totaldir;
	end_dz = totalz;

	if (offGround) {
		_mode = TargetMode;
		_targetOffGroundLeft = offGround;
		_targetDx = x_ - _x - end_dx;
		_targetDy = y_ - _y - end_dy;
		_targetDz = z_ - _z - end_dz;

		// Don't allow large changes in Z
		if (_targetDz > 16)
			_targetDz = 16;
		if (_targetDz < -16)
			_targetDz = -16;

	}

}

void AnimationTracker::checkWeaponHit() {
	int range = _animAction->frames[_dir][_currentFrame].attack_range();

	const Actor *a = getActor(_actor);
	assert(a);


	Box abox = a->getWorldBox();
	abox.MoveAbs(_x, _y, _z);
	abox.MoveRel(x_fact[_dir] * 32 * range, y_fact[_dir] * 32 * range, 0);

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor) {
		pout << "AnimationTracker: Checking hit, range " << range << ", box "
		     << abox._x << "," << abox._y << "," << abox._z << "," << abox.xd
		     << "," << abox.yd << "," << abox.zd << ": ";
	}
#endif

	CurrentMap *cm = World::get_instance()->getCurrentMap();

	UCList itemlist(2);
	LOOPSCRIPT(script, LS_TOKEN_END);

	cm->areaSearch(&itemlist, script, sizeof(script), 0, 320, false, _x, _y);

	ObjId hit = 0;
	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		ObjId itemid = itemlist.getuint16(i);
		if (itemid == _actor) continue; // don't want to hit self

		Actor *item = getActor(itemid);
		if (!item) continue;

		Box ibox = item->getWorldBox();

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
		pout << "nothing" << Std::endl;
	}
#endif

	_hitObject = hit;
}

void AnimationTracker::updateState(PathfindingState &state) {
	state._x = _x;
	state._y = _y;
	state._z = _z;
	state._flipped = _flipped;
	state._firstStep = _firstStep;
}


void AnimationTracker::updateActorFlags() {
	Actor *a = getActor(_actor);
	assert(a);

	if (_flipped)
		a->setFlag(Item::FLG_FLIPPED);
	else
		a->clearFlag(Item::FLG_FLIPPED);

	if (_firstStep)
		a->setActorFlag(Actor::ACT_FIRSTSTEP);
	else
		a->clearActorFlag(Actor::ACT_FIRSTSTEP);

	if (_animAction) {
		bool hanging = (_animAction->_flags & AnimAction::AAF_HANGING) != 0;
		if (hanging)
			a->setFlag(Item::FLG_HANGING);
		else
			a->clearFlag(Item::FLG_HANGING);
	}

	if (_currentFrame != _endFrame)
		a->_animFrame = _currentFrame;
}

void AnimationTracker::getInterpolatedPosition(int32 &x_, int32 &y_,
                                               int32 &z_, int fc) const {
	int32 dx = _x - _prevX;
	int32 dy = _y - _prevY;
	int32 dz = _z - _prevZ;

	x_ = _prevX + (dx * fc) / (_animAction->_frameRepeat + 1);
	y_ = _prevY + (dy * fc) / (_animAction->_frameRepeat + 1);
	z_ = _prevZ + (dz * fc) / (_animAction->_frameRepeat + 1);
}

void AnimationTracker::getSpeed(int32 &dx, int32 &dy, int32 &dz) const {
	dx = _x - _prevX;
	dy = _y - _prevY;
	dz = _z - _prevZ;
}


void AnimationTracker::save(ODataSource *ods) {
	ods->write4(_startFrame);
	ods->write4(_endFrame);
	uint8 ff = _firstFrame ? 1 : 0;
	ods->write1(ff);
	ods->write4(_currentFrame);

	ods->write2(_actor);
	ods->write1(static_cast<uint8>(_dir));

	if (_animAction) {
		ods->write4(_animAction->_shapeNum);
		ods->write4(_animAction->_action);
	} else {
		ods->write4(0);
		ods->write4(0);
	}

	ods->write4(static_cast<uint32>(_prevX));
	ods->write4(static_cast<uint32>(_prevY));
	ods->write4(static_cast<uint32>(_prevZ));
	ods->write4(static_cast<uint32>(_x));
	ods->write4(static_cast<uint32>(_y));
	ods->write4(static_cast<uint32>(_z));

	ods->write2(static_cast<uint16>(_mode));
	if (_mode == TargetMode) {
		ods->write4(static_cast<uint32>(_targetDx));
		ods->write4(static_cast<uint32>(_targetDy));
		ods->write4(static_cast<uint32>(_targetDz));
		ods->write4(static_cast<uint32>(_targetOffGroundLeft));
	}
	uint8 fs = _firstStep ? 1 : 0;
	ods->write1(fs);
	uint8 fl = _flipped ? 1 : 0;
	ods->write1(fl);
	ods->write4(_shapeFrame);

	uint8 flag = _done ? 1 : 0;
	ods->write1(flag);
	flag = _blocked ? 1 : 0;
	ods->write1(flag);
	flag = _unsupported ? 1 : 0;
	ods->write1(flag);
	ods->write2(_hitObject);
}

bool AnimationTracker::load(IDataSource *ids, uint32 version) {
	_startFrame = ids->read4();
	_endFrame = ids->read4();
	_firstFrame = (ids->read1() != 0);
	_currentFrame = ids->read4();

	_actor = ids->read2();
	_dir = ids->read1();

	uint32 shapenum = ids->read4();
	uint32 action = ids->read4();
	if (shapenum == 0) {
		_animAction = nullptr;
	} else {
		_animAction = GameData::get_instance()->getMainShapes()->
		             getAnim(shapenum, action);
		assert(_animAction);
	}

	_prevX = ids->read4();
	_prevY = ids->read4();
	_prevZ = ids->read4();
	_x = ids->read4();
	_y = ids->read4();
	_z = ids->read4();

	_mode = static_cast<Mode>(ids->read2());
	if (_mode == TargetMode) {
		_targetDx = ids->read4();
		_targetDy = ids->read4();
		if (version >= 5) {
			_targetDz = ids->read4();
			_targetOffGroundLeft = ids->read4();
		} else {
			// Versions before 5 stored the only _x,_y adjustment
			// to be made per frame. This is less accurate and ignores _z.

			_targetOffGroundLeft = 0;
			unsigned int i = _currentFrame;
			if (!_firstFrame) i = getNextFrame(i);

			for (; i != _endFrame; i = getNextFrame(i)) {
				AnimFrame &f = _animAction->frames[_dir][i];
				if (!(f._flags & AnimFrame::AFF_ONGROUND))
					++_targetOffGroundLeft;
			}

			_targetDx *= _targetOffGroundLeft;
			_targetDy *= _targetOffGroundLeft;
			_targetDz = 0;
		}
	}

	_firstStep = (ids->read1() != 0);
	_flipped = (ids->read1() != 0);
	_shapeFrame = ids->read4();

	_done = (ids->read1() != 0);
	_blocked = (ids->read1() != 0);
	_unsupported = (ids->read1() != 0);
	_hitObject = ids->read2();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
