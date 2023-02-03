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

#ifndef FREESCAPE_GFX_TINYGL_TEXTURE_H
#define FREESCAPE_GFX_TINYGL_TEXTURE_H

#include "graphics/tinygl/zgl.h"

#include "freescape/gfx.h"

namespace Freescape {

class TinyGLTexture : public Texture {
public:
	TinyGLTexture(const Graphics::Surface *surface);
	virtual ~TinyGLTexture();

	const static Graphics::PixelFormat getRGBAPixelFormat();

	TinyGL::BlitImage *getBlitTexture() const;

	void update(const Graphics::Surface *surface) override;
	void updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) override;

	TGLuint _internalFormat;
	TGLuint _sourceFormat;

private:
	TinyGL::BlitImage *_blitImage;
};

} // End of namespace Freescape

#endif // FREESCAPE_GFX_TINYGL_TEXTURE_H
