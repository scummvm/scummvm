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

#include "common/textconsole.h"
#include "common/debug.h"

#include "graphics/renderer.h"
#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_renderer_opengl.h"
#include "tetraedge/te/te_renderer_tinygl.h"
#include "tetraedge/te/te_light.h"

namespace Tetraedge {

TeRenderer::TeRenderer() : _textureEnabled(false), _shadowMode(ShadowModeNone), _matrixMode(MM_GL_PROJECTION),
_numTransparentMeshes(0), _pendingTransparentMeshProperties(0), _currentCamera(nullptr), _scissorEnabled(false),
_scissorX(0), _scissorY(0), _scissorWidth(0), _scissorHeight(0) {
}

void TeRenderer::addTransparentMesh(const TeMesh &mesh, uint i1, uint tricount, uint materialno) {
	const float orthNearVal = _currentCamera->orthoNearPlane();
	const TeMesh::Mode meshMode = mesh.getMode();
	if (!tricount) {
		if (meshMode == TeMesh::MeshMode_TriangleStrip) {
			tricount = mesh.numVerticies() - 2;
		} else if (meshMode == TeMesh::MeshMode_Triangles) {
			tricount = mesh.numIndexes() / 3;
		}
		if (!tricount)
			return;
	}

	uint vertcount = (_numTransparentMeshes + tricount) * 3;
	_transparentMeshVertexes.resize(vertcount);
	_transparentMeshNormals.resize(vertcount);
	_transparentMeshCoords.resize(vertcount);
	_transparentMeshColors.resize(vertcount);
	_transparentMeshVertexNums.resize(vertcount);

	uint newPropsSize = _pendingTransparentMeshProperties + (mesh.shouldDrawMaybe() ? tricount : 1);
	// Reserve blocks of 64 to avoid reallocating too much.
	uint newPropsReserve = (newPropsSize / 64 + 1) * 64;
	assert(newPropsReserve >= newPropsSize);
	_transparentMeshProps.reserve(newPropsReserve);
	_transparentMeshProps.resize(newPropsSize);
	if (meshMode == TeMesh::MeshMode_Triangles) {
		for (uint i = 0; i < tricount; i++) {
			const uint meshNo0 = (i1 + i) * 3;
			const uint propNo = (_numTransparentMeshes + i) * 3;

			_transparentMeshVertexes[propNo] = mesh.vertex(mesh.index(meshNo0));
			_transparentMeshVertexes[propNo + 1] = mesh.vertex(mesh.index(meshNo0 + 1));
			_transparentMeshVertexes[propNo + 2] = mesh.vertex(mesh.index(meshNo0 + 2));

			_transparentMeshNormals[propNo] = mesh.normal(mesh.index(meshNo0));
			_transparentMeshNormals[propNo + 1] = mesh.normal(mesh.index(meshNo0 + 1));
			_transparentMeshNormals[propNo + 2] = mesh.normal(mesh.index(meshNo0 + 2));

			if (mesh.hasUvs()) {
				_transparentMeshCoords[propNo] = mesh.textureUV(mesh.index(meshNo0));
				_transparentMeshCoords[propNo + 1] = mesh.textureUV(mesh.index(meshNo0 + 1));
				_transparentMeshCoords[propNo + 2] = mesh.textureUV(mesh.index(meshNo0 + 2));
			}

			if (!mesh.hasColor()) {
				_transparentMeshColors[propNo] = mesh.material(materialno)->_diffuseColor;
				_transparentMeshColors[propNo + 1] = mesh.material(materialno)->_diffuseColor;
				_transparentMeshColors[propNo + 2] = mesh.material(materialno)->_diffuseColor;
			} else {
				_transparentMeshColors[propNo] = mesh.color(mesh.index(meshNo0));
				_transparentMeshColors[propNo + 1] = mesh.color(mesh.index(meshNo0 + 1));
				_transparentMeshColors[propNo + 2] = mesh.color(mesh.index(meshNo0 + 2));
			}
		}
	} else if (meshMode == TeMesh::MeshMode_TriangleStrip && tricount > 0) {
		for (uint i = 0; i < tricount; i++) {
			const uint meshNo0 = (i1 + i);  // TODO: This appears to be the only difference between this and the above?
			const uint propNo = (_numTransparentMeshes + i) * 3;

			_transparentMeshVertexes[propNo] = mesh.vertex(mesh.index(meshNo0));
			_transparentMeshVertexes[propNo + 1] = mesh.vertex(mesh.index(meshNo0 + 1));
			_transparentMeshVertexes[propNo + 2] = mesh.vertex(mesh.index(meshNo0 + 2));

			_transparentMeshNormals[propNo] = mesh.normal(mesh.index(meshNo0));
			_transparentMeshNormals[propNo + 1] = mesh.normal(mesh.index(meshNo0 + 1));
			_transparentMeshNormals[propNo + 2] = mesh.normal(mesh.index(meshNo0 + 2));

			if (mesh.hasUvs()) {
				_transparentMeshCoords[propNo] = mesh.textureUV(mesh.index(meshNo0));;
				_transparentMeshCoords[propNo + 1] = mesh.textureUV(mesh.index(meshNo0 + 1));
				_transparentMeshCoords[propNo + 2] = mesh.textureUV(mesh.index(meshNo0 + 2));
			}

			if (!mesh.hasColor()) {
				_transparentMeshColors[propNo] =  mesh.material(materialno)->_diffuseColor;
				_transparentMeshColors[propNo + 1] =  mesh.material(materialno)->_diffuseColor;
				_transparentMeshColors[propNo + 2] =  mesh.material(materialno)->_diffuseColor;
			} else {
				_transparentMeshColors[propNo] = mesh.color(mesh.index(meshNo0));
				_transparentMeshColors[propNo + 1] = mesh.color(mesh.index(meshNo0 + 1));
				_transparentMeshColors[propNo + 2] = mesh.color(mesh.index(meshNo0 + 2));
			}
		}
	}

	if (!mesh.shouldDrawMaybe()) {
		// TODO: better variable names.
		const TeMatrix4x4 &currentMatrix = _matriciesStacks[MM_GL_MODELVIEW].currentMatrix();
		const TeVector3f32 v1trans = currentMatrix.mult4x3(_transparentMeshVertexes[_numTransparentMeshes * 3]);
		const TeVector3f32 v2trans = currentMatrix.mult4x3(_transparentMeshVertexes[_numTransparentMeshes * 3 + 1]);
		const TeVector3f32 v3trans = currentMatrix.mult4x3(_transparentMeshVertexes[_numTransparentMeshes * 3 + 2]);
		TeVector3f32 midpoint = (v1trans + v2trans + v3trans) / 3.0;

		midpoint.z() -= orthNearVal;
		float zOrder;
		if (_currentCamera->projMatrixType() < 4) {
			zOrder = -midpoint.squaredLength();
		} else if (_currentCamera->projMatrixType() == 4) {
			zOrder = midpoint.z() * midpoint.z();
		} else {
			zOrder = midpoint.squaredLength();
		}

		TransparentMeshProperties &destProperties = _transparentMeshProps[_pendingTransparentMeshProperties];

		destProperties._vertexCount = tricount * 3;
		destProperties._camera = _currentCamera;

		destProperties._material = *mesh.material(materialno);
		destProperties._matrix = currentMatrix;
		destProperties._glTexEnvMode = mesh.getTexEnvMode();
		destProperties._sourceTransparentMesh = _numTransparentMeshes * 3;
		destProperties._hasColor = mesh.hasColor();
		destProperties._zOrder = zOrder;
		destProperties._scissorEnabled = _scissorEnabled;
		destProperties._scissorX = _scissorX;
		destProperties._scissorY = _scissorY;
		destProperties._scissorWidth = _scissorWidth;
		destProperties._scissorHeight = _scissorHeight;
		destProperties._shouldDraw = false;
	} else {
		for (uint i = 0; i < tricount; i++) {
			const TeMatrix4x4 &currentMatrix = _matriciesStacks[MM_GL_MODELVIEW].currentMatrix();

			const int meshPropNo = (_numTransparentMeshes + i) * 3;
			_transparentMeshVertexes[meshPropNo] = currentMatrix.mult4x3(_transparentMeshVertexes[meshPropNo]);
			_transparentMeshVertexes[meshPropNo + 1] = currentMatrix.mult4x3(_transparentMeshVertexes[meshPropNo + 1]);
			_transparentMeshVertexes[meshPropNo + 2] = currentMatrix.mult4x3(_transparentMeshVertexes[meshPropNo + 2]);

			_transparentMeshNormals[meshPropNo] = currentMatrix.mult3x3(_transparentMeshNormals[meshPropNo]);
			_transparentMeshNormals[meshPropNo + 1] = currentMatrix.mult3x3(_transparentMeshNormals[meshPropNo + 1]);
			_transparentMeshNormals[meshPropNo + 2] = currentMatrix.mult3x3(_transparentMeshNormals[meshPropNo + 2]);

			TeVector3f32 midpoint = (_transparentMeshVertexes[meshPropNo] + _transparentMeshVertexes[meshPropNo + 1] + _transparentMeshVertexes[meshPropNo + 2]) / 3.0;
			midpoint.z() -= orthNearVal;

			float zOrder;
			if (_currentCamera->projMatrixType() < 4) {
				zOrder = -midpoint.squaredLength();
			} else if (_currentCamera->projMatrixType() == 4) {
				zOrder = midpoint.z() * midpoint.z();
			} else {
				zOrder = midpoint.squaredLength();
			}

			TransparentMeshProperties &destProperties = _transparentMeshProps[_pendingTransparentMeshProperties + i];
			destProperties._vertexCount = 3;
			destProperties._camera = _currentCamera;

			destProperties._material = *mesh.material(materialno);
			destProperties._glTexEnvMode = mesh.getTexEnvMode();
			destProperties._sourceTransparentMesh = meshPropNo;
			destProperties._hasColor = mesh.hasColor();
			destProperties._zOrder = zOrder;
			destProperties._scissorEnabled = _scissorEnabled;
			destProperties._scissorX = _scissorX;
			destProperties._scissorY = _scissorY;
			destProperties._scissorWidth = _scissorWidth;
			destProperties._scissorHeight = _scissorHeight;
			destProperties._shouldDraw = true;
		}
	}
	_numTransparentMeshes += tricount;
	_pendingTransparentMeshProperties = _transparentMeshProps.size();
}

void TeRenderer::create() {
	_textureEnabled = false;
	_currentCamera = nullptr;
	_shadowMode = ShadowModeNone;
	_numTransparentMeshes = 0;
	_pendingTransparentMeshProperties = 0;
}

TeMatrix4x4 TeRenderer::currentMatrix() {
	return _matriciesStacks[_matrixMode].currentMatrix();
}

void TeRenderer::loadIdentityMatrix() {
	_matriciesStacks[_matrixMode].loadIdentity();
}

void TeRenderer::loadMatrix(const TeMatrix4x4 &matrix) {
	_matriciesStacks[_matrixMode].loadMatrix(matrix);
}

void TeRenderer::loadCurrentMatrixToGL() {
	const TeMatrix4x4 current = currentMatrix();
	loadMatrixToGL(current);
}

void TeRenderer::multiplyMatrix(const TeMatrix4x4 &matrix) {
	_matriciesStacks[_matrixMode].multiplyMatrix(matrix);
}


static bool compareTransparentMeshProperties(const TeRenderer::TransparentMeshProperties &p1,
											const TeRenderer::TransparentMeshProperties &p2) {
	return (p1._zOrder < p2._zOrder);
}

void TeRenderer::optimiseTransparentMeshProperties() {
	if (_transparentMeshProps.size() <= 1)
		return;

	// Note: this first bit of logic is in renderTransparentMeshes in the
	// original game, but was moved here to split out OGL-specific code.
	//dumpTransparentMeshProps();

	Common::sort(_transparentMeshProps.begin(), _transparentMeshProps.end(),
		 compareTransparentMeshProperties);

	int vertTotal = 0;
	for (uint i = 0; i < _transparentMeshProps.size(); i++) {
		const uint vcount = _transparentMeshProps[i]._vertexCount;
		for (uint j = 0; j < vcount; j++)
			_transparentMeshVertexNums[vertTotal + j] = (short)(_transparentMeshProps[i]._sourceTransparentMesh + j);
		vertTotal += vcount;
	}

	uint i = 0;
	for (uint other = 1; other < _transparentMeshProps.size(); other++) {
		uint nextI = other;
		if (_transparentMeshProps[i]._camera == _transparentMeshProps[other]._camera
			&& _transparentMeshProps[i]._material == _transparentMeshProps[other]._material
			&& _transparentMeshProps[i]._glTexEnvMode == _transparentMeshProps[other]._glTexEnvMode
			&& _transparentMeshProps[i]._matrix == _transparentMeshProps[other]._matrix
			&& _transparentMeshProps[i]._hasColor == _transparentMeshProps[other]._hasColor
			&& _transparentMeshProps[i]._scissorEnabled == _transparentMeshProps[other]._scissorEnabled
			&& _transparentMeshProps[i]._scissorX == _transparentMeshProps[other]._scissorX
			&& _transparentMeshProps[i]._scissorY == _transparentMeshProps[other]._scissorY
			&& _transparentMeshProps[i]._scissorWidth == _transparentMeshProps[other]._scissorWidth
			&& _transparentMeshProps[i]._scissorHeight == _transparentMeshProps[other]._scissorHeight) {
			_transparentMeshProps[i]._vertexCount += _transparentMeshProps[other]._vertexCount;
			_transparentMeshProps[other]._shouldDraw = false;
			nextI = i;
		}
		i = nextI;
	}

	//dumpTransparentMeshProps();
	//dumpTransparentMeshData();
}

void TeRenderer::popMatrix() {
	_matriciesStacks[_matrixMode].popMatrix();
}

void TeRenderer::pushMatrix() {
	_matriciesStacks[_matrixMode].pushMatrix();
}

void TeRenderer::dumpTransparentMeshProps() const {
	debug("** Transparent MeshProps: num:%d pending:%d **", _numTransparentMeshes, _pendingTransparentMeshProperties);
	debug("draw? / nverts / source / transl / zorder");
	for (uint i = 0; i < _transparentMeshProps.size(); i++) {
		debug("%s %d %d %s %f",
			  _transparentMeshProps[i]._shouldDraw ? "draw" : "nodr",
			  _transparentMeshProps[i]._vertexCount,
			  _transparentMeshProps[i]._sourceTransparentMesh,
			  _transparentMeshProps[i]._matrix.translation().dump().c_str(),
			  _transparentMeshProps[i]._zOrder
			  );
	}
}

void TeRenderer::dumpTransparentMeshData() const {
	debug("** Transparent Meshes: num:%d pending:%d **", _numTransparentMeshes, _pendingTransparentMeshProperties);
	debug("vert / normal / coord / color / vertNo");
	for (uint i = 0; i < _transparentMeshVertexes.size(); i++) {
		debug("%s %s %s %s %d",
			  _transparentMeshVertexes[i].dump().c_str(),
			  _transparentMeshNormals[i].dump().c_str(),
			  _transparentMeshCoords[i].dump().c_str(),
			  _transparentMeshColors[i].dump().c_str(),
			  _transparentMeshVertexNums[i]
			  );
	}
}

void TeRenderer::rotate(const TeQuaternion &rot) {
	_matriciesStacks[_matrixMode].rotate(rot);
}

void TeRenderer::rotate(float angle, float rx, float ry, float rz) {
	_matriciesStacks[_matrixMode].rotate(angle, TeVector3f32(rx, ry, rz));
}

void TeRenderer::scale(float xs, float ys, float zs) {
	_matriciesStacks[_matrixMode].scale(TeVector3f32(xs, ys, zs));
}

void TeRenderer::setScissor(int x, int y, int w, int h) {
	_scissorX = x;
	_scissorY = y;
	_scissorWidth = w;
	_scissorHeight = h;
}

void TeRenderer::translate(float x, float y, float z) {
	_matriciesStacks[_matrixMode].translate(TeVector3f32(x, y, z));
}


/*static*/
TeRenderer *TeRenderer::makeInstance() {
	Graphics::RendererType r = g_engine->preferredRendererType();

#if defined(USE_OPENGL_GAME)
	if (r == Graphics::kRendererTypeOpenGL)
		return new TeRendererOpenGL();
#endif

#if defined(USE_TINYGL)
	if (r == Graphics::kRendererTypeTinyGL)
		return new TeRendererTinyGL();
#endif
	error("Couldn't create TeRenderer for selected renderer");
}


} // end namespace Tetraedge
