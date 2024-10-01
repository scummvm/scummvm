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
// Intermediate level drawing utility functions.
//
// GfxUtil namespace is meant for intermediate-to-lower level functions, that
// implement specific conversions, tricks and hacks for drawing bitmaps and
// geometry.
// The suggested convention is to add only those functions, that do not require
// any knowledge of higher-level engine types and objects.
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_GFX_UTIL_H
#define AGS_ENGINE_GFX_GFX_UTIL_H

#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gfx/gfx_def.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

using Shared::Bitmap;

namespace GfxUtil {
// Creates a COPY of the source bitmap, converted to the given format.
// Keeps mask pixels intact, only converting mask color value if necessary.
Bitmap *ConvertBitmap(Bitmap *src, int dst_color_depth);

// Considers the given information about source and destination surfaces,
// then draws a bimtap over another either using requested blending mode,
// or fallbacks to common "magic pink" transparency mode;
// optionally uses blending alpha (overall image transparency).
void DrawSpriteBlend(Bitmap *ds, const Point &ds_at, Bitmap *sprite,
                     Shared::BlendMode blend_mode, bool dst_has_alpha = true, bool src_has_alpha = true, int blend_alpha = 0xFF);

// Draws a bitmap over another one with given alpha level (0 - 255),
// takes account of the bitmap's mask color,
// ignores image's alpha channel, even if there's one;
// does a conversion if sprite and destination color depths do not match.
void DrawSpriteWithTransparency(Bitmap *ds, Bitmap *sprite, int x, int y, int alpha = 0xFF);
} // namespace GfxUtil

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
