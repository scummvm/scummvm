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

#if defined(USE_OPENGL_SHADERS)

#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
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

bool XMeshOpenGLShader::loadFromXData(const Common::String &filename, XFileData *xobj) {
	if (XMesh::loadFromXData(filename, xobj)) {
		auto indexData = _skinMesh->_mesh->_indexData;
		float *vertexData = _skinMesh->_mesh->_vertexData;
		uint32 vertexCount = _skinMesh->_mesh->_vertexCount;

		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, 4 * XSkinMeshLoader::kVertexComponentCount * vertexCount, vertexData, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * indexData.size(), indexData.data(), GL_STATIC_DRAW);

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool XMeshOpenGLShader::render(XModel *model) {
	float *vertexData = _skinMesh->_mesh->_vertexData;
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);

	_shader->enableVertexAttribute("position", _vertexBuffer, 3, GL_FLOAT, false, 4 * XSkinMeshLoader::kVertexComponentCount, 4 * XSkinMeshLoader::kPositionOffset);
	_shader->enableVertexAttribute("texcoord", _vertexBuffer, 2, GL_FLOAT, false, 4 * XSkinMeshLoader::kVertexComponentCount, 4 * XSkinMeshLoader::kTextureCoordOffset);
	_shader->enableVertexAttribute("normal", _vertexBuffer, 3, GL_FLOAT, false, 4 * XSkinMeshLoader::kVertexComponentCount, 4 * XSkinMeshLoader::kNormalOffset);

	_shader->use(true);

	for (uint32 i = 0; i < numAttrs; i++) {
		int materialIndex = attrs[i]._attribId;

		if (_materials[materialIndex]->getSurface()) {
			glEnable(GL_TEXTURE_2D);
			static_cast<BaseSurfaceOpenGL3D *>(_materials[materialIndex]->getSurface())->setTexture();
		} else {
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// wme does not seem to care about specular or emissive light values
		Math::Vector4d diffuse(_materials[materialIndex]->_material._diffuse._data);
		_shader->setUniform("diffuse", diffuse);
		_shader->setUniform("ambient", diffuse);

		size_t offset = 2 * attrsTable->_ptr[i]._faceStart * 3;
		glDrawElements(GL_TRIANGLES, attrsTable->_ptr[i]._faceCount * 3, GL_UNSIGNED_SHORT, (void *)offset);
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

bool XMeshOpenGLShader::renderFlatShadowModel() {
	float *vertexData = _skinMesh->_mesh->_vertexData;
	if (vertexData == nullptr) {
		return false;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);

	_flatShadowShader->enableVertexAttribute("position", _vertexBuffer, 3, GL_FLOAT, false, 4 * XSkinMeshLoader::kVertexComponentCount, 4 * XSkinMeshLoader::kPositionOffset);
	_flatShadowShader->use(true);

	glDrawElements(GL_TRIANGLES, _skinMesh->_mesh->_dxmesh->getNumFaces() * 3, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

bool XMeshOpenGLShader::update(FrameNode *parentFrame) {
	XMesh::update(parentFrame);

	float *vertexData = _skinMesh->_mesh->_vertexData;
	uint32 vertexCount = _skinMesh->_mesh->_vertexCount;

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * XSkinMeshLoader::kVertexComponentCount * vertexCount, vertexData);

	return true;
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_SHADERS)
