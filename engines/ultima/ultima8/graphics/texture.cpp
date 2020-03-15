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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/graphics/texture_bitmap.h"
#include "ultima/ultima8/graphics/texture_targa.h"
#include "ultima/ultima8/graphics/texture_png.h"
#include "ultima/ultima8/graphics/render_surface.h"

#include <cstring>

namespace Ultima {
namespace Ultima8 {


Texture::Texture() : _format(TEX_FMT_STANDARD), _glTex(0), _next(nullptr) {
}

//
// Destructor
//
Texture::~Texture() {
}

//
// Helper Macro for texture type detection
//
#define TRY_TYPE(TextureType)               \
	tex = new TextureType();                    \
	/* If read failed, delete the texture. */   \
	if (!tex->Read(ds)) {                       \
		delete tex;                             \
		tex = nullptr;                          \
	}                                           \
	else {                                      \
		/* Worked so return it */               \
		return tex;                             \
	}

void Texture::create(uint16 width, uint16 height, TextureFormat textureFormat) {
	_format = textureFormat;
	create(width, height, (_format == TEX_FMT_NATIVE) ? RenderSurface::getPixelFormat() :
		Texture::getPixelFormat());
}

//
// Create a texture from a Data Source
// (filename is used to help detection of type)
//
Texture *Texture::Create(IDataSource *ds, const char *filename) {
	Texture *tex;

	if (filename) {
		// Looks like it's a PNG
		if (Std::strstr(filename, ".png")) {
			TRY_TYPE(TexturePNG);
		}
		// Looks like it's a BMP
		if (Std::strstr(filename, ".bmp")) {
			TRY_TYPE(TextureBitmap);
		}
		// Looks like it's a TGA
		if (Std::strstr(filename, ".tga")) {
			TRY_TYPE(TextureTarga);
		}
	}

	// Now go through each type 1 by 1
	TRY_TYPE(TexturePNG);
	TRY_TYPE(TextureBitmap);
	TRY_TYPE(TextureTarga);

	// Couldn't find it
	return nullptr;
}

void Texture::loadSurface(const Graphics::Surface *surf) {
	assert(surf->format.bytesPerPixel == 2 || surf->format.bytesPerPixel == 4);
	create(surf->w, surf->h, Texture::getPixelFormat());
	this->_format = TEX_FMT_STANDARD;
	this->_wlog2 = -1;
	this->_hlog2 = -1;

	// Repack RGBA
	uint32 *buffer = (uint32 *)getPixels();
	uint32 pixel, i = 0;
	byte r, g, b, a;
	for (int y = 0; y < surf->h; ++y) {
		const byte *srcP = (const byte *)surf->getBasePtr(0, y);

		for (int x = 0; x < surf->w; ++x, srcP += surf->format.bytesPerPixel) {
			pixel = (surf->format.bytesPerPixel == 2) ? *((const uint16 *)srcP) : *((const uint32 *)srcP);
			surf->format.colorToARGB(pixel, a, r, g, b);

			buffer[i++] = (r << TEX32_R_SHIFT)
				| (g << TEX32_G_SHIFT)
				| (b << TEX32_B_SHIFT)
				| (a << TEX32_A_SHIFT);
		}
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
