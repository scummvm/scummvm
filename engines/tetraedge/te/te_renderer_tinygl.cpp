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
#include "common/config-manager.h"

#include "graphics/tinygl/tinygl.h"

#include "engines/util.h"

#include "tetraedge/te/te_renderer_tinygl.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_light_tinygl.h"
#include "tetraedge/te/te_mesh_tinygl.h"

namespace Tetraedge {

TeRendererTinyGL::TeRendererTinyGL() {
}

void TeRendererTinyGL::clearBuffer(TeRenderer::Buffer buf) {
	TGLenum glBuf = 0;
	if (buf & StencilBuffer)
		glBuf |= TGL_STENCIL_BUFFER_BIT;
	if (buf & DepthBuffer)
		glBuf |= TGL_DEPTH_BUFFER_BIT;
	if (buf & ColorBuffer)
		glBuf |= TGL_COLOR_BUFFER_BIT;
	tglClear(glBuf);
}

void TeRendererTinyGL::colorMask(bool r, bool g, bool b, bool a) {
	tglColorMask(r, g, b, a);
}

void TeRendererTinyGL::disableAllLights() {
	TeLightTinyGL::disableAll();
}

void TeRendererTinyGL::disableTexture() {
	tglDisable(TGL_TEXTURE_2D);
	_textureEnabled = false;
}

void TeRendererTinyGL::disableWireFrame() {
	tglPolygonMode(TGL_FRONT_AND_BACK, TGL_FILL);
}

void TeRendererTinyGL::disableZBuffer() {
	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);
}

void TeRendererTinyGL::drawLine(const TeVector3f32 &from, const TeVector3f32 &to) {
	error("TODO: Implement TeRenderer::drawLine");
}

void TeRendererTinyGL::enableAllLights() {
	TeLightTinyGL::enableAll();
}

void TeRendererTinyGL::enableTexture() {
	tglEnable(TGL_TEXTURE_2D);
	_textureEnabled = true;
}

void TeRendererTinyGL::enableWireFrame() {
	tglPolygonMode(TGL_FRONT_AND_BACK, TGL_LINE);
}

void TeRendererTinyGL::enableZBuffer() {
	tglEnable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_TRUE);
}

void TeRendererTinyGL::init(uint width, uint height) {
	initGraphics(width, height, nullptr);

	const Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
	debug(2, "INFO: TinyGL front buffer pixel format: %s", pixelFormat.toString().c_str());
	TinyGL::createContext(width, height, pixelFormat, 256, true, ConfMan.getBool("dirtyrects"));

	tglViewport(0, 0, width, height);

	tglDisable(TGL_CULL_FACE);
	TeLightTinyGL::disableAll();
	tglDisable(TGL_COLOR_MATERIAL);
	tglEnable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_TRUE);
	tglShadeModel(TGL_SMOOTH);
	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
	tglDepthFunc(TGL_LEQUAL);
	// Original does this, probably not needed?
	//tglHint(TGL_PERSPECTIVE_CORRECTION_HINT, TGL_DONT_CARE);
	tglClearDepth(1.0);
	//tglClearStencil(0);
	_clearColor = TeColor(0, 0, 0, 255);
	tglClearColor(0, 0, 0, 1.0);
	//TeOpenGLExtensions::loadExtensions(); // this does nothing in the game?
	_currentColor = TeColor(255, 255, 255, 255);
	_scissorEnabled = false;
	_scissorX = _scissorY = _scissorWidth = _scissorHeight = 0;
}


void TeRendererTinyGL::loadMatrixToGL(const TeMatrix4x4 &matrix) {
	//int mmode;
	//glGetIntegerv(TGL_MATRIX_MODE, &mmode);
	//debug("loadMatrixToGL[0x%x]: %s", mmode, matrix.toString().c_str());
	tglLoadMatrixf(matrix.getData());
}

