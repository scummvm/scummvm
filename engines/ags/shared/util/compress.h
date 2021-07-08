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

void rle_compress(Shared::Bitmap *, Shared::Stream *);
void rle_decompress(Shared::Bitmap *, Shared::Stream *);

// LZW compression
void save_lzw(Shared::Stream *out, const Shared::Bitmap *bmpp, const RGB *pall);
void load_lzw(Shared::Stream *in, Shared::Bitmap **bmm, int dst_bpp, RGB *pall);
void savecompressed_allegro(Shared::Stream *out, const Shared::Bitmap *bmpp, const RGB *pall);
void loadcompressed_allegro(Shared::Stream *in, Shared::Bitmap **bimpp, RGB *pall);

} // namespace AGS3

#endif
