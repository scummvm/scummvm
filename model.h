// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef MODEL_H
#define MODEL_H

#include "vector3d.h"
#include "matrix4.h"
#include "resource.h"

#include <cstring>

class CMap;
class Material;
class TextSplitter;

class Model : public Resource {
public:
	// Construct a 3D model from the given data.
	Model(const char *filename, const char *data, int len, const CMap &cmap);
	void loadBinary(const char *data, const CMap &cmap);
	void loadText(TextSplitter &ts, const CMap &cmap);
	void reload(const CMap &cmap);
	void draw() const;

	~Model();

	struct Geoset;
	struct Mesh;
	struct HierNode {
		HierNode() : _initialized(false) { }
		void loadBinary(const char *&data, HierNode *hierNodes, const Geoset &g);
		void draw() const;
		void addChild(HierNode *child);
		void removeChild(HierNode *child);
		void setMatrix(Matrix4 matrix);
		void update();

		char _name[64];
		Mesh *_mesh;
		int _flags, _type;
		int _depth, _numChildren;
		HierNode *_parent, *_child, *_sibling;
		Vector3d _pos, _pivot;
		float _pitch, _yaw, _roll;
		Vector3d _animPos;
		float _animPitch, _animYaw, _animRoll;
		bool _meshVisible, _hierVisible;
		int _priority, _totalWeight;
		bool _initialized;
		Matrix4 _matrix;
		Matrix4 _localMatrix;
		Matrix4 _pivotMatrix;
	};

	HierNode *copyHierarchy();
	int numNodes() const { return _numHierNodes; }

//private:
	struct Face {
		int loadBinary(const char *&data, ResPtr<Material> *materials);
		void draw(float *vertices, float *vertNormals, float *textureVerts) const;
		void changeMaterial(ResPtr<Material> materials);
		~Face();

		Material *_material;
		int _type, _geo, _light, _tex;
		float _extraLight;
		int _numVertices;
		int *_vertices, *_texVertices;
		Vector3d _normal;
	};

	struct Mesh {
		void loadBinary(const char *&data, ResPtr<Material> *materials);
		void loadText(TextSplitter &ts, ResPtr<Material> *materials);
		void changeMaterials(ResPtr<Material> *materials);
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
		Matrix4 _matrix;
	};

	struct Geoset {
		void loadBinary(const char *&data, ResPtr<Material> *materials);
		void loadText(TextSplitter &ts, ResPtr<Material> *materials);
		void changeMaterials(ResPtr<Material> *materials);
		Geoset() : _numMeshes(0) { }
		~Geoset();

		int _numMeshes;
		Mesh *_meshes;
	};

	int _numMaterials;
	char (*_materialNames)[32];
	ResPtr<Material> *_materials;
	Vector3d _insertOffset;
	int _numGeosets;
	Geoset *_geosets;
	float _radius;
	int _numHierNodes;
	HierNode *_rootHierNode;
};

#endif
