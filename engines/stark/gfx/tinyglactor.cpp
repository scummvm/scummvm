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

#include "engines/stark/gfx/tinyglactor.h"
#include "engines/stark/model/model.h"
#include "engines/stark/model/animhandler.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/settings.h"
#include "engines/stark/gfx/texture.h"

#include "math/vector2d.h"

namespace Stark {
namespace Gfx {

TinyGLActorRenderer::TinyGLActorRenderer(TinyGLDriver *gfx) :
		VisualActor(),
		_gfx(gfx),
		_faceVBO(nullptr) {
}

TinyGLActorRenderer::~TinyGLActorRenderer() {
	clearVertices();
}

void TinyGLActorRenderer::render(const Math::Vector3d &position, float direction, const LightEntryArray &lights) {
	if (_modelIsDirty) {
		clearVertices();
		uploadVertices();
		_modelIsDirty = false;
	}

	// TODO: Move updates outside of the rendering code
	_animHandler->animate(_time);
	_model->updateBoundingBox();

	bool drawShadow = false;
	if (_castsShadow &&
	    StarkScene->shouldRenderShadows() &&
	    StarkSettings->getBoolSetting(Settings::kShadow)) {
		drawShadow = true;
	}

	Math::Vector3d lightDirection;

	_gfx->set3DMode();

	Math::Matrix4 model = getModelMatrix(position, direction);
	Math::Matrix4 view = StarkScene->getViewMatrix();
	Math::Matrix4 projection = StarkScene->getProjectionMatrix();

	Math::Matrix4 modelViewMatrix = view * model;
	modelViewMatrix.transpose(); // TinyGL expects matrices transposed
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadMatrixf(modelViewMatrix.getData());

	Math::Matrix4 projectionMatrix = projection;
	projectionMatrix.transpose(); // TinyGL expects matrices transposed
	tglMatrixMode(TGL_PROJECTION);
	tglLoadMatrixf(projectionMatrix.getData());

	Math::Matrix4 normalMatrix;
	projectionMatrix.transpose();
	modelViewMatrix.transpose();

	normalMatrix = modelViewMatrix;
	normalMatrix.invertAffineOrthonormal();

	Math::Matrix4 mvp;
	if (drawShadow) {
		mvp = view * model;
		mvp.transpose();
		Math::Matrix4 modelInverse = model;
		modelInverse.inverse();
		lightDirection = getShadowLightDirection(lights, position, modelInverse.getRotation());
	}

	Common::Array<Face *> faces = _model->getFaces();
	Common::Array<Material *> mats = _model->getMaterials();
	const Common::Array<BoneNode *> &bones = _model->getBones();

	for (Common::Array<Face *>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		const Material *material = mats[(*face)->materialId];
		Math::Vector3d color;
		const Gfx::Texture *tex = resolveTexture(material);
		if (tex) {
			tex->bind();
			tglEnable(TGL_TEXTURE_2D);
		} else {
			tglBindTexture(TGL_TEXTURE_2D, 0);
			tglDisable(TGL_TEXTURE_2D);
		}
		auto vertexIndices = _faceEBO[*face];
		auto numVertexIndices = (*face)->vertexIndices.size();
		for (uint32 i = 0; i < numVertexIndices; i++) {
			if (tex) {
				color = Math::Vector3d(1.0f, 1.0f, 1.0f);
			} else {
				color = Math::Vector3d(material->r, material->g, material->b);
			}
			uint32 index = vertexIndices[i];
			auto vertex = _faceVBO[index];
			uint32 bone1 = vertex.bone1;
			uint32 bone2 = vertex.bone2;
			Math::Vector3d position1 = Math::Vector3d(vertex.pos1x, vertex.pos1y, vertex.pos1z);
			Math::Vector3d position2 = Math::Vector3d(vertex.pos2x, vertex.pos2y, vertex.pos2z);
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
			Math::Vector3d normal = Math::Vector3d(vertex.normalx, vertex.normaly, vertex.normalz);

			// Compute the vertex position in eye-space
			bone1Rotation.transform(position1);
			position1 += bone1Position;
			bone2Rotation.transform(position2);
			position2 += bone2Position;
			Math::Vector3d modelPosition = Math::Vector3d::interpolate(position2, position1, boneWeight);
			vertex.x = modelPosition.x();
			vertex.y = modelPosition.y();
			vertex.z = modelPosition.z();
			Math::Vector4d modelEyePosition;
			modelEyePosition = modelViewMatrix * Math::Vector4d(modelPosition.x(),
			                                                    modelPosition.y(),
			                                                    modelPosition.z(),
			                                                    1.0);
			// Compute the vertex normal in eye-space
			Math::Vector3d n1 = normal;
			bone1Rotation.transform(n1);
			Math::Vector3d n2 = normal;
			bone2Rotation.transform(n2);
			Math::Vector3d modelNormal = Math::Vector3d(Math::Vector3d::interpolate(n2, n1, boneWeight)).getNormalized();
			vertex.nx = modelNormal.x();
			vertex.ny = modelNormal.y();
			vertex.nz = modelNormal.z();
			Math::Vector3d modelEyeNormal;
			modelEyeNormal = normalMatrix.getRotation() * modelNormal;
			modelEyeNormal.normalize();

			if (drawShadow) {
				Math::Vector3d shadowPosition = modelPosition + lightDirection * (-modelPosition.y() / lightDirection.y());
				vertex.sx = shadowPosition.x();
				vertex.sy = 0.0f;
				vertex.sz = shadowPosition.z();
			}

			static const uint maxLights = 10;

			assert(lights.size() >= 1);
			assert(lights.size() <= maxLights);

			const LightEntry *ambient = lights[0];
			assert(ambient->type == LightEntry::kAmbient); // The first light must be the ambient light

			Math::Vector3d lightColor = ambient->color;

			for (uint li = 0; li < lights.size() - 1; li++) {
				const LightEntry *l = lights[li + 1];

				switch (l->type) {
					case LightEntry::kPoint: {
						Math::Vector3d vertexToLight = l->eyePosition.getXYZ() - modelEyePosition.getXYZ();

						float dist = vertexToLight.length();
						vertexToLight.normalize();
						float attn = CLIP((l->falloffFar - dist) / MAX(0.001f,  l->falloffFar - l->falloffNear), 0.0f, 1.0f);
						float incidence = MAX(0.0f, Math::Vector3d::dotProduct(modelEyeNormal, vertexToLight));
						lightColor += l->color * attn * incidence;
						break;
					}
					case LightEntry::kDirectional: {
						float incidence = MAX(0.0f, Math::Vector3d::dotProduct(modelEyeNormal, -l->eyeDirection));
						lightColor += (l->color * incidence);
						break;
					}
					case LightEntry::kSpot: {
						Math::Vector3d vertexToLight = l->eyePosition.getXYZ() - modelEyePosition.getXYZ();

						float dist = vertexToLight.length();
						float attn = CLIP((l->falloffFar - dist) / MAX(0.001f, l->falloffFar - l->falloffNear), 0.0f, 1.0f);

						vertexToLight.normalize();
						float incidence = MAX(0.0f, modelEyeNormal.dotProduct(vertexToLight));

						float cosAngle = MAX(0.0f, vertexToLight.dotProduct(-l->eyeDirection));
						float cone = CLIP((cosAngle - l->innerConeAngle.getCosine()) / MAX(0.001f, l->outerConeAngle.getCosine() - l->innerConeAngle.getCosine()), 0.0f, 1.0f);

						lightColor += l->color * attn * incidence * cone;
						break;
					}
					default:
						break;
				}
			}

			lightColor.x() = CLIP(lightColor.x(), 0.0f, 1.0f);
			lightColor.y() = CLIP(lightColor.y(), 0.0f, 1.0f);
			lightColor.z() = CLIP(lightColor.z(), 0.0f, 1.0f);
			color = color * lightColor;
			vertex.r = color.x();
			vertex.g = color.y();
			vertex.b = color.z();
			_faceVBO[index] = vertex;
		}

		tglEnableClientState(TGL_VERTEX_ARRAY);
		tglEnableClientState(TGL_COLOR_ARRAY);
		if (tex)
			tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
		tglEnableClientState(TGL_NORMAL_ARRAY);

		tglVertexPointer(3, TGL_FLOAT, sizeof(ActorVertex), &_faceVBO[0].x);
		if (tex)
			tglTexCoordPointer(2, TGL_FLOAT, sizeof(ActorVertex), &_faceVBO[0].texS);
		tglNormalPointer(TGL_FLOAT, sizeof(ActorVertex), &_faceVBO[0].nx);
		tglColorPointer(3, TGL_FLOAT, sizeof(ActorVertex), &_faceVBO[0].r);

		tglDrawElements(TGL_TRIANGLES, numVertexIndices, TGL_UNSIGNED_INT, vertexIndices);

		tglDisableClientState(TGL_VERTEX_ARRAY);
		tglDisableClientState(TGL_COLOR_ARRAY);
		tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
		tglDisableClientState(TGL_NORMAL_ARRAY);
	}

	if (drawShadow) {
		tglEnable(TGL_BLEND);
		tglEnable(TGL_STENCIL_TEST);
		tglDisable(TGL_TEXTURE_2D);

		tglColor4f(0.0f, 0.0f, 0.0f, 0.5f);

		for (Common::Array<Face *>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
			tglEnableClientState(TGL_VERTEX_ARRAY);

			tglVertexPointer(3, TGL_FLOAT, sizeof(ActorVertex), &_faceVBO[0].sx);

			tglDrawElements(TGL_TRIANGLES, (*face)->vertexIndices.size(), TGL_UNSIGNED_INT, _faceEBO[*face]);

			tglDisableClientState(TGL_VERTEX_ARRAY);
		}

		tglEnable(TGL_TEXTURE_2D);
		tglDisable(TGL_BLEND);
		tglDisable(TGL_STENCIL_TEST);
	}
}

void TinyGLActorRenderer::clearVertices() {
	delete[] _faceVBO;
	_faceVBO = nullptr;

	for (FaceBufferMap::iterator it = _faceEBO.begin(); it != _faceEBO.end(); ++it) {
		delete[] it->_value;
	}

	_faceEBO.clear();
}

void TinyGLActorRenderer::uploadVertices() {
	_faceVBO = createModelVBO(_model);

	Common::Array<Face *> faces = _model->getFaces();
	for (Common::Array<Face *>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		_faceEBO[*face] = createFaceEBO(*face);
	}
}

ActorVertex *TinyGLActorRenderer::createModelVBO(const Model *model) {
	const Common::Array<VertNode *> &modelVertices = model->getVertices();

	auto vertices = new ActorVertex[modelVertices.size()];
	// Build a vertex array
	int i = 0;
	for (Common::Array<VertNode *>::const_iterator tri = modelVertices.begin(); tri != modelVertices.end(); ++tri, i++) {
		vertices[i].pos1x = (*tri)->_pos1.x();
		vertices[i].pos1y = (*tri)->_pos1.y();
		vertices[i].pos1z = (*tri)->_pos1.z();
		vertices[i].pos2x = (*tri)->_pos2.x();
		vertices[i].pos2y = (*tri)->_pos2.y();
		vertices[i].pos2z = (*tri)->_pos2.z();
		vertices[i].bone1 = (*tri)->_bone1;
		vertices[i].bone2 = (*tri)->_bone2;
		vertices[i].boneWeight = (*tri)->_boneWeight;
		vertices[i].normalx = (*tri)->_normal.x();
		vertices[i].normaly = (*tri)->_normal.y();
		vertices[i].normalz = (*tri)->_normal.z();
		vertices[i].texS = -(*tri)->_texS;
		vertices[i].texT = (*tri)->_texT;
	}

	return vertices;
}

uint32 *TinyGLActorRenderer::createFaceEBO(const Face *face) {
	auto indices = new uint32[face->vertexIndices.size()];
	for (uint32 index = 0; index < face->vertexIndices.size(); index++) {
		indices[index] = face->vertexIndices[index];
	}

	return indices;
}

Math::Vector3d TinyGLActorRenderer::getShadowLightDirection(const LightEntryArray &lights,
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

bool TinyGLActorRenderer::getPointLightContribution(LightEntry *light,
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

bool TinyGLActorRenderer::getDirectionalLightContribution(LightEntry *light, Math::Vector3d &direction) {
	float brightness = (light->color.x() + light->color.y() + light->color.z()) / 3.0f;

	if (brightness <= 0) {
		return false;
	}

	direction = light->direction;
	direction.normalize();
	direction *= brightness;

	return true;
}

bool TinyGLActorRenderer::getSpotLightContribution(LightEntry *light, const Math::Vector3d &actorPosition,
                                                   Math::Vector3d &direction) {
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
