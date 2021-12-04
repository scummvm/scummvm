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

#include "engines/playground3d/gfx.h"

#include "engines/util.h"

#include "common/config-manager.h"

#include "graphics/renderer.h"
#include "graphics/surface.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
#include "graphics/opengl/context.h"
#endif

#include "math/glmath.h"

namespace Playground3d {

const float Renderer::cubeVertices[] = {
	// S     T      X      Y      Z      NX    NY     NZ     R     G     B
	0.0f, 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 1.0f, // blue
	1.0f, 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 1.0f, // magenta
	0.0f, 0.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 1.0f, // cyan
	1.0f, 0.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 1.0f, // white

	0.0f, 1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, 0.0f, // red
	1.0f, 1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // black
	0.0f, 0.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, 0.0f, // yellow
	1.0f, 0.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // green

	0.0f, 1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f, // magenta
	1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, // red
	0.0f, 0.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f, // white
	1.0f, 0.0f,  1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f, // yellow

	0.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, // black
	1.0f, 1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 1.0f, // blue
	0.0f, 0.0f, -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, // green
	1.0f, 0.0f, -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 1.0f, // cyan

	0.0f, 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 1.0f, // cyan
	1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, 1.0f, // white
	0.0f, 0.0f, -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, // green
	1.0f, 0.0f,  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, 0.0f, // yellow

	0.0f, 1.0f, -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f, // black
	1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 0.0f, // red
	0.0f, 0.0f, -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 1.0f, // blue
	1.0f, 0.0f,  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 1.0f  // magenta
};

Renderer::Renderer(OSystem *system)
		: _system(system) {
}

Renderer::~Renderer() {
}

Common::Rect Renderer::viewport() const {
	return _screenViewport;
}

void Renderer::computeScreenViewport() {
	int32 screenWidth = _system->getWidth();
	int32 screenHeight = _system->getHeight();

	// Aspect ratio correction
	int32 viewportWidth = MIN<int32>(screenWidth, screenHeight * kOriginalWidth / kOriginalHeight);
	int32 viewportHeight = MIN<int32>(screenHeight, screenWidth * kOriginalHeight / kOriginalWidth);
	_screenViewport = Common::Rect(viewportWidth, viewportHeight);

	// Pillarboxing
	_screenViewport.translate((screenWidth - viewportWidth) / 2, (screenHeight - viewportHeight) / 2);
}

Math::Matrix4 Renderer::makeProjectionMatrix(float fov, float nearClip, float farClip) const {
	float aspectRatio = kOriginalWidth / (float) kOriginalHeight;
	float xmaxValue = nearClip * tanf(fov * M_PI / 360.0f);
	float ymaxValue = xmaxValue / aspectRatio;
	return Math::makeFrustumMatrix(-xmaxValue, xmaxValue, -ymaxValue, ymaxValue, nearClip, farClip);
}

void Renderer::setupCameraPerspective(float pitch, float heading, float fov) {
	_projectionMatrix = makeProjectionMatrix(fov, 1.0f, 10000.0f);
	_modelViewMatrix = Math::Matrix4(180.0f - heading, pitch, 0.0f, Math::EO_XYZ);
	Math::Matrix4 proj = _projectionMatrix;
	Math::Matrix4 model = _modelViewMatrix;
	proj.transpose();
	model.transpose();
	_mvpMatrix = proj * model;
	_mvpMatrix.transpose();
}

Renderer *createRenderer(OSystem *system) {
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::parseRendererTypeCode(rendererConfig);
	Graphics::RendererType matchingRendererType = Graphics::getBestMatchingAvailableRendererType(desiredRendererType);

	bool isAccelerated = matchingRendererType != Graphics::kRendererTypeTinyGL;

	uint width = Renderer::kOriginalWidth;
	uint height = Renderer::kOriginalHeight;

	if (isAccelerated) {
		initGraphics3d(width, height);
	} else {
		initGraphics(width, height, nullptr);
	}

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
	bool backendCapableOpenGL = g_system->hasFeature(OSystem::kFeatureOpenGLForGame);
#endif

#if defined(USE_OPENGL_GAME)
	// Check the OpenGL context actually supports shaders
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGLShaders && !OpenGLContext.shadersSupported) {
		matchingRendererType = Graphics::kRendererTypeOpenGL;
	}
#endif

	if (matchingRendererType != desiredRendererType && desiredRendererType != Graphics::kRendererTypeDefault) {
		// Display a warning if unable to use the desired renderer
		warning("Unable to create a '%s' renderer", rendererConfig.c_str());
	}

#if defined(USE_OPENGL_SHADERS)
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGLShaders) {
		return CreateGfxOpenGLShader(system);
	}
#endif
#if defined(USE_OPENGL_GAME)
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGL) {
		return CreateGfxOpenGL(system);
	}
#endif
#if defined(USE_TINYGL)
	if (matchingRendererType == Graphics::kRendererTypeTinyGL) {
		return CreateGfxTinyGL(system);
	}
#endif

	error("Unable to create a '%s' renderer", rendererConfig.c_str());
}

} // End of namespace Playground3d
