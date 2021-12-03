/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "graphics/surface.h"
#include "graphics/tinygl/zblit.h"

#include "math/vector2d.h"
#include "math/glmath.h"

#include "engines/playground3d/gfx.h"
#include "engines/playground3d/gfx_tinygl.h"

namespace Playground3d {

static const TGLfloat dimRegionVertices[] = {
	//  X      Y
	-0.5f,  0.5f,
	 0.5f,  0.5f,
	-0.5f, -0.5f,
	 0.5f, -0.5f,
};

static const TGLuint dimRegionIndices[] = {
	0, 1, 2, 3
};

static const TGLfloat boxVertices[] = {
	//  X      Y
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	-1.0f, -1.0f,
	 1.0f, -1.0f,
};

static const TGLfloat bitmapVertices[] = {
	//  X      Y
	-0.2f,  0.2f,
	 0.2f,  0.2f,
	-0.2f, -0.2f,
	 0.2f, -0.2f,
};

static const TGLfloat textCords[] = {
	// S     T
	0.0f, 0.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
};

Renderer *CreateGfxTinyGL(OSystem *system) {
	return new TinyGLRenderer(system);
}

TinyGLRenderer::TinyGLRenderer(OSystem *system) :
		Renderer(system),
		_fb(nullptr) {
}

TinyGLRenderer::~TinyGLRenderer() {
}

void TinyGLRenderer::init() {
	debug("Initializing Software 3D Renderer");

	computeScreenViewport();

	_fb = new TinyGL::FrameBuffer(kOriginalWidth, kOriginalHeight, g_system->getScreenFormat());
	TinyGL::glInit(_fb, 512);
	tglEnableDirtyRects(false/*ConfMan.getBool("dirtyrects")*/);

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglDisable(TGL_LIGHTING);
	tglEnable(TGL_DEPTH_TEST);

	tglGenTextures(10, _textureRgbaId);
	tglGenTextures(10, _textureRgbId);
}

void TinyGLRenderer::deinit() {
	//tglDeleteTextures(10, &_textureRgbaId);
	//tglDeleteTextures(10, &_textureRgbId);
}

void TinyGLRenderer::loadTextureRGBA(Graphics::Surface *texture) {
	tglBindTexture(TGL_TEXTURE_2D, _textureRgbaId[0]);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, texture->w, texture->h, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, texture->getPixels());
	tglBindTexture(TGL_TEXTURE_2D, _textureRgbaId[1]);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, texture->w, texture->h, 0, TGL_BGRA, TGL_UNSIGNED_BYTE, texture->getPixels());
}

void TinyGLRenderer::loadTextureRGB(Graphics::Surface *texture) {
	tglBindTexture(TGL_TEXTURE_2D, _textureRgbId[0]);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, texture->w, texture->h, 0, TGL_RGB, TGL_UNSIGNED_BYTE, texture->getPixels());
}

void TinyGLRenderer::clear(const Math::Vector4d &clearColor) {
	tglClearColor(clearColor.x(), clearColor.y(), clearColor.z(), clearColor.w());
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT);
}

void TinyGLRenderer::setupViewport(int x, int y, int width, int height) {
	tglViewport(x, y, width, height);
}

void TinyGLRenderer::drawFace(uint face) {
	tglBegin(TGL_TRIANGLE_STRIP);
	for (uint i = 0; i < 4; i++) {
		tglColor3f(cubeVertices[11 * (4 * face + i) + 8], cubeVertices[11 * (4 * face + i) + 9], cubeVertices[11 * (4 * face + i) + 10]);
		tglVertex3f(cubeVertices[11 * (4 * face + i) + 2], cubeVertices[11 * (4 * face + i) + 3], cubeVertices[11 * (4 * face + i) + 4]);
		tglNormal3f(cubeVertices[11 * (4 * face + i) + 5], cubeVertices[11 * (4 * face + i) + 6], cubeVertices[11 * (4 * face + i) + 7]);
	}
	tglEnd();
}

void TinyGLRenderer::drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadMatrixf(_projectionMatrix.getData());

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadMatrixf(_modelViewMatrix.getData());

	tglTranslatef(pos.x(), pos.y(), pos.z());
	tglRotatef(roll.x(), 1.0f, 0.0f, 0.0f);
	tglRotatef(roll.y(), 0.0f, 1.0f, 0.0f);
	tglRotatef(roll.z(), 0.0f, 0.0f, 1.0f);

	for (uint i = 0; i < 6; i++) {
		drawFace(i);
	}
}

