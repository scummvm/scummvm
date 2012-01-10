/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/myst3/gfx.h"
#include "engines/myst3/scene.h"

#include "common/rect.h"
#include "common/textconsole.h"

#include "graphics/colormasks.h"
#include "graphics/surface.h"

#ifdef SDL_BACKEND
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace Myst3 {

class OpenGLTexture : public Texture {
public:
	OpenGLTexture(Graphics::Surface *surface);
	virtual ~OpenGLTexture();

	GLuint id;
};

OpenGLTexture::OpenGLTexture(Graphics::Surface *surface) {
	width = surface->w;
	height = surface->h;
	format = surface->format;

	GLuint internalFormat;
	if (format.bytesPerPixel == 4)
		internalFormat = GL_RGBA;
	else if (format.bytesPerPixel == 3)
		internalFormat = GL_RGB;
	else
		error("Unknown pixel format");

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

OpenGLTexture::~OpenGLTexture() {
	glDeleteTextures(1, &id);
}

Renderer::Renderer(OSystem *system) :
	_system(system) {
}

Renderer::~Renderer() {
}

Texture *Renderer::createTexture(Graphics::Surface *surface) {
	return new OpenGLTexture(surface);
}

void Renderer::freeTexture(Texture *texture) {
	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);
	delete glTexture;
}

void Renderer::init() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
}

void Renderer::setupCameraOrtho2D() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, Scene::_originalWidth, Scene::_originalHeight, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Renderer::setupCameraPerspective(float pitch, float heading) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0, (GLfloat)Scene::_originalWidth /(GLfloat)Scene::_originalHeight, 0.1, 100.0);

	// Rotate the model to simulate the rotation of the camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(pitch, -1.0f, 0.0f, 0.0f);
	glRotatef(heading - 180.0f, 0.0f, 1.0f, 0.0f);
}

void Renderer::drawRect2D(const Common::Rect &rect, uint32 color) {
	uint8 a, r, g, b;
	Graphics::colorToARGB< Graphics::ColorMasks<8888> >(color, a, r, g, b);

	glDisable(GL_TEXTURE_2D);
	glColor4f(r / 255.0, g / 255.0, b / 255.0, a / 255.0);

	if (a != 255) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glBegin(GL_TRIANGLE_STRIP);
		glVertex3f( rect.left, rect.bottom, 0.0f);
		glVertex3f( rect.right, rect.bottom, 0.0f);
		glVertex3f( rect.left, rect.top, 0.0f);
		glVertex3f( rect.right, rect.top, 0.0f);
	glEnd();

	glDisable(GL_BLEND);
}

void Renderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect,
		Texture *texture, float transparency) {

	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

	const float tLeft = textureRect.left / (float) glTexture->width;
	const float tWidth = textureRect.width() / (float) glTexture->width;
	const float tTop = textureRect.top / (float) glTexture->height;
	const float tHeight = textureRect.height() / (float) glTexture->height;

	const float sLeft = screenRect.left;
	const float sTop = screenRect.top;
	const float sWidth = screenRect.width();
	const float sHeight = screenRect.height();

	if (transparency >= 0.0) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
	} else {
		transparency = 1.0;
	}

	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, transparency);
	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, glTexture->id);
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

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

} // end of namespace Myst3
