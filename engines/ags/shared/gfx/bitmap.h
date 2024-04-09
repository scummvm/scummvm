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
// NOTE: color_depth is in BITS per pixel (i.e. 8, 16, 24, 32...).
// NOTE: in all of these color_depth may be passed as 0 in which case a default
// color depth will be used (as previously set for the system).
// Creates a new bitmap of the given format; the pixel contents are undefined.
Bitmap *CreateBitmap(int width, int height, int color_depth = 0);
// Creates a new bitmap and clears it with the given color
Bitmap *CreateClearBitmap(int width, int height, int color_depth = 0, int clear_color = 0);
// Creates a new bitmap and clears it with the transparent color
Bitmap *CreateTransparentBitmap(int width, int height, int color_depth = 0);
// Creates a sub-bitmap of the given bitmap; the sub-bitmap is a reference to
// particular region inside a parent.
// WARNING: the parent bitmap MUST be kept in memory for as long as sub-bitmap exists!
Bitmap *CreateSubBitmap(Bitmap *src, const Rect &rc);
// Creates a plain copy of the given bitmap, optionally converting to a different color depth;
// pass color depth 0 to keep the original one.
Bitmap *CreateBitmapCopy(Bitmap *src, int color_depth = 0);

// Load a bitmap from file; supported formats currently are: BMP, PCX.
Bitmap *LoadFromFile(const char *filename);
inline Bitmap *LoadFromFile(const String &filename) {
	return LoadFromFile(filename.GetCStr());
}
Bitmap *LoadFromFile(PACKFILE *pf);

// Stretches bitmap to the requested size. The new bitmap will have same
// colour depth. Returns original bitmap if no changes are necessary.
Bitmap *AdjustBitmapSize(Bitmap *src, int width, int height);
// Makes the given bitmap opaque (full alpha), while keeping pixel RGB unchanged.
void    MakeOpaque(Bitmap *bmp);
// Makes the given bitmap opaque (full alpha), while keeping pixel RGB unchanged.
// Skips mask color (leaves it with zero alpha).
void    MakeOpaqueSkipMask(Bitmap *bmp);
// Replaces fully transparent (alpha = 0) pixels with standard mask color.
void    ReplaceAlphaWithRGBMask(Bitmap *bmp);
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
