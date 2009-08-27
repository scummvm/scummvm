/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_MODEL_H
#define GRIM_MODEL_H

#include "engines/grim/resource.h"
#include "graphics/matrix4.h"

namespace Grim {

class TextSplitter;

class Model {
public:
	// Construct a 3D model from the given data.
	Model(const char *filename, const char *data, int len, const CMap *cmap);
	void loadBinary(const char *&data, const CMap *cmap);
	void loadText(TextSplitter *ts, const CMap *cmap);
	void reload(const CMap *cmap);
	void draw() const;

	~Model();

	Common::String _fname;

	struct Geoset;
	struct Mesh;
	struct HierNode {
		HierNode() : _initialized(false) { }
		void loadBinary(const char *&data, HierNode *hierNodes, const Geoset *g);
		void draw() const;
		void addChild(HierNode *child);
		void removeChild(HierNode *child);
		void setMatrix(Graphics::Matrix4 matrix);
		void update();

		char _name[64];
		Mesh *_mesh;
		int _flags, _type;
		int _depth, _numChildren;
		HierNode *_parent, *_child, *_sibling;
		Graphics::Vector3d _pos, _pivot;
		float _pitch, _yaw, _roll;
		Graphics::Vector3d _animPos;
		float _animPitch, _animYaw, _animRoll;
		bool _meshVisible, _hierVisible;
		int _priority, _totalWeight;
		bool _initialized;
		Graphics::Matrix4 _matrix;
		Graphics::Matrix4 _localMatrix;
		Graphics::Matrix4 _pivotMatrix;
	};

	HierNode *copyHierarchy();
	int numNodes() const { return _numHierNodes; }

//private:
	struct Face {
		int loadBinary(const char *&data, Material *materials[]);
		void draw(float *vertices, float *vertNormals, float *textureVerts) const;
		void changeMaterial(Material *material);
		~Face();

		Material *_material;
		int _type, _geo, _light, _tex;
		float _extraLight;
		int _numVertices;
		int *_vertices, *_texVertices;
		Graphics::Vector3d _normal;
	};

	struct Mesh {
		void loadBinary(const char *&data, Material *materials[]);
		void loadText(TextSplitter *ts, Material *materials[]);
		void changeMaterials(Material *materials[]);
		void draw() const;
		void update();
		Mesh() : _numFaces(0) { }
		~Mesh();

		char _name[32];
		float _radius;
		int _shadow, _geometryMode, _lightingMode, _textureMode;

		int _numVertices;
		int *_materialid;
		float *_vertices;		// sets of 3
		float *_verticesI;
		float *_vertNormals;	// sets of 3

		int _numTextureVerts;
		float *_textureVerts;	// sets of 2

		int _numFaces;
		Face *_faces;
		Graphics::Matrix4 _matrix;
	};

	struct Geoset {
		void loadBinary(const char *&data, Material *materials[]);
		void loadText(TextSplitter *ts, Material *materials[]);
		void changeMaterials(Material *materials[]);
		Geoset() : _numMeshes(0) { }
		~Geoset();

		int _numMeshes;
		Mesh *_meshes;
	};

	int _numMaterials;
	char (*_materialNames)[32];
	Material **_materials;
	Graphics::Vector3d _insertOffset;
	int _numGeosets;
	Geoset *_geosets;
	float _radius;
	int _numHierNodes;
	HierNode *_rootHierNode;
};

} // end of namespace Grim

#endif
