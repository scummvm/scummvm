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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#include "common/rect.h"
#include "common/textconsole.h"

#if defined(USE_OPENGL) && !defined(USE_GLES2) && !defined(USE_OPENGL_SHADERS)

#include "graphics/colormasks.h"
#include "graphics/surface.h"

#include "math/vector2d.h"

#include "engines/myst3/gfx.h"
#include "engines/myst3/gfx_opengl.h"
#include "engines/myst3/gfx_opengl_texture.h"

namespace Myst3 {

Renderer *Renderer::createRenderer(OSystem *system) {
	return new OpenGLRenderer(system);
}

static const GLfloat cubeFacesVertices[][12] = {
//      X        Y        Z
  { -320.0f, -320.0f, -320.0f,
     320.0f, -320.0f, -320.0f,
    -320.0f,  320.0f, -320.0f,
     320.0f,  320.0f, -320.0f },

  {  320.0f, -320.0f, -320.0f,
    -320.0f, -320.0f, -320.0f,
     320.0f, -320.0f,  320.0f,
    -320.0f, -320.0f,  320.0f },

  {  320.0f, -320.0f,  320.0f,
    -320.0f, -320.0f,  320.0f,
     320.0f,  320.0f,  320.0f,
    -320.0f,  320.0f,  320.0f },

  {  320.0f, -320.0f, -320.0f,
     320.0f, -320.0f,  320.0f,
     320.0f,  320.0f, -320.0f,
     320.0f,  320.0f,  320.0f },

  { -320.0f, -320.0f,  320.0f,
    -320.0f, -320.0f, -320.0f,
    -320.0f,  320.0f,  320.0f,
    -320.0f,  320.0f, -320.0f },

  {  320.0f,  320.0f,  320.0f,
    -320.0f,  320.0f,  320.0f,
     320.0f,  320.0f, -320.0f,
    -320.0f,  320.0f, -320.0f }
};

static const GLfloat faceTextureCoords[] = {
	// S     T
	0.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
};

Renderer *CreateGfxOpenGL(OSystem *system) {
	return new OpenGLRenderer(system);
}

OpenGLRenderer::OpenGLRenderer(OSystem *system) :
		BaseRenderer(system),
		_nonPowerOfTwoTexSupport(false) {
}

OpenGLRenderer::~OpenGLRenderer() {
}

Texture *OpenGLRenderer::createTexture(const Graphics::Surface *surface) {
	return new OpenGLTexture(surface, _nonPowerOfTwoTexSupport);
}

void OpenGLRenderer::freeTexture(Texture *texture) {
	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);
	delete glTexture;
}

void OpenGLRenderer::init(Graphics::PixelBuffer &screenBuffer) {
	// Check the available OpenGL extensions
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	if (strstr(extensions, "GL_ARB_texture_non_power_of_two"))
		_nonPowerOfTwoTexSupport = true;
	else
		warning("GL_ARB_texture_non_power_of_two is not available.");

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
}

void OpenGLRenderer::setupCameraOrtho2D() {
	glViewport(0, 0, kOriginalWidth, kOriginalHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, kOriginalWidth, kOriginalHeight, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OpenGLRenderer::setupCameraPerspective(float pitch, float heading, float fov) {
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

void OpenGLRenderer::drawRect2D(const Common::Rect &rect, uint32 color) {
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

void OpenGLRenderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect,
		Texture *texture, float transparency, bool additiveBlending) {

	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

	const float tLeft = textureRect.left / (float)glTexture->internalWidth;
	const float tWidth = textureRect.width() / (float)glTexture->internalWidth;
	const float tTop = textureRect.top / (float)glTexture->internalHeight;
	const float tHeight = textureRect.height() / (float)glTexture->internalHeight;

	const float sLeft = screenRect.left;
	const float sTop = screenRect.top;
	const float sWidth = screenRect.width();
	const float sHeight = screenRect.height();

	if (transparency >= 0.0) {
		if (additiveBlending) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		} else {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

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

void OpenGLRenderer::draw2DText(const Common::String &text, const Common::Point &position) {
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

		float cw = textureRect.width() / (float)glFont->internalWidth;
		float ch = textureRect.height() / (float)glFont->internalHeight;
		float cx = textureRect.left / (float)glFont->internalWidth;
		float cy = textureRect.top / (float)glFont->internalHeight;

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

void OpenGLRenderer::drawFace(uint face, Texture *texture) {
	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

	// Used fragment of the texture
	const float w = glTexture->width  / (float) glTexture->internalWidth;
	const float h = glTexture->height / (float) glTexture->internalHeight;

	glBindTexture(GL_TEXTURE_2D, glTexture->id);
	glBegin(GL_TRIANGLE_STRIP);
	for (uint i = 0; i < 4; i++) {
		glTexCoord2f(w * faceTextureCoords[2 * i + 0], h * faceTextureCoords[2 * i + 1]);
		glVertex3f(cubeFacesVertices[face][3 * i + 0], cubeFacesVertices[face][3 * i + 1], cubeFacesVertices[face][3 * i + 2]);
	}
	glEnd();
}

void OpenGLRenderer::drawCube(Texture **textures) {
	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);

	for (uint i = 0; i < 6; i++) {
		drawFace(i, textures[i]);
	}

	glDepthMask(GL_TRUE);
}

void OpenGLRenderer::drawTexturedRect3D(const Math::Vector3d &topLeft, const Math::Vector3d &bottomLeft,
		const Math::Vector3d &topRight, const Math::Vector3d &bottomRight, Texture *texture) {

	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);

	const float w = glTexture->width / (float)glTexture->internalWidth;
	const float h = glTexture->height / (float)glTexture->internalHeight;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

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
	glDepthMask(GL_TRUE);
}

Graphics::Surface *OpenGLRenderer::getScreenshot() {
	Graphics::Surface *s = new Graphics::Surface();
	s->create(kOriginalWidth, kOriginalHeight, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	glReadPixels(0, 0, kOriginalWidth, kOriginalHeight, GL_RGBA, GL_UNSIGNED_BYTE, s->getPixels());

	return s;
}

void OpenGLRenderer::screenPosToDirection(const Common::Point screen, float &pitch, float &heading) {
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

} // End of namespace Myst3

#endif
