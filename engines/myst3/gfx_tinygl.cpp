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

#include "common/config-manager.h"
#include "common/rect.h"
#include "common/textconsole.h"

#include "graphics/colormasks.h"
#include "graphics/surface.h"

#include "math/vector2d.h"
#include "math/glmath.h"

#include "engines/myst3/gfx.h"
#include "engines/myst3/gfx_tinygl.h"
#include "engines/myst3/gfx_tinygl_texture.h"
#include "graphics/tinygl/zblit.h"

namespace Myst3 {

Renderer *CreateGfxTinyGL(OSystem *system) {
	return new TinyGLRenderer(system);
}

TinyGLRenderer::TinyGLRenderer(OSystem *system) :
		Renderer(system),
		_fb(NULL) {
}

TinyGLRenderer::~TinyGLRenderer() {
}

Texture *TinyGLRenderer::createTexture(const Graphics::Surface *surface) {
	return new TinyGLTexture(surface);
}

void TinyGLRenderer::freeTexture(Texture *texture) {
	TinyGLTexture *glTexture = static_cast<TinyGLTexture *>(texture);
	delete glTexture;
}

void TinyGLRenderer::init() {
	debug("Initializing Software 3D Renderer");

	computeScreenViewport();

	_fb = new TinyGL::FrameBuffer(kOriginalWidth, kOriginalHeight, g_system->getScreenFormat());
	TinyGL::glInit(_fb, 512);
	tglEnableDirtyRects(ConfMan.getBool("dirtyrects"));

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglDisable(TGL_LIGHTING);
	tglEnable(TGL_TEXTURE_2D);
	tglEnable(TGL_DEPTH_TEST);
}

void TinyGLRenderer::clear() {
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT);
	tglColor3f(1.0f, 1.0f, 1.0f);
}

void TinyGLRenderer::selectTargetWindow(Window *window, bool is3D, bool scaled) {
	// NOTE: tinyGL viewport implementation needs to be checked as it doesn't behave the same as openGL

	if (!window) {
		// No window found ...
		if (scaled) {
			// ... in scaled mode draw in the original game screen area
			Common::Rect vp = viewport();
			tglViewport(vp.left, vp.top, vp.width(), vp.height());
			//tglViewport(vp.left, _system->getHeight() - vp.top - vp.height(), vp.width(), vp.height());
		} else {
			// ... otherwise, draw on the whole screen
			tglViewport(0, 0, _system->getWidth(), _system->getHeight());
		}
	} else {
		// Found a window, draw inside it
		Common::Rect vp = window->getPosition();
		tglViewport(vp.left, vp.top, vp.width(), vp.height());
		//tglViewport(vp.left, _system->getHeight() - vp.top - vp.height(), vp.width(), vp.height());
	}

	if (is3D) {
		tglMatrixMode(TGL_PROJECTION);
		tglLoadMatrixf(_projectionMatrix.getData());

		tglMatrixMode(TGL_MODELVIEW);
		tglLoadMatrixf(_modelViewMatrix.getData());
	} else {
		tglMatrixMode(TGL_PROJECTION);
		tglLoadIdentity();

		if (!window) {
			if (scaled) {
				tglOrtho(0.0, kOriginalWidth, kOriginalHeight, 0.0, -1.0, 1.0);
			} else {
				tglOrtho(0.0, _system->getWidth(), _system->getHeight(), 0.0, -1.0, 1.0);
			}
		} else {
			if (scaled) {
				Common::Rect originalRect = window->getOriginalPosition();
				tglOrtho(0.0, originalRect.width(), originalRect.height(), 0.0, -1.0, 1.0);
			} else {
				Common::Rect vp = window->getPosition();
				tglOrtho(0.0, vp.width(), vp.height(), 0.0, -1.0, 1.0);
			}
		}

		tglMatrixMode(TGL_MODELVIEW);
		tglLoadIdentity();
	}
}

void TinyGLRenderer::drawRect2D(const Common::Rect &rect, uint32 color) {
	uint8 a, r, g, b;
	Graphics::colorToARGB< Graphics::ColorMasks<8888> >(color, a, r, g, b);

	tglDisable(TGL_TEXTURE_2D);
	tglColor4f(r / 255.0, g / 255.0, b / 255.0, a / 255.0);

	if (a != 255) {
		tglEnable(TGL_BLEND);
		tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
	}

	tglBegin(TGL_TRIANGLE_STRIP);
		tglVertex3f(rect.left, rect.bottom, 0.0f);
		tglVertex3f(rect.right, rect.bottom, 0.0f);
		tglVertex3f(rect.left, rect.top, 0.0f);
		tglVertex3f(rect.right, rect.top, 0.0f);
	tglEnd();

	tglDisable(TGL_BLEND);
}

void TinyGLRenderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect,
		Texture *texture, float transparency, bool additiveBlending) {
	const float sLeft = screenRect.left;
	const float sTop = screenRect.top;
	const float sWidth = screenRect.width();
	const float sHeight = screenRect.height();

	if (transparency >= 0.0) {
		if (additiveBlending) {
			tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE);
		} else {
			tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
		}
		tglEnable(TGL_BLEND);
	} else {
		transparency = 1.0;
	}

	// HACK: tglBlit is not affected by the viewport, so we offset the draw coordinates here
	int viewPort[4];
	tglGetIntegerv(TGL_VIEWPORT, viewPort);

	tglEnable(TGL_TEXTURE_2D);
	tglDepthMask(TGL_FALSE);

	Graphics::BlitTransform transform(sLeft + viewPort[0], sTop + viewPort[1]);
	transform.sourceRectangle(textureRect.left, textureRect.top, sWidth, sHeight);
	transform.tint(transparency);
	tglBlit(((TinyGLTexture *)texture)->getBlitTexture(), transform);

	tglDisable(TGL_BLEND);
	tglDepthMask(TGL_TRUE);
}

void TinyGLRenderer::draw2DText(const Common::String &text, const Common::Point &position) {
	TinyGLTexture *glFont = static_cast<TinyGLTexture *>(_font);

	// The font only has uppercase letters
	Common::String textToDraw = text;
	textToDraw.toUppercase();

	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);

	tglEnable(TGL_TEXTURE_2D);
	tglDepthMask(TGL_FALSE);

	tglColor3f(1.0f, 1.0f, 1.0f);
	tglBindTexture(TGL_TEXTURE_2D, glFont->id);

	int x = position.x;
	int y = position.y;

	for (uint i = 0; i < textToDraw.size(); i++) {
		Common::Rect textureRect = getFontCharacterRect(textToDraw[i]);
		int w = textureRect.width();
		int h = textureRect.height();

		Graphics::BlitTransform transform(x, y);
		transform.sourceRectangle(textureRect.left, textureRect.top, w, h);
		transform.flip(true, false);
		Graphics::tglBlit(glFont->getBlitTexture(), transform);

		x += textureRect.width() - 3;
	}

	tglDisable(TGL_TEXTURE_2D);
	tglDisable(TGL_BLEND);
	tglDepthMask(TGL_TRUE);
}

void TinyGLRenderer::drawFace(uint face, Texture *texture) {
	TinyGLTexture *glTexture = static_cast<TinyGLTexture *>(texture);

	tglBindTexture(TGL_TEXTURE_2D, glTexture->id);
	tglBegin(TGL_TRIANGLE_STRIP);
	for (uint i = 0; i < 4; i++) {
		tglTexCoord2f(cubeVertices[5 * (4 * face + i) + 0], cubeVertices[5 * (4 * face + i) + 1]);
		tglVertex3f(cubeVertices[5 * (4 * face + i) + 2], cubeVertices[5 * (4 * face + i) + 3], cubeVertices[5 * (4 * face + i) + 4]);
	}
	tglEnd();
}

void TinyGLRenderer::drawCube(Texture **textures) {
	tglEnable(TGL_TEXTURE_2D);
	tglDepthMask(TGL_FALSE);

	for (uint i = 0; i < 6; i++) {
		drawFace(i, textures[i]);
	}

	tglDepthMask(TGL_TRUE);
}

void TinyGLRenderer::drawTexturedRect3D(const Math::Vector3d &topLeft, const Math::Vector3d &bottomLeft,
		const Math::Vector3d &topRight, const Math::Vector3d &bottomRight, Texture *texture) {

	TinyGLTexture *glTexture = static_cast<TinyGLTexture *>(texture);

	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
	tglEnable(TGL_BLEND);
	tglDepthMask(TGL_FALSE);

	tglBindTexture(TGL_TEXTURE_2D, glTexture->id);

	tglBegin(TGL_TRIANGLE_STRIP);
		tglTexCoord2f(0, 0);
		tglVertex3f(-topLeft.x(), topLeft.y(), topLeft.z());

		tglTexCoord2f(0, 1);
		tglVertex3f(-bottomLeft.x(), bottomLeft.y(), bottomLeft.z());

		tglTexCoord2f(1, 0);
		tglVertex3f(-topRight.x(), topRight.y(), topRight.z());

		tglTexCoord2f(1, 1);
		tglVertex3f(-bottomRight.x(), bottomRight.y(), bottomRight.z());
	tglEnd();

	tglDisable(TGL_BLEND);
	tglDepthMask(TGL_TRUE);
}

Graphics::Surface *TinyGLRenderer::getScreenshot() {
	Graphics::Surface *s = new Graphics::Surface();
	s->create(kOriginalWidth, kOriginalHeight, Texture::getRGBAPixelFormat());
	Graphics::PixelBuffer buf(s->format, (byte *)s->getPixels());
	_fb->copyToBuffer(buf);
	return s;
}

void TinyGLRenderer::flipBuffer() {
	TinyGL::tglPresentBuffer();
	g_system->copyRectToScreen(_fb->getPixelBuffer(), _fb->linesize,
	                           0, 0, _fb->xsize, _fb->ysize);
}

} // End of namespace Myst3
