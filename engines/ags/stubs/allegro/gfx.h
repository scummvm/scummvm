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

#ifndef AGS_STUBS_ALLEGRO_GRAPHICS_H
#define AGS_STUBS_ALLEGRO_GRAPHICS_H

#include "graphics/managed_surface.h"
#include "ags/stubs/allegro/base.h"

namespace AGS3 {

#define GFX_TEXT                       -1
#define GFX_AUTODETECT                 0
#define GFX_AUTODETECT_FULLSCREEN      1
#define GFX_AUTODETECT_WINDOWED        2
#define GFX_SAFE                       AL_ID('S','A','F','E')
#define GFX_NONE                       AL_ID('N','O','N','E')

/* drawing modes for draw_sprite_ex() */
#define DRAW_SPRITE_NORMAL 0
#define DRAW_SPRITE_LIT 1
#define DRAW_SPRITE_TRANS 2

/* flipping modes for draw_sprite_ex() */
#define DRAW_SPRITE_NO_FLIP 0x0
#define DRAW_SPRITE_H_FLIP  0x1
#define DRAW_SPRITE_V_FLIP  0x2
#define DRAW_SPRITE_VH_FLIP 0x3

/* Blender mode defines, for the gfx_driver->set_blender_mode() function */
#define blender_mode_none            0
#define blender_mode_trans           1
#define blender_mode_add             2
#define blender_mode_burn            3
#define blender_mode_color           4
#define blender_mode_difference      5
#define blender_mode_dissolve        6
#define blender_mode_dodge           7
#define blender_mode_hue             8
#define blender_mode_invert          9
#define blender_mode_luminance      10
#define blender_mode_multiply       11
#define blender_mode_saturation     12
#define blender_mode_screen         13
#define blender_mode_alpha          14


#define SCREEN_W     (gfx_driver ? gfx_driver->w : 0)
#define SCREEN_H     (gfx_driver ? gfx_driver->h : 0)

#define VIRTUAL_W    (screen ? screen->w : 0)
#define VIRTUAL_H    (screen ? screen->h : 0)

#define COLORCONV_NONE              0

#define COLORCONV_8_TO_15           1
#define COLORCONV_8_TO_16           2
#define COLORCONV_8_TO_24           4
#define COLORCONV_8_TO_32           8

#define COLORCONV_15_TO_8           0x10
#define COLORCONV_15_TO_16          0x20
#define COLORCONV_15_TO_24          0x40
#define COLORCONV_15_TO_32          0x80

#define COLORCONV_16_TO_8           0x100
#define COLORCONV_16_TO_15          0x200
#define COLORCONV_16_TO_24          0x400
#define COLORCONV_16_TO_32          0x800

#define COLORCONV_24_TO_8           0x1000
#define COLORCONV_24_TO_15          0x2000
#define COLORCONV_24_TO_16          0x4000
#define COLORCONV_24_TO_32          0x8000

#define COLORCONV_32_TO_8           0x10000
#define COLORCONV_32_TO_15          0x20000
#define COLORCONV_32_TO_16          0x40000
#define COLORCONV_32_TO_24          0x80000

#define COLORCONV_32A_TO_8          0x100000
#define COLORCONV_32A_TO_15         0x200000
#define COLORCONV_32A_TO_16         0x400000
#define COLORCONV_32A_TO_24         0x800000

#define COLORCONV_DITHER_PAL        0x1000000
#define COLORCONV_DITHER_HI         0x2000000
#define COLORCONV_KEEP_TRANS        0x4000000

#define COLORCONV_DITHER            (COLORCONV_DITHER_PAL |          \
                                     COLORCONV_DITHER_HI)

#define COLORCONV_EXPAND_256        (COLORCONV_8_TO_15 |             \
                                     COLORCONV_8_TO_16 |             \
                                     COLORCONV_8_TO_24 |             \
                                     COLORCONV_8_TO_32)

#define COLORCONV_REDUCE_TO_256     (COLORCONV_15_TO_8 |             \
                                     COLORCONV_16_TO_8 |             \
                                     COLORCONV_24_TO_8 |             \
                                     COLORCONV_32_TO_8 |             \
                                     COLORCONV_32A_TO_8)

#define COLORCONV_EXPAND_15_TO_16    COLORCONV_15_TO_16

#define COLORCONV_REDUCE_16_TO_15    COLORCONV_16_TO_15

#define COLORCONV_EXPAND_HI_TO_TRUE (COLORCONV_15_TO_24 |            \
                                     COLORCONV_15_TO_32 |            \
                                     COLORCONV_16_TO_24 |            \
                                     COLORCONV_16_TO_32)

#define COLORCONV_REDUCE_TRUE_TO_HI (COLORCONV_24_TO_15 |            \
                                     COLORCONV_24_TO_16 |            \
                                     COLORCONV_32_TO_15 |            \
                                     COLORCONV_32_TO_16)

#define COLORCONV_24_EQUALS_32      (COLORCONV_24_TO_32 |            \
                                     COLORCONV_32_TO_24)

#define COLORCONV_TOTAL             (COLORCONV_EXPAND_256 |          \
                                     COLORCONV_REDUCE_TO_256 |       \
                                     COLORCONV_EXPAND_15_TO_16 |     \
                                     COLORCONV_REDUCE_16_TO_15 |     \
                                     COLORCONV_EXPAND_HI_TO_TRUE |   \
                                     COLORCONV_REDUCE_TRUE_TO_HI |   \
                                     COLORCONV_24_EQUALS_32 |        \
                                     COLORCONV_32A_TO_15 |           \
                                     COLORCONV_32A_TO_16 |           \
                                     COLORCONV_32A_TO_24)

#define COLORCONV_PARTIAL           (COLORCONV_EXPAND_15_TO_16 |     \
                                     COLORCONV_REDUCE_16_TO_15 |     \
                                     COLORCONV_24_EQUALS_32)

#define COLORCONV_MOST              (COLORCONV_EXPAND_15_TO_16 |     \
                                     COLORCONV_REDUCE_16_TO_15 |     \
                                     COLORCONV_EXPAND_HI_TO_TRUE |   \
                                     COLORCONV_REDUCE_TRUE_TO_HI |   \
                                     COLORCONV_24_EQUALS_32)

#define COLORCONV_KEEP_ALPHA        (COLORCONV_TOTAL                 \
                                     & ~(COLORCONV_32A_TO_8 |        \
                                             COLORCONV_32A_TO_15 |       \
                                             COLORCONV_32A_TO_16 |       \
                                             COLORCONV_32A_TO_24))

class BITMAP : public Graphics::ManagedSurface {
};

AL_FUNC(void, set_color_conversion, (int mode));
AL_FUNC(int, get_color_conversion, ());
AL_FUNC(int, set_gfx_mode, (int card, int w, int h, int v_w, int v_h));

} // namespace AGS3

#endif
