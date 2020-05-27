/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/opengl/base_surface_opengl3d.h"
#include "graphics/opengl/system_headers.h"
#include "math/glmath.h"
#include "camera3d.h"

namespace Wintermute {
BaseRenderer *makeOpenGL3DRenderer(BaseGame *inGame) {
	return new BaseRenderOpenGL3D(inGame);
}

BaseRenderOpenGL3D::BaseRenderOpenGL3D(BaseGame *inGame)
    : _spriteBatchMode(false) {
}

BaseRenderOpenGL3D::~BaseRenderOpenGL3D() {
}

BaseImage *BaseRenderOpenGL3D::takeScreenshot() {
	warning("BaseRenderOpenGL3D::takeScreenshot not yet implemented");
	return nullptr;
}

bool BaseRenderOpenGL3D::saveScreenShot(const Common::String &filename, int sizeX, int sizeY) {
	warning("BaseRenderOpenGL3D::saveScreenshot not yet implemented");
	return true;
}

bool BaseRenderOpenGL3D::setViewport(int left, int top, int right, int bottom) {
	_viewportRect.setRect(left, top, right, bottom);
	glViewport(left, top, right - left, bottom - top);
	return true;
}

bool BaseRenderOpenGL3D::setViewport(Rect32 *rect) {
	return setViewport(rect->left, rect->top, rect->right, rect->bottom);
}

Rect32 BaseRenderOpenGL3D::getViewPort() {
	warning("BaseRenderOpenGL3D::getViewPort not yet implemented");
	return Rect32(0, 0, 0, 0);
}

void BaseRenderOpenGL3D::setWindowed(bool windowed) {
	warning("BaseRenderOpenGL3D::setWindowed not yet implemented");
}

Graphics::PixelFormat BaseRenderOpenGL3D::getPixelFormat() const {
	return OpenGL::Texture::getRGBAPixelFormat();
}

void BaseRenderOpenGL3D::fade(uint16 alpha) {
	fadeToColor(0, 0, 0, (byte)(255 - alpha));
}

void BaseRenderOpenGL3D::fadeToColor(byte r, byte g, byte b, byte a) {
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	setProjection2D();

	const int vertexSize = 16;
	byte vertices[4 * vertexSize];
	float *vertexCoords = reinterpret_cast<float *>(vertices);

	vertexCoords[0 * 4 + 1] = _viewportRect.left;
	vertexCoords[0 * 4 + 2] = _viewportRect.bottom;
	vertexCoords[0 * 4 + 3] = 0.0f;
	vertexCoords[1 * 4 + 1] = _viewportRect.left;
	vertexCoords[1 * 4 + 2] = _viewportRect.top;
	vertexCoords[1 * 4 + 3] = 0.0f;
	vertexCoords[2 * 4 + 1] = _viewportRect.right;
	vertexCoords[2 * 4 + 2] = _viewportRect.bottom;
	vertexCoords[2 * 4 + 3] = 0.0f;
	vertexCoords[3 * 4 + 1] = _viewportRect.right;
	vertexCoords[3 * 4 + 2] = _viewportRect.top;
	vertexCoords[3 * 4 + 3] = 0.0f;

	vertices[0 * vertexSize + 0] = r;
	vertices[0 * vertexSize + 1] = g;
	vertices[0 * vertexSize + 2] = b;
	vertices[0 * vertexSize + 3] = a;
	vertices[1 * vertexSize + 0] = r;
	vertices[1 * vertexSize + 1] = g;
	vertices[1 * vertexSize + 2] = b;
	vertices[1 * vertexSize + 3] = a;
	vertices[2 * vertexSize + 0] = r;
	vertices[2 * vertexSize + 1] = g;
	vertices[2 * vertexSize + 2] = b;
	vertices[2 * vertexSize + 3] = a;
	vertices[3 * vertexSize + 0] = r;
	vertices[3 * vertexSize + 1] = g;
	vertices[3 * vertexSize + 2] = b;
	vertices[3 * vertexSize + 3] = a;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, vertexSize, vertices + 4);
	glColorPointer(4, GL_UNSIGNED_BYTE, vertexSize, vertices);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	setup2D(true);
}

bool BaseRenderOpenGL3D::drawLine(int x1, int y1, int x2, int y2, uint32 color) {
	warning("BaseRenderOpenGL3D::drawLine not yet implemented");
	return true;
}

