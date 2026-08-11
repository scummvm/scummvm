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

#ifndef WINTERMUTE_SKIN_MESH_HELPER_H
#define WINTERMUTE_SKIN_MESH_HELPER_H

#include "engines/wintermute/base/gfx/xmath.h"

namespace Wintermute {

class XMesh;
class XMeshOpenGL;
class XMeshOpenGLShader;
class DXMesh;
class DXSkinInfo;

class SkinMeshHelper {
	friend class XMesh;
	friend class XMeshOpenGL;
	friend class XMeshOpenGLShader;

public:
	SkinMeshHelper(DXMesh *mesh, DXSkinInfo *skinInfo);
	virtual ~SkinMeshHelper();

	uint getNumFaces();
	uint getNumBones();
	bool getOriginalMesh(DXMesh **mesh);
	bool generateSkinnedMesh(uint32 *adjacencyOut, DXMesh **mesh);
	bool updateSkinnedMesh(const DXMatrix *boneTransforms, DXMesh *mesh);
	const char *getBoneName(uint32 boneIndex);
	DXMatrix *getBoneOffsetMatrix(uint32 boneIndex);

private:
	DXMesh *_mesh;
	DXSkinInfo *_skinInfo;
};

} // namespace Wintermute

#endif
