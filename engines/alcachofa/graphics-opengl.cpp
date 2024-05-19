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

#include "graphics.h"

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
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}

	virtual ~OpenGLTexture() override {
		if (_handle != 0)
			GL_CALL(glDeleteTextures(1, &_handle));
	}

	virtual void update(const ManagedSurface &surface) {
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

	inline GLuint handle() const { return _handle; }

private:
	GLuint _handle;
	bool _withMipmaps;
};

class OpenGLRenderer : public IRenderer {
public:
	OpenGLRenderer(Point resolution)
		: _resolution(resolution) {
		initViewportAndMatrices();
		GL_CALL(glDisable(GL_LIGHTING));
		GL_CALL(glDisable(GL_DEPTH_TEST));
		GL_CALL(glDisable(GL_SCISSOR_TEST));
		GL_CALL(glDisable(GL_STENCIL_TEST));
		GL_CALL(glEnable(GL_BLEND));
		GL_CALL(glDepthMask(GL_FALSE));
	}

	virtual ScopedPtr<ITexture> createTexture(int32 w, int32 h, bool withMipmaps) override {
		assert(w > 0 && h > 0);
		return ScopedPtr<ITexture>(new OpenGLTexture(w, h, withMipmaps));
	}

	virtual void begin() override {
		GL_CALL(glEnableClientState(GL_VERTEX_ARRAY));
		GL_CALL(glDisableClientState(GL_INDEX_ARRAY));
		GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));
		GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE));
		GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT));
		GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_CONSTANT));
		_currentLodBias = -1000.0f;
		_currentTexture = nullptr;
		_currentBlendMode = (BlendMode)-1;

#ifdef _DEBUG
		glClearColor(0.5f, 0.0f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
#endif
	}

	virtual void end() override {
		GL_CALL(glFlush());
		g_system->updateScreen();
	}

	virtual void setTexture(const ITexture *texture) override {
		if (texture == _currentTexture)
			return;
		else if (texture == nullptr) {
			GL_CALL(glDisable(GL_TEXTURE_2D));
			GL_CALL(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
		}
		else {
			if (_currentTexture == nullptr) {
				GL_CALL(glEnable(GL_TEXTURE_2D));
				GL_CALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
			}
			auto glTexture = dynamic_cast<const OpenGLTexture *>(texture);
			assert(glTexture != nullptr);
			GL_CALL(glBindTexture(GL_TEXTURE_2D, glTexture->handle()));
		}
		_currentTexture = texture;
	}

	virtual void setBlendMode(BlendMode blendMode) override {
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

		/** now the texture stage, mind that this always applies:
		 * SRC0_RGB is TEXTURE
		 * SRC1_RGB/ALPHA is CONSTANT
		 * COMBINE_ALPHA is REPLACE
		 */ 
		switch (blendMode) {
		case BlendMode::AdditiveAlpha:
		case BlendMode::Additive:
		case BlendMode::Multiply:
			// (1 - TintAlpha) * TexColor, TexAlpha
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_ONE_MINUS_SRC_ALPHA));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE));
			break;
		case BlendMode::Alpha:
			// TexColor, TintAlpha
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_CONSTANT));
			break;
		case BlendMode::Tinted:
			// (TintColor * TintAlpha) * TexColor, TexAlpha
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE));
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR)); // pre-multiplied with alpha
			GL_CALL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE));
			break;
		default: assert(false && "Invalid blend mode"); break;
		}
		_currentBlendMode = blendMode;
	}

	virtual void setLodBias(float lodBias) override {
		if (abs(_currentLodBias - lodBias) < epsilon)
			return;
		GL_CALL(glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, lodBias));
		_currentLodBias = lodBias;
	}

	virtual void quad(
		Vector2d center,
		Vector2d size,
		Color color,
		Angle rotation,
		Vector2d texMin,
		Vector2d texMax) override {
		size *= 0.5f;
		center += size;
		Vector2d positions[] = {
			center + Vector2d(-size.getX(), -size.getY()),
			center + Vector2d(-size.getX(), +size.getY()),
			center + Vector2d(+size.getX(), +size.getY()),
			center + Vector2d(+size.getX(), -size.getY()),
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

		float colors[] = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		GL_CALL(glVertexPointer(2, GL_FLOAT, 0, positions));
		if (_currentTexture != nullptr)
			GL_CALL(glTexCoordPointer(2, GL_FLOAT, 0, texCoords));
		GL_CALL(glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors));
		GL_CALL(glDrawArrays(GL_QUADS, 0, 4));

#if DEBUG
		// make sure we crash instead of someone using our stack arrays
		GL_CALL(glVertexPointer(2, GL_FLOAT, sizeof(Vector2d), nullptr));
		GL_CALL(glTexCoordPointer(2, GL_FLOAT, sizeof(Vector2d), nullptr));
#endif
	}

private:
	void initViewportAndMatrices() {
		int32 screenWidth = g_system->getWidth();
		int32 screenHeight = g_system->getHeight();
		Rect viewport(
			MIN<int32>(screenWidth, screenHeight * (float)_resolution.x / _resolution.y),
			MIN<int32>(screenHeight, screenWidth * (float)_resolution.y / _resolution.x));
		viewport.translate(
			(screenWidth - viewport.width()) / 2,
			(screenHeight - viewport.height()) / 2);

		GL_CALL(glViewport(viewport.left, viewport.top, viewport.width(), viewport.height()));
		GL_CALL(glMatrixMode(GL_PROJECTION));
		GL_CALL(glLoadIdentity());
		GL_CALL(glOrtho(0.0f, _resolution.x, _resolution.y, 0.0f, -1.0f, 1.0f));
		GL_CALL(glMatrixMode(GL_MODELVIEW));
		GL_CALL(glLoadIdentity());
	}

	Point _resolution;
	const ITexture *_currentTexture = nullptr;
	BlendMode _currentBlendMode = (BlendMode)-1;
	float _currentLodBias = 0.0f;
};

IRenderer *IRenderer::createOpenGLRenderer(Point resolution) {
	initGraphics3d(resolution.x, resolution.y);
	return new OpenGLRenderer(resolution);
}

}
