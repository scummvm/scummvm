/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/math/math_util.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/gfx/3dloader_3ds.h"
#include "engines/wintermute/base/gfx/3dface.h"
#include "engines/wintermute/base/gfx/3dvertex.h"
#include "engines/wintermute/base/gfx/3dutils.h"

namespace Wintermute {

#define MAIN3DS                       0x4D4D // level 1
#define EDIT3DS                       0x3D3D // level 1
#define NAMED_OBJECT                  0x4000 // level 2
#define TRIANGLE_MESH                 0x4100 // level 3
#define TRIANGLE_VERTEXLIST           0x4110 // level 4
#define TRIANGLE_FACELIST             0x4120 // level 4
#define CHUNK_CAMERA                  0x4700 // level 3
#define CHUNK_LIGHT                   0x4600
#define LIGHT_SPOTLIGHT               0x4610
#define LIGHT_IS_OFF                  0x4620
#define RGB_FLOAT                     0x0010
#define RGB_BYTE                      0x0011
#define KEYFRAMER                     0xB000 // level 1
#define KEYFRAMER_AMBIENT_INFO        0xB001
#define KEYFRAMER_MESH_INFO           0xB002
#define KEYFRAMER_CAMERA_INFO         0xB003
#define KEYFRAMER_CAMERA_TARGET_INFO  0xB004
#define KEYFRAMER_OMNI_INFO           0xB005
#define KEYFRAMER_SPOT_TARGET_INFO    0xB006
#define KEYFRAMER_SPOT_INFO           0xB007
#define NODE_HDR                      0xB010
#define ROLL_TRACK_TAG                0xB024


//////////////////////////////////////////////////////////////////////////
Loader3DS::Loader3DS(BaseGame *inGame) : BaseNamedObject(inGame) {
}


//////////////////////////////////////////////////////////////////////////
Loader3DS::~Loader3DS() {
	for (size_t i = 0; i < _objects.size(); i++)
		delete _objects[i];
	_objects.clear();
}



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
Loader3DS::FileObject3DS::FileObject3DS() {
	_type = OBJ_3DS_NONE;

	_numVertices = 0;
	_vertices = nullptr;

	_numFaces = 0;
	_faces = nullptr;

	_lightOff = false;
	_lightSpotlight = false;
	_lightColor = 0;
	_lightHotspot = false;
	_lightFalloff = false;

	_cameraBank = 0;
	_cameraLens = 0;
	_cameraFOV = 0;

	_hidden = false;
}


//////////////////////////////////////////////////////////////////////
Loader3DS::FileObject3DS::~FileObject3DS() {
	if (_vertices != NULL)
		delete[] _vertices;
	if (_faces != NULL)
		delete[] _faces;
}


//////////////////////////////////////////////////////////////////////////
bool Loader3DS::parseFile(const Common::String &filename) {
	_filename = filename;

	uint32 fileSize;
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename, &fileSize);
	if (!buffer)
		return false;

	uint16 keyframerSection = 0;
	Common::String keyframerObject;
	FileObject3DS *obj = nullptr;
	uint16 i;

	Common::MemoryReadStream fileStream(buffer, fileSize);

