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
#include "engines/stark/model/animhandler.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/settings.h"
#include "engines/stark/gfx/opengls.h"
#include "engines/stark/gfx/texture.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/shader.h"

namespace Stark {
namespace Gfx {

OpenGLSActorRenderer::OpenGLSActorRenderer(OpenGLSDriver *gfx) :
		VisualActor(),
		_gfx(gfx),
		_faceVBO(0) {
	_shader = _gfx->createActorShaderInstance();
	_shadowShader = _gfx->createShadowShaderInstance();
}

OpenGLSActorRenderer::~OpenGLSActorRenderer() {
	clearVertices();

	delete _shader;
	delete _shadowShader;
}

void OpenGLSActorRenderer::render(const Math::Vector3d &position, float direction, const LightEntryArray &lights) {
	if (_modelIsDirty) {
		// Update the OpenGL Buffer Objects if required
		clearVertices();
		uploadVertices();
		_modelIsDirty = false;
	}

	// TODO: Move updates outside of the rendering code
	_animHandler->animate(_time);
	_model->updateBoundingBox();

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

	_shader->enableVertexAttribute("position1", _faceVBO, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 0);
	_shader->enableVertexAttribute("position2", _faceVBO, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 12);
	_shader->enableVertexAttribute("bone1", _faceVBO, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 24);
	_shader->enableVertexAttribute("bone2", _faceVBO, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 28);
	_shader->enableVertexAttribute("boneWeight", _faceVBO, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 32);
	_shader->enableVertexAttribute("normal", _faceVBO, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 36);
	_shader->enableVertexAttribute("texcoord", _faceVBO, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 48);
	_shader->use(true);

	_shader->setUniform("modelViewMatrix", modelViewMatrix);
	_shader->setUniform("projectionMatrix", projectionMatrix);
	_shader->setUniform("normalMatrix", normalMatrix.getRotation());
	setBoneRotationArrayUniform(_shader, "boneRotation");
	setBonePositionArrayUniform(_shader, "bonePosition");
	setLightArrayUniform(lights);

	Common::Array<Face *> faces = _model->getFaces();
	Common::Array<Material *> mats = _model->getMaterials();

	for (Common::Array<Face *>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		// For each face draw its vertices from the VBO, indexed by the EBO
		const Material *material = mats[(*face)->materialId];
		const Gfx::Texture *tex = resolveTexture(material);
		if (tex) {
			tex->bind();
		} else {
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		_shader->setUniform("textured", tex != nullptr);
		_shader->setUniform("color", Math::Vector3d(material->r, material->g, material->b));

		GLuint ebo = _faceEBO[*face];
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glDrawElements(GL_TRIANGLES, (*face)->vertexIndices.size(), GL_UNSIGNED_INT, 0);
	}

	_shader->unbind();

	if (_castsShadow
	        && StarkScene->shouldRenderShadows()
	        && StarkSettings->getBoolSetting(Settings::kShadow)) {
		glEnable(GL_BLEND);
		glEnable(GL_STENCIL_TEST);

		_shadowShader->enableVertexAttribute("position1", _faceVBO, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 0);
		_shadowShader->enableVertexAttribute("position2", _faceVBO, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 12);
		_shadowShader->enableVertexAttribute("bone1", _faceVBO, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 24);
		_shadowShader->enableVertexAttribute("bone2", _faceVBO, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 28);
		_shadowShader->enableVertexAttribute("boneWeight", _faceVBO, 1, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 32);
		_shadowShader->use(true);

		Math::Matrix4 mvp = projection * view * model;
		mvp.transpose();
		_shadowShader->setUniform("mvp", mvp);

		setBoneRotationArrayUniform(_shadowShader, "boneRotation");
		setBonePositionArrayUniform(_shadowShader, "bonePosition");

		Math::Matrix4 modelInverse = model;
		modelInverse.inverse();
		setShadowUniform(lights, position, modelInverse.getRotation());

		for (Common::Array<Face *>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
			GLuint ebo = _faceEBO[*face];
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glDrawElements(GL_TRIANGLES, (*face)->vertexIndices.size(), GL_UNSIGNED_INT, 0);
		}

		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);

		_shadowShader->unbind();
	}
}

void OpenGLSActorRenderer::clearVertices() {
	OpenGL::ShaderGL::freeBuffer(_faceVBO); // Zero names are silently ignored
	_faceVBO = 0;

	for (FaceBufferMap::iterator it = _faceEBO.begin(); it != _faceEBO.end(); ++it) {
		OpenGL::ShaderGL::freeBuffer(it->_value);
	}

	_faceEBO.clear();
}

void OpenGLSActorRenderer::uploadVertices() {
	_faceVBO = createModelVBO(_model);

	Common::Array<Face *> faces = _model->getFaces();
	for (Common::Array<Face *>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		_faceEBO[*face] = createFaceEBO(*face);
	}
}

GLuint OpenGLSActorRenderer::createModelVBO(const Model *model) {
	const Common::Array<VertNode *> &modelVertices = model->getVertices();

	float *vertices = new float[14 * modelVertices.size()];
	float *vertPtr = vertices;

	// Build a vertex array
	for (Common::Array<VertNode *>::const_iterator tri = modelVertices.begin(); tri != modelVertices.end(); ++tri) {
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

	GLuint vbo = OpenGL::ShaderGL::createBuffer(GL_ARRAY_BUFFER, sizeof(float) * 14 * modelVertices.size(), vertices);
	delete[] vertices;

	return vbo;
}

GLuint OpenGLSActorRenderer::createFaceEBO(const Face *face) {
	return OpenGL::ShaderGL::createBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * face->vertexIndices.size(), &face->vertexIndices[0]);
}

void OpenGLSActorRenderer::setBonePositionArrayUniform(OpenGL::ShaderGL *shader, const char *uniform) {
	const Common::Array<BoneNode *> &bones = _model->getBones();

	GLint pos = shader->getUniformLocation(uniform);
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

void OpenGLSActorRenderer::setBoneRotationArrayUniform(OpenGL::ShaderGL *shader, const char *uniform) {
	const Common::Array<BoneNode *> &bones = _model->getBones();

	GLint rot = shader->getUniformLocation(uniform);
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

void OpenGLSActorRenderer::setLightArrayUniform(const LightEntryArray &lights) {
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

void OpenGLSActorRenderer::setShadowUniform(const LightEntryArray &lights,
		const Math::Vector3d &actorPosition, Math::Matrix3 worldToModelRot) {
	Math::Vector3d sumDirection;
	bool hasLight = false;

	// Compute the contribution from each lights
	// The ambient light is skipped intentionally
	for (uint i = 1; i < lights.size(); ++i) {
		LightEntry *light = lights[i];
		bool contributes = false;

		Math::Vector3d lightDirection;
		switch (light->type) {
			case LightEntry::kPoint:
				contributes = getPointLightContribution(light, actorPosition, lightDirection);
				break;
			case LightEntry::kDirectional:
				contributes = getDirectionalLightContribution(light, lightDirection);
				break;
			case LightEntry::kSpot:
				contributes = getSpotLightContribution(light, actorPosition, lightDirection);
				break;
			case LightEntry::kAmbient:
			default:
				break;
		}

		if (contributes) {
			sumDirection += lightDirection;
			hasLight = true;
		}
	}

	if (hasLight) {
		// Clip the horizontal length
		Math::Vector2d horizontalProjection(sumDirection.x(), sumDirection.y());
		float shadowLength = MIN(horizontalProjection.getMagnitude(), StarkScene->getMaxShadowLength());

		horizontalProjection.normalize();
		horizontalProjection *= shadowLength;

		sumDirection.x() = horizontalProjection.getX();
		sumDirection.y() = horizontalProjection.getY();
		sumDirection.z() = -1;
	} else {
		// Cast from above by default
		sumDirection.x() = 0;
		sumDirection.y() = 0;
		sumDirection.z() = -1;
	}

	//Transform the direction to the model space and pass to the shader
	sumDirection = worldToModelRot * sumDirection;
	_shadowShader->setUniform("lightDirection", sumDirection);
}

bool OpenGLSActorRenderer::getPointLightContribution(LightEntry *light,
		const Math::Vector3d &actorPosition, Math::Vector3d &direction, float weight) {
	float distance = light->position.getDistanceTo(actorPosition);

	if (distance > light->falloffFar) {
		return false;
	}

	float factor;
	if (distance > light->falloffNear) {
		if (light->falloffFar - light->falloffNear > 1) {
			factor = 1 - (distance - light->falloffNear) / (light->falloffFar - light->falloffNear);
		} else {
			factor = 0;
		}
	} else {
		factor = 1;
	}

	float brightness = (light->color.x() + light->color.y() + light->color.z()) / 3.0f;

	if (factor <= 0 || brightness <= 0) {
		return false;
	}

	direction = actorPosition - light->position;
	direction.normalize();
	direction *= factor * brightness * weight;

	return true;
}

bool OpenGLSActorRenderer::getDirectionalLightContribution(LightEntry *light, Math::Vector3d &direction) {
	float brightness = (light->color.x() + light->color.y() + light->color.z()) / 3.0f;

	if (brightness <= 0) {
		return false;
	}

	direction = light->direction;
	direction.normalize();
	direction *= brightness;

	return true;
}

bool OpenGLSActorRenderer::getSpotLightContribution(LightEntry *light,
		const Math::Vector3d &actorPosition, Math::Vector3d &direction) {
	Math::Vector3d lightToActor = actorPosition - light->position;
	lightToActor.normalize();

	float cosAngle = MAX(0.0f, lightToActor.dotProduct(light->direction));
	float cone = (cosAngle - light->innerConeAngle.getCosine()) /
			MAX(0.001f, light->outerConeAngle.getCosine() - light->innerConeAngle.getCosine());
	cone = CLIP(cone, 0.0f, 1.0f);

	if (cone <= 0) {
		return false;
	}

	return getPointLightContribution(light, actorPosition, direction, cone);
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
