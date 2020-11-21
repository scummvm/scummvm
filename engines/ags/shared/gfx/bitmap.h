//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// Base bitmap header
//
//=============================================================================
#ifndef __AGS_CN_GFX__BITMAP_H
#define __AGS_CN_GFX__BITMAP_H

#include "util/geometry.h"

namespace AGS
{
namespace Common
{

// Mask option for blitting one bitmap on another
enum BitmapMaskOption
{
	// Plain copies bitmap pixels
	kBitmap_Copy,
	// Consider mask color fully transparent and do not copy pixels having it
	kBitmap_Transparency
};

enum BitmapFlip
{
	kBitmap_HFlip,
	kBitmap_VFlip,
	kBitmap_HVFlip
};

} // namespace Common
} // namespace AGS


// Declare the actual bitmap class
#include "gfx/allegrobitmap.h"

namespace AGS
{
namespace Common
{

class Bitmap;

// TODO: revise this construction later
namespace BitmapHelper
{
    // Helper functions, that delete faulty bitmaps automatically, and return
    // NULL if bitmap could not be created.
    Bitmap *CreateBitmap(int width, int height, int color_depth = 0);
    Bitmap *CreateTransparentBitmap(int width, int height, int color_depth = 0);
	Bitmap *CreateSubBitmap(Bitmap *src, const Rect &rc);
    Bitmap *CreateBitmapCopy(Bitmap *src, int color_depth = 0);
	Bitmap *LoadFromFile(const char *filename);

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

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_GFX__BITMAP_H
