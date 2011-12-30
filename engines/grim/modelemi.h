/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_MODELEMI_H
#define GRIM_MODELEMI_H

#include "engines/grim/object.h"
#include "math/matrix4.h"
#include "math/vector2d.h"
#include "math/vector3d.h"
#include "math/vector4d.h"

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

class EMIModel;
	
class EMIMeshFace {
public:
	Vector3int *_indexes;
	uint32 _faceLength;
	uint32 _numFaces;
	uint32 _hasTexture;
	uint32 _texID;
	uint32 _flags;
	EMIModel *_parent;
	
	EMIMeshFace() : _numFaces(0), _hasTexture(0), _texID(0), _flags(0) { }
	void loadFace(Common::SeekableReadStream *data);
	void setParent(EMIModel *m) { _parent = m; }
	void render();
};

/* TODO: Remember to credit JohnDoe for his EMIMeshViewer, as most of the Skeletal
 * math, and understandings comes from his Delphi-code.
 */
class EMIModel : public Object {
public:
	int _numVertices;
	Math::Vector3d *_vertices;
	Math::Vector3d *_normals;
	EMIColormap *_colorMap;
	Math::Vector2d *_texVerts;
	
	uint32 _numFaces;
	EMIMeshFace *_faces;
	uint32 _numTextures;
	Common::String *_texNames;
	Material **_mats;
	
	int _numBones;
	
	// Stuff we dont know how to use:
	Math::Vector4d *_sphereData;
	Math::Vector3d *_boxData;
	Math::Vector3d *_boxData2;
	int _numTexSets;
	int _setType;
	
public:
	EMIModel(const Common::String &filename, Common::SeekableReadStream *data, EMIModel *parent = NULL);
	void setTex(int index); 
	void loadMesh(Common::SeekableReadStream *data);
	void prepareForRender();
	void prepare();
	void draw();
};
	
} // end of namespace Grim

#endif