void TeRendererTinyGL::loadProjectionMatrix(const TeMatrix4x4 &matrix) {
	tglMatrixMode(TGL_PROJECTION);
	_matrixMode = MM_GL_PROJECTION;
	_matriciesStacks[_matrixMode].loadIdentity();
	_matriciesStacks[_matrixMode].loadMatrix(matrix);
	tglMatrixMode(TGL_MODELVIEW);
	_matrixMode = MM_GL_MODELVIEW;
	_matriciesStacks[_matrixMode].loadIdentity();
}

Common::String TeRendererTinyGL::renderer() {
	return "TinyGL";
}


void TeRendererTinyGL::renderTransparentMeshes() {
	if (!_numTransparentMeshes)
		return;

	tglDepthMask(TGL_FALSE);

	// Note: some code moved to optimiseTransparentMeshProperties to minimise
	// non-OGL-speicifc code.
	optimiseTransparentMeshProperties();

	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglEnableClientState(TGL_NORMAL_ARRAY);
	tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
	tglEnableClientState(TGL_COLOR_ARRAY);

	tglVertexPointer(3, TGL_FLOAT, sizeof(TeVector3f32), _transparentMeshVertexes.data());
	tglNormalPointer(TGL_FLOAT, sizeof(TeVector3f32), _transparentMeshNormals.data());
	tglTexCoordPointer(2, TGL_FLOAT, sizeof(TeVector2f32), _transparentMeshCoords.data());
	tglColorPointer(4, TGL_UNSIGNED_BYTE, sizeof(TeColor), _transparentMeshColors.data());

	TeMaterial lastMaterial;
	TeMatrix4x4 lastMatrix;

	int vertsDrawn = 0;
	for (uint i = 0; i < _transparentMeshProps.size(); i++) {
		const TransparentMeshProperties &meshProperties = _transparentMeshProps[i];
		if (!meshProperties._shouldDraw)
			continue;

		const TeMaterial &material = meshProperties._material;

		meshProperties._camera->applyProjection();
		tglMatrixMode(TGL_MODELVIEW);
		_matrixMode = MM_GL_MODELVIEW;
		tglPushMatrix();
		_matriciesStacks[_matrixMode].pushMatrix();
		_matriciesStacks[_matrixMode].loadMatrix(meshProperties._matrix);
		tglPushMatrix();
		loadCurrentMatrixToGL();
		if (material._texture) {
			tglEnable(TGL_TEXTURE_2D);
			_textureEnabled = true;
		}
		if (material._isShadowTexture) {
			tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
			tglDisableClientState(TGL_COLOR_ARRAY);
		}

		if (material != lastMaterial) {
			applyMaterial(material);
			lastMaterial = material;
		}

		if (meshProperties._scissorEnabled) {
			tglEnable(TGL_SCISSOR_TEST);
			// TODO: No scissoring in TGL..
			/*
			tglScissor(meshProperties._scissorX,
					  meshProperties._scissorY,
					  meshProperties._scissorWidth,
					  meshProperties._scissorHeight);*/
		}
		// TODO: not supported in TGL
		//tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_DECAL/*meshProperties._glTexEnvMode*/);
		tglDrawElements(TGL_TRIANGLES, meshProperties._vertexCount, TGL_UNSIGNED_SHORT,
				   _transparentMeshVertexNums.data() + vertsDrawn);

		vertsDrawn += meshProperties._vertexCount;

		if (material._isShadowTexture) {
			tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
			tglEnableClientState(TGL_COLOR_ARRAY);
		}
		// TODO: not supported in TGL
		//tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_DECAL);
		if (meshProperties._scissorEnabled) {
			tglDisable(TGL_SCISSOR_TEST);
		}
		if (material._texture) {
			tglDisable(TGL_TEXTURE_2D);
			_textureEnabled = false;
		}
		tglPopMatrix();
		tglPopMatrix();
		_matriciesStacks[_matrixMode].popMatrix();
		TeCamera::restore();
	}
	tglDisableClientState(TGL_VERTEX_ARRAY);
	tglDisableClientState(TGL_NORMAL_ARRAY);
	tglDisableClientState(TGL_COLOR_ARRAY);
	tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
	_numTransparentMeshes = 0;
	_pendingTransparentMeshProperties = 0;
	tglDepthMask(TGL_TRUE);
	_transparentMeshProps.clear();
}

