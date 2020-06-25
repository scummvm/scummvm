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
#include "engines/wintermute/base/gfx/opengl/camera3d.h"
#include "graphics/opengl/system_headers.h"
#include "math/glmath.h"

namespace Wintermute {
BaseRenderer *makeOpenGL3DRenderer(BaseGame *inGame) {
	return new BaseRenderOpenGL3D(inGame);
}

BaseRenderOpenGL3D::BaseRenderOpenGL3D(BaseGame *inGame)
	: BaseRenderer(inGame), _spriteBatchMode(false) {
}

BaseRenderOpenGL3D::~BaseRenderOpenGL3D() {
}

bool BaseRenderOpenGL3D::setAmbientLightColor(uint32 color) {
	byte a = RGBCOLGetA(color);
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);

	float value[] = { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, value);
	return true;
}

bool BaseRenderOpenGL3D::setDefaultAmbientLightColor() {
	setAmbientLightColor(0x00000000);
	return true;
}

void BaseRenderOpenGL3D::setSpriteBlendMode(Graphics::TSpriteBlendMode blendMode) {
	switch (blendMode) {
	case Graphics::BLEND_NORMAL:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;

	case Graphics::BLEND_ADDITIVE:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;

	case Graphics::BLEND_SUBTRACTIVE:
		// wme3d takes the color value here
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		break;

	default:
		error("BaseRenderOpenGL3D::setSpriteBlendMode unsupported blend mode %i", blendMode);
	}
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
	return _viewportRect;
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

bool BaseRenderOpenGL3D::setProjection() {
	// is the viewport already set here?
	float viewportWidth = _viewportRect.right - _viewportRect.left;
	float viewportHeight = _viewportRect.bottom - _viewportRect.top;

	float verticalViewAngle = _fov;
	float aspectRatio = float(viewportWidth) / float(viewportHeight);
	// same defaults as wme
	float nearPlane = 90.0f;
	float farPlane = 10000.0f;
	float top = nearPlane * tanf(verticalViewAngle * 0.5f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-top * aspectRatio, top * aspectRatio, -top, top, nearPlane, farPlane);
	glGetFloatv(GL_PROJECTION_MATRIX, _lastProjectionMatrix.getData());
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

void BaseRenderOpenGL3D::pushWorldTransform(const Math::Matrix4 &transform) {
	glPushMatrix();
	glMultMatrixf(transform.getData());
}

void BaseRenderOpenGL3D::popWorldTransform() {
	glPopMatrix();
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
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_FOG);
		glLightModeli(GL_LIGHT_MODEL_AMBIENT, 0);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glAlphaFunc(GL_GEQUAL, 0.0f);
		glPolygonMode(GL_FRONT, GL_FILL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE0);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);

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

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		_fov = camera->_fov;
		setProjection();

		Math::Matrix4 viewMatrix;
		camera->getViewMatrix(&viewMatrix);
		glMultMatrixf(viewMatrix.getData());
		glTranslatef(-camera->_position.x(), -camera->_position.y(), -camera->_position.z());
		glGetFloatv(GL_MODELVIEW_MATRIX, _lastViewMatrix.getData());
	}

	return true;
}

bool BaseRenderOpenGL3D::setupLines() {
	warning("BaseRenderOpenGL3D::setupLines not yet implemented");
	return true;
}

void BaseRenderOpenGL3D::project(const Math::Matrix4 &worldMatrix, const Math::Vector3d &point, int &x, int &y) {
	Math::Vector3d windowCoords;
	Math::Matrix4 modelMatrix = worldMatrix * _lastViewMatrix;
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	Math::gluMathProject(point, modelMatrix.getData(), _lastProjectionMatrix.getData(), viewport, windowCoords);
	x = windowCoords.x();
	// The Wintermute script code will expect a Direct3D viewport
	y = viewport[3] - windowCoords.y();
}

Math::Ray BaseRenderOpenGL3D::rayIntoScene(int x, int y) {
	Math::Vector3d direction((((2.0f * x) / _viewportRect.width()) - 1) / _lastProjectionMatrix(0, 0),
	                         -(((2.0f * y) / _viewportRect.height()) - 1) / _lastProjectionMatrix(1, 1),
	                         -1.0f);

	Math::Matrix4 m = _lastViewMatrix;
	m.inverse();
	m.transpose();
	m.transform(&direction, false);

	Math::Vector3d origin = m.getPosition();
	return Math::Ray(origin, direction);
}

