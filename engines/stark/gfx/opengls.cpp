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

#include "engines/stark/gfx/opengls.h"

#include "common/system.h"

#include "math/matrix4.h"

#if defined(USE_OPENGL_SHADERS)

#include "engines/stark/gfx/openglsactor.h"
#include "engines/stark/gfx/openglsprop.h"
#include "engines/stark/gfx/openglssurface.h"
#include "engines/stark/gfx/openglsfade.h"
#include "engines/stark/gfx/opengltexture.h"

#include "graphics/surface.h"
#include "graphics/opengl/shader.h"

namespace Stark {
namespace Gfx {

static const GLfloat surfaceVertices[] = {
	// XS   YT
	0.0f, 0.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
};

static const GLfloat fadeVertices[] = {
	// XS   YT
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	-1.0f, -1.0f,
	 1.0f, -1.0f,
};

OpenGLSDriver::OpenGLSDriver() :
	_surfaceShader(nullptr),
	_actorShader(nullptr),
	_fadeShader(nullptr),
	_shadowShader(nullptr),
	_surfaceVBO(0),
	_fadeVBO(0) {
}

OpenGLSDriver::~OpenGLSDriver() {
	OpenGL::Shader::freeBuffer(_surfaceVBO);
	OpenGL::Shader::freeBuffer(_fadeVBO);
	delete _surfaceShader;
	delete _actorShader;
	delete _fadeShader;
	delete _shadowShader;
}

void OpenGLSDriver::init() {
	computeScreenViewport();

	static const char* attributes[] = { "position", "texcoord", nullptr };
	_surfaceShader = OpenGL::Shader::fromFiles("stark_surface", attributes);
	_surfaceVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(surfaceVertices), surfaceVertices);
	_surfaceShader->enableVertexAttribute("position", _surfaceVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
	_surfaceShader->enableVertexAttribute("texcoord", _surfaceVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);

	static const char* actorAttributes[] = { "position1", "position2", "bone1", "bone2", "boneWeight", "normal", "texcoord", nullptr };
	_actorShader = OpenGL::Shader::fromFiles("stark_actor", actorAttributes);

	static const char* shadowAttributes[] = { "position1", "position2", "bone1", "bone2", "boneWeight", nullptr };
	_shadowShader = OpenGL::Shader::fromFiles("stark_shadow", shadowAttributes);

	static const char* fadeAttributes[] = { "position", nullptr };
	_fadeShader = OpenGL::Shader::fromFiles("stark_fade", fadeAttributes);
	_fadeVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(fadeVertices), fadeVertices);
	_fadeShader->enableVertexAttribute("position", _fadeVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
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

void OpenGLSDriver::setViewport(const Common::Rect &rect) {
	_viewport = Common::Rect(
			_screenViewport.width() * rect.width() / kOriginalWidth,
			_screenViewport.height() * rect.height() / kOriginalHeight
			);

	_viewport.translate(
			_screenViewport.left + _screenViewport.width() * rect.left / kOriginalWidth,
			_screenViewport.top + _screenViewport.height() * rect.top / kOriginalHeight
			);

	_unscaledViewport = rect;

	glViewport(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height());
}

void OpenGLSDriver::clearScreen() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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

Texture *OpenGLSDriver::createBitmap(const Graphics::Surface *surface, const byte *palette) {
	return createTexture(surface, palette);
}

VisualActor *OpenGLSDriver::createActorRenderer() {
	return new OpenGLSActorRenderer(this);
}

VisualProp *OpenGLSDriver::createPropRenderer() {
	return new OpenGLSPropRenderer(this);
}

SurfaceRenderer *OpenGLSDriver::createSurfaceRenderer() {
	return new OpenGLSSurfaceRenderer(this);
}

FadeRenderer *OpenGLSDriver::createFadeRenderer() {
	return new OpenGLSFadeRenderer(this);
}

void OpenGLSDriver::start2DMode() {
	// Enable alpha blending
	glEnable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD); // It's the default

	// This blend mode prevents color fringes due to filtering.
	// It requires the textures to have their color values pre-multiplied
	// with their alpha value. This is the "Premultiplied Alpha" technique.
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

void OpenGLSDriver::end2DMode() {
	// Disable alpha blending
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void OpenGLSDriver::set3DMode() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Blending and stencil test are only used in rendering shadows
	// They are manually enabled and disabled there
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glStencilFunc(GL_EQUAL, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
}

bool OpenGLSDriver::computeLightsEnabled() {
	return false;
}

Common::Rect OpenGLSDriver::getViewport() const {
	return _viewport;
}

Common::Rect OpenGLSDriver::getUnscaledViewport() const {
	return _unscaledViewport;
}

OpenGL::Shader *OpenGLSDriver::createActorShaderInstance() {
	return _actorShader->clone();
}

OpenGL::Shader *OpenGLSDriver::createSurfaceShaderInstance() {
	return _surfaceShader->clone();
}

OpenGL::Shader *OpenGLSDriver::createFadeShaderInstance() {
	return _fadeShader->clone();
}

OpenGL::Shader *OpenGLSDriver::createShadowShaderInstance() {
	return _shadowShader->clone();
}

Graphics::Surface *OpenGLSDriver::getViewportScreenshot() const {
	Graphics::Surface *s = new Graphics::Surface();
	s->create(_viewport.width(), _viewport.height(), getRGBAPixelFormat());

	glReadPixels(_viewport.left, g_system->getHeight() - _viewport.bottom, _viewport.width(), _viewport.height(),
	             GL_RGBA, GL_UNSIGNED_BYTE, s->getPixels());

	flipVertical(s);

	return s;
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_OPENGL_SHADERS)
