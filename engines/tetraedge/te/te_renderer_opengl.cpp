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
#include "common/system.h"

#include "graphics/opengl/system_headers.h"

#include "engines/util.h"

#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_renderer_opengl.h"
#include "tetraedge/te/te_light_opengl.h"
#include "tetraedge/te/te_mesh_opengl.h"

namespace Tetraedge {

TeRendererOpenGL::TeRendererOpenGL() {
}

void TeRendererOpenGL::clearBuffer(TeRenderer::Buffer buf) {
	GLenum glBuf = 0;
	if (buf & StencilBuffer)
		glBuf |= GL_STENCIL_BUFFER_BIT;
	if (buf & DepthBuffer)
		glBuf |= GL_DEPTH_BUFFER_BIT;
	if (buf & ColorBuffer)
		glBuf |= GL_COLOR_BUFFER_BIT;
	glClear(glBuf);
}

void TeRendererOpenGL::colorMask(bool r, bool g, bool b, bool a) {
	glColorMask(r, g, b, a);
}

void TeRendererOpenGL::disableAllLights() {
	TeLightOpenGL::disableAll();
}

void TeRendererOpenGL::disableTexture() {
	glDisable(GL_TEXTURE_2D);
	_textureEnabled = false;
}

void TeRendererOpenGL::disableWireFrame() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void TeRendererOpenGL::disableZBuffer() {
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

void TeRendererOpenGL::drawLine(const TeVector3f32 &from, const TeVector3f32 &to) {
	error("TODO: Implement TeRenderer::drawLine");
}

void TeRendererOpenGL::enableAllLights() {
	TeLightOpenGL::enableAll();
}

void TeRendererOpenGL::enableTexture() {
	glEnable(GL_TEXTURE_2D);
	_textureEnabled = true;
}

void TeRendererOpenGL::enableWireFrame() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void TeRendererOpenGL::enableZBuffer() {
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void TeRendererOpenGL::init(uint width, uint height) {
	initGraphics3d(width, height);
	glDisable(GL_CULL_FACE);
	TeLightOpenGL::disableAll();
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	// Note: original doesn't separate but blends are nicer that way.
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
	// Original does this, probably not needed?
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_DONT_CARE);
	glClearDepth(1.0);
	glClearStencil(0);
	_clearColor = TeColor(0, 0, 0, 255);
	glClearColor(0, 0, 0, 1.0);
	//TeOpenGLExtensions::loadExtensions(); // this does nothing in the game?
	_currentColor = TeColor(255, 255, 255, 255);
	_scissorEnabled = false;
	_scissorX = _scissorY = _scissorWidth = _scissorHeight = 0;
}

void TeRendererOpenGL::loadMatrixToGL(const TeMatrix4x4 &matrix) {
	//int mmode;
	//glGetIntegerv(GL_MATRIX_MODE, &mmode);
	//debug("loadMatrixToGL[0x%x]: %s", mmode, matrix.toString().c_str());
	glLoadMatrixf(matrix.getData());
}

void TeRendererOpenGL::loadProjectionMatrix(const TeMatrix4x4 &matrix) {
	glMatrixMode(GL_PROJECTION);
	_matrixMode = MM_GL_PROJECTION;
	_matriciesStacks[_matrixMode].loadIdentity();
	_matriciesStacks[_matrixMode].loadMatrix(matrix);
	glMatrixMode(GL_MODELVIEW);
	_matrixMode = MM_GL_MODELVIEW;
	_matriciesStacks[_matrixMode].loadIdentity();
}

Common::String TeRendererOpenGL::renderer() {
	return Common::String((const char *)glGetString(GL_RENDERER));
}


void TeRendererOpenGL::renderTransparentMeshes() {
	if (!_numTransparentMeshes)
		return;

	glDepthMask(GL_FALSE);

	// Note: some code moved to optimiseTransparentMeshProperties to minimise
	// non-OGL-speicifc code.
	optimiseTransparentMeshProperties();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(TeVector3f32), _transparentMeshVertexes.data());
	glNormalPointer(GL_FLOAT, sizeof(TeVector3f32), _transparentMeshNormals.data());
	glTexCoordPointer(2, GL_FLOAT, sizeof(TeVector2f32), _transparentMeshCoords.data());
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(TeColor), _transparentMeshColors.data());

	TeMaterial lastMaterial;
	TeMatrix4x4 lastMatrix;

	int vertsDrawn = 0;
	for (uint i = 0; i < _transparentMeshProps.size(); i++) {
		const TransparentMeshProperties &meshProperties = _transparentMeshProps[i];
		if (!meshProperties._shouldDraw)
			continue;

		const TeMaterial &material = meshProperties._material;

		meshProperties._camera->applyProjection();
		glMatrixMode(GL_MODELVIEW);
		_matrixMode = MM_GL_MODELVIEW;
		glPushMatrix();
		_matriciesStacks[_matrixMode].pushMatrix();
		_matriciesStacks[_matrixMode].loadMatrix(meshProperties._matrix);
		glPushMatrix();
		loadCurrentMatrixToGL();
		if (material._texture) {
			glEnable(GL_TEXTURE_2D);
			_textureEnabled = true;
		}
		if (material._isShadowTexture) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
		}

