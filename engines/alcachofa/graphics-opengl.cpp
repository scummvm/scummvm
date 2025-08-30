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

#include "alcachofa/graphics.h"
#include "alcachofa/detection.h"
#include "alcachofa/graphics-opengl.h"

#include "common/system.h"
#include "common/config-manager.h"
#include "engines/util.h"
#include "graphics/renderer.h"

using namespace Common;
using namespace Math;
using namespace Graphics;

namespace Alcachofa {

static bool areComponentsInOrder(const PixelFormat &format, int r, int g, int b, int a) {
	return format == PixelFormat(4, 8, 8, 8, 8, r * 8, g * 8, b * 8, a * 8);
}

static bool isCompatibleFormat(const PixelFormat &format) {
	return areComponentsInOrder(format, 0, 1, 2, 3) ||
		areComponentsInOrder(format, 3, 2, 1, 0);
}

OpenGLTexture::OpenGLTexture(int32 w, int32 h, bool withMipmaps)
	: ITexture({ (int16)w, (int16)h })
	, _withMipmaps(withMipmaps) {
	glEnable(GL_TEXTURE_2D); // will error on GLES2, but that is okay
	OpenGL::clearGLError(); // we will just ignore it
	GL_CALL(glGenTextures(1, &_handle));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, _handle));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	setMirrorWrap(false);
}

OpenGLTexture::~OpenGLTexture() {
	if (_handle != 0)
		GL_CALL(glDeleteTextures(1, &_handle));
}

void OpenGLTexture::update(const Surface &surface) {
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
		glEnable(GL_TEXTURE_2D);
		OpenGL::clearGLError();
		pixels = surface.getPixels();
	}

	GL_CALL(glBindTexture(GL_TEXTURE_2D, _handle));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface.w, surface.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
	if (_withMipmaps)
		GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
	else
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));

	if (!_tmpSurface.empty()) {
		if (!_didConvertOnce)
			_tmpSurface.free();
		_didConvertOnce = true;
	}
}

void OpenGLTexture::setMirrorWrap(bool wrap) {
	if (_mirrorWrap == wrap)
		return;
	_mirrorWrap = wrap;
	GLint wrapMode;
	if (wrap)
		wrapMode = OpenGLContext.textureMirrorRepeatSupported ? GL_MIRRORED_REPEAT : GL_REPEAT;
	else
		wrapMode = OpenGLContext.textureEdgeClampSupported ? GL_CLAMP_TO_EDGE : GL_CLAMP;

	GL_CALL(glBindTexture(GL_TEXTURE_2D, _handle));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode));
}

OpenGLRenderer::OpenGLRenderer(Point resolution) : _resolution(resolution) {
	initGraphics3d(resolution.x, resolution.y);
	GL_CALL(glDisable(GL_DEPTH_TEST));
	GL_CALL(glDisable(GL_SCISSOR_TEST));
	GL_CALL(glDisable(GL_STENCIL_TEST));
	GL_CALL(glDisable(GL_CULL_FACE));
	GL_CALL(glEnable(GL_BLEND));
	GL_CALL(glDepthMask(GL_FALSE));

	if (!OpenGLContext.NPOTSupported || !OpenGLContext.textureMirrorRepeatSupported) {
		g_system->messageBox(LogMessageType::kWarning, "Old OpenGL detected, some graphical errors will occur.");
	}
}

ScopedPtr<ITexture> OpenGLRenderer::createTexture(int32 w, int32 h, bool withMipmaps) {
	assert(w >= 0 && h >= 0);
	return ScopedPtr<ITexture>(new OpenGLTexture(w, h, withMipmaps));
}

void OpenGLRenderer::resetState() {
	setViewportToScreen();
	_currentOutput = nullptr;
	_currentLodBias = -1000.0f;
	_currentTexture = nullptr;
	_currentBlendMode = (BlendMode)-1;
	_isFirstDrawCommand = true;
}

void OpenGLRenderer::end() {
	GL_CALL(glFlush());

	if (_currentOutput != nullptr) {
		g_system->presentBuffer();
		GL_CALL(glReadPixels(
			0,
			0,
			_outputSize.x,
			_outputSize.y,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			_currentOutput->getPixels()
		));
		if (_currentOutput->format != PixelFormat::createFormatRGBA32()) {
			auto targetFormat = _currentOutput->format;
			_currentOutput->format = PixelFormat::createFormatRGBA32();
			_currentOutput->convertToInPlace(targetFormat);
		}
	}
}