BaseSurface *Wintermute::BaseRenderOpenGL3D::createSurface() {
	return new BaseSurfaceOpenGL3D(_gameRef, this);
}

void BaseRenderOpenGL3D::endSaveLoad() {
	warning("BaseRenderOpenGL3D::endLoad not yet implemented");
}

bool BaseRenderOpenGL3D::drawSprite(BaseSurfaceOpenGL3D &tex, const Wintermute::Rect32 &rect,
                                    float zoomX, float zoomY, const Wintermute::Vector2 &pos,
                                    uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
                                    bool mirrorX, bool mirrorY) {
	Vector2 scale(zoomX / 100.0f, zoomY / 100.0f);
	return drawSpriteEx(tex, rect, pos, Vector2(0.0f, 0.0f), scale, 0.0f, color, alphaDisable, blendMode, mirrorX, mirrorY);
}

#include "common/pack-start.h"

struct SpriteVertex {
	float u;
	float v;
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
	float x;
	float y;
	float z;
} PACKED_STRUCT;

#include "common/pack-end.h"

bool BaseRenderOpenGL3D::drawSpriteEx(BaseSurfaceOpenGL3D &tex, const Wintermute::Rect32 &rect,
                                      const Wintermute::Vector2 &pos, const Wintermute::Vector2 &rot, const Wintermute::Vector2 &scale,
                                      float angle, uint32 color, bool alphaDisable, Graphics::TSpriteBlendMode blendMode,
                                      bool mirrorX, bool mirrorY) {
	// original wme has a batch mode for sprites, we ignore this for the moment

	// The ShaderSurfaceRenderer sets an array buffer which appearently conflicts with us
	// Reset it!
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (_forceAlphaColor != 0) {
		color = _forceAlphaColor;
	}

	float width = (rect.right - rect.left) * scale.x;
	float height = (rect.bottom - rect.top) * scale.y;

	glBindTexture(GL_TEXTURE_2D, tex.getTextureName());

	// for sprites we clamp to the edge, to avoid line fragments at the edges
	// this is not done by wme, though
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// might as well provide getters for those
	int texWidth;
	int texHeight;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);

	float texLeft = (float)rect.left / (float)texWidth;
	float texTop = (float)rect.top / (float)texHeight;
	float texRight = (float)rect.right / (float)texWidth;
	float texBottom = (float)rect.bottom / (float)texHeight;

	float offset = _viewportRect.height() / 2.0f;
	float correctedYPos = (pos.y - offset) * -1.0f + offset;

	// to be implemented
	if (mirrorX) {
		warning("BaseRenderOpenGL3D::SpriteEx x mirroring is not yet implemented");
	}

	if (mirrorY) {
		warning("BaseRenderOpenGL3D::SpriteEx y mirroring is not yet implemented");
	}

	SpriteVertex vertices[4] = {};

	// texture coords
	vertices[0].u = texLeft;
	vertices[0].v = texTop;

	vertices[1].u = texLeft;
	vertices[1].v = texBottom;

	vertices[2].u = texRight;
	vertices[2].v = texTop;

	vertices[3].u = texRight;
	vertices[3].v = texBottom;

	// position coords
	vertices[0].x = pos.x - 0.5f;
	vertices[0].y = correctedYPos - 0.5f;
	vertices[0].z = -0.9f;

	vertices[1].x = pos.x - 0.5f;
	vertices[1].y = correctedYPos - height - 0.5f;
	vertices[1].z = -0.9f;

	vertices[2].x = pos.x + width - 0.5f;
	vertices[2].y = correctedYPos - 0.5f;
	vertices[2].z = -0.9f;

	vertices[3].x = pos.x + width - 0.5f;
	vertices[3].y = correctedYPos - height - 0.5;
	vertices[3].z = -0.9f;

	// not exactly sure about the color format, but this seems to work
	byte a = RGBCOLGetA(color);
	byte r = RGBCOLGetR(color);
	byte g = RGBCOLGetG(color);
	byte b = RGBCOLGetB(color);

	for (int i = 0; i < 4; ++i) {
		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;
		vertices[i].a = a;
	}

	// transform vertices here if necessary, add offset

	if (alphaDisable) {
		glDisable(GL_ALPHA_TEST);
	}

	setSpriteBlendMode(blendMode);

	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glInterleavedArrays(GL_T2F_C4UB_V3F, 0, vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (alphaDisable) {
		glEnable(GL_ALPHA_TEST);
	}

	return true;
}

} // namespace Wintermute
