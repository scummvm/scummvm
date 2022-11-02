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

#ifndef AGS_LIB_ALLEGRO_COLOR_H
#define AGS_LIB_ALLEGRO_COLOR_H

#include "common/scummsys.h"
#include "graphics/screen.h"
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

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

#include "common/pack-start.h"  // START STRUCT PACKING

struct color {
	byte r, g, b;
	byte filler;

	void readFromFile(AGS::Shared::Stream *file);
	void writeToFile(AGS::Shared::Stream *file) const;
	void clear() {
		r = g = b = filler = 0;
	}
} PACKED_STRUCT;

typedef color RGB;
typedef RGB PALETTE[PAL_SIZE];

#include "common/pack-end.h"    // END STRUCT PACKING

struct RGB_MAP {
	byte data[32][32][32];
};

struct COLOR_MAP {
	byte data[PAL_SIZE][PAL_SIZE];
};

enum BlenderMode {
	kSourceAlphaBlender,
	kArgbToArgbBlender,
	kArgbToRgbBlender,
	kRgbToArgbBlender,
	kRgbToRgbBlender,
	kAlphaPreservedBlenderMode,
	kOpaqueBlenderMode,
	kAdditiveBlenderMode,
	kTintBlenderMode,
	kTintLightBlenderMode
};

extern int makecol(byte r, byte g, byte b);

AL_ARRAY(const int, _rgb_scale_5);
AL_ARRAY(const int, _rgb_scale_6);

AL_FUNC(void, set_palette, (AL_CONST PALETTE p));
AL_FUNC(void, set_palette_range, (AL_CONST PALETTE p, int from, int to, int retracesync));

AL_FUNC(void, get_color, (int idx, RGB *p));
AL_FUNC(void, get_palette, (PALETTE p));
AL_FUNC(void, get_palette_range, (PALETTE p, int from, int to));

AL_FUNC(void, fade_interpolate, (AL_CONST PALETTE source, AL_CONST PALETTE dest, PALETTE output, int pos, int from, int to));

AL_FUNC(void, select_palette, (AL_CONST PALETTE p));
AL_FUNC(void, unselect_palette, (void));

AL_FUNC(void, create_rgb_table, (RGB_MAP *table, AL_CONST PALETTE pal, AL_METHOD(void, callback, (int pos))));
AL_FUNC(void, create_light_table, (COLOR_MAP *table, AL_CONST PALETTE pal, int r, int g, int b, AL_METHOD(void, callback, (int pos))));
AL_FUNC(void, create_trans_table, (COLOR_MAP *table, AL_CONST PALETTE pal, int r, int g, int b, AL_METHOD(void, callback, (int pos))));

AL_FUNC(void, set_blender_mode, (BlenderMode, int r, int g, int b, int a));
AL_FUNC(void, set_alpha_blender, (void));
AL_FUNC(void, set_trans_blender, (int r, int g, int b, int a));

AL_FUNC(void, hsv_to_rgb, (float h, float s, float v, int *r, int *g, int *b));
AL_FUNC(void, rgb_to_hsv, (int r, int g, int b, float *h, float *s, float *v));

AL_FUNC(int, bestfit_color, (AL_CONST PALETTE pal, int r, int g, int b));

AL_FUNC(int, makecol8, (int r, int g, int b));
AL_FUNC(int, makecol_depth, (int color_depth, int r, int g, int b));

AL_FUNC(int, makeacol_depth, (int color_depth, int r, int g, int b, int a));

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
extern int getr24(int c);
extern int getg24(int c);
extern int getb24(int c);
extern int getr32(int c);
extern int getg32(int c);
extern int getb32(int c);
extern int geta32(int c);

} // namespace AGS3

#endif
