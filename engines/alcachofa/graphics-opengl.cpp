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
#include "engines/util.h"

using namespace Common;
using namespace Math;
using namespace Graphics;

namespace Alcachofa {

static bool areComponentsInOrder(const PixelFormat &format, int r, int g, int b, int a) {
	return format == (a < 0
		? PixelFormat(3, 8, 8, 8, 0, r * 8, g * 8, b * 8, 0)
		: PixelFormat(4, 8, 8, 8, 8, r * 8, g * 8, b * 8, a * 8));
}

OpenGLFormat getOpenGLFormatOf(const PixelFormat &format) {
	if (areComponentsInOrder(format, 0, 1, 2, 3))
		return { GL_RGBA, GL_UNSIGNED_BYTE };
	else if (areComponentsInOrder(format, 3, 2, 1, 0))
		return { GL_RGBA, GL_UNSIGNED_INT_8_8_8_8 };
	else if (areComponentsInOrder(format, 0, 1, 2, -1))
		return { GL_RGB, GL_UNSIGNED_BYTE };
	else if (areComponentsInOrder(format, 2, 1, 0, 3))
		return { GL_BGRA, GL_UNSIGNED_BYTE };
	else if (areComponentsInOrder(format, 2, 1, 0, -1))
		return { GL_BGR, GL_UNSIGNED_BYTE };
	// we could look for packed formats here as well in the future
	else
		return { GL_NONE, GL_NONE };
}

OpenGLTexture::OpenGLTexture(int32 w, int32 h, bool withMipmaps)
	: ITexture({ (int16)w, (int16)h })
	, _withMipmaps(withMipmaps) {
	GL_CALL(glEnable(GL_TEXTURE_2D));
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
	OpenGLFormat format = getOpenGLFormatOf(surface.format);
	assert(surface.w == size().x && surface.h == size().y);
	assert(format.isValid());

	GL_CALL(glEnable(GL_TEXTURE_2D));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, _handle));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface.w, surface.h, 0, format._format, format._type, surface.getPixels()));
	if (_withMipmaps)
		GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
	else
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));
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

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode));
}

OpenGLRenderer::OpenGLRenderer(Point resolution) : _resolution(resolution) {
	GL_CALL(glDisable(GL_LIGHTING));
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
		auto format = getOpenGLFormatOf(_currentOutput->format);
		GL_CALL(glReadPixels(
			0,
			0,
			_outputSize.x,
			_outputSize.y,
			format._format,
			format._type,
			_currentOutput->getPixels()
		));
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

	auto format = getOpenGLFormatOf(output.format);
	if (format._format == GL_NONE) {
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

}
