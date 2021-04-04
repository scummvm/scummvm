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

#include "engines/stark/gfx/openglactor.h"

#include "engines/stark/model/model.h"
#include "engines/stark/model/animhandler.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/settings.h"
#include "engines/stark/gfx/texture.h"

#include "math/vector2d.h"

#if defined(USE_OPENGL_GAME)

namespace Stark {
namespace Gfx {

OpenGLActorRenderer::OpenGLActorRenderer(OpenGLDriver *gfx) :
		VisualActor(),
		_gfx(gfx),
		_faceVBO(nullptr) {
}

OpenGLActorRenderer::~OpenGLActorRenderer() {
	clearVertices();
}

void OpenGLActorRenderer::render(const Math::Vector3d &position, float direction, const LightEntryArray &lights) {
	if (_modelIsDirty) {
		clearVertices();
		uploadVertices();
		_modelIsDirty = false;
	}

	// TODO: Move updates outside of the rendering code
	_animHandler->animate(_time);
	_model->updateBoundingBox();

	bool drawShadow = false;
	if (_castsShadow
			&& StarkScene->shouldRenderShadows()
			&& StarkSettings->getBoolSetting(Settings::kShadow)) {
		drawShadow = true;
	}
	Math::Vector3d lightDirection;

	_gfx->set3DMode();
	_gfx->setupLights(lights);

	Math::Matrix4 model = getModelMatrix(position, direction);
	Math::Matrix4 view = StarkScene->getViewMatrix();
	Math::Matrix4 projection = StarkScene->getProjectionMatrix();

	Math::Matrix4 modelViewMatrix = view * model;
	modelViewMatrix.transpose(); // OpenGL expects matrices transposed when compared to ScummVM's

	Math::Matrix4 projectionMatrix = projection;
	projectionMatrix.transpose(); // OpenGL expects matrices transposed when compared to ScummVM's

	Math::Matrix4 mvp;
	if (drawShadow) {
		mvp = view * model;
		mvp.transpose();
		Math::Matrix4 modelInverse = model;
		modelInverse.inverse();
		lightDirection = getShadowLightDirection(lights, position, modelInverse.getRotation());
	}

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projectionMatrix.getData());

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelViewMatrix.getData());

	glEnable(GL_TEXTURE_2D);

