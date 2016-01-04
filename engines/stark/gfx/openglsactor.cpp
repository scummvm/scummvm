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

#include "engines/stark/gfx/openglsactor.h"

#include "engines/stark/model/model.h"
#include "engines/stark/model/skeleton.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"
#include "engines/stark/gfx/opengls.h"
#include "engines/stark/gfx/texture.h"

#include "graphics/opengl/shader.h"

namespace Stark {
namespace Gfx {

OpenGLSActorRenderer::OpenGLSActorRenderer(OpenGLSDriver *gfx) :
		VisualActor(),
		_gfx(gfx) {
	_shader = _gfx->createActorShaderInstance();
}

OpenGLSActorRenderer::~OpenGLSActorRenderer() {
	clearVertices();

	delete _shader;
}

void OpenGLSActorRenderer::render(const Math::Vector3d position, float direction, const LightEntryArray &lights) {
	if (_meshIsDirty) {
		// Update the OpenGL Buffer Objects if required
		clearVertices();
		uploadVertices();
		_meshIsDirty = false;
	}

	_model->getSkeleton()->animate(_time);

	_gfx->set3DMode();

	Math::Matrix4 model = getModelMatrix(position, direction);
	Math::Matrix4 view = StarkScene->getViewMatrix();
	Math::Matrix4 projection = StarkScene->getProjectionMatrix();

	Math::Matrix4 modelViewMatrix = view * model;
	modelViewMatrix.transpose(); // OpenGL expects matrices transposed when compared to ResidualVM's

	Math::Matrix4 projectionMatrix = projection;
	projectionMatrix.transpose(); // OpenGL expects matrices transposed when compared to ResidualVM's

	Math::Matrix4 normalMatrix = modelViewMatrix;
	normalMatrix.invertAffineOrthonormal();
	//normalMatrix.transpose(); // OpenGL expects matrices transposed when compared to ResidualVM's
	//normalMatrix.transpose(); // No need to transpose twice in a row

	_shader->use(true);
	_shader->setUniform("modelViewMatrix", modelViewMatrix);
	_shader->setUniform("projectionMatrix", projectionMatrix);
	_shader->setUniform("normalMatrix", normalMatrix.getRotation());
	setBoneRotationArrayUniform("boneRotation");
	setBonePositionArrayUniform("bonePosition");
	setLightArrayUniform("lights", lights);

	Common::Array<MeshNode *> meshes = _model->getMeshes();
	Common::Array<MaterialNode *> mats = _model->getMaterials();
	const Gfx::TextureSet *texture = _model->getTextureSet();

	for (Common::Array<MeshNode *>::const_iterator mesh = meshes.begin(); mesh != meshes.end(); ++mesh) {
		for (Common::Array<FaceNode *>::const_iterator face = (*mesh)->_faces.begin(); face != (*mesh)->_faces.end(); ++face) {
			// For each face draw its vertices from the VBO, indexed by the EBO
			const MaterialNode *material = mats[(*face)->_matIdx];
			const Gfx::Texture *tex = texture->getTexture(material->_texName);
			if (tex) {
				tex->bind();
			} else {
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			GLuint vbo = _faceVBO[*face];
			GLuint ebo = _faceEBO[*face];

			_shader->enableVertexAttribute("position1", vbo, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 0);
			_shader->enableVertexAttribute("position2", vbo, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 12);
			_shader->enableVertexAttribute("bone1", vbo, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 24);
			_shader->enableVertexAttribute("bone2", vbo, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 28);
			_shader->enableVertexAttribute("boneWeight", vbo, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 32);
			_shader->enableVertexAttribute("normal", vbo, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 36);
			_shader->enableVertexAttribute("texcoord", vbo, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 48);
			_shader->use(true);
			_shader->setUniform("textured", tex != nullptr);
			_shader->setUniform("color", Math::Vector3d(material->_r, material->_g, material->_b));

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glDrawElements(GL_TRIANGLES, 3 * (*face)->_tris.size(), GL_UNSIGNED_INT, 0);

			glUseProgram(0);
		}
	}
}

void OpenGLSActorRenderer::clearVertices() {
	for (FaceBufferMap::iterator it = _faceVBO.begin(); it != _faceVBO.end(); ++it) {
		OpenGL::Shader::freeBuffer(it->_value);
	}

	for (FaceBufferMap::iterator it = _faceEBO.begin(); it != _faceEBO.end(); ++it) {
		OpenGL::Shader::freeBuffer(it->_value);
	}

	_faceVBO.clear();
	_faceEBO.clear();
}

void OpenGLSActorRenderer::uploadVertices() {
	Common::Array<MeshNode *> meshes = _model->getMeshes();
	for (Common::Array<MeshNode *>::const_iterator mesh = meshes.begin(); mesh != meshes.end(); ++mesh) {
		for (Common::Array<FaceNode *>::const_iterator face = (*mesh)->_faces.begin(); face != (*mesh)->_faces.end(); ++face) {
			_faceVBO[*face] = createFaceVBO(*face);
			_faceEBO[*face] = createFaceEBO(*face);
		}
	}
}

uint32 OpenGLSActorRenderer::createFaceVBO(const FaceNode *face) {
	float *vertices = new float[14 * face->_verts.size()];
	float *vertPtr = vertices;

	// Build a vertex array
	for (Common::Array<VertNode *>::const_iterator tri = face->_verts.begin(); tri != face->_verts.end(); ++tri) {
		*vertPtr++ = (*tri)->_pos1.x();
		*vertPtr++ = (*tri)->_pos1.y();
		*vertPtr++ = (*tri)->_pos1.z();

		*vertPtr++ = (*tri)->_pos2.x();
		*vertPtr++ = (*tri)->_pos2.y();
		*vertPtr++ = (*tri)->_pos2.z();

		*vertPtr++ = (*tri)->_bone1;
		*vertPtr++ = (*tri)->_bone2;

		*vertPtr++ = (*tri)->_boneWeight;

		*vertPtr++ = (*tri)->_normal.x();
		*vertPtr++ = (*tri)->_normal.y();
		*vertPtr++ = (*tri)->_normal.z();

		*vertPtr++ = -(*tri)->_texS;
		*vertPtr++ = (*tri)->_texT;
	}

	uint32 vbo = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(float) * 14 * face->_verts.size(), vertices);
	delete[] vertices;

	return vbo;
}

uint32 OpenGLSActorRenderer::createFaceEBO(const FaceNode *face) {
	uint32 *indices = new uint32[3 * face->_tris.size()];
	uint32 *idxPtr = indices;

	// Build a vertex indices array
	for (Common::Array<TriNode *>::const_iterator tri = face->_tris.begin(); tri != face->_tris.end(); ++tri) {
		for (int vert = 0; vert < 3; ++vert) {
			if (vert == 0)
				*idxPtr++ = (*tri)->_vert1;
			else if (vert == 1)
				*idxPtr++ = (*tri)->_vert2;
			else
				*idxPtr++ = (*tri)->_vert3;
		}
	}

	uint32 ebo = OpenGL::Shader::createBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * 3 * face->_tris.size(), indices);
	delete[] indices;

