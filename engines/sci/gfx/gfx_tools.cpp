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

#include "sci/sci_memory.h"
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

gfx_mode_t *gfx_new_mode(int xfact, int yfact, const Graphics::PixelFormat &format, Palette *palette, int flags) {
	gfx_mode_t *mode = (gfx_mode_t *)sci_malloc(sizeof(gfx_mode_t));

	mode->xfact = xfact;
	mode->yfact = yfact;
	mode->bytespp = format.bytesPerPixel;
	
	// FIXME: I am not sure whether the following assignments are quite right.
	// The only code using these are the built-in scalers of the SCI engine.
	// And those are pretty weird, so I am not sure I interpreted them correctly.
	// They also seem somewhat inefficient and and should probably just be
	// replaced resp. rewritten from scratch.
	if (format.bytesPerPixel > 1) {
		mode->red_mask = format.ARGBToColor(0, 0xFF, 0, 0);
		mode->green_mask = format.ARGBToColor(0, 0, 0xFF, 0);
		mode->blue_mask = format.ARGBToColor(0, 0, 0, 0xFF);
		mode->alpha_mask = format.ARGBToColor(0xFF, 0, 0, 0);
		mode->red_shift = format.rLoss;
		mode->green_shift = format.gLoss;
		mode->blue_shift = format.bLoss;
		mode->alpha_shift = format.aLoss;
	} else {
		mode->red_mask = mode->green_mask = mode->blue_mask = 0;
		mode->alpha_mask = 0;
		mode->red_shift = mode->green_shift = mode->blue_shift = 0;
		mode->alpha_shift = 0;
	}
	mode->flags = flags;
	mode->palette = palette;

	return mode;
}

void gfx_free_mode(gfx_mode_t *mode) {
	if (mode->palette)
		mode->palette->free();
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
	clone->palette = NULL;
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
	pxm->palette = NULL;

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
		if (driver->mode->palette) {
			if (pxm->palette)
				pxm->palette->free();
		}
	}

	free(pxm->index_data);
	free(pxm->alpha_map);
	free(pxm->data);
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
