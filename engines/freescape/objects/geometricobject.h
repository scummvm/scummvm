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

#ifndef FREESCAPE_GEOMETRICOBJECT_H
#define FREESCAPE_GEOMETRICOBJECT_H

#include "freescape/language/instruction.h"
#include "freescape/objects/object.h"

namespace Freescape {

class GeometricObject : public Object {
public:
	static int numberOfColoursForObjectOfType(Type type);
	static int numberOfOrdinatesForType(Type type);
	static bool isPyramid(Type type);
	static bool isPolygon(Type type);

	GeometricObject(
		Type type,
		uint16 objectID,
		uint16 flags,
		const Math::Vector3d &origin,
		const Math::Vector3d &size,
		Common::Array<uint8> *colours,
		Common::Array<uint16> *ordinates,
		FCLInstructionVector conditionInstructions,
		Common::String *conditionSource = nullptr);
	virtual ~GeometricObject();

	GeometricObject *duplicate();
	void createBoundingBox();
	bool collides(const Math::AABB &boundingBox);
	void draw(Freescape::Renderer *gfx) override;
	bool isDrawable();
	bool isPlanar();

	Common::String *conditionSource;
	FCLInstructionVector condition;

private:
	Common::Array<uint8> *colours;
	Common::Array<uint16> *ordinates;
};

} // End of namespace Freescape

#endif
