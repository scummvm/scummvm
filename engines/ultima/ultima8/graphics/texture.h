/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef ULTIMA8_GRAPHICS_TEXTURE_H
#define ULTIMA8_GRAPHICS_TEXTURE_H

#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima8 {

//
// Texturing Helper Macros
//

// 32 Bit Texture bit operations
#define TEX32_A_SHIFT           24
#define TEX32_A_MASK            0xFF000000
#define TEX32_A(col32)          (((col32)&TEX32_A_MASK)>>TEX32_A_SHIFT)

#define TEX32_G_SHIFT           8
#define TEX32_G_MASK            0x0000FF00
#define TEX32_G(col32)          (((col32)&TEX32_G_MASK)>>TEX32_G_SHIFT)

#define TEX32_B_SHIFT           16
#define TEX32_B_MASK            0x00FF0000
#define TEX32_B(col32)          (((col32)&TEX32_B_MASK)>>TEX32_B_SHIFT)

#define TEX32_R_SHIFT           0
#define TEX32_R_MASK            0x000000FF
#define TEX32_R(col32)          (((col32)&TEX32_R_MASK)>>TEX32_R_SHIFT)

#define TEX32_AG(col32)         (((col32) >> 8) & 0x00FF00FF)
#define TEX32_RB(col32)         (((col32) >> 0) & 0x00FF00FF)

#define TEX32_PACK_RGBA(r,g,b,a)    (((a)<<TEX32_A_SHIFT)|((r)<<TEX32_R_SHIFT)|\
                                     ((g)<<TEX32_G_SHIFT)|((b)<<TEX32_B_SHIFT))
#define TEX32_PACK_RGBA16(r,g,b,a)  ((((a)>>8)<<TEX32_A_SHIFT)|\
                                     (((r)>>8)<<TEX32_R_SHIFT)|\
                                     (((g)>>8)<<TEX32_G_SHIFT)|\
                                     (((b)>>8)<<TEX32_B_SHIFT))

// 64 Bit Texture bit operations
#define TEX64_A_SHIFT           16
#define TEX64_A_MASK            0xFFFF0000
#define TEX64_A_MASK_H          0xFF000000

#define TEX64_G_SHIFT           0
#define TEX64_G_MASK            0x0000FFFF
#define TEX64_G_MASK_H          0x0000FF00

#define TEX64_B_SHIFT           16
#define TEX64_B_MASK            0xFFFF0000
#define TEX64_B_MASK_H          0xFF000000

#define TEX64_R_SHIFT           0
#define TEX64_R_MASK            0x0000FFFF
#define TEX64_R_MASK_H          0x0000FF00

enum TextureFormat {
	TEX_FMT_STANDARD        = 0,    // Standard texture format as defined using Macros above
	TEX_FMT_NATIVE          = 1     // The native format of the RenderSurface
};


class IDataSource;

//
// Basic 32 Bit Texture
//
class Texture : public Graphics::ManagedSurface {
public:
	uint32          _format;

	// Use CalcLOG2s to calculate these (can be -1 which indicates not log2)
	int32           _wlog2;
	int32           _hlog2;

	// For OpenGL
	uint32          _glTex;
	Texture         *_next;

	static Graphics::PixelFormat getPixelFormat() {
		return Graphics::PixelFormat(4, 8, 8, 8, 8, TEX32_R_SHIFT, TEX32_G_SHIFT, TEX32_B_SHIFT, TEX32_A_SHIFT);
	}

	Texture();

	virtual ~Texture();

	void create(uint16 width, uint16 height) override {
		Graphics::ManagedSurface::create(width, height);
	}
	void create(uint16 width, uint16 height, const Graphics::PixelFormat &pixelFormat) override {
		Graphics::ManagedSurface::create(width, height, pixelFormat);
	}
	void create(ManagedSurface &surf, const Common::Rect &bounds) override {
		Graphics::ManagedSurface::create(surf, bounds);
	}
	void create(uint16 width, uint16 height, TextureFormat textureFormat);

	// Calc texture log2's
	void CalcLOG2s() {
		_wlog2 = -1;
		_hlog2 = -1;
		for (int i = 0; i < 32; i++) {
			if (this->w == (1 << i))
				_wlog2 = i;

			if (this->h == (1 << i))
				_hlog2 = i;
		}
	}

	// Create a texture from a Data Source (filename is use to help detection of type)
	static Texture *Create(IDataSource *ds, const char *filename = NULL);

	// Loads the data from the passed surfcae
	void loadSurface(const Graphics::Surface *surf);
protected:

	// Read from a File. No filetype supported by this class
	virtual bool Read(IDataSource * /*ds*/) {
		return false;
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
