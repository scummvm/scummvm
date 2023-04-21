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

//=============================================================================
//
// Base bitmap header
//
//=============================================================================

#ifndef AGS_SHARED_GFX_BITMAP_H
#define AGS_SHARED_GFX_BITMAP_H

#include "ags/shared/gfx/gfx_def.h"
#include "ags/shared/util/geometry.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

// Mask option for blitting one bitmap on another
enum BitmapMaskOption {
	// Plain copies bitmap pixels
	kBitmap_Copy,
	// Consider mask color fully transparent and do not copy pixels having it
	kBitmap_Transparency
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#include "ags/shared/gfx/allegro_bitmap.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class Bitmap;

// TODO: revise this construction later
namespace BitmapHelper {

// Helper functions, that delete faulty bitmaps automatically, and return
// NULL if bitmap could not be created.
Bitmap *CreateBitmap(int width, int height, int color_depth = 0);
Bitmap *CreateClearBitmap(int width, int height, int color_depth = 0, int clear_color = 0);
Bitmap *CreateTransparentBitmap(int width, int height, int color_depth = 0);
Bitmap *CreateSubBitmap(Bitmap *src, const Rect &rc);
Bitmap *CreateBitmapCopy(Bitmap *src, int color_depth = 0);
Bitmap *LoadFromFile(const char *filename);
inline Bitmap *LoadFromFile(const String &filename) {
	return LoadFromFile(filename.GetCStr());
}
Bitmap *LoadFromFile(PACKFILE *pf);

// Stretches bitmap to the requested size. The new bitmap will have same
// colour depth. Returns original bitmap if no changes are necessary.
Bitmap *AdjustBitmapSize(Bitmap *src, int width, int height);
// Copy transparency mask and/or alpha channel from one bitmap into another.
// Destination and mask bitmaps must be of the same pixel format.
// Transparency is merged, meaning that fully transparent pixels on
// destination should remain such regardless of mask pixel values.
void    CopyTransparency(Bitmap *dst, const Bitmap *mask, bool dst_has_alpha, bool mask_has_alpha);
// Copy pixel data into bitmap from memory buffer. It is required that the
// source matches bitmap format and has enough data.
// Pitch is given in bytes and defines the length of the source scan line.
// Offset is optional and defines horizontal offset, in pixels.
void    ReadPixelsFromMemory(Bitmap *dst, const uint8_t *src_buffer, const size_t src_pitch, const size_t src_px_offset = 0);

} // namespace BitmapHelper
} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
