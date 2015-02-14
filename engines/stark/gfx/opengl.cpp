/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/stark/gfx/opengl.h"

#include "common/system.h"

#include "graphics/pixelbuffer.h"

#include "math/matrix4.h"

#include "engines/stark/gfx/opengltexture.h"

#ifdef USE_OPENGL

#ifdef SDL_BACKEND
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#endif

namespace Stark {
namespace Gfx {

OpenGLDriver::OpenGLDriver() {
}

OpenGLDriver::~OpenGLDriver() {
}

void OpenGLDriver::setupScreen(int screenW, int screenH, bool fullscreen) {
	g_system->setupScreen(screenW, screenH, fullscreen, true);

	_screenWidth = screenW;
	_screenHeight = screenH;
}

void OpenGLDriver::setGameViewport() {
	_viewport = Common::Rect(kGameViewportWidth, kGameViewportHeight);
	_viewport.translate(0, _screenHeight - kGameViewportHeight - kTopBorderHeight);

	glViewport(_viewport.left, _viewport.top, _viewport.width(), _viewport.height());
}

void OpenGLDriver::setScreenViewport() {
	_viewport = Common::Rect(_screenWidth, _screenHeight);

	glViewport(_viewport.left, _viewport.top, _viewport.width(), _viewport.height());
}

void OpenGLDriver::setupPerspective(const Math::Matrix4 &projectionMatrix) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(projectionMatrix.getData());
}

void OpenGLDriver::setupCamera(const Math::Vector3d &position, const Math::Matrix4 &lookAt) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(lookAt.getData());
	glTranslatef(-position.x(), -position.y(), -position.z());
}

void OpenGLDriver::clearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLDriver::flipBuffer() {
	g_system->updateScreen();
}

Texture *OpenGLDriver::createTexture(const Graphics::Surface *surface, const byte *palette) {
	OpenGlTexture *texture = new OpenGlTexture();

	if (surface) {
		texture->update(surface, palette);
	}

	return texture;
}

void OpenGLDriver::drawSurface(const Texture *texture, const Common::Point &dest) {
	// Source texture rectangle
	const float tLeft = 0.0;
	const float tWidth = 1.0;
	const float tTop = 0.0;
	const float tHeight = 1.0;

	// Destination rectangle
	const float sLeft = dest.x;
	const float sTop = dest.y;
	const float sWidth = texture->width();
	const float sHeight = texture->height();

	start2DMode();

	glColor3f(1.f, 1.f, 1.f);
	glEnable(GL_TEXTURE_2D);

	texture->bind();

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(tLeft, tTop + tHeight);
		glVertex3f(sLeft + 0, sTop + sHeight, 1.0f);

		glTexCoord2f(tLeft + tWidth, tTop + tHeight);
		glVertex3f(sLeft + sWidth, sTop + sHeight, 1.0f);

		glTexCoord2f(tLeft, tTop);
		glVertex3f(sLeft + 0, sTop + 0, 1.0f);

		glTexCoord2f(tLeft + tWidth, tTop);
		glVertex3f(sLeft + sWidth, sTop + 0, 1.0f);
	glEnd();

	end2DMode();
}

void OpenGLDriver::start2DMode() {

	// Load the ModelView matrix with the identity
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Load the Projection matrix with the identity
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, _viewport.width(), _viewport.height(), 0.0, -1.0, 1.0);

	// Enable alpha blending
	glEnable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD); // It's the default
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthMask(GL_FALSE);
}

void OpenGLDriver::end2DMode() {
	// Disable alpha blending
	glDisable(GL_BLEND);

	glDepthMask(GL_TRUE);

	// Pop the identity Projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	// Pop the identity ModelView matrix
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

void OpenGLDriver::set3DMode() {
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // USE_OPENGL
