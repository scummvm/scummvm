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
#include "engines/wintermute/base/gfx/3deffect.h"
#include "engines/wintermute/base/gfx/3deffect_params.h"
#include "engines/wintermute/base/gfx/skin_mesh_helper.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/base_renderer3d.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL_GAME)

#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
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
	if (!_blendedMesh)
		return false;

	// For WME DX, mesh model is not visible, possible it's clipped.
	// For OpenGL, mesh is visible, skip draw it here instead in core.
	if (!_gameRef->_renderer3D->_camera)
		return false;

	auto fvf = _blendedMesh->getFVF();
	uint32 vertexSize = DXGetFVFVertexSize(fvf) / sizeof(float);
	float *vertexData = (float *)_blendedMesh->getVertexBuffer().ptr();
	if (vertexData == nullptr) {
		return false;
	}
	uint32 offset = 0, normalOffset = 0, textureOffset = 0;
	if (fvf & DXFVF_XYZ) {
		offset += sizeof(DXVector3) / sizeof(float);
	}
	if (fvf & DXFVF_NORMAL) {
		normalOffset = offset;
		offset += sizeof(DXVector3) / sizeof(float);
	}
	if (fvf & DXFVF_DIFFUSE) {
		offset += sizeof(DXColorValue) / sizeof(float);
	}
	if (fvf & DXFVF_TEX1) {
		textureOffset = offset;
	}
	uint32 *indexData = (uint32 *)_blendedMesh->getIndexBuffer().ptr();

	bool noAttrs = false;
	auto attrsTable = _blendedMesh->getAttributeTable();
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
		attrs[0]._vertexCount = _blendedMesh->getNumVertices();
		attrs[0]._faceCount = _blendedMesh->getNumFaces();
	}

	for (uint32 i = 0; i < numAttrs; i++) {
		Material *mat = _materials[attrs[i]._attribId];
		bool textureEnable = false;
		if (mat->getSurface()) {
			textureEnable = true;
			glEnable(GL_TEXTURE_2D);
			static_cast<BaseSurfaceOpenGL3D *>(mat->getSurface())->setTexture();
		} else {
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		if (mat->getEffect()) {
			renderEffect(mat);
		} else {
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat->_material._diffuse._data);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat->_material._diffuse._data);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat->_material._specular._data);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat->_material._emissive._data);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->_material._power);
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		if (textureEnable)
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, vertexSize * sizeof(float), vertexData);
		glNormalPointer(GL_FLOAT, vertexSize * sizeof(float), vertexData + normalOffset);
		if (textureEnable)
			glTexCoordPointer(2, GL_FLOAT, vertexSize * sizeof(float), vertexData + textureOffset);

		glDrawElements(GL_TRIANGLES, attrsTable->_ptr[i]._faceCount * 3, GL_UNSIGNED_INT, indexData + attrsTable->_ptr[i]._faceStart * 3);

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

void XMeshOpenGL::renderEffect(Material *material) {
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->_material._diffuse._data);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->_material._diffuse._data);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->_material._specular._data);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material->_material._emissive._data);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->_material._power);
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_GAME)
