/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
#include "engines/grim/material.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/resource.h"
#include "engines/grim/modelemi.h"


namespace Grim {

struct Vector3int {
	int _x;
	int _y;
	int _z;
	void setVal(int x, int y, int z) {
		_x = x; _y = y; _z = z;
	}
};

Common::String readLAString(Common::ReadStream &ms) {
	int strLength = ms.readUint32LE();
	char* readString = new char[strLength];
	ms.read(readString, strLength);
	
	Common::String retVal(readString);
	delete[] readString;
	
	return retVal;
}
	
Math::Vector2d *readVector2d(Common::ReadStream &ms, int count = 1) {
	Math::Vector2d *vec2d = new Math::Vector2d[count];
	char buf[8];
	for (int i = 0; i < count; i++) {
		ms.read(buf, 8);
		vec2d[i].setX(get_float(buf));
		vec2d[i].setY(get_float(buf + 4));
	}
	return vec2d;
}

Math::Vector3d *readVector3d(Common::ReadStream &ms, int count = 1) {
	Math::Vector3d *vec3d = new Math::Vector3d[count];
	char buf[12];
	for (int i = 0; i < count; i++) {
		ms.read(buf, 12);
		vec3d[i] = Math::Vector3d::get_vector3d(buf);
	}
	return vec3d;
}

Math::Vector4d *readVector4d(Common::ReadStream &ms) {
	Math::Vector4d *vec4d = new Math::Vector4d();
	char buf[16];
	ms.read(buf, 16);
	*vec4d = Math::Vector4d::get_vector4d(buf);
	return vec4d;
}

void EMIMeshFace::loadFace(Common::SeekableReadStream *data) {
	_flags = data->readUint32LE();
	_hasTexture = data->readUint32LE();

	if(_hasTexture)
		_texID = data->readUint32LE();
	_faceLength = data->readUint32LE();
	_faceLength = _faceLength / 3;
	int x = 0, y = 0, z = 0;
	_indexes = new Vector3int[_faceLength];
	int j = 0;
	int readsize;
	if (g_grim->getGamePlatform() == Common::kPlatformPS2) {
		readsize = 4;
	} else {
		readsize = 2;
	}
	for (uint32 i = 0; i < _faceLength; i ++) {
		data->read((char *)&x, readsize);
		data->read((char *)&y, readsize);
		data->read((char *)&z, readsize);
		_indexes[j++].setVal(x,y,z);
	}
}

void EMIModel::setTex(int index) {
	_mats[index]->select();
}

// May be removed when I get through the conversion
void EMIMeshFace::render() {
	if(_hasTexture) {
		_parent->setTex(_texID);
	}
	//glDrawElements(GL_TRIANGLES, _faceLength * 3, GL_UNSIGNED_INT, _indexes);
}

void EMIModel::loadMesh(Common::SeekableReadStream *data) {
	int strLength = 0; // Usefull for PS2-strings
	
	Common::String nameString = readLAString(*data);
	
	_sphereData = readVector4d(*data);

	_boxData = readVector3d(*data);
	_boxData2 = readVector3d(*data);

	_numTexSets = data->readUint32LE();
	_setType = data->readUint32LE();
	_numTextures = data->readUint32LE();

	_texNames = new Common::String[_numTextures];

	for(uint32 i = 0;i < _numTextures; i++) {
		_texNames[i] = readLAString(*data);
		// Every texname seems to be followed by 4 0-bytes (Ref mk1.mesh,
		// this is intentional)
		data->skip(4);
	}

	// 4 unknown bytes - usually with value 19
	data->skip(4);

	_numVertices = data->readUint32LE();

	// Vertices
	_vertices = readVector3d(*data, _numVertices);
	_normals = readVector3d(*data, _numVertices);
	_colorMap = new EMIColormap[_numVertices];
	for (int i = 0; i < _numVertices; ++i) {
		_colorMap[i].r = data->readByte();
		_colorMap[i].g = data->readByte();
		_colorMap[i].b = data->readByte();
		_colorMap[i].a = data->readByte();
	}
	_texVerts = readVector2d(*data, _numVertices);

	// Faces

	_numFaces = data->readUint32LE();

	// Handle the empty-faced fx/screen?.mesh-files
	if (data->eos()) {
		_numFaces = 0;
		_faces = NULL;
		return;
	}

	_faces = new EMIMeshFace[_numFaces];

	for(uint32 j = 0;j < _numFaces; j++) {
		_faces[j].setParent(this);
		_faces[j].loadFace(data);
	}

	int hasBones = data->readUint32LE();

	// TODO add in the bone-stuff, as well as the skeleton
	prepare(); // <- Initialize materials etc.
}

void EMIModel::prepareForRender() {
	// TODO, this was intended to update the vertices from the skeleton.
}

void EMIModel::prepare() {
	_mats = new Material*[_numTextures];
	for (uint32 i = 0; i < _numTextures; i++) {
		// HACK: As we dont know what specialty-textures are yet, we skip loading them
		if (!_texNames[i].contains("specialty"))
			_mats[i] = g_resourceloader->loadMaterial(_texNames[i].c_str(), NULL);
	}
	prepareForRender();
}

void EMIModel::draw() {
	prepareForRender();
	// We will need to add a call to the skeleton, to get the modified vertices, but for now,
	// I'll be happy with just static drawing
	for(uint32 i = 0; i < _numFaces; i++) {
		_faces[i].render();
		g_driver->drawEMIModelFace(this, &_faces[i]);
	}
}

EMIModel::EMIModel(const Common::String &filename, Common::SeekableReadStream *data, EMIModel *parent) : _fname(filename) {
	loadMesh(data);
}
	
} // end of namespace Grim