void TeRendererTinyGL::reset() {
	clearBuffer(AllBuffers);
	tglMatrixMode(TGL_PROJECTION);
	_matrixMode = MM_GL_PROJECTION;
	_matriciesStacks[MM_GL_PROJECTION].loadIdentity();
	tglMatrixMode(TGL_MODELVIEW);
	_matrixMode = MM_GL_MODELVIEW;
	_matriciesStacks[MM_GL_MODELVIEW].loadIdentity();
}

void TeRendererTinyGL::setClearColor(const TeColor &col) {
	_clearColor = col;
	tglClearColor(col.r() / 255.0f, col.g() / 255.0f, col.b() / 255.0f, col.a() / 255.0f);
}

void TeRendererTinyGL::setCurrentColor(const TeColor &col) {
	if (col == _currentColor)
		return;

	tglColor4ub(col.r(), col.g(), col.b(), col.a());
	_currentColor = col;
}

void TeRendererTinyGL::setMatrixMode(enum MatrixMode mode) {
	TGLenum glmode = 0;
	if (mode == MM_GL_TEXTURE)
		glmode = TGL_TEXTURE;
	else if (mode == MM_GL_MODELVIEW)
		glmode = TGL_MODELVIEW;
	else if (mode == MM_GL_PROJECTION)
		glmode = TGL_PROJECTION;

	if (glmode)
		tglMatrixMode(glmode);
	_matrixMode = mode;
}

void TeRendererTinyGL::setViewport(int x, int y, int w, int h) {
	tglViewport(x, y, w, h);
}

void TeRendererTinyGL::shadowMode(enum ShadowMode mode) {
	_shadowMode = mode;
	if (mode == ShadowModeNone) {
		tglDisable(TGL_CULL_FACE);
		tglShadeModel(TGL_SMOOTH);
		return;
	}

	if (mode == ShadowModeCreating) {
		tglEnable(TGL_CULL_FACE);
		tglCullFace(TGL_BACK);
	} else { // ShadowModeDrawing
		tglDisable(TGL_CULL_FACE);
	}
	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
	tglShadeModel(TGL_FLAT);
	TeLightTinyGL::disableAll();
}

