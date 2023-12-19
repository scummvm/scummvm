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

#include "twp/twp.h"
#include "twp/camera.h"
#include "twp/room.h"
#include "twp/object.h"
#include "twp/util.h"
#include "twp/scenegraph.h"

namespace Twp {

void Camera::clamp(Math::Vector2d at) {
	if (_room) {
		Math::Vector2d roomSize = _room->_roomSize;
		Math::Vector2d screenSize = _room->getScreenSize();

		_pos.setX(Twp::clamp(at.getX(), screenSize.getX() / 2.f + _bounds.left(), screenSize.getX() / 2 + _bounds.right()));
		_pos.setY(Twp::clamp(at.getY(), _bounds.bottom(), _bounds.top() - screenSize.getY() / 2));
		//_pos.setX(Twp::clamp(_pos.getX(), screenSize.getX() / 2.f, MAX(roomSize.getX() - screenSize.getX() / 2.f, 0.f)));
		_pos.setX(Twp::clamp(_pos.getX(), screenSize.getX() / 2.f, MAX(10000.f, 0.f)));
		_pos.setY(Twp::clamp(_pos.getY(), screenSize.getY() / 2, MAX(roomSize.getY() - screenSize.getY() / 2, 0.f)));
	}
}

void Camera::setAtCore(Math::Vector2d at) {
	Math::Vector2d screenSize = _room->getScreenSize();
	_pos = at;
	clamp(_pos);
	g_engine->getGfx().cameraPos(_pos - screenSize / 2.f);
}

void Camera::setAt(Math::Vector2d at) {
	setAtCore(at);
	_target = _pos;
	_time = 0;
	_moving = false;
}

void Camera::panTo(Math::Vector2d target, float time, InterpolationKind interpolation) {
	if (!_moving) {
		_moving = true;
		_init = _pos;
		_elapsed = 0.f;
	}
	_function = easing(interpolation);
	_target = target;
	_time = time;
}

void Camera::update(Room *room, Object *follow, float elapsed) {
	_room = room;
	_elapsed += elapsed;
	bool isMoving = _elapsed < _time;

	if (_moving && !isMoving) {
		_moving = false;
		_time = 0.f;
		setAt(_target);
	}

	if (isMoving) {
		float t = _elapsed / _time;
		Math::Vector2d d = _target - _init;
		Math::Vector2d pos = _init + (d * _function.func(t));
		setAtCore(pos);
		return;
	}

	if (follow && follow->_node->isVisible() && follow->_room == room) {
		Math::Vector2d screen = room->getScreenSize();
		Math::Vector2d pos = follow->_node->getPos();
		Math::Vector2d margin(screen.getX() / 6.f, screen.getY() / 6.f);
		Math::Vector2d cameraPos = getAt();

		Math::Vector2d d = pos - cameraPos;
		Math::Vector2d delta = d * elapsed;
		bool sameActor = _follow == follow;

		float x, y;
		if (sameActor && (pos.getX() > (cameraPos.getX() + margin.getX())))
			x = pos.getX() - margin.getX();
		else if (sameActor && (pos.getX() < (cameraPos.getX() - margin.getX())))
			x = pos.getX() + margin.getX();
		else
			x = cameraPos.getX() + (d.getX() > 0 ? MIN(delta.getX(), d.getX()) : MAX(delta.getX(), d.getX()));
		if (sameActor && (pos.getY() > (cameraPos.getY() + margin.getY())))
			y = pos.getY() - margin.getY();
		else if (sameActor && (pos.getY() < (cameraPos.getY() - margin.getY())))
			y = pos.getY() + margin.getY();
		else
			y = cameraPos.getY() + d.getY() > 0 ? MIN(delta.getY(), d.getY()) : MAX(delta.getY(), d.getY());
		setAtCore(Math::Vector2d(x, y));
		if (!sameActor && (fabs(pos.getX() - x) < 1.f) && (fabs(pos.getY() - y) < 1.f))
			_follow = follow;
	}
}

InterpolationMethod intToInterpolationMethod(int value) {
	bool loop = (value & 0x10);
	bool swing = (value & 0x20);
	InterpolationKind kind = (InterpolationKind)(value & 0x0F);
	InterpolationMethod im;
	im.kind = kind;
	im.loop = loop;
	im.swing = swing;
	return im;
}

} // namespace Twp
