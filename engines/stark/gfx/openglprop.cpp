/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/stark/gfx/openglprop.h"

#include "engines/stark/gfx/texture.h"
#include "engines/stark/formats/biffmesh.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

#if defined(USE_OPENGL_GAME)

namespace Stark {
namespace Gfx {

OpenGLPropRenderer::OpenGLPropRenderer(OpenGLDriver *gfx) :
		VisualProp(),
		_gfx(gfx),
		_faceVBO(nullptr),
		_modelIsDirty(true) {
}

OpenGLPropRenderer::~OpenGLPropRenderer() {
	clearVertices();
}

void OpenGLPropRenderer::render(const Math::Vector3d &position, float direction, const LightEntryArray &lights) {
	if (_modelIsDirty) {
		// Update the OpenGL Buffer Objects if required
		clearVertices();
		uploadVertices();
		_modelIsDirty = false;
	}

	_gfx->set3DMode();
	_gfx->setupLights(lights);

	Math::Matrix4 model = getModelMatrix(position, direction);
	Math::Matrix4 view = StarkScene->getViewMatrix();
	Math::Matrix4 projection = StarkScene->getProjectionMatrix();

	Math::Matrix4 modelViewMatrix = view * model;
	modelViewMatrix.transpose(); // OpenGL expects matrices transposed when compared to ScummVM's

	Math::Matrix4 projectionMatrix = projection;
	projectionMatrix.transpose(); // OpenGL expects matrices transposed when compared to ScummVM's

	const Common::Array<Face> &faces = _model->getFaces();
	const Common::Array<Material> &materials = _model->getMaterials();

	glEnable(GL_COLOR_MATERIAL);
	for (Common::Array<Face>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		const Material &material = materials[face->materialId];

		// For each face draw its vertices from the VBO, indexed by the EBO
		const Gfx::Texture *tex = _texture->getTexture(material.texture);
		if (material.doubleSided)
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		else
			glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		if (tex) {
			tex->bind();
			glColor3f(1.0f, 1.0f, 1.0f);
		} else {
			glBindTexture(GL_TEXTURE_2D, 0);
			glColor3f(material.r, material.g, material.b);
		}

		auto vertexIndices = _faceEBO[face];

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		glVertexPointer(3, GL_FLOAT, sizeof(PropVertex), &_faceVBO[0].x);
		if (tex)
			glTexCoordPointer(2, GL_FLOAT, sizeof(PropVertex), &_faceVBO[0].texS);
		glNormalPointer(GL_FLOAT, sizeof(PropVertex), &_faceVBO[0].nx);

		glDrawElements(GL_TRIANGLES, face->vertexIndices.size(), GL_UNSIGNED_INT, vertexIndices);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	}
	glDisable(GL_COLOR_MATERIAL);

}

void OpenGLPropRenderer::clearVertices() {
	delete[] _faceVBO;
	_faceVBO = nullptr;

	for (FaceBufferMap::iterator it = _faceEBO.begin(); it != _faceEBO.end(); ++it) {
		delete[] it->_value;
	}

	_faceEBO.clear();
}

void OpenGLPropRenderer::uploadVertices() {
	_faceVBO = createFaceVBO();

	const Common::Array<Face> &faces = _model->getFaces();
	for (Common::Array<Face>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		_faceEBO[face] = createFaceEBO(face);
	}
}

PropVertex *OpenGLPropRenderer::createFaceVBO() {
	const Common::Array<Formats::BiffMesh::Vertex> &modelVertices = _model->getVertices();
	auto vertices = new PropVertex[modelVertices.size()];
	// Build a vertex array
	for (uint32 i = 0; i < modelVertices.size(); i++) {
		vertices[i].x = modelVertices[i].position.x();
		vertices[i].y = modelVertices[i].position.y();
		vertices[i].z = modelVertices[i].position.z();
		vertices[i].nx = modelVertices[i].normal.x();
		vertices[i].ny = modelVertices[i].normal.y();
		vertices[i].nz = modelVertices[i].normal.z();
		vertices[i].texS = modelVertices[i].texturePosition.x();
		vertices[i].texT = modelVertices[i].texturePosition.y();
	}

	return vertices;
}

uint32 *OpenGLPropRenderer::createFaceEBO(const Face *face) {
	auto indices = new uint32[face->vertexIndices.size()];
	for (uint32 index = 0; index < face->vertexIndices.size(); index++) {
		indices[index] = face->vertexIndices[index];
	}

	return indices;
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_OPENGL_GAME)
