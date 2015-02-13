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
//#include <GL/glu.h>
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
	glViewport(0, _screenHeight - kGameViewportHeight - kTopBorderHeight, _screenWidth, kGameViewportHeight);
}

void OpenGLDriver::setScreenViewport() {
	glViewport(0, 0, _screenWidth, _screenHeight);
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

MipMapTexture *OpenGLDriver::createMipMapTexture() {
	return new OpenGlMipMapTexture();
}

void OpenGLDriver::drawSurface(const Graphics::Surface *surface, Common::Point dest, Common::Rect rect) {
	// Draw the whole surface by default
	if (rect.isEmpty())
		rect = Common::Rect(surface->w, surface->h);

	start2DMode();

	glDisable(GL_TEXTURE_2D);

	glRasterPos2f(-1.0, 1.0);
	glBitmap(0, 0, 0, 0, dest.x, -dest.y, nullptr);
	glDrawPixels(surface->w, surface->h, GL_RGBA, GL_UNSIGNED_BYTE, surface->getPixels());

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

	// Enable alpha blending
	glEnable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD); // It's the default
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthMask(GL_FALSE);

	// Flip the Y component
	glPixelZoom(1.0f, -1.0f);

	// Required by RGB sources, but not by RGBA
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
