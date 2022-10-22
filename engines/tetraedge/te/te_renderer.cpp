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

#include "graphics/opengl/system_headers.h"

#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_light.h"

namespace Tetraedge {

TeRenderer::TeRenderer() : _textureEnabled(false), _shadowMode(ShadowMode0), _matrixMode(MM_GL_PROJECTION),
_numTransparentMeshes(0), _pendingTransparentMeshProperties(0) {
}

void TeRenderer::TransparentMeshProperties::setFromMaterial(const TeMaterial &material) {
	_texture = material._texture;
	_enableLights = material._enableLights;
	_enableSomethingDefault0 = material._enableSomethingDefault0;
	_shininess = material._shininess;
	_emissionColor = material._emissionColor;
	_specularColor = material._specularColor;
	_diffuseColor = material._diffuseColor;
	_ambientColor = material._ambientColor;
	_materialMode = material._mode;
}

void TeRenderer::addTransparentMesh(const TeMesh &mesh, unsigned long i1, unsigned long meshno, unsigned long materialno) {
	const float orthNearVal = _currentCamera->_orthNearVal;
	const TeMesh::Mode meshMode = mesh.getMode();
	if (!meshno) {
		if (meshMode == TeMesh::MeshMode_TriangleStrip) {
			meshno = mesh.numVerticies() - 2;
		} else if (meshMode == TeMesh::MeshMode_Triangles) {
			meshno = mesh.numIndexes() / 3;
		} else {
			return;
		}
		if (meshno == 0)
			return;
	}
	_transparentMeshVertexes.resize((_numTransparentMeshes + meshno) * 3);
	_transparentMeshNormals.resize((_numTransparentMeshes + meshno) * 3);
	_transparentMeshCoords.resize((_numTransparentMeshes + meshno) * 3);
	_transparentMeshColors.resize((_numTransparentMeshes + meshno) * 3);
	_transparentMeshTriangleNums.resize((_numTransparentMeshes + meshno) * 3);

	int newPropsSize = mesh.shouldDrawMaybe() ? _pendingTransparentMeshProperties + meshno : _pendingTransparentMeshProperties + 1;
	_transparentMeshProperties.resize(newPropsSize);
	if (meshMode == TeMesh::MeshMode_Triangles && meshno > 0) {
		for (unsigned int i = 0; i < meshno; i++) {
			const uint meshNo0 = (i1 + i) * 3;
			const uint meshPropNo = (_numTransparentMeshes + i) * 3;

			_transparentMeshVertexes[meshPropNo] = mesh.vertex(mesh.index(meshNo0));
			_transparentMeshVertexes[meshPropNo + 1] = mesh.vertex(mesh.index(meshNo0 + 1));
			_transparentMeshVertexes[meshPropNo + 2] = mesh.vertex(mesh.index(meshNo0 + 2));

			_transparentMeshNormals[meshPropNo] = mesh.normal(mesh.index(meshNo0));
			_transparentMeshNormals[meshPropNo + 1] = mesh.normal(mesh.index(meshNo0 + 1));
			_transparentMeshNormals[meshPropNo + 2] = mesh.normal(mesh.index(meshNo0 + 2));

			if (mesh.hasUvs()) {
				_transparentMeshCoords[meshPropNo] = mesh.textureUV(mesh.index(meshNo0));
				_transparentMeshCoords[meshPropNo + 1] = mesh.textureUV(mesh.index(meshNo0) + 1);
				_transparentMeshCoords[meshPropNo + 2] = mesh.textureUV(mesh.index(meshNo0) + 2);
			}

			if (!mesh.hasColor()) {
				_transparentMeshColors[meshPropNo] = mesh.material(materialno)->_diffuseColor;
				_transparentMeshColors[meshPropNo + 1] = mesh.material(materialno)->_diffuseColor;
				_transparentMeshColors[meshPropNo + 2] = mesh.material(materialno)->_diffuseColor;
			} else {
				_transparentMeshColors[meshPropNo] = mesh.color(mesh.index(meshNo0));
				_transparentMeshColors[meshPropNo + 1] = mesh.color(mesh.index(meshNo0) + 1);
				_transparentMeshColors[meshPropNo + 2] = mesh.color(mesh.index(meshNo0) + 2);
			}
		}
	} else if (meshMode == TeMesh::MeshMode_TriangleStrip && meshno > 0) {
		for (unsigned int i = 0; i < meshno; i++) {
			const uint meshNo0 = (i1 + i);  // TODO: This appears to be the only difference between this and the above?
			const uint meshPropNo = (_numTransparentMeshes + i) * 3;

			_transparentMeshVertexes[meshPropNo] = mesh.vertex(mesh.index(meshNo0));
			_transparentMeshVertexes[meshPropNo + 1] = mesh.vertex(mesh.index(meshNo0 + 1));
			_transparentMeshVertexes[meshPropNo + 2] = mesh.vertex(mesh.index(meshNo0 + 2));

			_transparentMeshNormals[meshPropNo] = mesh.normal(mesh.index(meshNo0));
			_transparentMeshNormals[meshPropNo + 1] = mesh.normal(mesh.index(meshNo0 + 1));
			_transparentMeshNormals[meshPropNo + 2] = mesh.normal(mesh.index(meshNo0 + 2));

			if (mesh.hasUvs()) {
				_transparentMeshCoords[meshPropNo] = mesh.textureUV(mesh.index(meshNo0));;
				_transparentMeshCoords[meshPropNo + 1] = mesh.textureUV(mesh.index(meshNo0 + 1));
				_transparentMeshCoords[meshPropNo + 2] = mesh.textureUV(mesh.index(meshNo0 + 2));
			}

			if (!mesh.hasColor()) {
				_transparentMeshColors[meshPropNo] =  mesh.material(materialno)->_diffuseColor;
				_transparentMeshColors[meshPropNo + 1] =  mesh.material(materialno)->_diffuseColor;
				_transparentMeshColors[meshPropNo + 2] =  mesh.material(materialno)->_diffuseColor;
			} else {
				_transparentMeshColors[meshPropNo] = mesh.color(mesh.index(meshNo0));
				_transparentMeshColors[meshPropNo + 1] = mesh.color(mesh.index(meshNo0 + 1));
				_transparentMeshColors[meshPropNo + 2] = mesh.color(mesh.index(meshNo0 + 2));
			}
		}
	}

	if (!mesh.shouldDrawMaybe()) {
		// TODO: better variable names.
		const TeMatrix4x4 &currentMatrix = _matriciesStacks[MM_GL_MODELVIEW].currentMatrix();
		const TeVector3f32 local_268 = currentMatrix.mult4x3(_transparentMeshVertexes[_numTransparentMeshes * 3]);
		const TeVector3f32 local_278 = currentMatrix.mult4x3(_transparentMeshVertexes[_numTransparentMeshes * 3 + 1]);
		const TeVector3f32 local_288 = currentMatrix.mult4x3(_transparentMeshVertexes[_numTransparentMeshes * 3 + 2]);
		TeVector3f32 local_298 = (local_268 + local_278 + local_288) / 3.0;

		local_298.z() -= orthNearVal;
		float length;
		if (_currentCamera->_projectionMatrixType < 4) {
			length = -local_298.squaredLength();
		} else if (_currentCamera->_projectionMatrixType == 4) {
			length = local_298.z() * local_298.z();
		} else {
			length = local_298.squaredLength();
		}

		TransparentMeshProperties &destProperties = _transparentMeshProperties[_pendingTransparentMeshProperties];

		destProperties._triangleCount = meshno * 3;
		destProperties._camera = _currentCamera;

		const TeMaterial *material = mesh.material(materialno);
		destProperties.setFromMaterial(*material);
		destProperties._matrix = currentMatrix;

		destProperties._glTexEnvMode = mesh.gltexenvMode();
		destProperties._sourceTransparentMesh = _numTransparentMeshes * 3;
		destProperties._hasColor = mesh.hasColor();
		destProperties._zLength = length;
		destProperties._scissorEnabled = _scissorEnabled;
		destProperties._scissorX = _scissorX;
		destProperties._scissorY = _scissorY;
		destProperties._scissorWidth = _scissorWidth;
		destProperties._scissorHeight = _scissorHeight;
		destProperties._shouldDraw = false;
	} else {
		for (uint i = 0; i < meshno; i++) {
			const TeMatrix4x4 &currentMatrix = _matriciesStacks[MM_GL_MODELVIEW].currentMatrix();

			const int meshPropNo = (_numTransparentMeshes + i) * 3;
			_transparentMeshVertexes[meshPropNo] = currentMatrix.mult4x3(_transparentMeshVertexes[meshPropNo]);
			_transparentMeshVertexes[meshPropNo + 1] = currentMatrix.mult4x3(_transparentMeshVertexes[meshPropNo + 1]);
			_transparentMeshVertexes[meshPropNo + 2] = currentMatrix.mult4x3(_transparentMeshVertexes[meshPropNo + 2]);

			_transparentMeshNormals[meshPropNo] = currentMatrix.mult3x3(_transparentMeshNormals[meshPropNo]);
			_transparentMeshNormals[meshPropNo + 1] = currentMatrix.mult3x3(_transparentMeshNormals[meshPropNo + 1]);
			_transparentMeshNormals[meshPropNo + 2] = currentMatrix.mult3x3(_transparentMeshNormals[meshPropNo + 2]);

			TeVector3f32 local_208 = (_transparentMeshVertexes[meshPropNo] + _transparentMeshVertexes[meshPropNo + 1] + _transparentMeshVertexes[meshPropNo + 2]) / 3.0;
			local_208.z() -= orthNearVal;

			float length;
			if (_currentCamera->_projectionMatrixType < 4) {
				length = -local_208.squaredLength();
			} else if (_currentCamera->_projectionMatrixType == 4) {
				length = local_208.z() * local_208.z();
			} else {
				length = local_208.squaredLength();
			}

			TransparentMeshProperties &destProperties = _transparentMeshProperties[_pendingTransparentMeshProperties + i];
			destProperties._triangleCount = 3;
			destProperties._camera = _currentCamera;

			const TeMaterial *material = mesh.material(materialno);
			destProperties.setFromMaterial(*material);
			destProperties._glTexEnvMode = mesh.gltexenvMode();
			destProperties._sourceTransparentMesh = meshPropNo;
			destProperties._hasColor = mesh.hasColor();
			destProperties._zLength = length;
			destProperties._scissorEnabled = _scissorEnabled;
			destProperties._scissorX = _scissorX;
			destProperties._scissorY = _scissorY;
			destProperties._scissorWidth = _scissorWidth;
			destProperties._scissorHeight = _scissorHeight;
			destProperties._shouldDraw = true;
		}
	}
	_numTransparentMeshes += meshno;
	_pendingTransparentMeshProperties = _transparentMeshProperties.size();
}

void TeRenderer::clearBuffer(TeRenderer::Buffer buf) {
	glClear(buf);
}

void TeRenderer::create() {
	_textureEnabled = false;
	_currentCamera = nullptr;
	_shadowMode = ShadowMode0;
	_numTransparentMeshes = 0;
	_pendingTransparentMeshProperties = 0;
}

TeMatrix4x4 TeRenderer::currentMatrix() {
	return _matriciesStacks[_matrixMode].currentMatrix();
}

void TeRenderer::disableTexture() {
	glDisable(GL_TEXTURE_2D);
	_textureEnabled = false;
}

void TeRenderer::disableWireFrame() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void TeRenderer::disableZBuffer() {
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

void TeRenderer::drawLine(const TeVector3f32 &from, const TeVector3f32 &to) {
	error("TODO: TeRenderer::drawLine Implement me.");
}

void TeRenderer::enableTexture() {
	glEnable(GL_TEXTURE_2D);
	_textureEnabled = true;
}

void TeRenderer::enableWireFrame() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void TeRenderer::enableZBuffer() {
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void TeRenderer::init() {
	glDisable(GL_CULL_FACE);
	TeLight::disableAll();
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_DONT_CARE);
	glClearDepth(1.0);
	glClearStencil(0);
	_clearColor = TeColor(0, 0, 0, 255);
	glClearColor(0, 0, 0, 1.0);
	debug("[TeRenderer::init] Vendor : %s", glGetString(GL_VENDOR));
	debug("[TeRenderer::init] Renderer : %s", glGetString(GL_RENDERER));
	debug("[TeRenderer::init] Version : %s", glGetString(GL_VERSION));
	int bits;
	glGetIntegerv(GL_STENCIL_BITS, &bits);
	debug("[TeRenderer::init] Sentil buffer bits : %d", bits);
	glGetIntegerv(GL_DEPTH_BITS, &bits);
	debug("[TeRenderer::init] Depth buffer bits : %d", bits);
	//debug("[TeRenderer::init] Extensions : %s\n", glGetString(GL_EXTENSIONS));
	//TeOpenGLExtensions::loadExtensions(); // this does nothing in the game?
	_currentColor = TeColor(255, 255, 255, 255);
	_scissorEnabled = false;
	_scissorX = _scissorY = _scissorWidth = _scissorHeight = 0;
}


void TeRenderer::loadIdentityMatrix() {
	_matriciesStacks[_matrixMode].loadIdentity();
}

void TeRenderer::loadMatrix(const TeMatrix4x4 &matrix) {
	_matriciesStacks[_matrixMode].loadMatrix(matrix);
}

void TeRenderer::loadMatrixToGL(const TeMatrix4x4 &matrix) {
	int mmode;
	glGetIntegerv(GL_MATRIX_MODE, &mmode);
	//debug("loadMatrixToGL[0x%x]: %s", mmode, matrix.toString().c_str());
	glLoadMatrixf(matrix.getData());
}

void TeRenderer::loadCurrentMatrixToGL() {
	const TeMatrix4x4 current = currentMatrix();
	loadMatrixToGL(current);
}

void TeRenderer::loadProjectionMatrix(const TeMatrix4x4 &matrix) {
	glMatrixMode(GL_PROJECTION);
	_matrixMode = MM_GL_PROJECTION;
	_matriciesStacks[_matrixMode].loadIdentity();
	_matriciesStacks[_matrixMode].loadMatrix(matrix);
	glMatrixMode(GL_MODELVIEW);
	_matrixMode = MM_GL_MODELVIEW;
	_matriciesStacks[_matrixMode].loadIdentity();
}

void TeRenderer::multiplyMatrix(const TeMatrix4x4 &matrix) {
	_matriciesStacks[_matrixMode].multiplyMatrix(matrix);
}

void TeRenderer::optimiseTransparentMeshProperties() {
	if (!_transparentMeshProperties.empty()) {
		//warning("FIXME: Implement TeRenderer::optimiseTransparentMeshProperties.");
	}
}

void TeRenderer::popMatrix() {
	_matriciesStacks[_matrixMode].popMatrix();
}

void TeRenderer::pushMatrix() {
	_matriciesStacks[_matrixMode].pushMatrix();
}

Common::String TeRenderer::renderer() {
	return Common::String((const char *)glGetString(GL_RENDERER));
}


static int compareTransparentMeshProperties(const TeRenderer::TransparentMeshProperties &p1,
											const TeRenderer::TransparentMeshProperties &p2) {
	if (p1._zLength < p2._zLength)
		return -1;
	if (p1._zLength == p2._zLength)
		return 0;
	return 1;
}

void TeRenderer::dumpTransparentMeshes() const {
	debug("** Transparent Meshes: num:%ld pending:%d **", _numTransparentMeshes, _pendingTransparentMeshProperties);
	debug("vert / normal / coord / color / trianglenum");
	for (unsigned int i = 0; i < _transparentMeshVertexes.size(); i++) {
		debug("%s %s %s %s %d",
			  _transparentMeshVertexes[i].dump().c_str(),
			  _transparentMeshNormals[i].dump().c_str(),
			  _transparentMeshCoords[i].dump().c_str(),
			  _transparentMeshColors[i].dump().c_str(),
			  _transparentMeshTriangleNums[i]
			  );
	}
}

void TeRenderer::renderTransparentMeshes() {
	if (!_numTransparentMeshes)
		return;

	glDepthMask(GL_FALSE);
	Common::sort(_transparentMeshProperties.begin(), _transparentMeshProperties.end(),
		 compareTransparentMeshProperties);

	int triangles = 0;
	for (unsigned int i = 0; i < _transparentMeshProperties.size(); i++) {
		const uint tcount = _transparentMeshProperties[i]._triangleCount;
		for (unsigned int j = 0; j < tcount; j++)
			_transparentMeshTriangleNums[triangles + j] = (short)(_transparentMeshProperties[i]._sourceTransparentMesh + j);
		triangles += tcount;
	}

	//dumpTransparentMeshes();

	optimiseTransparentMeshProperties();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 12, _transparentMeshVertexes.data());
	glNormalPointer(GL_FLOAT, 12, _transparentMeshNormals.data());
	glTexCoordPointer(2, GL_FLOAT, 8, _transparentMeshCoords.data());
	glColorPointer(4, GL_UNSIGNED_BYTE, 4, _transparentMeshColors.data());

	TeMaterial lastMaterial;

	triangles = 0;
	for (unsigned int i = 0; i < _transparentMeshProperties.size(); i++) {
		const TransparentMeshProperties &meshProperties = _transparentMeshProperties[i];
		if (!meshProperties._shouldDraw)
			continue;

		const TeIntrusivePtr<Te3DTexture> &texture = meshProperties._texture;

		TeMaterial material(texture, meshProperties._materialMode);
		material._ambientColor = meshProperties._ambientColor;
		material._diffuseColor = meshProperties._diffuseColor;
		material._specularColor = meshProperties._specularColor;
		material._emissionColor = meshProperties._emissionColor;
		material._shininess = meshProperties._shininess;
		material._enableLights = meshProperties._enableLights;
		material._enableSomethingDefault0 = meshProperties._enableSomethingDefault0;

		meshProperties._camera->applyProjection();
		glMatrixMode(GL_MODELVIEW);
		_matrixMode = MM_GL_MODELVIEW;
		glPushMatrix();
		_matriciesStacks[_matrixMode].pushMatrix();
		_matriciesStacks[_matrixMode].loadMatrix(meshProperties._matrix);
		glPushMatrix();
		loadCurrentMatrixToGL();
		if (texture) {
			glEnable(GL_TEXTURE_2D);
			_textureEnabled = true;
		}
		if (material._enableSomethingDefault0) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
		}

		if (material != lastMaterial) {
			material.apply();
			lastMaterial = material;
		}

		if (meshProperties._scissorEnabled) {
			glEnable(GL_SCISSOR_TEST);
			glScissor(meshProperties._scissorX,
					  meshProperties._scissorY,
					  meshProperties._scissorWidth,
					  meshProperties._scissorHeight);
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, meshProperties._glTexEnvMode);
		glDrawElements(GL_TRIANGLES, meshProperties._triangleCount, GL_UNSIGNED_SHORT,
				   _transparentMeshTriangleNums.data() + triangles);

		triangles += meshProperties._triangleCount;

		if (material._enableSomethingDefault0) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		if (meshProperties._scissorEnabled) {
			glDisable(GL_SCISSOR_TEST);
		}
		if (texture) {
			glDisable(GL_TEXTURE_2D);
			_textureEnabled = false;
		}
		glPopMatrix();
		glPopMatrix();
		_matriciesStacks[_matrixMode].popMatrix();
		TeCamera::restore();
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	_numTransparentMeshes = 0;
	_pendingTransparentMeshProperties = 0;
	glDepthMask(GL_TRUE);
	_transparentMeshProperties.clear();
}

void TeRenderer::reset() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	_matrixMode = MM_GL_PROJECTION;
	_matriciesStacks[MM_GL_PROJECTION].loadIdentity();
	glMatrixMode(GL_MODELVIEW);
	_matrixMode = MM_GL_MODELVIEW;
	_matriciesStacks[MM_GL_MODELVIEW].loadIdentity();
}

