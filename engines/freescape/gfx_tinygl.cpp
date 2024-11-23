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

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#include "common/config-manager.h"
#include "common/system.h"
#include "graphics/tinygl/tinygl.h"
#include "math/glmath.h"

#include "freescape/objects/object.h"
#include "freescape/gfx_tinygl.h"
#include "freescape/gfx_tinygl_texture.h"

namespace Freescape {

Renderer *CreateGfxTinyGL(int screenW, int screenH, Common::RenderMode renderMode) {
	return new TinyGLRenderer(screenW, screenH, renderMode);
}

TinyGLRenderer::TinyGLRenderer(int screenW, int screenH, Common::RenderMode renderMode) : Renderer(screenW, screenH, renderMode, true) {
	_verts = (Vertex *)malloc(sizeof(Vertex) * kVertexArraySize);
	_texCoord = (Coord *)malloc(sizeof(Coord) * kVertexArraySize);
	_texturePixelFormat = getRGBAPixelFormat();
	_variableStippleArray = nullptr;
}

TinyGLRenderer::~TinyGLRenderer() {
	for (auto &it : _stippleTextureCache) {
		delete (TinyGL3DTexture *)it._value;
	}
	TinyGL::destroyContext();
	free(_verts);
	free(_texCoord);
}

Texture *TinyGLRenderer::createTexture(const Graphics::Surface *surface, bool is3D) {
	if (is3D)
		return new TinyGL3DTexture(surface);
	else
		return new TinyGL2DTexture(surface);
}

void TinyGLRenderer::freeTexture(Texture *texture) {
	//TinyGLTexture *glTexture = static_cast<TinyGLTexture *>(texture);
	delete texture;
}

void TinyGLRenderer::init() {
	// debug("Initializing Software 3D Renderer");

	computeScreenViewport();

	TinyGL::createContext(_screenW, _screenH, g_system->getScreenFormat(), 512, true, ConfMan.getBool("dirtyrects"));

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglDisable(TGL_LIGHTING);
	tglDisable(TGL_TEXTURE_2D);
	tglEnable(TGL_DEPTH_TEST);
	_stippleEnabled = false;
	_lastColorSet0 = 0;
	_lastColorSet1 = 0;
	_stippleTexture = nullptr;
}

void TinyGLRenderer::setViewport(const Common::Rect &rect) {
	_viewport = rect;
	tglViewport(rect.left, g_system->getHeight() - rect.bottom, rect.width(), rect.height());
}

void TinyGLRenderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture) {
	const float sLeft = screenRect.left;
	const float sTop = screenRect.top;
	const float sWidth = screenRect.width();
	const float sHeight = screenRect.height();

	// HACK: tglBlit is not affected by the viewport, so we offset the draw coordinates here
	int viewPort[4];
	tglGetIntegerv(TGL_VIEWPORT, viewPort);

	TinyGL::BlitTransform transform(sLeft + viewPort[0], sTop + viewPort[1]);
	transform.sourceRectangle(textureRect.left, textureRect.top, sWidth, sHeight);
	tglBlit(((TinyGL2DTexture *)texture)->getBlitTexture(), transform);
}

void TinyGLRenderer::drawSkybox(Texture *texture, Math::Vector3d camera) {
	TinyGL3DTexture *glTexture = static_cast<TinyGL3DTexture *>(texture);
	tglDisable(TGL_DEPTH_TEST);
	tglEnable(TGL_TEXTURE_2D);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_REPEAT);

	tglBindTexture(TGL_TEXTURE_2D, glTexture->_id);
	tglColor4f(1.f, 1.f, 1.f, 1.f);
	tglVertexPointer(3, TGL_FLOAT, 0, _skyVertices);
	tglNormalPointer(TGL_FLOAT, 0, _skyNormals);
	if (texture->_width == 1008)
		tglTexCoordPointer(2, TGL_FLOAT, 0, _skyUvs1008);
	else if (texture->_width == 128)
		tglTexCoordPointer(2, TGL_FLOAT, 0, _skyUvs128);
	else
		error("Unsupported skybox texture width %d", glTexture->_width);

	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
	tglEnableClientState(TGL_NORMAL_ARRAY);

	tglPolygonMode(TGL_BACK, TGL_FILL);

	tglPushMatrix();
	{
		tglTranslatef(camera.x(), camera.y(), camera.z());
		tglDrawArrays(TGL_QUADS, 0, 16);
	}
	tglPopMatrix();

	tglDisableClientState(TGL_NORMAL_ARRAY);
	tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);
	tglDisableClientState(TGL_VERTEX_ARRAY);

	tglBindTexture(TGL_TEXTURE_2D, 0);
	tglDisable(TGL_TEXTURE_2D);
	tglEnable(TGL_DEPTH_TEST);
	tglFlush();
}

