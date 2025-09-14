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

#include "alcachofa/graphics-opengl-base.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "graphics/renderer.h"

using namespace Common;
using namespace Graphics;
using namespace Math;

namespace Alcachofa {

static bool areComponentsInOrder(const PixelFormat &format, int r, int g, int b, int a) {
	return format == PixelFormat(4, 8, 8, 8, 8, r * 8, g * 8, b * 8, a * 8);
}

bool isCompatibleFormat(const PixelFormat &format) {
	return areComponentsInOrder(format, 0, 1, 2, 3) ||
		areComponentsInOrder(format, 3, 2, 1, 0);
}

OpenGLTextureBase::OpenGLTextureBase(int32 w, int32 h, bool withMipmaps)
	: ITexture({ (int16)w, (int16)h })
	, _withMipmaps(withMipmaps) {}

void OpenGLTextureBase::update(const Surface &surface) {
	assert(isCompatibleFormat(surface.format));
	assert(surface.w == size().x && surface.h == size().y);

	// GLES2 only supports GL_RGBA but we need BlendBlit::getSupportedPixelFormat to use blendBlit
	// We also do not want to keep surface memory for textures that are not updated repeatedly
	const void *pixels;
	if (!areComponentsInOrder(surface.format, 0, 1, 2, 3)) {
		if (_tmpSurface.empty())
			_tmpSurface.create(surface.w, surface.h, PixelFormat::createFormatRGBA32());
		crossBlit(
			(byte *)_tmpSurface.getPixels(),
			(const byte *)surface.getPixels(),
			_tmpSurface.pitch,
			surface.pitch,
			surface.w,
			surface.h,
			_tmpSurface.format,
			surface.format);
		pixels = _tmpSurface.getPixels();
	} else {
		pixels = surface.getPixels();
	}

	updateInner(pixels);

	if (!_tmpSurface.empty()) {
		if (!_didConvertOnce)
			_tmpSurface.free();
		_didConvertOnce = true;
	}
}

OpenGLRendererBase::OpenGLRendererBase(Point resolution) : _resolution(resolution) {}

bool OpenGLRendererBase::hasOutput() const {
	return _currentOutput != nullptr;
}

void OpenGLRendererBase::resetState() {
	setViewportToScreen();
	_currentOutput = nullptr;
	_currentLodBias = -1000.0f;
	_currentBlendMode = (BlendMode)-1;
	_isFirstDrawCommand = true;
}

void OpenGLRendererBase::setViewportToScreen() {
	int32 screenWidth = g_system->getWidth();
	int32 screenHeight = g_system->getHeight();
	Rect viewport(
		MIN<int32>(screenWidth, screenHeight * (float)_resolution.x / _resolution.y),
		MIN<int32>(screenHeight, screenWidth * (float)_resolution.y / _resolution.x));
	viewport.translate(
		(screenWidth - viewport.width()) / 2,
		(screenHeight - viewport.height()) / 2);

	setViewportInner(viewport.left, viewport.top, viewport.width(), viewport.height());
	setMatrices(true);
}

void OpenGLRendererBase::setViewportToRect(int16 outputWidth, int16 outputHeight) {
	_outputSize.x = MIN(outputWidth, g_system->getWidth());
	_outputSize.y = MIN(outputHeight, g_system->getHeight());
	setViewportInner(0, 0, _outputSize.x, _outputSize.y);
	setMatrices(false);
}

void OpenGLRendererBase::getQuadPositions(Vector2d topLeft, Vector2d size, Angle rotation, Vector2d positions[]) const {
	positions[0] = topLeft + Vector2d(0, 0);
	positions[1] = topLeft + Vector2d(0, +size.getY());
	positions[2] = topLeft + Vector2d(+size.getX(), +size.getY());
	positions[3] = topLeft + Vector2d(+size.getX(), 0);
	if (abs(rotation.getDegrees()) > epsilon) {
		const Vector2d zero(0, 0);
		for (int i = 0; i < 4; i++)
			positions[i].rotateAround(zero, rotation);
	}
}

void OpenGLRendererBase::getQuadTexCoords(Vector2d texMin, Vector2d texMax, Vector2d texCoords[]) const {
	texCoords[0] = { texMin.getX(), texMin.getY() };
	texCoords[1] = { texMin.getX(), texMax.getY() };
	texCoords[2] = { texMax.getX(), texMax.getY() };
	texCoords[3] = { texMax.getX(), texMin.getY() };
}

IRenderer *IRenderer::createOpenGLRenderer(Point resolution) {
	const auto available = Renderer::getAvailableTypes();
	const auto &rendererCode = ConfMan.get("renderer");
	RendererType rendererType = Renderer::parseTypeCode(rendererCode);
	rendererType = (RendererType)(rendererType & available);

	IRenderer *renderer = nullptr;
	switch (rendererType) {
	case kRendererTypeOpenGLShaders:
		renderer = createOpenGLRendererShaders(resolution);
		break;
	case kRendererTypeOpenGL:
		renderer = createOpenGLRendererClassic(resolution);
		break;
	case kRendererTypeTinyGL:
		renderer = createTinyGLRenderer(resolution);
		break;
	default:
		if (available & kRendererTypeOpenGLShaders)
			renderer = createOpenGLRendererShaders(resolution);
		else if (available & kRendererTypeOpenGL)
			renderer = createOpenGLRendererClassic(resolution);
		else if (available & kRendererTypeTinyGL)
			renderer = createTinyGLRenderer(resolution);
		break;
	}

	if (renderer == nullptr)
		error("Could not create a renderer, available: %d", (int)available);
	return renderer;
}

#ifndef USE_OPENGL_SHADERS
IRenderer *IRenderer::createOpenGLRendererShaders(Point _) {
	(void)_;
	return nullptr;
}
#endif

#ifndef USE_OPENGL_GAME
IRenderer *IRenderer::createOpenGLRendererClassic(Point _) {
	(void)_;
	return nullptr;
}
#endif

#ifndef USE_TINYGL
IRenderer *IRenderer::createTinyGLRenderer(Point _) {
	(void)_;
	return nullptr;
}
#endif

}
