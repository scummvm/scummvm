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

#include "graphics/opengl/system_headers.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_mesh.h"
#include "tetraedge/te/te_material.h"

namespace Tetraedge {

TeMesh::TeMesh() : _matrixForced(false), _glMeshMode(GL_POINTS),
_hasAlpha(false), _gltexEnvMode(GL_MODULATE), _initialMaterialIndexCount(0),
_drawWires(false), _shouldDraw(true) {
}


void TeMesh::defaultMaterial(const TeIntrusivePtr<Te3DTexture> &texture) {
	TeMaterial::Mode mode = TeMaterial::MaterialMode1;
	if (texture && !texture->hasAlpha())
		mode = TeMaterial::MaterialMode0;
	_materials.resize(1);
	_materials[0] = TeMaterial(texture, mode);
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

void TeMesh::draw() {
	if (!worldVisible())
		return;

	TeRenderer *renderer = g_engine->getRenderer();
	renderer->pushMatrix();
	if (_matrixForced)
		renderer->multiplyMatrix(_forcedMatrix);
	else
		renderer->multiplyMatrix(worldTransformationMatrix());

	/*
	debug("Draw mesh %p (%s, %d verts %d norms %d indexes %d materials %d updated)", this, name().empty() ? "no name" : name().c_str(), _verticies.size(), _normals.size(), _indexes.size(), _materials.size(), _updatedVerticies.size());
	debug("   renderMatrix %s", renderer->currentMatrix().toString().c_str());
	if (!_materials.empty())
		debug("   material   %s", _materials[0].dump().c_str());
	debug("   position   %s", position().dump().c_str());
	debug("   worldPos   %s", worldPosition().dump().c_str());
	debug("   scale      %s", scale().dump().c_str());
	debug("   worldScale %s", worldScale().dump().c_str());
	debug("   rotation   %s", rotation().dump().c_str());
	debug("   worldRot   %s", worldRotation().dump().c_str());
	*/

	Common::Array<TeVector3f32> &normals = (_updatedVerticies.empty() ? _normals : _updatedNormals);
	Common::Array<TeVector3f32> &verticies = (_updatedVerticies.empty() ? _verticies : _updatedVerticies);
	if (renderer->shadowMode() != TeRenderer::ShadowMode1) {
		if (_faceCounts.empty()) {
			if (hasAlpha(0) && _shouldDraw) {
				renderer->addTransparentMesh(*this, 0, 0, 0);
				renderer->popMatrix();
				return;
			}
		} else {
			assert(_faceCounts.size() == _materials.size());
			int totalFaceCount = 0;
			for (uint i = 0; i < _faceCounts.size(); i++) {
				if (!_faceCounts[i])
					continue;
				if (hasAlpha(i)) {
					renderer->addTransparentMesh(*this, totalFaceCount, _faceCounts[i], i);
				}
				totalFaceCount += _faceCounts[i];
			}
		}
	}

	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
	renderer->pushMatrix();
	renderer->loadCurrentMatrixToGL();
	glEnableClientState(GL_VERTEX_ARRAY);
	if (!normals.empty())
		glEnableClientState(GL_NORMAL_ARRAY);

	if (!_colors.empty())
		glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 12, verticies.data());
	if (!normals.empty())
		glNormalPointer(GL_FLOAT, 12, normals.data());

	if (!_uvs.empty() && renderer->shadowMode() != TeRenderer::ShadowMode2)
		glTexCoordPointer(2, GL_FLOAT, 8, _uvs.data());

	if (!_colors.empty())
		glColorPointer(4, GL_UNSIGNED_BYTE, 4, _colors.data());

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, _gltexEnvMode);
	if (renderer->scissorEnabled()) {
		glEnable(GL_SCISSOR_TEST);
		uint scissorx = renderer->scissorX();
		uint scissory = renderer->scissorY();
		uint scissorwidth = renderer->scissorWidth();
		uint scissorheight = renderer->scissorHeight();
		glScissor(scissorx, scissory, scissorwidth, scissorheight);
	}

