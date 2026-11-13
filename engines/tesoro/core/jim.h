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

#ifndef TESORO_CORE_JIM_H
#define TESORO_CORE_JIM_H

#include "common/array.h"
#include "tesoro/core/object.h"
#include "tesoro/core/walkarea.h"

namespace Tesoro {

class World;

/** Jim, the player's character: an object with nine animations (four speaking, four walking, the standing frames). Facing is 1 up, 2 down, 3 left, 4 right. */
class Jim : public Object {
public:
	enum { kBanks = 9 };
	// The order of the banks, as the files are loaded.
	enum Bank { kSpeakLeft, kSpeakRight, kStanding, kWalkUp, kWalkDown, kWalkLeft, kWalkRight, kSpeakDown, kSpeakUp };

	explicit Jim(World &world) : _world(world) {}

	/** Sets Jim up from his nine banks. Stands facing left. */
	void init(const Common::Array<AnimPlayer> &levelPlayers);

	/** Walks to (x, y). direct: straight there, not round the waypoints. target is told on arrival. */
	int walkTo(int32 x, int32 y, WalkTarget *target, bool direct);

	void setFacing(int facing);

	bool walking() const { return _walking; }
	void stopWalking() { _walking = false; }
	bool direct() const { return _direct; }
	int facing() const { return _facing; }
	WalkTarget *target() const { return _target; }
	int32 destX() const { return _destX; }
	int32 destY() const { return _destY; }

	/** Moves Jim into a room of a level and puts him at (x, y), the camera following him. */
	void enterLevel(Level *level, Room *room, int32 x, int32 y);

	/** Takes Jim out of his level, for when the level is thrown away. */
	void leaveLevel();

	int32 scale() const { return _scale; }

	/** For now only keeps his size up to date. Returns 1 to go on. */
	int update() override;
	void updateAnimation(int32 now) override { _anim.tickClearMovement(now); }
	void updateBounds(int32 now) override;
	void draw(const Camera &camera, Display &display) override;
	bool isHitter(const Object *other) const override { return other->kindId() == 7; }

private:

	bool startWalkAnimation(int facing);
	void advanceAnimation(int16 &moveX, int16 &moveY);
	void tickAnimation();
	void showFacing(int facing);

	World &_world;

	bool _walking = false;
	bool _direct = false;
	WalkTarget *_target = nullptr;
	int32 _fixedX = 0, _fixedY = 0;	// the position in 24.8 fixed point
	int32 _moveX = 0, _moveY = 0;	// the animation's movement already consumed
	int32 _destX = 0, _destY = 0;
	int32 _scale = 0x100;	// 0x100 is unscaled
	int _facing = 3;
};

} // End of namespace Tesoro

#endif
