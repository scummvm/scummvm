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

//#define DEBUG

static int matches_patternlist(gfx_res_pattern_t *patterns, int nr, int val) {
	for (int i = 0; i < nr; i++)
		if (patterns[i].min <= val
		        && patterns[i].max >= val)
			return 1;

	return 0;
}

#ifdef DEBUG
static void print_pattern(gfx_res_pattern_t *pat) {
	fprintf(stderr, "[%d..%d]",
	        pat->min, pat->max);
}
#endif

static int resource_matches_patternlists(gfx_res_conf_t *conf, int type, int nr, int loop, int cel) {
	int loc;
#ifdef DEBUG
	int i;
	fprintf(stderr, "[DEBUG:gfx-res] Trying to match against %d/%d/%d choices\n",
	        conf->patterns_nr, conf->loops_nr, conf->cels_nr);
	for (i = 0; i < conf->patterns_nr; i++) {
		fprintf(stderr, "[DEBUG:gfx-res] Pat #%d: ", i);
		print_pattern(conf->patterns + i);
		fprintf(stderr, "\n");
	}
	loc = conf->patterns_nr;
	for (i = 0; i < conf->loops_nr; i++) {
		fprintf(stderr, "[DEBUG:gfx-res] Loop #%d: ", i);
		print_pattern(conf->patterns + i + loc);
		fprintf(stderr, "\n");
	}
	loc += conf->loops_nr;
	for (i = 0; i < conf->cels_nr; i++) {
		fprintf(stderr, "[DEBUG:gfx-res] Cel #%d: ", i);
		print_pattern(conf->patterns + i + loc);
		fprintf(stderr, "\n");
	}
#endif
	if (conf->patterns_nr && !matches_patternlist(conf->patterns, conf->patterns_nr, nr))
		return 0;

	if (type == GFX_RESOURCE_TYPE_CURSOR)
		return 1;

	/* Otherwise, we must match at least the loop (pic)
	** and, for views, the cel as well  */
	loc = conf->patterns_nr;
	if (conf->loops_nr &&
	        !matches_patternlist(conf->patterns + loc,
	                             conf->loops_nr,
	                             loop))
		return 0;

	if (type != GFX_RESOURCE_TYPE_VIEW)
		return 1;

	loc += conf->loops_nr;

	if (!conf->cels_nr)
		return 1;

	return matches_patternlist(conf->patterns + loc, conf->cels_nr, cel);
}

static gfx_res_conf_t *find_match(gfx_res_conf_t *conflist, int type, int nr, int loop, int cel) {
	while (conflist) {
		if (resource_matches_patternlists(conflist, type, nr, loop, cel)) {
#ifdef DEBUG
			fprintf(stderr, "[DEBUG:gfx-res] Found match!\n");
#endif
			return conflist;
		}

		conflist = conflist->next;
	}

	return NULL;
}

void apply_mod(byte *factor, gfx_pixmap_t *pxm) {
	Palette *pal = pxm->palette;
	int i, pal_size = pal ? pal->size() : 0;

	// Does not have a dynamically allocated palette? Must dup current one
	if (pal && pal->isShared()) {
		pal = pxm->palette->copy();
		pxm->palette->free();
		pxm->palette = pal;
	}

	for (i = 0; i < pal_size; i++) {
		int v;

#define UPDATE_COL(nm, idx)                        \
		v = nm;             \
		v *= factor[idx]; \
		v >>= 4;                   \
		nm = (v > 255)? 255 : v;

		PaletteEntry c = pal->getColor(i);
		UPDATE_COL(c.r, 0);
		UPDATE_COL(c.g, 1);
		UPDATE_COL(c.b, 2);
		pal->setColor(i, c.r, c.g, c.b);

#undef UPDATE_COL
	}
}

int gfx_get_res_config(gfx_options_t *options, gfx_pixmap_t *pxm) {
	int restype = GFXR_RES_TYPE(pxm->ID);
	int nr = GFXR_RES_NR(pxm->ID);
	int loop = pxm->loop;
	int cel = pxm->cel;

	gfx_res_conf_t *conf;

#ifdef DEBUG
	fprintf(stderr, "[DEBUG:gfx-res] Trying to conf %d/%d/%d/%d (ID=%d)\n",
	        restype, nr, loop, cel, pxm->ID);
#endif

	if (pxm->ID < 0 || restype < 0 || restype >= GFX_RESOURCE_TYPES_NR)
		return 1; // Not appropriate

	conf = find_match(options->res_conf.assign[restype], restype, nr, loop, cel);

	if (conf) {
		// Assign palette
		if (pxm->palette)
			pxm->palette->free();

		pxm->palette = new Palette(conf->conf.assign.assign.palette.colors,
								   conf->conf.assign.assign.palette.colors_nr);
		pxm->palette->name = "res";
	}

	conf = options->res_conf.mod[restype];
	while (conf) {
		conf = find_match(conf, restype, nr, loop, cel);
		if (conf) {
			apply_mod(conf->conf.factor, pxm);
			conf = conf->next;
		}
	}
	fflush(NULL);

	return 0;
}

} // End of namespace Sci

#endif