	if (_faceCounts.empty()) {
		if (!_materials.empty())
			_materials[0].apply();

		glDrawElements(_glMeshMode, _indexes.size(), GL_UNSIGNED_SHORT, _indexes.data());
		if (!_materials.empty()) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			renderer->disableTexture();
		}
	} else {
		int totalFaceCount = 0;
		assert(_faceCounts.size() == _materials.size());
		for (uint i = 0; i < _materials.size(); i++) {
			if (!_faceCounts[i])
				continue;
			if (!hasAlpha(i) || renderer->shadowMode() == TeRenderer::ShadowMode1 || !_shouldDraw) {
				_materials[i].apply();
				glDrawElements(_glMeshMode, _faceCounts[i] * 3, GL_UNSIGNED_SHORT, _indexes.data() + totalFaceCount * 3);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				renderer->disableTexture();
			}
			totalFaceCount += _faceCounts[i];
		}
	}

	if (!renderer->scissorEnabled())
		glDisable(GL_SCISSOR_TEST);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	//renderer->setCurrentColor(renderer->currentColor()); // pointless?

	if (_drawWires && !normals.empty()) {
		TeLight::disableAll();
		glBegin(GL_LINES);
		renderer->setCurrentColor(TeColor(255, 255, 255, 255));
		for (uint i = 0; i < verticies.size(); i++) {
			glVertex3f(verticies[i].x(), verticies[i].y(), verticies[i].z());
			glVertex3f(verticies[i].x() + normals[i].x(),
					verticies[i].y() + normals[i].y(),
					verticies[i].z() + normals[i].z());
		}
		glEnd();
	}

	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
	renderer->popMatrix();
	renderer->popMatrix();
}

TeMesh::Mode TeMesh::getMode() const {
	// Do the reverse translation of setConf... why? I dunno.. the game does that..
	switch(_glMeshMode) {
	case GL_POINTS:
		return MeshMode_Points;
	case GL_LINES:
		return MeshMode_Lines;
	case GL_LINE_LOOP:
		return MeshMode_LineLoop;
	case GL_LINE_STRIP:
		return MeshMode_LineStrip;
	case GL_TRIANGLES:
		return MeshMode_Triangles;
	case GL_TRIANGLE_STRIP:
		return MeshMode_TriangleStrip;
	case GL_TRIANGLE_FAN:
		return MeshMode_TriangleFan;
	default:
		return MeshMode_None;
	}
}

bool TeMesh::hasAlpha(uint idx) {
	// Note: I don't understand this logic, but it's what the game does..
	bool hasGlobalAlpha = _hasAlpha && !_colors.empty();

	bool retval = hasGlobalAlpha;
	if (idx < _materials.size()) {
		const TeMaterial &material = _materials[idx];
		if (material._enableSomethingDefault0) {
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

void TeMesh::resizeUpdatedTables(unsigned long newSize) {
	_updatedVerticies.resize(newSize);
	_updatedNormals.resize(newSize);
}

void TeMesh::setglTexEnvBlend() {
	_gltexEnvMode = GL_BLEND;
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

void TeMesh::setConf(unsigned long vertexCount, unsigned long indexCount, enum Mode mode, uint materialCount, uint materialIndexCount) {
	destroy();
	_initialMaterialIndexCount = materialIndexCount;
	_verticies.resize(vertexCount);
	_indexes.resize(indexCount);
	_materials.resize(materialCount);
	_matricies.resize(vertexCount);
	switch(mode) {
	case MeshMode_Points:
		_glMeshMode = GL_POINTS;
		break;
	case MeshMode_Lines:
		_glMeshMode = GL_LINES;
		break;
	case MeshMode_LineLoop:
		_glMeshMode = GL_LINE_LOOP;
		break;
	case MeshMode_LineStrip:
		_glMeshMode = GL_LINE_STRIP;
		break;
	case MeshMode_Triangles:
		_glMeshMode = GL_TRIANGLES;
		break;
	case MeshMode_TriangleStrip:
		_glMeshMode = GL_TRIANGLE_STRIP;
		break;
	case MeshMode_TriangleFan:
		_glMeshMode = GL_TRIANGLE_FAN;
		break;
	default:
		error("Setting invalid mesh mode.");
	}
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
	dest._enableSomethingDefault0 = src._enableSomethingDefault0;
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

	const Common::Array<TeVector3f32> &animverts = vertexanim->getVertices();
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

/*
TeMesh &TeMesh::operator=(const TeMesh &other) {
	copy(other);
	return *this;
}

void TeMesh::copy(const TeMesh &other) {
	destroy();
	_drawWires = false;
	_hasAlpha = other._hasAlpha;
	_shouldDraw = other._shouldDraw;
	_verticies = other._verticies;
	_normals = other._normals;
	_uvs = other._uvs;
	_colors = other._colors;

	_glMeshMode = other._glMeshMode;
	_gltexEnvMode = other._gltexEnvMode;
	_matrixForced = other._matrixForced;
	_forceMatrix = other._forceMatrix;
}*/

} // end namespace Tetraedge
