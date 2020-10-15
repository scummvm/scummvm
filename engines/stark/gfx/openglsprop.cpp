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

#include "engines/stark/gfx/openglsprop.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"
#include "engines/stark/formats/biffmesh.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/shader.h"

namespace Stark {
namespace Gfx {

OpenGLSPropRenderer::OpenGLSPropRenderer(Driver *gfx) :
		VisualProp(),
		_gfx(gfx),
		_faceVBO(0),
		_modelIsDirty(true) {
	static const char* attributes[] = { "position", "normal", "texcoord", nullptr };
	_shader = OpenGL::ShaderGL::fromFiles("stark_prop", attributes);
}

OpenGLSPropRenderer::~OpenGLSPropRenderer() {
	clearVertices();

	delete _shader;
}

void OpenGLSPropRenderer::render(const Math::Vector3d &position, float direction, const LightEntryArray &lights) {
	if (_modelIsDirty) {
		// Update the OpenGL Buffer Objects if required
		clearVertices();
		uploadVertices();
		_modelIsDirty = false;
	}

	_gfx->set3DMode();

	Math::Matrix4 model = getModelMatrix(position, direction);
	Math::Matrix4 view = StarkScene->getViewMatrix();
	Math::Matrix4 projection = StarkScene->getProjectionMatrix();

	Math::Matrix4 modelViewMatrix = view * model;
	modelViewMatrix.transpose(); // OpenGL expects matrices transposed when compared to ScummVM's

	Math::Matrix4 projectionMatrix = projection;
	projectionMatrix.transpose(); // OpenGL expects matrices transposed when compared to ScummVM's

	Math::Matrix4 normalMatrix = modelViewMatrix;
	normalMatrix.invertAffineOrthonormal();
	//normalMatrix.transpose(); // OpenGL expects matrices transposed when compared to ScummVM's
	//normalMatrix.transpose(); // No need to transpose twice in a row

	_shader->enableVertexAttribute("position", _faceVBO, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
	_shader->enableVertexAttribute("normal", _faceVBO, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 12);
	_shader->enableVertexAttribute("texcoord", _faceVBO, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 24);
	_shader->use(true);

	_shader->setUniform("modelViewMatrix", modelViewMatrix);
	_shader->setUniform("projectionMatrix", projectionMatrix);
	_shader->setUniform("normalMatrix", normalMatrix.getRotation());
	setLightArrayUniform(lights);

	const Common::Array<Face> &faces = _model->getFaces();
	const Common::Array<Material> &materials = _model->getMaterials();

	for (Common::Array<Face>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		const Material &material = materials[face->materialId];

		// For each face draw its vertices from the VBO, indexed by the EBO
		const Gfx::Texture *tex = _texture->getTexture(material.texture);
		if (tex) {
			tex->bind();
		} else {
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		_shader->setUniform("textured", tex != nullptr);
		_shader->setUniform("color", Math::Vector3d(material.r, material.g, material.b));
		_shader->setUniform("doubleSided", material.doubleSided ? 1 : 0);

		GLuint ebo = _faceEBO[face];
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glDrawElements(GL_TRIANGLES, face->vertexIndices.size(), GL_UNSIGNED_INT, 0);
	}

	_shader->unbind();
}

void OpenGLSPropRenderer::clearVertices() {
	OpenGL::ShaderGL::freeBuffer(_faceVBO);

	for (FaceBufferMap::iterator it = _faceEBO.begin(); it != _faceEBO.end(); ++it) {
		OpenGL::ShaderGL::freeBuffer(it->_value);
	}

	_faceEBO.clear();
}

void OpenGLSPropRenderer::uploadVertices() {
	_faceVBO = createFaceVBO();

	const Common::Array<Face> &faces = _model->getFaces();
	for (Common::Array<Face>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		_faceEBO[face] = createFaceEBO(face);
	}
}

GLuint OpenGLSPropRenderer::createFaceVBO() {
	const Common::Array<Formats::BiffMesh::Vertex> &vertices = _model->getVertices();

	return OpenGL::ShaderGL::createBuffer(GL_ARRAY_BUFFER, sizeof(float) * 9 * vertices.size(), &vertices.front());
}

GLuint OpenGLSPropRenderer::createFaceEBO(const Face *face) {
	return OpenGL::ShaderGL::createBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * face->vertexIndices.size(), &face->vertexIndices.front());
}

void OpenGLSPropRenderer::setLightArrayUniform(const LightEntryArray &lights) {
	static const uint maxLights = 10;

	assert(lights.size() >= 1);
	assert(lights.size() <= maxLights);

	const LightEntry *ambient = lights[0];
	assert(ambient->type == LightEntry::kAmbient); // The first light must be the ambient light
	_shader->setUniform("ambientColor", ambient->color);

	Math::Matrix4 viewMatrix = StarkScene->getViewMatrix();
	Math::Matrix3 viewMatrixRot = viewMatrix.getRotation();

	for (uint i = 0; i < lights.size() - 1; i++) {
		const LightEntry *l = lights[i + 1];

		Math::Vector4d worldPosition;
		worldPosition.x() = l->position.x();
		worldPosition.y() = l->position.y();
		worldPosition.z() = l->position.z();
		worldPosition.w() = 1.0;

		Math::Vector4d eyePosition = viewMatrix * worldPosition;

		// The light type is stored in the w coordinate of the position to save an uniform slot
		eyePosition.w() = l->type;

		Math::Vector3d worldDirection = l->direction;
		Math::Vector3d eyeDirection = viewMatrixRot * worldDirection;
		eyeDirection.normalize();

		_shader->setUniform(Common::String::format("lights[%d].position", i).c_str(), eyePosition);
		_shader->setUniform(Common::String::format("lights[%d].direction", i).c_str(), eyeDirection);
		_shader->setUniform(Common::String::format("lights[%d].color", i).c_str(), l->color);

		Math::Vector4d params;
		params.x() = l->falloffNear;
		params.y() = l->falloffFar;
		params.z() = l->innerConeAngle.getCosine();
		params.w() = l->outerConeAngle.getCosine();

		_shader->setUniform(Common::String::format("lights[%d].params", i).c_str(), params);
	}

	for (uint i = lights.size() - 1; i < maxLights; i++) {
		// Make sure unused lights are disabled
		_shader->setUniform(Common::String::format("lights[%d].position", i).c_str(), Math::Vector4d());
	}
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
