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

#ifndef STARK_RESOURCES_PATH_H
#define STARK_RESOURCES_PATH_H

#include <common/rect.h>
#include <math/vector3d.h>
#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

/**
 * A path can be followed by an item in a location
 *
 * Path are made of a list of vertices. Two consecutive vertices delimit an edge.
 * Each vertex has a weight. A higher weight means a higher movement speed.
 */
class Path : public Object {
public:
	static const Type::ResourceType TYPE = Type::kPath;

	enum SubType {
		kPath2D = 1,
		kPath3D = 2
	};

	/** Path factory */
	static Object *construct(Object *parent, byte subType, uint16 index, const Common::String &name);

	Path(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Path();

	// Resource API
	virtual	void readData(Formats::XRCReadStream *stream) override;

	/** Get the edge count in the path */
	virtual uint getEdgeCount() const = 0;

	/**
	 * Get a unit vector pointing in the direction of an edge
	 *
	 * Only valid for 3D paths
	 */
	virtual Math::Vector3d getEdgeDirection(uint edgeIndex) const;

	/** Get the sort key to be used by the item following the path */
	virtual float getSortKey() const;

	/** Get an edge's length */
	float getWeightedEdgeLength(uint edgeIndex) const;

	/** Get the scene position from a position in an edge */
	Math::Vector3d getWeightedPositionInEdge(uint edgeIndex, float positionInEdge);

protected:
	void printData() override;
	float getEdgeLength(uint edgeIndex) const;
	virtual float getVertexWeight(uint vertexIndex) const = 0;
	virtual Math::Vector3d getVertexPosition(uint vertexIndex) const = 0;

	uint32 _field_30;

};

/**
 * A 2D path for 2D items
 */
class Path2D : public Path {
public:
	Path2D(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Path2D();

	struct Vertex {
		float weight;
		Common::Point position;
	};

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;

	// Path API
	uint getEdgeCount() const override;

protected:
	float getVertexWeight(uint vertexIndex) const override;
	Math::Vector3d getVertexPosition(uint vertexIndex) const override;

private:
	// Resource API
	void printData() override;

	Common::Array<Vertex> _vertices;
};

/**
 * A 3D path for 3D items
 */
class Path3D : public Path {
public:
	Path3D(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Path3D();

	struct Vertex {
		float weight;
		Math::Vector3d position;
	};

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;

	// Path API
	uint getEdgeCount() const override;
	float getSortKey() const override;
	Math::Vector3d getEdgeDirection(uint edgeIndex) const override;

	/** Get the full position in world coordinates of one of the vertices of the path */
	Math::Vector3d getVertexPosition3D(uint vertexIndex, int32 *faceIndex);

protected:
	float getVertexWeight(uint vertexIndex) const override;
	Math::Vector3d getVertexPosition(uint vertexIndex) const override;

private:
	// Resource API
	void printData() override;

	Common::Array<Vertex> _vertices;
	float _sortKey;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_PATH_H