void OpenGLRenderer::setBlendFunc(BlendMode blendMode) {
	switch (blendMode) {
	case BlendMode::AdditiveAlpha:
		GL_CALL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
		break;
	case BlendMode::Additive:
		GL_CALL(glBlendFunc(GL_ONE, GL_ONE));
		break;
	case BlendMode::Multiply:
		GL_CALL(glBlendFunc(GL_DST_COLOR, GL_ONE));
		break;
	case BlendMode::Alpha:
		GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		break;
	case BlendMode::Tinted:
		GL_CALL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
		break;
	default: assert(false && "Invalid blend mode"); break;
	}
}

void OpenGLRenderer::setOutput(Surface &output) {
	assert(_isFirstDrawCommand);
	setViewportToRect(output.w, output.h);
	_currentOutput = &output;

	// just debug warnings as it will only produce a graphical glitch while
	// there is some chance the resolution could change from here to ::end
	// and this is per-frame so maybe don't spam the console with the same message

	if (output.w > g_system->getWidth() || output.h > g_system->getHeight())
		debugC(0, kDebugGraphics, "Output is larger than screen, output will be cropped (%d, %d) > (%d, %d)",
			output.w, output.h, g_system->getWidth(), g_system->getHeight());

	if (!isCompatibleFormat(output.format)) {
		auto formatString = output.format.toString();
		debugC(0, kDebugGraphics, "Cannot use pixelformat of given output surface: %s", formatString.c_str());
		_currentOutput = nullptr;
	}

	if (output.pitch != output.format.bytesPerPixel * output.w) {
		// Maybe there would be a way with glPixelStore
		debugC(0, kDebugGraphics, "Incompatible output surface pitch");
		_currentOutput = nullptr;
	}
}

bool OpenGLRenderer::hasOutput() const {
	return _currentOutput != nullptr;
}

void OpenGLRenderer::setViewportToScreen() {
	int32 screenWidth = g_system->getWidth();
	int32 screenHeight = g_system->getHeight();
	Rect viewport(
		MIN<int32>(screenWidth, screenHeight * (float)_resolution.x / _resolution.y),
		MIN<int32>(screenHeight, screenWidth * (float)_resolution.y / _resolution.x));
	viewport.translate(
		(screenWidth - viewport.width()) / 2,
		(screenHeight - viewport.height()) / 2);

	GL_CALL(glViewport(viewport.left, viewport.top, viewport.width(), viewport.height()));
	setMatrices(true);
}

void OpenGLRenderer::setViewportToRect(int16 outputWidth, int16 outputHeight) {
	_outputSize.x = MIN(outputWidth, g_system->getWidth());
	_outputSize.y = MIN(outputHeight, g_system->getHeight());
	GL_CALL(glViewport(0, 0, _outputSize.x, _outputSize.y));
	setMatrices(false);
}

void OpenGLRenderer::checkFirstDrawCommand() {
	// We delay clearing the screen. It is much easier for the game
		// to switch to a framebuffer before
	if (!_isFirstDrawCommand)
		return;
	_isFirstDrawCommand = false;
	GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void OpenGLRenderer::getQuadPositions(Vector2d topLeft, Vector2d size, Angle rotation, Vector2d positions[]) const {
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

void OpenGLRenderer::getQuadTexCoords(Vector2d texMin, Vector2d texMax, Vector2d texCoords[]) const {
	texCoords[0] = { texMin.getX(), texMin.getY() };
	texCoords[1] = { texMin.getX(), texMax.getY() };
	texCoords[2] = { texMax.getX(), texMax.getY() };
	texCoords[3] = { texMax.getX(), texMin.getY() };
}

IRenderer *IRenderer::createOpenGLRenderer(Point resolution) {
	const auto available = Renderer::getAvailableTypes() & ~kRendererTypeTinyGL;
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
	default:
		if (available & kRendererTypeOpenGLShaders)
			renderer = createOpenGLRendererShaders(resolution);
		else if (available & kRendererTypeOpenGL)
			renderer = createOpenGLRendererClassic(resolution);
		break;
	}

	if (renderer == nullptr)
		error("Could not create a renderer, GL context type: %d", (int)OpenGLContext.type);
	return renderer;
}

#ifndef USE_OPENGL_SHADERS
IRenderer *createOpenGLRendererShaders(Point _) {
	(void)_;
	return nullptr;
}
#endif

#ifndef USE_OPENGL_GAME
IRenderer *createOpenGLRendererClassic(Point _) {
	(void)_;
	return nullptr;
}
#endif

}
