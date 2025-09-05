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
#include "engines/wintermute/base/gfx/base_renderer3d.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL_SHADERS)

#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d_shader.h"
#include "engines/wintermute/base/gfx/opengl/meshx_opengl_shader.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
XMeshOpenGLShader::XMeshOpenGLShader(BaseGame *inGame, OpenGL::Shader *shader, OpenGL::Shader *flatShadowShader) :
	XMesh(inGame), _shader(shader), _flatShadowShader(flatShadowShader) {
	glGenBuffers(1, &_vertexBuffer);
	glGenBuffers(1, &_indexBuffer);
}

//////////////////////////////////////////////////////////////////////////
XMeshOpenGLShader::~XMeshOpenGLShader() {
	glDeleteBuffers(1, &_vertexBuffer);
	glDeleteBuffers(1, &_indexBuffer);
}

bool XMeshOpenGLShader::loadFromXData(const char *filename, XFileData *xobj) {
	if (XMesh::loadFromXData(filename, xobj)) {
		uint32 *indexData = (uint32 *)_blendedMesh->getIndexBuffer().ptr();
		uint32 indexDataSize = _blendedMesh->getIndexBuffer().size() / sizeof(uint32);
		float *vertexData = (float *)_blendedMesh->getVertexBuffer().ptr();
		uint32 vertexSize = DXGetFVFVertexSize(_blendedMesh->getFVF()) / sizeof(float);
		uint32 vertexCount = _blendedMesh->getNumVertices();

		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, 4 * vertexSize * vertexCount, vertexData, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * indexDataSize, indexData, GL_STATIC_DRAW);

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool XMeshOpenGLShader::render(XModel *model) {
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);

	_shader->enableVertexAttribute("position", _vertexBuffer, 3, GL_FLOAT, false, 4 * vertexSize, 0);
	_shader->enableVertexAttribute("texcoord", _vertexBuffer, 2, GL_FLOAT, false, 4 * vertexSize, 4 * textureOffset);
	_shader->enableVertexAttribute("normal", _vertexBuffer, 3, GL_FLOAT, false, 4 * vertexSize, 4 * normalOffset);
	_shader->use(true);

	for (uint32 i = 0; i < numAttrs; i++) {
		Material *mat = _materials[attrs[i]._attribId];
		if (mat->getSurface()) {
			glEnable(GL_TEXTURE_2D);
			static_cast<BaseSurfaceOpenGL3D *>(mat->getSurface())->setTexture();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
			_shader->setUniform("useTexture", true);
		} else {
			_shader->setUniform("useTexture", false);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}

		if (mat->getEffect()) {
			renderEffect(mat);
		} else {
			Math::Vector4d diffuse(mat->_material._diffuse._data);
			_shader->setUniform("diffuse", diffuse);
			_shader->setUniform("ambient", diffuse);
		}

		size_t offsetFace = 4 * attrsTable->_ptr[i]._faceStart * 3;
		glDrawElements(GL_TRIANGLES, attrsTable->_ptr[i]._faceCount * 3, GL_UNSIGNED_INT, (void *)offsetFace);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (noAttrs) {
		delete[] attrs;
	}

	return true;
}

bool XMeshOpenGLShader::renderFlatShadowModel(uint32 shadowColor) {
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

	Math::Vector4d color;
	color.x() = RGBCOLGetR(shadowColor) / 255.0f;
	color.y() = RGBCOLGetG(shadowColor) / 255.0f;
	color.z() = RGBCOLGetB(shadowColor) / 255.0f;
	color.w() = RGBCOLGetA(shadowColor) / 255.0f;

	_flatShadowShader->enableVertexAttribute("position", _vertexBuffer, 3, GL_FLOAT, false, 4 * vertexSize, 0);
	_flatShadowShader->use(true);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, (GLuint)~0);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);

	for (uint32 i = 0; i < numAttrs; i++) {
		size_t offsetFace = 4 * attrsTable->_ptr[i]._faceStart * 3;
		glDrawElements(GL_TRIANGLES, attrsTable->_ptr[i]._faceCount * 3, GL_UNSIGNED_INT, (void *)offsetFace);
	}

	glStencilFunc(GL_EQUAL, 1, (GLuint)~0);
	glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

	_flatShadowShader->setUniform("shadowColor", color);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthMask(GL_TRUE);

	for (uint32 i = 0; i < numAttrs; i++) {
		size_t offsetFace = 4 * attrsTable->_ptr[i]._faceStart * 3;
		glDrawElements(GL_TRIANGLES, attrsTable->_ptr[i]._faceCount * 3, GL_UNSIGNED_INT, (void *)offsetFace);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (noAttrs) {
		delete[] attrs;
	}

	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);

	return true;
}

bool XMeshOpenGLShader::update(FrameNode *parentFrame) {
	XMesh::update(parentFrame);

	float *vertexData = (float *)_blendedMesh->getVertexBuffer().ptr();
	uint32 vertexSize = DXGetFVFVertexSize(_blendedMesh->getFVF()) / sizeof(float);
	uint32 vertexCount = _blendedMesh->getNumVertices();

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * vertexSize * vertexCount, vertexData);

	return true;
}

void XMeshOpenGLShader::renderEffect(Material *material) {
	Math::Vector4d diffuse(material->_material._diffuse._data);
	_shader->setUniform("diffuse", diffuse);
	_shader->setUniform("ambient", diffuse);
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_SHADERS)
