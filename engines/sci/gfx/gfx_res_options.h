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

#ifndef SCI_GFX_GFX_RES_OPTIONS_H
#define SCI_GFX_GFX_RES_OPTIONS_H

#ifdef CUSTOM_GRAPHICS_OPTIONS

#include "sci/sci.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_resmgr.h"

namespace Sci {
/** @name Configuration options for per-resource customisations */
/** @{ */

struct gfx_res_pattern_t {
	int min, max;
};

/**
 * GFX resource modifications/
 */
struct gfx_res_conf_t {
	int type; /**< Resource type-- only one allowed */

	/* If any of the following is 0, it means that there is no restriction.
	** Otherwise, one of the patterns associated with them must match. */
	int patterns_nr; /**< Number of patterns (only 'view' patterns for views) */
	int loops_nr, cels_nr; /**< Number of loop/cel patterns, for views only.
			       ** For pics, loops_nr identifies the palette. */

	gfx_res_pattern_t *patterns;

	int colors_nr;
	PaletteEntry *colors;
	byte rFactor, gFactor, bFactor;

	gfx_res_conf_t *next;
};


struct gfx_res_fullconf_t {
	gfx_res_conf_t *assign[GFX_RESOURCE_TYPES_NR];
	gfx_res_conf_t *mod[GFX_RESOURCE_TYPES_NR];
};


struct gfx_options_t;

/**
 * Configures a graphical pixmap according to config options.
 *
 * Modifies pxm as considered appropriate by configuration options. Does
 * not do anything in colour index mode.
 *
 * @param[in] res_conf	The resource options according to which modifications
 * 						should be performed
 * @param[in] pxm		The pixmap to configure
 * @return				0 on success, non-zero otherwise
 */
int gfx_get_res_config(gfx_res_fullconf_t res_conf, gfx_pixmap_t *pxm);

/** @} */
} // End of namespace Sci

#endif

#endif // SCI_GFX_GFX_RES_OPTIONS_H
