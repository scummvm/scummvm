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

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#include "graphics/surface.h"

#include "math/glmath.h"
#include "math/vector2d.h"
#include "math/rect2d.h"
#include "math/quat.h"

#include "graphics/opengl/shader.h"

#include "engines/playground3d/gfx.h"
#include "engines/playground3d/gfx_opengl_shaders.h"

namespace Playground3d {

Renderer *CreateGfxOpenGLShader(OSystem *system) {
	return new ShaderRenderer(system);
}

ShaderRenderer::ShaderRenderer(OSystem *system) :
		Renderer(system),
		_currentViewport(kOriginalWidth, kOriginalHeight),
		_cubeShader(nullptr),
		_cubeVBO(0) {
}

ShaderRenderer::~ShaderRenderer() {
	OpenGL::ShaderGL::freeBuffer(_cubeVBO);

	delete _cubeShader;
}

void ShaderRenderer::init() {
	debug("Initializing OpenGL Renderer with shaders");

	computeScreenViewport();

	glEnable(GL_DEPTH_TEST);

	static const char* attributes[] = { "position", "normal", "color", "texcoord",  NULL };
	_cubeShader = OpenGL::ShaderGL::fromFiles("playground3d_cube", attributes);
	_cubeVBO = OpenGL::ShaderGL::createBuffer(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices);
	_cubeShader->enableVertexAttribute("texcoord", _cubeVBO, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 0);
	_cubeShader->enableVertexAttribute("position", _cubeVBO, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 8);
	_cubeShader->enableVertexAttribute("normal", _cubeVBO, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 20);
	_cubeShader->enableVertexAttribute("color", _cubeVBO, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 32);
}

void ShaderRenderer::clear() {
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ShaderRenderer::drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	Common::Rect vp = viewport();
	glViewport(vp.left, _system->getHeight() - vp.top - vp.height(), vp.width(), vp.height());

	auto rotateMatrix = (Math::Quaternion::fromEuler(roll.x(), roll.y(), roll.z(), Math::EO_XYZ)).inverse().toMatrix();
	_cubeShader->use();
	_cubeShader->setUniform("textured", false);
	_cubeShader->setUniform("mvpMatrix", _mvpMatrix);
	_cubeShader->setUniform("rotateMatrix", rotateMatrix);
	_cubeShader->setUniform("modelPos", pos);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
}

void ShaderRenderer::drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) {
	error("Polygon offset test not implemented yet");
}

} // End of namespace Playground3d

#endif
