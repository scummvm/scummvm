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
#include "tetraedge/te/te_mesh_opengl.h"
#include "tetraedge/te/te_material.h"

namespace Tetraedge {

TeMeshOpenGL::TeMeshOpenGL() : _glMeshMode(GL_POINTS), _gltexEnvMode(GL_MODULATE) {
}

void TeMeshOpenGL::draw() {
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

	const Common::Array<TeVector3f32> &normals = (_updatedVerticies.empty() ? _normals : _updatedNormals);
	const Common::Array<TeVector3f32> &verticies = (_updatedVerticies.empty() ? _verticies : _updatedVerticies);
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
			renderer->applyMaterial(_materials[0]);

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
				renderer->applyMaterial(_materials[i]);
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
		renderer->disableAllLights();
		error("TODO: Properly implement _drawWires case in TeMesh::draw");
		/*
		// TODO: Reimplement without glBegin/glEnd
		glBegin(GL_LINES);
		renderer->setCurrentColor(TeColor(255, 255, 255, 255));
		for (uint i = 0; i < verticies.size(); i++) {
			glVertex3f(verticies[i].x(), verticies[i].y(), verticies[i].z());
			glVertex3f(verticies[i].x() + normals[i].x(),
					verticies[i].y() + normals[i].y(),
					verticies[i].z() + normals[i].z());
		}
		glEnd();
		*/
	}

	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
	renderer->popMatrix();
	renderer->popMatrix();
}

TeMesh::Mode TeMeshOpenGL::getMode() const {
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

void TeMeshOpenGL::setMode(enum Mode mode) {
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
		error("Invalid mesh mode %d", (int)mode);
	}
}

void TeMeshOpenGL::setglTexEnvBlend() {
	_gltexEnvMode = GL_BLEND;
}

uint32 TeMeshOpenGL::getTexEnvMode() const {
	return _gltexEnvMode;
}

void TeMeshOpenGL::setConf(unsigned long vertexCount, unsigned long indexCount, enum Mode mode, uint materialCount, uint materialIndexCount) {
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

} // end namespace Tetraedge