bool BaseRenderOpenGL3D::drawRect(int x1, int y1, int x2, int y2, uint32 color, int width) {
	warning("BaseRenderOpenGL3D::drawRect not yet implemented");
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::setProjection() {
	// is the viewport already set here?
	float viewportWidth = _viewportRect.right - _viewportRect.left;
	float viewportHeight = _viewportRect.bottom - _viewportRect.top;

	float horizontal_view_angle = M_PI * 0.5f;
	float aspect_ratio = float(viewportHeight) / float(viewportWidth);
	float near_plane = 1.0f;
	float far_plane = 1900.0f;
	float right = near_plane * tanf(horizontal_view_angle * 0.5f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-right, right, -right * aspect_ratio, right * aspect_ratio, near_plane, far_plane);
	glMatrixMode(GL_MODELVIEW);

	return true;
}

bool BaseRenderOpenGL3D::setProjection2D() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _viewportRect.width(), 0, _viewportRect.height(), -1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	return true;
}

void BaseRenderOpenGL3D::resetModelViewTransform() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

bool BaseRenderOpenGL3D::windowedBlt() {
	warning("BaseRenderOpenGL3D::windowedBlt not yet implemented");
	return true;
}

bool BaseRenderOpenGL3D::fill(byte r, byte g, byte b, Common::Rect *rect) {
	glClearColor(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	return true;
}

void Wintermute::BaseRenderOpenGL3D::onWindowChange() {
	warning("BaseRenderOpenGL3D::onWindowChange not yet implemented");
}

bool BaseRenderOpenGL3D::initRenderer(int width, int height, bool windowed) {
	_windowed = windowed;
	_width = width;
	_height = height;

	setViewport(0, 0, width, height);

	_active = true;
	// setup a proper state
	setup2D(true);
	return true;
}

bool Wintermute::BaseRenderOpenGL3D::flip() {
	g_system->updateScreen();
	return true;
}

bool BaseRenderOpenGL3D::indicatorFlip() {
	warning("BaseRenderOpenGL3D::indicatorFlip not yet implemented");
	return true;
}

bool BaseRenderOpenGL3D::forcedFlip() {
	warning("BaseRenderOpenGL3D::forcedFlip not yet implemented");
	return true;
}

void BaseRenderOpenGL3D::initLoop() {
	deleteRectList();
	setup2D();
}

bool BaseRenderOpenGL3D::setup2D(bool force) {
	if (_state3D || force) {
		_state3D = false;

		// some states are still missing here

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_STENCIL);
		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_FOG);
		glLightModeli(GL_LIGHT_MODEL_AMBIENT, 0);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_CCW);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glAlphaFunc(GL_GEQUAL, 0.0f);
		glPolygonMode(GL_FRONT, GL_FILL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE0);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS);

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);

		setProjection2D();
	}

	return true;
}

bool BaseRenderOpenGL3D::setup3D(Camera3D* camera, bool force) {
	if (!_state3D || force) {
		_state3D = true;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glAlphaFunc(GL_GEQUAL, 0x08);

		// no culling for the moment
		glDisable(GL_CULL_FACE);

		setProjection();

		Math::Matrix4 viewMatrix;
		camera->GetViewMatrix(&viewMatrix);
		glLoadMatrixf(viewMatrix.getData());
	}

	return true;
}

bool BaseRenderOpenGL3D::setupLines() {
	warning("BaseRenderOpenGL3D::setupLines not yet implemented");
	return true;
}

BaseSurface *Wintermute::BaseRenderOpenGL3D::createSurface() {
	return new BaseSurfaceOpenGL3D(nullptr, this);
}

void BaseRenderOpenGL3D::endSaveLoad() {
	warning("BaseRenderOpenGL3D::endLoad not yet implemented");
}

bool BaseRenderOpenGL3D::drawSprite(const OpenGL::Texture &tex, const Wintermute::Rect32 &rect,
                                                float zoomX, float zoomY, const Wintermute::Vector2 &pos,
                                                uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
                                                bool mirrorX, bool mirrorY) {
	Vector2 scale(zoomX / 100.0f, zoomY / 100.0f);
	return drawSpriteEx(tex, rect, pos, Vector2(0.0f, 0.0f), scale, 0.0f, color, alphaDisable, blendMode, mirrorX, mirrorY);
}