void TinyGLRenderer::updateProjectionMatrix(float fov, float aspectRatio, float nearClipPlane, float farClipPlane) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	float xmaxValue = nearClipPlane * tan(Math::deg2rad(fov) / 2);
	float ymaxValue = xmaxValue / aspectRatio;

	// Corrected glFrustum call
	tglFrustum(-xmaxValue, xmaxValue, -ymaxValue, ymaxValue, nearClipPlane, farClipPlane);
	tglScalef(-1.0f, 1.0f, 1.0f);

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
}

void TinyGLRenderer::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) {
	Math::Vector3d up_vec(0, 1, 0);

	Math::Matrix4 lookMatrix = Math::makeLookAtMatrix(pos, interest, up_vec);
	tglMultMatrixf(lookMatrix.getData());
	tglTranslatef(-pos.x(), -pos.y(), -pos.z());
}

void TinyGLRenderer::renderSensorShoot(byte color, const Math::Vector3d sensor, const Math::Vector3d player, const Common::Rect viewArea) {
	tglColor3ub(255, 255, 255);
	polygonOffset(true);
	tglEnableClientState(TGL_VERTEX_ARRAY);
	copyToVertexArray(0, player);
	copyToVertexArray(1, sensor);
	tglVertexPointer(3, TGL_FLOAT, 0, _verts);
	tglDrawArrays(TGL_LINES, 0, 2);
	tglDisableClientState(TGL_VERTEX_ARRAY);
	polygonOffset(false);
}

void TinyGLRenderer::renderPlayerShootBall(byte color, const Common::Point position, int frame, const Common::Rect viewArea) {
	/*uint8 r, g, b;

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrtho(0, _screenW, _screenH, 0, 0, 1);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderZX) {
		r = g = b = 255;
	} else {
		r = g = b = 255;
		tglEnable(TGL_BLEND);
		tglBlendFunc(TGL_ONE_MINUS_DST_COLOR, TGL_ZERO);
	}

	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);

	tglColor3ub(r, g, b);
	int triangleAmount = 20;
	float twicePi = (float)(2.0 * M_PI);
	float coef = (9 - frame) / 9.0;
	float radius = (1 - coef) * 4.0;

	Common::Point initial_position(viewArea.left + viewArea.width() / 2 + 2, viewArea.height() + viewArea.top);
	Common::Point ball_position = coef * position + (1 - coef) * initial_position;

	tglEnableClientState(TGL_VERTEX_ARRAY);
	copyToVertexArray(0, Math::Vector3d(ball_position.x, ball_position.y, 0));

	for(int i = 0; i <= triangleAmount; i++) {
		float x = ball_position.x + (radius * cos(i *  twicePi / triangleAmount));
		float y = ball_position.y + (radius * sin(i * twicePi / triangleAmount));
		copyToVertexArray(i + 1, Math::Vector3d(x, y, 0));
	}

	tglVertexPointer(3, TGL_FLOAT, 0, _verts);
	tglDrawArrays(TGL_TRIANGLE_FAN, 0, triangleAmount + 2);
	tglDisableClientState(TGL_VERTEX_ARRAY);

	tglDisable(TGL_BLEND);
	tglEnable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_TRUE);*/
}