	while (fileStream.pos() < fileStream.size()) {
		uint16 chunkId = fileStream.readUint16LE();
		uint32 chunkLength = fileStream.readUint32LE();

		bool handled = true;

		switch (chunkId) {
			case MAIN3DS:
			break;

			case EDIT3DS:
			break;

			//////////////////////////////////////////////////////////////////////
			// object ////////////////////////////////////////////////////////////
			case NAMED_OBJECT: {
				obj = new FileObject3DS;
				for (int8 current = fileStream.readByte(); current != 0; current = fileStream.readByte()) {
					obj->_name.insertChar(current, obj->_name.size());
				}
				_objects.add(obj);
			}
			break;


			// mesh //////////////////////////////////////////////////////////////
			case TRIANGLE_MESH:
				if (obj == nullptr)
					break;
				obj->_type = OBJ_3DS_MESH;
			break;


			case TRIANGLE_VERTEXLIST:
				if (obj == nullptr || obj->_type != OBJ_3DS_MESH)
					break;

				obj->_numVertices = fileStream.readUint16LE();
				obj->_vertices = new DXVector3[obj->_numVertices];
				for (i = 0; i < obj->_numVertices; i++) {
					obj->_vertices[i]._x = fileStream.readFloatLE();
					obj->_vertices[i]._z = fileStream.readFloatLE();
					obj->_vertices[i]._y = fileStream.readFloatLE();
				}
			break;


			case TRIANGLE_FACELIST:
				if (obj == nullptr || obj->_type != OBJ_3DS_MESH)
					break;

				obj->_numFaces = fileStream.readUint16LE();

				obj->_faces = new SFace[obj->_numFaces];
				for (i = 0; i < obj->_numFaces; i++) {
					obj->_faces[i]._a = fileStream.readUint16LE();
					obj->_faces[i]._c = fileStream.readUint16LE();
					obj->_faces[i]._b = fileStream.readUint16LE();
					fileStream.readUint16LE(); // skip
				}
			break;


			// camera //////////////////////////////////////////////////////////////
			case CHUNK_CAMERA:
				if (obj == nullptr)
					break;
				obj->_type = OBJ_3DS_CAMERA;

				obj->_cameraPos._x = fileStream.readFloatLE();
				obj->_cameraPos._z = fileStream.readFloatLE();
				obj->_cameraPos._y = fileStream.readFloatLE();

				obj->_cameraTarget._x = fileStream.readFloatLE();
				obj->_cameraTarget._z = fileStream.readFloatLE();
				obj->_cameraTarget._y = fileStream.readFloatLE();
				
				obj->_cameraBank = fileStream.readFloatLE();
				obj->_cameraLens = fileStream.readFloatLE();
				if (obj->_cameraLens > 0)
					obj->_cameraFOV = 1900.0f / obj->_cameraLens;
				else
					obj->_cameraFOV = 45.0f;
			break;


			// light //////////////////////////////////////////////////////////////
			case CHUNK_LIGHT:
				if (obj == nullptr)
					break;
				obj->_type = OBJ_3DS_LIGHT;

				obj->_lightPos._x = fileStream.readFloatLE();
				obj->_lightPos._z = fileStream.readFloatLE();
				obj->_lightPos._y = fileStream.readFloatLE();
			break;

			case LIGHT_SPOTLIGHT:
				if (obj == nullptr || obj->_type != OBJ_3DS_LIGHT)
					break;

				obj->_lightTarget._x = fileStream.readFloatLE();
				obj->_lightTarget._z = fileStream.readFloatLE();
				obj->_lightTarget._y = fileStream.readFloatLE();

				obj->_lightHotspot = fileStream.readFloatLE();
				obj->_lightFalloff = fileStream.readFloatLE();
				obj->_lightSpotlight = true;
			break;

			case LIGHT_IS_OFF:
				if (obj == nullptr || obj->_type != OBJ_3DS_LIGHT)
					break;

				obj->_lightOff = true;
			break;


			// colors ////////////////////////////////////////////////////////////////////////
			case RGB_FLOAT:
				if (obj && obj->_type == OBJ_3DS_LIGHT) {
					float r, g, b;
					r = fileStream.readFloatLE();
					g = fileStream.readFloatLE();
					b = fileStream.readFloatLE();

					obj->_lightColor = BYTETORGBA((int)(r * 255), (int)(g * 255), (int)(b * 255), 255);
				} else
					handled = false;
			break;

			case RGB_BYTE:
				if (obj && obj->_type == OBJ_3DS_LIGHT) {
					byte r, g, b;
					r = fileStream.readByte();
					g = fileStream.readByte();
					b = fileStream.readByte();

					obj->_lightColor = BYTETORGBA(r, g, b, 255);
				} else
					handled = false;
			break;

			// keyframer stuff
			case KEYFRAMER:
			break;

			case KEYFRAMER_AMBIENT_INFO:
			case KEYFRAMER_MESH_INFO:
			case KEYFRAMER_CAMERA_INFO:
			case KEYFRAMER_CAMERA_TARGET_INFO:
			case KEYFRAMER_OMNI_INFO:
			case KEYFRAMER_SPOT_TARGET_INFO:
			case KEYFRAMER_SPOT_INFO:
				keyframerSection = chunkId;
			break;

			case NODE_HDR:
				// object name
				keyframerObject.clear();
				for (int8 current = fileStream.readByte(); current != 0; current = fileStream.readByte()) {
					keyframerObject.insertChar(current, keyframerObject.size());
				}
				// flags1
				fileStream.readUint16LE();
				// flags2
				fileStream.readUint16LE();
				// hierarchy
				fileStream.readUint16LE();
			break;

			case ROLL_TRACK_TAG:
				if (keyframerSection == KEYFRAMER_CAMERA_INFO && !keyframerObject.empty()) {
					// flags
					fileStream.readUint16LE();
					// unknown
					fileStream.readUint16LE();
					fileStream.readUint16LE();
					fileStream.readUint16LE();
					fileStream.readUint16LE();
					// keys
					uint16 keys = fileStream.readUint16LE();
					// unknown
					fileStream.readUint16LE();

					for (uint16 key = 0; key < keys; key++) {
						// frame number
						fileStream.readUint16LE();
						// unknown
						fileStream.readUint32LE();
						// camera roll
						float cameraRoll = fileStream.readFloatLE();

						// inject this roll value to the camera
						if (key == 0) {
							for (size_t index = 0; index < _objects.size(); index++) {
								if (_objects[index]->_type == OBJ_3DS_CAMERA && _objects[index]->_name == keyframerObject) {
									_objects[index]->_cameraBank = cameraRoll;
									break;
								}
							}
						}
					}
				} else
					handled = false;
				break;

			default:
				handled = false;
		}

		if (!handled)
			fileStream.seek(chunkLength - 6, SEEK_CUR);
	}

