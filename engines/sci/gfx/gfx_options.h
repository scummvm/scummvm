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


#ifndef SCI_GFX_GFX_OPTIONS_H
#define SCI_GFX_GFX_OPTIONS_H

#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

// Define this to enable user-defined custom graphics options
#define CUSTOM_GRAPHICS_OPTIONS

#ifdef CUSTOM_GRAPHICS_OPTIONS
#include "sci/gfx/gfx_res_options.h"
#endif

namespace Sci {

/**
 * All user options to the rendering pipeline
 *
 * See note in sci_conf.h for config_entry_t before changing types of
 * variables
 */
struct gfx_options_t {
#ifdef CUSTOM_GRAPHICS_OPTIONS
	/* SCI0 pic resource options */
	int pic0_unscaled; /* Don't draw scaled SCI0 pics */
	DitherMode pic0_dither_mode; /* Mode to use for pic0 dithering, defined in gfx_resource.h */

	gfx_brush_mode_t pic0_brush_mode;
	gfx_line_mode_t pic0_line_mode;

	gfx_xlate_filter_t cursor_xlate_filter;
	gfx_xlate_filter_t view_xlate_filter;
	gfx_xlate_filter_t pic_xlate_filter; /* Only relevant if (pic0_unscaled) */
	gfx_xlate_filter_t text_xlate_filter;
	gfx_res_fullconf_t res_conf; /* Resource customisation: Per-resource palettes etc. */

	int workarounds;	// Workaround flags - see below
#endif
};

#ifdef CUSTOM_GRAPHICS_OPTIONS
/* SQ3 counts whitespaces towards the total text size, as does gfxop_get_text_params() if this is set: */
#define GFX_WORKAROUND_WHITESPACE_COUNT (1 << 0)
#endif

} // End of namespace Sci

#endif // SCI_GFX_GFX_OPTIONS_H