void TinyGLRenderer::renderPlayerShootRay(byte color, const Common::Point position, const Common::Rect viewArea) {
	uint8 r, g, b;
	readFromPalette(color, r, g, b); // TODO: should use opposite color

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrthof(0, _screenW, _screenH, 0, 0, 1);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderZX) {
		r = g = b = 255;
	} else {
		r = g = b = 255;
		tglEnable(TGL_BLEND);
		tglBlendFunc(TGL_ONE_MINUS_DST_COLOR, TGL_ZERO);
	}

	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);

	tglColor3ub(r, g, b);

	int viewPort[4];
	tglGetIntegerv(TGL_VIEWPORT, viewPort);

	tglEnableClientState(TGL_VERTEX_ARRAY);
	copyToVertexArray(0, Math::Vector3d(viewArea.left, viewArea.height() + viewArea.top, 0));
	copyToVertexArray(1, Math::Vector3d(position.x, position.y, 0));
	copyToVertexArray(2, Math::Vector3d(viewArea.left, viewArea.height() + viewArea.top + 3, 0));
	copyToVertexArray(3, Math::Vector3d(position.x, position.y, 0));

	copyToVertexArray(4, Math::Vector3d(viewArea.right, viewArea.height() + viewArea.top, 0));
	copyToVertexArray(5, Math::Vector3d(position.x, position.y, 0));
	copyToVertexArray(6, Math::Vector3d(viewArea.right, viewArea.height() + viewArea.top + 3, 0));
	copyToVertexArray(7, Math::Vector3d(position.x, position.y, 0));

	tglVertexPointer(3, TGL_FLOAT, 0, _verts);
	tglDrawArrays(TGL_LINES, 0, 8);
	tglDisableClientState(TGL_VERTEX_ARRAY);

	tglDisable(TGL_BLEND);
	tglEnable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_TRUE);
}

void TinyGLRenderer::renderCrossair(const Common::Point crossairPosition) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrtho(0, _screenW, _screenH, 0, 0, 1);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_ONE_MINUS_DST_COLOR, TGL_ZERO);

	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);

	useColor(255, 255, 255);

	tglEnableClientState(TGL_VERTEX_ARRAY);
	copyToVertexArray(0, Math::Vector3d(crossairPosition.x - 3, crossairPosition.y, 0));
	copyToVertexArray(1, Math::Vector3d(crossairPosition.x - 1, crossairPosition.y, 0));

	copyToVertexArray(2, Math::Vector3d(crossairPosition.x + 2, crossairPosition.y, 0));
	copyToVertexArray(3, Math::Vector3d(crossairPosition.x + 4, crossairPosition.y, 0));

	copyToVertexArray(4, Math::Vector3d(crossairPosition.x, crossairPosition.y - 3, 0));
	copyToVertexArray(5, Math::Vector3d(crossairPosition.x, crossairPosition.y - 1, 0));

	copyToVertexArray(6, Math::Vector3d(crossairPosition.x, crossairPosition.y + 2, 0));
	copyToVertexArray(7, Math::Vector3d(crossairPosition.x, crossairPosition.y + 4, 0));

	tglVertexPointer(3, TGL_FLOAT, 0, _verts);
	tglDrawArrays(TGL_LINES, 0, 8);
	tglDisableClientState(TGL_VERTEX_ARRAY);

	tglDisable(TGL_BLEND);
	tglEnable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_TRUE);
}

void TinyGLRenderer::setStippleData(byte *data) {
	if (!data) {
		_stippleTexture = nullptr;
		assert(_stippleEnabled == false);
		_variableStippleArray = nullptr;
		return;
	}
	if (_stippleTextureCache.contains(uint64(data))) {

	}
	assert(_stippleTextureCache.size() <= 16);

	_variableStippleArray = data;
}

void TinyGLRenderer::useStipple(bool enabled) {
	_stippleEnabled = enabled;

	if (enabled) {
		assert(_variableStippleArray);
	} else {
		_stippleTexture = nullptr;
	}
}

