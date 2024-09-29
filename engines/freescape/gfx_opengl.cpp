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
#include "math/glmath.h"

#include "freescape/objects/object.h"
#include "freescape/gfx_opengl.h"
#include "freescape/gfx_opengl_texture.h"

#ifdef USE_OPENGL_GAME

namespace Freescape {

Renderer *CreateGfxOpenGL(int screenW, int screenH, Common::RenderMode renderMode, bool authenticGraphics) {
	return new OpenGLRenderer(screenW, screenH, renderMode, authenticGraphics);
}

OpenGLRenderer::OpenGLRenderer(int screenW, int screenH, Common::RenderMode renderMode, bool authenticGraphics) : Renderer(screenW, screenH, renderMode, authenticGraphics) {
	_verts = (Vertex *)malloc(sizeof(Vertex) * kVertexArraySize);
	_coords = (Coord *)malloc(sizeof(Coord) * kCoordsArraySize);
	_texturePixelFormat = OpenGLTexture::getRGBAPixelFormat();
	_isAccelerated = true;
	_variableStippleArray = nullptr;
}

OpenGLRenderer::~OpenGLRenderer() {
	free(_verts);
	free(_coords);
}

Texture *OpenGLRenderer::createTexture(const Graphics::Surface *surface) {
	return new OpenGLTexture(surface);
}

void OpenGLRenderer::freeTexture(Texture *texture) {
	delete texture;
}

Common::Point OpenGLRenderer::nativeResolution() {
	GLint vect[4];
	glGetIntegerv(GL_VIEWPORT, vect);
	return Common::Point(vect[2], vect[3]);
}

void OpenGLRenderer::init() {

	computeScreenViewport();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	setViewport(_viewport);
	glEnable(GL_DEPTH_CLAMP);

	scaleStipplePattern(_defaultStippleArray, _stipples[15]);
	memcpy(_defaultStippleArray, _stipples[15], 128);
}

void OpenGLRenderer::setViewport(const Common::Rect &rect) {
	_viewport = Common::Rect(
					_screenViewport.width() * rect.width() / _screenW,
					_screenViewport.height() * rect.height() / _screenH
					);

	_viewport.translate(
					_screenViewport.left + _screenViewport.width() * rect.left / _screenW,
					_screenViewport.top + _screenViewport.height() * rect.top / _screenH
					);

	_unscaledViewport = rect;
	glViewport(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height());
	glScissor(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height());
}

void OpenGLRenderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture) {
	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

	const float tLeft = textureRect.left / (float)glTexture->_internalWidth;
	const float tWidth = textureRect.width() / (float)glTexture->_internalWidth;
	const float tTop = textureRect.top / (float)glTexture->_internalHeight;
	const float tHeight = textureRect.height() / (float)glTexture->_internalHeight;

	float sLeft = screenRect.left;
	float sTop = screenRect.top;
	float sRight = sLeft + screenRect.width();
	float sBottom = sTop + screenRect.height();

	SWAP(sTop, sBottom);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenW, 0, _screenH, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glDepthMask(GL_FALSE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	copyToVertexArray(0, Math::Vector3d(sLeft + 0, sBottom, 1.0f));
	copyToVertexArray(1, Math::Vector3d(sRight, sBottom, 1.0f));
	copyToVertexArray(2, Math::Vector3d(sLeft + 0, sTop + 0, 1.0f));
	copyToVertexArray(3, Math::Vector3d(sRight, sTop + 0, 1.0));
	glVertexPointer(3, GL_FLOAT, 0, _verts);

	copyToCoordArray(0, Math::Vector2d(tLeft, tTop + tHeight));
	copyToCoordArray(1, Math::Vector2d(tLeft + tWidth, tTop + tHeight));
	copyToCoordArray(2, Math::Vector2d(tLeft, tTop));
	copyToCoordArray(3, Math::Vector2d(tLeft + tWidth, tTop));
	glTexCoordPointer(2, GL_FLOAT, 0, _coords);

	glBindTexture(GL_TEXTURE_2D, glTexture->_id);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, 0); // Bind the default (empty) texture to avoid darker colors!
	glFlush();
}