	Common::Array<Face *> faces = _model->getFaces();
	Common::Array<Material *> mats = _model->getMaterials();
	const Common::Array<BoneNode *> &bones = _model->getBones();

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	for (Common::Array<Face *>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		const Material *material = mats[(*face)->materialId];
		const Gfx::Texture *tex = resolveTexture(material);
		if (tex) {
			tex->bind();
			glColor3f(1.0f, 1.0f, 1.0f);
		} else {
			glBindTexture(GL_TEXTURE_2D, 0);
			glColor3f(material->r, material->g, material->b);
		}

		auto vertexIndices = _faceEBO[*face];
		auto numVertexIndices = (*face)->vertexIndices.size();
		for (uint32 i = 0; i < numVertexIndices; i++) {
			uint32 index = vertexIndices[i];
			auto vertex = _faceVBO[index];
			uint32 bone1 = vertex.bone1;
			uint32 bone2 = vertex.bone2;
			Math::Vector3d position1 = vertex.pos1;
			Math::Vector3d position2 = vertex.pos2;
			Math::Vector3d bone1Position = Math::Vector3d(bones[bone1]->_animPos.x(),
														  bones[bone1]->_animPos.y(),
														  bones[bone1]->_animPos.z());
			Math::Vector3d bone2Position = Math::Vector3d(bones[bone2]->_animPos.x(),
														  bones[bone2]->_animPos.y(),
														  bones[bone2]->_animPos.z());
			Math::Quaternion bone1Rotation = Math::Quaternion(bones[bone1]->_animRot.x(),
														  bones[bone1]->_animRot.y(),
														  bones[bone1]->_animRot.z(),
														  bones[bone1]->_animRot.w());
			Math::Quaternion bone2Rotation = Math::Quaternion(bones[bone2]->_animRot.x(),
														  bones[bone2]->_animRot.y(),
														  bones[bone2]->_animRot.z(),
														  bones[bone2]->_animRot.w());
			float boneWeight = vertex.boneWeight;
			Math::Vector3d normal = vertex.normal;

			// Compute the vertex position in eye-space
			bone1Rotation.transform(position1);
			position1 += bone1Position;
			bone2Rotation.transform(position2);
			position2 += bone2Position;
			Math::Vector3d modelPosition = position2 * (1.0 - boneWeight) + position1 * boneWeight;
			vertex.x = modelPosition.x();
			vertex.y = modelPosition.y();
			vertex.z = modelPosition.z();

			// Compute the vertex normal in eye-space
			Math::Vector3d n1 = normal;
			bone1Rotation.transform(n1);
			Math::Vector3d n2 = normal;
			bone2Rotation.transform(n2);
			Math::Vector3d modelNormal = Math::Vector3d(n2 * (1.0 - boneWeight) + n1 * boneWeight).getNormalized();
			vertex.nx = modelNormal.x();
			vertex.ny = modelNormal.y();
			vertex.nz = modelNormal.z();

			if (drawShadow) {
				Math::Vector3d shadowPosition = modelPosition + lightDirection * (-modelPosition.y() / lightDirection.y());
				vertex.sx = shadowPosition.x();
				vertex.sy = 0.0f;
				vertex.sz = shadowPosition.z();
			}

			_faceVBO[index] = vertex;
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		glVertexPointer(3, GL_FLOAT, sizeof(ActorVertex), &_faceVBO[0].x);
		if (tex)
			glTexCoordPointer(2, GL_FLOAT, sizeof(ActorVertex), &_faceVBO[0].texS);
		glNormalPointer(GL_FLOAT, sizeof(ActorVertex), &_faceVBO[0].nx);

		glDrawElements(GL_TRIANGLES, numVertexIndices, GL_UNSIGNED_INT, vertexIndices);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	}
	glDisable(GL_COLOR_MATERIAL);


	if (drawShadow) {
		glEnable(GL_BLEND);
		glEnable(GL_STENCIL_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(projectionMatrix.getData());

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(mvp.getData());

		glColor4f(0.0f, 0.0f, 0.0f, 0.5f);

		for (Common::Array<Face *>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
			auto vertexIndices = _faceEBO[*face];

			glEnableClientState(GL_VERTEX_ARRAY);

			glVertexPointer(3, GL_FLOAT, sizeof(ActorVertex), &_faceVBO[0].sx);

			glDrawElements(GL_TRIANGLES, (*face)->vertexIndices.size(), GL_UNSIGNED_INT, vertexIndices);

			glDisableClientState(GL_VERTEX_ARRAY);
		}

		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
	}
}

void OpenGLActorRenderer::clearVertices() {
	delete[] _faceVBO;
	_faceVBO = nullptr;

	for (FaceBufferMap::iterator it = _faceEBO.begin(); it != _faceEBO.end(); ++it) {
		delete[] it->_value;
	}

	_faceEBO.clear();
}

void OpenGLActorRenderer::uploadVertices() {
	_faceVBO = createModelVBO(_model);

	Common::Array<Face *> faces = _model->getFaces();
	for (Common::Array<Face *>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		_faceEBO[*face] = createFaceEBO(*face);
	}
}

ActorVertex *OpenGLActorRenderer::createModelVBO(const Model *model) {
	const Common::Array<VertNode *> &modelVertices = model->getVertices();

	auto vertices = new ActorVertex[modelVertices.size()];
	// Build a vertex array
	int i = 0;
	for (Common::Array<VertNode *>::const_iterator tri = modelVertices.begin(); tri != modelVertices.end(); ++tri, i++) {
		vertices[i].pos1 = Math::Vector3d((*tri)->_pos1.x(), (*tri)->_pos1.y(), (*tri)->_pos1.z());
		vertices[i].pos2 = Math::Vector3d((*tri)->_pos2.x(), (*tri)->_pos2.y(), (*tri)->_pos2.z());
		vertices[i].bone1 = (*tri)->_bone1;
		vertices[i].bone2 = (*tri)->_bone2;
		vertices[i].boneWeight = (*tri)->_boneWeight;
		vertices[i].normal = Math::Vector3d((*tri)->_normal.x(), (*tri)->_normal.y(), (*tri)->_normal.z());
		vertices[i].texS = -(*tri)->_texS;
		vertices[i].texT = (*tri)->_texT;
	}

	return vertices;
}

uint32 *OpenGLActorRenderer::createFaceEBO(const Face *face) {
	auto indices = new uint32[face->vertexIndices.size()];
	for (uint32 index = 0; index < face->vertexIndices.size(); index++) {
		indices[index] = face->vertexIndices[index];
	}

	return indices;
}

Math::Vector3d OpenGLActorRenderer::getShadowLightDirection(const LightEntryArray &lights,
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
	return worldToModelRot * sumDirection;
}

bool OpenGLActorRenderer::getPointLightContribution(LightEntry *light,
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

bool OpenGLActorRenderer::getDirectionalLightContribution(LightEntry *light, Math::Vector3d &direction) {
	float brightness = (light->color.x() + light->color.y() + light->color.z()) / 3.0f;

	if (brightness <= 0) {
		return false;
	}

	direction = light->direction;
	direction.normalize();
	direction *= brightness;

	return true;
}

bool OpenGLActorRenderer::getSpotLightContribution(LightEntry *light,
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

#endif // defined(USE_OPENGL_GAME)
