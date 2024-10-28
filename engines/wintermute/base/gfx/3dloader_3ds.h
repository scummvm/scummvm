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

#ifndef WINTERMUTE_3D_LOADER_3DS_H
#define WINTERMUTE_3D_LOADER_3DS_H

#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/base/gfx/3dcamera.h"
#include "engines/wintermute/base/gfx/3dlight.h"
#include "engines/wintermute/base/gfx/3dmesh.h"

#include "common/str.h"

namespace Wintermute {

class Loader3DS : public BaseNamedObject {
public:
	enum E3DSFileObjectType{
		OBJ_3DS_NONE, OBJ_3DS_MESH, OBJ_3DS_CAMERA, OBJ_3DS_LIGHT
	};

	struct SFace{
		uint16 _a;
		uint16 _b;
		uint16 _c;
	};

	class FileObject3DS {
	public:
		DXVector3 _cameraTarget;
		float _cameraBank;
		float _cameraLens;
		float _cameraFOV;
		DXVector3 _cameraPos;
		DXVector3 _lightTarget;
		DXVector3 _lightPos;
		uint32 _lightColor;
		float _lightHotspot;
		float _lightFalloff;
		bool _lightOff;
		bool _lightSpotlight;
		bool _hidden;
		uint16 _numFaces;
		SFace *_faces;
		DXVector3 *_vertices;
		uint16 _numVertices;
		Common::String _name;
		E3DSFileObjectType _type;
		virtual ~FileObject3DS();
		FileObject3DS();
	};

public:
	Common::String getCameraName(int index);
	Common::String getLightName(int index);
	Common::String getMeshName(int index);
	bool loadCamera(int index, Camera3D *camera);
	uint getNumCameras();
	bool loadLight(int index, Light3D *light);
	uint getNumLights();
	bool loadMesh(int index, Mesh3DS *mesh);
	uint getNumMeshes();
	bool parseFile(const Common::String &filename);
	Common::String _filename;
	Loader3DS(BaseGame *inGame);
	virtual ~Loader3DS();
	BaseArray<FileObject3DS *> _objects;
};

} // namespace Wintermute

#endif
