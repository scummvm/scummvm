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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
#include "ultima/ultima8/gfx/anim_dat.h"
#include "ultima/ultima8/world/actors/anim_action.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/world/loop_script.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

//#define WATCHACTOR 1

#ifdef WATCHACTOR
static const int watchactor = WATCHACTOR;
#endif

AnimationTracker::AnimationTracker() : _firstFrame(true), _done(false),
	_blocked(false), _unsupported(false), _hitObject(0), _mode(NormalMode),
	_actor(0), _dir(dir_north), _animAction(nullptr),
	_prev(), _curr(), _start(),
	_targetDx(0), _targetDy(0), _targetDz(0), _targetOffGroundLeft(0),
	_firstStep(false), _shapeFrame(0), _currentFrame(0), _startFrame(0),
	_endFrame(0), _flipped(false) {
}

AnimationTracker::~AnimationTracker() {
}


bool AnimationTracker::init(const Actor *actor, Animation::Sequence action,
							Direction dir, const PathfindingState *state) {
	assert(actor);
	_actor = actor->getObjId();
	uint32 shape = actor->getShape();
	uint32 actionnum = AnimDat::getActionNumberForSequence(action, actor);
	_animAction = GameData::get_instance()->getMainShapes()->
	             getAnim(shape, actionnum);
	if (!_animAction) {
#ifdef WATCHACTOR
		if (actor && actor->getObjId() == watchactor) {
			debugC(kDebugActor, "AnimationTracker: no animation action %d for shape %d",
				  actionnum, shape);
		}
#endif
		return false;
	}

	_dir = dir;

	if (state == 0) {
		_animAction->getAnimRange(actor, _dir, _startFrame, _endFrame);
		_curr = actor->getLocation();
		_flipped = actor->hasFlags(Item::FLG_FLIPPED);
		_firstStep = actor->hasActorFlags(Actor::ACT_FIRSTSTEP);
	} else {
		_animAction->getAnimRange(state->_lastAnim, state->_direction,
		                         state->_firstStep, _dir, _startFrame, _endFrame);
		_flipped = state->_flipped;
		_firstStep = state->_firstStep;
		_curr = state->_point;
	}
	_start = _curr;

#ifdef WATCHACTOR
	if (actor && actor->getObjId() == watchactor) {
		debugC(kDebugActor, "AnimationTracker: playing action %d %d-%d (animAction flags: 0x04%x)",
			  actionnum, _startFrame, _endFrame, _animAction->getFlags());
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

	if (!_animAction || frame == _endFrame)
		return _endFrame;

	// loop if necessary
	if (frame >= _animAction->getSize()) {
		if (_animAction->hasFlags(AnimAction::AAF_LOOPING)) {
			// CHECKME: unknown flag
			frame = 1;
		} else {
			frame = 0;
		}
	}

	return frame;
}

bool AnimationTracker::stepFrom(const Point3 &pt) {
	_curr = pt;

	return step();
}

void AnimationTracker::evaluateMaxAnimTravel(int32 &max_endx, int32 &max_endy, Direction dir) {
	max_endx = _curr.x;
	max_endy = _curr.y;

	if (_done) return;

	Actor *a = getActor(_actor);
	assert(a);

	unsigned int testframe;
	if (_firstFrame)
		testframe = _startFrame;
	else
		testframe = getNextFrame(_currentFrame);

	for (;;) {
		const AnimFrame &f = _animAction->getFrame(dir, testframe);
		// determine movement for this frame
		int32 dx = 4 * Direction_XFactor(dir) * f._deltaDir;
		int32 dy = 4 * Direction_YFactor(dir) * f._deltaDir;
		max_endx += dx;
		max_endy += dy;
		testframe = getNextFrame(testframe);
		if (testframe == _endFrame)
			return;
	}
}

bool AnimationTracker::step() {
	if (_done) return false;

	if (_firstFrame)
		_currentFrame = _startFrame;
	else
		_currentFrame = getNextFrame(_currentFrame);

	if (_currentFrame == _endFrame) {
		_done = true;

		// toggle ACT_FIRSTSTEP flag if necessary. This is remembered
		// between two-step animations.
		if (_animAction->hasFlags(AnimAction::AAF_TWOSTEP))
			_firstStep = !_firstStep;

		return false;
	}

	const bool is_u8 = GAME_IS_U8;
	const bool is_crusader = !is_u8;

	_prev = _curr;

	// reset status flags
	_unsupported = false;
	_blocked = false;


	_firstFrame = false;

	const AnimFrame &f = _animAction->getFrame(_dir, _currentFrame);

	_shapeFrame = f._frame;
	_flipped = (is_u8 && f.is_flipped())
			|| (is_crusader && f.is_cruflipped());

	// determine movement for this frame
	Direction movedir = _dir;
	if (_animAction->hasFlags(AnimAction::AAF_ROTATED)) {
		movedir = Direction_TurnByDelta(movedir, 4, dirmode_16dirs);
	}

	int32 dx = 4 * Direction_XFactor(movedir) * f._deltaDir;
	int32 dy = 4 * Direction_YFactor(movedir) * f._deltaDir;
	int32 dz = f._deltaZ;

	if (_mode == TargetMode && !f.is_onground()) {
		dx += _targetDx / _targetOffGroundLeft;
		dy += _targetDy / _targetOffGroundLeft;
		dz += _targetDz / _targetOffGroundLeft;

		_targetDx -= _targetDx / _targetOffGroundLeft;
		_targetDy -= _targetDy / _targetOffGroundLeft;
		_targetDz -= _targetDz / _targetOffGroundLeft;

		--_targetOffGroundLeft;
	}

	// determine footpad
	Actor *a = getActor(_actor);
	assert(a);

	bool actorflipped = a->hasFlags(Item::FLG_FLIPPED);
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
	tx = _curr.x + dx;
	ty = _curr.y + dy;
	tz = _curr.z + dz;

	// Only for particularly large steps we do a full sweepTest
	if (ABS(dx) >= xd - 8 || ABS(dy) >= yd - 8 || ABS(dz) >= zd - 8) {

		Point3 start = _curr;
		Point3 end(tx, ty, tz);
		int32 dims[3] = { xd, yd, zd };

		// Do the sweep test
		Std::list<CurrentMap::SweepItem> collisions;
		cm->sweepTest(start, end, dims, a->getShapeInfo()->_flags, _actor,
		              false, &collisions);

		for (const auto &collision : collisions) {
			// hit something, can't move
			if (!collision._touching && collision._blocking) {
#ifdef WATCHACTOR
				if (a->getObjId() == watchactor) {
					debugC(kDebugActor, "AnimationTracker: did sweepTest for large step; collision at time %d", it->_hitTime);
				}
#endif
				_blocked = true;
				_curr = collision.GetInterpolatedCoords(end, start);
				return false;
			}
		}

		// If it succeeded, we proceed as usual
	}

	Box target(tx, ty, tz, xd, yd, zd);
	Box start(_start.x, _start.y, _start.z, xd, yd, zd);
	PositionInfo info = cm->getPositionInfo(target, start, a->getShapeInfo()->_flags, _actor);

	if (is_u8 && info.valid && info.supported && info.land) {
		// Might need to check for bridge traversal adjustments
		uint32 supportshape = info.land->getShape();
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
				if (dy == 0 && dx != 0 && !info.land->hasFlags(Item::FLG_FLIPPED)) {
					// Moving left or right on horizontal bridge
					// descentdelta = 60*dy/dx
					// 60*dy = descentdelta * dx
					// dy = descentdelta * dx / 60;
					ty += descentdelta * dx / 60;
				} else if (dx == 0 && dy != 0 && info.land->hasFlags(Item::FLG_FLIPPED)) {
					// Moving up or down on vertical bridge
					tx += descentdelta * dy / 60;
				}
			}
		}
	}

	if (!info.valid || (f.is_onground() && !info.supported)) {
		// If on ground, try to adjust properly. Never do it for dead Crusader NPCs,
		// as they don't get gravity and the death process gets stuck.
		// TODO: Profile the effect of disabling this for pathfinding.
		//       It shouldn't be necessary in that case, and may provide a
		//       worthwhile speed-up.
		if (f.is_onground() && zd > 8 && !(is_crusader && a->isDead())) {
			bool targetok = cm->scanForValidPosition(tx, ty, tz, a, _dir,
			                                    true, tx, ty, tz);

			if (!targetok) {
				_blocked = true;
				return false;
			} else {
#ifdef WATCHACTOR
				if (a->getObjId() == watchactor) {
					debugC(kDebugActor, "AnimationTracker: adjusted step: x: %d, %d, %d y: %d, %d, %d z: %d, %d, %d",
						  tx, _x, dx, ty, _y, dy, tz, _z, dz);
				}
#endif
			}
		} else {
			if (!info.valid) {
				_blocked = true;
				return false;
			}
		}
	}

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor) {
		debugC(kDebugActor, "AnimationTracker: step (%d, %d, %d) + (%d, %d, %d)",
			_x, _y, _z, tx - _x, ty - _y, tz - _z);
	}
