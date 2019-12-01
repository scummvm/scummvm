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

#include "ultima8/misc/pent_include.h"
#include "ultima8/graphics/texture.h"
#include "ultima8/graphics/textureBitmap.h"
#include "ultima8/graphics/textureTarga.h"
#include "ultima8/graphics/texturePNG.h"

#include <cstring>

namespace Ultima8 {

//
// Base Clear Func
//
bool Texture::Clear() {
	// Temporary fix to prevent us from freeing a RenderSurface's memory
	if (format != TEX_FMT_NATIVE)
		delete [] buffer;
	buffer = 0;

	return true;
}

//
// Destructor
//
Texture::~Texture() {
	Clear();
}

//
// Helper Macro for texture type detection
//
#define TRY_TYPE(TextureType)               \
	tex = new TextureType();                    \
	/* If read failed, delete the texture. */   \
	if (!tex->Read(ds)) {                       \
		delete tex;                             \
		tex = 0;                                \
	}                                           \
	else {                                      \
		/* Worked so return it */               \
		return tex;                             \
	}

//
// Create a texture from a Data Source
// (filename is used to help detection of type)
//
Texture *Texture::Create(IDataSource *ds, const char *filename) {
	Texture *tex;

	if (filename) {
		// Looks like it's a PNG
		if (std::strstr(filename, ".png")) {
			TRY_TYPE(TexturePNG);
		}
		// Looks like it's a BMP
		if (std::strstr(filename, ".bmp")) {
			TRY_TYPE(TextureBitmap);
		}
		// Looks like it's a TGA
		if (std::strstr(filename, ".tga")) {
			TRY_TYPE(TextureTarga);
		}
	}

	// Now go through each type 1 by 1
	TRY_TYPE(TexturePNG);
	TRY_TYPE(TextureBitmap);
	TRY_TYPE(TextureTarga);

	// Couldn't find it
	return 0;
}

} // End of namespace Ultima8
