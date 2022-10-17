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
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->pushMatrix();
	if (_matrixForced)
		renderer->multiplyMatrix(_forceMatrix);
	else
		renderer->multiplyMatrix(transformationMatrix());

	Common::Array<TeVector3f32> &normals = (_updatedVerticies.empty() ? _normals : _updatedNormals);
	Common::Array<TeVector3f32> &verticies = (_updatedVerticies.empty() ? _verticies : _updatedVerticies);
	if (renderer->shadowMode() != TeRenderer::ShadowMode1) {
		if (_faceCounts.empty()) {
			if (hasAlpha(0) && _shouldDraw) {
				renderer->addTransparentMesh(*this, 0, 0, 0);
				renderer->popMatrix();
				return;
			}
		} else if (!_materials.empty()) {
			for (unsigned int i = 0; i < _faceCounts.size(); i++) {
				int totalFaceCount = 0;
				if (_faceCounts[i]) {
					if (hasAlpha(i)) {
						renderer->addTransparentMesh(*this, totalFaceCount, _faceCounts[i], i);
					}
					totalFaceCount += _faceCounts[i];
				}
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
		int totalfacecount = 0;
		for (unsigned int i = 0; i < _materials.size(); i++) {
			if (_faceCounts[i]) {
			if (!hasAlpha(i) || renderer->shadowMode() == TeRenderer::ShadowMode1 || !_shouldDraw) {
				_materials[i].apply();
				glDrawElements(_glMeshMode, _faceCounts[i] * 3, GL_UNSIGNED_SHORT, _indexes.data() + totalfacecount * 3);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				renderer->disableTexture();
			}
			totalfacecount += _faceCounts[i];
		  }
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
		if (!_verticies.empty()) {
			error("TODO: Implement wire drawing here in TeMesh::draw..");
			/*
			offset1 = 1;
			offset2 = 2;
			do {
			  i = (ulong)offset2;
			  uVar5 = (ulong)(offset2 - 1);
			  totalfacecount = (ulong)(offset2 - 2);
			  glVertex3f
						((&verticiesbuf->f1)[totalfacecount],(&verticiesbuf->f1)[uVar5],
						 (&verticiesbuf->f1)[i]);
			  glVertex3f
						((&verticiesbuf->f1)[totalfacecount] + (&normalsbuf->f1)[totalfacecount],
						 (&verticiesbuf->f1)[uVar5] + (&normalsbuf->f1)[uVar5],
						 (&verticiesbuf->f1)[i] + (&normalsbuf->f1)[i]);
			  offset2 = offset2 + 3;
			  i = (ulong)offset1;
			  offset1 = offset1 + 1;
			} while (i < (this->verticiesArray).len);
			 */
		}
		glEnd();
	}

	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
	renderer->popMatrix();
	renderer->popMatrix();
}

TeMesh::Mode TeMesh::getMode() const {
	// Do the reverse translation of setMode... why? I dunno.. the game does that..
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
	// FIXME: this logic is a bit sketchy.  Check it again.
	if (_hasAlpha && !_colors.empty())
		return true;

	bool retval = false;
	if (idx < _materials.size()) {
		const TeMaterial &material = _materials[idx];
		if (material._enableSomethingDefault0)
			retval = true;
		//if (material._mode == TeMaterial::MaterialMode1)
		//	return false;
		if (material._ambientColor.a() == 255)
			retval = (material._diffuseColor.a() != 255);
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

void TeMesh::setColor(const TeColor &col) {
	Te3DObject2::setColor(col);

	if (!_verticies.empty()) {
		const TeColor colnow = Te3DObject2::color();
		_colors.resize(_verticies.size());
		if (colnow.a() != 255)
			_hasAlpha = true;

		for (unsigned int i = 0; i < _verticies.size(); i++) {
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

void TeMesh::setConf(unsigned long vertexCount, unsigned long indexCount, enum Mode mode, unsigned int materialCount, unsigned int materialIndexCount) {
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

void TeMesh::setIndex(unsigned int idx, unsigned int val) {
	_indexes[idx] = val;
}

void TeMesh::setNormal(unsigned int idx, const TeVector3f32 &val) {
	_normals.resize(_verticies.size());
	_normals[idx] = val;
}

void TeMesh::setTextureUV(unsigned int idx, const TeVector2f32 &val) {
	_uvs.resize(_verticies.size());
	_uvs[idx] = val;
}

void TeMesh::setVertex(unsigned int idx, const TeVector3f32 &val) {
	_verticies[idx] = val;
}

TeVector3f32 TeMesh::vertex(uint idx) const {
	if (!_updatedVerticies.empty())
		return _updatedVerticies[idx];
	else
		return _verticies[idx];
}

void TeMesh::attachMaterial(uint idx, const TeMaterial &material) {
	TeMaterial &mat = _materials[idx];
	mat._texture = material._texture;
	mat._enableLights = material._enableLights;
	mat._enableSomethingDefault0 = material._enableSomethingDefault0;
	mat._emissionColor = material._emissionColor;
	mat._diffuseColor = material._diffuseColor;
	mat._mode = material._mode;
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
	for (unsigned int i = 0; i < _verticies.size(); i++) {
		_updatedVerticies[i] = animverts[i];
	}
	for (unsigned int i = 0; i < _normals.size(); i++) {
		_updatedNormals[i] = _normals[i];
	}
}

void TeMesh::updateTo(const Common::Array<TeMatrix4x4> *matricies1, const Common::Array<TeMatrix4x4> *matricies2,
				Common::Array<TeVector3f32> &verts, Common::Array<TeVector3f32> &normals) {
	static const TeMatrix4x4 emptyMatrix;
	for (unsigned int i = 0; i < _verticies.size(); i++) {
		unsigned long m = _matricies[i];
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