bool BaseRenderOpenGL3D::drawSpriteEx(const OpenGL::Texture &tex, const Wintermute::Rect32 &rect,
                                                  const Wintermute::Vector2 &pos, const Wintermute::Vector2 &rot, const Wintermute::Vector2 &scale,
                                                  float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
                                                  bool mirrorX, bool mirrorY) {
	// original wme has a batch mode for sprites, we ignore this for the moment

	// The ShaderSurfaceRenderer sets an array buffer which appearently conflicts with us
	// Reset it!
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	float width = (rect.right - rect.left) * scale.x;
	float height = (rect.bottom - rect.top) * scale.y;

	float texLeft = (float)rect.left / (float)tex.getWidth();
	float texTop = (float)rect.top / (float)tex.getHeight();
	float texRight = (float)rect.right / (float)tex.getWidth();
	float texBottom = (float)rect.bottom / (float)tex.getHeight();

	float offset = _viewportRect.height() / 2.0f;
	float corrected_y = (pos.y - offset) * -1.0f + offset;

	// to be implemented
	if (mirrorX) {
		warning("BaseRenderOpenGL3D::SpriteEx x mirroring is not yet implemented");
	}

	if (mirrorY) {
		warning("BaseRenderOpenGL3D::SpriteEx y mirroring is not yet implemented");
	}

	// provide space for 3d position coords, 2d texture coords and a 32 bit color value
	const int vertexSize = 24;
	byte vertices[vertexSize * 4] = {};

	float *vertexCoords = reinterpret_cast<float *>(vertices);

	// texture coords
	vertexCoords[0 * 6 + 0] = texLeft;
	vertexCoords[0 * 6 + 1] = texTop;
	vertexCoords[1 * 6 + 0] = texLeft;
	vertexCoords[1 * 6 + 1] = texBottom;
	vertexCoords[2 * 6 + 0] = texRight;
	vertexCoords[2 * 6 + 1] = texTop;
	vertexCoords[3 * 6 + 0] = texRight;
	vertexCoords[3 * 6 + 1] = texBottom;

	// position coords
	vertexCoords[0 * 6 + 3] = pos.x - 0.5f;
	vertexCoords[0 * 6 + 4] = corrected_y - 0.5f;
	vertexCoords[0 * 6 + 5] = -1.1f;
	vertexCoords[1 * 6 + 3] = pos.x - 0.5f;
	vertexCoords[1 * 6 + 4] = corrected_y - height - 0.5f;
	vertexCoords[1 * 6 + 5] = -1.1f;
	vertexCoords[2 * 6 + 3] = pos.x + width - 0.5f;
	vertexCoords[2 * 6 + 4] = corrected_y - 0.5f;
	vertexCoords[2 * 6 + 5] = -1.1f;
	vertexCoords[3 * 6 + 3] = pos.x + width - 0.5f;
	vertexCoords[3 * 6 + 4] = corrected_y - height - 0.5;
	vertexCoords[3 * 6 + 5] = -1.1f;

	// not exactly sure about the color format, but this seems to work
	byte a = RGBCOLGetA(color);
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);

	vertices[0 * vertexSize + 8 + 0] = r;
	vertices[0 * vertexSize + 8 + 1] = g;
	vertices[0 * vertexSize + 8 + 2] = b;
	vertices[0 * vertexSize + 8 + 3] = a;
	vertices[1 * vertexSize + 8 + 0] = r;
	vertices[1 * vertexSize + 8 + 1] = g;
	vertices[1 * vertexSize + 8 + 2] = b;
	vertices[1 * vertexSize + 8 + 3] = a;
	vertices[2 * vertexSize + 8 + 0] = r;
	vertices[2 * vertexSize + 8 + 1] = g;
	vertices[2 * vertexSize + 8 + 2] = b;
	vertices[2 * vertexSize + 8 + 3] = a;
	vertices[3 * vertexSize + 8 + 0] = r;
	vertices[3 * vertexSize + 8 + 2] = g;
	vertices[3 * vertexSize + 8 + 2] = b;
	vertices[3 * vertexSize + 8 + 3] = a;

	// transform vertices here if necessary, add offset

	if (alphaDisable) {
		glDisable(GL_ALPHA_TEST);
	}

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 24, vertices + 12);
	glColorPointer(4, GL_UNSIGNED_BYTE, 24, vertices + 8);
	glTexCoordPointer(2, GL_FLOAT, 24, vertices);

	glBindTexture(GL_TEXTURE_2D, tex.getTextureName());

	// we probably should do this in a vertex buffer anyways
	//glInterleavedArrays(GL_T2F_C4UB_V3F, 0, vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (alphaDisable) {
		glEnable(GL_ALPHA_TEST);
	}

	return true;
}

} // namespace Wintermute
