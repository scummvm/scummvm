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

#ifndef STARK_MODEL_MODEL_H
#define STARK_MODEL_MODEL_H

#include "common/array.h"
#include "common/str.h"

#include "math/ray.h"
#include "math/vector3d.h"

namespace Stark {

namespace Gfx {
class TextureSet;
}

class ArchiveReadStream;

class VertNode {
public:
	Math::Vector3d _pos1, _pos2;
	Math::Vector3d _normal;
	float _texS, _texT;
	uint32 _bone1, _bone2;
	float _boneWeight;
};

struct Face {
	uint32 materialId;
	Common::Array<uint32> vertexIndices;

	Face() : materialId(0) {}
};

struct Material {
	Common::String name;
	Common::String texture;
	float r, g, b;
	bool doubleSided;

	Material() : r(0), g(0), b(0), doubleSided(false) {};
};

class BoneNode {
public:
	BoneNode() : _parent(-1), _idx(0), _u1(0) {}
	~BoneNode() { }

	/** Perform a collision test with the ray */
	bool intersectRay(const Math::Ray &ray) const;

	/** Expand a bounding box with the model space BB of this bone */
	void expandModelSpaceBB(Math::AABB &aabb) const;

	Common::String _name;
	float _u1;
	Common::Array<uint32> _children;
	int _parent;
	uint32 _idx;

	Math::Vector3d _animPos;
	Math::Quaternion _animRot;

	/** Bone space bounding box */
	Math::AABB _boundingBox;
};

/**
 * A 3D Model
 */
class Model {
public:
	Model();
	~Model();

	/**
	 * Try and initialise object from the specified stream
	 */
	void readFromStream(ArchiveReadStream *stream);

	const Common::Array<VertNode *> &getVertices() const { return _vertices; }
	const Common::Array<Face *> &getFaces() const { return _faces; }
	const Common::Array<Material *> &getMaterials() const { return _materials; }
	const Common::Array<BoneNode *> &getBones() const { return _bones; };

	/** Perform a collision test with a ray */
	bool intersectRay(const Math::Ray &ray) const;

	/** Update the model bounding box with the current animation state */
	void updateBoundingBox();

	/** Retrieve the model space bounding box for the current animation state */
	Math::AABB getBoundingBox() const;

private:
	void buildBonesBoundingBoxes();
	void buildBoneBoundingBox(BoneNode *bone) const;
	void readBones(ArchiveReadStream *stream);

	Common::String _name;
	uint32 _u1;
	float _u2;

	Common::Array<VertNode *> _vertices;
	Common::Array<Material *> _materials;
	Common::Array<Face *> _faces;
	Common::Array<BoneNode *> _bones;
	Math::AABB _boundingBox;
};

} // End of namespace Stark

#endif // STARK_MODEL_MODEL_H
