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

#include "sci/include/sci_memory.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

rect_t gfx_rect_fullscreen = {0, 0, 320, 200};

void gfx_clip_box_basic(rect_t *box, int maxx, int maxy) {
	if (box->x < 0)
		box->x = 0;

	if (box->y < 0)
		box->y = 0;

	if (box->x + box->xl > maxx)
		box->xl = maxx - box->x + 1;

	if (box->y + box->yl > maxy)
		box->yl = maxy - box->y + 1;
}

gfx_mode_t *gfx_new_mode(int xfact, int yfact, int bytespp, unsigned int red_mask, unsigned int green_mask,
	unsigned int blue_mask, unsigned int alpha_mask, int red_shift, int green_shift,
	int blue_shift, int alpha_shift, int palette, int flags) {
	gfx_mode_t *mode = (gfx_mode_t *)sci_malloc(sizeof(gfx_mode_t));

	mode->xfact = xfact;
	mode->yfact = yfact;
	mode->bytespp = bytespp;
	mode->red_mask = red_mask;
	mode->green_mask = green_mask;
	mode->blue_mask = blue_mask;
	mode->alpha_mask = alpha_mask;
	mode->red_shift = red_shift;
	mode->green_shift = green_shift;
	mode->blue_shift = blue_shift;
	mode->alpha_shift = alpha_shift;
	mode->flags = flags;

	if (palette) {
		mode->palette = (gfx_palette_t *)sci_malloc(sizeof(gfx_palette_t));
		mode->palette->max_colors_nr = palette;
		mode->palette->colors = (gfx_palette_color_t *)sci_calloc(sizeof(gfx_palette_color_t), palette); // Initialize with empty entries
	} else
		mode->palette = NULL;

	return mode;
}

void gfx_free_mode(gfx_mode_t *mode) {
	if (mode->palette) {
		free(mode->palette->colors);
		free(mode->palette);
	}
	free(mode);
}

void gfx_copy_pixmap_box_i(gfx_pixmap_t *dest, gfx_pixmap_t *src, rect_t box) {
	int width, height;
	int offset;

	if ((dest->index_xl != src->index_xl) || (dest->index_yl != src->index_yl))
		return;

	gfx_clip_box_basic(&box, dest->index_xl, dest->index_yl);

	if (box.xl <= 0 || box.yl <= 0)
		return;

	height = box.yl;
	width = box.xl;

	offset = box.x + (box.y * dest->index_xl);

	while (height--) {
		memcpy(dest->index_data + offset, src->index_data + offset, width);
		offset += dest->index_xl;
	}
}

gfx_pixmap_t *gfx_clone_pixmap(gfx_pixmap_t *pxm, gfx_mode_t *mode) {
	gfx_pixmap_t *clone = (gfx_pixmap_t *)sci_malloc(sizeof(gfx_pixmap_t));
	*clone = *pxm;
	clone->index_data = NULL;
	clone->colors = NULL;
	clone->data = NULL;
	gfx_pixmap_alloc_data(clone, mode);

	memcpy(clone->data, pxm->data, clone->data_size);
	if (clone->alpha_map) {
		clone->alpha_map = (byte *) sci_malloc(clone->xl * clone->yl);
		memcpy(clone->alpha_map, pxm->alpha_map, clone->xl * clone->yl);
	}

	return clone;
}

gfx_pixmap_t *gfx_new_pixmap(int xl, int yl, int resid, int loop, int cel) {
	gfx_pixmap_t *pxm = (gfx_pixmap_t *)sci_malloc(sizeof(gfx_pixmap_t));

	pxm->alpha_map = NULL;
	pxm->data = NULL;
	pxm->internal.info = NULL;
	pxm->colors = NULL;
	pxm->internal.handle = 0;

	pxm->index_xl = xl;
	pxm->index_yl = yl;

	pxm->ID = resid;
	pxm->loop = loop;
	pxm->cel = cel;

	pxm->index_data = NULL;

	pxm->flags = 0;

	pxm->color_key = 0xff;

	return pxm;
}

