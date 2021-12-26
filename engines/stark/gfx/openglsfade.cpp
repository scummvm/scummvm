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

#include "engines/stark/gfx/openglsfade.h"

#include "engines/stark/gfx/opengls.h"

#if defined(USE_OPENGL_SHADERS)

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

#endif // defined(USE_OPENGL_SHADERS)
