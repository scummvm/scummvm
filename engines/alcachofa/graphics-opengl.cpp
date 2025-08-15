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

#include "common/system.h"
#include "engines/util.h"
#include "graphics/managed_surface.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/debug.h"

using namespace Common;
using namespace Math;
using namespace Graphics;

namespace Alcachofa {

struct OpenGLFormat {
	GLenum _format, _type;
	inline bool isValid() const { return _format != GL_NONE; }
};

static bool areComponentsInOrder(const PixelFormat &format, int r, int g, int b, int a) {
	return format == (a < 0
		? PixelFormat(3, 8, 8, 8, 0, r * 8, g * 8, b * 8, 0)
		: PixelFormat(4, 8, 8, 8, 8, r * 8, g * 8, b * 8, a * 8));
}

static OpenGLFormat getOpenGLFormatOf(const PixelFormat &format) {
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


class OpenGLTexture : public ITexture {
public:
	OpenGLTexture(int32 w, int32 h, bool withMipmaps)
		: ITexture({ (int16)w, (int16)h })
		, _withMipmaps(withMipmaps) {
		GL_CALL(glEnable(GL_TEXTURE_2D));
		GL_CALL(glGenTextures(1, &_handle));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, _handle));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		setMirrorWrap(false);
	}

	~OpenGLTexture() override {
		if (_handle != 0)
			GL_CALL(glDeleteTextures(1, &_handle));
	}