void gfx_free_pixmap(gfx_driver_t *driver, gfx_pixmap_t *pxm) {
	if (driver) {
		if (driver->mode->palette && pxm->flags & GFX_PIXMAP_FLAG_PALETTE_ALLOCATED
		        && !(pxm->flags & GFX_PIXMAP_FLAG_DONT_UNALLOCATE_PALETTE) && !(pxm->flags & GFX_PIXMAP_FLAG_EXTERNAL_PALETTE)) {
			int i;
			int error = 0;

			GFXDEBUG("UNALLOCATING %d\n", pxm->colors_nr);
			for (i = 0; i < pxm->colors_nr; i++)
				if (gfx_free_color(driver->mode->palette, pxm->colors + i))
					error++;

			if (error) {
				GFXWARN("%d errors occured while freeing %d colors of pixmap with ID %06x/%d/%d\n",
				        error, pxm->colors_nr, pxm->ID, pxm->loop, pxm->cel);
			}
		}
	}

	if (pxm->index_data)
		free(pxm->index_data);

	if (pxm->alpha_map)
		free(pxm->alpha_map);

	if (pxm->data)
		free(pxm->data);

	if (pxm->colors && !(pxm->flags & GFX_PIXMAP_FLAG_EXTERNAL_PALETTE))
		free(pxm->colors);

	free(pxm);
}

gfx_pixmap_t *gfx_pixmap_alloc_index_data(gfx_pixmap_t *pixmap) {
	int size;

	if (pixmap->index_data) {
		GFXWARN("Attempt to allocate pixmap index data twice!\n");
		return pixmap;
	}

	size = pixmap->index_xl * pixmap->index_yl;
	if (!size)
		size = 1;

	pixmap->index_data = (byte*)sci_malloc(size);

	memset(pixmap->index_data, 0, size);

	return pixmap;
}

gfx_pixmap_t *gfx_pixmap_free_index_data(gfx_pixmap_t *pixmap) {
	if (!pixmap->index_data) {
		GFXWARN("Attempt to free pixmap index data twice!\n");
		return pixmap;
	}

	free(pixmap->index_data);
	pixmap->index_data = NULL;
	return pixmap;
}

gfx_pixmap_t *gfx_pixmap_alloc_data(gfx_pixmap_t *pixmap, gfx_mode_t *mode) {
	int size;

	if (pixmap->data) {
		GFXWARN("Attempt to allocate pixmap data twice!\n");
		return pixmap;
	}

	if (pixmap->flags & GFX_PIXMAP_FLAG_SCALED_INDEX) {
		pixmap->xl = pixmap->index_xl;
		pixmap->yl = pixmap->index_yl;
	} else {
		pixmap->xl = pixmap->index_xl * mode->xfact;
		pixmap->yl = pixmap->index_yl * mode->yfact;
	}

	size = pixmap->xl * pixmap->yl * mode->bytespp;
	if (!size)
		size = 1;

	pixmap->data = (byte*)sci_malloc(pixmap->data_size = size);
	return pixmap;
}

gfx_pixmap_t *gfx_pixmap_free_data(gfx_pixmap_t *pixmap) {
	if (!pixmap->data) {
		GFXWARN("Attempt to free pixmap data twice!\n");
		return pixmap;
	}

	free(pixmap->data);
	pixmap->data = NULL;

	return pixmap;
}

