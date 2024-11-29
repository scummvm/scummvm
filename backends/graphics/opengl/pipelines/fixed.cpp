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

#include "backends/graphics/opengl/pipelines/fixed.h"
#include "graphics/opengl/debug.h"

namespace OpenGL {

#if !USE_FORCED_GLES2
void FixedPipeline::activateInternal() {
	Pipeline::activateInternal();

	GL_CALL(glDisable(GL_LIGHTING));
	GL_CALL(glDisable(GL_FOG));
	GL_CALL(glShadeModel(GL_FLAT));
	GL_CALL(glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST));

	GL_CALL(glEnableClientState(GL_VERTEX_ARRAY));
	GL_CALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));

#if !USE_FORCED_GLES
	if (OpenGLContext.multitextureSupported) {
		GL_CALL(glActiveTexture(GL_TEXTURE0));
	}
#endif
	GL_CALL(glEnable(GL_TEXTURE_2D));
	GL_CALL(glColor4f(_r, _g, _b, _a));
}

void FixedPipeline::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	_r = r;
	_g = g;
	_b = b;
	_a = a;

	if (isActive()) {
		GL_CALL(glColor4f(r, g, b, a));
	}
}

void FixedPipeline::drawTextureInternal(const Texture &texture, const GLfloat *coordinates, const GLfloat *texcoords) {
	assert(isActive());

	texture.bind();

	GL_CALL(glTexCoordPointer(2, GL_FLOAT, 0, texcoords));
	GL_CALL(glVertexPointer(2, GL_FLOAT, 0, coordinates));
	GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

void FixedPipeline::setProjectionMatrix(const Math::Matrix4 &projectionMatrix) {
	assert(isActive());

	GL_CALL(glMatrixMode(GL_PROJECTION));
	GL_CALL(glLoadMatrixf(projectionMatrix.getData()));

	GL_CALL(glMatrixMode(GL_MODELVIEW));
	GL_CALL(glLoadIdentity());
}
#endif // !USE_FORCED_GLES2

} // End of namespace OpenGL