void TinyGLRenderer::renderFace(const Common::Array<Math::Vector3d> &vertices) {
	assert(vertices.size() >= 2);
	const Math::Vector3d &v0 = vertices[0];

	if (vertices.size() == 2) {
		const Math::Vector3d &v1 = vertices[1];
		if (v0 == v1)
			return;

		tglEnableClientState(TGL_VERTEX_ARRAY);
		copyToVertexArray(0, v0);
		copyToVertexArray(1, v1);
		tglVertexPointer(3, TGL_FLOAT, 0, _verts);
		tglDrawArrays(TGL_LINES, 0, 2);
		tglDisableClientState(TGL_VERTEX_ARRAY);
		return;
	}

	if (_stippleEnabled) {
		if (_stippleTextureCache.contains(uint64(_variableStippleArray))) {
			_stippleTexture = _stippleTextureCache[uint64(_variableStippleArray)];
		} else {
			_stippleTexture = new TinyGL3DTexture(_variableStippleArray, _lastColorSet0, _lastColorSet1);
			_stippleTextureCache[uint64(_variableStippleArray)] = _stippleTexture;
		}
	} else if (_variableStippleArray)
		return; // We are in the middle of a stipple rendering operation, so we should skip this face

	tglEnableClientState(TGL_VERTEX_ARRAY);
	uint vi = 0;
	for (uint i = 1; i < vertices.size() - 1; i++) { // no underflow since vertices.size() > 2
		const Math::Vector3d &v1 = vertices[i];
		const Math::Vector3d &v2 = vertices[i + 1];
		vi = 3 * (i - 1); // no underflow since i >= 1
		copyToVertexArray(vi + 0, v0);
		copyToVertexArray(vi + 1, v1);
		copyToVertexArray(vi + 2, v2);
	}
	tglVertexPointer(3, TGL_FLOAT, 0, _verts);

	if (_stippleEnabled) {
		tglClear(TGL_STENCIL_BUFFER_BIT);
		tglEnable(TGL_STENCIL_TEST);
		tglStencilFunc(TGL_ALWAYS, 1, 0xFF);        // Always pass stencil test
		tglStencilOp(TGL_KEEP, TGL_KEEP, TGL_REPLACE); // Replace stencil buffer where drawn
		tglEnable(TGL_DEPTH_TEST);
		tglDepthMask(TGL_TRUE);
		tglColorMask(TGL_FALSE, TGL_FALSE, TGL_FALSE, TGL_FALSE);
	}

	tglDrawArrays(TGL_TRIANGLES, 0, vi + 3);
	tglDisableClientState(TGL_VERTEX_ARRAY);

	if (_stippleEnabled) {
		tglColorMask(TGL_TRUE, TGL_TRUE, TGL_TRUE, TGL_TRUE);
		tglStencilFunc(TGL_EQUAL, 1, 0xFF); // Only render where stencil value is 1
		tglStencilOp(TGL_KEEP, TGL_KEEP, TGL_KEEP); // Don't change stencil buffer

		tglMatrixMode(TGL_PROJECTION);
		tglPushMatrix();

		tglLoadIdentity();
		tglOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0); // Orthographic projection

		tglScalef(1, 1, 1);
		tglMatrixMode(TGL_MODELVIEW);
		tglPushMatrix();
		tglLoadIdentity();

		tglEnable(TGL_TEXTURE_2D);
		tglBindTexture(TGL_TEXTURE_2D, _stippleTexture->_id);
		tglColor4f(1.f, 1.f, 1.f, 1.f);
		tglDepthMask(TGL_FALSE);

		tglBegin(TGL_QUADS);
		tglTexCoord2f(0.0f, 0.0f); tglVertex2f(0.0f, 0.0f);
		tglTexCoord2f(1.0, 0.0f); tglVertex2f(1.0, 0.0f);
		tglTexCoord2f(1.0, 1.0); tglVertex2f(1.0, 1.0);
		tglTexCoord2f(0.0f, 1.0); tglVertex2f(0.0f, 1.0);
		tglEnd();

		tglDepthMask(TGL_TRUE);
		tglDisable(TGL_STENCIL_TEST);
		tglDisable(TGL_TEXTURE_2D);
		tglDisable(TGL_TEXTURE);
		tglFlush();
		tglBindTexture(TGL_TEXTURE_2D, 0);

		tglMatrixMode(TGL_PROJECTION);
		tglPopMatrix();

		tglMatrixMode(TGL_MODELVIEW);
		tglPopMatrix();
	}
}

