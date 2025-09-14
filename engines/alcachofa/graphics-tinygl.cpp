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
#include "alcachofa/graphics-opengl-base.h"

#include "common/system.h"
#include "common/config-manager.h"
#include "engines/util.h"
#include "graphics/tinygl/tinygl.h"

using namespace Common;
using namespace Math;
using namespace Graphics;

namespace Alcachofa {

class TinyGLTexture : public OpenGLTextureBase {
public:
	TinyGLTexture(int32 w, int32 h, bool withMipmaps)
		: OpenGLTextureBase(w, h, withMipmaps) {
		tglEnable(TGL_TEXTURE_2D);
		tglGenTextures(1, &_handle);
		tglBindTexture(TGL_TEXTURE_2D, _handle);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR_MIPMAP_LINEAR);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_LINEAR);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_MIRRORED_REPEAT);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_MIRRORED_REPEAT);
	}

	~TinyGLTexture() override {
		if (_handle != 0)
			tglDeleteTextures(1, &_handle);
	}

	inline TGLuint handle() const { return _handle; }

protected:
	void updateInner(const void *pixels) override {
		tglEnable(TGL_TEXTURE_2D);
		tglBindTexture(TGL_TEXTURE_2D, _handle);
		tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, size().x, size().y, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, pixels);
		if (_withMipmaps && false)
			warning("NO TINYGL MIPMAPS IMPLEMENTED YET");
		else
			tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAX_LEVEL, 0);
	}

	TGLuint _handle;
};

class TinyGLRenderer : public OpenGLRendererBase {
public:
	TinyGLRenderer(Point resolution) : OpenGLRendererBase(resolution) {
		initGraphics(resolution.x, resolution.y, nullptr);
		debug("Using framebuffer format: %s", g_system->getScreenFormat().toString().c_str());
		if (g_system->getScreenFormat().bytesPerPixel < 2)
			error("Alcachofa needs at least 16bit colors"); 
		_context = TinyGL::createContext(
			resolution.x, resolution.y,
			g_system->getScreenFormat(),
			1024, // some background images are even larger than this
			false, false);
		TinyGL::setContext(_context);

		tglDisable(TGL_DEPTH_TEST);
		tglDisable(TGL_SCISSOR_TEST);
		tglDisable(TGL_STENCIL_TEST);
		tglDisable(TGL_CULL_FACE);
		tglEnable(TGL_BLEND);
		tglDepthMask(TGL_FALSE);
	}

	~TinyGLRenderer() override {
		if (_context != nullptr)
			TinyGL::destroyContext(_context);
	}

	ScopedPtr<ITexture> createTexture(int32 w, int32 h, bool withMipmaps) override {
		assert(w >= 0 && h >= 0);
		return ScopedPtr<ITexture>(new TinyGLTexture(w, h, withMipmaps));
	}

	void begin() override {
		resetState();
		_currentTexture = nullptr;
	}

	void setTexture(ITexture *texture) override {
		if (texture == _currentTexture)
			return;
		else if (texture == nullptr) {
			tglDisable(TGL_TEXTURE_2D);
			_currentTexture = nullptr;
		} else {
			if (_currentTexture == nullptr) {
				tglEnable(TGL_TEXTURE_2D);
			}
			auto glTexture = dynamic_cast<TinyGLTexture *>(texture);
			assert(glTexture != nullptr);
			tglBindTexture(TGL_TEXTURE_2D, glTexture->handle());
			_currentTexture = glTexture;
		}
	}

	void setBlendMode(BlendMode blendMode) override {
		if (blendMode == _currentBlendMode)
			return;
		switch (blendMode) {
		case BlendMode::AdditiveAlpha:
			tglBlendFunc(TGL_ONE, TGL_ONE_MINUS_SRC_ALPHA);
			break;
		case BlendMode::Additive:
			tglBlendFunc(TGL_ONE, TGL_ONE);
			break;
		case BlendMode::Multiply:
			tglBlendFunc(TGL_DST_COLOR, TGL_ONE);
			break;
		case BlendMode::Alpha:
			tglBlendFunc(TGL_SRC_ALPHA, TGL_ONE_MINUS_SRC_ALPHA);
			break;
		case BlendMode::Tinted:
			tglBlendFunc(TGL_ONE, TGL_ONE_MINUS_SRC_ALPHA);
			break;
		default: assert(false && "Invalid blend mode"); break;
		}

		tglTexEnvi(TGL_TEXTURE_ENV, TGL_TEXTURE_ENV_MODE, TGL_COMBINE);
		switch (blendMode) {
		case BlendMode::AdditiveAlpha:
		case BlendMode::Additive:
		case BlendMode::Multiply:
			// TintAlpha * TexColor, TexAlpha
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_COMBINE_RGB, TGL_MODULATE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_COMBINE_ALPHA, TGL_REPLACE);

			tglTexEnvi(TGL_TEXTURE_ENV, TGL_SOURCE0_RGB, TGL_TEXTURE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_OPERAND0_RGB, TGL_SRC_COLOR);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_SOURCE0_ALPHA, TGL_TEXTURE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_OPERAND0_ALPHA, TGL_SRC_ALPHA);

