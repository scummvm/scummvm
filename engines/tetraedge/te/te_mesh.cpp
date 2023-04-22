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

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_mesh.h"
#include "tetraedge/te/te_mesh_opengl.h"
#include "tetraedge/te/te_mesh_tinygl.h"
#include "tetraedge/te/te_material.h"

namespace Tetraedge {

TeMesh::TeMesh() : _matrixForced(false), _hasAlpha(false), _initialMaterialIndexCount(0),
_drawWires(false), _shouldDraw(true) {
}


void TeMesh::defaultMaterial(const TeIntrusivePtr<Te3DTexture> &texture) {
	TeMaterial::Mode mode = TeMaterial::MaterialMode1;
	if (texture && !texture->hasAlpha())
		mode = TeMaterial::MaterialMode0;
	_materials.resize(1);
	_materials[0] = TeMaterial(texture, mode);
}

TeMaterial *TeMesh::material(uint index) {
	assert(!_materials.empty());
	if (index < _materials.size()) {
		return &_materials[index];
	} else {
		return &_materials[0];
	}
}

const TeMaterial *TeMesh::material(uint index) const {
	assert(!_materials.empty());
	if (index < _materials.size()) {
		return &_materials[index];
	} else {
		return &_materials[0];
	}
}

void TeMesh::destroy() {
	_hasAlpha = false;
	_updatedVerticies.clear();
	_updatedNormals.clear();
	_verticies.clear();
	_normals.clear();
	_uvs.clear();
	_colors.clear();
	_indexes.clear();
	_materialIndexes.clear();
	_faceCounts.clear();
	_matricies.clear();
}

bool TeMesh::hasAlpha(uint idx) {
	// Note: I don't understand this logic, but it's what the game does..
	bool hasGlobalAlpha = _hasAlpha && !_colors.empty();

	bool retval = hasGlobalAlpha;
	if (idx < _materials.size()) {
		const TeMaterial &material = _materials[idx];
		if (material._isShadowTexture) {
			retval = false;
		} else {
			retval = true;
			if (!hasGlobalAlpha && material._mode != TeMaterial::MaterialMode1 && material._ambientColor.a() == 255)
				retval = (material._diffuseColor.a() != 255);
		}
	}
	return retval;
}

TeVector3f32 TeMesh::normal(uint idx) const {
	if (!_updatedNormals.empty())
		return _updatedNormals[idx];
	else
		return _normals[idx];
}

void TeMesh::resizeUpdatedTables(uint newSize) {
	_updatedVerticies.resize(newSize);
	_updatedNormals.resize(newSize);
}

void TeMesh::setColor(const TeColor &col) {
	Te3DObject2::setColor(col);

	if (!_verticies.empty()) {
		const TeColor colnow = Te3DObject2::color();
		_colors.resize(_verticies.size());
		if (colnow.a() != 255)
			_hasAlpha = true;

		for (uint i = 0; i < _verticies.size(); i++) {
			_colors[i] = colnow;
		}
	}
}

void TeMesh::setColor(uint idx, const TeColor &col) {
	if (col.a() != 255) {
		_hasAlpha = true;
	}
	_colors.resize(_verticies.size());
	_colors[idx] = col;
}

void TeMesh::setConf(uint vertexCount, uint indexCount, enum Mode mode, uint materialCount, uint materialIndexCount) {
	destroy();
	_initialMaterialIndexCount = materialIndexCount;
	_verticies.resize(vertexCount);
	_indexes.resize(indexCount);
	_materials.resize(materialCount);
	_matricies.resize(vertexCount);
	setMode(mode);
}

void TeMesh::setIndex(uint idx, uint val) {
	_indexes[idx] = val;
}

void TeMesh::setNormal(uint idx, const TeVector3f32 &val) {
	_normals.resize(_verticies.size());
	_normals[idx] = val;
}

void TeMesh::setTextureUV(uint idx, const TeVector2f32 &val) {
	_uvs.resize(_verticies.size());
	_uvs[idx] = val;
}

void TeMesh::setVertex(uint idx, const TeVector3f32 &val) {
	_verticies[idx] = val;
}

TeVector3f32 TeMesh::vertex(uint idx) const {
	if (!_updatedVerticies.empty())
		return _updatedVerticies[idx];
	else
		return _verticies[idx];
}

void TeMesh::attachMaterial(uint idx, const TeMaterial &src) {
	TeMaterial &dest = _materials[idx];
	dest._texture = src._texture;
	dest._enableLights = src._enableLights;
	dest._isShadowTexture = src._isShadowTexture;
	dest._emissionColor = src._emissionColor;
	dest._shininess = src._shininess;
	dest._diffuseColor = src._diffuseColor;
	dest._specularColor = src._specularColor;
	dest._mode = src._mode;
	dest._ambientColor = src._ambientColor;
}

void TeMesh::facesPerMaterial(uint idx, unsigned short value) {
	_faceCounts.resize(_materials.size());
	_faceCounts[idx] = value;
}

void TeMesh::matrixIndex(uint num, unsigned short val) {
	_matricies[num] = val;
}

void TeMesh::update(const Common::Array<TeMatrix4x4> *matricies1, const Common::Array<TeMatrix4x4> *matricies2) {
	if (visible()) {
		if (matricies1) {
			_updatedVerticies.resize(_verticies.size());
			_updatedNormals.resize(_normals.size());
			updateTo(matricies1, matricies2, _updatedVerticies, _updatedNormals);
		} else {
			_updatedVerticies.clear();
			_updatedNormals.clear();
		}
	}
}

void TeMesh::update(TeIntrusivePtr<TeModelVertexAnimation> vertexanim) {
	_updatedVerticies.resize(_verticies.size());
	_updatedNormals.resize(_normals.size());

	const Common::Array<TeVector3f32> animverts = vertexanim->getVertices();
	assert(animverts.size() >= _verticies.size());
	for (uint i = 0; i < _verticies.size(); i++) {
		_updatedVerticies[i] = animverts[i];
	}
	for (uint i = 0; i < _normals.size(); i++) {
		_updatedNormals[i] = _normals[i];
	}
}

void TeMesh::updateTo(const Common::Array<TeMatrix4x4> *matricies1, const Common::Array<TeMatrix4x4> *matricies2,
				Common::Array<TeVector3f32> &verts, Common::Array<TeVector3f32> &normals) {
	static const TeMatrix4x4 emptyMatrix;
	for (uint i = 0; i < _verticies.size(); i++) {
		uint m = _matricies[i];
		const TeMatrix4x4 *mat;
		if (m < matricies1->size()) {
			mat = &((*matricies1)[m]);
		} else {
			m -= matricies1->size();
			if (m < matricies2->size())
				mat = &((*matricies2)[m]);
			else
				mat = &emptyMatrix;
		}
		verts[i] = mat->mult4x3(_verticies[i]);
		normals[i] = mat->mult3x3(_normals[i]);
	}
}

/*static*/
TeMesh *TeMesh::makeInstance() {
	Graphics::RendererType r = g_engine->preferredRendererType();

#if defined(USE_OPENGL_GAME)
	if (r == Graphics::kRendererTypeOpenGL)
		return new TeMeshOpenGL();
#endif

#if defined(USE_TINYGL)
	if (r == Graphics::kRendererTypeTinyGL)
		return new TeMeshTinyGL();
#endif
	error("Couldn't create TeMesh for selected renderer");

}

} // end namespace Tetraedge
