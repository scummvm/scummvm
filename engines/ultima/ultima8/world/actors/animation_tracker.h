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

#ifndef WORLD_ACTORS_ANIMATIONTRACKER_H
#define WORLD_ACTORS_ANIMATIONTRACKER_H

#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/world/actors/pathfinder.h"

namespace Ultima {
namespace Ultima8 {

class Actor;
class AnimAction;
struct AnimFrame;

class AnimationTracker {
public:
	AnimationTracker();
	~AnimationTracker();

	//! initialize the AnimationTracker for the given actor, action, dir
	//! if state is non-zero, start from that state instead of the Actor's
	//! current state
	bool init(const Actor *actor, Animation::Sequence action, Direction dir,
	          const PathfindingState *state = 0);

	//! evaluate the maximum distance the actor will travel if the current
	//! animation runs to completion by incremental calls to step
	void evaluateMaxAnimTravel(int32 &max_endx, int32 &max_endy, Direction dir);

	//! do a single step of the animation
	//! returns true if everything ok, false if not
	//! caller must decide if animation should continue after a 'false'
	bool step();

	//! do a single step of the animation, starting at (x,y,z)
	//! returns true if everything ok, false if not
	//! caller must decide if animation should continue after a 'false'
	bool stepFrom(int32 x, int32 y, int32 z);

	//! update the PathfindingState with latest coordinates and flags
	void updateState(PathfindingState &state);

	//! update the Actor with latest flags and animframe
	void updateActorFlags();

	//! get the current position
	void getPosition(int32 &x, int32 &y, int32 &z) const {
		x = _x;
		y = _y;
		z = _z;
	}

	void getInterpolatedPosition(int32 &x, int32 &y, int32 &z, int fc)
            const;

	//! get the difference between current position and previous position
	void getSpeed(int32 &dx, int32 &dy, int32 &dz) const;

	//! get the current (shape)frame
	uint32 getFrame() const {
		return _shapeFrame;
	}

	//! get the current AnimAction
	const AnimAction *getAnimAction() const {
		return _animAction;
	}

	//! get the current AnimFrame
	const AnimFrame *getAnimFrame() const;

	void setTargetedMode(int32 x, int32 y, int32 z);

	bool isDone() const {
		return _done;
	}
	bool isBlocked() const {
		return _blocked;
	}
	bool isUnsupported() const {
		return _unsupported;
	}
	ObjId hitSomething() const {
		return _hitObject;
	}

	bool load(Common::ReadStream *rs, uint32 version);
	void save(Common::WriteStream *ods);

private:
	enum Mode {
		NormalMode = 0,
		TargetMode
	};

	unsigned int getNextFrame(unsigned int frame) const;
	void checkWeaponHit();

	unsigned int _startFrame, _endFrame;
	bool _firstFrame;
	unsigned int _currentFrame;

	ObjId _actor;
	Direction _dir;

	const AnimAction *_animAction;

	// actor state
	int32 _prevX, _prevY, _prevZ;
	int32 _x, _y, _z;
	int32 _startX, _startY, _startZ;
	int32 _targetDx, _targetDy, _targetDz;
	int32 _targetOffGroundLeft;
	bool _firstStep, _flipped;
	uint32 _shapeFrame;

	// status flags
	bool _done;
	bool _blocked;
	bool _unsupported;
	ObjId _hitObject;

	Mode _mode;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