void OpenGLRenderer::drawSkybox(Texture *texture, Math::Vector3d camera) {
	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, glTexture->_id);
	glVertexPointer(3, GL_FLOAT, 0, _skyVertices);
	glNormalPointer(GL_FLOAT, 0, _skyNormals);
	if (texture->_width == 1008)
		glTexCoordPointer(2, GL_FLOAT, 0, _skyUvs1008);
	else if (texture->_width == 128)
		glTexCoordPointer(2, GL_FLOAT, 0, _skyUvs128);
	else
		error("Unsupported skybox texture width %d", texture->_width);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glPolygonMode(GL_BACK, GL_FILL);

	glPushMatrix();
	{
		glTranslatef(camera.x(), camera.y(), camera.z());
		glDrawArrays(GL_QUADS, 0, 16);
	}
	glPopMatrix();

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glFlush();
}

void OpenGLRenderer::updateProjectionMatrix(float fov, float yminValue, float ymaxValue, float nearClipPlane, float farClipPlane) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Determining xmaxValue and ymaxValue still needs some work for matching the 3D view in freescape games
	/*float aspectRatio = _screenW / (float)_screenH;

	float xmaxValue = nearClipPlane * tan(Common::deg2rad(fov) / 2);
	float ymaxValue = xmaxValue / aspectRatio;
	// debug("max values: %f %f", xmaxValue, ymaxValue);

	glFrustum(xmaxValue, -xmaxValue, -ymaxValue, ymaxValue, nearClipPlane, farClipPlane);*/
	glFrustum(1.5, -1.5, yminValue, ymaxValue, nearClipPlane, farClipPlane);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OpenGLRenderer::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) {
	Math::Vector3d up_vec(0, 1, 0);

	Math::Matrix4 lookMatrix = Math::makeLookAtMatrix(pos, interest, up_vec);
	glMultMatrixf(lookMatrix.getData());
	glTranslatef(-pos.x(), -pos.y(), -pos.z());
	glTranslatef(_shakeOffset.x, _shakeOffset.y, 0);
}

