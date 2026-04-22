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

#ifndef MADS_CORE_COLOR_H
#define MADS_CORE_COLOR_H

#include "common/stream.h"
#include "mads/madsv2/core/loader.h"

namespace MADS {
namespace MADSV2 {


#define COLOR_MAX_USER_COLORS           256
#define COLOR_FIRST_USER_COLOR          16

#define COLOR_MAX_SHADOW_COLORS         3

#define COLOR_MAX_CYCLES                8

#define COLOR_HIGHEST                   254
#define COLOR_TRANSPARENT               255

#define COLOR_SOLID                     0
#define COLOR_BLACK_THATCH              0x0d
#define COLOR_NO_X16                    0x0ff

#define COLOR_ERR_LISTOVERFLOW          -1

#define COLOR_GROUP_MAP_TO_CYCLE        0x80  /* Map to cycling colors  */
#define COLOR_GROUP_MAP_TO_CLOSEST      0x40  /* Map to closest color   */
#define COLOR_GROUP_FORCE_TO_CLOSEST    0x20  /* Force to closest color */
#define COLOR_GROUP_MAP_TO_SHADOW       0x10  /* Map to shadow color    */
#define COLOR_GROUP_FORCE_TO_SKIP       0x08  /* Force to skip code     */

#define COLOR_GROUP_SPECIAL_2           0x02  /* Special color group 2  */
#define COLOR_GROUP_SPECIAL_1           0x01  /* Special color group 1  */

#define COLOR_DP(x) (((((x + 1) * 101) + 26) / 64) - 1)


/* A fully parameterized color (in contrast to a plain RGBcolor as */
/* defined in general.mac)                                         */

struct Color {
	byte r, g, b;         /* RGB values                  */
	byte x16;           /* 16 color dither translation */
	byte cycle;         /* Color cycling handle        */
	byte group;         /* Color grouping flags        */

	static constexpr int SIZE = 1 + 1 + 1 + 1 + 1 + 1;
	void load(Common::SeekableReadStream *src);
};

typedef Color *ColorPtr;


/* Palette-independent color list; ready to map into palette */

struct ColorList {
	uint16 num_colors;
	Color table[COLOR_MAX_USER_COLORS];

	static constexpr int SIZE = 2 + (Color::SIZE * COLOR_MAX_USER_COLORS);
	bool load(Load &load_handle, int size);
	void load(Common::SeekableReadStream *src);
};

typedef ColorList *ColorListPtr;


/* Palette-independent color cycling range & timing information */

struct Cycle {
	byte num_colors;                    /* Number of colors in the cycle */
	byte first_list_color;              /* First color in color list     */
	byte first_palette_color;           /* First color in final palette  */
	byte ticks;                         /* 60/s ticks between cycles     */

	static constexpr int SIZE = 1 + 1 + 1 + 1;
	void load(Common::SeekableReadStream *src);
};

typedef Cycle *CyclePtr;


/* List of color cycling ranges */

struct CycleList {
	int   num_cycles;
	Cycle table[COLOR_MAX_CYCLES];

	static constexpr int SIZE = 2 + (Cycle::SIZE * COLOR_MAX_CYCLES);
	void load(Common::SeekableReadStream *src);
};

typedef CycleList *CycleListPtr;


struct ShadowList {
	uint16 num_shadow_colors;
	int shadow_color[COLOR_MAX_SHADOW_COLORS];

	static constexpr int SIZE = 2 + (2 * COLOR_MAX_SHADOW_COLORS);
	void load(Common::SeekableReadStream *src);
};

typedef ShadowList *ShadowListPtr;

/**
 * Given a main color (0-15, or 16 for "no translation") and a
 * thatching color (0-15, same as main color for solid thatch),
 * produces a valid "thatch" byte.
 */
extern byte color_thatch(int color, int thatching);
extern void color_list_start_scan(byte *list_flags);

/**
 * Updates a color list <list>, based on colors used in <scan_buf>
 * and defined in <scan_pal>.  Adds any colors used in picture but
 * not defined in list to the list; sets the corresponding color flag
 * for each color that is used in the buffer.
 *
 * @return		Returns # of colors in updated list, or negative for error.
 */
extern int color_list_update(ColorListPtr list, Buffer *scan_buf,
	Palette *scan_pal, byte *list_flags,
	byte *palette_map, CycleListPtr cycle);

/**
 * Removes from color list "list" any colors whose corresponding
 * flags are not set.  (Use after color_list_update to remove colors
 * no longer being used).  Voids any previously existing palette_map.
*/
extern int color_list_purge(ColorListPtr list, byte *list_flags);

/**
 * Adds the colors in the given list to the given palette, filling
 * from the specified "base color".  Also generates a palette_map
 * which can be used to map palette colors back into the color list.
*/
extern int color_list_palette(ColorListPtr list, Buffer *scan_buf,
	Palette *scan_pal, int base_color,
	byte *palette_map, CycleListPtr cycle);

/**
 * Used by roomedit when loading attribute buffers whose colors
 * are often based on the main image, but which are not necessarily
 * consistent and may contain additional, if irrelevant, colors.
 * Conforms the incoming picture with the available palette.  Priority
 * of actions for each color used is:
 *         (1) Use equivalent color already existing in palette.
 *         (2) Allocate a new palette color if available.
 *         (3) Use "white".
 */
extern void color_list_conform(ColorListPtr list, Buffer *scan_buf,
	Palette *scan_pal, Palette *main_pal,
	int base_color);

/**
 * Fills the specified area of the specified buffer with the
 * specified 16-color thatch (analogous to buffer_rect_fill).
 */
extern void color_trans_fill_buf(Buffer unto, int unto_x, int unto_y,
	int size_x, int size_y, byte thatch_color);

/**
 * Copies a rectangle of the buffer "from" into the buffer "unto",
 * But uses the specified color list and palette map to perform
 * 256-to-16 color translations.  If "mask_flag" is true, then
 * displays "white" if a translation exists for a pixel, "black"
 * otherwise.  If "mask_flag" is false, then translated color is
 * displayed if available, and untranslated color is displayed
 * otherwise.
 *
 *      Analogous to buffer_rect_copy();
 */
extern void color_trans_show_buf(Buffer from, Buffer unto,
	int from_x, int from_y, int unto_x, int unto_y,
	int size_x, int size_y, ColorListPtr list,
	byte *palette_map, int mask_flag);

extern void color_buffer_palette_to_list(ColorListPtr list,
	Buffer *scan_buf, byte *palette_map);
extern void color_buffer_list_to_palette(Buffer *scan_buf, int marker);
extern void color_transparent_swap(Buffer *scan_buf, Palette *scan_pal, byte transparent);
extern void color_buffer_list_to_main(ColorListPtr color_list, Buffer *scan_buf);
extern void color_buffer_list_to_x16(ColorListPtr color_list, Buffer *scan_buf);

/**
 * Given a valid thatch byte, returns a primary "color" and
 * secondary "thatching" color.
 */
extern void color_split_thatch(int thatch, int *color, int *thatching);

} // namespace MADSV2
} // namespace MADS

#endif
