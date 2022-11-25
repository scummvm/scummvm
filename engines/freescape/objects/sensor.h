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

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#ifndef FREESCAPE_SENSOR_H
#define FREESCAPE_SENSOR_H

#include "freescape/objects/object.h"
#include "freescape/language/instruction.h"

namespace Freescape {

class Sensor : public Object {
public:
	Sensor(
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
	}
	byte _firingInterval;
	uint16 _firingRange;
	uint16 _axis;

	Common::String _conditionSource;
	FCLInstructionVector _condition;

	virtual ~Sensor() {
		delete _colours;
	}
	bool isDrawable() override { return true; }
	bool isPlanar() override { return true; }
	void scale(int factor) override { _origin = _origin / factor; };
	Object *duplicate() override { return (new Sensor(_objectID, _origin, _rotation, (*_colours)[0], _firingInterval, _firingRange, _axis, _flags, _condition, _conditionSource)); };

	ObjectType getType() override { return kSensorType; };
	Math::Vector3d getRotation() { return _rotation; }

	void draw(Freescape::Renderer *gfx) override {
		Math::Vector3d origin(_origin.x() - 1, _origin.y() - 1, _origin.z() - 1);
		gfx->renderCube(_origin, _size, _colours);
	};

	private:
		Common::Array<uint8> *_colours;
};

} // End of namespace Freescape

#endif // FREESCAPE_SENSOR_H
