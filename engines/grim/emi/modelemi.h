/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef GRIM_MODELEMI_H
#define GRIM_MODELEMI_H

#include "engines/grim/object.h"
#include "engines/grim/actor.h"
#include "math/matrix4.h"
#include "math/vector2d.h"
#include "math/vector3d.h"
#include "math/vector4d.h"
#include "math/aabb.h"

namespace Common {
class SeekableReadStream;
}

namespace Grim {

class Material;

struct EMIColormap {
	unsigned char r, g, b, a;
};

// Todo: port this to math::vector
struct Vector3int;

class EMICostume;
class EMIModel;
struct BoneInfo;
struct Bone;
class Skeleton;

class EMIMeshFace {
public:
	Vector3int *_indexes;
	uint32 _indicesEBO;
	uint32 _faceLength;
	uint32 _numFaces;
	uint32 _hasTexture;
	uint32 _texID;
	uint32 _flags;
	EMIModel *_parent;

	enum MeshFaceFlags {
		kNoLighting = 0x20, // guessed, but distinctive for screen actors
		kAlphaBlend = 0x10000,
		kUnknownBlend = 0x40000 // used only in intro screen actors
	};

	EMIMeshFace() : _faceLength(0), _numFaces(0), _hasTexture(0), _texID(0), _flags(0), _indexes(NULL), _parent(NULL), _indicesEBO(0) { }
	~EMIMeshFace();
	void loadFace(Common::SeekableReadStream *data);
	void setParent(EMIModel *m) { _parent = m; }
	void render();
};

/* TODO: Remember to credit JohnDoe for his EMIMeshViewer, as most of the Skeletal
 * math, and understandings comes from his Delphi-code.
 */
class EMIModel : public Object {
public:
	enum TextureFlags {
		BlendAdditive = 0x400
		// There are more flags, but their purpose is currently unknown.
	};

	Common::String _meshName;
	Actor::AlphaMode _meshAlphaMode;
	float _meshAlpha;
	int _numVertices;
	Math::Vector3d *_vertices;
	Math::Vector3d *_drawVertices;
	Math::Vector3d *_normals;
	Math::Vector3d *_drawNormals;
	Math::Vector3d *_lighting;
	EMIColormap *_colorMap;
	Math::Vector2d *_texVerts;

	uint32 _numFaces;
	EMIMeshFace *_faces;
	uint32 _numTextures;
	Common::String *_texNames;
	uint32 *_texFlags;
	Material **_mats;

	Skeleton *_skeleton;

	int _numBones;

	// Bone-stuff:
	int _numBoneInfos;
	BoneInfo *_boneInfos;
	Common::String *_boneNames;
	int *_vertexBoneInfo;

	// Stuff we dont know how to use:
	float _radius;
	Math::Vector3d *_center;
	Math::Vector3d *_boxData;
	Math::Vector3d *_boxData2;
	int _numTexSets;
	int _setType;

	Common::String _fname;
	EMICostume *_costume;

	void *_userData;
	bool _lightingDirty;

public:
	EMIModel(const Common::String &filename, Common::SeekableReadStream *data, EMICostume *costume);
	~EMIModel();
	void setTex(uint32 index);
	void setSkeleton(Skeleton *skel);
	void loadMesh(Common::SeekableReadStream *data);
	void prepareForRender();
	void prepareTextures();
	void draw();
	void updateLighting(const Math::Matrix4 &modelToWorld);
	void getBoundingBox(int *x1, int *y1, int *x2, int *y2) const;
	Math::AABB calculateWorldBounds(const Math::Matrix4 &matrix) const;
};

} // end of namespace Grim

#endif