	void update(const Surface &surface) override {
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

	void setMirrorWrap(bool wrap) {
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

	inline GLuint handle() const { return _handle; }

private:
	GLuint _handle;
	bool _withMipmaps;
	bool _mirrorWrap = true;
};

class OpenGLRenderer : public IDebugRenderer {
public:
	OpenGLRenderer(Point resolution)
		: _resolution(resolution) {
		setViewportToScreen();

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

	ScopedPtr<ITexture> createTexture(int32 w, int32 h, bool withMipmaps) override {
		assert(w >= 0 && h >= 0);
		return ScopedPtr<ITexture>(new OpenGLTexture(w, h, withMipmaps));
	}

	void begin() override {
		GL_CALL(glEnableClientState(GL_VERTEX_ARRAY));
		GL_CALL(glDisableClientState(GL_INDEX_ARRAY));
		GL_CALL(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
		setViewportToScreen();
		_currentOutput = nullptr;
		_currentLodBias = -1000.0f;
		_currentTexture = nullptr;
		_currentBlendMode = (BlendMode)-1;
		_isFirstDrawCommand = true;
	}

	void end() override {
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

	void setTexture(ITexture *texture) override {
		if (texture == _currentTexture)
			return;
		else if (texture == nullptr) {
			GL_CALL(glDisable(GL_TEXTURE_2D));
			GL_CALL(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
			_currentTexture = nullptr;
		}
		else {
			if (_currentTexture == nullptr) {
				GL_CALL(glEnable(GL_TEXTURE_2D));
				GL_CALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
			}
			auto glTexture = dynamic_cast<OpenGLTexture *>(texture);
			assert(glTexture != nullptr);
			GL_CALL(glBindTexture(GL_TEXTURE_2D, glTexture->handle()));
			_currentTexture = glTexture;
		}
	}

	void setBlendMode(BlendMode blendMode) override {
		if (blendMode == _currentBlendMode)
			return;
		// first the blend func
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

		GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));
		switch (blendMode) {
		case BlendMode::AdditiveAlpha:
		case BlendMode::Additive:
		case BlendMode::Multiply:
			// TintAlpha * TexColor, TexAlpha
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));

			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA));

			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA)); // alpha replaces color
			break;
		case BlendMode::Alpha:
			// TexColor, TintAlpha
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));

			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA));
			break;
		case BlendMode::Tinted:
			// (TintColor * TintAlpha) * TexColor, TexAlpha
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));

			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA));

			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR)); // we have to pre-multiply
			break;
		default: assert(false && "Invalid blend mode"); break;
		}
		_currentBlendMode = blendMode;
	}

	void setLodBias(float lodBias) override {
		if (abs(_currentLodBias - lodBias) < epsilon)
			return;
		GL_CALL(glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, lodBias));
		_currentLodBias = lodBias;
	}

	void setOutput(Surface &output) override {
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

	bool hasOutput() const override {
		return _currentOutput != nullptr;
	}

	void quad(
		Vector2d topLeft,
		Vector2d size,
		Color color,
		Angle rotation,
		Vector2d texMin,
		Vector2d texMax) override {
		Vector2d positions[] = {
			topLeft + Vector2d(0,			0),
			topLeft + Vector2d(0,			+size.getY()),
			topLeft + Vector2d(+size.getX(), +size.getY()),
			topLeft + Vector2d(+size.getX(), 0),
		};
		if (abs(rotation.getDegrees()) > epsilon) {
			const Vector2d zero(0, 0);
			for (int i = 0; i < 4; i++)
				positions[i].rotateAround(zero, rotation);
		}

		Vector2d texCoords[] = {
			{ texMin.getX(), texMin.getY() },
			{ texMin.getX(), texMax.getY() },
			{ texMax.getX(), texMax.getY() },
			{ texMax.getX(), texMin.getY() }
		};
		if (_currentTexture != nullptr) {
			// float equality is fine here, if it was calculated it was not a normal graphic
			_currentTexture->setMirrorWrap(texMin != Vector2d() || texMax != Vector2d(1, 1));
		}

		float colors[] = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
		if (_currentBlendMode == BlendMode::Tinted)
		{
			colors[0] *= colors[3];
			colors[1] *= colors[3];
			colors[2] *= colors[3];
		}

		checkFirstDrawCommand();
		GL_CALL(glColor4fv(colors));
		GL_CALL(glVertexPointer(2, GL_FLOAT, 0, positions));
		if (_currentTexture != nullptr)
			GL_CALL(glTexCoordPointer(2, GL_FLOAT, 0, texCoords));
		GL_CALL(glDrawArrays(GL_QUADS, 0, 4));

#ifdef _DEBUG
		// make sure we crash instead of someone using our stack arrays
		GL_CALL(glVertexPointer(2, GL_FLOAT, sizeof(Vector2d), nullptr));
		GL_CALL(glTexCoordPointer(2, GL_FLOAT, sizeof(Vector2d), nullptr));
#endif
	}

	void debugPolygon(
		Span<Vector2d> points,
		Color color
	) override {
		checkFirstDrawCommand();
		setTexture(nullptr);
		setBlendMode(BlendMode::Alpha);
		GL_CALL(glVertexPointer(2, GL_FLOAT, 0, points.data()));
		GL_CALL(glLineWidth(4.0f));
		GL_CALL(glPointSize(8.0f));

		GL_CALL(glColor4ub(color.r, color.g, color.b, color.a));
		if (points.size() > 2)
			GL_CALL(glDrawArrays(GL_POLYGON, 0, points.size()));

		color.a = (byte)(MIN(255.0f, color.a * 1.3f));
		GL_CALL(glColor4ub(color.r, color.g, color.b, color.a));
		if (points.size() > 1)
			GL_CALL(glDrawArrays(GL_LINE_LOOP, 0, points.size()));

		color.a = (byte)(MIN(255.0f, color.a * 1.3f));
		GL_CALL(glColor4ub(color.r, color.g, color.b, color.a));
		if (points.size() > 0)
			GL_CALL(glDrawArrays(GL_POINTS, 0, points.size()));
	}

	void debugPolyline(
		Span<Vector2d> points,
		Color color
	) override {
		checkFirstDrawCommand();
		setTexture(nullptr);
		setBlendMode(BlendMode::Alpha);
		GL_CALL(glVertexPointer(2, GL_FLOAT, 0, points.data()));
		GL_CALL(glLineWidth(4.0f));
		GL_CALL(glPointSize(8.0f));

		GL_CALL(glColor4ub(color.r, color.g, color.b, color.a));
		if (points.size() > 1)
			GL_CALL(glDrawArrays(GL_LINE_STRIP, 0, points.size()));

		color.a = (byte)(MIN(255.0f, color.a * 1.3f));
		GL_CALL(glColor4ub(color.r, color.g, color.b, color.a));
		if (points.size() > 0)
			GL_CALL(glDrawArrays(GL_POINTS, 0, points.size()));
	}

private:
	void setMatrices(bool flipped) {
		float bottom = flipped ? _resolution.y : 0.0f;
		float top = flipped ? 0.0f : _resolution.y;

		GL_CALL(glMatrixMode(GL_PROJECTION));
		GL_CALL(glLoadIdentity());
		GL_CALL(glOrtho(0.0f, _resolution.x, bottom, top, -1.0f, 1.0f));
		GL_CALL(glMatrixMode(GL_MODELVIEW));
		GL_CALL(glLoadIdentity());
	}

	void setViewportToScreen() {
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

	void setViewportToRect(int16 outputWidth, int16 outputHeight) {
		_outputSize.x = MIN(outputWidth, g_system->getWidth());
		_outputSize.y = MIN(outputHeight, g_system->getHeight());
		GL_CALL(glViewport(0, 0, _outputSize.x, _outputSize.y));
		setMatrices(false);
	}

	void checkFirstDrawCommand() {
		// We delay clearing the screen. It is much easier for the game
		// to switch to a framebuffer before
		if (!_isFirstDrawCommand)
			return;
		_isFirstDrawCommand = false;
		GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
	}

	Point _resolution, _outputSize;
	Surface *_currentOutput = nullptr;
	OpenGLTexture *_currentTexture = nullptr;
	BlendMode _currentBlendMode = (BlendMode)-1;
	float _currentLodBias = 0.0f;
	bool _isFirstDrawCommand = false;
};

IRenderer *IRenderer::createOpenGLRenderer(Point resolution) {
	initGraphics3d(resolution.x, resolution.y);
	return new OpenGLRenderer(resolution);
}

}
