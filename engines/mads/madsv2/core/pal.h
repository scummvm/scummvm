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

#ifndef MADS_CORE_PAL_H
#define MADS_CORE_PAL_H

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/color.h"

namespace MADS {
namespace MADSV2 {

/* Palette mapping options to be passed to pal_allocate */

#define PAL_MAP_BACKGROUND      0x8000  /* Loading initial background       */
#define PAL_MAP_RESERVED        0x4000  /* Allow mapping of reserved colors */
#define PAL_MAP_ANY_TO_CLOSEST  0x2000  /* Any color can map to closest     */
#define PAL_MAP_ALL_TO_CLOSEST  0x1000  /* Any colors that can map must map */
#define PAL_MAP_TOP_COLORS      0x0800  /* Allow mapping to top four colors */
#define PAL_MAP_DEFINE_RESERVED 0x0400  /* Define initial reserved color    */

#define PAL_MAP_MASK            0xfc00  /* Mask for all palette flags on,   */
					/* so that a routine can accept PAL */
					/* flags along with other flags of  */
					/* its own in the same register, &  */
					/* then forward a clean copy of the */
					/* PAL flags to pal_allocate().     */


#define PAL_FORCE_SHADOW        240

/* Internal facts */

#define PAL_MAXFLAGS            0x20    /* Number of available flags */

#define PAL_RESERVED            0x0001  /* ... 0001 */
#define PAL_CYCLE               0x0002  /* ... 0010 */


/* Returned by pal_deallocate */

#define PAL_ERR_BADFLAG         -1      /* Flag number out of range */
#define PAL_ERR_FLAGNOTUSED     -2      /* Attempt to clear unused flag */


/* Returned by pal_allocate   */

#define PAL_ERR_OUTOFFLAGS      -10     /* No available usage flags */
#define PAL_ERR_OUTOFCOLORS     -11     /* Not enough available colors */


/* Data available to system: */

extern Palette master_palette;       /* Main global palette for program */
extern ShadowListPtr master_shadow;  /* Main global shadow color list   */

extern dword color_status[256];      /* Mapping status of each palette color */
extern int   flag_used[PAL_MAXFLAGS];/* Mapping status of each palette handle*/

extern int palette_locked;

extern int palette_low_search_limit; /* Artificial search boundaries */
extern int palette_high_search_limit;

extern void (*pal_manager_update)();
extern int pal_manager_active;
extern int pal_manager_colors;


/**
 * Initializes an empty palette with no owners.  Reserves the
 * first <reserve_bottom> colors and last <reserve_top> colors
 * for the system.  Color 0 is always reserved.
 */
extern void pal_init(int reserve_bottom, int reserve_top);

extern void pal_lock(void);
extern void pal_unlock(void);

/**
 * Given a color list, allocates all its colors in the global palette.
 * Function returns a color usage handle which can be used later by
 * pal_deallocate.  A negative result signifies an error.
 * The color list pointed to by new_list is udpated such that when
 * a color is allocated, its new value is stored in the X16 byte.
 *
 * Flags available include:
 *
 * PAL_MAP_BACKGROUND      set when loading the initial color list
 *                         for the room background; allows new cycling
 *                         ranges to be created.
 *
 * PAL_MAP_RESERVED        allows colors to map to the reserved
 *                         color areas.
 *
 * PAL_MAP_ANY_TO_CLOSEST  allows any color to map to the closest
 *                         available color instead of an exact match.
 *
 * PAL_MAP_ALL_TO_CLOSEST  forces any color that is allowed to map
 *                         to the closest color to do so even if free
 *                         color slots exist.  If combined with the
 *                         previous flag, no new colors will be added
 *                         to the palette.
 */
extern int pal_allocate(ColorListPtr new_list, ShadowListPtr shadow_list, int pal_flags);

/**
 * Erases usage of of colors marked by use_flag, and returns that
 * flag to availability.  Returns PAL_ error code if failure.
 *
 * NOTE: If the FLAGNOTUSED error is returned, the flag's colors
 * (if any) have ALREADY been removed from the list by the function.
 * This allows the pal_allocate function to clean up after an
 * OUTOFCOLORS error.
 */
extern int  pal_deallocate(int use_flag);

/**
 * Compresses a list of handles to a single handle (designed for use
 * by room loader so that background sprites don't take up lots of handles)
 */
extern void pal_compact(word master_handle, int num_slaves, word *slave);

/**
 * Computes the sum of squared differences between the two colors
 */
extern int  pal_get_hash(RGBcolor *one, RGBcolor *two);

/**
 * Sorts a shadow color list by intensity.
 */
extern void pal_shadow_sort(ShadowListPtr shadow, ColorListPtr list);

/**
 * Local routine to find the colors in a list for which shadow mapping is requested.
 */
extern void pal_init_shadow(ShadowListPtr shadow, ColorListPtr new_list);
extern void pal_activate_shadow(ShadowListPtr shadow);
extern int pal_get_flags(void);
extern int pal_get_colors(void);
extern void pal_interface(Palette fixpal);
extern void pal_white(Palette fixpal);
extern void pal_grey(Palette fixpal, int base_color, int num_colors,
	int low_grey, int high_grey);
extern int pal_get_color(RGBcolor color, int color_handle,
	int override_reserved, int *color_number);
extern void pal_change_color(int color, int r, int g, int b);

} // namespace MADSV2
} // namespace MADS

#endif
