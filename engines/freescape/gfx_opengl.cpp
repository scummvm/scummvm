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
#include "math/glmath.h"

#include "freescape/objects/object.h"
#include "freescape/gfx_opengl.h"
#include "freescape/gfx_opengl_texture.h"

namespace Freescape {

Renderer *CreateGfxOpenGL(OSystem *system, int screenW, int screenH, Common::RenderMode renderMode) {
	return new OpenGLRenderer(system, screenW, screenH, renderMode);
}

OpenGLRenderer::OpenGLRenderer(OSystem *system, int screenW, int screenH, Common::RenderMode renderMode) : Renderer(system, screenW, screenH, renderMode) {
	_verts = (Vertex*) malloc(sizeof(Vertex) * 20);
	_texturePixelFormat = OpenGLTexture::getRGBAPixelFormat();
}

OpenGLRenderer::~OpenGLRenderer() {
	free(_verts);
}

Texture *OpenGLRenderer::createTexture(const Graphics::Surface *surface) {
	return new OpenGLTexture(surface);
}

void OpenGLRenderer::freeTexture(Texture *texture) {
	delete texture;
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

void OpenGLRenderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
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

	glBindTexture(GL_TEXTURE_2D, glTexture->_id);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(tLeft, tTop + tHeight);
	glVertex3f(sLeft + 0, sBottom, 1.0f);

	glTexCoord2f(tLeft + tWidth, tTop + tHeight);
	glVertex3f(sRight, sBottom, 1.0f);

	glTexCoord2f(tLeft, tTop);
	glVertex3f(sLeft + 0, sTop + 0, 1.0f);

	glTexCoord2f(tLeft + tWidth, tTop);
	glVertex3f(sRight, sTop + 0, 1.0f);
	glEnd();

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glFlush();
}

void OpenGLRenderer::updateProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float aspectRatio = _screenW / (float)_screenH;

	float xmaxValue = nearClipPlane * tan(Common::deg2rad(fov) / 2);
	float ymaxValue = xmaxValue / aspectRatio;
	// debug("max values: %f %f", xmaxValue, ymaxValue);

	glFrustum(xmaxValue, -xmaxValue, -ymaxValue, ymaxValue, nearClipPlane, farClipPlane);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OpenGLRenderer::positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) {
	Math::Vector3d up_vec(0, 1, 0);

	Math::Matrix4 lookMatrix = Math::makeLookAtMatrix(pos, interest, up_vec);
	glMultMatrixf(lookMatrix.getData());
	glTranslatef(-pos.x(), -pos.y(), -pos.z());
}

void OpenGLRenderer::renderCrossair(byte color, const Common::Point position) {
	uint8 r, g, b;
	readFromPalette(color, r, g, b); // TODO: should use opposite color

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenW, _screenH, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glColor3ub(r, g, b);

	glLineWidth(15); // It will not work in every OpenGL implementation since the
					 // spec doesn't require support for line widths other than 1
	glBegin(GL_LINES);
	glVertex2f(position.x - 1, position.y);
	glVertex2f(position.x + 3, position.y);

	glVertex2f(position.x, position.y - 3);
	glVertex2f(position.x, position.y + 3);
	glEnd();
	glLineWidth(1);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::renderShoot(byte color, const Common::Point position) {
	uint8 r, g, b;
	readFromPalette(color, r, g, b); // TODO: should use opposite color

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, _screenW, _screenH, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glColor3ub(r, g, b);

	int viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	glLineWidth(10); // It will not work in every OpenGL implementation since the
					 // spec doesn't require support for line widths other than 1
	glBegin(GL_LINES);
	glVertex2f(0, _screenH - 2);
	glVertex2f(position.x, position.y);

	glVertex2f(0, _screenH - 2);
	glVertex2f(position.x, position.y);

	glVertex2f(_screenW, _screenH - 2);
	glVertex2f(position.x, position.y);

	glVertex2f(_screenW, _screenH);
	glVertex2f(position.x, position.y);

	glEnd();
	glLineWidth(1);

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
		glDrawArrays(GL_LINES, 0, 2);
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

void OpenGLRenderer::polygonOffset(bool enabled) {
	if (enabled) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-10.0f, 1.0f);
	} else {
		glPolygonOffset(0, 0);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void OpenGLRenderer::useColor(uint8 r, uint8 g, uint8 b) {
	glColor3ub(r, g, b);
}

void OpenGLRenderer::setSkyColor(uint8 color) {
	uint8 r, g, b;
	assert(getRGBAt(color, r, g, b)); // TODO: move check inside this function
	glClearColor(r / 255., g / 255., b / 255., 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::drawFloor(uint8 color) {
	uint8 r, g, b;
	assert(getRGBAt(color, r, g, b)); // TODO: move check inside this function
	glColor3ub(r, g, b);

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

} // End of namespace Freescape
