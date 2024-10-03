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

#include "engines/wintermute/base/gfx/xmaterial.h"
#include "engines/wintermute/base/gfx/xskinmesh_loader.h"
#include "engines/wintermute/base/gfx/skin_mesh_helper.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL_GAME)

#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/meshx_opengl.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
XMeshOpenGL::XMeshOpenGL(BaseGame *inGame) : XMesh(inGame) {
}

//////////////////////////////////////////////////////////////////////////
XMeshOpenGL::~XMeshOpenGL() {
}

//////////////////////////////////////////////////////////////////////////
bool XMeshOpenGL::render(XModel *model) {
	float *vertexData = _skinMesh->_mesh->_vertexData;
	auto indexData = _skinMesh->_mesh->_indexData;
	if (vertexData == nullptr) {
		return false;
	}

	bool noAttrs = false;
	auto attrsTable = _skinMesh->_mesh->_dxmesh->getAttributeTable();
	uint32 numAttrs = attrsTable->_size;
	DXAttributeRange *attrs;
	if (numAttrs == 0) {
		noAttrs = true;
		numAttrs = 1;
		attrs = new DXAttributeRange[numAttrs];
	} else {
		attrs = attrsTable->_ptr;
	}

	if (noAttrs) {
		attrs[0]._attribId = 0;
		attrs[0]._vertexStart = attrs[0]._faceStart = 0;
		attrs[0]._vertexCount = _skinMesh->_mesh->_dxmesh->getNumVertices();
		attrs[0]._faceCount = _skinMesh->_mesh->_dxmesh->getNumFaces();
	}

	for (uint32 i = 0; i < numAttrs; i++) {
		int materialIndex = attrs[i]._attribId;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _materials[materialIndex]->_material._diffuse._data);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _materials[materialIndex]->_material._diffuse._data);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _materials[materialIndex]->_material._specular._data);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, _materials[materialIndex]->_material._emissive._data);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, _materials[materialIndex]->_material._power);

		bool textureEnable = false;
		if (_materials[materialIndex]->getSurface()) {
			textureEnable = true;
			glEnable(GL_TEXTURE_2D);
			static_cast<BaseSurfaceOpenGL3D *>(_materials[materialIndex]->getSurface())->setTexture();
		} else {
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		if (textureEnable)
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, XSkinMeshLoader::kVertexComponentCount * sizeof(float), vertexData + XSkinMeshLoader::kPositionOffset);
		glNormalPointer(GL_FLOAT, XSkinMeshLoader::kVertexComponentCount * sizeof(float), vertexData + XSkinMeshLoader::kNormalOffset);
		if (textureEnable)
			glTexCoordPointer(2, GL_FLOAT, XSkinMeshLoader::kVertexComponentCount * sizeof(float), vertexData + XSkinMeshLoader::kTextureCoordOffset);

		glDrawElements(GL_TRIANGLES, attrsTable->_ptr[i]._faceCount * 3, GL_UNSIGNED_SHORT, indexData.data() + attrsTable->_ptr[i]._faceStart * 3);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	if (noAttrs) {
		delete[] attrs;
	}

	return true;
}

bool XMeshOpenGL::renderFlatShadowModel() {
	return true;
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_GAME)
