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

// Based on Phantasma code by Thomas Harte (2013)

#ifndef FREESCAPE_SENSOR_H
#define FREESCAPE_SENSOR_H

#include "freescape/objects/object.h"

namespace Freescape {

class Sensor : public Object {
public:

	Sensor(
		uint16 objectID,
		const Math::Vector3d &origin,
		const Math::Vector3d &rotation);
	virtual ~Sensor();

	bool isDrawable();
	bool isPlanar();
	Type getType() override { return Type::Sensor; };
	Math::Vector3d getRotation() { return rotation; }

	void draw(Freescape::Renderer *gfx) override { error("cannot render sensor"); };
};

} // End of namespace Freescape

#endif
