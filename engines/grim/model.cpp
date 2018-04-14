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

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/func.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/model.h"
#include "engines/grim/material.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/resource.h"
#include "engines/grim/colormap.h"
#include "engines/grim/sprite.h"

namespace Grim {


/**
 * @class Model
 */
Model::Model(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap, Model *parent) :
		Object(), _parent(parent), _numMaterials(0), _numGeosets(0), _cmap(cmap), _fname(filename) {

	if (data->readUint32BE() == MKTAG('L','D','O','M'))
		loadBinary(data);
	else {
		data->seek(0, SEEK_SET);
		TextSplitter ts(_fname, data);
		loadText(&ts);
	}

	Math::Vector3d max;

	_rootHierNode->update();
	bool first = true;
	for (int i = 0; i < _numHierNodes; ++i) {
		ModelNode &node = _rootHierNode[i];
		if (node._mesh) {
			g_driver->createMesh(node._mesh);
			Mesh &mesh = *node._mesh;
			Math::Vector3d p = mesh._matrix.getPosition();
			float x = p.x();
			float y = p.y();
			float z = p.z();
			for (int k = 0; k < mesh._numVertices * 3; k += 3) {
				if (first || mesh._vertices[k] + x < _bboxPos.x())
					_bboxPos.x() = mesh._vertices[k] + x;
				if (first || mesh._vertices[k + 1] + y < _bboxPos.y())
					_bboxPos.y() = mesh._vertices[k + 1] + y;
				if (first || mesh._vertices[k + 2] + z < _bboxPos.z())
					_bboxPos.z() = mesh._vertices[k + 2] + z;

				if (first || mesh._vertices[k] + x > max.x())
					max.x() = mesh._vertices[k] + x;
				if (first || mesh._vertices[k + 1] + y > max.y())
					max.y() = mesh._vertices[k + 1] + y;
				if (first || mesh._vertices[k + 2] + z > max.z())
					max.z() = mesh._vertices[k + 2] + z;

				first = false;
			}
		}
	}

	_bboxSize = max - _bboxPos;
}

Model::~Model() {
	for (int i = 0; i < _numMaterials; ++i) {
		if (!_materialsShared[i]) {
			delete _materials[i];
		}
	}
	delete[] _materials;
	delete[] _materialNames;
	delete[] _materialsShared;
	delete[] _geosets;
	delete[] _rootHierNode;
	g_resourceloader->uncacheModel(this);
}

void Model::loadBinary(Common::SeekableReadStream *data) {
	_numMaterials = data->readUint32LE();
	_materials = new Material*[_numMaterials];
	_materialNames = new char[_numMaterials][32];
	_materialsShared = new bool[_numMaterials];
	for (int i = 0; i < _numMaterials; i++) {
		data->read(_materialNames[i], 32);
		_materialsShared[i] = false;
		_materials[i] = nullptr;
		loadMaterial(i, _cmap);
	}
	data->seek(32, SEEK_CUR); // skip name
	data->seek(4, SEEK_CUR);
	_numGeosets = data->readUint32LE();
	_geosets = new Geoset[_numGeosets];
	for (int i = 0; i < _numGeosets; i++)
		_geosets[i].loadBinary(data, _materials);
	data->seek(4, SEEK_CUR);
	_numHierNodes = data->readUint32LE();
	_rootHierNode = new ModelNode[_numHierNodes];
	for (int i = 0; i < _numHierNodes; i++) {
		_rootHierNode[i].loadBinary(data, _rootHierNode, &_geosets[0]);
	}
	_radius = data->readFloatLE();
	data->seek(36, SEEK_CUR);
	_insertOffset.readFromStream(data);
}

void Model::loadText(TextSplitter *ts) {
	ts->expectString("section: header");
	int major, minor;
	ts->scanString("3do %d.%d", 2, &major, &minor);
	ts->expectString("section: modelresource");
	ts->scanString("materials %d", 1, &_numMaterials);
	_materials = new Material*[_numMaterials];
	_materialNames = new char[_numMaterials][32];
	_materialsShared = new bool[_numMaterials];
	for (int i = 0; i < _numMaterials; i++) {
		char materialName[32];
		int num;
		_materialsShared[i] = false;
		_materials[i] = nullptr;

		ts->scanString("%d: %32s", 2, &num, materialName);
		strcpy(_materialNames[num], materialName);
		loadMaterial(num, _cmap);
	}

	ts->expectString("section: geometrydef");
	ts->scanString("radius %f", 1, &_radius);
	ts->scanString("insert offset %f %f %f", 3, &_insertOffset.x(), &_insertOffset.y(), &_insertOffset.z());
	ts->scanString("geosets %d", 1, &_numGeosets);
	_geosets = new Geoset[_numGeosets];
	for (int i = 0; i < _numGeosets; i++) {
		int num;
		ts->scanString("geoset %d", 1, &num);
		_geosets[num].loadText(ts, _materials);
	}

	ts->expectString("section: hierarchydef");
	ts->scanString("hierarchy nodes %d", 1, &_numHierNodes);
	_rootHierNode = new ModelNode[_numHierNodes];
	for (int i = 0; i < _numHierNodes; i++) {
		int num, mesh, parent, child, sibling, numChildren;
		unsigned int flags, type;
		float x, y, z, pitch, yaw, roll, pivotx, pivoty, pivotz;
		char name[64];
		ts->scanString(" %d: %x %x %d %d %d %d %d %f %f %f %f %f %f %f %f %f %64s",
					   18, &num, &flags, &type, &mesh, &parent, &child, &sibling,
				 &numChildren, &x, &y, &z, &pitch, &yaw, &roll, &pivotx, &pivoty, &pivotz, name);
		_rootHierNode[num]._flags = (int)flags;
		_rootHierNode[num]._type = (int)type;
		if (mesh < 0)
			_rootHierNode[num]._mesh = nullptr;
		else
			_rootHierNode[num]._mesh = &_geosets[0]._meshes[mesh];
		if (parent >= 0) {
			_rootHierNode[num]._parent = &_rootHierNode[parent];
			_rootHierNode[num]._depth = _rootHierNode[parent]._depth + 1;
		} else {
			_rootHierNode[num]._parent = nullptr;
			_rootHierNode[num]._depth = 0;
		}
		if (child >= 0)
			_rootHierNode[num]._child = &_rootHierNode[child];
		else
			_rootHierNode[num]._child = nullptr;
		if (sibling >= 0)
			_rootHierNode[num]._sibling = &_rootHierNode[sibling];
		else
			_rootHierNode[num]._sibling = nullptr;

		_rootHierNode[num]._numChildren = numChildren;
		_rootHierNode[num]._pos = Math::Vector3d(x, y, z);
		_rootHierNode[num]._rot = Math::Quaternion::fromEuler(yaw, pitch, roll, Math::EO_ZXY);
		_rootHierNode[num]._animRot = _rootHierNode[num]._rot;
		_rootHierNode[num]._animPos = _rootHierNode[num]._pos;
		_rootHierNode[num]._pivot = Math::Vector3d(pivotx, pivoty, pivotz);
		_rootHierNode[num]._meshVisible = true;
		_rootHierNode[num]._hierVisible = true;
		_rootHierNode[num]._sprite = nullptr;
		_rootHierNode[num]._initialized = true;
	}

	if (!ts->isEof())
		Debug::warning(Debug::Models, "Unexpected junk at end of model text");
}

void Model::draw() const {
	_rootHierNode->draw();
}

ModelNode *Model::getHierarchy() const {
	return _rootHierNode;
}

void Model::reload(CMap *cmap) {
	// Load the new colormap
	for (int i = 0; i < _numMaterials; i++) {
		loadMaterial(i, cmap);
	}
	for (int i = 0; i < _numGeosets; i++)
		_geosets[i].changeMaterials(_materials);
	_cmap = cmap;
}

void Model::loadMaterial(int index, CMap *cmap) {
	Material *mat = nullptr;
	if (!_materialsShared[index]) {
		mat = _materials[index];
	}
	_materials[index] = nullptr;
	if (_parent) {
		_materials[index] = _parent->findMaterial(_materialNames[index], cmap);
		if (_materials[index]) {
			_materialsShared[index] = true;
		}
	}
	if (!_materials[index]) {
		if (mat && cmap->getFilename() == _cmap->getFilename()) {
			_materials[index] = mat;
		} else {
			_materials[index] = g_resourceloader->loadMaterial(_materialNames[index], cmap, false);
		}
		_materialsShared[index] = false;
	}
	if (mat != _materials[index]) {
		delete mat;
	}
}

Material *Model::findMaterial(const char *name, CMap *cmap) const {
	for (int i = 0; i < _numMaterials; ++i) {
		if (scumm_stricmp(name, _materialNames[i]) == 0) {
			if (cmap->getFilename() != _cmap->getFilename())
				_materials[i]->reload(cmap);
			return _materials[i];
		}
	}

	return nullptr;
}

/**
 * @class Model::Geoset
 */
Model::Geoset::~Geoset() {
	delete[] _meshes;
}

void Model::Geoset::loadBinary(Common::SeekableReadStream *data, Material *materials[]) {
	_numMeshes =  data->readUint32LE();
	_meshes = new Mesh[_numMeshes];
	for (int i = 0; i < _numMeshes; i++)
		_meshes[i].loadBinary(data, materials);
}

void Model::Geoset::loadText(TextSplitter *ts, Material *materials[]) {
	ts->scanString("meshes %d", 1, &_numMeshes);
	_meshes = new Mesh[_numMeshes];
	for (int i = 0; i < _numMeshes; i++) {
		int num;
		ts->scanString("mesh %d", 1, &num);
		_meshes[num].loadText(ts, materials);
	}
}

void Model::Geoset::changeMaterials(Material *materials[]) {
	for (int i = 0; i < _numMeshes; i++)
		_meshes[i].changeMaterials(materials);
}

/**
 * @class MeshFace
 */
MeshFace::MeshFace() :
		_material(nullptr), _type(0), _geo(0), _light(0), _tex(0),
		_extraLight(0), _numVertices(0), _vertices(nullptr),
		_texVertices(nullptr), _userData(nullptr) {
}

MeshFace::~MeshFace() {
	delete[] _vertices;
	delete[] _texVertices;
}

void MeshFace::stealData(MeshFace &other) {
	*this = other;
	other._vertices = nullptr;
	other._texVertices = nullptr;
}

int MeshFace::loadBinary(Common::SeekableReadStream *data, Material *materials[]) {
	data->seek(4, SEEK_CUR);
	_type = data->readUint32LE();
	_geo = data->readUint32LE();
	_light = data->readUint32LE();
	_tex = data->readUint32LE();
	_numVertices = data->readUint32LE();
	data->seek(4, SEEK_CUR);
	int texPtr = data->readUint32LE();
	int materialPtr = data->readUint32LE();
	data->seek(12, SEEK_CUR);
	_extraLight = data->readFloatLE();
	data->seek(12, SEEK_CUR);
	_normal.readFromStream(data);

	_vertices = new int[_numVertices];
	for (int i = 0; i < _numVertices; i++) {
		_vertices[i] = data->readUint32LE();
	}

	if (texPtr != 0) {
		_texVertices = new int[_numVertices];
		for (int i = 0; i < _numVertices; i++) {
			_texVertices[i] = data->readUint32LE();
		}
	}

	if (materialPtr != 0) {
		materialPtr = data->readUint32LE();
		_material = materials[materialPtr];
	}

	return materialPtr;
}

int MeshFace::loadText(TextSplitter *ts, Material *materials[], int offset) {
	int readlen, materialid;

	if (ts->isEof())
		error("Expected face data, got EOF");

	ts->scanStringAtOffsetNoNewLine(offset, "%d %x %d %d %d %f %d%n", 7, &materialid, &_type, &_geo, &_light, &_tex, &_extraLight, &_numVertices, &readlen);
	readlen += offset;

	assert(materialid != -1);
	_material = materials[materialid];
	_vertices = new int[_numVertices];
	_texVertices = new int[_numVertices];
	for (int i = 0; i < _numVertices; ++i) {
		int readlen2;

		ts->scanStringAtOffsetNoNewLine(readlen, " %d, %d%n", 2, &_vertices[i], &_texVertices[i], &readlen2);
		readlen += readlen2;
	}
	ts->nextLine();

	return materialid;
}

void MeshFace::changeMaterial(Material *material) {
	_material = material;
}

void MeshFace::draw(const Mesh *mesh) const {
	if (_light == 0 && !g_driver->isShadowModeActive())
		g_driver->disableLights();

	_material->select();
	g_driver->drawModelFace(mesh, this);

	if (_light == 0 && !g_driver->isShadowModeActive())
		g_driver->enableLights();
}

/**
 * @class Mesh
 */
Mesh::Mesh() :
		_numFaces(0), _radius(0.0f), _shadow(0), _geometryMode(0),
		_lightingMode(0), _textureMode(0), _numVertices(0), _materialid(nullptr),
		_vertices(nullptr), _verticesI(nullptr), _vertNormals(nullptr),
		_numTextureVerts(0), _textureVerts(nullptr), _faces(nullptr), _userData(nullptr) {
	_name[0] = '\0';

}


Mesh::~Mesh() {
	g_driver->destroyMesh(this);

	delete[] _vertices;
	delete[] _verticesI;
	delete[] _vertNormals;
	delete[] _textureVerts;
	delete[] _faces;
	delete[] _materialid;
}

void Mesh::loadBinary(Common::SeekableReadStream *data, Material *materials[]) {
	data->read(_name, 32);
	data->seek(4, SEEK_CUR);
	_geometryMode = data->readUint32LE();
	_lightingMode = data->readUint32LE();
	_textureMode = data->readUint32LE();
	_numVertices = data->readUint32LE();
	_numTextureVerts = data->readUint32LE();
	_numFaces =  data->readUint32LE();
	_vertices = new float[3 * _numVertices];
	_verticesI = new float[_numVertices];
	_vertNormals = new float[3 * _numVertices];
	_textureVerts = new float[2 * _numTextureVerts];
	_faces = new MeshFace[_numFaces];
	_materialid = new int[_numFaces];
	for (int i = 0; i < 3 * _numVertices; i++) {
		_vertices[i] = data->readFloatLE();
	}
	for (int i = 0; i < 2 * _numTextureVerts; i++) {
		_textureVerts[i] = data->readFloatLE();
	}
	for (int i = 0; i < _numVertices; i++) {
		_verticesI[i] = data->readFloatLE();
	}
	data->seek(_numVertices * 4, SEEK_CUR);
	for (int i = 0; i < _numFaces; i++)
		_materialid[i] = _faces[i].loadBinary(data, materials);
	for (int i = 0; i < 3 * _numVertices; i++) {
		_vertNormals[i] = data->readFloatLE();
	}
	_shadow = data->readUint32LE();
	data->seek(4, SEEK_CUR);
	_radius = data->readFloatLE();
	data->seek(24, SEEK_CUR);
	sortFaces();
}

void Mesh::loadText(TextSplitter *ts, Material *materials[]) {
	ts->scanString("name %32s", 1, _name);
	ts->scanString("radius %f", 1, &_radius);

	// In data001/rope_scale.3do, the shadow line is missing
	if (sscanf(ts->getCurrentLine(), "shadow %d", &_shadow) < 1) {
		_shadow = 0;
	} else
		ts->nextLine();
	ts->scanString("geometrymode %d", 1, &_geometryMode);
	ts->scanString("lightingmode %d", 1, &_lightingMode);
	ts->scanString("texturemode %d", 1, &_textureMode);
	ts->scanString("vertices %d", 1, &_numVertices);
	_vertices = new float[3 * _numVertices];
	_verticesI = new float[_numVertices];
	_vertNormals = new float[3 * _numVertices];

	for (int i = 0; i < _numVertices; i++) {
		int num;
		float x, y, z, ival;
		ts->scanString(" %d: %f %f %f %f", 5, &num, &x, &y, &z, &ival);
		_vertices[3 * num] = x;
		_vertices[3 * num + 1] = y;
		_vertices[3 * num + 2] = z;
		_verticesI[num] = ival;
	}

	ts->scanString("texture vertices %d", 1, &_numTextureVerts);
	_textureVerts = new float[2 * _numTextureVerts];

	for (int i = 0; i < _numTextureVerts; i++) {
		int num;
		float x, y;
		ts->scanString(" %d: %f %f", 3, &num, &x, &y);
		_textureVerts[2 * num] = x;
		_textureVerts[2 * num + 1] = y;
	}

	ts->expectString("vertex normals");
	for (int i = 0; i < _numVertices; i++) {
		int num;
		float x, y, z;
		ts->scanString(" %d: %f %f %f", 4, &num, &x, &y, &z);
		_vertNormals[3 * num] = x;
		_vertNormals[3 * num + 1] = y;
		_vertNormals[3 * num + 2] = z;
	}

	ts->scanString("faces %d", 1, &_numFaces);
	_faces = new MeshFace[_numFaces];
	_materialid = new int[_numFaces];
	for (int i = 0; i < _numFaces; i++) {
		int num, readlen;
		ts->scanStringNoNewLine(" %d:%n ", 1, &num, &readlen);
		_materialid[num] = _faces[num].loadText(ts, materials, readlen);
	}

	ts->expectString("face normals");
	for (int i = 0; i < _numFaces; i++) {
		int num;
		float x, y, z;
		ts->scanString(" %d: %f %f %f", 4, &num, &x, &y, &z);
		_faces[num].setNormal(Math::Vector3d(x, y, z));
	}
	sortFaces();
}

void Mesh::sortFaces() {
	if (_numFaces < 2)
		return;

	MeshFace *newFaces = new MeshFace[_numFaces];
	int *newMaterialid = new int[_numFaces];
	bool *copied = new bool[_numFaces];
	for (int i = 0; i < _numFaces; ++i)
		copied[i] = false;

	for (int cur = 0, writeIdx = 0; cur < _numFaces; ++cur) {
		if (copied[cur])
			continue;

		for (int other = cur; other < _numFaces; ++other) {
			if (_faces[cur].getMaterial() == _faces[other].getMaterial() && !copied[other]) {
				copied[other] = true;
				newFaces[writeIdx].stealData(_faces[other]);
				newMaterialid[writeIdx] = _materialid[other];
				writeIdx++;
			}
		}
	}

	delete[] _faces;
	_faces = newFaces;
	delete[] _materialid;
	_materialid = newMaterialid;
	delete[] copied;
}

void Mesh::update() {
}

void Mesh::changeMaterials(Material *materials[]) {
	for (int i = 0; i < _numFaces; i++)
		_faces[i].changeMaterial(materials[_materialid[i]]);
}

void Mesh::draw() const {
	if (_lightingMode == 0)
		g_driver->disableLights();

	g_driver->drawMesh(this);

	if (_lightingMode == 0)
		g_driver->enableLights();
}

void Mesh::getBoundingBox(int *x1, int *y1, int *x2, int *y2) const {
	int winX1, winY1, winX2, winY2;
	g_driver->getScreenBoundingBox(this, &winX1, &winY1, &winX2, &winY2);
	if (winX1 != -1 && winY1 != -1 && winX2 != -1 && winY2 != -1) {
		*x1 = MIN(*x1, winX1);
		*y1 = MIN(*y1, winY1);
		*x2 = MAX(*x2, winX2);
		*y2 = MAX(*y2, winY2);
	}
}

/**
 * @class ModelNode
 */
ModelNode::ModelNode() :
		_initialized(false), _needsUpdate(true), _mesh(nullptr), _flags(0), _type(0),
		_depth(0), _numChildren(0), _parent(nullptr), _child(nullptr), _sprite(nullptr),
		_sibling(nullptr), _meshVisible(false), _hierVisible(false) {
	_name[0] = '\0';
}

ModelNode::~ModelNode() {
	ModelNode *child = _child;
	while (child) {
		child->_parent = nullptr;
		child = child->_sibling;
	}
}

void ModelNode::loadBinary(Common::SeekableReadStream *data, ModelNode *hierNodes, const Model::Geoset *g) {
	data->read(_name, 64);
	_flags = data->readUint32LE();
	data->seek(4, SEEK_CUR);
	_type = data->readUint32LE();
	int meshNum = data->readUint32LE();
	if (meshNum < 0)
		_mesh = nullptr;
	else
		_mesh = g->_meshes + meshNum;
	_depth = data->readUint32LE();
	int parentPtr = data->readUint32LE();
	_numChildren = data->readUint32LE();
	int childPtr = data->readUint32LE();
	int siblingPtr = data->readUint32LE();
	_pivot.readFromStream(data);
	_pos.readFromStream(data);
	float pitch = data->readFloatLE();
	float yaw = data->readFloatLE();
	float roll = data->readFloatLE();
	_rot = Math::Quaternion::fromEuler(yaw, pitch, roll, Math::EO_ZXY);
	_animRot = _rot;
	_animPos = _pos;
	_sprite = nullptr;

	data->seek(48, SEEK_CUR);

	if (parentPtr != 0)
		_parent = hierNodes + data->readUint32LE();
	else
		_parent = nullptr;

	if (childPtr != 0)
		_child = hierNodes + data->readUint32LE();
	else
		_child = nullptr;

	if (siblingPtr != 0)
		_sibling = hierNodes + data->readUint32LE();
	else
		_sibling = nullptr;

	_meshVisible = true;
	_hierVisible = true;
	_initialized = true;
}

void ModelNode::draw() const {
	if (_sibling || _child) {
		translateViewpointStart();
	}
	translateViewpoint();
	if (_hierVisible) {
		if (_child) {
			translateViewpointStart();
		}
		g_driver->translateViewpoint(_pivot);

		if (!g_driver->isShadowModeActive()) {
			Sprite *sprite = _sprite;
			while (sprite) {
				sprite->draw();
				sprite = sprite->_next;
			}
		}

		if (_mesh && _meshVisible) {
			_mesh->draw();
		}

		if (_child) {
			translateViewpointFinish();
			_child->draw();
		}
	}

	if (_sibling || _child) {
		translateViewpointFinish();
	}
	if (_sibling) {
		_sibling->draw();
	}
}

void ModelNode::getBoundingBox(int *x1, int *y1, int *x2, int *y2) const {
	if (_sibling || _child) {
		translateViewpointStart();
	}
	translateViewpoint();
	if (_hierVisible) {
		if (_child) {
			translateViewpointStart();
		}
		g_driver->translateViewpoint(_pivot);

		if (_mesh && _meshVisible) {
			_mesh->getBoundingBox(x1, y1, x2, y2);
		}

		if (_child) {
			translateViewpointFinish();
			_child->getBoundingBox(x1, y1, x2, y2);
		}
	}

	if (_sibling || _child) {
		translateViewpointFinish();
	}
	if (_sibling) {
		_sibling->getBoundingBox(x1, y1, x2, y2);
	}
}

void ModelNode::addChild(ModelNode *child) {
	ModelNode **childPos = &_child;
	while (*childPos)
		childPos = &(*childPos)->_sibling;
	*childPos = child;
	child->_parent = this;
}

void ModelNode::removeChild(ModelNode *child) {
	ModelNode **childPos = &_child;
	while (*childPos && *childPos != child)
		childPos = &(*childPos)->_sibling;
	if (*childPos) {
		*childPos = child->_sibling;
		child->_parent = nullptr;
	}
}

void ModelNode::setMatrix(const Math::Matrix4 &matrix) {
	_matrix = matrix;
	if (_sibling)
		_sibling->setMatrix(matrix);
}

void ModelNode::update() {
	if (!_initialized)
		return;

	if (_hierVisible && _needsUpdate) {
		_localMatrix = _animRot.toMatrix();
		_localMatrix.setPosition(_animPos);

		_matrix = _matrix * _localMatrix;

		_pivotMatrix = _matrix;
		_pivotMatrix.translate(_pivot);

		if (_mesh) {
			_mesh->_matrix = _pivotMatrix;
		}

		if (_child) {
			_child->setMatrix(_matrix);
			_child->update();
		}

		_needsUpdate = false;
	}

	if (_sibling) {
		_sibling->update();
	}
}

void ModelNode::addSprite(Sprite *sprite) {
	sprite->_next = _sprite;
	_sprite = sprite;
}

void ModelNode::removeSprite(const Sprite *sprite) {
	Sprite *curr = _sprite;
	Sprite *prev = nullptr;
	while (curr) {
		if (curr == sprite) {
			if (prev)
				prev->_next = curr->_next;
			else
				_sprite = curr->_next;
		}
		prev = curr;
		curr = curr->_next;
	}
}

void ModelNode::translateViewpoint() const {
	g_driver->translateViewpoint(_animPos);

	Math::Matrix4 rot = _animRot.toMatrix();
	rot.transpose();
	g_driver->rotateViewpoint(rot);
}

void ModelNode::translateViewpointStart() const {
	g_driver->translateViewpointStart();
}

void ModelNode::translateViewpointFinish() const {
	g_driver->translateViewpointFinish();
}

} // end of namespace Grim
