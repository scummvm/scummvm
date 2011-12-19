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

#include "common/endian.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/model.h"
#include "engines/grim/material.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/resource.h"
#include "engines/grim/colormap.h"

namespace Grim {

void Sprite::draw() const {
	if (!_visible)
		return;

	_material->select();
	g_driver->drawSprite(this);
}

/**
 * @class Model
 */
Model::Model(const Common::String &filename, Common::SeekableReadStream *data, CMap *cmap, Model *parent) :
		Object(), _parent(parent), _numMaterials(0), _numGeosets(0), _cmap(cmap) {
	_fname = filename;

	if (g_grim->getGameType() == GType_MONKEY4) {
		loadEMI(data);
	} else if (data->readUint32BE() == MKTAG('L','D','O','M'))
		loadBinary(data, cmap);
	else {
		data->seek(0, SEEK_SET);
		TextSplitter ts(data);
		loadText(&ts, cmap);
	}
	delete data;

	Math::Vector3d max;

	_rootHierNode->update();
	bool first = true;
	for (int i = 0; i < _numHierNodes; ++i) {
		ModelNode &node = _rootHierNode[i];
		if (node._mesh) {
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

void Model::loadEMI(Common::SeekableReadStream *data) {
	char name[64];

	int nameLength = data->readUint32LE();
	assert(nameLength < 64);

	data->read(name, nameLength);

	// skip over some unkown floats
	data->seek(48, SEEK_CUR);

	_numMaterials = data->readUint32LE();
	_materials = new Material*[_numMaterials];
	_materialNames = new char[_numMaterials][32];
	for (int i = 0; i < _numMaterials; i++) {
		nameLength = data->readUint32LE();
		assert(nameLength < 32);

		data->read(_materialNames[i], nameLength);
		// I'm not sure what specialty mateials are, but they are handled differently.
		if (memcmp(_materialNames[i], "specialty", 9) == 0) {
			_materials[i] = 0;
		} else {
			loadMaterial(i, 0);
		}
		data->seek(4, SEEK_CUR);
	}

	data->seek(4, SEEK_CUR);


}
void Model::loadBinary(Common::SeekableReadStream *data, CMap *cmap) {
	char v3[4 * 3], f[4];
	_numMaterials = data->readUint32LE();
	_materials = new Material*[_numMaterials];
	_materialNames = new char[_numMaterials][32];
	_materialsShared = new bool[_numMaterials];
	for (int i = 0; i < _numMaterials; i++) {
		data->read(_materialNames[i], 32);
		_materialsShared[i] = false;
		_materials[i] = NULL;
		loadMaterial(i, cmap);
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
	data->read(f, 4);
	_radius = get_float(f);
	data->seek(36, SEEK_CUR);
	data->read(v3, 3 * 4);
	_insertOffset = Math::Vector3d::get_vector3d(v3);
}

void Model::loadText(TextSplitter *ts, CMap *cmap) {
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
		_materials[i] = NULL;

		ts->scanString("%d: %32s", 2, &num, materialName);
		strcpy(_materialNames[num], materialName);
		loadMaterial(num, cmap);
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
			_rootHierNode[num]._mesh = NULL;
		else
			_rootHierNode[num]._mesh = &_geosets[0]._meshes[mesh];
		if (parent >= 0) {
			_rootHierNode[num]._parent = &_rootHierNode[parent];
			_rootHierNode[num]._depth = _rootHierNode[parent]._depth + 1;
		} else {
			_rootHierNode[num]._parent = NULL;
			_rootHierNode[num]._depth = 0;
		}
		if (child >= 0)
			_rootHierNode[num]._child = &_rootHierNode[child];
		else
			_rootHierNode[num]._child = NULL;
		if (sibling >= 0)
			_rootHierNode[num]._sibling = &_rootHierNode[sibling];
		else
			_rootHierNode[num]._sibling = NULL;

		_rootHierNode[num]._numChildren = numChildren;
		_rootHierNode[num]._pos = Math::Vector3d(x, y, z);
		_rootHierNode[num]._pitch = pitch;
		_rootHierNode[num]._yaw = yaw;
		_rootHierNode[num]._roll = roll;
		_rootHierNode[num]._pivot = Math::Vector3d(pivotx, pivoty, pivotz);
		_rootHierNode[num]._meshVisible = true;
		_rootHierNode[num]._hierVisible = true;
		_rootHierNode[num]._sprite = NULL;
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
	Material *mat = NULL;
	if (!_materialsShared[index]) {
		mat = _materials[index];
	}
	_materials[index] = NULL;
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
			_materials[index] = g_resourceloader->loadMaterial(_materialNames[index], cmap);
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

	return NULL;
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
MeshFace::~MeshFace() {
	delete[] _vertices;
	delete[] _texVertices;
}

int MeshFace::loadBinary(Common::SeekableReadStream *data, Material *materials[]) {
	char v3[4 * 3], f[4];
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
	data->read(f, 4);
	_extraLight = get_float(f);
	data->seek(12, SEEK_CUR);
	data->read(v3, 4 * 3);
	_normal = Math::Vector3d::get_vector3d(v3);

	_vertices = new int[_numVertices];
	for (int i = 0; i < _numVertices; i++)
		_vertices[i] = data->readUint32LE();
	if (texPtr == 0)
		_texVertices = NULL;
	else {
		_texVertices = new int[_numVertices];
		for (int i = 0; i < _numVertices; i++)
			_texVertices[i] = data->readUint32LE();
	}
	if (materialPtr == 0)
		_material = 0;
	else {
		materialPtr = data->readUint32LE();
		_material = materials[materialPtr];
	}
	return materialPtr;
}

void MeshFace::changeMaterial(Material *material) {
	_material = material;
}

void MeshFace::draw(float *vertices, float *vertNormals, float *textureVerts) const {
	_material->select();
	g_driver->drawModelFace(this, vertices, vertNormals, textureVerts);
}

/**
 * @class Mesh
 */
Mesh::~Mesh() {
	delete[] _vertices;
	delete[] _verticesI;
	delete[] _vertNormals;
	delete[] _textureVerts;
	delete[] _faces;
	delete[] _materialid;
}

void Mesh::loadBinary(Common::SeekableReadStream *data, Material *materials[]) {
	char f[4];
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
		data->read(f, 4);
		_vertices[i] = get_float(f);
	}
	for (int i = 0; i < 2 * _numTextureVerts; i++) {
		data->read(f, 4);
		_textureVerts[i] = get_float(f);
	}
	for (int i = 0; i < _numVertices; i++) {
		data->read(f, 4);
		_verticesI[i] = get_float(f);
	}
	data->seek(_numVertices * 4, SEEK_CUR);
	for (int i = 0; i < _numFaces; i++)
		_materialid[i] = _faces[i].loadBinary(data, materials);
	for (int i = 0; i < 3 * _numVertices; i++) {
		data->read(f, 4);
		_vertNormals[i] = get_float(f);
	}
	_shadow = data->readUint32LE();
	data->seek(4, SEEK_CUR);
	data->read(f, 4);
	_radius = get_float(f);
	data->seek(24, SEEK_CUR);
}

void Mesh::loadText(TextSplitter *ts, Material* materials[]) {
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
		int num, materialid, geo, light, tex, verts;
		unsigned int type;
		float extralight;
		int readlen;

		if (ts->isEof())
			error("Expected face data, got EOF");

		if (sscanf(ts->getCurrentLine(), " %d: %d %x %d %d %d %f %d%n", &num, &materialid, &type, &geo, &light, &tex, &extralight, &verts, &readlen) < 8)
			error("Expected face data, got '%s'", ts->getCurrentLine());

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

			if (sscanf(ts->getCurrentLine() + readlen, " %d, %d%n", &_faces[num]._vertices[j], &_faces[num]._texVertices[j], &readlen2) < 2)
				error("Could not read vertex indices in line '%s'",

			ts->getCurrentLine());
			readlen += readlen2;
		}
		ts->nextLine();
	}

	ts->expectString("face normals");
	for (int i = 0; i < _numFaces; i++) {
		int num;
		float x, y, z;
		ts->scanString(" %d: %f %f %f", 4, &num, &x, &y, &z);
		_faces[num]._normal = Math::Vector3d(x, y, z);
	}
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

	for (int i = 0; i < _numFaces; i++)
		_faces[i].draw(_vertices, _vertNormals, _textureVerts);

	if (_lightingMode == 0)
		g_driver->enableLights();
}

void Mesh::getBoundingBox(int *x1, int *y1, int *x2, int *y2) const {
	int winX1, winY1, winX2, winY2;
	g_driver->getBoundingBoxPos(this, &winX1, &winY1, &winX2, &winY2);
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
ModelNode::~ModelNode() {
	ModelNode *child = _child;
	while (child) {
		child->_parent = NULL;
		child = child->_sibling;
	}
}

void ModelNode::loadBinary(Common::SeekableReadStream *data, ModelNode *hierNodes, const Model::Geoset *g) {
	char v3[4 * 3], f[4];
	data->read(_name, 64);
	_flags = data->readUint32LE();
	data->seek(4, SEEK_CUR);
	_type = data->readUint32LE();
	int meshNum = data->readUint32LE();
	if (meshNum < 0)
		_mesh = NULL;
	else
		_mesh = g->_meshes + meshNum;
	_depth = data->readUint32LE();
	int parentPtr = data->readUint32LE();
	_numChildren = data->readUint32LE();
	int childPtr = data->readUint32LE();
	int siblingPtr = data->readUint32LE();
	data->read(v3, 4 * 3);
	_pivot = Math::Vector3d::get_vector3d(v3);
	data->read(v3, 4 * 3);
	_pos = Math::Vector3d::get_vector3d(v3);
	data->read(f, 4);
	_pitch = get_float(f);
	data->read(f, 4);
	_yaw = get_float(f);
	data->read(f, 4);
	_roll = get_float(f);
	_animPos.set(0,0,0);
	_animPitch = 0;
	_animYaw = 0;
	_animRoll = 0;
	_sprite = NULL;

	data->seek(48, SEEK_CUR);

	if (parentPtr != 0)
		_parent = hierNodes + data->readUint32LE();
	else
		_parent = NULL;

	if (childPtr != 0)
		_child = hierNodes + data->readUint32LE();
	else
		_child = NULL;

	if (siblingPtr != 0)
		_sibling = hierNodes + data->readUint32LE();
	else
		_sibling = NULL;

	_meshVisible = true;
	_hierVisible = true;
	_initialized = true;
}

void ModelNode::draw() const {
	translateViewpoint();
	if (_hierVisible) {
		g_driver->translateViewpointStart();
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

		g_driver->translateViewpointFinish();

		if (_child) {
			_child->draw();
		}
	}
	translateViewpointBack();

	if (_sibling) {
		_sibling->draw();
	}
}

void ModelNode::getBoundingBox(int *x1, int *y1, int *x2, int *y2) const {
	translateViewpoint();
	if (_hierVisible) {
		g_driver->translateViewpointStart();
		g_driver->translateViewpoint(_pivot);

		if (_mesh && _meshVisible) {
			_mesh->getBoundingBox(x1, y1, x2, y2);
		}

		g_driver->translateViewpointFinish();

		if (_child) {
			_child->getBoundingBox(x1, y1, x2, y2);
		}
	}
	translateViewpointBack();

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
		child->_parent = NULL;
	}
}

void ModelNode::setMatrix(Math::Matrix4 matrix) {
	_matrix = matrix;
	if (_sibling)
		_sibling->setMatrix(matrix);
}

void ModelNode::update() {
	if (!_initialized)
		return;

	if (_hierVisible) {
		Math::Vector3d animPos = _pos + _animPos;
		Math::Angle animPitch = _pitch + _animPitch;
		Math::Angle animYaw = _yaw + _animYaw;
		Math::Angle animRoll = _roll + _animRoll;

		_localMatrix.setPosition(animPos);
		_localMatrix.buildFromPitchYawRoll(animPitch, animYaw, animRoll);

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
	}

	if (_sibling) {
		_sibling->update();
	}
}

void ModelNode::addSprite(Sprite *sprite) {
	sprite->_next = _sprite;
	_sprite = sprite;
}

void ModelNode::removeSprite(Sprite *sprite) {
	Sprite* curr = _sprite;
	Sprite* prev = NULL;
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
	Math::Vector3d animPos = _pos + _animPos;
	Math::Angle animPitch = _pitch + _animPitch;
	Math::Angle animYaw = _yaw + _animYaw;
	Math::Angle animRoll = _roll + _animRoll;
	g_driver->translateViewpointStart();

	g_driver->translateViewpoint(animPos);
	g_driver->rotateViewpoint(animYaw, Math::Vector3d(0, 0, 1));
	g_driver->rotateViewpoint(animPitch, Math::Vector3d(1, 0, 0));
	g_driver->rotateViewpoint(animRoll, Math::Vector3d(0, 1, 0));
}

void ModelNode::translateViewpointBack() const {
	g_driver->translateViewpointFinish();
}

} // end of namespace Grim
