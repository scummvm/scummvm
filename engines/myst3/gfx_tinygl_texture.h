/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GFX_TINYGL_TEXTURE_H
#define GFX_TINYGL_TEXTURE_H

#include "graphics/surface.h"
#include "graphics/tinygl/zgl.h"
#include "common/textconsole.h"

#include "engines/myst3/gfx.h"
#include "graphics/tinygl/zblit.h"

namespace Myst3 {

class TinyGLTexture : public Texture {
public:
	TinyGLTexture(const Graphics::Surface *surface);
	virtual ~TinyGLTexture();

	Graphics::BlitImage *getBlitTexture() const;

	void update(const Graphics::Surface *surface) override;
	void updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) override;

	TGLuint id;
	TGLuint internalFormat;
	TGLuint sourceFormat;
private:
	Graphics::BlitImage *_blitImage;
};

} // End of namespace Myst3

#endif
