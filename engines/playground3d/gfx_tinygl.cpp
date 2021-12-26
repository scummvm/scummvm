/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/rect.h"
#include "common/textconsole.h"

#include "graphics/surface.h"
#include "graphics/tinygl/tinygl.h"

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
		_blitImageRgba(nullptr),
		_blitImageRgb(nullptr),
		_blitImageRgb565(nullptr),
		_blitImageRgba5551(nullptr),
		_blitImageRgba4444(nullptr) {
}

TinyGLRenderer::~TinyGLRenderer() {
	TinyGL::destroyContext();
}

void TinyGLRenderer::init() {
	debug("Initializing Software 3D Renderer");

	computeScreenViewport();

	TinyGL::createContext(kOriginalWidth, kOriginalHeight, g_system->getScreenFormat(), 512, true, ConfMan.getBool("dirtyrects"));

	tglMatrixMode(TGL_PROJECTION);
	tglLoadIdentity();

	tglMatrixMode(TGL_MODELVIEW);
	tglLoadIdentity();

	tglDisable(TGL_LIGHTING);
	tglEnable(TGL_DEPTH_TEST);

	tglGenTextures(5, _textureRgbaId);
	tglGenTextures(5, _textureRgbId);
	tglGenTextures(2, _textureRgb565Id);
	tglGenTextures(2, _textureRgba5551Id);
	tglGenTextures(2, _textureRgba4444Id);
	_blitImageRgba = tglGenBlitImage();
	_blitImageRgb = tglGenBlitImage();
	_blitImageRgb565 = tglGenBlitImage();
	_blitImageRgba5551 = tglGenBlitImage();
	_blitImageRgba4444 = tglGenBlitImage();
}

void TinyGLRenderer::deinit() {
	tglDeleteTextures(5, _textureRgbaId);
	tglDeleteTextures(5, _textureRgbId);
	tglDeleteTextures(2, _textureRgb565Id);
	tglDeleteTextures(2, _textureRgba5551Id);
	tglDeleteTextures(2, _textureRgba4444Id);
	tglDeleteBlitImage(_blitImageRgba);
	tglDeleteBlitImage(_blitImageRgb);
	tglDeleteBlitImage(_blitImageRgb565);
	tglDeleteBlitImage(_blitImageRgba5551);
	tglDeleteBlitImage(_blitImageRgba4444);
}

void TinyGLRenderer::loadTextureRGBA(Graphics::Surface *texture) {
	tglBindTexture(TGL_TEXTURE_2D, _textureRgbaId[0]);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, texture->w, texture->h, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, texture->getPixels());
	tglUploadBlitImage(_blitImageRgba, *texture, 0, false);
}

void TinyGLRenderer::loadTextureRGB(Graphics::Surface *texture) {
	tglBindTexture(TGL_TEXTURE_2D, _textureRgbId[0]);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, texture->w, texture->h, 0, TGL_RGB, TGL_UNSIGNED_BYTE, texture->getPixels());
	tglUploadBlitImage(_blitImageRgb, *texture, 0, false);
}

void TinyGLRenderer::loadTextureRGB565(Graphics::Surface *texture) {
	tglBindTexture(TGL_TEXTURE_2D, _textureRgb565Id[0]);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, texture->w, texture->h, 0, TGL_RGB, TGL_UNSIGNED_SHORT_5_6_5, texture->getPixels());
	tglUploadBlitImage(_blitImageRgb565, *texture, 0, false);
}

void TinyGLRenderer::loadTextureRGBA5551(Graphics::Surface *texture) {
	tglBindTexture(TGL_TEXTURE_2D, _textureRgba5551Id[0]);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, texture->w, texture->h, 0, TGL_RGBA, TGL_UNSIGNED_SHORT_5_5_5_1, texture->getPixels());
	tglUploadBlitImage(_blitImageRgba5551, *texture, 0, false);
}

