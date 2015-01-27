/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_RESOURCES_FLOOR_H
#define STARK_RESOURCES_FLOOR_H

#include "common/array.h"
#include "common/str.h"

#include "math/vector3d.h"

#include "engines/stark/resources/resource.h"

namespace Stark {

class FloorFace;
class XRCReadStream;

/**
 * This resource represents the floor field of a 3D layer.
 * Characters can only walk on the floor field.
 *
 * The floor field is made of a list of faces building a mesh.
 */
class Floor : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kFloor;

	Floor(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Floor();

	// Resource API
	void onAllLoaded() override;

	/** Obtain the vertex for an index */
	Math::Vector3d getVertex(uint32 indice) const;

	/**
	 * Obtain the index of the face containing the point when both the floorfield
	 * and the point are projected on a Z=0 plane.
	 *
	 * Return -1 if no face contains the point.
	 */
	int32 findFaceContainingPoint(const Math::Vector3d &point) const;

	/** Fill the z coordinate of the point so that it is on the plane of a face */
	void computePointHeightInFace(Math::Vector3d &point, uint32 faceIndex) const;

	/** Obtain the distance to the camera for a face */
	float getDistanceFromCamera(uint32 faceIndex) const;

protected:
	void readData(XRCReadStream *stream) override;
	void printData() override;

	uint32 _facesCount;
	Common::Array<Math::Vector3d> _vertices;
	Common::Array<FloorFace *> _faces;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_FLOOR_H
