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

#include "engines/stark/gfx/openglsfade.h"

#include "engines/stark/gfx/opengls.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/shader.h"

namespace Stark {
namespace Gfx {

OpenGLSFadeRenderer::OpenGLSFadeRenderer(OpenGLSDriver *gfx) :
	FadeRenderer(),
	_gfx(gfx) {
	_shader = _gfx->createFadeShaderInstance();
}

OpenGLSFadeRenderer::~OpenGLSFadeRenderer() {
	delete _shader;
}

void OpenGLSFadeRenderer::render(float fadeLevel) {
	_gfx->start2DMode();

	_shader->use();
	_shader->setUniform1f("alphaLevel", 1.0 - fadeLevel);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	_shader->unbind();

	_gfx->end2DMode();
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