	return ebo;
}

void OpenGLSActorRenderer::setBonePositionArrayUniform(const char *uniform) {
	const Common::Array<BoneNode *> bones = _model->getSkeleton()->getBones();

	GLint pos = _shader->getUniformLocation(uniform);
	if (pos == -1) {
		error("No uniform named '%s'", uniform);
	}

	float *positions = new float[3 * bones.size()];
	float *positionsPtr = positions;

	for (uint i = 0; i < bones.size(); i++) {
		*positionsPtr++ = bones[i]->_animPos.x();
		*positionsPtr++ = bones[i]->_animPos.y();
		*positionsPtr++ = bones[i]->_animPos.z();
	}

	glUniform3fv(pos, bones.size(), positions);
	delete[] positions;
}

void OpenGLSActorRenderer::setBoneRotationArrayUniform(const char *uniform) {
	const Common::Array<BoneNode *> bones = _model->getSkeleton()->getBones();

	GLint rot = _shader->getUniformLocation(uniform);
	if (rot == -1) {
		error("No uniform named '%s'", uniform);
	}

	float *rotations = new float[4 * bones.size()];
	float *rotationsPtr = rotations;

	for (uint i = 0; i < bones.size(); i++) {
		*rotationsPtr++ =  bones[i]->_animRot.x();
		*rotationsPtr++ =  bones[i]->_animRot.y();
		*rotationsPtr++ =  bones[i]->_animRot.z();
		*rotationsPtr++ =  bones[i]->_animRot.w();
	}

	glUniform4fv(rot, bones.size(), rotations);
	delete[] rotations;
}

void OpenGLSActorRenderer::setLightArrayUniform(const char *uniform, const LightEntryArray &lights) {
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
