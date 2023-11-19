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

#ifndef FREESCAPE_GEOMETRICOBJECT_H
#define FREESCAPE_GEOMETRICOBJECT_H

#include "freescape/language/instruction.h"
#include "freescape/objects/group.h"
#include "freescape/objects/object.h"

namespace Freescape {

class GeometricObject : public Object {
public:
	static int numberOfColoursForObjectOfType(ObjectType type);
	static int numberOfOrdinatesForType(ObjectType type);
	static bool isPyramid(ObjectType type);
	static bool isPolygon(ObjectType type);

	GeometricObject(
		ObjectType type,
		uint16 objectID,
		uint16 flags,
		const Math::Vector3d &origin,
		const Math::Vector3d &size,
		Common::Array<uint8> *colours,
		Common::Array<uint16> *ordinates,
		FCLInstructionVector conditionInstructions,
		Common::String conditionSource = "");
	virtual ~GeometricObject();
	void setOrigin(Math::Vector3d origin) override;
	void offsetOrigin(Math::Vector3d origin_);
	void restoreOrdinates();

	Object *duplicate() override;
	void scale(int factor) override;
	void computeBoundingBox();
	bool collides(const Math::AABB &boundingBox);
	void draw(Freescape::Renderer *gfx) override;
	bool isDrawable() override;
	bool isPlanar() override;

	Common::String _conditionSource;
	FCLInstructionVector _condition;

private:
	Common::Array<uint8> *_colours;
	Common::Array<uint16> *_ordinates;
	Common::Array<uint16> *_initialOrdinates;
};

} // End of namespace Freescape

#endif // FREESCAPE_GEOMETRICOBJECT_H
