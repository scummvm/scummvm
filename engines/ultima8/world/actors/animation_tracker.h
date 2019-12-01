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

#include "ultima8/world/actors/Animation.h"
#include "pathfinder.h"

namespace Ultima8 {

class Actor;
class IDataSource;
class ODataSource;
struct AnimAction;
struct AnimFrame;

class AnimationTracker {
public:
	AnimationTracker();
	~AnimationTracker();

	//! initialize the AnimationTracker for the given actor, action, dir
	//! if state is non-zero, start from that state instead of the Actor's
	//! current state
	bool init(Actor *actor, Animation::Sequence action, uint32 dir,
	          PathfindingState *state = 0);

	//! evaluate the maximum distance the actor will travel if the current
	//! animation runs to completion by incremental calls to step
	void evaluateMaxAnimTravel(int32 &max_endx, int32 &max_endy, uint32 dir);

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
	void getPosition(int32 &x_, int32 &y_, int32 &z_) {
		x_ = x;
		y_ = y;
		z_ = z;
	}

	void getInterpolatedPosition(int32 &x_, int32 &y_, int32 &z_, int fc);

	//! get the difference between current position and previous position
	void getSpeed(int32 &dx, int32 &dy, int32 &dz);

	//! get the current (shape)frame
	uint32 getFrame() {
		return shapeframe;
	}

	//! get the current AnimAction
	AnimAction *getAnimAction() {
		return animaction;
	}

	//! get the current AnimFrame
	AnimFrame *getAnimFrame();

	void setTargetedMode(int32 x_, int32 y_, int32 z_);

	bool isDone() const {
		return done;
	}
	bool isBlocked() const {
		return blocked;
	}
	bool isUnsupported() const {
		return unsupported;
	}
	ObjId hitSomething() const {
		return hitobject;
	}

	bool load(IDataSource *ids, uint32 version);
	void save(ODataSource *ods);

private:
	enum Mode {
		NormalMode = 0,
		TargetMode
	};

	unsigned int getNextFrame(unsigned int frame);
	void checkWeaponHit();

	unsigned int startframe, endframe;
	bool firstframe;
	unsigned int currentframe;

	ObjId actor;
	unsigned int dir;

	AnimAction *animaction;

	// actor state
	int32 prevx, prevy, prevz;
	int32 x, y, z;
	int32 startx, starty, startz;
	int32 target_dx, target_dy, target_dz;
	int32 target_offground_left;
	bool firststep, flipped;
	uint32 shapeframe;

	// status flags
	bool done;
	bool blocked;
	bool unsupported;
	ObjId hitobject;

	Mode mode;
};

} // End of namespace Ultima8

#endif
