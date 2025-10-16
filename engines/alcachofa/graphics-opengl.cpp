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

#include "alcachofa/alcachofa.h"
#include "alcachofa/graphics.h"
#include "alcachofa/detection.h"
#include "alcachofa/graphics-opengl.h"

#include "common/system.h"
#include "common/translation.h"
#include "common/config-manager.h"
#include "engines/util.h"
#include "graphics/renderer.h"
#include "gui/error.h"

using namespace Common;
using namespace Math;
using namespace Graphics;

namespace Alcachofa {

//
// OpenGL classes, calls to gl* are allowed here
//

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

void OpenGLTexture::setMirrorWrap(bool wrap) {
	if (_mirrorWrap == wrap)
		return;
	_mirrorWrap = wrap;
	GLint wrapMode;
	if (wrap)
		wrapMode = OpenGLContext.textureMirrorRepeatSupported ? GL_MIRRORED_REPEAT : GL_REPEAT;
	else
#if USE_FORCED_GLES2 // GLES2 does not define GL_CLAMP
		wrapMode = GL_CLAMP_TO_EDGE;
#else
		wrapMode = OpenGLContext.textureEdgeClampSupported ? GL_CLAMP_TO_EDGE : GL_CLAMP;
#endif

	GL_CALL(glBindTexture(GL_TEXTURE_2D, _handle));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode));
}

void OpenGLTexture::update(const Surface &surface) {
	assert(surface.format == g_engine->renderer().getPixelFormat());
	assert(surface.w == size().x && surface.h == size().y);

	const void *pixels = surface.getPixels();

	glEnable(GL_TEXTURE_2D);
	OpenGL::clearGLError();
	GL_CALL(glBindTexture(GL_TEXTURE_2D, _handle));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size().x, size().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
	if (_withMipmaps)
		GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
	else
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));
}

OpenGLRenderer::OpenGLRenderer(Point resolution) : OpenGLRendererBase(resolution) {
	initGraphics3d(resolution.x, resolution.y);
	GL_CALL(glDisable(GL_DEPTH_TEST));
	GL_CALL(glDisable(GL_SCISSOR_TEST));
	GL_CALL(glDisable(GL_STENCIL_TEST));
	GL_CALL(glDisable(GL_CULL_FACE));
	GL_CALL(glEnable(GL_BLEND));
	GL_CALL(glDepthMask(GL_FALSE));

	if (!OpenGLContext.textureMirrorRepeatSupported) {
		GUI::displayErrorDialog(_("Old OpenGL detected, some graphical errors will occur."));
	}
}

ScopedPtr<ITexture> OpenGLRenderer::createTexture(int32 w, int32 h, bool withMipmaps) {
	assert(w >= 0 && h >= 0);
	return ScopedPtr<ITexture>(new OpenGLTexture(w, h, withMipmaps));
}

PixelFormat OpenGLRenderer::getPixelFormat() const {
	return PixelFormat::createFormatRGBA32();
}

bool OpenGLRenderer::requiresPoTTextures() const {
	return !OpenGLContext.NPOTSupported;
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

	if (output.format != getPixelFormat()) {
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

void OpenGLRenderer::setViewportInner(int x, int y, int width, int height) {
	GL_CALL(glViewport(x, y, width, height));
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
