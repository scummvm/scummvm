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

#ifndef __AGS_EE_GFX__GFXUTIL_H
#define __AGS_EE_GFX__GFXUTIL_H

#include "gfx/bitmap.h"
#include "gfx/gfx_def.h"

namespace AGS
{
namespace Engine
{

using Common::Bitmap;

namespace GfxUtil
{
    // Creates a COPY of the source bitmap, converted to the given format.
    Bitmap *ConvertBitmap(Bitmap *src, int dst_color_depth);

    // Considers the given information about source and destination surfaces,
    // then draws a bimtap over another either using requested blending mode,
    // or fallbacks to common "magic pink" transparency mode;
    // optionally uses blending alpha (overall image transparency).
    void DrawSpriteBlend(Bitmap *ds, const Point &ds_at, Bitmap *sprite,
        Common::BlendMode blend_mode, bool dst_has_alpha = true, bool src_has_alpha = true, int blend_alpha = 0xFF);

    // Draws a bitmap over another one with given alpha level (0 - 255),
    // takes account of the bitmap's mask color,
    // ignores image's alpha channel, even if there's one;
    // does proper conversion depending on respected color depths.
    void DrawSpriteWithTransparency(Bitmap *ds, Bitmap *sprite, int x, int y, int alpha = 0xFF);
} // namespace GfxUtil

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__GFXUTIL_H