void TinyGLRenderer::drawCelestialBody(Math::Vector3d position, float radius, byte color) {
	/*uint8 r1, g1, b1, r2, g2, b2;
	byte *stipple = nullptr;
	getRGBAt(color, 0, r1, g1, b1, r2, g2, b2, stipple);

	int triangleAmount = 20;
	float twicePi = (float)(2.0 * M_PI);*/

	// Quick billboard effect inspired from this code:
	// http://www.lighthouse3d.com/opengl/billboarding/index.php?billCheat
	/*glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	GLfloat m[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	for(int i = 1; i < 4; i++)
		for(int j = 0; j < 4; j++ ) {
			if (i == 2)
				continue;
			if (i == j)
				m[i*4 + j] = 1.0;
			else
				m[i*4 + j] = 0.0;
		}

	glLoadMatrixf(m);*/
	/*tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);

	setStippleData(stipple);
	useColor(r1, g1, b1);

	tglEnableClientState(TGL_VERTEX_ARRAY);
	copyToVertexArray(0, position);
	float adj = 1.25; // Perspective correction

	for(int i = 0; i <= triangleAmount; i++) {
		copyToVertexArray(i + 1,
			Math::Vector3d(position.x(), position.y() + (radius * cos(i *  twicePi / triangleAmount)),
						position.z() + (adj * radius * sin(i * twicePi / triangleAmount)))
		);
	}

	tglVertexPointer(3, TGL_FLOAT, 0, _verts);
	tglDrawArrays(TGL_TRIANGLE_FAN, 0, triangleAmount + 2);
	tglDisableClientState(TGL_VERTEX_ARRAY);

	if (r1 != r2 || g1 != g2 || b1 != b2) {
		useStipple(true);
		useColor(r2, g2, b2);

		tglEnableClientState(TGL_VERTEX_ARRAY);
		copyToVertexArray(0, position);

		for(int i = 0; i <= triangleAmount; i++) {
			copyToVertexArray(i + 1,
				Math::Vector3d(position.x(), position.y() + (radius * cos(i *  twicePi / triangleAmount)),
							position.z() + (adj * radius * sin(i * twicePi / triangleAmount)))
			);
		}

		tglVertexPointer(3, TGL_FLOAT, 0, _verts);
		tglDrawArrays(TGL_TRIANGLE_FAN, 0, triangleAmount + 2);
		tglDisableClientState(TGL_VERTEX_ARRAY);

		useStipple(false);
	}

	tglEnable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_TRUE);
	//tglPopMatrix();*/
}

void TinyGLRenderer::depthTesting(bool enabled) {
	if (enabled) {
		tglClear(TGL_DEPTH_BUFFER_BIT);
		tglEnable(TGL_DEPTH_TEST);
	} else {
		tglDisable(TGL_DEPTH_TEST);
	}
}

void TinyGLRenderer::polygonOffset(bool enabled) {
	if (enabled) {
		tglEnable(TGL_POLYGON_OFFSET_FILL);
		tglPolygonOffset(-1.0f, 1.0f);
	} else {
		tglPolygonOffset(0, 0);
		tglDisable(TGL_POLYGON_OFFSET_FILL);
	}
}

void TinyGLRenderer::useColor(uint8 r, uint8 g, uint8 b) {
	_lastColorSet1 = _lastColorSet0;
	_lastColorSet0 = _texturePixelFormat.RGBToColor(r, g, b);
	tglColor3ub(r, g, b);
}