void TinyGLRenderer::loadTextureRGBA4444(Graphics::Surface *texture) {
	tglBindTexture(TGL_TEXTURE_2D, _textureRgba4444Id[0]);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
	tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, texture->w, texture->h, 0, TGL_RGBA, TGL_UNSIGNED_SHORT_4_4_4_4, texture->getPixels());
	tglUploadBlitImage(_blitImageRgba4444, *texture, 0, false);
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
	Common::List<Common::Rect> dirtyAreas;
	TinyGL::presentBuffer(dirtyAreas);

	Graphics::Surface glBuffer;
	TinyGL::getSurfaceRef(glBuffer);

	if (!dirtyAreas.empty()) {
		for (Common::List<Common::Rect>::iterator itRect = dirtyAreas.begin(); itRect != dirtyAreas.end(); ++itRect) {
			g_system->copyRectToScreen(glBuffer.getBasePtr((*itRect).left, (*itRect).top), glBuffer.pitch,
			                           (*itRect).left, (*itRect).top, (*itRect).width(), (*itRect).height());
		}
	}
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

	tglEnableClientState(TGL_VERTEX_ARRAY);
	tglEnableClientState(TGL_TEXTURE_COORD_ARRAY);

	tglTranslatef(-0.799f, 0.8, 0);
	//tglTranslatef(-0.8, 0.8, 0); // some gfx issue

	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), bitmapVertices);
	tglTexCoordPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), textCords);
	tglBindTexture(TGL_TEXTURE_2D, _textureRgbaId[0]);
	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglTranslatef(0.5, 0, 0);

	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), bitmapVertices);
	tglTexCoordPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), textCords);
	tglBindTexture(TGL_TEXTURE_2D, _textureRgbId[0]);
	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglTranslatef(0.501, 0, 0);
	//tglTranslatef(0.5, 0, 0); // some gfx issue

	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), bitmapVertices);
	tglTexCoordPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), textCords);
	tglBindTexture(TGL_TEXTURE_2D, _textureRgb565Id[0]);
	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglTranslatef(0.5, 0, 0);

	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), bitmapVertices);
	tglTexCoordPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), textCords);
	tglBindTexture(TGL_TEXTURE_2D, _textureRgba5551Id[0]);
	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglTranslatef(-1.5, -0.5, 0);

	tglVertexPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), bitmapVertices);
	tglTexCoordPointer(2, TGL_FLOAT, 2 * sizeof(TGLfloat), textCords);
	tglBindTexture(TGL_TEXTURE_2D, _textureRgba4444Id[0]);
	tglDrawArrays(TGL_TRIANGLE_STRIP, 0, 4);

	tglDisableClientState(TGL_VERTEX_ARRAY);
	tglDisableClientState(TGL_TEXTURE_COORD_ARRAY);

	int blitTextureWidth, blitTextureHeight;
	tglGetBlitImageSize(_blitImageRgba, blitTextureWidth, blitTextureHeight);

	TinyGL::BlitTransform transform(0, 250);
	transform.sourceRectangle(0, 0, blitTextureWidth, blitTextureHeight);
	tglBlit(_blitImageRgba, transform);
	
	transform = TinyGL::BlitTransform(130, 250);
	transform.sourceRectangle(0, 0, blitTextureWidth, blitTextureHeight);
	tglBlit(_blitImageRgb, transform);

	transform = TinyGL::BlitTransform(260, 250);
	transform.sourceRectangle(0, 0, blitTextureWidth, blitTextureHeight);
	tglBlit(_blitImageRgb565, transform);

	transform = TinyGL::BlitTransform(390, 250);
	transform.sourceRectangle(0, 0, blitTextureWidth, blitTextureHeight);
	tglBlit(_blitImageRgba5551, transform);

	transform = TinyGL::BlitTransform(520, 250);
	transform.sourceRectangle(0, 0, blitTextureWidth, blitTextureHeight);
	tglBlit(_blitImageRgba4444, transform);

	tglMatrixMode(TGL_MODELVIEW);
	tglPopMatrix();

	tglMatrixMode(TGL_PROJECTION);
	tglPopMatrix();
}

} // End of namespace Playground3d
