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

#include "freescape/area.h"
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
		Common::String conditionSource_);

	byte _firingInterval;
	uint16 _firingRange;
	uint16 _axis;
	bool _isShooting;

	Common::String _conditionSource;
	FCLInstructionVector _condition;

	virtual ~Sensor() { delete _colours; }
	bool isDrawable() override { return true; }
	bool isPlanar() override { return true; }
	bool isShooting() { return _isShooting; }
	void scale(int factor) override;
	Object *duplicate() override;

	ObjectType getType() override { return kSensorType; };
	Math::Vector3d getRotation() { return _rotation; }
	void shouldShoot(bool shooting) { _isShooting = shooting; }

	void draw(Freescape::Renderer *gfx, float offset = 0.0) override;

	bool playerDetected(const Math::Vector3d &position, Area *area);

	private:
		Common::Array<uint8> *_colours;
};

} // End of namespace Freescape

#endif // FREESCAPE_SENSOR_H