#endif

	_curr.x = tx;
	_curr.y = ty;
	_curr.z = tz;


	// if attack animation, see if we hit something
	if (_animAction->hasFlags(AnimAction::AAF_ATTACK) &&
	        (_hitObject == 0) && f.attack_range() > 0) {
		checkWeaponHit();
	}

	if (f.is_onground()) {
		// needs support
		target = Box(tx, ty, tz, xd, yd, zd);
		info = cm->getPositionInfo(target, start, a->getShapeInfo()->_flags, _actor);

		if (!info.supported) {
			_unsupported = true;
			return false;
		}
	}

	if (f.is_callusecode()) {
		a->callUsecodeEvent_calledFromAnim();
	}

	return true;
}

const AnimFrame *AnimationTracker::getAnimFrame() const {
	return &_animAction->getFrame(_dir, _currentFrame);
}

void AnimationTracker::setTargetedMode(const Point3 &pt) {
	unsigned int i;
	int totaldir = 0;
	int totalz = 0;
	int offGround = 0;
	int32 end_dx, end_dy, end_dz;

	for (i = _startFrame; i != _endFrame; i = getNextFrame(i)) {
		const AnimFrame &f = _animAction->getFrame(_dir, i);
		totaldir += f._deltaDir;  // This line sometimes seg faults.. ????
		totalz += f._deltaZ;
		if (!f.is_onground())
			++offGround;
	}

	end_dx = 4 * Direction_XFactor(_dir) * totaldir;
	end_dy = 4 * Direction_YFactor(_dir) * totaldir;
	end_dz = totalz;

	if (offGround) {
		_mode = TargetMode;
		_targetOffGroundLeft = offGround;
		_targetDx = pt.x - _curr.x - end_dx;
		_targetDy = pt.y - _curr.y - end_dy;
		_targetDz = pt.z - _curr.z - end_dz;

		// Don't allow large changes in Z
		if (_targetDz > 16)
			_targetDz = 16;
		if (_targetDz < -16)
			_targetDz = -16;

	}

}

