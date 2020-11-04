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

#include "engines/wintermute/base/gfx/x/material.h"
#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/meshx_opengl_shader.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
MeshXOpenGLShader::MeshXOpenGLShader(BaseGame *inGame, OpenGL::ShaderGL *shader, OpenGL::ShaderGL *flatShadowShader) :
	MeshX(inGame), _shader(shader), _flatShadowShader(flatShadowShader) {
	glGenBuffers(1, &_vertexBuffer);
	glGenBuffers(1, &_indexBuffer);
}

//////////////////////////////////////////////////////////////////////////
MeshXOpenGLShader::~MeshXOpenGLShader() {
	glDeleteBuffers(1, &_vertexBuffer);
	glDeleteBuffers(1, &_indexBuffer);
}

bool MeshXOpenGLShader::loadFromX(const Common::String &filename, XFileLexer &lexer, Common::Array<MaterialReference> &materialReferences) {
	if (MeshX::loadFromX(filename, lexer, materialReferences)) {
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, 4 * kVertexComponentCount * _vertexCount, _vertexData, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * _indexData.size(), _indexData.data(), GL_STATIC_DRAW);

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool MeshXOpenGLShader::render(ModelX *model) {
	if (_vertexData == nullptr) {
		return false;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);

	_shader->enableVertexAttribute("position", _vertexBuffer, 3, GL_FLOAT, false, 4 * kVertexComponentCount, 4 * kPositionOffset);
	_shader->enableVertexAttribute("texcoord", _vertexBuffer, 2, GL_FLOAT, false, 4 * kVertexComponentCount, 4 * kTextureCoordOffset);
	_shader->enableVertexAttribute("normal", _vertexBuffer, 3, GL_FLOAT, false, 4 * kVertexComponentCount, 4 * kNormalOffset);

	_shader->use(true);

	for (uint32 i = 0; i < _numAttrs; i++) {
		int materialIndex = _materialIndices[i];

		if (_materials[materialIndex]->getSurface()) {
			glEnable(GL_TEXTURE_2D);
			static_cast<BaseSurfaceOpenGL3D *>(_materials[materialIndex]->getSurface())->setTexture();
		} else {
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// wme does not seem to care about specular or emissive light values
		Math::Vector4d diffuse(_materials[materialIndex]->_diffuse.data);
		_shader->setUniform("diffuse", diffuse);
		_shader->setUniform("ambient", diffuse);

		size_t offset = 2 * _indexRanges[i];
		glDrawElements(GL_TRIANGLES, _indexRanges[i + 1] - _indexRanges[i], GL_UNSIGNED_SHORT, (void *)offset);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

bool MeshXOpenGLShader::renderFlatShadowModel() {
	if (_vertexData == nullptr) {
		return false;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);

	_flatShadowShader->enableVertexAttribute("position", _vertexBuffer, 3, GL_FLOAT, false, 4 * kVertexComponentCount, 4 * kPositionOffset);
	_flatShadowShader->use(true);

	glDrawElements(GL_TRIANGLES, _indexRanges.back(), GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

bool MeshXOpenGLShader::update(FrameNode *parentFrame) {
	MeshX::update(parentFrame);

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * kVertexComponentCount * _vertexCount, _vertexData);

	return true;
}

} // namespace Wintermute

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
