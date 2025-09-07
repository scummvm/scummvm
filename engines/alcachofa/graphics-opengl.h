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

#ifndef ALCACHOFA_GRAPHICS_OPENGL_H
#define ALCACHOFA_GRAPHICS_OPENGL_H

#include "alcachofa/graphics.h"

#include "graphics/managed_surface.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/debug.h"

namespace Alcachofa {

/** This base class does not call any gl* functions so we can use it for TinyGL as well */
class OpenGLTextureBase : public ITexture {
public:
	OpenGLTextureBase(int32 w, int32 h, bool withMipmaps);
	~OpenGLTextureBase() override = default;
	void update(const Graphics::Surface &surface) override;

	inline GLuint handle() const { return _handle; }

protected:
	virtual void updateInner(const void *pixels) = 0; ///< expects pixels to be RGBA32

	GLuint _handle = 0;
	bool _withMipmaps;
	bool _mirrorWrap = true;
	bool _didConvertOnce = false;
	Graphics::ManagedSurface _tmpSurface;
};

class OpenGLTexture : public OpenGLTextureBase {
public:
	OpenGLTexture(int32 w, int32 h, bool withMipmaps);
	~OpenGLTexture() override;

	void setMirrorWrap(bool wrap);

protected:
	void updateInner(const void *pixels) override;
};

class OpenGLRendererBase : public virtual IRenderer {
public:
	OpenGLRendererBase(Common::Point resolution);

	bool hasOutput() const override;

protected:
	virtual void setViewportInner(int x, int y, int width, int height) = 0;
	virtual void setMatrices(bool flipped) = 0;

	void resetState();
	void setViewportToScreen();
	void setViewportToRect(int16 outputWidth, int16 outputHeight);
	void getQuadPositions(Math::Vector2d topLeft, Math::Vector2d size, Math::Angle rotation, Math::Vector2d positions[]) const;
	void getQuadTexCoords(Math::Vector2d texMin, Math::Vector2d texMax, Math::Vector2d texCoords[]) const;

	Common::Point _resolution, _outputSize;
	Graphics::Surface *_currentOutput = nullptr;
	BlendMode _currentBlendMode = (BlendMode)-1;
	float _currentLodBias = 0.0f;
	bool _isFirstDrawCommand = false;
};

class OpenGLRenderer : public OpenGLRendererBase {
public:
	OpenGLRenderer(Common::Point resolution);

	Common::ScopedPtr<ITexture> createTexture(int32 w, int32 h, bool withMipmaps) override;
	void end() override;
	void setOutput(Graphics::Surface &output) override;

protected:
	void setViewportInner(int x, int y, int width, int height) override;
	void setBlendFunc(BlendMode blendMode); ///< just the blend-func, not texenv/shader uniform
	void checkFirstDrawCommand();

	OpenGLTexture *_currentTexture = nullptr;
};

IRenderer *createOpenGLRendererClassic(Common::Point resolution);
IRenderer *createOpenGLRendererShaders(Common::Point resolution);
IRenderer *createTinyGLRenderer(Common::Point resolution);

}

#endif // ALCACHOFA_GRAPHICS_OPENGL_H
