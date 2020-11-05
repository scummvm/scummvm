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

#if (defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)) && !defined(USE_GLES2)

#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/meshx_opengl.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
MeshXOpenGL::MeshXOpenGL(BaseGame *inGame) : MeshX(inGame) {
}

//////////////////////////////////////////////////////////////////////////
MeshXOpenGL::~MeshXOpenGL() {
}

//////////////////////////////////////////////////////////////////////////
bool MeshXOpenGL::render(ModelX *model) {
	if (_vertexData == nullptr) {
		return false;
	}

	for (uint32 i = 0; i < _numAttrs; i++) {
		int materialIndex = _materialIndices[i];
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _materials[materialIndex]->_diffuse.data);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _materials[materialIndex]->_diffuse.data);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, _materials[materialIndex]->_specular.data);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, _materials[materialIndex]->_emissive.data);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, _materials[materialIndex]->_shininess);

		if (_materials[materialIndex]->getSurface()) {
			glEnable(GL_TEXTURE_2D);
			static_cast<BaseSurfaceOpenGL3D *>(_materials[materialIndex]->getSurface())->setTexture();
		} else {
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, kVertexComponentCount * sizeof(float), _vertexData + kPositionOffset);
		glNormalPointer(GL_FLOAT, kVertexComponentCount * sizeof(float), _vertexData + kNormalOffset);
		glTexCoordPointer(2, GL_FLOAT, kVertexComponentCount * sizeof(float), _vertexData + kTextureCoordOffset);

		glDrawElements(GL_TRIANGLES, _indexRanges[i + 1] - _indexRanges[i], GL_UNSIGNED_SHORT, _indexData.data() + _indexRanges[i]);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	return true;
}

bool MeshXOpenGL::renderFlatShadowModel() {
	return true;
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_GAME) && !defined(USE_GLES2)
