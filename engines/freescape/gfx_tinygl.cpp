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

// Based on Phantasma code by Thomas Harte (2013)

#include "common/config-manager.h"
#include "common/math.h"
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

TinyGLRenderer::TinyGLRenderer(int screenW, int screenH, Common::RenderMode renderMode) : Renderer(screenW, screenH, renderMode) {
	_verts = (Vertex*) malloc(sizeof(Vertex) * 20);
	_texturePixelFormat = TinyGLTexture::getRGBAPixelFormat();
}

TinyGLRenderer::~TinyGLRenderer() {
	TinyGL::destroyContext();
	free(_verts);
}

Texture *TinyGLRenderer::createTexture(const Graphics::Surface *surface) {
	return new TinyGLTexture(surface);
}

void TinyGLRenderer::freeTexture(Texture *texture) {
	TinyGLTexture *glTexture = static_cast<TinyGLTexture *>(texture);
	delete glTexture;
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
}

void TinyGLRenderer::setViewport(const Common::Rect &rect) {
	tglViewport(rect.left, g_system->getHeight() - rect.bottom, rect.width(), rect.height());
}

void TinyGLRenderer::clear() {
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT);
	tglColor3f(1.0f, 1.0f, 1.0f);
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
	tglBlit(((TinyGLTexture *)texture)->getBlitTexture(), transform);
}

void TinyGLRenderer::updateProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	float aspectRatio = _screenW / (float)_screenH;

	float xmaxValue = nearClipPlane * tan(Common::deg2rad(fov) / 2);
	float ymaxValue = xmaxValue / aspectRatio;
	// debug("max values: %f %f", xmaxValue, ymaxValue);

	tglFrustum(xmaxValue, -xmaxValue, -ymaxValue, ymaxValue, nearClipPlane, farClipPlane);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
}

void TinyGLRenderer::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) {
	Math::Vector3d up_vec(0, 1, 0);

	Math::Matrix4 lookMatrix = Math::makeLookAtMatrix(pos, interest, up_vec);
	tglMultMatrixf(lookMatrix.getData());
	tglTranslatef(-pos.x(), -pos.y(), -pos.z());
}

void TinyGLRenderer::renderCrossair(byte color, const Common::Point position) {
	uint8 r, g, b;
	readFromPalette(color, r, g, b); // TODO: should use opposite color

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrtho(0, _screenW, _screenH, 0, 0, 1);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);

	tglColor3ub(r, g, b);

	tglBegin(TGL_LINES);
	tglVertex2f(position.x - 1, position.y);
	tglVertex2f(position.x + 3, position.y);

	tglVertex2f(position.x, position.y - 3);
	tglVertex2f(position.x, position.y + 3);
	tglEnd();

	tglDepthMask(TGL_TRUE);
	tglEnable(TGL_DEPTH_TEST);
}

void TinyGLRenderer::renderShoot(byte color, const Common::Point position) {
	uint8 r, g, b;
	readFromPalette(color, r, g, b); // TODO: should use opposite color

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrtho(0, _screenW, _screenH, 0, 0, 1);
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);

	tglColor3ub(r, g, b);

	int viewPort[4];
	tglGetIntegerv(TGL_VIEWPORT, viewPort);

	tglBegin(TGL_LINES);
	tglVertex2f(0, _screenH - 2);
	tglVertex2f(position.x, position.y);

	tglVertex2f(0, _screenH - 2);
	tglVertex2f(position.x, position.y);

	tglVertex2f(_screenW, _screenH - 2);
	tglVertex2f(position.x, position.y);

	tglVertex2f(_screenW, _screenH);
	tglVertex2f(position.x, position.y);

	tglEnd();

	tglEnable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_TRUE);
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
	tglDrawArrays(TGL_TRIANGLES, 0, vi + 3);
	tglDisableClientState(TGL_VERTEX_ARRAY);
}

void TinyGLRenderer::polygonOffset(bool enabled) {
	if (enabled) {
		tglEnable(TGL_POLYGON_OFFSET_FILL);
		tglPolygonOffset(-2.0f, 1.0f);
	} else {
		tglPolygonOffset(0, 0);
		tglDisable(TGL_POLYGON_OFFSET_FILL);
	}
}

void TinyGLRenderer::useColor(uint8 r, uint8 g, uint8 b) {
	tglColor3ub(r, g, b);
}

void TinyGLRenderer::setSkyColor(uint8 color) {
	uint8 r, g, b;
	assert(getRGBAt(color, r, g, b)); // TODO: move check inside this function
	tglClearColor(r / 255., g / 255., b / 255., 1.0);
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT);
}

void TinyGLRenderer::drawFloor(uint8 color) {
	uint8 r, g, b;
	assert(getRGBAt(color, r, g, b)); // TODO: move check inside this function
	tglColor3ub(r, g, b);

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

} // End of namespace Freescape
