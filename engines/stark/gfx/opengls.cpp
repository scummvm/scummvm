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

#include "common/config-manager.h"
#include "common/system.h"

#include "math/matrix4.h"

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#include "engines/stark/gfx/openglsactor.h"
#include "engines/stark/gfx/openglsprop.h"
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
	delete _actorShader;
}

void OpenGLSDriver::init() {
	bool fullscreen = ConfMan.getBool("fullscreen");
	g_system->setupScreen(kOriginalWidth, kOriginalHeight, fullscreen, true);

	computeScreenViewport();

	static const char* attributes[] = { "position", "texcoord", nullptr };
	_boxShader = Graphics::Shader::fromFiles("stark_box", attributes);
	_boxVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices);
	_boxShader->enableVertexAttribute("position", _boxVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
	_boxShader->enableVertexAttribute("texcoord", _boxVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);

	static const char* actorAttributes[] = { "position1", "position2", "bone1", "bone2", "boneWeight", "normal", "texcoord", nullptr };
	_actorShader = Graphics::Shader::fromFiles("stark_actor", actorAttributes);
}

void OpenGLSDriver::setScreenViewport(bool noScaling) {
	if (noScaling) {
		_viewport = Common::Rect(g_system->getWidth(), g_system->getHeight());
		_unscaledViewport = _viewport;
	} else {
		_viewport = _screenViewport;
		_unscaledViewport = Common::Rect(kOriginalWidth, kOriginalHeight);
	}

	glViewport(_viewport.left, _viewport.top, _viewport.width(), _viewport.height());
}

void OpenGLSDriver::setViewport(Common::Rect rect, bool noScaling) {
	if (noScaling) {
		_viewport = rect;
		_unscaledViewport = rect;
	} else {
		_viewport = Common::Rect(
				_screenViewport.width() * rect.width() / kOriginalWidth,
				_screenViewport.height() * rect.height() / kOriginalHeight
				);

		_viewport.translate(
				_screenViewport.left + _screenViewport.width() * rect.left / kOriginalWidth,
				_screenViewport.top + _screenViewport.height() * rect.top / kOriginalHeight
				);

		_unscaledViewport = rect;
	}

	glViewport(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height());
}

Math::Vector2d OpenGLSDriver::scaled(float x, float y) const {
	return Math::Vector2d(x / (float) _unscaledViewport.width(), y / (float) _unscaledViewport.height());
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
	return new OpenGLSActorRenderer(this);
}

VisualProp *OpenGLSDriver::createPropRenderer() {
	return new OpenGLSPropRenderer(this);
}

void OpenGLSDriver::drawSurface(const Texture *texture, const Common::Point &dest) {
	// Source texture rectangle
	const float tLeft = 0.0;
	const float tWidth = 1.0;
	const float tTop = 0.0;
	const float tHeight = 1.0;

	// Destination rectangle
	const float sLeft = dest.x;
	const float sTop = dest.y;
	const float sWidth = texture->width();
	const float sHeight = texture->height();

	start2DMode();

	_boxShader->use();
	_boxShader->setUniform("textured", true);
	_boxShader->setUniform("color", Math::Vector4d(1.0f, 1.0f, 1.0f, 1.0f));
	_boxShader->setUniform("verOffsetXY", scaled(sLeft, sTop));
	_boxShader->setUniform("verSizeWH", scaled(sWidth, sHeight));
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

Graphics::Shader *OpenGLSDriver::createActorShaderInstance() {
	return _actorShader->clone();
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