void AnimationTracker::checkWeaponHit() {
	int range = _animAction->getFrame(_dir, _currentFrame).attack_range();

	const Actor *a = getActor(_actor);
	assert(a);


	Box abox = a->getWorldBox();
	abox.moveTo(_curr.x, _curr.y, _curr.z);
	abox.translate(Direction_XFactor(_dir) * 32 * range, Direction_YFactor(_dir) * 32 * range, 0);

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor) {
		debugC(kDebugActor, "AnimationTracker: Checking hit, range %d, box %d, %d, %d : %d, %d, %d",
			range, abox._x, abox._y, abox._z, abox._xd, abox._yd, abox._zd);
	}
#endif

	CurrentMap *cm = World::get_instance()->getCurrentMap();

	UCList itemlist(2);
	LOOPSCRIPT(script, LS_TOKEN_END);

	cm->areaSearch(&itemlist, script, sizeof(script), 0, 320, false, _curr.x, _curr.y);

	ObjId hit = 0;
	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		ObjId itemid = itemlist.getuint16(i);
		if (itemid == _actor) continue; // don't want to hit self

		Actor *item = getActor(itemid);
		if (!item) continue;

		Box ibox = item->getWorldBox();

		if (abox.overlaps(ibox)) {
			hit = itemid;
#ifdef WATCHACTOR
			if (a->getObjId() == watchactor) {
				debugC(kDebugActor, "hit: %s", item->dumpInfo().c_str());
			}
#endif
			break;
		}
	}

#ifdef WATCHACTOR
	if (a->getObjId() == watchactor && !hit) {
		debugC(kDebugActor, "nothing");
	}
#endif

	_hitObject = hit;
}

void AnimationTracker::updateState(PathfindingState &state) {
	state._point = _curr;
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

	if (_animAction && GAME_IS_U8) {
		bool hanging = _animAction->hasFlags(AnimAction::AAF_HANGING);
		if (hanging)
			a->setFlag(Item::FLG_HANGING);
		else
			a->clearFlag(Item::FLG_HANGING);
	}

	if (_currentFrame != _endFrame)
		a->_animFrame = _currentFrame;
}

Point3 AnimationTracker::getInterpolatedPosition(int fc) const {
	int32 dx = _curr.x - _prev.x;
	int32 dy = _curr.y - _prev.y;
	int32 dz = _curr.z - _prev.z;

	int repeat = _animAction->getFrameRepeat();

	return Point3(_prev.x + (dx * fc) / (repeat + 1),
				  _prev.y + (dy * fc) / (repeat + 1),
				  _prev.z + (dz * fc) / (repeat + 1));
}