void OpenGLRenderer::renderSensorShoot(byte color, const Math::Vector3d sensor, const Math::Vector3d target, const Common::Rect viewArea) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	glColor3ub(255, 255, 255);

	glLineWidth(20);
	glEnableClientState(GL_VERTEX_ARRAY);
	copyToVertexArray(0, sensor);
	copyToVertexArray(1, target);
	glVertexPointer(3, GL_FLOAT, 0, _verts);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableClientState(GL_VERTEX_ARRAY);
	glLineWidth(1);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void OpenGLRenderer::renderCrossair(const Common::Point crossairPosition) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenW, _screenH, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	useColor(255, 255, 255);

	glLineWidth(MAX(2, g_system->getWidth() / 192)); // It will not work in every OpenGL implementation since the
					 // spec doesn't require support for line widths other than 1
	glEnableClientState(GL_VERTEX_ARRAY);
	copyToVertexArray(0, Math::Vector3d(crossairPosition.x - 3, crossairPosition.y, 0));
	copyToVertexArray(1, Math::Vector3d(crossairPosition.x - 1, crossairPosition.y, 0));

	copyToVertexArray(2, Math::Vector3d(crossairPosition.x + 1, crossairPosition.y, 0));
	copyToVertexArray(3, Math::Vector3d(crossairPosition.x + 3, crossairPosition.y, 0));

	copyToVertexArray(4, Math::Vector3d(crossairPosition.x, crossairPosition.y - 3, 0));
	copyToVertexArray(5, Math::Vector3d(crossairPosition.x, crossairPosition.y - 1, 0));

	copyToVertexArray(6, Math::Vector3d(crossairPosition.x, crossairPosition.y + 1, 0));
	copyToVertexArray(7, Math::Vector3d(crossairPosition.x, crossairPosition.y + 3, 0));

	glVertexPointer(3, GL_FLOAT, 0, _verts);
	glDrawArrays(GL_LINES, 0, 8);
	glDisableClientState(GL_VERTEX_ARRAY);
	glLineWidth(1);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void OpenGLRenderer::renderPlayerShootRay(byte color, const Common::Point position, const Common::Rect viewArea) {
	uint8 r, g, b;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenW, _screenH, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderZX) {
		r = g = b = 255;
	} else {
		r = g = b = 255;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glColor3ub(r, g, b);

	glLineWidth(5); // It will not work in every OpenGL implementation since the
					 // spec doesn't require support for line widths other than 1
	glEnableClientState(GL_VERTEX_ARRAY);
	copyToVertexArray(0, Math::Vector3d(viewArea.left, viewArea.height() + viewArea.top, 0));
	copyToVertexArray(1, Math::Vector3d(position.x, position.y, 0));
	copyToVertexArray(2, Math::Vector3d(viewArea.left, viewArea.height() + viewArea.top + 3, 0));
	copyToVertexArray(3, Math::Vector3d(position.x, position.y, 0));

	copyToVertexArray(4, Math::Vector3d(viewArea.right, viewArea.height() + viewArea.top, 0));
	copyToVertexArray(5, Math::Vector3d(position.x, position.y, 0));
	copyToVertexArray(6, Math::Vector3d(viewArea.right, viewArea.height() + viewArea.top + 3, 0));
	copyToVertexArray(7, Math::Vector3d(position.x, position.y, 0));

	glVertexPointer(3, GL_FLOAT, 0, _verts);
	glDrawArrays(GL_LINES, 0, 8);
	glDisableClientState(GL_VERTEX_ARRAY);
	glLineWidth(1);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void OpenGLRenderer::drawCelestialBody(Math::Vector3d position, float radius, byte color) {
	uint8 r1, g1, b1, r2, g2, b2;
	byte *stipple = nullptr;
	getRGBAt(color, 0, r1, g1, b1, r2, g2, b2, stipple);

	int triangleAmount = 20;
	float twicePi = (float)(2.0 * M_PI);

	// Quick billboard effect inspired from this code:
	// http://www.lighthouse3d.com/opengl/billboarding/index.php?billCheat
	glMatrixMode(GL_MODELVIEW);
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

	glLoadMatrixf(m);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	setStippleData(stipple);
	useColor(r1, g1, b1);

	glEnableClientState(GL_VERTEX_ARRAY);
	copyToVertexArray(0, position);
	float adj = 1.25; // Perspective correction

	for(int i = 0; i <= triangleAmount; i++) {
		copyToVertexArray(i + 1,
			Math::Vector3d(position.x(), position.y() + (radius * cos(i *  twicePi / triangleAmount)),
						position.z() + (adj * radius * sin(i * twicePi / triangleAmount)))
		);
	}

	glVertexPointer(3, GL_FLOAT, 0, _verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, triangleAmount + 2);
	glDisableClientState(GL_VERTEX_ARRAY);

	if (r1 != r2 || g1 != g2 || b1 != b2) {
		useStipple(true);
		useColor(r2, g2, b2);

		glEnableClientState(GL_VERTEX_ARRAY);
		copyToVertexArray(0, position);

		for(int i = 0; i <= triangleAmount; i++) {
			copyToVertexArray(i + 1,
				Math::Vector3d(position.x(), position.y() + (radius * cos(i *  twicePi / triangleAmount)),
							position.z() + (adj * radius * sin(i * twicePi / triangleAmount)))
			);
		}

		glVertexPointer(3, GL_FLOAT, 0, _verts);
		glDrawArrays(GL_TRIANGLE_FAN, 0, triangleAmount + 2);
		glDisableClientState(GL_VERTEX_ARRAY);

		useStipple(false);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glPopMatrix();
}

void OpenGLRenderer::renderPlayerShootBall(byte color, const Common::Point position, int frame, const Common::Rect viewArea) {
	uint8 r, g, b;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenW, _screenH, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderZX) {
		r = g = b = 255;
	} else {
		r = g = b = 255;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glColor3ub(r, g, b);
	int triangleAmount = 20;
	float twicePi = (float)(2.0 * M_PI);
	float coef = (9 - frame) / 9.0;
	float radius = (1 - coef) * 4.0;

	Common::Point initial_position(viewArea.left + viewArea.width() / 2 + 2, viewArea.height() + viewArea.top);
	Common::Point ball_position = coef * position + (1 - coef) * initial_position;

	glEnableClientState(GL_VERTEX_ARRAY);
	copyToVertexArray(0, Math::Vector3d(ball_position.x, ball_position.y, 0));

	for(int i = 0; i <= triangleAmount; i++) {
		float x = ball_position.x + (radius * cos(i *  twicePi / triangleAmount));
		float y = ball_position.y + (radius * sin(i * twicePi / triangleAmount));
		copyToVertexArray(i + 1, Math::Vector3d(x, y, 0));
	}

	glVertexPointer(3, GL_FLOAT, 0, _verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, triangleAmount + 2);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}


void OpenGLRenderer::renderFace(const Common::Array<Math::Vector3d> &vertices) {
	assert(vertices.size() >= 2);
	const Math::Vector3d &v0 = vertices[0];

	if (vertices.size() == 2) {
		const Math::Vector3d &v1 = vertices[1];
		if (v0 == v1)
			return;

		glEnableClientState(GL_VERTEX_ARRAY);
		copyToVertexArray(0, v0);
		copyToVertexArray(1, v1);
		glVertexPointer(3, GL_FLOAT, 0, _verts);
		glLineWidth(MAX(1, g_system->getWidth() / 192));
		glDrawArrays(GL_LINES, 0, 2);
		glLineWidth(1);
		glDisableClientState(GL_VERTEX_ARRAY);
		return;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	uint vi = 0;
	for (uint i = 1; i < vertices.size() - 1; i++) { // no underflow since vertices.size() > 2
		const Math::Vector3d &v1 = vertices[i];
		const Math::Vector3d &v2 = vertices[i + 1];
		vi = 3 * (i - 1); // no underflow since i >= 1
		copyToVertexArray(vi + 0, v0);
		copyToVertexArray(vi + 1, v1);
		copyToVertexArray(vi + 2, v2);
	}
	glVertexPointer(3, GL_FLOAT, 0, _verts);
	glDrawArrays(GL_TRIANGLES, 0, vi + 3);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void OpenGLRenderer::depthTesting(bool enabled) {
	if (enabled) {
		// If we re-enable depth testing, we need to clear the depth buffer
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
}

void OpenGLRenderer::polygonOffset(bool enabled) {
	if (enabled) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-10.0f, 1.0f);
	} else {
		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void OpenGLRenderer::setStippleData(byte *data) {
	if (!data)
		data = _defaultStippleArray;

	_variableStippleArray = data;
	//for (int i = 0; i < 128; i++)
	//	_variableStippleArray[i] = data[(i / 16) % 4];
}

void OpenGLRenderer::useStipple(bool enabled) {
	if (enabled) {
		GLfloat factor = 0;
		glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(factor - 1.0f, -1.0f);
		glEnable(GL_POLYGON_STIPPLE);
		if (_renderMode == Common::kRenderZX    ||
			_renderMode == Common::kRenderCPC   ||
			_renderMode == Common::kRenderCGA   ||
			_renderMode == Common::kRenderHercG)
			glPolygonStipple(_variableStippleArray);
		else
			glPolygonStipple(_defaultStippleArray);
	} else {
		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_POLYGON_STIPPLE);
	}
}

void OpenGLRenderer::useColor(uint8 r, uint8 g, uint8 b) {
	glColor3ub(r, g, b);
}

void OpenGLRenderer::clear(uint8 r, uint8 g, uint8 b, bool ignoreViewport) {
	if (ignoreViewport)
		glDisable(GL_SCISSOR_TEST);
	glClearColor(r / 255., g / 255., b / 255., 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (ignoreViewport)
		glEnable(GL_SCISSOR_TEST);
}

void OpenGLRenderer::drawFloor(uint8 color) {
	uint8 r1, g1, b1, r2, g2, b2;
	byte *stipple;
	assert(getRGBAt(color, 0, r1, g1, b1, r2, g2, b2, stipple)); // TODO: move check inside this function
	glColor3ub(r1, g1, b1);

	glEnableClientState(GL_VERTEX_ARRAY);
	copyToVertexArray(0, Math::Vector3d(-100000.0, 0.0, -100000.0));
	copyToVertexArray(1, Math::Vector3d(100000.0, 0.0, -100000.0));
	copyToVertexArray(2, Math::Vector3d(100000.0, 0.0, 100000.0));
	copyToVertexArray(3, Math::Vector3d(-100000.0, 0.0, 100000.0));
	glVertexPointer(3, GL_FLOAT, 0, _verts);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void OpenGLRenderer::flipBuffer() {}

Graphics::Surface *OpenGLRenderer::getScreenshot() {
	Common::Rect screen = viewport();
	Graphics::Surface *s = new Graphics::Surface();
	s->create(screen.width(), screen.height(), OpenGLTexture::getRGBAPixelFormat());
	glReadPixels(screen.left, screen.top, screen.width(), screen.height(), GL_RGBA, GL_UNSIGNED_BYTE, s->getPixels());
	flipVertical(s);
	return s;
}

} // End of namespace Freescape

#endif
