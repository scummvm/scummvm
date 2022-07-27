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


#include "common/config-manager.h"
#include "graphics/renderer.h"
#include "graphics/surface.h"
#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
#include "graphics/opengl/context.h"
#endif

#include "math/glmath.h"

#include "engines/freescape/gfx.h"
#include "engines/util.h"

namespace Freescape {

Renderer::Renderer(OSystem *system)
		: _system(system) {

	_currentPixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	_originalPixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	_palettePixelFormat = Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
	_keyColor = -1;
}

Renderer::~Renderer() {}

Graphics::Surface *Renderer::convertFromPalette(Graphics::PixelBuffer *rawsurf) {
	Graphics::Surface * surf = new Graphics::Surface();
	surf->create(kOriginalWidth, kOriginalHeight, _originalPixelFormat);
	surf->copyRectToSurface(rawsurf->getRawBuffer(), surf->w, 0, 0, surf->w, surf->h);
	surf->convertToInPlace(_currentPixelFormat, _palette->getRawBuffer());
	return surf;
}

Common::Rect Renderer::getFontCharacterRect(uint8 character) {
	uint index = 0;

	if (character == ' ')
		index = 0;
	else if (character >= '0' && character <= '9')
		index = 1 + character - '0';
	else if (character >= 'A' && character <= 'Z')
		index = 1 + 10 + character - 'A';
	else if (character == '|')
		index = 1 + 10 + 26;
	else if (character == '/')
		index = 2 + 10 + 26;
	else if (character == ':')
		index = 3 + 10 + 26;

	return Common::Rect(16 * index, 0, 16 * (index + 1), 32);
}

Common::Rect Renderer::viewport() const {
	return _screenViewport;
}

void Renderer::computeScreenViewport() {
	int32 screenWidth = _system->getWidth();
	int32 screenHeight = _system->getHeight();
	_screenViewport = Common::Rect(screenWidth, screenHeight);
}

Renderer *createRenderer(OSystem *system) {
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	Graphics::RendererType desiredRendererType = Graphics::kRendererTypeTinyGL; //Graphics::parseRendererTypeCode(rendererConfig);
	Graphics::RendererType matchingRendererType = Graphics::kRendererTypeTinyGL; //Graphics::getBestMatchingAvailableRendererType(desiredRendererType);

	bool isAccelerated = 0; //matchingRendererType != Graphics::kRendererTypeTinyGL;

	uint width = Renderer::kOriginalWidth;
	uint height = Renderer::kOriginalHeight;

	if (isAccelerated) {
		initGraphics3d(width, height);
	} else {
		initGraphics(width, height, &pixelFormat);
	}

/*#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
	bool backendCapableOpenGL = g_system->hasFeature(OSystem::kFeatureOpenGLForGame);
#endif

#if defined(USE_OPENGL_GAME)
	// Check the OpenGL context actually supports shaders
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGLShaders && !OpenGLContext.shadersSupported) {
		matchingRendererType = Graphics::kRendererTypeOpenGL;
	}
#endif*/

	if (matchingRendererType != desiredRendererType && desiredRendererType != Graphics::kRendererTypeDefault) {
		// Display a warning if unable to use the desired renderer
		warning("Unable to create a '%s' renderer", rendererConfig.c_str());
	}
/*
#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGLShaders) {
		return CreateGfxOpenGLShader(system);
	}
#endif
#if defined(USE_OPENGL_GAME) && !defined(USE_GLES2)
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGL) {
		return CreateGfxOpenGL(system);
	}
#endif*/
	if (matchingRendererType == Graphics::kRendererTypeTinyGL) {
		return CreateGfxTinyGL(system);
	}

	error("Unable to create a '%s' renderer", rendererConfig.c_str());
}

} // End of namespace Freescape