void TinyGLRenderer::drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	tglMatrixMode(TGL_PROJECTION);
	tglLoadMatrixf(_projectionMatrix.getData());

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadMatrixf(_modelViewMatrix.getData());

	tglTranslatef(pos.x(), pos.y(), pos.z());
	tglRotatef(roll.y(), 0.0f, 1.0f, 0.0f);

	tglColor3f(0.0f, 1.0f, 0.0f);
	tglBegin(TGL_TRIANGLES);
	tglVertex3f(-1.0f,  1.0, 0.0f);
	tglVertex3f( 1.0f,  1.0, 0.0f);
	tglVertex3f( 0.0f, -1.0, 0.0f);
	tglEnd();

	tglPolygonOffset(-1.0f, 0.0f);
	tglEnable(TGL_POLYGON_OFFSET_FILL);
	tglColor3f(1.0f, 1.0f, 1.0f);
	tglBegin(TGL_TRIANGLES);
	tglVertex3f(-0.5f,  0.5, 0.0f);
	tglVertex3f( 0.5f,  0.5, 0.0f);
	tglVertex3f( 0.0f, -0.5, 0.0f);
	tglEnd();
	tglDisable(TGL_POLYGON_OFFSET_FILL);
}

void TinyGLRenderer::flipBuffer() {
	TinyGL::tglPresentBuffer();
	g_system->copyRectToScreen(_fb->getPixelBuffer(), _fb->linesize, 0, 0, _fb->xsize, _fb->ysize);
}

void TinyGLRenderer::dimRegionInOut(float fade) {
	tglMatrixMode(TGL_PROJECTION);
	tglPushMatrix();
	tglLoadIdentity();

	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
	tglLoadIdentity();

	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_ONE, TGL_ONE_MINUS_SRC_ALPHA);
	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);

	tglColor4f(0.0f, 0.0f, 0.0f, 1.0f - fade);
	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglVertexPointer(2, TGL_FLOAT, 0, dimRegionVertices);
	tglDrawElements(TGL_TRIANGLE_STRIP, 4, TGL_UNSIGNED_INT, dimRegionIndices);
	//tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);
	tglDisableClientState(TGL_VERTEX_ARRAY);

	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();

	tglMatrixMode(TGL_PROJECTION);
	tglPopMatrix();
}

void TinyGLRenderer::drawInViewport() {
	static TGLfloat box2Vertices[] = {
		//  X      Y
		-0.1f,  0.1f,
		 0.1f,  0.1f,
		-0.1f, -0.1f,
		 0.1f, -0.1f,
	};
	tglMatrixMode(TGL_PROJECTION);
	tglPushMatrix();
	tglLoadIdentity();

	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
	tglLoadIdentity();

	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_ONE, TGL_ONE_MINUS_SRC_ALPHA);
	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);

	tglColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), &boxVertices[0]);
	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);
	tglDisableClientState(TGL_VERTEX_ARRAY);

	tglPushMatrix();
	_pos.x() += 0.01;
	_pos.y() += 0.01;
	if (_pos.x() >= 1.0f) {
		_pos.x() = -1.0;
		_pos.y() = -1.0;
	}
	tglTranslatef(_pos.x(), _pos.y(), 0);

	tglPolygonOffset(-1.0f, 0.0f);
	tglEnable(TGL_POLYGON_OFFSET_FILL);
	tglColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), &box2Vertices[0]);
	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);
	tglDisableClientState(TGL_VERTEX_ARRAY);
	tglDisable(TGL_POLYGON_OFFSET_FILL);

	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();
	tglPopMatrix();

	tglMatrixMode(TGL_PROJECTION);
	tglPopMatrix();
}

void TinyGLRenderer::drawRgbaTexture() {
	tglMatrixMode(TGL_PROJECTION);
	tglPushMatrix();
	tglLoadIdentity();

	tglMatrixMode(TGL_MODELVIEW);
	tglPushMatrix();
	tglLoadIdentity();

	tglEnable(TGL_BLEND);
	tglBlendFunc(TGL_ONE, TGL_ONE_MINUS_SRC_ALPHA);
	tglDisable(TGL_DEPTH_TEST);
	tglDepthMask(TGL_FALSE);
	tglEnable(TGL_TEXTURE_2D);

	tglColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	tglTranslatef(-0.8, 0.8, 0);

	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), bitmapVertices);
	tglTexCoordPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), textCords);
	tglBindTexture(TGL_TEXTURE_2D, _textureRgbaId[0]);
	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);
	tglDisableClientState(TGL_VERTEX_ARRAY);
	tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);

	tglTranslatef(0.5, 0, 0);

	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), bitmapVertices);
	tglTexCoordPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), textCords);
	tglBindTexture(TGL_TEXTURE_2D, _textureRgbaId[1]);
	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);
	tglDisableClientState(TGL_VERTEX_ARRAY);
	tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);

	tglTranslatef(0.5, 0, 0);

	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);
	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), bitmapVertices);
	tglTexCoordPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), textCords);
	tglBindTexture(TGL_TEXTURE_2D, _textureRgbId[0]);
	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);
	tglDisableClientState(TGL_VERTEX_ARRAY);
	tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);

	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();

	tglMatrixMode(TGL_PROJECTION);
	tglPopMatrix();
}

} // End of namespace Playground3d