void TinyGLRenderer::clear(uint8 r, uint8 g, uint8 b, bool ignoreViewport) {
	tglClear(TGL_DEPTH_BUFFER_BIT | TGL_STENCIL_BITS);
	if (ignoreViewport) {
		tglClearColor(r / 255., g / 255., b / 255., 1.0);
		tglClear(TGL_COLOR_BUFFER_BIT);
	} else {
		// Disable viewport
		tglViewport(0, 0, g_system->getWidth(), g_system->getHeight());
		useColor(r, g, b);

		tglMatrixMode(TGL_PROJECTION);
		tglPushMatrix();
		tglLoadIdentity();

		tglOrtho(0, _screenW, _screenH, 0, 0, 1);
		tglMatrixMode(TGL_MODELVIEW);
		tglPushMatrix();
		tglLoadIdentity();

		tglDisable(TGL_DEPTH_TEST);
		tglDepthMask(TGL_FALSE);

		tglEnableClientState(TGL_VERTEX_ARRAY);
		copyToVertexArray(0, Math::Vector3d(_viewport.left, _viewport.top, 0));
		copyToVertexArray(1, Math::Vector3d(_viewport.left, _viewport.bottom, 0));
		copyToVertexArray(2, Math::Vector3d(_viewport.right, _viewport.bottom, 0));

		copyToVertexArray(3, Math::Vector3d(_viewport.left, _viewport.top, 0));
		copyToVertexArray(4, Math::Vector3d(_viewport.right, _viewport.top, 0));
		copyToVertexArray(5, Math::Vector3d(_viewport.right, _viewport.bottom, 0));

		tglVertexPointer(3, TGL_FLOAT, 0, _verts);
		tglDrawArrays(TGL_TRIANGLES, 0, 6);
		tglDisableClientState(TGL_VERTEX_ARRAY);

		tglEnable(TGL_DEPTH_TEST);
		tglDepthMask(TGL_TRUE);

		tglPopMatrix();
		tglMatrixMode(TGL_PROJECTION);
		tglPopMatrix();

		// Restore viewport
		tglViewport(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height());
	}
}

void TinyGLRenderer::drawFloor(uint8 color) {
	uint8 r1, g1, b1, r2, g2, b2;
	byte *stipple = nullptr;
	assert(getRGBAt(color, 0, r1, g1, b1, r2, g2, b2, stipple)); // TODO: move check inside this function
	tglColor3ub(r1, g1, b1);

	tglEnableClientState(TGL_VERTEX_ARRAY);
	copyToVertexArray(0, Math::Vector3d(-100000.0, 0.0, -100000.0));
	copyToVertexArray(1, Math::Vector3d(100000.0, 0.0, -100000.0));
	copyToVertexArray(2, Math::Vector3d(100000.0, 0.0, 100000.0));
	copyToVertexArray(3, Math::Vector3d(-100000.0, 0.0, 100000.0));
	tglVertexPointer(3, TGL_FLOAT, 0, _verts);
	tglDrawArrays(TGL_QUADS, 0, 4);
	tglDisableClientState(TGL_VERTEX_ARRAY);
}

void TinyGLRenderer::flipBuffer() {
	Common::List<Common::Rect> dirtyAreas;
	TinyGL::presentBuffer(dirtyAreas);

	Graphics::Surface glBuffer;
	TinyGL::getSurfaceRef(glBuffer);

	if (!dirtyAreas.empty()) {
		for (auto &it : dirtyAreas) {
			g_system->copyRectToScreen(glBuffer.getBasePtr(it.left, it.top), glBuffer.pitch,
									   it.left, it.top, it.width(), it.height());
		}
	}
}

Graphics::Surface *TinyGLRenderer::getScreenshot() {
	Graphics::Surface glBuffer;
	TinyGL::getSurfaceRef(glBuffer);

	Graphics::Surface *s = new Graphics::Surface();
	s->create(_screenW, _screenH, getRGBAPixelFormat());
	s->copyFrom(glBuffer);

	return s;
}

} // End of namespace Freescape
