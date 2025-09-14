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

#include "alcachofa/graphics-opengl-base.h"

#include "graphics/managed_surface.h"
#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/debug.h"

namespace Alcachofa {

class OpenGLTexture : public OpenGLTextureBase {
public:
	OpenGLTexture(int32 w, int32 h, bool withMipmaps);
	~OpenGLTexture() override;

	inline GLuint handle() const { return _handle; }
	void setMirrorWrap(bool wrap);

protected:
	void updateInner(const void *pixels) override;

	GLuint _handle = 0;
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

}

#endif // ALCACHOFA_GRAPHICS_OPENGL_H
