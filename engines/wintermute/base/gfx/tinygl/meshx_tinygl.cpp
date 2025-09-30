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
#include "engines/wintermute/base/base_engine.h"

#if defined(USE_TINYGL)

#include "engines/wintermute/base/gfx/tinygl/base_surface_tinygl.h"
#include "engines/wintermute/base/gfx/tinygl/base_render_tinygl.h"
#include "engines/wintermute/base/gfx/tinygl/meshx_tinygl.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
XMeshTinyGL::XMeshTinyGL(BaseGame *inGame) : XMesh(inGame) {
}

//////////////////////////////////////////////////////////////////////////
XMeshTinyGL::~XMeshTinyGL() {
}

//////////////////////////////////////////////////////////////////////////
bool XMeshTinyGL::render(XModel *model) {
	if (!_blendedMesh)
		return false;

	// For WME DX, mesh model is not visible, possible it's clipped.
	// For OpenGL, mesh is visible, skip draw it here instead in core.
	if (!_game->_renderer3D->_camera)
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
			tglEnable(TGL_TEXTURE_2D);
			static_cast<BaseSurfaceTinyGL *>(mat->getSurface())->setTexture();
			tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_LINEAR);
			tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR_MIPMAP_LINEAR);
		} else {
			tglBindTexture(TGL_TEXTURE_2D, 0);
			tglDisable(TGL_TEXTURE_2D);
		}

		if (mat->getEffect()) {
			renderEffect(mat);
		} else {
			tglMaterialfv(TGL_FRONT_AND_BACK, TGL_DIFFUSE, mat->_material._diffuse._data);
			tglMaterialfv(TGL_FRONT_AND_BACK, TGL_AMBIENT, mat->_material._diffuse._data);
			tglMaterialfv(TGL_FRONT_AND_BACK, TGL_SPECULAR, mat->_material._specular._data);
			tglMaterialfv(TGL_FRONT_AND_BACK, TGL_EMISSION, mat->_material._emissive._data);
			tglMaterialf(TGL_FRONT_AND_BACK, TGL_SHININESS, mat->_material._power);
		}

		tglEnableClientState(TGL_VERTEX_ARRAY);
		tglEnableClientState(TGL_NORMAL_ARRAY);
		if (textureEnable)
			tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);

		tglVertexPointer(3, TGL_FLOAT, vertexSize * sizeof(float), vertexData);
		tglNormalPointer(TGL_FLOAT, vertexSize * sizeof(float), vertexData + normalOffset);
		if (textureEnable)
			tglTexCoordPointer(2, TGL_FLOAT, vertexSize * sizeof(float), vertexData + textureOffset);

		tglDrawElements(TGL_TRIANGLES, attrsTable->_ptr[i]._faceCount * 3, TGL_UNSIGNED_INT, indexData + attrsTable->_ptr[i]._faceStart * 3);

		tglDisableClientState(TGL_VERTEX_ARRAY);
		tglDisableClientState(TGL_NORMAL_ARRAY);
		tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
	}

	tglBindTexture(TGL_TEXTURE_2D, 0);
	tglDisable(TGL_TEXTURE_2D);

	if (noAttrs) {
		delete[] attrs;
	}

	return true;
}

bool XMeshTinyGL::renderFlatShadowModel(uint32 shadowColor) {
	if (!_blendedMesh)
		return false;

	// For WME DX, mesh model is not visible, possible it's clipped.
	// For OpenGL, mesh is visible, skip draw it here instead in core.
	if (!_game->_renderer3D->_camera)
		return false;

	// W/A for the scene with the table in the laboratory where the engine switches to flat shadows.
	// Presumably, it's supposed to disable shadows.
	// Instead, OpenGL draws graphical glitches.
	// Original DX version does not have this issue due to rendering shadows differently.
	if (BaseEngine::instance().getGameId() == "alphapolaris")
		return false;

	uint32 vertexSize = DXGetFVFVertexSize(_blendedMesh->getFVF()) / sizeof(float);
	float *vertexData = (float *)_blendedMesh->getVertexBuffer().ptr();
	if (vertexData == nullptr) {
		return false;
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

	tglBindTexture(TGL_TEXTURE_2D, 0);
	tglDisable(TGL_TEXTURE_2D);

	tglDisable(TGL_LIGHTING);
	tglShadeModel(TGL_FLAT);

	tglColorMask(TGL_FALSE, TGL_FALSE, TGL_FALSE, TGL_FALSE);
	tglDepthMask(TGL_FALSE);

	tglEnable(TGL_STENCIL_TEST);
	tglStencilFunc(TGL_ALWAYS, 1, (TGLuint)~0);
	tglStencilOp(TGL_REPLACE, TGL_REPLACE, TGL_REPLACE);

	for (uint32 i = 0; i < numAttrs; i++) {
		tglEnableClientState(TGL_VERTEX_ARRAY);

		tglVertexPointer(3, TGL_FLOAT, vertexSize * sizeof(float), vertexData);

		tglDrawElements(TGL_TRIANGLES, attrsTable->_ptr[i]._faceCount * 3, TGL_UNSIGNED_INT, indexData + attrsTable->_ptr[i]._faceStart * 3);

		tglDisableClientState(TGL_VERTEX_ARRAY);
	}


	tglStencilFunc(TGL_EQUAL, 1, (TGLuint)~0);
	tglStencilOp(TGL_ZERO, TGL_ZERO, TGL_ZERO);

	tglColor4ub(RGBCOLGetR(shadowColor), RGBCOLGetG(shadowColor), RGBCOLGetB(shadowColor), RGBCOLGetA(shadowColor));
	tglColorMask(TGL_TRUE, TGL_TRUE, TGL_TRUE, TGL_TRUE);
	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);

	tglDepthMask(TGL_TRUE);

	for (uint32 i = 0; i < numAttrs; i++) {
		tglEnableClientState(TGL_VERTEX_ARRAY);

		tglVertexPointer(3, TGL_FLOAT, vertexSize * sizeof(float), vertexData);

		tglDrawElements(TGL_TRIANGLES, attrsTable->_ptr[i]._faceCount * 3, TGL_UNSIGNED_INT, indexData + attrsTable->_ptr[i]._faceStart * 3);

		tglDisableClientState(TGL_VERTEX_ARRAY);
	}

	if (noAttrs) {
		delete[] attrs;
	}

	tglDisable(TGL_BLEND);
	tglDisable(TGL_STENCIL_TEST);
	tglShadeModel(TGL_SMOOTH);
	tglEnable(TGL_LIGHTING);

	return true;
}

void XMeshTinyGL::renderEffect(Material *material) {
	tglMaterialfv(TGL_FRONT_AND_BACK, TGL_DIFFUSE, material->_material._diffuse._data);
	tglMaterialfv(TGL_FRONT_AND_BACK, TGL_AMBIENT, material->_material._diffuse._data);
	tglMaterialfv(TGL_FRONT_AND_BACK, TGL_SPECULAR, material->_material._specular._data);
	tglMaterialfv(TGL_FRONT_AND_BACK, TGL_EMISSION, material->_material._emissive._data);
	tglMaterialf(TGL_FRONT_AND_BACK, TGL_SHININESS, material->_material._power);
}

} // namespace Wintermute

#endif // defined(USE_TINYGL)
