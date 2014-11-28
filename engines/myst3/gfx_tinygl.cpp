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

static const TGLfloat cubeFacesVertices[][12] = {
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

static const TGLfloat faceTextureCoords[] = {
	// S     T
	0.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f,
};

Renderer *CreateGfxTinyGL(OSystem *system) {
	return new TinyGLRenderer(system);
}

TinyGLRenderer::TinyGLRenderer(OSystem *system) :
		BaseRenderer(system),
		_nonPowerOfTwoTexSupport(false),
		_fb(NULL) {
}

TinyGLRenderer::~TinyGLRenderer() {
}

Texture *TinyGLRenderer::createTexture(const Graphics::Surface *surface) {
	return new TinyGLTexture(surface, _nonPowerOfTwoTexSupport);
}

void TinyGLRenderer::freeTexture(Texture *texture) {
	TinyGLTexture *glTexture = static_cast<TinyGLTexture *>(texture);
	delete glTexture;
}

void TinyGLRenderer::init() {
	debug("Initializing Software 3D Renderer");

	bool fullscreen = ConfMan.getBool("fullscreen");
	Graphics::PixelBuffer screenBuffer = _system->setupScreen(kOriginalWidth, kOriginalHeight, fullscreen, false);
	computeScreenViewport();

	_nonPowerOfTwoTexSupport = true;

	_fb = new TinyGL::FrameBuffer(kOriginalWidth, kOriginalHeight, screenBuffer);
	TinyGL::glInit(_fb, 512);

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

void TinyGLRenderer::setupCameraOrtho2D(bool noScaling) {
	tglViewport(0, 0, kOriginalWidth, kOriginalHeight);
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tglOrtho(0.0, kOriginalWidth, kOriginalHeight, 0.0, -1.0, 1.0);

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
}

void TinyGLRenderer::setupCameraPerspective(float pitch, float heading, float fov) {
	// TODO: Find a correct and exact formula for the FOV
	TGLfloat glFOV = 0.63 * fov; // Approximative and experimental formula
	if (fov > 79.0 && fov < 81.0)
		glFOV = 50.5; // Somewhat good value for fov == 80
	else if (fov > 59.0 && fov < 61.0)
		glFOV = 36.0; // Somewhat good value for fov == 60

	// NOTE: tinyGL viewport implementation needs to be checked as it doesn't behave the same as openGL
	tglViewport(0, kTopBorderHeight, kOriginalWidth, kFrameHeight);
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	Math::Matrix4 m = Math::makePerspectiveMatrix(glFOV, (TGLfloat)kOriginalWidth / (TGLfloat)kFrameHeight, 1.0, 10000.0);
	tglMultMatrixf(m.getData());

	// Rotate the model to simulate the rotation of the camera
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
	tglRotatef(pitch, -1.0f, 0.0f, 0.0f);
	tglRotatef(heading - 180.0f, 0.0f, 1.0f, 0.0f);

	tglGetFloatv(TGL_MODELVIEW_MATRIX, _cubeModelViewMatrix);
	tglGetFloatv(TGL_PROJECTION_MATRIX, _cubeProjectionMatrix);
	tglGetIntegerv(TGL_VIEWPORT, (TGLint *)_cubeViewport);
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

	for (int x = rect.left; x < rect.right; x++) {
		for (int y = rect.top; y < rect.bottom; y++) {
			_fb->writePixel(y * kOriginalWidth + x, a, r, g, b);
		}
	}

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

	tglEnable(TGL_TEXTURE_2D);
	tglDepthMask(TGL_FALSE);

	Graphics::BlitTransform transform(sLeft, sTop);
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

	// Used fragment of the texture
	const float w = glTexture->width / (float)glTexture->internalWidth;
	const float h = glTexture->height / (float)glTexture->internalHeight;

	tglBindTexture(TGL_TEXTURE_2D, glTexture->id);
	tglBegin(TGL_TRIANGLE_STRIP);
	for (uint i = 0; i < 4; i++) {
		tglTexCoord2f(w * faceTextureCoords[2 * i + 0], h * faceTextureCoords[2 * i + 1]);
		tglVertex3f(cubeFacesVertices[face][3 * i + 0], cubeFacesVertices[face][3 * i + 1], cubeFacesVertices[face][3 * i + 2]);
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

	const float w = glTexture->width / (float)glTexture->internalWidth;
	const float h = glTexture->height / (float)glTexture->internalHeight;

	tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
	tglEnable(TGL_BLEND);
	tglDepthMask(TGL_FALSE);

	tglBindTexture(TGL_TEXTURE_2D, glTexture->id);

	tglBegin(TGL_TRIANGLE_STRIP);
		tglTexCoord2f(0, 0);
		tglVertex3f(-topLeft.x(), topLeft.y(), topLeft.z());

		tglTexCoord2f(0, h);
		tglVertex3f(-bottomLeft.x(), bottomLeft.y(), bottomLeft.z());

		tglTexCoord2f(w, 0);
		tglVertex3f(-topRight.x(), topRight.y(), topRight.z());

		tglTexCoord2f(w, h);
		tglVertex3f(-bottomRight.x(), bottomRight.y(), bottomRight.z());
	tglEnd();

	tglDisable(TGL_BLEND);
	tglDepthMask(TGL_TRUE);
}

Graphics::Surface *TinyGLRenderer::getScreenshot() {
	Graphics::Surface *s = new Graphics::Surface();
	s->create(kOriginalWidth, kOriginalHeight, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
	Graphics::PixelBuffer buf(s->format, (byte *)s->getPixels());
	_fb->copyToBuffer(buf);
	return s;
}

void TinyGLRenderer::screenPosToDirection(const Common::Point screen, float &pitch, float &heading) {
	// Screen coords to 3D coords
	Math::Vector3d obj;
	Math::gluMathUnProject<float, int>(Math::Vector3d(screen.x, kOriginalHeight - screen.y, 0.9f),
		_cubeModelViewMatrix, _cubeProjectionMatrix, _cubeViewport, obj);

	// 3D coords to polar coords
	obj.normalize();

	Math::Vector2d horizontalProjection = Math::Vector2d(obj.x(), obj.z());
	horizontalProjection.normalize();

	pitch = 90 - Math::Angle::arcCosine(obj.y()).getDegrees();
	heading = Math::Angle::arcCosine(horizontalProjection.getY()).getDegrees();

	if (horizontalProjection.getX() > 0.0)
		heading = 360 - heading;
}

void TinyGLRenderer::flipBuffer() {
	TinyGL::tglPresentBuffer();
}

} // End of namespace Myst3
