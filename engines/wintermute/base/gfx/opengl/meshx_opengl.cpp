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

		glVertexPointer(3, GL_FLOAT, kVertexComponentCount * sizeof(float), _vertexData + kPositionOffset);
		glNormalPointer(GL_FLOAT, kVertexComponentCount * sizeof(float), _vertexData + kNormalOffset);
		if (textureEnable)
			glTexCoordPointer(2, GL_FLOAT, kVertexComponentCount * sizeof(float), _vertexData + kTextureCoordOffset);

		glDrawElements(GL_TRIANGLES, _indexRanges[i + 1] - _indexRanges[i], GL_UNSIGNED_SHORT, _indexData.data() + _indexRanges[i]);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	return true;
}

bool XMeshOpenGL::renderFlatShadowModel() {
	return true;
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_GAME)
