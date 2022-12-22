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

#include "freescape/objects/sensor.h"

namespace Freescape {

Sensor::Sensor(
	uint16 objectID_,
	const Math::Vector3d &origin_,
	const Math::Vector3d &rotation_,
	byte color_,
	byte firingInterval_,
	uint16 firingRange_,
	uint16 axis_,
	uint8 flags_,
	FCLInstructionVector condition_,
	Common::String conditionSource_) {
	_objectID = objectID_;
	_origin = origin_;
	_rotation = rotation_;
	_size = Math::Vector3d(3, 3, 3);
	_colours = new Common::Array<uint8>;
	for (int i = 0; i < 6; i++)
		_colours->push_back(color_);
	_firingInterval = firingInterval_;
	_firingRange = firingRange_;
	_axis = axis_;
	_flags = flags_;
	_conditionSource = conditionSource_;
	_condition = condition_;
	_isShooting = false;
}

Object *Sensor::duplicate() { 
	Sensor *sensor = new Sensor(_objectID, _origin, _rotation, (*_colours)[0], _firingInterval, _firingRange, _axis, _flags, _condition, _conditionSource);  
	return sensor;
}

void Sensor::draw(Freescape::Renderer *gfx) {
	Math::Vector3d origin(_origin.x() - 1, _origin.y() - 1, _origin.z() - 1);
	gfx->renderCube(_origin, _size, _colours);
}

bool Sensor::playerDetected(const Math::Vector3d &position, Area *area) {
	if (isDestroyed() || isInvisible())
		return false;

	Math::Vector3d diff = _origin - position;
	bool detected = false;

	if (_axis == 0x01 && diff.x() >= 0)
		detected = true;
	else if (_axis == 0x02 && diff.x() <= 0)
		detected = true;
	else if (_axis == 0x04 && diff.y() >= 0)
		detected = true;
	else if (_axis == 0x08 && diff.y() <= 0)
		detected = true;
	else if (_axis == 0x10 && diff.z() >= 0)
		detected = true;
	else if (_axis == 0x20 && diff.z() <= 0)
		detected = true;

	if (detected) {
		Math::Ray sight(_origin, -diff);
		detected = area->checkInSight(sight, diff.length());
	}

	if (detected) {
		detected = ABS(diff.x() + ABS(diff.y())) + ABS(diff.z()) <= _firingRange;
	}

	return detected;
}

} // End of namespace Freescape
