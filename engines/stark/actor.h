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

#ifndef STARK_ACTOR_H
#define STARK_ACTOR_H

#include "common/array.h"
#include "common/str.h"

#include "math/vector3d.h"

namespace Stark {

namespace Gfx {
class TextureSet;
}

class ArchiveReadStream;
class Skeleton;
class SkeletonAnim;


class VertNode {
public:
	Math::Vector3d _pos1, _pos2;
	Math::Vector3d _normal;
	float _texS, _texT;
	uint32 _bone1, _bone2;
	float _boneWeight;
};

class TriNode {
public:
	uint32 _vert1, _vert2, _vert3;
};

class FaceNode {
public:
	FaceNode() { }

	~FaceNode() {
		for (Common::Array<VertNode *>::iterator it = _verts.begin(); it != _verts.end(); ++it)
			delete *it;

		for (Common::Array<TriNode *>::iterator it = _tris.begin(); it != _tris.end(); ++it)
			delete *it;
	}

	uint32 _matIdx;
	Common::Array<VertNode *> _verts;
	Common::Array<TriNode *> _tris;
};


class MeshNode {
public:
	MeshNode() { }
	~MeshNode() {
		Common::Array<FaceNode *>::iterator it = _faces.begin();
		while (it != _faces.end()) {
			delete *it;
			++it;
		}

	}
	Common::String _name;
	Common::Array<FaceNode *> _faces;
};


class MaterialNode {
public:
	Common::String _name;
	uint32 _unknown1;
	Common::String _texName;
	float _r, _g, _b;
};

class UnknownNode {
public:
	float _u1, _u2, _u3, _u4;
};


/**
 * Base object for handling 3D actors
 */
class Actor {
public:
	Actor();
	~Actor();

	/**
	 * Try and initialise object from the specified stream
	 */
	void readFromStream(ArchiveReadStream *stream);

	const Common::Array<MeshNode *> &getMeshes() const { return _meshes; }
	const Common::Array<MaterialNode *> &getMaterials() const { return _materials; }
	Skeleton *getSkeleton() const { return _skeleton; }
	const Gfx::TextureSet *getTextureSet() const { return _textureSet; }
	float getFacingDirection() const { return _facingDirection; }

	/**
	 * Load animation data from the specified stream
	 */
	void setAnim(SkeletonAnim *anim);

	/**
	 * Load texture data from the specified stream
	 */
	void setTextureSet(Gfx::TextureSet *textureSet);

private:
	uint32 _u1;
	float _facingDirection;

	Common::Array<MaterialNode *> _materials;
	Common::Array<MeshNode *> _meshes;
	Skeleton *_skeleton;
	Gfx::TextureSet *_textureSet;
};

} // End of namespace Stark

#endif // STARK_ACTOR_H
