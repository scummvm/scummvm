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

#ifndef STARK_GFX_TINYGL_TEXTURE_H
#define STARK_GFX_TINYGL_TEXTURE_H

#include "engines/stark/gfx/texture.h"

#include "graphics/tinygl/tinygl.h"

namespace Stark {
namespace Gfx {

/**
 * An TinyGL texture wrapper
 */
class TinyGlTexture : public Texture {
public:
	TinyGlTexture();
	virtual ~TinyGlTexture();

	// Texture API
	void bind() const override;
	void update(const Graphics::Surface *surface, const byte *palette = nullptr) override;
	void setSamplingFilter(SamplingFilter filter) override;
	void setLevelCount(uint32 count) override;
	void addLevel(uint32 level, const Graphics::Surface *surface, const byte *palette = nullptr) override;

protected:
	void updateLevel(uint32 level, const Graphics::Surface *surface, const byte *palette = nullptr);

	TGLuint _id;
	uint32 _levelCount;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_TINYGL_TEXTURE_H
