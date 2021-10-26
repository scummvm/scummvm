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

Renderer *CreateGfxTinyGL(OSystem *system) {
	return new TinyGLRenderer(system);
}

TinyGLRenderer::TinyGLRenderer(OSystem *system) :
		Renderer(system),
		_fb(NULL) {
}

TinyGLRenderer::~TinyGLRenderer() {
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
	tglEnable(TGL_DEPTH_TEST);
}

void TinyGLRenderer::clear() {
	tglClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	tglClear(TGL_COLOR_BUFFER_BIT | TGL_DEPTH_BUFFER_BIT);
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
	Common::Rect vp = viewport();
	tglViewport(vp.left, _system->getHeight() - vp.top - vp.height(), vp.width(), vp.height());

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
	Common::Rect vp = viewport();
	tglViewport(vp.left, _system->getHeight() - vp.top - vp.height(), vp.width(), vp.height());

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

} // End of namespace Playground3d
