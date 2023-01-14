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

#include "graphics/tinygl/tinygl.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_mesh_tinygl.h"

namespace Tetraedge {

TeMeshTinyGL::TeMeshTinyGL() : _glMeshMode(TGL_POINTS), _gltexEnvMode(TGL_MODULATE) {
}

void TeMeshTinyGL::draw() {
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

	const Common::Array<TeVector3f32> &normals = (_updatedVerticies.empty() ? _normals : _updatedNormals);
	const Common::Array<TeVector3f32> &verticies = (_updatedVerticies.empty() ? _verticies : _updatedVerticies);

	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
	renderer->pushMatrix();
	renderer->loadCurrentMatrixToGL();
	tglEnableClientState(TGL_VERTEX_ARRAY);
	if (!normals.empty())
		tglEnableClientState(TGL_NORMAL_ARRAY);

	if (!_colors.empty())
		tglEnableClientState(TGL_COLOR_ARRAY);

	tglVertexPointer(3, TGL_FLOAT, 12, verticies.data());
	if (!normals.empty())
		tglNormalPointer(TGL_FLOAT, 12, normals.data());

	if (!_uvs.empty() && renderer->shadowMode() != TeRenderer::ShadowMode2)
		tglTexCoordPointer(2, TGL_FLOAT, 8, _uvs.data());

	if (!_colors.empty())
		tglColorPointer(4, TGL_UNSIGNED_BYTE, 4, _colors.data());

	// TODO: not supported in TGL
	//tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, _gltexEnvMode);
	if (renderer->scissorEnabled()) {
		tglEnable(TGL_SCISSOR_TEST);
		// TODO: Scissor not supported by TGL
		/*
		uint scissorx = renderer->scissorX();
		uint scissory = renderer->scissorY();
		uint scissorwidth = renderer->scissorWidth();
		uint scissorheight = renderer->scissorHeight();
		//tglScissor(scissorx, scissory, scissorwidth, scissorheight);
		*/
	}

	if (_faceCounts.empty()) {
		if (!_materials.empty())
			renderer->applyMaterial(_materials[0]);

		tglDrawElements(_glMeshMode, _indexes.size(), TGL_UNSIGNED_SHORT, _indexes.data());
		if (!_materials.empty()) {
			tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
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
				tglDrawElements(_glMeshMode, _faceCounts[i] * 3, TGL_UNSIGNED_SHORT, _indexes.data() + totalFaceCount * 3);
				tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
				renderer->disableTexture();
			}
			totalFaceCount += _faceCounts[i];
		}
	}

	if (!renderer->scissorEnabled())
		tglDisable(TGL_SCISSOR_TEST);

	// TODO: not supported in TGL
	//tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_MODULATE);
	tglDisableClientState(TGL_VERTEX_ARRAY);
	tglDisableClientState(TGL_NORMAL_ARRAY);
	tglDisableClientState(TGL_COLOR_ARRAY);

	//renderer->setCurrentColor(renderer->currentColor()); // pointless?

	if (_drawWires && !normals.empty()) {
		renderer->disableAllLights();
		error("TODO: Properly implement _drawWires case in TeMesh::draw");
		/*
		// TODO: Reimplement without glBegin/glEnd
		glBegin(TGL_LINES);
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

TeMesh::Mode TeMeshTinyGL::getMode() const {
	switch(_glMeshMode) {
	case TGL_POINTS:
		return MeshMode_Points;
	case TGL_LINES:
		return MeshMode_Lines;
	case TGL_LINE_LOOP:
		return MeshMode_LineLoop;
	case TGL_LINE_STRIP:
		return MeshMode_LineStrip;
	case TGL_TRIANGLES:
		return MeshMode_Triangles;
	case TGL_TRIANGLE_STRIP:
		return MeshMode_TriangleStrip;
	case TGL_TRIANGLE_FAN:
		return MeshMode_TriangleFan;
	default:
		return MeshMode_None;
	}
}

void TeMeshTinyGL::setMode(enum Mode mode) {
	switch(mode) {
	case MeshMode_Points:
		_glMeshMode = TGL_POINTS;
		break;
	case MeshMode_Lines:
		_glMeshMode = TGL_LINES;
		break;
	case MeshMode_LineLoop:
		_glMeshMode = TGL_LINE_LOOP;
		break;
	case MeshMode_LineStrip:
		_glMeshMode = TGL_LINE_STRIP;
		break;
	case MeshMode_Triangles:
		_glMeshMode = TGL_TRIANGLES;
		break;
	case MeshMode_TriangleStrip:
		_glMeshMode = TGL_TRIANGLE_STRIP;
		break;
	case MeshMode_TriangleFan:
		_glMeshMode = TGL_TRIANGLE_FAN;
		break;
	default:
		error("Invalid mesh mode %d", (int)mode);
	}
}

void TeMeshTinyGL::setglTexEnvBlend() {
	_gltexEnvMode = TGL_BLEND;
}

uint32 TeMeshTinyGL::getTexEnvMode() const {
	return _gltexEnvMode;
}


} // end namespace Tetraedge
