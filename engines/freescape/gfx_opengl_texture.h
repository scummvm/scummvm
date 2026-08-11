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

#ifndef FREESCAPE_GFX_OPENGL_TEXTURE_H
#define FREESCAPE_GFX_OPENGL_TEXTURE_H

#include "graphics/opengl/system_headers.h"
#include "graphics/surface.h"

#include "freescape/gfx.h"

namespace Freescape {

class OpenGLTexture : public Texture {
public:
	OpenGLTexture(const Graphics::Surface *surface);
	OpenGLTexture();
	virtual ~OpenGLTexture();

	void update(const Graphics::Surface *surface) override;
	void updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) override;

	void copyFromFramebuffer(const Common::Rect &screen);

	GLuint _id;
	GLuint _internalFormat;
	GLuint _sourceFormat;
	uint32 _internalWidth;
	uint32 _internalHeight;
	bool _upsideDown;

private:
	void updateTexture(const Graphics::Surface *surface, const Common::Rect &rect);
};

} // End of namespace Freescape

#endif // FREESCAPE_GFX_OPENGL_TEXTURE_H
