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

#include "common/endian.h"

#include "engines/grim/model.h"
#include "engines/grim/actor.h"
#include "engines/grim/material.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/gfx_base.h"

namespace Grim {

Model::Model(const char *filename, const char *data, int len, const CMap &cmap) :
		Resource(filename), _numMaterials(0), _numGeosets(0) {

	if (len >= 4 && READ_BE_UINT32(data) == MKID_BE('LDOM'))
		loadBinary(data, cmap);
	else {
		TextSplitter ts(data, len);
		loadText(ts, cmap);
	}
}

void Model::reload(const CMap &cmap) {
	// Load the new colormap
	for (int i = 0; i < _numMaterials; i++)
		_materials[i] = g_resourceloader->loadMaterial(_materialNames[i], cmap);
	for (int i = 0; i < _numGeosets; i++)
		_geosets[i].changeMaterials(_materials);
}

void Model::loadBinary(const char *data, const CMap &cmap) {
	_numMaterials = READ_LE_UINT32(data + 4);
	data += 8;
	_materials = new ResPtr<Material>[_numMaterials];
	_materialNames = new char[_numMaterials][32];
	for (int i = 0; i < _numMaterials; i++) {
		strcpy(_materialNames[i], data);
		_materials[i] = g_resourceloader->loadMaterial(_materialNames[i], cmap);
		data += 32;
	}
	data += 32; // skip name
	_numGeosets = READ_LE_UINT32(data + 4);
	data += 8;
	_geosets = new Geoset[_numGeosets];
	for (int i = 0; i < _numGeosets; i++)
		_geosets[i].loadBinary(data, _materials);
	_numHierNodes = READ_LE_UINT32(data + 4);
	data += 8;
	_rootHierNode = new HierNode[_numHierNodes];
	for (int i = 0; i < _numHierNodes; i++)
		_rootHierNode[i].loadBinary(data, _rootHierNode, _geosets[0]);
	_radius = get_float(data);
	_insertOffset = Graphics::get_vector3d(data + 40);
}

Model::~Model() {
	delete[] _materials;
	delete[] _materialNames;
	delete[] _geosets;
	delete[] _rootHierNode;
}

void Model::Geoset::loadBinary(const char *&data, ResPtr<Material> *materials) {
	_numMeshes = READ_LE_UINT32(data);
	data += 4;
	_meshes = new Mesh[_numMeshes];
	for (int i = 0; i < _numMeshes; i++)
		_meshes[i].loadBinary(data, materials);
}

Model::Geoset::~Geoset() {
	delete[] _meshes;
}

void Model::Mesh::loadBinary(const char *&data, ResPtr<Material> *materials) {
	memcpy(_name, data, 32);
	_geometryMode = READ_LE_UINT32(data + 36);
	_lightingMode = READ_LE_UINT32(data + 40);
	_textureMode = READ_LE_UINT32(data + 44);
	_numVertices = READ_LE_UINT32(data + 48);
	_numTextureVerts = READ_LE_UINT32(data + 52);
	_numFaces = READ_LE_UINT32(data + 56);
	_vertices = new float[3 * _numVertices];
	_verticesI = new float[_numVertices];
	_vertNormals = new float[3 * _numVertices];
	_textureVerts = new float[2 * _numTextureVerts];
	data += 60;
	for (int i = 0; i < 3 * _numVertices; i++) {
		_vertices[i] = get_float(data);
		data += 4;
	}
	for (int i = 0; i < 2 * _numTextureVerts; i++) {
		_textureVerts[i] = get_float(data);
		data += 4;
	}
	for (int i = 0; i < _numVertices; i++) {
		_verticesI[i] = get_float(data);
		data += 4;
	}
	data += _numVertices * 4;
	_faces = new Face[_numFaces];
	_materialid = new int[_numFaces];
	for (int i = 0; i < _numFaces; i++)
		_materialid[i] = _faces[i].loadBinary(data, materials);
	_vertNormals = new float[3 * _numVertices];
	for (int i = 0; i < 3 * _numVertices; i++) {
		_vertNormals[i] = get_float(data);
		data += 4;
	}
	_shadow = READ_LE_UINT32(data);
	_radius = get_float(data + 8);
	data += 36;
}

Model::Mesh::~Mesh() {
	delete[] _vertices;
	delete[] _verticesI;
	delete[] _vertNormals;
	delete[] _textureVerts;
	delete[] _faces;
}

void Model::Mesh::update() {
}

void Model::Face::changeMaterial(ResPtr<Material> material) {
	_material = material;
}

int Model::Face::loadBinary(const char *&data, ResPtr<Material> *materials) {
	_type = READ_LE_UINT32(data + 4);
	_geo = READ_LE_UINT32(data + 8);
	_light = READ_LE_UINT32(data + 12);
	_tex = READ_LE_UINT32(data + 16);
	_numVertices = READ_LE_UINT32(data + 20);
	int texPtr = READ_LE_UINT32(data + 28);
	int materialPtr = READ_LE_UINT32(data + 32);
	_extraLight = get_float(data + 48);
	_normal = Graphics::get_vector3d(data + 64);
	data += 76;

	_vertices = new int[_numVertices];
	for (int i = 0; i < _numVertices; i++) {
		_vertices[i] = READ_LE_UINT32(data);
		data += 4;
	}
	if (texPtr == 0)
		_texVertices = NULL;
	else {
		_texVertices = new int[_numVertices];
		for (int i = 0; i < _numVertices; i++) {
			_texVertices[i] = READ_LE_UINT32(data);
			data += 4;
		}
	}
	if (materialPtr == 0)
		_material = 0;
	else {
		_material = materials[READ_LE_UINT32(data)];
		materialPtr = READ_LE_UINT32(data);
		data += 4;
	}
	return materialPtr;
}

Model::Face::~Face() {
	delete[] _vertices;
	delete[] _texVertices;
}

void Model::HierNode::loadBinary(const char *&data, Model::HierNode *hierNodes, const Geoset &g) {
	memcpy(_name, data, 64);
	_flags = READ_LE_UINT32(data + 64);
	_type = READ_LE_UINT32(data + 72);
	int meshNum = READ_LE_UINT32(data + 76);
	if (meshNum < 0)
		_mesh = NULL;
	else
		_mesh = g._meshes + meshNum;
	_depth = READ_LE_UINT32(data + 80);
	int parentPtr = READ_LE_UINT32(data + 84);
	_numChildren = READ_LE_UINT32(data + 88);
	int childPtr = READ_LE_UINT32(data + 92);
	int siblingPtr = READ_LE_UINT32(data + 96);
	_pivot = Graphics::get_vector3d(data + 100);
	_pos = Graphics::get_vector3d(data + 112);
	_pitch = get_float(data + 124);
	_yaw = get_float(data + 128);
	_roll = get_float(data + 132);
	_animPos = _pos;
	_animPitch = _pitch;
	_animYaw = _yaw;
	_animRoll = _roll;
	_priority = -1;
	_totalWeight = 1;

	data += 184;

	if (parentPtr != 0) {
		_parent = hierNodes + READ_LE_UINT32(data);
		data += 4;
	} else
		_parent = NULL;
	if (childPtr != 0) {
		_child = hierNodes + READ_LE_UINT32(data);
		data += 4;
	} else
		_child = NULL;
	if (siblingPtr != 0) {
		_sibling = hierNodes + READ_LE_UINT32(data);
		data += 4;
	} else
		_sibling = NULL;

	_meshVisible = true;
	_hierVisible = true;
	_totalWeight = 1;
	_initialized = true;
}

void Model::draw() const {
	_rootHierNode->draw();
}

Model::HierNode *Model::copyHierarchy() {
	HierNode *result = new HierNode[_numHierNodes];
	memcpy(result, _rootHierNode, _numHierNodes * sizeof(HierNode));
	// Now adjust pointers
	for (int i = 0; i < _numHierNodes; i++) {
		if (result[i]._parent)
			result[i]._parent = result + (_rootHierNode[i]._parent - _rootHierNode);
		if (result[i]._child)
			result[i]._child = result + (_rootHierNode[i]._child - _rootHierNode);
		if (result[i]._sibling)
			result[i]._sibling = result + (_rootHierNode[i]._sibling - _rootHierNode);
	}
	return result;
}

void Model::loadText(TextSplitter &ts, const CMap &cmap) {
	ts.expectString("section: header");
	int major, minor;
	ts.scanString("3do %d.%d", 2, &major, &minor);
	ts.expectString("section: modelresource");
	ts.scanString("materials %d", 1, &_numMaterials);
	_materials = new ResPtr<Material>[_numMaterials];
	_materialNames = new char[_numMaterials][32];
	for (int i = 0; i < _numMaterials; i++) {
		char materialName[32];
		int num;

		ts.scanString("%d: %32s", 2, &num, materialName);
		_materials[num] = g_resourceloader->loadMaterial(materialName, cmap);
		strcpy(_materialNames[num], materialName);
	}

	ts.expectString("section: geometrydef");
	ts.scanString("radius %f", 1, &_radius);
	ts.scanString("insert offset %f %f %f", 3, &_insertOffset.x(), &_insertOffset.y(), &_insertOffset.z());
	ts.scanString("geosets %d", 1, &_numGeosets);
	_geosets = new Geoset[_numGeosets];
	for (int i = 0; i < _numGeosets; i++) {
		int num;
		ts.scanString("geoset %d", 1, &num);
		_geosets[num].loadText(ts, _materials);
	}

	ts.expectString("section: hierarchydef");
	ts.scanString("hierarchy nodes %d", 1, &_numHierNodes);
	_rootHierNode = new HierNode[_numHierNodes];
	for (int i = 0; i < _numHierNodes; i++) {
		int num, mesh, parent, child, sibling, numChildren;
		unsigned int flags, type;
		float x, y, z, pitch, yaw, roll, pivotx, pivoty, pivotz;
		char name[64];
		ts.scanString(" %d: %x %x %d %d %d %d %d %f %f %f %f %f %f %f %f %f %64s",
			18, &num, &flags, &type, &mesh, &parent, &child, &sibling,
			&numChildren, &x, &y, &z, &pitch, &yaw, &roll, &pivotx, &pivoty, &pivotz, name);
		_rootHierNode[num]._flags = (int)flags;
		_rootHierNode[num]._type = (int)type;
		if (mesh < 0)
			_rootHierNode[num]._mesh = NULL;
		else
			_rootHierNode[num]._mesh = _geosets[0]._meshes + mesh;
		if (parent >= 0) {
			_rootHierNode[num]._parent = _rootHierNode + parent;
			_rootHierNode[num]._depth = _rootHierNode[parent]._depth + 1;
		} else {
			_rootHierNode[num]._parent = NULL;
			_rootHierNode[num]._depth = 0;
		}
		if (child >= 0)
			_rootHierNode[num]._child = _rootHierNode + child;
		else
			_rootHierNode[num]._child = NULL;
		if (sibling >= 0)
			_rootHierNode[num]._sibling = _rootHierNode + sibling;
		else
			_rootHierNode[num]._sibling = NULL;

		_rootHierNode[num]._numChildren = numChildren;
		_rootHierNode[num]._pos = Graphics::Vector3d(x, y, z);
		_rootHierNode[num]._pitch = pitch;
		_rootHierNode[num]._yaw = yaw;
		_rootHierNode[num]._roll = roll;
		_rootHierNode[num]._pivot = Graphics::Vector3d(pivotx, pivoty, pivotz);
		_rootHierNode[num]._meshVisible = true;
		_rootHierNode[num]._hierVisible = true;
		_rootHierNode[num]._totalWeight = 1;
	}

	if (!ts.eof() && (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL))
		warning("Unexpected junk at end of model text");
}

void Model::Geoset::changeMaterials(ResPtr<Material> *materials) {
	for (int i = 0; i < _numMeshes; i++)
		_meshes[i].changeMaterials(materials);
}

void Model::Geoset::loadText(TextSplitter &ts, ResPtr<Material> *materials) {
	ts.scanString("meshes %d", 1, &_numMeshes);
	_meshes = new Mesh[_numMeshes];
	for (int i = 0; i < _numMeshes; i++) {
		int num;
		ts.scanString("mesh %d", 1, &num);
		_meshes[num].loadText(ts, materials);
	}
}

void Model::Mesh::changeMaterials(ResPtr<Material> *materials) {
	for (int i = 0; i < _numFaces; i++)
		_faces[i].changeMaterial(materials[_materialid[i]]);
}

void Model::Mesh::loadText(TextSplitter &ts, ResPtr<Material> *materials) {
	ts.scanString("name %32s", 1, _name);
	ts.scanString("radius %f", 1, &_radius);

	// In data001/rope_scale.3do, the shadow line is missing
	if (sscanf(ts.currentLine(), "shadow %d", &_shadow) < 1) {
		_shadow = 0;
	} else
		ts.nextLine();
	ts.scanString("geometrymode %d", 1, &_geometryMode);
	ts.scanString("lightingmode %d", 1, &_lightingMode);
	ts.scanString("texturemode %d", 1, &_textureMode);
	ts.scanString("vertices %d", 1, &_numVertices);
	_vertices = new float[3 * _numVertices];
	_verticesI = new float[_numVertices];
	_vertNormals = new float[3 * _numVertices];

	for (int i = 0; i < _numVertices; i++) {
		int num;
		float x, y, z, ival;
		ts.scanString(" %d: %f %f %f %f", 5, &num, &x, &y, &z, &ival);
		_vertices[3 * num] = x;
		_vertices[3 * num + 1] = y;
		_vertices[3 * num + 2] = z;
		_verticesI[num] = ival;
	}

	ts.scanString("texture vertices %d", 1, &_numTextureVerts);
	_textureVerts = new float[2 * _numTextureVerts];

	for (int i = 0; i < _numTextureVerts; i++) {
		int num;
		float x, y;
		ts.scanString(" %d: %f %f", 3, &num, &x, &y);
		_textureVerts[2 * num] = x;
		_textureVerts[2 * num + 1] = y;
	}

	ts.expectString("vertex normals");
	for (int i = 0; i < _numVertices; i++) {
		int num;
		float x, y, z;
		ts.scanString(" %d: %f %f %f", 4, &num, &x, &y, &z);
		_vertNormals[3 * num] = x;
		_vertNormals[3 * num + 1] = y;
		_vertNormals[3 * num + 2] = z;
	}

	ts.scanString("faces %d", 1, &_numFaces);
	_faces = new Face[_numFaces];
	_materialid = new int[_numFaces];
	for (int i = 0; i < _numFaces; i++) {
		int num, materialid, geo, light, tex, verts;
		unsigned int type;
		float extralight;
		int readlen;

		if (ts.eof())
			error("Expected face data, got EOF");

		if (sscanf(ts.currentLine(), " %d: %d %x %d %d %d %f %d%n", &num, &materialid, &type, &geo, &light, &tex, &extralight, &verts, &readlen) < 8)
			error("Expected face data, got '%s'", ts.currentLine());

		assert(materialid != -1);
		_materialid[num] = materialid;
		_faces[num]._material = materials[materialid];
		_faces[num]._type = (int)type;
		_faces[num]._geo = geo;
		_faces[num]._light = light;
		_faces[num]._tex = tex;
		_faces[num]._extraLight = extralight;
		_faces[num]._numVertices = verts;
		_faces[num]._vertices = new int[verts];
		_faces[num]._texVertices = new int[verts];
		for (int j = 0; j < verts; j++) {
			int readlen2;

			if (sscanf(ts.currentLine() + readlen, " %d, %d%n", _faces[num]._vertices + j, _faces[num]._texVertices + j, &readlen2) < 2)
				error("Could not read vertex indices in line '%s'",

			ts.currentLine());
			readlen += readlen2;
		}
		ts.nextLine();
	}

	ts.expectString("face normals");
	for (int i = 0; i < _numFaces; i++) {
		int num;
		float x, y, z;
		ts.scanString(" %d: %f %f %f", 4, &num, &x, &y, &z);
		_faces[num]._normal = Graphics::Vector3d(x, y, z);
	}
}

void Model::HierNode::draw() const {
	g_driver->drawHierachyNode(this);
}

void Model::HierNode::addChild(HierNode *child) {
	HierNode **childPos = &_child;
	while (*childPos)
		childPos = &(*childPos)->_sibling;
	*childPos = child;
	child->_parent = this;
}

void Model::HierNode::removeChild(HierNode *child) {
	HierNode **childPos = &_child;
	while (*childPos && *childPos != child)
		childPos = &(*childPos)->_sibling;
	if (*childPos) {
		*childPos = child->_sibling;
		child->_parent = NULL;
	}
}

void Model::HierNode::setMatrix(Graphics::Matrix4 matrix) {
	_matrix = matrix;
}

void Model::HierNode::update() {
	if (!_initialized)
		return;

	_localMatrix._pos.set(_animPos.x() / _totalWeight, _animPos.y() / _totalWeight, _animPos.z() / _totalWeight);
	_localMatrix._rot.buildFromPitchYawRoll(_animPitch / _totalWeight, _animYaw / _totalWeight, _animRoll / _totalWeight);

	_matrix *= _localMatrix;

	_pivotMatrix = _matrix;

	_pivotMatrix.translate(_pivot.x(), _pivot.y(), _pivot.z());

	if (_mesh) {
		_mesh->_matrix = _pivotMatrix;
	}

	if (_child) {
		_child->setMatrix(_matrix);
		_child->update();
	}
}

void Model::Mesh::draw() const {
	int winX1, winY1, winX2, winY2;
	g_driver->getBoundingBoxPos(this, &winX1, &winY1, &winX2, &winY2);
	if (winX1 != -1 && winY1 != -1 && winX2 != -1 && winY2 != -1) {
		g_winX1 = MIN(g_winX1, winX1);
		g_winY1 = MIN(g_winY1, winY1);
		g_winX2 = MAX(g_winX2, winX2);
		g_winY2 = MAX(g_winY2, winY2);
	}

	for (int i = 0; i < _numFaces; i++)
		_faces[i].draw(_vertices, _vertNormals, _textureVerts);
}

void Model::Face::draw(float *vertices, float *vertNormals, float *textureVerts) const {
	_material->select();
	g_driver->drawModelFace(this, vertices, vertNormals, textureVerts);
}

} // end of namespace Grim
