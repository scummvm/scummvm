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
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

#include "graphics/opengles2/shader.h"

namespace Stark {
namespace Gfx {

OpenGLSActorRenderer::OpenGLSActorRenderer() :
		VisualActor() {
	static const char* attributes[] = { "position1", "position2", "bone1", "bone2", "boneWeight", "texcoord", nullptr };
	_shader = Graphics::Shader::fromFiles("stark_actor", attributes);
}

OpenGLSActorRenderer::~OpenGLSActorRenderer() {
	clearVertices();

	delete _shader;
}

void OpenGLSActorRenderer::render(Gfx::Driver *gfx, const Math::Vector3d position, float direction) {
	if (_meshIsDirty) {
		// Update the OpenGL Buffer Objects if required
		clearVertices();
		uploadVertices();
		_meshIsDirty = false;
	}

	_model->getSkeleton()->animate(_time);

	gfx->set3DMode();

	Math::Matrix4 model = getModelMatrix(position, direction);
	Math::Matrix4 view = StarkScene->getViewMatrix();
	Math::Matrix4 projection = StarkScene->getProjectionMatrix();

	Math::Matrix4 mvp = projection * view * model;
	mvp.transpose();

	_shader->use(true);
	_shader->setUniform("mvp", mvp);
	setBoneRotationArrayUniform("boneRotation");
	setBonePositionArrayUniform("bonePosition");

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

			_shader->enableVertexAttribute("position1", vbo, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 0);
			_shader->enableVertexAttribute("position2", vbo, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 12);
			_shader->enableVertexAttribute("bone1", vbo, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 24);
			_shader->enableVertexAttribute("bone2", vbo, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 28);
			_shader->enableVertexAttribute("boneWeight", vbo, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 32);
			_shader->enableVertexAttribute("texcoord", vbo, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 36);
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
		Graphics::Shader::freeBuffer(it->_value);
	}

	for (FaceBufferMap::iterator it = _faceEBO.begin(); it != _faceEBO.end(); ++it) {
		Graphics::Shader::freeBuffer(it->_value);
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
	float *vertices = new float[11 * face->_verts.size()];
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

		*vertPtr++ = -(*tri)->_texS;
		*vertPtr++ = (*tri)->_texT;
	}

	uint32 vbo = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(float) * 11 * face->_verts.size(), vertices);
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

	uint32 ebo = Graphics::Shader::createBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * 3 * face->_tris.size(), indices);
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

} // End of namespace Gfx
} // End of namespace Stark
