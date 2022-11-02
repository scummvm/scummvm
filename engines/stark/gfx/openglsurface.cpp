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

#include "engines/stark/gfx/openglsurface.h"
#include "engines/stark/gfx/texture.h"

#if defined(USE_OPENGL_GAME)

namespace Stark {
namespace Gfx {

static const GLfloat textCords[] = {
	// S   T
	0.0f, 0.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
};

OpenGLSurfaceRenderer::OpenGLSurfaceRenderer(OpenGLDriver *gfx) :
		SurfaceRenderer(),
		_gfx(gfx) {
}

OpenGLSurfaceRenderer::~OpenGLSurfaceRenderer() {
}

void OpenGLSurfaceRenderer::render(const Texture *texture, const Common::Point &dest) {
	render(texture, dest, texture->width(), texture->height());
}

void OpenGLSurfaceRenderer::render(const Texture *texture, const Common::Point &dest, uint width, uint height) {
	// Destination rectangle with given width and height
	_gfx->start2DMode();

	const Math::Vector2d surfaceVertices[] = {
		// X   Y
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },
	};

	Math::Vector2d verSizeWH;
	if (_noScalingOverride) {
		verSizeWH = normalizeCurrentCoordinates(width, height);
	} else {
		verSizeWH = normalizeOriginalCoordinates(width, height);
	}
	auto verOffsetXY = normalizeOriginalCoordinates(dest.x, dest.y);
	auto nativeViewport = _gfx->getViewport();
	auto viewport = Math::Vector2d(nativeViewport.width(), nativeViewport.height());

	SurfaceVertex vertices[4] = {};
	for (int32 v = 0; v < 4; v++) {
		Math::Vector2d pos = verOffsetXY + (surfaceVertices[v] * verSizeWH);

		if (_snapToGrid) {
			// Align vertex coordinates to the native pixel grid
			// This ensures text does not get garbled by nearest neighbors scaling
			pos.setX(floor(pos.getX() * viewport.getX() + 0.5) / viewport.getX());
			pos.setY(floor(pos.getY() * viewport.getY() + 0.5) / viewport.getY());
		}

		// position coords
		vertices[v].x = pos.getX() * 2.0 - 1.0;
		vertices[v].y = -1.0 * (pos.getY() * 2.0 - 1.0);
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);

	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(SurfaceVertex), &vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, 2 * sizeof(float), textCords);
	glColor3f(1.0f - _fadeLevel, 1.0f - _fadeLevel, 1.0f - _fadeLevel);

	texture->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	_gfx->end2DMode();
}

Math::Vector2d OpenGLSurfaceRenderer::normalizeOriginalCoordinates(int x, int y) const {
	Common::Rect viewport = _gfx->getUnscaledViewport();
	return Math::Vector2d(x / (float)viewport.width(), y / (float)viewport.height());
}

Math::Vector2d OpenGLSurfaceRenderer::normalizeCurrentCoordinates(int x, int y) const {
	Common::Rect viewport = _gfx->getViewport();
	return Math::Vector2d(x / (float)viewport.width(), y / (float)viewport.height());
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // if defined(USE_OPENGL_GAME)
