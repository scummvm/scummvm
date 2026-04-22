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

#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/color.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/sort.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/magic.h"

namespace MADS {
namespace MADSV2 {


extern int kidney;

Palette master_palette;             /* Main global palette for program   */
ShadowListPtr master_shadow = NULL; /* Main global shadowing color list  */

dword color_status[256];        /* Mapping status of each palette color  */
int   flag_used[PAL_MAXFLAGS];  /* Mapping status of each palette handle */

int palette_locked = false;
int palette_ever_initialized = false;

int palette_reserved_bottom;
int palette_reserved_top;

int palette_low_search_limit;
int palette_high_search_limit;

void (*pal_manager_update)() = NULL;
int pal_manager_active = false;
int pal_manager_colors = 0;


static void pal_exec(void (*target)(), int manager_mode) {
	if (target == NULL)
		return;

	pal_manager_active = manager_mode;
	target();
	pal_manager_active = false;
}

void pal_init(int reserve_bottom, int reserve_top) {
	int count;

	palette_low_search_limit = 0;
	palette_high_search_limit = 256;

	for (count = 0; count < 256; count++) {
		color_status[count] = 0;
	}

	pal_interface(master_palette);

	if (!palette_ever_initialized) {
		pal_interface(master_palette);
		palette_ever_initialized = true;
	}

	if (reserve_bottom > 0) {
		for (count = 0; count < reserve_bottom; count++) {
			color_status[count] = PAL_RESERVED;
		}
	}

	if (reserve_top > 0) {
		for (count = 0; count < reserve_top; count++) {
			color_status[255 - count] = PAL_RESERVED;
		}
	}

	for (count = 0; count < PAL_MAXFLAGS; count++) {
		flag_used[count] = false;
	}

	flag_used[0] = true;  // Flag[0] is RESERVED status
	flag_used[1] = true;  // Flag[1] is CYCLE status

	palette_locked = false;

	palette_reserved_bottom = reserve_bottom;
	palette_reserved_top = reserve_top;

	pal_manager_colors = 0;
	pal_exec(pal_manager_update, 1);
}

void pal_lock() {
	int count;

	if ((flag_used[PAL_MAXFLAGS - 1]) && !palette_locked) {
		error_report(ERROR_NO_MORE_PALETTE_FLAGS, ERROR, MODULE_PAL, PAL_MAXFLAGS, 66);
	}

	palette_locked = true;

	flag_used[PAL_MAXFLAGS - 1] = true;

	for (count = 0; count < 256; count++) {
		if (color_status[count]) color_status[count] |= 0x80000000L;
	}
}

void pal_unlock() {
	int count;

	if (palette_locked) {
		for (count = 0; count < 256; count++) {
			color_status[count] &= 0x7fffffffL;
		}
		flag_used[PAL_MAXFLAGS - 1] = false;
		palette_locked = false;
	}
}

int pal_deallocate(int use_flag) {
	int return_code = PAL_ERR_BADFLAG;
	int count;
	dword mask;

	if (!kidney) {
		if ((use_flag >= PAL_MAXFLAGS) || (use_flag <= 0)) goto done;

		// Get a mask of everything but our special bit
		mask = ~(1L << (dword)use_flag);

		for (count = 0; count < 256; count++) {
			color_status[count] &= mask;
			if (color_status[count] == PAL_CYCLE) {
				color_status[count] = 0;
			}
		}
	}

	if (!flag_used[use_flag]) {
		return_code = PAL_ERR_FLAGNOTUSED;
		goto done;
	}
	flag_used[use_flag] = false;

	return_code = false;

	pal_exec(pal_manager_update, 4);

done:
	return false;
}

void pal_compact(word master_handle, int num_slaves, word *slave) {
	dword mask_test;
	dword mask_out;
	dword mask_in;
	int count;

	mask_out = 0xffffffffL;
	mask_test = 0L;
	for (count = 0; count < num_slaves; count++) {
		mask_out ^= (1L << (dword)(slave[count]));
		mask_test |= (1L << (dword)(slave[count]));
		flag_used[slave[count]] = false;
	}

	mask_in = (1L << (dword)master_handle);

	for (count = 0; count < 256; count++) {
		if (color_status[count] & mask_test) {
			color_status[count] &= mask_out;
			color_status[count] |= mask_in;
		}
	}

	flag_used[master_handle] = true;
}

int pal_get_hash(RGBcolor *one, RGBcolor *two) {
	int dx = 0;
	signed char diff;

	diff = one->r - two->r;
	dx += diff * diff;

	diff = one->g - two->g;
	dx += diff * diff;

	diff = one->b - two->b;
	dx += diff * diff;

	return dx;
}

void pal_shadow_sort(ShadowListPtr shadow, ColorListPtr list) {
	int count;
	long shadow_hash[COLOR_MAX_SHADOW_COLORS];

	for (count = 0; count < shadow->num_shadow_colors; count++) {
		shadow_hash[count] = magic_hash_color((RGBcolor *) & list->table[shadow->shadow_color[count]]);
	}

	sort_insertion(shadow->num_shadow_colors, &shadow->shadow_color[0], &shadow_hash[0]);
}

void pal_init_shadow(ShadowListPtr shadow, ColorListPtr new_list) {
	int count;

	shadow->num_shadow_colors = 0;
	for (count = 0; count < new_list->num_colors; count++) {
		if (new_list->table[count].group & COLOR_GROUP_MAP_TO_SHADOW) {
			if (shadow->num_shadow_colors < COLOR_MAX_SHADOW_COLORS) {
				shadow->shadow_color[shadow->num_shadow_colors] = count;
				shadow->num_shadow_colors++;
			}
		}
	}
}

void pal_activate_shadow(ShadowListPtr shadow) {
	master_shadow = shadow;
}

static int pal_get_new_flag() {
	int return_code;
	int count;

	// First, we need to find an available color handle for the new list
	return_code = PAL_ERR_OUTOFFLAGS;

	for (count = 0; (count < PAL_MAXFLAGS); count++) {
		if (!flag_used[count]) {
			return_code = count;
			goto done;
		}
	}

	// If no handles left to allocate:
	if (return_code < 0) {
#ifndef disable_error_check
		error_report(ERROR_NO_MORE_PALETTE_FLAGS, ERROR, MODULE_PAL, PAL_MAXFLAGS, 100);
#endif
	}

done:
	return (return_code);
}


static int pal_free_colors(int *first_free) {
	int free_colors, count;

	*first_free = -1;

	free_colors = 0;
	for (count = 0; count < 256; count++) {
		if (!color_status[count]) {
			free_colors++;
			if (*first_free < 0) {
				*first_free = count;
			}
		}
	}

	if (*first_free < 0) first_free = 0;

	return free_colors;
}

int pal_allocate(ColorListPtr new_list, ShadowListPtr shadow_list, int pal_flags) {
	int target_color, list_color;
	int return_code;
	int found;
	int count;
	int best_target_color;
	int search_color;
	int target_shadow;
	int shadow;
	int shadowing_enabled;
	int shadowing_special;
	int defining_background;
	int conduct_search, conduct_insert;
	int free_colors;
	int hash, best_hash;
	int palette_limit;
	int first_free;
	int search_start;
	int search_stop;
	dword mask;
	dword reserved_mask;
	dword cycle_mask;
	dword bonus;
	ShadowList incoming_shadow;
	byte reordering_hash[256];
	byte reordering_index[256];

	incoming_shadow.num_shadow_colors = 0;
	palette_limit = (pal_flags & PAL_MAP_TOP_COLORS) ? 256 : 252;

	if (pal_flags & PAL_MAP_RESERVED) {
		search_start = 0;
		search_stop = palette_limit;
	} else {
		search_start = palette_reserved_bottom;
		search_stop = MIN(256 - palette_reserved_top, palette_limit);
	}

	search_start = MAX(search_start, palette_low_search_limit);
	search_stop = MIN(search_stop, palette_high_search_limit);

	// Get a new color handle
	return_code = pal_get_new_flag();
	if (return_code < 0) goto done;

	// Set up the proper masking bit for our chosen color handle
	mask = 1L << (dword)return_code;

	// Check if we are defining a new background picture
	defining_background = pal_flags & PAL_MAP_BACKGROUND;

	// If a shadowing description was passed, enable shadow checking
	shadowing_enabled = (shadow_list != NULL);
	shadowing_special = false;
	if (shadowing_enabled) {
		if (defining_background || (shadow_list->num_shadow_colors == 0)) {
			shadowing_enabled = false;
		}
		if (defining_background && shadow_list->num_shadow_colors) {
			shadowing_special = true;
		}
	}

	// If we are mapping shadow colors, we need to make a list of all
	// shadowable colors in our color list and then sort it by intensity.
	if (shadowing_enabled) {
		pal_init_shadow(&incoming_shadow, new_list);
		pal_shadow_sort(&incoming_shadow, new_list);
	}

	// Get a count of totally free colors in the palette
	free_colors = pal_free_colors(&first_free);

	first_free = MAX(first_free, search_start);

	// Sort the color list so that all cycle colors remain at the beginning
	// of the list but that all colors which are allowed to use inexact RGB
	// mappings are placed at the bottom of the list.  That way, colors which
	// require an exact RGB mapping are given the first chance to allocate
	// free color space.
	for (count = 0; count < new_list->num_colors; count++) {
		reordering_index[count] = (byte)count;
		reordering_hash[count] = 0;
		if (!(new_list->table[count].group & COLOR_GROUP_MAP_TO_CYCLE)) {
			reordering_hash[count] |= 0x40;
		}
		if ((new_list->table[count].group & (COLOR_GROUP_MAP_TO_CLOSEST | COLOR_GROUP_FORCE_TO_CLOSEST))) {
			reordering_hash[count] |= 0x20;
		}
	}
	sort_insertion_8(new_list->num_colors, reordering_index, reordering_hash);

	if (pal_flags & PAL_MAP_RESERVED) {
		reserved_mask = 0xffffffff;
	} else {
		reserved_mask = 0xfffffffe;
	}

	// Now, for each color in our color list, find an appropriate mapping or
	// create a new one from available color space.
	for (search_color = 0; search_color < new_list->num_colors; search_color++) {

		list_color = reordering_index[search_color];

		found = false;
		best_target_color = -1;

		// Don't insert colors that are being forced to skip codes
		if (new_list->table[list_color].group & COLOR_GROUP_FORCE_TO_SKIP) {
			found = true;
			best_target_color = SS_SKIP;
		}

		// If we are doing shadowing, check to see if our color is one of the
		// shadow colors.  If so, just map it right to the corresponding shadow
		// color in the master palette, without regard for RGB match.
		if (shadowing_enabled) {
			if (new_list->table[list_color].group & COLOR_GROUP_MAP_TO_SHADOW) {
				for (shadow = 0; !found && (shadow < incoming_shadow.num_shadow_colors); shadow++) {
					if (list_color == incoming_shadow.shadow_color[shadow]) {
						found = true;
						target_shadow = MIN(shadow_list->num_shadow_colors - 1, shadow);
						best_target_color = shadow_list->shadow_color[target_shadow];
					}
				}
			}
		}

		if (shadowing_special) {
			if (new_list->table[list_color].group & COLOR_GROUP_MAP_TO_SHADOW) {
				for (shadow = 0; !found && (shadow < master_shadow->num_shadow_colors); shadow++) {
					if (list_color == master_shadow->shadow_color[shadow]) {
						found = true;
						best_target_color = shadow + PAL_FORCE_SHADOW;
						memcpy(&(master_palette[best_target_color].r), &(new_list->table[list_color].r), 3);
					}
				}
			}
		}

		if (new_list->table[list_color].group & COLOR_GROUP_MAP_TO_CYCLE) {
			cycle_mask = 0;
		} else {
			cycle_mask = PAL_CYCLE;
		}

		// Now decide if we should search the existing palette for an already
		// existing mapping.  The only reasons not to are  A) if we have
		// found a (shadowing) match; and  B) if we are defining an initial
		// background and therefore should not have any matches.
		conduct_search = !found && !defining_background && cycle_mask != 0;

		if (conduct_search) {
			// Now, decide whether we need an exact match or are willing to just
			// take the closest.
			if ((new_list->table[list_color].group & COLOR_GROUP_FORCE_TO_CLOSEST) ||
				(((pal_flags & PAL_MAP_ANY_TO_CLOSEST) || (new_list->table[list_color].group & COLOR_GROUP_MAP_TO_CLOSEST)) &&
					((pal_flags & PAL_MAP_ALL_TO_CLOSEST) || (!free_colors)))
				) {
				best_hash = 0x7fff;
			} else {
				best_hash = 1;
			}


			// Search through the existing palette for an appropriate color
			for (target_color = search_start; target_color < search_stop; target_color++) {

				if (color_status[target_color]) {
					if ((!(color_status[target_color] & PAL_RESERVED)) || (pal_flags & PAL_MAP_RESERVED)) {
						if (!(color_status[target_color] & cycle_mask)) {
							if (best_hash > 1) {
								hash = pal_get_hash((RGBcolor *) & new_list->table[list_color], &master_palette[target_color]);
							} else {
								// This is a little hack (or "optimization") to compare the 3 RGB bytes much
								// more quickly when we are looking for an exact match only.
								hash = ((*(word *) & new_list->table[list_color] == *(word *) & master_palette[target_color]) &&
									(*(((byte *) & new_list->table[list_color]) + 2) == *(((byte *) & master_palette[target_color]) + 2))) ? 0 : 1;
							}
							if (hash < best_hash) {
								found = true;
								best_target_color = target_color;
								best_hash = hash;
							}
						}
					}
				}
			}
		}

		// Now, decide if we should insert a new color into the palette.  We
		// need to if we have not yet found a match, but we must also check
		// to see if we are allowed to create a new color for this color list
		// item (we are not if the item is being forced to map to closest).
		conduct_insert = (!found) &&
			(!((pal_flags & PAL_MAP_ALL_TO_CLOSEST) &&
				((new_list->table[list_color].group & (COLOR_GROUP_MAP_TO_CLOSEST | COLOR_GROUP_FORCE_TO_CLOSEST)) ||
					(pal_flags & PAL_MAP_ANY_TO_CLOSEST))
				));

		if (conduct_insert) {
			for (target_color = first_free; (!found) && (target_color < search_stop); target_color++) {
				if (!color_status[target_color]) {
					free_colors--;
					first_free++;
					found = true;
					best_target_color = target_color;
					*(RGBcolor *) &master_palette[target_color].r = *(RGBcolor *) & new_list->table[list_color].r;
					// memcpy(&(master_palette[target_color].r), &(new_list->table[list_color].r), 3);
				}
			}
		}

		// If we found a mapping for this color, flag our handle for it and
		// make a note of the list-to-palette mapping for this color in the
		// "x16" slot of the color list item.
		if (found) {
			bonus = (defining_background && (new_list->table[list_color].cycle != 0)) ? PAL_CYCLE : 0;
			color_status[best_target_color] |= (mask | bonus);
			new_list->table[list_color].x16 = (byte)best_target_color;
		} else {
			pal_manager_colors = new_list->num_colors;
			pal_exec(pal_manager_update, 3);
#ifndef disable_error_check
			error_report(ERROR_NO_MORE_COLORS, ERROR, MODULE_PAL, new_list->num_colors, search_color);
#endif
			return_code = PAL_ERR_OUTOFCOLORS;
			goto done;
		}
	}

	flag_used[return_code] = true;

	pal_manager_colors = new_list->num_colors;
	pal_exec(pal_manager_update, 2);

done:
#ifdef palette_dumps
	pal_dump();
#endif
	return return_code;
}

/**
 * Returns number of available flags - checksum use
 */
int pal_get_flags() {
	int a, out;

	out = 0;
	for (a = 0; a < PAL_MAXFLAGS; a++)
		if (!flag_used[a])
			out++;
	return(out);
}

/*
 * Returns number of colors available in palette
 */
int pal_get_colors() {
	int a, out;

	out = 0;
	for (a = 0; a < 256; a++)
		if (color_status[a] == 0)
			out++;
	return(out);
}

void pal_interface(Palette fixpal) {
	int intensity, red, green, blue;
	int base, newCol;
	int color;

	for (intensity = 0; intensity < 2; intensity++) {
		base = intensity * 21;
		newCol = intensity ? 63 : 42;
		for (red = 0; red < 2; red++) {
			for (green = 0; green < 2; green++) {
				for (blue = 0; blue < 2; blue++) {
					color = (intensity << 3) + (red << 2) + (green << 1) + blue;
					fixpal[color].r = (byte)(red ? newCol : base);
					fixpal[color].g = (byte)(green ? newCol : base);
					fixpal[color].b = (byte)(blue ? newCol : base);
				}
			}
		}
	}

	fixpal[6].g = 21;
}



void pal_white(Palette fixpal) {
	int count;
	byte num[4] = { 0, 21, 42, 63 };

	for (count = 0; count < 4; count++) {
		fixpal[count].r = num[count];
		fixpal[count].g = num[count];
		fixpal[count].b = num[count];
	}
}


void pal_grey(Palette fixpal, int base_color, int num_colors,
	int low_grey, int high_grey) {
	int count;
	int dif;
	int level;
	word accum = 0;

	dif = (high_grey - low_grey);
	level = low_grey;

	for (count = 0; count < num_colors; count++) {
		fixpal[base_color + count].r = (byte)level;
		fixpal[base_color + count].g = (byte)level;
		fixpal[base_color + count].b = (byte)level;
		if (num_colors > 1) {
			accum += dif;
			while (accum >= (word)(num_colors - 1)) {
				accum -= (num_colors - 1);
				level++;
			}
		}
	}
}


int pal_get_color(RGBcolor color, int color_handle, int override_reserved, int *color_number) {
	int count;
	int result;
	int found;
	dword mask;

	if (color_handle < 0) {
		result = PAL_ERR_OUTOFFLAGS;  // Default if next loop fails

		for (count = 0; count < PAL_MAXFLAGS; count++) {
			if (!flag_used[count]) {
				result = count;
				break;
			}
		}

		if (result < 0) {
#ifndef disable_error_check
			error_report(ERROR_NO_MORE_PALETTE_FLAGS, ERROR, MODULE_PAL, PAL_MAXFLAGS, 1);
#endif
			return result;  // No flags left to allocate
		}
	} else {
		result = color_handle;
	}

	mask = 1L << (dword)result;

	found = false;
	for (count = 0; (!found) && (count < 256); count++) {
		if (!(color_status[count] & PAL_RESERVED) || override_reserved) {
			if (!(color_status[count] & PAL_CYCLE)) {
				if (memcmp(&color, &master_palette[count].r, sizeof(RGBcolor)) == 0) {
					color_status[count] |= mask;
					if (color_number != NULL) *color_number = count;
					found = true;
				}
			}
		}
	}

	if (!found) {
		for (count = 0; (!found) && (count < 256); count++) {
			if (color_status[count] == 0) {
				memcpy(&master_palette[count].r, &color, sizeof(RGBcolor));
				color_status[count] = mask;
				if (color_number != NULL) *color_number = count;
				found = true;
			}
		}
	}

	if (!found) {
#ifndef disable_error_check
		error_report(ERROR_NO_MORE_COLORS, ERROR, MODULE_PAL, 1, 1);
#endif
		result = PAL_ERR_OUTOFCOLORS;
	}

	return result;
}

void pal_change_color(int color, int r, int g, int b) {
	master_palette[color].r = (byte)r;
	master_palette[color].g = (byte)g;
	master_palette[color].b = (byte)b;
	mcga_setpal_range((Palette *)master_palette, color, 1);
}

} // namespace MADSV2
} // namespace MADS
