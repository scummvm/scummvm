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

#ifndef STARK_RESOURCES_FLOOR_FACE_H
#define STARK_RESOURCES_FLOOR_FACE_H

#include "common/array.h"
#include "common/str.h"

#include "math/ray.h"
#include "math/vector3d.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class FloorEdge;

/**
 * A floor face is a 3D triangle used to build the floor
 */
class FloorFace : public Object {
public:
	static const Type::ResourceType TYPE = Type::kFloorFace;

	FloorFace(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~FloorFace();

	// Resource API
	virtual void onAllLoaded() override;

	/** Return true if the point is inside the face when both are projected on a Z=0 plane*/
	bool isPointInside(const Math::Vector3d &point) const;

	/** Fill the z coordinate of the point so that it is on the plane */
	void computePointHeight(Math::Vector3d &point) const;

	/**
	 * Check if a ray is intersecting this face
	 *
	 * @param origin The ray's origin
	 * @param direction The ray's direction
	 * @param intersection The intersection between the ray and the face. Only valid when the return value is true.
	 * @return true if the ray intersects the face, false otherwise.
	 */
	bool intersectRay(const Math::Ray &ray, Math::Vector3d &intersection) const;

	/**
	 * Compute the distance between the face center and the ray
	 */
	float distanceToRay(const Math::Ray &ray) const;

	/** Obtain the distance to the camera */
	float getDistanceFromCamera() const;

	/** Get one of the three vertex indices from the face */
	int16 getVertexIndex(int32 index) const;

	/** Add an edge to the triangle edge list */
	void addEdge(FloorEdge *edge);

	/** Get the triangle's edge list */
	Common::Array<FloorEdge *> getEdges() const;

	/**
	 * Find the edge closest to a point
	 *
	 * Distance are compared using the middle point of each edge of the face
	 */
	FloorEdge *findNearestEdge(const Math::Vector3d &point) const;

	/** Get the point at the center of the face's triangle */
	Math::Vector3d getCenter() const;

	/** Checks if the face is non degenerate */
	bool hasVertices() const;

	/** Allow or disallow characters to walk on this face */
	void enable(bool enable);
	bool isEnabled() const;

protected:
	void readData(Formats::XRCReadStream *stream) override;
	void printData() override;

	int16 _indices[3];
	Math::Vector3d _vertices[3];

	Common::Array<FloorEdge *> _edges; // Owned by Floor

	float _distanceFromCamera;
	float _unk2;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_FLOOR_FACE_H
