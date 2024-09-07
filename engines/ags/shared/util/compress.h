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

#ifndef AGS_SHARED_UTIL_COMPRESS_H
#define AGS_SHARED_UTIL_COMPRESS_H

#include "ags/shared/core/types.h"
#include "ags/shared/util/wgt2_allg.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
class Bitmap;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

bool rle_compress(const uint8_t *data, size_t data_sz, int image_bpp, Shared::Stream *out);
bool rle_decompress(uint8_t *data, size_t data_sz, int image_bpp, Shared::Stream *in);
// Packs a 8-bit bitmap using RLE compression, and writes into stream along with the palette
void save_rle_bitmap8(Shared::Stream *out, const Shared::Bitmap *bmp, const RGB(*pal)[256] = nullptr);
// Reads a 8-bit bitmap with palette from the stream and unpacks from RLE
Shared::Bitmap *load_rle_bitmap8(Shared::Stream *in, RGB(*pal)[256] = nullptr);
// Skips the 8-bit RLE bitmap
void skip_rle_bitmap8(Shared::Stream *in);

// LZW compression
bool lzw_compress(const uint8_t *data, size_t data_sz, int image_bpp, Shared::Stream *out);
bool lzw_decompress(uint8_t *data, size_t data_sz, int image_bpp, Shared::Stream *in, size_t in_sz);
// Saves bitmap with an optional palette compressed by LZW
void save_lzw(Shared::Stream *out, const Shared::Bitmap *bmpp, const RGB(*pal)[256] = nullptr);
// Loads bitmap decompressing
Shared::Bitmap *load_lzw(Shared::Stream *in, int dst_bpp, RGB (*pal)[256] = nullptr);

// Deflate compression
bool deflate_compress(const uint8_t *data, size_t data_sz, int image_bpp, Shared::Stream *out);
bool inflate_decompress(uint8_t *data, size_t data_sz, int image_bpp, Shared::Stream *in, size_t in_sz);

} // namespace AGS3

#endif
