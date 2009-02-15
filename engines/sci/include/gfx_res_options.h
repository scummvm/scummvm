/***************************************************************************
 gfx_res_options.h  Copyright (C) 2002 Christoph Reichenbach


 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence as
 published by the Free Software Foundaton; either version 2 of the
 Licence, or (at your option) any later version.

 It is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 merchantibility or fitness for a particular purpose. See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with this program; see the file COPYING. If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.


 Please contact the maintainer for any program-related bug reports or
 inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

/* Configuration options for per-resource customisations */

#ifndef _GFX_RES_OPTIONS_H_
#define _GFX_RES_OPTIONS_H_

#include "sci/include/gfx_resource.h"
#include "sci/include/gfx_resmgr.h"

#define GFX_RES_PATTERN_MIN 0
#define GFX_RES_PATTERN_MAX 65535

typedef struct _gfx_res_pattern {
	int min, max;
} gfx_res_pattern_t;

typedef struct _gfx_res_pattern_list {
	gfx_res_pattern_t pattern;
	struct _gfx_res_pattern_list *next;
} gfx_res_pattern_list_t;


/* GFX resource assignments */

#define GFX_RES_ASSIGN_TYPE_PALETTE 0 /* Assign a palette */

typedef struct {
	short type; /* GFX_RES_ASSIGN_TYPE_* */

	union {
		struct {
			int colors_nr;
			gfx_pixmap_color_t *colors;
		} palette;
	} assign;
} gfx_res_assign_t;


/* GFX resource modifications */

#define GFX_RES_MULTIPLY_FIXED 0 /* Linear palette update */

typedef struct {
	short type; /* GFX_RES_ASSIGN_TYPE_* */

	union {
		byte factor[3]; /* divide by 16 to retrieve factor */
	} mod;
} gfx_res_mod_t;


typedef struct _gfx_res_conf {
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

	struct _gfx_res_conf *next;
} gfx_res_conf_t;


typedef gfx_res_conf_t *gfx_res_conf_p_t;

typedef struct {
	gfx_res_conf_p_t assign[GFX_RESOURCE_TYPES_NR];
	gfx_res_conf_p_t mod[GFX_RESOURCE_TYPES_NR];
} gfx_res_fullconf_t;


struct _gfx_options;

int
gfx_update_conf(struct _gfx_options *options,
                char *line);
/* Updates the configuration
** Parameters: (gfx_options_t *) options: The options list to update
**             (char *) line: The text line to parse
** Modifies  : (gfx_options_t *) options
** Returns   : (int) 0 on success, 1 if an error occured
** The line passed to it should begin with the resource type and be
** terminated by a semicolon.
*/

int
gfx_get_res_config(struct _gfx_options *options,
                   gfx_pixmap_t *pxm);
/* Configures a graphical pixmap according to config options
** Parameters: (gfx_options_t *) options: The options according to which
**                                        configuration should be performed
**             (gfx_resource_type_t) pxm: The pixmap to configure
** Returns   : (int) 0 on success, non-zero otherwise
** Modifies pxm as considered appropriate by configuration options. Does
** not do anything in colour index mode.
*/

#endif /* !_GFX_RES_OPTIONS_H_ */
