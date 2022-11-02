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
		clearVertices();
		uploadVertices();
		_modelIsDirty = false;
	}

	_gfx->set3DMode();
	if (!_gfx->computeLightsEnabled())
		_gfx->setupLights(lights);

	Math::Matrix4 model = getModelMatrix(position, direction);
	Math::Matrix4 view = StarkScene->getViewMatrix();
	Math::Matrix4 projection = StarkScene->getProjectionMatrix();

	Math::Matrix4 modelViewMatrix = view * model;
	modelViewMatrix.transpose(); // OpenGL expects matrices transposed
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelViewMatrix.getData());

	Math::Matrix4 projectionMatrix = projection;
	projectionMatrix.transpose(); // OpenGL expects matrices transposed
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projectionMatrix.getData());

	Math::Matrix4 normalMatrix;
	if (_gfx->computeLightsEnabled()) {
		projectionMatrix.transpose();
		modelViewMatrix.transpose();

		normalMatrix = modelViewMatrix;
		normalMatrix.invertAffineOrthonormal();
	}

	const Common::Array<Face> &faces = _model->getFaces();
	const Common::Array<Material> &materials = _model->getMaterials();

	if (!_gfx->computeLightsEnabled())
		glEnable(GL_COLOR_MATERIAL);
	for (Common::Array<Face>::const_iterator face = faces.begin(); face != faces.end(); ++face) {
		const Material &material = materials[face->materialId];
		Math::Vector3d color;
		const Gfx::Texture *tex = _texture->getTexture(material.texture);
		if (tex) {
			tex->bind();
			glEnable(GL_TEXTURE_2D);
		} else {
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}
		auto vertexIndices = _faceEBO[face];
		auto numVertexIndices = (face)->vertexIndices.size();
		if (!_gfx->computeLightsEnabled()) {
			if (material.doubleSided)
				glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
			else
				glColorMaterial(GL_FRONT, GL_DIFFUSE);
		}
		for (uint32 i = 0; i < numVertexIndices; i++) {
			uint32 index = vertexIndices[i];
			auto vertex = _faceVBO[index];
			if (tex) {
				if (_gfx->computeLightsEnabled())
					color = Math::Vector3d(1.0f, 1.0f, 1.0f);
				else
					glColor3f(1.0f, 1.0f, 1.0f);
				if (material.doubleSided) {
					vertex.texS = vertex.stexS;
					vertex.texT = 1.0f - vertex.stexT;
				} else {
					vertex.texS = 1.0f - vertex.stexS;
					vertex.texT = 1.0f - vertex.stexT;
				}
			} else {
				if (_gfx->computeLightsEnabled())
					color = Math::Vector3d(material.r, material.g, material.b);
				else
					glColor3f(material.r, material.g, material.b);
			}

			if (_gfx->computeLightsEnabled()) {
				Math::Vector4d modelEyePosition = modelViewMatrix * Math::Vector4d(vertex.x, vertex.y, vertex.z, 1.0);
				Math::Vector3d modelEyeNormal = normalMatrix.getRotation() *  Math::Vector3d(vertex.nx, vertex.ny, vertex.nz);
				modelEyeNormal.normalize();

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
			}
			_faceVBO[index] = vertex;
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		if (_gfx->computeLightsEnabled())
			glEnableClientState(GL_COLOR_ARRAY);
		if (tex)
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		glVertexPointer(3, GL_FLOAT, sizeof(PropVertex), &_faceVBO[0].x);
		if (tex)
			glTexCoordPointer(2, GL_FLOAT, sizeof(PropVertex), &_faceVBO[0].texS);
		glNormalPointer(GL_FLOAT, sizeof(PropVertex), &_faceVBO[0].nx);
		if (_gfx->computeLightsEnabled())
			glColorPointer(3, GL_FLOAT, sizeof(PropVertex), &_faceVBO[0].r);

		glDrawElements(GL_TRIANGLES, face->vertexIndices.size(), GL_UNSIGNED_INT, vertexIndices);

		glDisableClientState(GL_VERTEX_ARRAY);
		if (_gfx->computeLightsEnabled())
			glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
	}
	if (!_gfx->computeLightsEnabled())
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
		vertices[i].stexS = modelVertices[i].texturePosition.x();
		vertices[i].stexT = modelVertices[i].texturePosition.y();
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