			tglTexEnvi(TGL_TEXTURE_ENV, TGL_SOURCE1_RGB, TGL_PRIMARY_COLOR);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_OPERAND1_RGB, TGL_SRC_ALPHA); // alpha replaces color
			break;
		case BlendMode::Alpha:
			// TexColor, TintAlpha
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_COMBINE_RGB, TGL_REPLACE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_COMBINE_ALPHA, TGL_REPLACE);

			tglTexEnvi(TGL_TEXTURE_ENV, TGL_SOURCE0_RGB, TGL_TEXTURE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_OPERAND0_RGB, TGL_SRC_COLOR);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_SOURCE0_ALPHA, TGL_PRIMARY_COLOR);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_OPERAND0_ALPHA, TGL_SRC_ALPHA);
			break;
		case BlendMode::Tinted:
			// (TintColor * TintAlpha) * TexColor, TexAlpha
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_COMBINE_RGB, TGL_MODULATE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_COMBINE_ALPHA, TGL_REPLACE);

			tglTexEnvi(TGL_TEXTURE_ENV, TGL_SOURCE0_RGB, TGL_TEXTURE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_OPERAND0_RGB, TGL_SRC_COLOR);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_SOURCE0_ALPHA, TGL_TEXTURE);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_OPERAND0_ALPHA, TGL_SRC_ALPHA);

			tglTexEnvi(TGL_TEXTURE_ENV, TGL_SOURCE1_RGB, TGL_PRIMARY_COLOR);
			tglTexEnvi(TGL_TEXTURE_ENV, TGL_OPERAND1_RGB, TGL_SRC_COLOR); // we have to pre-multiply
			break;
		default:
			assert(false && "Invalid blend mode");
			break;
		}
		_currentBlendMode = blendMode;
	}

	void setLodBias(float lodBias) override {
		_currentLodBias = lodBias;
		// TinyGL does not support lod bias
	}

	void setOutput(Surface &output) override {
		assert(_isFirstDrawCommand);
		setViewportToRect(output.w, output.h);
		_currentOutput = &output;

		if (output.w > _resolution.x || output.h > _resolution.y)
			debugC(0, kDebugGraphics, "Output is larger than screen, output will be cropped (%d, %d) > (%d, %d)",
				output.w, output.h, _resolution.x, _resolution.y);

		// no need to check format for TinyGL, we need to be prepared for conversion anyways
	}

	void end() override {
		tglFlush();
		TinyGL::presentBuffer();

		Surface framebuffer;
		TinyGL::getSurfaceRef(framebuffer);
		if (_currentOutput == nullptr) {
			g_system->copyRectToScreen(framebuffer.getPixels(), framebuffer.pitch, 0, 0, framebuffer.w, framebuffer.h);
			g_system->updateScreen();
		} else {
			framebuffer = framebuffer.getSubArea(Rect(0, 0, _currentOutput->w, _currentOutput->h));
			crossBlit(
				(byte *)_currentOutput->getPixels(),
				(const byte *)framebuffer.getPixels(),
				_currentOutput->pitch,
				framebuffer.pitch,
				framebuffer.w,
				framebuffer.h,
				_currentOutput->format,
				framebuffer.format);
		}
	}

	void quad(
		Vector2d topLeft,
		Vector2d size,
		Color color,
		Angle rotation,
		Vector2d texMin,
		Vector2d texMax) override {
		Vector2d positions[4], texCoords[4];
		getQuadPositions(topLeft, size, rotation, positions);
		getQuadTexCoords(texMin, texMax, texCoords);

		float colors[] = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
		if (_currentBlendMode == BlendMode::Tinted) {
			colors[0] *= colors[3];
			colors[1] *= colors[3];
			colors[2] *= colors[3];
		}

		if (_isFirstDrawCommand) {
			_isFirstDrawCommand = false;
			tglClearColor(0, 0, 0, 0);
			tglClear(TGL_COLOR_BUFFER_BIT);
		}

		tglColor4fv(colors);
		tglBegin(TGL_QUADS);
		for (int i = 0; i < 4; i++) {
			tglTexCoord2f(texCoords[i].getX(), texCoords[i].getY());
			tglVertex2f(positions[i].getX(), positions[i].getY());
		}
		tglEnd();
		TinyGL::presentBuffer();
	}

protected:
	void setViewportInner(int x, int y, int width, int height) override {
		tglViewport(x, y, width, height);
	}

	void setMatrices(bool flipped) override {
		float bottom = flipped ? _resolution.y : 0.0f;
		float top = flipped ? 0.0f : _resolution.y;

		tglMatrixMode(TGL_PROJECTION);
		tglLoadIdentity();
		tglOrtho(0.0f, _resolution.x, bottom, top, -1.0f, 1.0f);
		tglMatrixMode(TGL_MODELVIEW);
		tglLoadIdentity();
	}

private:
	TinyGL::ContextHandle *_context = nullptr;
	TinyGLTexture *_currentTexture = nullptr;
};

IRenderer *IRenderer::createTinyGLRenderer(Point resolution) {
	debug("Use TinyGL renderer");
	return new TinyGLRenderer(resolution);
}

}