		if (material != lastMaterial) {
			applyMaterial(material);
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
		glDrawElements(GL_TRIANGLES, meshProperties._vertexCount, GL_UNSIGNED_SHORT,
				   _transparentMeshVertexNums.data() + vertsDrawn);

		vertsDrawn += meshProperties._vertexCount;

		if (material._isShadowTexture) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		if (meshProperties._scissorEnabled) {
			glDisable(GL_SCISSOR_TEST);
		}
		if (material._texture) {
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
	_transparentMeshProps.clear();
}

void TeRendererOpenGL::reset() {
	clearBuffer(AllBuffers);
	glMatrixMode(GL_PROJECTION);
	_matrixMode = MM_GL_PROJECTION;
	_matriciesStacks[MM_GL_PROJECTION].loadIdentity();
	glMatrixMode(GL_MODELVIEW);
	_matrixMode = MM_GL_MODELVIEW;
	_matriciesStacks[MM_GL_MODELVIEW].loadIdentity();
}

void TeRendererOpenGL::setClearColor(const TeColor &col) {
	_clearColor = col;
	glClearColor(col.r() / 255.0f, col.g() / 255.0f, col.b() / 255.0f, col.a() / 255.0f);
}

void TeRendererOpenGL::setCurrentColor(const TeColor &col) {
	if (col == _currentColor)
		return;

	glColor4ub(col.r(), col.g(), col.b(), col.a());
	_currentColor = col;
}

void TeRendererOpenGL::setMatrixMode(enum MatrixMode mode) {
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

void TeRendererOpenGL::setViewport(int x, int y, int w, int h) {
	glViewport(x, y, w, h);
}

void TeRendererOpenGL::shadowMode(enum ShadowMode mode) {
	_shadowMode = mode;
	if (mode == ShadowModeNone) {
		glDisable(GL_CULL_FACE);
		glShadeModel(GL_SMOOTH);
		return;
	}

	if (mode == ShadowModeCreating) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	} else { // ShadowModeDrawing
		glDisable(GL_CULL_FACE);
	}
	glEnable(GL_BLEND);
	// Note: original doesn't separate but blends are nicer that way.
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_FLAT);
	TeLightOpenGL::disableAll();
}

void TeRendererOpenGL::applyMaterial(const TeMaterial &m) {
	//debug("TeMaterial::apply (%s)", dump().c_str());
	static const float constColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	if (_shadowMode == ShadowModeNone) {
		if (m._enableLights)
			TeLightOpenGL::enableAll();
		else
			TeLightOpenGL::disableAll();

		if (m._texture) {
			enableTexture();
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			m._texture->bind();
		}

		glDisable(GL_ALPHA_TEST);
		if (m._mode == TeMaterial::MaterialMode0) {
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_CONSTANT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		} else {
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			if (m._mode != TeMaterial::MaterialMode1) {
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.5);
			}
		}
		const float ambient[4] = { m._ambientColor.r() / 255.0f, m._ambientColor.g() / 255.0f,
			m._ambientColor.b() / 255.0f, m._ambientColor.a() / 255.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

		const float specular[4] = { m._specularColor.r() / 255.0f, m._specularColor.g() / 255.0f,
			m._specularColor.b() / 255.0f, m._specularColor.a() / 255.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

		const float emission[4] = { m._emissionColor.r() / 255.0f, m._emissionColor.g() / 255.0f,
			m._emissionColor.b() / 255.0f, m._emissionColor.a() / 255.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);

		glMaterialf(GL_FRONT, GL_SHININESS, m._shininess);

		const float diffuse[4] = { m._diffuseColor.r() / 255.0f, m._diffuseColor.g() / 255.0f,
			m._diffuseColor.b() / 255.0f, m._diffuseColor.a() / 255.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);

		setCurrentColor(m._diffuseColor);
	} else if (_shadowMode == ShadowModeCreating) {
		// NOTE: Replicate seeming bug from original here, it sets 255.0 but the
		// colors should be scaled -1.0 .. 1.0?
		static const float fullColor[4] = { 255.0f, 255.0f, 255.0f, 255.0f };
		TeLightOpenGL::disableAll();
		glDisable(GL_ALPHA_TEST);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fullColor);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fullColor);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fullColor);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, fullColor);
	}

	if (!m._isShadowTexture) {
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_Q);
	} else {
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);
		glEnable(GL_TEXTURE_2D);
		TeLightOpenGL::disableAll();
		glDisable(GL_ALPHA_TEST);
		enableTexture();
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		const float diffuse[4] = { m._diffuseColor.r() / 255.0f, m._diffuseColor.g() / 255.0f,
			m._diffuseColor.b() / 255.0f, m._diffuseColor.a() / 255.0f };

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, diffuse);
	}
}

void TeRendererOpenGL::updateGlobalLight() {
	TeLightOpenGL::updateGlobal();
}

void TeRendererOpenGL::updateScreen() {
	g_system->updateScreen();
}

Common::String TeRendererOpenGL::vendor() {
	return Common::String((const char *)glGetString(GL_VENDOR));
}

} // end namespace Tetraedge