	delete[] buffer;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint Loader3DS::getNumMeshes() {
	int ret = 0;

	for (size_t i = 0; i < _objects.size(); i++)
		if (_objects[i]->_type == OBJ_3DS_MESH)
			ret++;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
Common::String Loader3DS::getMeshName(int index) {
	int pos = -1;

	for (size_t i = 0; i < _objects.size(); i++) {
		if (_objects[i]->_type == OBJ_3DS_MESH)
			pos++;
		if (pos == index)
			return _objects[i]->_name;
	}
	return Common::String();
}


//////////////////////////////////////////////////////////////////////////
bool Loader3DS::loadMesh(int index, Mesh3DS *mesh) {
	if (!mesh)
		return false;

	int pos = -1;
	for (size_t i = 0; i < _objects.size(); i++) {
		if (_objects[i]->_type == OBJ_3DS_MESH)
			pos++;
		if (pos == index){
			FileObject3DS *obj = _objects[i];
			mesh->cleanup();

			mesh->_numVertices = obj->_numVertices;
			mesh->_numFaces = obj->_numFaces;

			int j;

			mesh->_vertices = new Vertex3D[mesh->_numVertices];
			for (j = 0; j < mesh->_numVertices; j++) {
				mesh->_vertices[j]._pos._x = obj->_vertices[j]._x;
				mesh->_vertices[j]._pos._y = obj->_vertices[j]._y;
				mesh->_vertices[j]._pos._z = obj->_vertices[j]._z;
			}

			mesh->_faces = new Face3D[mesh->_numFaces];
			for (j = 0; j < mesh->_numFaces; j++) {
				mesh->_faces[j]._vertices[0] = obj->_faces[j]._a;
				mesh->_faces[j]._vertices[1] = obj->_faces[j]._b;
				mesh->_faces[j]._vertices[2] = obj->_faces[j]._c;
			}

			mesh->setName(obj->_name.c_str());

			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint Loader3DS::getNumLights() {
	int ret = 0;

	for (size_t i = 0; i < _objects.size(); i++)
		if (_objects[i]->_type == OBJ_3DS_LIGHT)
			ret++;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
Common::String Loader3DS::getLightName(int index) {
	int pos = -1;

	for (size_t i = 0; i < _objects.size(); i++) {
		if (_objects[i]->_type == OBJ_3DS_LIGHT)
			pos++;
		if (pos == index)
			return _objects[i]->_name;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
bool Loader3DS::loadLight(int index, Light3D *light) {
	if (!light)
		return false;

	int pos = -1;
	for (size_t i = 0; i < _objects.size(); i++) {
		if (_objects[i]->_type == OBJ_3DS_LIGHT) {
			pos++;
			if (pos == index) {
				light->setName(_objects[i]->_name.c_str());
				light->_pos = _objects[i]->_lightPos;
				light->_target = _objects[i]->_lightTarget;
				light->_isSpotlight = _objects[i]->_lightSpotlight;
				light->_active = !_objects[i]->_lightOff;
				light->_diffuseColor = _objects[i]->_lightColor;
				light->_falloff = _objects[i]->_lightFalloff;
			}
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint Loader3DS::getNumCameras() {
	int ret = 0;

	for (size_t i = 0; i < _objects.size(); i++)
		if (_objects[i]->_type == OBJ_3DS_CAMERA)
			ret++;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
Common::String Loader3DS::getCameraName(int index) {
	int pos = -1;
	for (size_t i = 0; i < _objects.size(); i++) {
		if (_objects[i]->_type == OBJ_3DS_CAMERA)
			pos++;
		if (pos == index)
			return _objects[i]->_name;
	}
	return Common::String();
}


//////////////////////////////////////////////////////////////////////////
bool Loader3DS::loadCamera(int index, Camera3D *camera) {
	if (!camera) return
		false;

	int pos = -1;
	for (size_t i = 0; i < _objects.size(); i++) {
		if (_objects[i]->_type == OBJ_3DS_CAMERA)
			pos++;
		if (pos == index) {
			camera->setupPos(_objects[i]->_cameraPos, _objects[i]->_cameraTarget, _objects[i]->_cameraBank);
			camera->setName(_objects[i]->_name.c_str());
			camera->_fov = camera->_origFov = degToRad(_objects[i]->_cameraFOV);

			return true;
		}
	}
	return false;
}

} // namespace Wintermute
