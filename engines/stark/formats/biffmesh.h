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

#ifndef STARK_FORMATS_BIFF_MESH_H
#define STARK_FORMATS_BIFF_MESH_H

#include "engines/stark/model/model.h"

#include "common/array.h"

#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Stark {

class ArchiveReadStream;

namespace Formats {

class BiffObject;
class BiffMesh;

/**
 * A mesh reader
 *
 * Reads a mesh out of a BIFF archive
 */
class BiffMeshReader {
public:
	/** Read a mesh from a BIFF archive stream */
	static BiffMesh *read(ArchiveReadStream *stream);

private:
	static BiffObject *biffObjectBuilder(uint32 type);

};

/**
 * A mesh read out of a BIFF archive
 */
class BiffMesh {
public:
	struct Vertex {
		Math::Vector3d position;
		Math::Vector3d normal;
		Math::Vector3d texturePosition;
	};

	BiffMesh(const Common::Array<Vertex> &vertices, const Common::Array<Face> &faces, const Common::Array<Material> &materials);

	const Common::Array<Vertex> &getVertices() const;
	const Common::Array<Face> &getFaces() const;
	const Common::Array<Material> &getMaterials() const;
	Math::Matrix4 getTransform() const;

	void setTransform(const Math::Matrix4 &transform);

private:
	Common::Array<Vertex> _vertices;
	Common::Array<Face> _faces;
	Common::Array<Material> _materials;

	Math::Matrix4 _transform;
};

} // End of namespace Formats
} // End of namespace Stark

#endif // STARK_FORMATS_BIFF_MESH_H
