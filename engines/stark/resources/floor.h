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

#include "math/ray.h"
#include "math/vector3d.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Floor;
class FloorFace;

/**
 * A floor face (triangle) edge
 *
 * Used for path finding
 */
class FloorEdge {
public:
	FloorEdge(uint16 vertexIndex1, uint16 vertexIndex2, uint32 faceIndex1);

	/** Build a list of neighbour edges in the graph */
	void buildNeighbours(const Floor *floor);

	/** Set the edge middle position */
	void computeMiddle(const Floor *floor);

	/** Set the edge's second face */
	void setOtherFace(uint32 faceIndex);

	/** Check if the edge has the same vertices as the parameters */
	bool hasVertices(uint16 vertexIndex1, uint16 vertexIndex2) const;

	int32 getFaceIndex1() const;
	int32 getFaceIndex2() const;
private:
	void addNeighboursFromFace(const FloorFace *face);

	uint16 _vertexIndex1;
	uint16 _vertexIndex2;
	Math::Vector3d _middle;
	int32 _faceIndex1;
	int32 _faceIndex2;

	Common::Array<FloorEdge *> _neighbours;
};

/**
 * This resource represents the floor of a 3D layer.
 * Characters can only walk on the floor.
 *
 * The floor is made of a list of faces building a mesh.
 */
class Floor : public Object {
public:
	static const Type::ResourceType TYPE = Type::kFloor;

	Floor(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Floor();

	// Resource API
	void onAllLoaded() override;

	/** Obtain the vertex for an index */
	Math::Vector3d getVertex(uint32 index) const;

	/**
	 * Obtain the index of the face containing the point when both the floorfield
	 * and the point are projected on a Z=0 plane.
	 *
	 * Return -1 if no face contains the point.
	 */
	int32 findFaceContainingPoint(const Math::Vector3d &point) const;

	/** Fill the z coordinate of the point so that it is on the plane of a face */
	void computePointHeightInFace(Math::Vector3d &point, uint32 faceIndex) const;

	/**
	 * Check if a ray is intersecting the floor
	 *
	 * @param origin The ray's origin
	 * @param direction The ray's direction
	 * @param intersection The intersection between the ray and the floor. Only valid when the return value is true.
	 * @return -1 if no face contains the point, the hit face index otherwise
	 */
	int32 findFaceHitByRay(const Math::Ray &ray, Math::Vector3d &intersection) const;

	/** Obtain the distance to the camera for a face */
	float getDistanceFromCamera(uint32 faceIndex) const;

	/** Get a floor face by its index */
	FloorFace *getFace(uint32 index) const;

protected:
	void readData(Formats::XRCReadStream *stream) override;
	void printData() override;

	void buildEdgeList();
	void addFaceEdgeToList(uint32 faceIndex, uint32 index1, uint32 index2);

	uint32 _facesCount;
	Common::Array<Math::Vector3d> _vertices;
	Common::Array<FloorFace *> _faces;
	Common::Array<FloorEdge> _edges;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_FLOOR_H
