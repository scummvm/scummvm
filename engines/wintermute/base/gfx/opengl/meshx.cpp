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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "common/math.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/opengl/material.h"
#include "engines/wintermute/base/gfx/opengl/meshx.h"
#include "engines/wintermute/base/gfx/opengl/shadow_volume.h"
#include "engines/wintermute/base/gfx/x/frame_node.h"
#include "engines/wintermute/base/gfx/x/modelx.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
MeshX::MeshX(BaseGame *inGame) : BaseNamedObject(inGame),
                                 _BBoxStart(0.0f, 0.0f, 0.0f), _BBoxEnd(0.0f, 0.0f, 0.0f),
                                 _numAttrs(0), _maxFaceInfluence(0),
                                 _vertexData(nullptr), _vertexPositionData(nullptr),
                                 _vertexCount(0), _indexData(nullptr), _indexCount(0),
                                 _skinAdjacency(nullptr), _adjacency(nullptr), _skinnedMesh(false) {
}

//////////////////////////////////////////////////////////////////////////
MeshX::~MeshX() {
	delete[] _adjacency;
	delete[] _skinAdjacency;
	delete[] _vertexData;
	delete[] _vertexPositionData;
	delete[] _indexData;

	for (uint32 i = 0; i < _materials.size(); i++) {
		delete _materials[i];
	}

	_materials.clear();
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::loadFromX(char *filename) {
	warning("MeshX::loadFromX not implemented yet");

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::generateMesh() {
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::findBones(FrameNode *rootFrame) {
	// normal meshes don't have bones
	if (!_skinnedMesh) {
		return true;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::update(FrameNode *parentFrame) {
	if (_vertexData == nullptr) {
		return false;
	}

	bool res = false;

	// update skinned mesh
	if (_skinnedMesh) {

	} else { // update static mesh
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth) {
	if (_vertexData == nullptr) {
		return false;
	}

	return shadow->addMesh(_adjacency, modelMat, light, extrusionDepth);
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::render(ModelX *model) {
	if (_vertexData == nullptr) {
		return false;
	}

	bool res = false;

	// is this correct?
	for (uint32 i = 0; i < _numAttrs; i++) {
		// get the correct material

		// set material

		// set texture (if any)

		// render

		// maintain polycount
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::pickPoly(Math::Vector3d *pickRayOrig, Math::Vector3d *pickRayDir) {
	if (_vertexData == nullptr) {
		return false;
	}

	bool res = false;

	// to be implemented

	return res;
}

////////////////////////////////////////////////////////////////////////////
bool MeshX::setMaterialSprite(const Common::String &matName, BaseSprite *sprite) {
	for (uint32 i = 0; i < _materials.size(); i++) {
		if (_materials[i]->getName() && _materials[i]->getName() == matName) {
			_materials[i]->setSprite(sprite);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::setMaterialTheora(const Common::String &matName, VideoTheoraPlayer *theora) {
	for (uint32 i = 0; i < _materials.size(); i++) {
		if (_materials[i]->getName() && _materials[i]->getName() == matName) {
			_materials[i]->setTheora(theora);
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::invalidateDeviceObjects() {
	// release buffers here

	for (uint32 i = 0; i < _materials.size(); i++) {
		_materials[i]->invalidateDeviceObjects();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool MeshX::restoreDeviceObjects() {
	for (uint32 i = 0; i < _materials.size(); i++) {
		_materials[i]->restoreDeviceObjects();
	}

	if (_skinnedMesh) {
		return generateMesh();
	} else {
		return true;
	}
}

} // namespace Wintermute