int gfx_alloc_color(gfx_palette_t *pal, gfx_pixmap_color_t *color) {
	int i;
	int dr, dg, db;
	int bestdelta = 1 + ((0x100 * 0x100) * 3);
	int bestcolor = -1;
	int firstfree = -1;

	if (pal == NULL)
		return GFX_OK;

	if (pal->max_colors_nr <= 0) {
		GFXERROR("Palette has zero or less color entries!\n");
		return GFX_ERROR;
	}


	if (color->global_index != GFX_COLOR_INDEX_UNMAPPED) {
#if 0
		GFXDEBUG("Attempt to allocate color twice: index 0x%d (%02x/%02x/%02x)!\n",
		         color->global_index, color->r, color->g, color->b);
#endif
		return GFX_OK;
	}

	for (i = 0; i < pal->max_colors_nr; i++) {
		gfx_palette_color_t *pal_color = pal->colors + i;

		if (pal_color->lockers) {
			int delta;

			dr = abs(pal_color->r - color->r);
			dg = abs(pal_color->g - color->g);
			db = abs(pal_color->b - color->b);

			if (dr == 0 && dg == 0 && db == 0) {
				color->global_index = i;
				return GFX_OK;
			}

			delta = (dr * dr) + (dg * dg) + (db * db);
			if (delta < bestdelta) {
				bestdelta = delta;
				bestcolor = i;
			}
		} else
			if (firstfree == -1)
				firstfree = i;
	}

	if (firstfree != -1) {
		pal->colors[firstfree].r = color->r;
		pal->colors[firstfree].g = color->g;
		pal->colors[firstfree].b = color->b;
		pal->colors[firstfree].lockers = 1;
		color->global_index = firstfree;

		return 42; // positive value to indicate that this color still needs to be set
	}

	color->global_index = bestcolor;

	//GFXWARN("Out of palette colors- doing approximated mapping");
	return GFX_OK;
}

int gfx_free_color(gfx_palette_t *pal, gfx_pixmap_color_t *color) {
	gfx_palette_color_t *palette_color = pal->colors + color->global_index;

	if (!pal)
		return GFX_OK;

	if (color->global_index == GFX_COLOR_INDEX_UNMAPPED) {
		GFXWARN("Attempt to free unmapped color %02x/%02x/%02x!\n", color->r, color->g, color->b);
		BREAKPOINT();
		return GFX_ERROR;
	}

	if (color->global_index >= pal->max_colors_nr) {
		GFXERROR("Attempt to free invalid color index %d (%02x/%02x/%02x)\n", color->global_index, color->r, color->g, color->b);
		return GFX_ERROR;
	}

	if (!palette_color->lockers) {
		GFXERROR("Attempt to free unused color index %d (%02x/%02x/%02x)\n", color->global_index, color->r, color->g, color->b);
		return GFX_ERROR;
	}

	if (palette_color->lockers != GFX_COLOR_SYSTEM)
		--(palette_color->lockers);

	color->global_index = GFX_COLOR_INDEX_UNMAPPED;

	return GFX_OK;
}

gfx_pixmap_t *gfx_pixmap_scale_index_data(gfx_pixmap_t *pixmap, gfx_mode_t *mode) {
	byte *old_data, *new_data, *initial_new_data;
	byte *linestart;
	int linewidth;
	int xl, yl;
	int i, yc;
	int xfact = mode->xfact;
	int yfact = mode->yfact;

	if (xfact == 1 && yfact == 1)
		return pixmap;

	if (!pixmap)
		return NULL;

	if (pixmap->flags & GFX_PIXMAP_FLAG_SCALED_INDEX)
		return pixmap; // Already done

	old_data = pixmap->index_data;

	if (!old_data) {
		GFXERROR("Attempt to scale index data without index data!\n");
		return pixmap;
	}

	xl = pixmap->index_xl;
	yl = pixmap->index_yl;
	linewidth = xfact * xl;
	initial_new_data = new_data = (byte *)sci_malloc(linewidth * yfact * yl);

	for (yc = 0; yc < yl; yc++) {

		linestart = new_data;

		if (xfact == 1) {
			memcpy(new_data, old_data, linewidth);
			new_data += linewidth;
			old_data += linewidth;
		} else for (i = 0; i < xl; i++) {
				byte fillc = *old_data++;
				memset(new_data, fillc, xfact);
				new_data += xfact;
			}

		for (i = 1; i < yfact; i++) {
			memcpy(new_data, linestart, linewidth);
			new_data += linewidth;
		}
	}

	free(pixmap->index_data);
	pixmap->index_data = initial_new_data;

	pixmap->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;

	pixmap->index_xl = linewidth;
	pixmap->index_yl *= yfact;

	return pixmap;
}

} // End of namespace Sci
