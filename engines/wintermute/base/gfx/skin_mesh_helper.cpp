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
#include "engines/wintermute/base/gfx/xskinmesh_loader.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
SkinMeshHelper::SkinMeshHelper(XSkinMeshLoader *mesh) {
	_mesh = mesh;
}

//////////////////////////////////////////////////////////////////////////
SkinMeshHelper::~SkinMeshHelper() {
	delete _mesh;
}

//////////////////////////////////////////////////////////////////////////
uint SkinMeshHelper::getNumFaces() {
	return 0;//_mesh->getNumFaces();
}

//////////////////////////////////////////////////////////////////////////
uint SkinMeshHelper::getNumBones() {
	return 0;//_mesh->getNumBones();
}

//////////////////////////////////////////////////////////////////////////
bool SkinMeshHelper::getOriginalMesh(XSkinMeshLoader **mesh) {
	return true;//_mesh->cloneMeshFVF(_mesh->getOptions(), _mesh->getFVF(), mesh);
}

//////////////////////////////////////////////////////////////////////////
bool SkinMeshHelper::generateSkinnedMesh(uint32 options, float minWeight, uint32 *adjacencyOut, XSkinMeshLoader **mesh) {
	bool res = getOriginalMesh(mesh);
	/*	if (res) {
	 (*mesh)->generateAdjacency(adjacencyOut);
	 }*/
	
	return res;
}

//////////////////////////////////////////////////////////////////////////
bool SkinMeshHelper::updateSkinnedMesh(const Math::Matrix4 *boneTransforms, XSkinMeshLoader *mesh) {
	return true;//_mesh->updateSkinnedMesh(boneTransforms);
}

//////////////////////////////////////////////////////////////////////////
const char *SkinMeshHelper::getBoneName(uint boneIndex) {
	return "";//_mesh->getBoneName(boneIndex);
}

//////////////////////////////////////////////////////////////////////////
Math::Matrix4 SkinMeshHelper::getBoneOffsetMatrix(uint boneIndex) {
	return Math::Matrix4();//_mesh->getBoneOffsetMatrix(boneIndex);
}

} // namespace Wintermute
