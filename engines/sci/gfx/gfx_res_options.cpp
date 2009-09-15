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

#include "sci/sci.h"
#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_options.h"
#include "sci/gfx/gfx_resmgr.h"

#ifdef CUSTOM_GRAPHICS_OPTIONS

namespace Sci {

static bool matches_patternlist(gfx_res_pattern_t *patterns, int nr, int val) {
	for (int i = 0; i < nr; i++)
		if (patterns[i].min <= val && patterns[i].max >= val)
			return true;

	return false;
}

static bool resource_matches_patternlists(gfx_res_conf_t *conf, int type, int nr, int loop, int cel) {
	int loc;
	if (conf->patterns_nr && !matches_patternlist(conf->patterns, conf->patterns_nr, nr))
		return false;

	if (type == GFX_RESOURCE_TYPE_CURSOR)
		return true;

	/* Otherwise, we must match at least the loop (pic)
	** and, for views, the cel as well  */
	loc = conf->patterns_nr;
	if (conf->loops_nr &&
	        !matches_patternlist(conf->patterns + loc,
	                             conf->loops_nr,
	                             loop))
		return false;

	if (type != GFX_RESOURCE_TYPE_VIEW)
		return true;

	loc += conf->loops_nr;

	if (!conf->cels_nr)
		return true;

	return matches_patternlist(conf->patterns + loc, conf->cels_nr, cel);
}

static gfx_res_conf_t *find_match(gfx_res_conf_t *conflist, int type, int nr, int loop, int cel) {
	while (conflist) {
		if (resource_matches_patternlists(conflist, type, nr, loop, cel))
			return conflist;

		conflist = conflist->next;
	}

	return NULL;
}

void apply_mod(byte rFactor, byte gFactor, byte bFactor, gfx_pixmap_t *pxm) {
	Palette *pal = pxm->palette;
	int i, pal_size = pal ? pal->size() : 0;

	// Id the pixmap's palette is shared, duplicate it
	if (pal && pal->isShared()) {
		pal = pxm->palette->copy();
		pxm->palette->free();
		pxm->palette = pal;
	}

	for (i = 0; i < pal_size; i++) {
		PaletteEntry c = pal->getColor(i);
		c.r = CLIP<int>((c.r * rFactor) >> 4, 0, 255);
		c.g = CLIP<int>((c.g * gFactor) >> 4, 0, 255);
		c.b = CLIP<int>((c.b * bFactor) >> 4, 0, 255);
		pal->setColor(i, c.r, c.g, c.b);
	}
}

int gfx_get_res_config(gfx_res_fullconf_t res_conf, gfx_pixmap_t *pxm) {
	int restype = GFXR_RES_TYPE(pxm->ID);
	int nr = GFXR_RES_NR(pxm->ID);
	int loop = pxm->loop;
	int cel = pxm->cel;

	gfx_res_conf_t *conf;

	if (pxm->ID < 0 || restype < 0 || restype >= GFX_RESOURCE_TYPES_NR)
		return 1; // Not appropriate

	// Find assignment config, if available
	conf = find_match(res_conf.assign[restype], restype, nr, loop, cel);
	if (conf) {
		// Assign palette
		if (pxm->palette)
			pxm->palette->free();

		pxm->palette = new Palette(conf->colors,
								   conf->colors_nr);
		pxm->palette->name = "res";
	}

	// Find mod config, if available
	conf = res_conf.mod[restype];
	while (conf) {
		conf = find_match(conf, restype, nr, loop, cel);
		if (conf) {
			apply_mod(conf->rFactor, conf->gFactor, conf->bFactor, pxm);
			conf = conf->next;
		}
	}

	return 0;
}

} // End of namespace Sci

#endif
