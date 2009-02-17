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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#ifndef _GFX_OPTIONS_H_
#define _GFX_OPTIONS_H_

#include "sci/include/gfx_resource.h"
#include "sci/include/gfx_tools.h"
#include "sci/include/gfx_res_options.h"

/* Dirty rectangle heuristics: */

/* One: Redraw one rectangle surrounding the dirty area (insert is O(1)) */
#define GFXOP_DIRTY_FRAMES_ONE 1

/* Clusters: Accumulate dirty rects, merging those that overlap (insert is O(n))  */
#define GFXOP_DIRTY_FRAMES_CLUSTERS 2


typedef struct _gfx_options {
	/* gfx_options_t: Contains all user options to the rendering pipeline */
	/* See note in sci_conf.h for config_entry_t before changing types of
	** variables */

	int buffer_pics_nr; /* Number of unused pics to buffer */

	int correct_rendering; /* Whether to render slow, but correct (rather than
				** fast and almost correct) */

	/* SCI0 pic resource options */
	int pic0_unscaled; /* Don't draw scaled SCI0 pics */

	int pic0_dither_mode; /* Defined in gfx_resource.h */
	int pic0_dither_pattern; /* Defined in gfx_resource.h */

	gfx_brush_mode_t pic0_brush_mode;
	gfx_line_mode_t pic0_line_mode;

	gfx_xlate_filter_t cursor_xlate_filter;
	gfx_xlate_filter_t view_xlate_filter;
	gfx_xlate_filter_t pic_xlate_filter; /* Only relevant if (pic0_unscaled) */
	gfx_xlate_filter_t text_xlate_filter;
	gfxr_font_scale_filter_t fixed_font_xlate_filter; /* Scale filter for systems that provide font support which isn't scaled */

	gfxr_antialiasing_t pic0_antialiasing;

	gfx_res_fullconf_t res_conf; /* Resource customisation: Per-resource palettes etc. */

	int dirty_frames;

	int workarounds; /* Workaround flags- see below */

	rect_t pic_port_bounds;
} gfx_options_t;

/* SQ3 counts whitespaces towards the total text size, as does gfxop_get_text_params() if this is set: */
#define GFX_WORKAROUND_WHITESPACE_COUNT (1<<0)


#endif /* !_GFX_OPTIONS_H_ */

