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

#ifndef GRIM_MODEL_H
#define GRIM_MODEL_H

#include "engines/grim/object.h"
#include "math/matrix4.h"
#include "math/quat.h"

namespace Common {
class SeekableReadStream;
}

namespace Grim {

class TextSplitter;
class Material;
class Mesh;
class ModelNode;
class CMap;
class Sprite;

class Model : public Object {
public:
	// Construct a 3D model from the given data.
	Model(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap, Model *parent = NULL);

	void reload(CMap *cmap);
	void draw() const;
	Material *findMaterial(const char *name, CMap *cmap) const;

	~Model();
	const Common::String &getFilename() const { return _fname; }
	const ObjectPtr<CMap> &getCMap() const { return _cmap; }

	ModelNode *getHierarchy() const;
	int getNumNodes() const { return _numHierNodes; }

	struct Geoset {
		void loadBinary(Common::SeekableReadStream *data, Material *materials[]);
		void loadText(TextSplitter *ts, Material *materials[]);
		void changeMaterials(Material *materials[]);
		Geoset() : _numMeshes(0), _meshes(NULL) { }
		~Geoset();

		int _numMeshes;
		Mesh *_meshes;
	};

//private:
	void loadMaterial(int index, CMap *cmap);
	void loadBinary(Common::SeekableReadStream *data);
	void loadText(TextSplitter *ts);

	Common::String _fname;
	ObjectPtr<CMap> _cmap;
	Model *_parent;
	int _numMaterials;
	char (*_materialNames)[32];
	Material **_materials;
	bool *_materialsShared;
	Math::Vector3d _insertOffset;
	int _numGeosets;
	Geoset *_geosets;
	float _radius;
	int _numHierNodes;
	ModelNode *_rootHierNode;
	Math::Vector3d _bboxPos;
	Math::Vector3d _bboxSize;
};

class MeshFace {
public:
	MeshFace();
	~MeshFace();
	void stealData(MeshFace &other);
	int loadBinary(Common::SeekableReadStream *data, Material *materials[]);
	int loadText(TextSplitter *ts, Material *materials[], int offset);
	void draw(const Mesh *mesh) const;
	void changeMaterial(Material *material);

	bool hasTexture() const { return _texVertices != nullptr; }

	const Math::Vector3d &getNormal() const { return _normal; }
	void setNormal(const Math::Vector3d &normal) { _normal = normal; }
	const Material *getMaterial() const { return _material; }
	int getNumVertices() const { return _numVertices; }
	int getVertex(int i) const { return _vertices[i]; }
	int getTextureVertex(int i) const { return _texVertices[i]; }
	int getLight() const { return _light; }

private:
	Material *_material;
	int _type, _geo, _light, _tex;
	float _extraLight;
	int _numVertices;
	int *_vertices, *_texVertices;
	Math::Vector3d _normal;

public:
	void *_userData;
};

class Mesh {
public:
	void loadBinary(Common::SeekableReadStream *data, Material *materials[]);
	void loadText(TextSplitter *ts, Material *materials[]);
	void changeMaterials(Material *materials[]);
	void draw() const;
	void getBoundingBox(int *x1, int *y1, int *x2, int *y2) const;
	void update();
	Mesh();
	~Mesh();

	char _name[32];
	float _radius;
	int _shadow, _geometryMode, _lightingMode, _textureMode;

	int _numVertices;
	int *_materialid;
	float *_vertices;       // sets of 3
	float *_verticesI;
	float *_vertNormals;    // sets of 3

	int _numTextureVerts;
	float *_textureVerts;   // sets of 2

	int _numFaces;
	MeshFace *_faces;
	Math::Matrix4 _matrix;

	void *_userData;

private:
	void sortFaces();
};

class ModelNode {
public:
	ModelNode();
	~ModelNode();
	void loadBinary(Common::SeekableReadStream *data, ModelNode *hierNodes, const Model::Geoset *g);
	void draw() const;
	void getBoundingBox(int *x1, int *y1, int *x2, int *y2) const;
	void addChild(ModelNode *child);
	void removeChild(ModelNode *child);
	void setMatrix(const Math::Matrix4 &matrix);
	void update();
	void addSprite(Sprite *sprite);
	void removeSprite(const Sprite *sprite);
	void translateViewpoint() const;
	void translateViewpointStart() const;
	void translateViewpointFinish() const;

	char _name[64];
	Mesh *_mesh;
	/**
	 * A value of 0x100 (256) specifies that when animating this node, keyframes should not be
	 * interpolated (lerped), but instead the transition from source to target is to occur
	 * discretely.
	 */
	int _flags;
	/**
	 * Each KeyFrameAnim has a type identifier. This type field is a bitmask which is ANDed againts
	 * the type in the KeyFrameAnim to control which KeyFrameAnims animate on which nodes of the character.
	 * This enables selectively controlling the animations to act only on certain bones.
	 */
	int _type;
	int _depth, _numChildren;
	ModelNode *_parent, *_child, *_sibling;
	// Specifies the bind pose for this node. This data is read from the model file and never altered
	// (could be const).
	Math::Vector3d _pos, _pivot;
	Math::Quaternion _rot;
	// Specifies the animated pose for this node.
	Math::Vector3d _animPos;
	Math::Quaternion _animRot;
	bool _meshVisible, _hierVisible;
	bool _initialized;
	bool _needsUpdate;
	Math::Matrix4 _matrix;
	Math::Matrix4 _localMatrix;
	Math::Matrix4 _pivotMatrix;
	Sprite *_sprite;
};

} // end of namespace Grim

#endif
