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

/* Configuration options for per-resource customisations */

#ifndef _GFX_RES_OPTIONS_H_
#define _GFX_RES_OPTIONS_H_

#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_resmgr.h"

namespace Sci {

struct gfx_res_pattern_t {
	int min, max;
};

/* GFX resource assignments */

struct gfx_res_assign_t {
	short type; /* GFX_RES_ASSIGN_TYPE_* */

	union {
		struct {
			int colors_nr;
			gfx_pixmap_color_t *colors;
		} palette;
	} assign;
};


/* GFX resource modifications */

#define GFX_RES_MULTIPLY_FIXED 0 /* Linear palette update */

struct gfx_res_mod_t {
	short type; /* GFX_RES_ASSIGN_TYPE_* */

	union {
		byte factor[3]; /* divide by 16 to retrieve factor */
	} mod;
};


struct gfx_res_conf_t {
	int type; /* Resource type-- only one allowed */

	/* If any of the following is 0, it means that there is no restriction.
	** Otherwise, one of the patterns associated with them must match. */
	int patterns_nr; /* Number of patterns (only 'view' patterns for views) */
	int loops_nr, cels_nr; /* Number of loop/cel patterns, for views only.
			       ** For pics, loops_nr identifies the palette. */

	gfx_res_pattern_t *patterns;

	union {
		gfx_res_assign_t assign;
		gfx_res_mod_t mod;
	} conf; /* The actual configuration */

	gfx_res_conf_t *next;
};


typedef gfx_res_conf_t *gfx_res_conf_p_t;

struct gfx_res_fullconf_t {
	gfx_res_conf_p_t assign[GFX_RESOURCE_TYPES_NR];
	gfx_res_conf_p_t mod[GFX_RESOURCE_TYPES_NR];
};


struct gfx_options_t;

int gfx_get_res_config(gfx_options_t *options, gfx_pixmap_t *pxm);
/* Configures a graphical pixmap according to config options
** Parameters: (gfx_options_t *) options: The options according to which
**                                        configuration should be performed
**             (gfx_resource_type_t) pxm: The pixmap to configure
** Returns   : (int) 0 on success, non-zero otherwise
** Modifies pxm as considered appropriate by configuration options. Does
** not do anything in colour index mode.
*/

} // End of namespace Sci

#endif /* !_GFX_RES_OPTIONS_H_ */