void TeRenderer::rotate(const TeQuaternion &rot) {
	error("TODO: TeRenderer::rotate Implement me.");
}

void TeRenderer::rotate(float f1, float f2, float f3, float f4) {
	error("TODO: TeRenderer::rotate Implement me.");
}

void TeRenderer::scale(float xs, float ys, float zs) {
	error("TODO: TeRenderer::scale Implement me.");
}

void TeRenderer::setClearColor(const TeColor &col) {
	_clearColor = col;
	glClearColor(col.r() / 255.0f, col.g() / 255.0f, col.b() / 255.0f, col.a() / 255.0f);
}

void TeRenderer::setCurrentColor(const TeColor &col) {
	if (col == _currentColor)
		return;

	glColor4ub(col.r(), col.g(), col.b(), col.a());
	_currentColor = col;
}

void TeRenderer::setMatrixMode(enum MatrixMode mode) {
	GLenum glmode = 0;
	if (mode == MM_GL_TEXTURE)
		glmode = GL_TEXTURE;
	else if (mode == MM_GL_MODELVIEW)
		glmode = GL_MODELVIEW;
	else if (mode == MM_GL_PROJECTION)
		glmode = GL_PROJECTION;

	if (glmode)
		glMatrixMode(glmode);
	_matrixMode = mode;
}

void TeRenderer::setScissor(int x, int y, int w, int h) {
	_scissorX = x;
	_scissorY = y;
	_scissorWidth = w;
	_scissorHeight = h;
}

void TeRenderer::setViewport(int x, int y, int w, int h) {
	glViewport(x, y, w, h);
}

void TeRenderer::shadowMode(enum ShadowMode mode) {
	_shadowMode = mode;
	if (mode == ShadowMode0) {
		glDisable(GL_CULL_FACE);
		glShadeModel(GL_SMOOTH);
		return;
	}

	if (mode == ShadowMode1) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	} else { // ShadowMode2
		glDisable(GL_CULL_FACE);
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_FLAT);
	TeLight::disableAll();
}

void TeRenderer::translate(float x, float y, float z) {
	error("TODO: TeRenderer::translate Implement me.");
}

Common::String TeRenderer::vendor() {
	return Common::String((const char *)glGetString(GL_VENDOR));
}

} // end namespace Tetraedge
