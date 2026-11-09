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

#ifndef TESORO_CORE_CAMERAFRAME_H
#define TESORO_CORE_CAMERAFRAME_H

#include "tesoro/core/object.h"

namespace Tesoro {

/**
 * Object kind 1, one per level. Its first step's rectangle is the area the camera may show, and the point it
 * stands on is where the camera starts, both applied when the level attaches its objects.
 */
class CameraFrame : public Object {
public:
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers);

	/** The object named in the level data, found when the frame is attached; null if none. */
	Object *target() const { return _target; }
	byte flag() const { return _flag; }
	void setFlag(byte flag) { _flag = flag; }
	uint16 value() const { return _value; }
	uint16 count() const { return _count; }
	void setValueCount(uint16 count) { _count = count; }
	/** Makes the camera show the rectangle of the first step and start at the frame's position. False if there is no bank. */
	bool frameCamera(Camera &camera);

private:
	char _targetName[13] = {};
	Object *_target = nullptr;
	byte _flag = 0;
	uint16 _value = 0;
	uint16 _count = 0;
};

} // End of namespace Tesoro

#endif