void AnimationTracker::getSpeed(int32 &dx, int32 &dy, int32 &dz) const {
	dx = _curr.x - _prev.x;
	dy = _curr.y - _prev.y;
	dz = _curr.z - _prev.z;
}


void AnimationTracker::save(Common::WriteStream *ws) {
	ws->writeUint32LE(_startFrame);
	ws->writeUint32LE(_endFrame);
	uint8 ff = _firstFrame ? 1 : 0;
	ws->writeByte(ff);
	ws->writeUint32LE(_currentFrame);

	ws->writeUint16LE(_actor);
	ws->writeByte(static_cast<uint8>(Direction_ToUsecodeDir(_dir)));

	if (_animAction) {
		ws->writeUint32LE(_animAction->getShapeNum());
		ws->writeUint32LE(_animAction->getAction());
	} else {
		ws->writeUint32LE(0);
		ws->writeUint32LE(0);
	}

	ws->writeUint32LE(static_cast<uint32>(_prev.x));
	ws->writeUint32LE(static_cast<uint32>(_prev.y));
	ws->writeUint32LE(static_cast<uint32>(_prev.z));
	ws->writeUint32LE(static_cast<uint32>(_curr.x));
	ws->writeUint32LE(static_cast<uint32>(_curr.y));
	ws->writeUint32LE(static_cast<uint32>(_curr.z));

	ws->writeUint16LE(static_cast<uint16>(_mode));
	if (_mode == TargetMode) {
		ws->writeUint32LE(static_cast<uint32>(_targetDx));
		ws->writeUint32LE(static_cast<uint32>(_targetDy));
		ws->writeUint32LE(static_cast<uint32>(_targetDz));
		ws->writeUint32LE(static_cast<uint32>(_targetOffGroundLeft));
	}
	uint8 fs = _firstStep ? 1 : 0;
	ws->writeByte(fs);
	uint8 fl = _flipped ? 1 : 0;
	ws->writeByte(fl);
	ws->writeUint32LE(_shapeFrame);

	uint8 flag = _done ? 1 : 0;
	ws->writeByte(flag);
	flag = _blocked ? 1 : 0;
	ws->writeByte(flag);
	flag = _unsupported ? 1 : 0;
	ws->writeByte(flag);
	ws->writeUint16LE(_hitObject);
}

bool AnimationTracker::load(Common::ReadStream *rs, uint32 version) {
	_startFrame = rs->readUint32LE();
	_endFrame = rs->readUint32LE();
	_firstFrame = (rs->readByte() != 0);
	_currentFrame = rs->readUint32LE();

	_actor = rs->readUint16LE();
	_dir = Direction_FromUsecodeDir(rs->readByte());

	uint32 shapenum = rs->readUint32LE();
	uint32 action = rs->readUint32LE();
	if (shapenum == 0) {
		_animAction = nullptr;
	} else {
		_animAction = GameData::get_instance()->getMainShapes()->
		             getAnim(shapenum, action);
		assert(_animAction);
	}

	_prev.x = rs->readUint32LE();
	_prev.y = rs->readUint32LE();
	_prev.z = rs->readUint32LE();
	_curr.x = rs->readUint32LE();
	_curr.y = rs->readUint32LE();
	_curr.z = rs->readUint32LE();

	_mode = static_cast<Mode>(rs->readUint16LE());
	if (_mode == TargetMode) {
		_targetDx = rs->readUint32LE();
		_targetDy = rs->readUint32LE();
		if (version >= 5) {
			_targetDz = rs->readUint32LE();
			_targetOffGroundLeft = rs->readUint32LE();
		} else {
			// Versions before 5 stored the only _x,_y adjustment
			// to be made per frame. This is less accurate and ignores _z.

			_targetOffGroundLeft = 0;
			unsigned int i = _currentFrame;
			if (!_firstFrame) i = getNextFrame(i);

			for (; _animAction && i != _endFrame; i = getNextFrame(i)) {
				const AnimFrame &f = _animAction->getFrame(_dir, i);
				if (f.is_onground())
					++_targetOffGroundLeft;
			}

			_targetDx *= _targetOffGroundLeft;
			_targetDy *= _targetOffGroundLeft;
			_targetDz = 0;
		}
	}

	_firstStep = (rs->readByte() != 0);
	_flipped = (rs->readByte() != 0);
	_shapeFrame = rs->readUint32LE();

	_done = (rs->readByte() != 0);
	_blocked = (rs->readByte() != 0);
	_unsupported = (rs->readByte() != 0);
	_hitObject = rs->readUint16LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
