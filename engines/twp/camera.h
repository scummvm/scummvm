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

#ifndef TWP_CAMERA_H
#define TWP_CAMERA_H

#include "common/func.h"
#include "math/vector2d.h"
#include "twp/easing.h"
#include "twp/rectf.h"

namespace Twp {

class Object;
class Room;

class Camera {
public:
	void setAt(const Math::Vector2d &at);
	inline Math::Vector2d getAt() const { return _pos; }

	inline void setBounds(const Rectf &bounds) { _bounds = bounds; }
	inline Rectf getBounds() const { return _bounds; }

	inline void setRoom(Common::SharedPtr<Room> room) { _room = room; }
	inline Common::SharedPtr<Room> getRoom() const { return _room; }

	inline bool isMoving() const { return _moving; }
	void panTo(const Math::Vector2d &target, float time, InterpolationKind interpolation);

	void update(Common::SharedPtr<Room> room, Common::SharedPtr<Object> follow, float elapsed);

private:
	void clamp(const Math::Vector2d &at);
	void setAtCore(const Math::Vector2d &at);

private:
	Math::Vector2d _pos;
	Rectf _bounds = Rectf::fromMinMax(Math::Vector2d(-10000, -10000), Math::Vector2d(10000, 10000));
	bool _moving = false;
	Math::Vector2d _init, _target;
	float _elapsed = 0.f;
	float _time = 0.f;
	Common::SharedPtr<Room> _room;
	Common::SharedPtr<Object> _follow;
	EasingFunc_t _function = {&linear};
};
} // namespace Twp

#endif
