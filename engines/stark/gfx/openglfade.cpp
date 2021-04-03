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

#include "engines/stark/gfx/openglfade.h"

#include "engines/stark/gfx/opengl.h"

#if defined(USE_OPENGL_GAME)

namespace Stark {
namespace Gfx {

static const GLfloat fadeVertices[] = {
	// X   Y
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	-1.0f, -1.0f,
	 1.0f, -1.0f,
};

OpenGLFadeRenderer::OpenGLFadeRenderer(OpenGLDriver *gfx) :
	FadeRenderer(),
	_gfx(gfx) {
}

OpenGLFadeRenderer::~OpenGLFadeRenderer() {
}

void OpenGLFadeRenderer::render(float fadeLevel) {
	_gfx->start2DMode();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);

	glColor4f(0.0f, 0.0f, 0.0f, 1.0f - fadeLevel);
	glVertexPointer(2, GL_FLOAT, 2 * sizeof(GLfloat), &fadeVertices[0]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	_gfx->end2DMode();
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_OPENGL_GAME)
