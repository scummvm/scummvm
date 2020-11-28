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

#ifndef AGS_LIB_ALLEGRO_COLOR_H
#define AGS_LIB_ALLEGRO_COLOR_H

#include "common/scummsys.h"
#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/alconfig.h"

namespace AGS3 {

#define PAL_SIZE     256
#define MASK_COLOR_8       0
#define MASK_COLOR_15      0x7C1F
#define MASK_COLOR_16      0xF81F
#define MASK_COLOR_24      0xFF00FF
#define MASK_COLOR_32      0xFF00FF

class BITMAP;

#include "common/pack-start.h"  // START STRUCT PACKING

struct color {
	byte r, g, b;
} PACKED_STRUCT;

typedef color RGB;
typedef RGB PALETTE[PAL_SIZE];

AL_VAR(PALETTE, _current_palette);

#include "common/pack-end.h"    // END STRUCT PACKING

struct RGB_MAP {
	byte data[32][32][32];
};

struct COLOR_MAP {
	byte data[PAL_SIZE][PAL_SIZE];
};

AL_VAR(PALETTE, black_palette);
AL_VAR(PALETTE, desktop_palette);
AL_VAR(PALETTE, default_palette);

AL_VAR(RGB_MAP *, rgb_map);
AL_VAR(COLOR_MAP *, color_map);

AL_VAR(PALETTE, _current_palette);

extern int _rgb_r_shift_15;
extern int _rgb_g_shift_15;
extern int _rgb_b_shift_15;
extern int _rgb_r_shift_16;
extern int _rgb_g_shift_16;
extern int _rgb_b_shift_16;
extern int _rgb_r_shift_24;
extern int _rgb_g_shift_24;
extern int _rgb_b_shift_24;
extern int _rgb_r_shift_32;
extern int _rgb_g_shift_32;
extern int _rgb_b_shift_32;
extern int _rgb_a_shift_32;

AL_ARRAY(int, _rgb_scale_5);
AL_ARRAY(int, _rgb_scale_6);

AL_FUNC(void, set_color, (int idx, AL_CONST RGB *p));
AL_FUNC(void, set_palette, (AL_CONST PALETTE p));
AL_FUNC(void, set_palette_range, (AL_CONST PALETTE p, int from, int to, int retracesync));

AL_FUNC(void, get_color, (int idx, RGB *p));
AL_FUNC(void, get_palette, (PALETTE p));
AL_FUNC(void, get_palette_range, (PALETTE p, int from, int to));

AL_FUNC(void, fade_interpolate, (AL_CONST PALETTE source, AL_CONST PALETTE dest, PALETTE output, int pos, int from, int to));
AL_FUNC(void, fade_from_range, (AL_CONST PALETTE source, AL_CONST PALETTE dest, int speed, int from, int to));
AL_FUNC(void, fade_in_range, (AL_CONST PALETTE p, int speed, int from, int to));
AL_FUNC(void, fade_out_range, (int speed, int from, int to));
AL_FUNC(void, fade_from, (AL_CONST PALETTE source, AL_CONST PALETTE dest, int speed));
AL_FUNC(void, fade_in, (AL_CONST PALETTE p, int speed));
AL_FUNC(void, fade_out, (int speed));

AL_FUNC(void, select_palette, (AL_CONST PALETTE p));
AL_FUNC(void, unselect_palette, (void));

AL_FUNC(void, generate_332_palette, (PALETTE pal));
AL_FUNC(int, generate_optimized_palette, (BITMAP *image, PALETTE pal, AL_CONST signed char rsvdcols[256]));

AL_FUNC(void, create_rgb_table, (RGB_MAP *table, AL_CONST PALETTE pal, AL_METHOD(void, callback, (int pos))));
AL_FUNC(void, create_light_table, (COLOR_MAP *table, AL_CONST PALETTE pal, int r, int g, int b, AL_METHOD(void, callback, (int pos))));
AL_FUNC(void, create_trans_table, (COLOR_MAP *table, AL_CONST PALETTE pal, int r, int g, int b, AL_METHOD(void, callback, (int pos))));
AL_FUNC(void, create_color_table, (COLOR_MAP *table, AL_CONST PALETTE pal, AL_METHOD(void, blend, (AL_CONST PALETTE pal, int x, int y, RGB *rgb)), AL_METHOD(void, callback, (int pos))));
AL_FUNC(void, create_blender_table, (COLOR_MAP *table, AL_CONST PALETTE pal, AL_METHOD(void, callback, (int pos))));

typedef AL_METHOD(unsigned long, BLENDER_FUNC, (unsigned long x, unsigned long y, unsigned long n));

AL_FUNC(void, set_blender_mode, (BLENDER_FUNC b15, BLENDER_FUNC b16, BLENDER_FUNC b24, int r, int g, int b, int a));
AL_FUNC(void, set_blender_mode_ex, (BLENDER_FUNC b15, BLENDER_FUNC b16, BLENDER_FUNC b24, BLENDER_FUNC b32, BLENDER_FUNC b15x, BLENDER_FUNC b16x, BLENDER_FUNC b24x, int r, int g, int b, int a));

AL_FUNC(void, set_alpha_blender, (void));
AL_FUNC(void, set_write_alpha_blender, (void));
AL_FUNC(void, set_trans_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_add_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_burn_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_color_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_difference_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_dissolve_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_dodge_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_hue_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_invert_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_luminance_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_multiply_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_saturation_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_screen_blender, (int r, int g, int b, int a));

AL_FUNC(void, hsv_to_rgb, (float h, float s, float v, int *r, int *g, int *b));
AL_FUNC(void, rgb_to_hsv, (int r, int g, int b, float *h, float *s, float *v));

AL_FUNC(int, bestfit_color, (AL_CONST PALETTE pal, int r, int g, int b));

AL_FUNC(int, makecol, (int r, int g, int b));
AL_FUNC(int, makecol8, (int r, int g, int b));
AL_FUNC(int, makecol_depth, (int color_depth, int r, int g, int b));

AL_FUNC(int, makeacol, (int r, int g, int b, int a));
AL_FUNC(int, makeacol_depth, (int color_depth, int r, int g, int b, int a));

AL_FUNC(int, makecol15_dither, (int r, int g, int b, int x, int y));
AL_FUNC(int, makecol16_dither, (int r, int g, int b, int x, int y));

AL_FUNC(int, getr, (int c));
AL_FUNC(int, getg, (int c));
AL_FUNC(int, getb, (int c));
AL_FUNC(int, geta, (int c));

AL_FUNC(int, getr_depth, (int color_depth, int c));
AL_FUNC(int, getg_depth, (int color_depth, int c));
AL_FUNC(int, getb_depth, (int color_depth, int c));
AL_FUNC(int, geta_depth, (int color_depth, int c));

extern int makecol15(int r, int g, int b);
extern int makecol16(int r, int g, int b);
extern int makecol24(int r, int g, int b);
extern int makecol32(int r, int g, int b);
extern int makeacol32(int r, int g, int b, int a);
extern int getr8(int c);
extern int getg8(int c);
extern int getb8(int c);
extern int getr15(int c);
extern int getg15(int c);
extern int getb15(int c);
extern int getr16(int c);
extern int getg16(int c);
extern int getb16(int c);
extern int getr32(int c);
extern int getg32(int c);
extern int getb32(int c);
extern int geta32(int c);

extern int getr_depth(int color_depth, int c);
extern int getg_depth(int color_depth, int c);
extern int getb_depth(int color_depth, int c);
extern int geta_depth(int color_depth, int c);

} // namespace AGS3

#endif
