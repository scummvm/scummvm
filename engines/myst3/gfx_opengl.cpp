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

#include "common/rect.h"
#include "common/textconsole.h"

#if defined(USE_OPENGL_GAME) && !defined(USE_GLES2)

#include "graphics/colormasks.h"
#include "graphics/opengl/context.h"
#include "graphics/surface.h"

#include "engines/myst3/gfx.h"
#include "engines/myst3/gfx_opengl.h"
#include "engines/myst3/gfx_opengl_texture.h"

namespace Myst3 {

Renderer *CreateGfxOpenGL(OSystem *system) {
	return new OpenGLRenderer(system);
}

OpenGLRenderer::OpenGLRenderer(OSystem *system) :
		Renderer(system) {
}

OpenGLRenderer::~OpenGLRenderer() {
}

Texture *OpenGLRenderer::createTexture(const Graphics::Surface *surface) {
	return new OpenGLTexture(surface);
}

void OpenGLRenderer::freeTexture(Texture *texture) {
	OpenGLTexture *glTexture = static_cast<OpenGLTexture *>(texture);
	delete glTexture;
}

void OpenGLRenderer::init() {
	debug("Initializing OpenGL Renderer");

	computeScreenViewport();

	// Check the available OpenGL extensions
	if (!OpenGLContext.NPOTSupported) {
		warning("GL_ARB_texture_non_power_of_two is not available.");
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::clear() {
	glClearColor(0.f, 0.f, 0.f, 1.f); // Solid black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
}

void OpenGLRenderer::selectTargetWindow(Window *window, bool is3D, bool scaled) {
	if (!window) {
		// No window found ...
		if (scaled) {
			// ... in scaled mode draw in the original game screen area
			Common::Rect vp = viewport();
			glViewport(vp.left, _system->getHeight() - vp.top - vp.height(), vp.width(), vp.height());
		} else {
			// ... otherwise, draw on the whole screen
			glViewport(0, 0, _system->getWidth(), _system->getHeight());
		}
	} else {
		// Found a window, draw inside it
		Common::Rect vp = window->getPosition();
		glViewport(vp.left, _system->getHeight() - vp.top - vp.height(), vp.width(), vp.height());
	}

	if (is3D) {
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(_projectionMatrix.getData());

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(_modelViewMatrix.getData());
	} else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		if (!window) {
			if (scaled) {
				glOrtho(0.0, kOriginalWidth, kOriginalHeight, 0.0, -1.0, 1.0);
			} else {
				glOrtho(0.0, _system->getWidth(), _system->getHeight(), 0.0, -1.0, 1.0);
			}
		} else {
			if (scaled) {
				Common::Rect originalRect = window->getOriginalPosition();
				glOrtho(0.0, originalRect.width(), originalRect.height(), 0.0, -1.0, 1.0);
			} else {
				Common::Rect vp = window->getPosition();
				glOrtho(0.0, vp.width(), vp.height(), 0.0, -1.0, 1.0);
			}
		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
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
		glVertex3f(rect.left, rect.bottom, 0.0f);
		glVertex3f(rect.right, rect.bottom, 0.0f);
		glVertex3f(rect.left, rect.top, 0.0f);
		glVertex3f(rect.right, rect.top, 0.0f);
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

	float sLeft = screenRect.left;
	float sTop = screenRect.top;
	float sRight = sLeft + screenRect.width();
	float sBottom = sTop + screenRect.height();

	if (glTexture->upsideDown) {
		SWAP(sTop, sBottom);
	}

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
		glTexCoord2f(w * cubeVertices[5 * (4 * face + i) + 0], h * cubeVertices[5 * (4 * face + i) + 1]);
		glVertex3f(cubeVertices[5 * (4 * face + i) + 2], cubeVertices[5 * (4 * face + i) + 3], cubeVertices[5 * (4 * face + i) + 4]);
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
	Common::Rect screen = viewport();

	Graphics::Surface *s = new Graphics::Surface();
	s->create(screen.width(), screen.height(), Texture::getRGBAPixelFormat());

	glReadPixels(screen.left, screen.top, screen.width(), screen.height(), GL_RGBA, GL_UNSIGNED_BYTE, s->getPixels());

	flipVertical(s);

	return s;
}

Texture *OpenGLRenderer::copyScreenshotToTexture() {
	OpenGLTexture *texture = new OpenGLTexture();

	Common::Rect screen = viewport();
	texture->copyFromFramebuffer(screen);

	return texture;
}

} // End of namespace Myst3

#endif