void TeRendererTinyGL::applyMaterial(const TeMaterial &m) {
	//debug("TeMaterial::apply (%s)", dump().c_str());
	//static const float constColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	if (_shadowMode == TeRenderer::ShadowModeNone) {
		if (m._enableLights)
			TeLightTinyGL::enableAll();
		else
			TeLightTinyGL::disableAll();

		if (m._texture) {
			enableTexture();
			tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
			m._texture->bind();
		}

		tglDisable(TGL_ALPHA_TEST);
		if (m._mode == TeMaterial::MaterialMode0) {
			/* TODO: Find TGL equivalents for this stuff*/
			/*
			tglTexEnvfv(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_COLOR, constColor);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_COMBINE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_COMBINE_RGB, TGL_MODULATE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_SOURCE0_RGB, TGL_TEXTURE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_OPERAND0_RGB, TGL_SRC_COLOR);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_COMBINE_ALPHA, TGL_REPLACE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_SOURCE0_ALPHA, TGL_CONSTANT);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_OPERAND0_ALPHA, TGL_SRC_ALPHA);
			*/
		} else {
			// TODO: GL_MODULATE supported in TGL
			//tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_DECAL);
			if (m._mode != TeMaterial::MaterialMode1) {
				tglEnable(TGL_ALPHA_TEST);
				tglAlphaFunc(TGL_GREATER, 0.5);
			}
		}
		const float ambient[4] = { m._ambientColor.r() / 255.0f, m._ambientColor.g() / 255.0f,
			m._ambientColor.b() / 255.0f, m._ambientColor.a() / 255.0f };
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_AMBIENT, ambient);

		const float specular[4] = { m._specularColor.r() / 255.0f, m._specularColor.g() / 255.0f,
			m._specularColor.b() / 255.0f, m._specularColor.a() / 255.0f };
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_SPECULAR, specular);

		const float emission[4] = { m._emissionColor.r() / 255.0f, m._emissionColor.g() / 255.0f,
			m._emissionColor.b() / 255.0f, m._emissionColor.a() / 255.0f };
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_EMISSION, emission);

		tglMaterialf(TGL_FRONT, TGL_SHININESS, m._shininess);

		const float diffuse[4] = { m._diffuseColor.r() / 255.0f, m._diffuseColor.g() / 255.0f,
			m._diffuseColor.b() / 255.0f, m._diffuseColor.a() / 255.0f };
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_DIFFUSE, diffuse);

		setCurrentColor(m._diffuseColor);
	} else if (_shadowMode == TeRenderer::ShadowModeCreating) {
		// NOTE: Diverge from original here, it sets 255.0 but the
		// colors should be scaled -1.0 .. 1.0.
		static const float fullColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		TeLightTinyGL::disableAll();
		tglDisable(TGL_ALPHA_TEST);
		// TODO: GL_MODULATE not supported in TGL
		//tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_DECAL);
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_AMBIENT, fullColor);
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_DIFFUSE, fullColor);
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_SPECULAR, fullColor);
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_EMISSION, fullColor);
	}

	if (!m._isShadowTexture) {
		tglDisable(TGL_TEXTURE_GEN_S);
		tglDisable(TGL_TEXTURE_GEN_T);
		tglDisable(TGL_TEXTURE_GEN_R);
		tglDisable(TGL_TEXTURE_GEN_Q);
	} else {
		// TODO: GL_MODULATE not supported in TGL
		//tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_DECAL);
		tglEnable(TGL_TEXTURE_GEN_S);
		tglEnable(TGL_TEXTURE_GEN_T);
		tglEnable(TGL_TEXTURE_GEN_R);
		tglEnable(TGL_TEXTURE_GEN_Q);
		tglEnable(TGL_TEXTURE_2D);
		TeLightTinyGL::disableAll();
		tglDisable(TGL_ALPHA_TEST);
		enableTexture();
		// TODO: GL_MODULATE not supported in TGL
		//tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_DECAL);

		const float diffuse[4] = { m._diffuseColor.r() / 255.0f, m._diffuseColor.g() / 255.0f,
			m._diffuseColor.b() / 255.0f, m._diffuseColor.a() / 255.0f };

		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_AMBIENT, diffuse);
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_DIFFUSE, diffuse);
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_SPECULAR, diffuse);
		tglMaterialfv(TGL_FRONT_AND_BACK, TGL_EMISSION, diffuse);
	}
}

void TeRendererTinyGL::updateGlobalLight() {
	TeLightTinyGL::updateGlobal();
}

void TeRendererTinyGL::updateScreen() {
      Common::List<Common::Rect> dirtyAreas;
      TinyGL::presentBuffer(dirtyAreas);

      Graphics::Surface glBuffer;
      TinyGL::getSurfaceRef(glBuffer);

      if (!dirtyAreas.empty()) {
          for (Common::List<Common::Rect>::iterator itRect = dirtyAreas.begin(); itRect != dirtyAreas.end(); ++itRect) {
              g_system->copyRectToScreen(glBuffer.getBasePtr((*itRect).left, (*itRect).top), glBuffer.pitch,
                                         (*itRect).left, (*itRect).top, (*itRect).width(), (*itRect).height());
          }
      }

      g_system->updateScreen();
}

Common::String TeRendererTinyGL::vendor() {
	return "TinyGL vendor";
}

} // end namespace Tetraedge
