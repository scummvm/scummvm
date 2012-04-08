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

#include "math/vector2d.h"

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
	GLuint sourceFormat;
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

	if (format.bytesPerPixel == 4) {
		internalFormat = GL_RGBA;
		sourceFormat = GL_UNSIGNED_BYTE;
	} else if (format.bytesPerPixel == 3) {
		internalFormat = GL_RGB;
		sourceFormat = GL_UNSIGNED_BYTE;
	} else if (format.bytesPerPixel == 2) {
		internalFormat = GL_RGB;
		sourceFormat = GL_UNSIGNED_SHORT_5_6_5;
	} else
		error("Unknown pixel format");

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, internalWidth, internalHeight, 0, internalFormat, sourceFormat, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	update(surface);
}

OpenGLTexture::~OpenGLTexture() {
	glDeleteTextures(1, &id);
}

void OpenGLTexture::update(const Graphics::Surface *surface) {
	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h, internalFormat, sourceFormat, surface->pixels);
}

Renderer::Renderer(OSystem *system) :
	_system(system),
	_font(0) {
}

Renderer::~Renderer() {
	if (_font)
		freeTexture(_font);
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

void Renderer::initFont(const Graphics::Surface *surface) {
	_font = createTexture(surface);
}

void Renderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
}

void Renderer::setupCameraOrtho2D() {
	glViewport(0, 0, kOriginalWidth, kOriginalHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, kOriginalWidth, kOriginalHeight, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Renderer::setupCameraPerspective(float pitch, float heading, float fov) {
	// TODO: Find a correct and exact formula for the FOV
	GLfloat glFOV = 0.63 * fov; // Approximative and experimental formula
	if (fov > 79.0 && fov < 81.0)
		glFOV = 50.5; // Somewhat good value for fov == 80
	else if (fov > 59.0 && fov < 61.0)
		glFOV = 36.0; // Somewhat good value for fov == 60

	glViewport(0, kBottomBorderHeight, kOriginalWidth, kFrameHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(glFOV, (GLfloat)kOriginalWidth / (GLfloat)kFrameHeight, 1.0, 10000.0);

	// Rotate the model to simulate the rotation of the camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(pitch, -1.0f, 0.0f, 0.0f);
	glRotatef(heading - 180.0f, 0.0f, 1.0f, 0.0f);

	glGetDoublev(GL_MODELVIEW_MATRIX, _cubeModelViewMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, _cubeProjectionMatrix);
	glGetIntegerv(GL_VIEWPORT, (GLint *)_cubeViewport);
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

Common::Rect Renderer::getFontCharacterRect(uint8 character) {
	uint index = 0;

	if (character == ' ')
		index = 0;
	else if (character >= '0' && character <= '9')
		index = 1 + character - '0';
	else if (character >= 'A' && character <= 'Z')
		index = 1 + 10 + character - 'A';
	else if (character == '|')
		index = 1 + 10 + 26;

	return Common::Rect(16 * index, 0, 16 * (index + 1), 32);
}

void Renderer::draw2DText(const Common::String &text, const Common::Point &position) {
	OpenGLTexture *glFont = static_cast<OpenGLTexture *>(_font);

	// The font only has uppercase letters
	Common::String textToDraw = text;
	textToDraw.toUppercase();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, glFont->id);

	int x = position.x;
	int y = position.y;

	for (uint i = 0; i < textToDraw.size(); i++) {
		Common::Rect textureRect = getFontCharacterRect(textToDraw[i]);
		int w = textureRect.width();
		int h = textureRect.height();

		float cw = textureRect.width() / (float) glFont->internalWidth;
		float ch = textureRect.height() / (float) glFont->internalHeight;
		float cx = textureRect.left / (float) glFont->internalWidth;
		float cy = textureRect.top / (float) glFont->internalHeight;

		glBegin(GL_QUADS);
		glTexCoord2f(cx, cy + ch);
		glVertex3f(x, y, 1.0f);
		glTexCoord2f(cx + cw, cy + ch);
		glVertex3f(x + w, y, 1.0f);
		glTexCoord2f(cx + cw, cy);
		glVertex3f(x + w, y + h, 1.0f);
		glTexCoord2f(cx, cy);
		glVertex3f(x, y + h, 1.0f);
		glEnd();

		x += textureRect.width() - 3;
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void Renderer::drawCube(Texture **textures) {
	OpenGLTexture *texture0 = static_cast<OpenGLTexture *>(textures[0]);

	// Size of the cube
	float t = 256.0f;

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

Graphics::Surface *Renderer::getScreenshot() {
	Graphics::Surface *s = new Graphics::Surface();
	s->create(kOriginalWidth, kOriginalHeight, Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0));

	glReadPixels(0, 0, kOriginalWidth, kOriginalHeight, GL_RGB, GL_UNSIGNED_BYTE, s->pixels);

	return s;
}

void Renderer::screenPosToDirection(const Common::Point screen, float &pitch, float &heading) {
	double x, y, z;

	// Screen coords to 3D coords
	gluUnProject(screen.x, kOriginalHeight - screen.y, 0.9, _cubeModelViewMatrix, _cubeProjectionMatrix, (GLint *)_cubeViewport, &x, &y, &z);

	// 3D coords to polar coords
	Math::Vector3d v = Math::Vector3d(x, y, z);
	v.normalize();

	Math::Vector2d horizontalProjection = Math::Vector2d(v.x(), v.z());
	horizontalProjection.normalize();

	pitch = 90 - Math::Angle::arcCosine(v.y()).getDegrees();
	heading = Math::Angle::arcCosine(horizontalProjection.getY()).getDegrees();

	if (horizontalProjection.getX() > 0.0)
		heading = 360 - heading;
}

} // end of namespace Myst3
