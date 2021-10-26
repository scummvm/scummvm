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

#include "common/rect.h"
#include "common/textconsole.h"

#if defined(USE_OPENGL_GAME) && !defined(USE_GLES2)

#include "graphics/opengl/context.h"
#include "graphics/surface.h"

#include "engines/playground3d/gfx.h"
#include "engines/playground3d/gfx_opengl.h"

namespace Playground3d {

Renderer *CreateGfxOpenGL(OSystem *system) {
	return new OpenGLRenderer(system);
}

OpenGLRenderer::OpenGLRenderer(OSystem *system) :
		Renderer(system) {
}

OpenGLRenderer::~OpenGLRenderer() {
}

void OpenGLRenderer::init() {
	debug("Initializing OpenGL Renderer");

	computeScreenViewport();

#if defined(USE_OPENGL_SHADERS)
	// The ShaderSurfaceRenderer sets an array buffer which conflict with fixed pipeline rendering
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif // defined(USE_OPENGL_SHADERS)

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::clear() {
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::drawFace(uint face) {
	glBegin(GL_TRIANGLE_STRIP);
	for (uint i = 0; i < 4; i++) {
		glColor3f(cubeVertices[11 * (4 * face + i) + 8], cubeVertices[11 * (4 * face + i) + 9], cubeVertices[11 * (4 * face + i) + 10]);
		glVertex3f(cubeVertices[11 * (4 * face + i) + 2], cubeVertices[11 * (4 * face + i) + 3], cubeVertices[11 * (4 * face + i) + 4]);
		glNormal3f(cubeVertices[11 * (4 * face + i) + 5], cubeVertices[11 * (4 * face + i) + 6], cubeVertices[11 * (4 * face + i) + 7]);
	}
	glEnd();
}

void OpenGLRenderer::drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	Common::Rect vp = viewport();
	glViewport(vp.left, _system->getHeight() - vp.top - vp.height(), vp.width(), vp.height());

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(_projectionMatrix.getData());

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(_modelViewMatrix.getData());

	glTranslatef(pos.x(), pos.y(), pos.z());
	glRotatef(roll.x(), 1.0f, 0.0f, 0.0f);
	glRotatef(roll.y(), 0.0f, 1.0f, 0.0f);
	glRotatef(roll.z(), 0.0f, 0.0f, 1.0f);

	for (uint i = 0; i < 6; i++) {
		drawFace(i);
	}
}

void OpenGLRenderer::drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	Common::Rect vp = viewport();
	glViewport(vp.left, _system->getHeight() - vp.top - vp.height(), vp.width(), vp.height());

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(_projectionMatrix.getData());

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(_modelViewMatrix.getData());

	glTranslatef(pos.x(), pos.y(), pos.z());
	glRotatef(roll.y(), 0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_TRIANGLES);
	glVertex3f(-1.0f,  1.0, 0.0f);
	glVertex3f( 1.0f,  1.0, 0.0f);
	glVertex3f( 0.0f, -1.0, 0.0f);
	glEnd();

	glPolygonOffset(-1.0f, 0.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	glVertex3f(-0.5f,  0.5, 0.0f);
	glVertex3f( 0.5f,  0.5, 0.0f);
	glVertex3f( 0.0f, -0.5, 0.0f);
	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);
}

} // End of namespace Playground3d

#endif
