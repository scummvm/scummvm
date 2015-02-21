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

#include "engines/stark/gfx/opengls.h"

#include "common/system.h"

#include "math/matrix4.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#include "engines/stark/gfx/openglsactor.h"
#include "engines/stark/gfx/opengltexture.h"

#include "graphics/pixelbuffer.h"
#include "graphics/opengles2/shader.h"

namespace Stark {
namespace Gfx {

static const GLfloat boxVertices[] = {
	// XS   YT
	0.0, 0.0,
	1.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
};

OpenGLSDriver::OpenGLSDriver() :
	_boxShader(nullptr),
	_boxVBO(0) {
}

OpenGLSDriver::~OpenGLSDriver() {
	Graphics::Shader::freeBuffer(_boxVBO);
	delete _boxShader;
}

void OpenGLSDriver::setupScreen(int screenW, int screenH, bool fullscreen) {
	g_system->setupScreen(screenW, screenH, fullscreen, true);

	_screenWidth = screenW;
	_screenHeight = screenH;

	static const char* attributes[] = { "position", "texcoord", nullptr };
	_boxShader = Graphics::Shader::fromFiles("stark_box", attributes);
	_boxVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices);
	_boxShader->enableVertexAttribute("position", _boxVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
	_boxShader->enableVertexAttribute("texcoord", _boxVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
}

void OpenGLSDriver::setGameViewport() {
	_viewport = Common::Rect(kGameViewportWidth, kGameViewportHeight);
	_viewport.translate(0, _screenHeight - kGameViewportHeight - kTopBorderHeight);

	glViewport(_viewport.left, _viewport.top, _viewport.width(), _viewport.height());
}

void OpenGLSDriver::setScreenViewport() {
	_viewport = Common::Rect(_screenWidth, _screenHeight);

	glViewport(_viewport.left, _viewport.top, _viewport.width(), _viewport.height());
}

void OpenGLSDriver::setupCamera(const Math::Matrix4 &projection, const Math::Matrix4 &view) {
}

void OpenGLSDriver::clearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLSDriver::flipBuffer() {
	g_system->updateScreen();
}

Texture *OpenGLSDriver::createTexture(const Graphics::Surface *surface, const byte *palette) {
	OpenGlTexture *texture = new OpenGlTexture();

	if (surface) {
		texture->update(surface, palette);
	}

	return texture;
}

VisualActor *OpenGLSDriver::createActorRenderer() {
	return new OpenGLSActorRenderer();
}

void OpenGLSDriver::drawSurface(const Texture *texture, const Common::Point &dest) {
	// Source texture rectangle
	const float tLeft = 0.0;
	const float tWidth = 1.0;
	const float tTop = 0.0;
	const float tHeight = 1.0;

	// Destination rectangle
	const float sLeft = dest.x / (float) _viewport.width();
	const float sTop = dest.y / (float) _viewport.height();
	const float sWidth = texture->width() / (float) _viewport.width();
	const float sHeight = texture->height() / (float) _viewport.height();

	start2DMode();

	_boxShader->use();
	_boxShader->setUniform("textured", true);
	_boxShader->setUniform("color", Math::Vector4d(1.0f, 1.0f, 1.0f, 1.0f));
	_boxShader->setUniform("verOffsetXY", Math::Vector2d(sLeft, sTop));
	_boxShader->setUniform("verSizeWH", Math::Vector2d(sWidth, sHeight));
	_boxShader->setUniform("texOffsetXY", Math::Vector2d(tLeft, tTop));
	_boxShader->setUniform("texSizeWH", Math::Vector2d(tWidth, tHeight));

	texture->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	end2DMode();
}

void OpenGLSDriver::start2DMode() {
	// Enable alpha blending
	glEnable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD); // It's the default
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthMask(GL_FALSE);
}

void OpenGLSDriver::end2DMode() {
	// Disable alpha blending
	glDisable(GL_BLEND);

	glDepthMask(GL_TRUE);
}

void OpenGLSDriver::set3DMode() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
