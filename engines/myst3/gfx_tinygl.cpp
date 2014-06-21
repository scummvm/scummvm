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

#if !defined(USE_GLES2) && !defined(USE_OPENGL_SHADERS)

#include "graphics/colormasks.h"
#include "graphics/surface.h"

#include "math/vector2d.h"

#include "engines/myst3/gfx.h"
#include "engines/myst3/gfx_tinygl.h"
#include "engines/myst3/gfx_tinygl_texture.h"

#include <gl/GLU.h>

namespace Myst3 {

Renderer *Renderer::createRenderer(OSystem *system) {
	return new TinyGLRenderer(system);
}

TinyGLRenderer::TinyGLRenderer(OSystem *system) :
	BaseRenderer(system),
	_nonPowerOfTwoTexSupport(false) {
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

void TinyGLRenderer::init(Graphics::PixelBuffer &screenBuffer) {
	// Check the available OpenGL extensions
	_nonPowerOfTwoTexSupport = true;	
	
	_fb = new TinyGL::FrameBuffer(640, 480, screenBuffer);
	TinyGL::glInit(_fb);

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

void TinyGLRenderer::setupCameraOrtho2D() {
	tglViewport(0, 0, kOriginalWidth, kOriginalHeight);
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tgluOrtho2D(0.0, kOriginalWidth, kOriginalHeight, 0.0);

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

	tglViewport(0, kBottomBorderHeight, kOriginalWidth, kFrameHeight);
	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();
	tgluPerspective(glFOV, (TGLfloat)kOriginalWidth / (TGLfloat)kFrameHeight, 1.0, 10000.0);

	// Rotate the model to simulate the rotation of the camera
	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();
	tglRotatef(pitch, -1.0f, 0.0f, 0.0f);
	tglRotatef(heading - 180.0f, 0.0f, 1.0f, 0.0f);

	float modelView[16], projection[16];
	tglGetFloatv(TGL_MODELVIEW_MATRIX, modelView);
	tglGetFloatv(TGL_PROJECTION_MATRIX, projection);
	for(int i = 0; i < 16; i++) {
		_cubeModelViewMatrix[i] = modelView[i];
		_cubeProjectionMatrix[i] = projection[i];
	}

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

	tglBegin(TGL_TRIANGLE_STRIP);
		tglVertex3f( rect.left, rect.bottom, 0.0f);
		tglVertex3f( rect.right, rect.bottom, 0.0f);
		tglVertex3f( rect.left, rect.top, 0.0f);
		tglVertex3f( rect.right, rect.top, 0.0f);
	tglEnd();

	tglDisable(TGL_BLEND);
}

void TinyGLRenderer::drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect,
		Texture *texture, float transparency) {

	TinyGLTexture *glTexture = static_cast<TinyGLTexture *>(texture);

	const float tLeft = textureRect.left / (float) glTexture->internalWidth;
	const float tWidth = textureRect.width() / (float) glTexture->internalWidth;
	const float tTop = textureRect.top / (float) glTexture->internalHeight;
	const float tHeight = textureRect.height() / (float) glTexture->internalHeight;

	const float sLeft = screenRect.left;
	const float sTop = screenRect.top;
	const float sWidth = screenRect.width();
	const float sHeight = screenRect.height();

	if (transparency >= 0.0) {
		tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
		tglEnable(TGL_BLEND);
	} else {
		transparency = 1.0;
	}

	tglEnable(TGL_TEXTURE_2D);
	tglColor4f(1.0f, 1.0f, 1.0f, transparency);
	tglDepthMask(TGL_FALSE);

	tglBindTexture(TGL_TEXTURE_2D, glTexture->id);
	tglBegin(TGL_TRIANGLE_STRIP);
		tglTexCoord2f(tLeft, tTop + tHeight);
		tglVertex3f(sLeft + 0, sTop + sHeight, 1.0f);

		tglTexCoord2f(tLeft + tWidth, tTop + tHeight);
		tglVertex3f(sLeft + sWidth, sTop + sHeight, 1.0f);

		tglTexCoord2f(tLeft, tTop);
		tglVertex3f(sLeft + 0, sTop + 0, 1.0f);

		tglTexCoord2f(tLeft + tWidth, tTop);
		tglVertex3f(sLeft + sWidth, sTop + 0, 1.0f);
	tglEnd();

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

		float cw = textureRect.width() / (float) glFont->internalWidth;
		float ch = textureRect.height() / (float) glFont->internalHeight;
		float cx = textureRect.left / (float) glFont->internalWidth;
		float cy = textureRect.top / (float) glFont->internalHeight;

		tglBegin(TGL_QUADS);
		tglTexCoord2f(cx, cy + ch);
		tglVertex3f(x, y, 1.0f);
		tglTexCoord2f(cx + cw, cy + ch);
		tglVertex3f(x + w, y, 1.0f);
		tglTexCoord2f(cx + cw, cy);
		tglVertex3f(x + w, y + h, 1.0f);
		tglTexCoord2f(cx, cy);
		tglVertex3f(x, y + h, 1.0f);
		tglEnd();

		x += textureRect.width() - 3;
	}

	tglDisable(TGL_TEXTURE_2D);
	tglDisable(TGL_BLEND);
	tglDepthMask(TGL_TRUE);
}

void TinyGLRenderer::drawCube(Texture **textures) {
	TinyGLTexture *texture0 = static_cast<TinyGLTexture *>(textures[0]);

	// Size of the cube
	float t = 256.0f;

	// Used fragment of the textures
	float s = texture0->width / (float) texture0->internalWidth;

	tglEnable(TGL_TEXTURE_2D);
	tglDepthMask(TGL_FALSE);

	tglBindTexture(TGL_TEXTURE_2D, static_cast<TinyGLTexture *>(textures[4])->id);
	tglBegin(TGL_TRIANGLE_STRIP);			// X-
		tglTexCoord2f(0, s); tglVertex3f(-t,-t, t);
		tglTexCoord2f(s, s); tglVertex3f(-t,-t,-t);
		tglTexCoord2f(0, 0); tglVertex3f(-t, t, t);
		tglTexCoord2f(s, 0); tglVertex3f(-t, t,-t);
	tglEnd();

	tglBindTexture(TGL_TEXTURE_2D, static_cast<TinyGLTexture *>(textures[3])->id);
	tglBegin(TGL_TRIANGLE_STRIP);			// X+
		tglTexCoord2f(0, s); tglVertex3f( t,-t,-t);
		tglTexCoord2f(s, s); tglVertex3f( t,-t, t);
		tglTexCoord2f(0, 0); tglVertex3f( t, t,-t);
		tglTexCoord2f(s, 0); tglVertex3f( t, t, t);
	tglEnd();

	tglBindTexture(TGL_TEXTURE_2D, static_cast<TinyGLTexture *>(textures[1])->id);
	tglBegin(TGL_TRIANGLE_STRIP);			// Y-
		tglTexCoord2f(0, s); tglVertex3f( t,-t,-t);
		tglTexCoord2f(s, s); tglVertex3f(-t,-t,-t);
		tglTexCoord2f(0, 0); tglVertex3f( t,-t, t);
		tglTexCoord2f(s, 0); tglVertex3f(-t,-t, t);
	tglEnd();

	tglBindTexture(TGL_TEXTURE_2D, static_cast<TinyGLTexture *>(textures[5])->id);
	tglBegin(TGL_TRIANGLE_STRIP);			// Y+
		tglTexCoord2f(0, s); tglVertex3f( t, t, t);
		tglTexCoord2f(s, s); tglVertex3f(-t, t, t);
		tglTexCoord2f(0, 0); tglVertex3f( t, t,-t);
		tglTexCoord2f(s, 0); tglVertex3f(-t, t,-t);
	tglEnd();

	tglBindTexture(TGL_TEXTURE_2D, static_cast<TinyGLTexture *>(textures[0])->id);
	tglBegin(TGL_TRIANGLE_STRIP);			// Z-
		tglTexCoord2f(0, s); tglVertex3f(-t,-t,-t);
		tglTexCoord2f(s, s); tglVertex3f( t,-t,-t);
		tglTexCoord2f(0, 0); tglVertex3f(-t, t,-t);
		tglTexCoord2f(s, 0); tglVertex3f( t, t,-t);
	tglEnd();

	tglBindTexture(TGL_TEXTURE_2D, static_cast<TinyGLTexture *>(textures[2])->id);
	tglBegin(TGL_TRIANGLE_STRIP);			// Z+
		tglTexCoord2f(0, s); tglVertex3f( t,-t, t);
		tglTexCoord2f(s, s); tglVertex3f(-t,-t, t);
		tglTexCoord2f(0, 0); tglVertex3f( t, t, t);
		tglTexCoord2f(s, 0); tglVertex3f(-t, t, t);
	tglEnd();

	tglDepthMask(TGL_TRUE);
}

void TinyGLRenderer::drawTexturedRect3D(const Math::Vector3d &topLeft, const Math::Vector3d &bottomLeft,
		const Math::Vector3d &topRight, const Math::Vector3d &bottomRight, Texture *texture) {

	TinyGLTexture *glTexture = static_cast<TinyGLTexture *>(texture);

	const float w = glTexture->width / (float) glTexture->internalWidth;
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
	double x, y, z;

	// Screen coords to 3D coords
	gluUnProject(screen.x, kOriginalHeight - screen.y, 0.9, _cubeModelViewMatrix, _cubeProjectionMatrix, (TGLint *)_cubeViewport, &x, &y, &z);

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

#endif
