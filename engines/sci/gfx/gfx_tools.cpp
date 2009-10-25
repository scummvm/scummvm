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

#include "sci/gfx/gfx_tools.h"

namespace Sci {

rect_t gfx_rect_fullscreen = {0, 0, 320, 200};

void gfx_clip_box_basic(rect_t *box, int maxx, int maxy) {
	if (box->x < 0)
		box->x = 0;

	if (box->y < 0)
		box->y = 0;

	if (box->x + box->width > maxx)
		box->width = maxx - box->x + 1;

	if (box->y + box->height > maxy)
		box->height = maxy - box->y + 1;
}

gfx_mode_t *gfx_new_mode(int scaleFactor, Palette *palette) {
	gfx_mode_t *mode = (gfx_mode_t *)malloc(sizeof(gfx_mode_t));

	mode->scaleFactor = scaleFactor;
	mode->xsize = scaleFactor * 320;
	mode->ysize = scaleFactor * 200;
	mode->palette = palette;

	return mode;
}

void gfx_free_mode(gfx_mode_t *mode) {
	mode->palette->free();
	free(mode);
	mode = NULL;
}

void gfx_copy_pixmap_box_i(gfx_pixmap_t *dest, gfx_pixmap_t *src, rect_t box) {
	int width, height;
	int offset;

	if ((dest->index_width != src->index_width) || (dest->index_height != src->index_height))
		return;

	gfx_clip_box_basic(&box, dest->index_width, dest->index_height);

	if (box.width <= 0 || box.height <= 0)
		return;

	height = box.height;
	width = box.width;

	offset = box.x + (box.y * dest->index_width);

	while (height--) {
		memcpy(dest->index_data + offset, src->index_data + offset, width);
		offset += dest->index_width;
	}
}

gfx_pixmap_t *gfx_new_pixmap(int xl, int yl, int resid, int loop, int cel) {
	gfx_pixmap_t *pxm = (gfx_pixmap_t *)malloc(sizeof(gfx_pixmap_t));

	pxm->alpha_map = NULL;
	pxm->data = NULL;
	pxm->palette = NULL;
	pxm->palette_revision = -1;

	pxm->index_width = xl;
	pxm->index_height = yl;

	pxm->ID = resid;
	pxm->loop = loop;
	pxm->cel = cel;

	pxm->index_data = NULL;

	pxm->flags = 0;

	pxm->color_key = 0xff;

	return pxm;
}

void gfx_free_pixmap(gfx_pixmap_t *pxm) {
	if (pxm->palette)
		pxm->palette->free();

	free(pxm->index_data);
	free(pxm->alpha_map);
	free(pxm->data);
	free(pxm);
}

gfx_pixmap_t *gfx_pixmap_alloc_index_data(gfx_pixmap_t *pixmap) {
	int size;

	if (pixmap->index_data) {
		warning("[GFX] Attempt to allocate pixmap index data twice");
		return pixmap;
	}

	size = pixmap->index_width * pixmap->index_height;
	if (!size)
		size = 1;

	pixmap->index_data = (byte*)malloc(size);

	memset(pixmap->index_data, 0, size);

	return pixmap;
}

gfx_pixmap_t *gfx_pixmap_alloc_data(gfx_pixmap_t *pixmap, gfx_mode_t *mode) {
	int size;

	if (pixmap->data) {
		warning("[GFX] Attempt to allocate pixmap data twice");
		return pixmap;
	}

	if (pixmap->flags & GFX_PIXMAP_FLAG_SCALED_INDEX) {
		pixmap->width = pixmap->index_width;
		pixmap->height = pixmap->index_height;
	} else {
		pixmap->width = pixmap->index_width * mode->scaleFactor;
		pixmap->height = pixmap->index_height * mode->scaleFactor;
	}

	size = pixmap->width * pixmap->height;
	if (!size)
		size = 1;

	pixmap->data = (byte*)malloc(pixmap->data_size = size);
	return pixmap;
}

gfx_pixmap_t *gfx_pixmap_scale_index_data(gfx_pixmap_t *pixmap, gfx_mode_t *mode) {
	byte *old_data, *new_data, *initial_new_data;
	byte *linestart;
	int linewidth;
	int xl, yl;
	int i, yc;
	int xfact = mode->scaleFactor;
	int yfact = mode->scaleFactor;

	if (xfact == 1 && yfact == 1)
		return pixmap;

	if (!pixmap)
		return NULL;

	if (pixmap->flags & GFX_PIXMAP_FLAG_SCALED_INDEX)
		return pixmap; // Already done

	old_data = pixmap->index_data;

	if (!old_data) {
		error("Attempt to scale index data without index data!\n");
		return pixmap;
	}

	xl = pixmap->index_width;
	yl = pixmap->index_height;
	linewidth = xfact * xl;
	initial_new_data = new_data = (byte *)malloc(linewidth * yfact * yl);

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

	pixmap->index_width = linewidth;
	pixmap->index_height *= yfact;

	return pixmap;
}

} // End of namespace Sci
