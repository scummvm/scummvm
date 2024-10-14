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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/base/gfx/skin_mesh_helper.h"
#include "engines/wintermute/base/gfx/xskinmesh.h"
#include "engines/wintermute/base/gfx/xfile_loader.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
SkinMeshHelper::SkinMeshHelper(DXMesh *mesh, DXSkinInfo *skinInfo) {
	_mesh = mesh;
	_skinInfo = skinInfo;
}

//////////////////////////////////////////////////////////////////////////
SkinMeshHelper::~SkinMeshHelper() {
	delete _mesh;
	_mesh = nullptr;
	delete _skinInfo;
	_skinInfo = nullptr;
}

//////////////////////////////////////////////////////////////////////////
uint SkinMeshHelper::getNumFaces() {
	return _mesh->getNumFaces();
}

//////////////////////////////////////////////////////////////////////////
uint SkinMeshHelper::getNumBones() {
	return _skinInfo->getNumBones();
}

//////////////////////////////////////////////////////////////////////////
bool SkinMeshHelper::getOriginalMesh(DXMesh **mesh) {
	return _mesh->cloneMesh(mesh);
}

//////////////////////////////////////////////////////////////////////////
bool SkinMeshHelper::generateSkinnedMesh(uint32 *adjacencyOut, DXMesh **mesh) {
	bool res = getOriginalMesh(mesh);
	if (res) {
		(*mesh)->generateAdjacency(adjacencyOut);
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool SkinMeshHelper::updateSkinnedMesh(const DXMatrix *boneTransforms, DXMesh *mesh) {
	void *sourceVerts = reinterpret_cast<void *>(_mesh->getVertexBuffer().ptr());
	void *targetVerts = reinterpret_cast<void *>(mesh->getVertexBuffer().ptr());

	return _skinInfo->updateSkinnedMesh(boneTransforms, sourceVerts, targetVerts);
}

//////////////////////////////////////////////////////////////////////////
const char *SkinMeshHelper::getBoneName(uint32 boneIndex) {
	return _skinInfo->getBoneName(boneIndex);
}

//////////////////////////////////////////////////////////////////////////
DXMatrix *SkinMeshHelper::getBoneOffsetMatrix(uint32 boneIndex) {
	return _skinInfo->getBoneOffsetMatrix(boneIndex);
}

} // namespace Wintermute
