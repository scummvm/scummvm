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


#ifndef TESORO_CORE_WALKFLOOR_H
#define TESORO_CORE_WALKFLOOR_H

#include "tesoro/core/object.h"
#include "tesoro/core/walkarea.h"

namespace Tesoro {

class PathObject;

/**
 * Object kind 7: WalkFloor.
 * WalkFloor determines the scale of things at each point on the floor, and is also collidable to determine Jim's route.
 * */
class WalkFloor : public Object {
public:
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now);

	/** How big things are at a point, 0x100 meaning life size. */
	int32 scaleAt(int32 x, int32 y);

	/** The route object this floor names, found when it is attached. */
	PathObject *path() const { return _path; }

protected:
	void onAttach() override;

private:
	char _pathName[13] = {};
	PathObject *_path = nullptr;
	WalkArea _area;
};

} // End of namespace Tesoro

#endif
