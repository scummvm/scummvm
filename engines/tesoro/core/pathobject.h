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


#ifndef TESORO_CORE_PATHOBJECT_H
#define TESORO_CORE_PATHOBJECT_H

#include "tesoro/core/object.h"

namespace Tesoro {

/**
 * Object kind 8: PathObject.
 * Its animation points are the corners of the route Jim takes round obstacles.
 * */
class PathObject : public Object {
public:
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now);

	void updateAnimation(int32 now) override { _anim.tickClearMovement(now); }
	void draw(const Camera &camera, Display &display) override { drawAnimation(camera, display, true); }
	int update() override;
};

} // End of namespace Tesoro

#endif
