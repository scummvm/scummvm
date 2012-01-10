/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(WIN32)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#endif

#include "engines/myst3/gfx.h"

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
	OpenGLTexture(const Graphics::Surface *surface);
	virtual ~OpenGLTexture();

	void update(const Graphics::Surface *surface);

	GLuint id;
	GLuint internalFormat;
	uint32 internalWidth;
	uint32 internalHeight;
};

// From Bit Twiddling Hacks
static uint32 upperPowerOfTwo(uint32 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

OpenGLTexture::OpenGLTexture(const Graphics::Surface *surface) {
	width = surface->w;
	height = surface->h;
	format = surface->format;

	internalHeight = upperPowerOfTwo(height);
	internalWidth = upperPowerOfTwo(width);

	if (format.bytesPerPixel == 4)
		internalFormat = GL_RGBA;
	else if (format.bytesPerPixel == 3)
		internalFormat = GL_RGB;
	else
		error("Unknown pixel format");

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, internalWidth, internalHeight, 0, internalFormat, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	update(surface);
}

OpenGLTexture::~OpenGLTexture() {
	glDeleteTextures(1, &id);
}

void OpenGLTexture::update(const Graphics::Surface *surface) {
	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h, internalFormat, GL_UNSIGNED_BYTE, surface->pixels);
}

Renderer::Renderer(OSystem *system) :
	_system(system) {
}

Renderer::~Renderer() {
}

Texture *Renderer::createTexture(const Graphics::Surface *surface) {
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
	gluOrtho2D(0.0, originalWidth, originalHeight, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Renderer::setupCameraPerspective(float pitch, float heading) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0, (GLfloat)originalWidth /(GLfloat)originalHeight, 0.1, 100.0);

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

	const float tLeft = textureRect.left / (float) glTexture->internalWidth;
	const float tWidth = textureRect.width() / (float) glTexture->internalWidth;
	const float tTop = textureRect.top / (float) glTexture->internalHeight;
	const float tHeight = textureRect.height() / (float) glTexture->internalHeight;

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

void Renderer::drawCube(Texture **textures) {
	OpenGLTexture *texture0 = static_cast<OpenGLTexture *>(textures[0]);

	// Size of the cube
	float t = 1.0f;

	// Used fragment of the textures
	float s = texture0->width / (float) texture0->internalWidth;

	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[4])->id);
	glBegin(GL_TRIANGLE_STRIP);			// X-
		glTexCoord2f(0, s); glVertex3f(-t,-t, t);
		glTexCoord2f(s, s); glVertex3f(-t,-t,-t);
		glTexCoord2f(0, 0); glVertex3f(-t, t, t);
		glTexCoord2f(s, 0); glVertex3f(-t, t,-t);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[3])->id);
	glBegin(GL_TRIANGLE_STRIP);			// X+
		glTexCoord2f(0, s); glVertex3f( t,-t,-t);
		glTexCoord2f(s, s); glVertex3f( t,-t, t);
		glTexCoord2f(0, 0); glVertex3f( t, t,-t);
		glTexCoord2f(s, 0); glVertex3f( t, t, t);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[1])->id);
	glBegin(GL_TRIANGLE_STRIP);			// Y-
		glTexCoord2f(0, s); glVertex3f( t,-t,-t);
		glTexCoord2f(s, s); glVertex3f(-t,-t,-t);
		glTexCoord2f(0, 0); glVertex3f( t,-t, t);
		glTexCoord2f(s, 0); glVertex3f(-t,-t, t);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[5])->id);
	glBegin(GL_TRIANGLE_STRIP);			// Y+
		glTexCoord2f(0, s); glVertex3f( t, t, t);
		glTexCoord2f(s, s); glVertex3f(-t, t, t);
		glTexCoord2f(0, 0); glVertex3f( t, t,-t);
		glTexCoord2f(s, 0); glVertex3f(-t, t,-t);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[0])->id);
	glBegin(GL_TRIANGLE_STRIP);			// Z-
		glTexCoord2f(0, s); glVertex3f(-t,-t,-t);
		glTexCoord2f(s, s); glVertex3f( t,-t,-t);
		glTexCoord2f(0, 0); glVertex3f(-t, t,-t);
		glTexCoord2f(s, 0); glVertex3f( t, t,-t);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLTexture *>(textures[2])->id);
	glBegin(GL_TRIANGLE_STRIP);			// Z+
		glTexCoord2f(0, s); glVertex3f( t,-t, t);
		glTexCoord2f(s, s); glVertex3f(-t,-t, t);
		glTexCoord2f(0, 0); glVertex3f( t, t, t);
		glTexCoord2f(s, 0); glVertex3f(-t, t, t);
	glEnd();

	glDepthMask(GL_TRUE);
}

void Renderer::drawTexturedRect3D(const Math::Vector3d &topLeft, const Math::Vector3d &bottomLeft,
		const Math::Vector3d &topRight, const Math::Vector3d &bottomRight, Texture *texture) {

	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

	const float w = glTexture->width / (float) glTexture->internalWidth;
	const float h = glTexture->height / (float)glTexture->internalHeight;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, glTexture->id);

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0, 0);
		glVertex3f(-topLeft.x(), topLeft.y(), topLeft.z());

		glTexCoord2f(0, h);
		glVertex3f(-bottomLeft.x(), bottomLeft.y(), bottomLeft.z());

		glTexCoord2f(w, 0);
		glVertex3f(-topRight.x(), topRight.y(), topRight.z());

		glTexCoord2f(w, h);
		glVertex3f(-bottomRight.x(), bottomRight.y(), bottomRight.z());
	glEnd();

	glDisable(GL_BLEND);
}


} // end of namespace Myst3
