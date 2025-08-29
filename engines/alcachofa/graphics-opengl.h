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

struct OpenGLFormat {
	GLenum _format, _type;
	inline bool isValid() const { return _format != GL_NONE; }
};

OpenGLFormat getOpenGLFormatOf(const Graphics::PixelFormat &format);

class OpenGLTexture : public ITexture {
public:
	OpenGLTexture(int32 w, int32 h, bool withMipmaps);
	~OpenGLTexture() override;
	void update(const Graphics::Surface &surface) override;
	void setMirrorWrap(bool wrap);

	inline GLuint handle() const { return _handle; }

private:
	GLuint _handle;
	bool _withMipmaps;
	bool _mirrorWrap = true;
};

class OpenGLRenderer : public virtual IRenderer {
public:
	OpenGLRenderer(Common::Point resolution);

	Common::ScopedPtr<ITexture> createTexture(int32 w, int32 h, bool withMipmaps) override;
	void end() override;
	void setOutput(Graphics::Surface &output) override;
	bool hasOutput() const override;

protected:
	void resetState();
	void setBlendFunc(BlendMode blendMode); ///< just the blend-func, not texenv/shader uniform
	void setViewportToScreen();
	void setViewportToRect(int16 outputWidth, int16 outputHeight);
	virtual void setMatrices(bool flipped) = 0;
	void checkFirstDrawCommand();

	Common::Point _resolution, _outputSize;
	Graphics::Surface *_currentOutput = nullptr;
	OpenGLTexture *_currentTexture = nullptr;
	BlendMode _currentBlendMode = (BlendMode)-1;
	float _currentLodBias = 0.0f;
	bool _isFirstDrawCommand = false;
};

}

#endif // ALCACHOFA_GRAPHICS_OPENGL_H
