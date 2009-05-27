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

#include "common/scummsys.h"
#include "common/system.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/gfx/gfx_driver.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

struct _scummvm_driver_state {
	gfx_pixmap_t *priority[2];
	byte *visual[2];
	uint8 *pointer_data;
	int xsize, ysize;
};

#define S ((struct _scummvm_driver_state *)(drv->state))

static int scummvm_init(gfx_driver_t *drv, int xfact, int yfact, int bytespp) {
	int i;

	if (!drv->state) // = S
		drv->state = new _scummvm_driver_state;
	if (!drv->state)
		return GFX_FATAL;

	S->xsize = xfact * 320;
	S->ysize = yfact * 200;

	S->pointer_data = NULL;
	//S->buckystate = 0;

	for (i = 0; i < 2; i++) {
		S->priority[i] = gfx_pixmap_alloc_index_data(gfx_new_pixmap(S->xsize, S->ysize, GFX_RESID_NONE, -i, -777));
		if (!S->priority[i]) {
			printf("Out of memory: Could not allocate priority maps! (%dx%d)\n", S->xsize, S->ysize);
			return GFX_FATAL;
		}
	}
	// create the visual buffers
	for (i = 0; i < 2; i++) {
		S->visual[i] = NULL;
		S->visual[i] = new byte[S->xsize * S->ysize];
		if (!S->visual[i]) {
			printf("Out of memory: Could not allocate visual buffers! (%dx%d)\n", S->xsize, S->ysize);
			return GFX_FATAL;
		}
		memset(S->visual[i], 0, S->xsize * S->ysize);
	}

	Graphics::PixelFormat format = { bytespp, 0, 0, 0, 0, 0, 0, 0, 0 };
	drv->mode = gfx_new_mode(xfact, yfact, format, new Palette(256), 0);
	drv->mode->palette->name = "global";

	return GFX_OK;
}

static void scummvm_exit(gfx_driver_t *drv) {
	int i;
	if (S) {
		for (i = 0; i < 2; i++) {
			gfx_free_pixmap(S->priority[i]);
			S->priority[i] = NULL;
		}

		for (i = 0; i < 2; i++) {
			delete[] S->visual[i];
			S->visual[i] = NULL;
		}

		delete[] S->pointer_data;
		S->pointer_data = NULL;

		delete S;
	}
}


// Drawing operations

static void drawProc(int x, int y, int c, void *data) {
	gfx_driver_t *drv = (gfx_driver_t *)data;
	uint8 *p = S->visual[0];
	p[y * 320*drv->mode->xfact + x] = c;
}

static int scummvm_draw_line(gfx_driver_t *drv, Common::Point start, Common::Point end,
	gfx_color_t color, gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	uint32 scolor = color.visual.parent_index;
	int xfact = (line_mode == GFX_LINE_MODE_FINE)? 1: drv->mode->xfact;
	int yfact = (line_mode == GFX_LINE_MODE_FINE)? 1: drv->mode->yfact;
	int xsize = S->xsize;
	int ysize = S->ysize;

	if (color.mask & GFX_MASK_VISUAL) {
		Common::Point nstart, nend;

		for (int xc = 0; xc < xfact; xc++) {
			for (int yc = 0; yc < yfact; yc++) {

				nstart.x = CLIP<int16>(start.x + xc, 0, xsize);
				nstart.y = CLIP<int16>(start.y + yc, 0, ysize);
				nend.x = CLIP<int16>(end.x + xc, 0, xsize - 1);
				nend.y = CLIP<int16>(end.y + yc, 0, ysize - 1);

				Graphics::drawLine(nstart.x, nstart.y, nend.x, nend.y, scolor, drawProc, drv);

				if (color.mask & GFX_MASK_PRIORITY) {
					gfx_draw_line_pixmap_i(S->priority[0], nstart, nend, color.priority);
				}
			}
		}
	}

	return GFX_OK;
}

static int scummvm_draw_filled_rect(gfx_driver_t *drv, rect_t rect, gfx_color_t color1, gfx_color_t color2,
	gfx_rectangle_fill_t shade_mode) {
	if (color1.mask & GFX_MASK_VISUAL) {
		for (int i = rect.y; i < rect.y + rect.height; i++) {
			memset(S->visual[0] + i * S->xsize + rect.x, color1.visual.parent_index, rect.width);
		}
	}

	if (color1.mask & GFX_MASK_PRIORITY)
		gfx_draw_box_pixmap_i(S->priority[0], rect, color1.priority);

	return GFX_OK;
}

// Pixmap operations

static int scummvm_draw_pixmap(gfx_driver_t *drv, gfx_pixmap_t *pxm, int priority,
							   rect_t src, rect_t dest, gfx_buffer_t buffer) {
	int bufnr = (buffer == GFX_BUFFER_STATIC) ? 1 : 0;

	if (dest.width != src.width || dest.height != src.height) {
		printf("Attempt to scale pixmap (%dx%d)->(%dx%d): Not supported\n", src.width, src.height, dest.width, dest.height);
		return GFX_ERROR;
	}

	gfx_crossblit_pixmap(drv->mode, pxm, priority, src, dest, S->visual[bufnr], S->xsize,
	                     S->priority[bufnr]->index_data, S->priority[bufnr]->index_width, 1, 0);

	return GFX_OK;
}

static int scummvm_grab_pixmap(gfx_driver_t *drv, rect_t src, gfx_pixmap_t *pxm, gfx_map_mask_t map) {
	if (src.x < 0 || src.y < 0) {
		printf("Attempt to grab pixmap from invalid coordinates (%d,%d)\n", src.x, src.y);
		return GFX_ERROR;
	}

	if (!pxm->data) {
		printf("Attempt to grab pixmap to unallocated memory\n");
		return GFX_ERROR;
	}

	switch (map) {

	case GFX_MASK_VISUAL:
		pxm->width = src.width;
		pxm->height = src.height;
		for (int i = 0; i < src.height; i++) {
			memcpy(pxm->data + i * src.width, S->visual[0] + (i + src.y) * S->xsize + src.x, src.width);
		}
		break;

	case GFX_MASK_PRIORITY:
		printf("FIXME: priority map grab not implemented yet!\n");
		break;

	default:
		printf("Attempt to grab pixmap from invalid map 0x%02x\n", map);
		return GFX_ERROR;
	}

	return GFX_OK;
}

// Buffer operations

static int scummvm_update(gfx_driver_t *drv, rect_t src, Common::Point dest, gfx_buffer_t buffer) {
	//TODO

	/*
	if (src.x != dest.x || src.y != dest.y) {
		printf("Updating %d (%d,%d)(%dx%d) to (%d,%d) on %d\n", buffer, src.x, src.y, src.width, src.height, dest.x, dest.y, data_dest);
	} else {
		printf("Updating %d (%d,%d)(%dx%d) to %d\n", buffer, src.x, src.y, src.width, src.height, data_dest);
	}
	*/

	switch (buffer) {
	case GFX_BUFFER_BACK:
		for (int i = 0; i < src.height; i++) {
			memcpy(S->visual[0] + (dest.y + i) * S->xsize + dest.x,
			       S->visual[1] + (src.y + i) * S->xsize + src.x, src.width);
		}

		if ((src.x == dest.x) && (src.y == dest.y))
			gfx_copy_pixmap_box_i(S->priority[0], S->priority[1], src);
		break;
	case GFX_BUFFER_FRONT:
		g_system->copyRectToScreen(S->visual[0] + src.x + src.y * S->xsize, S->xsize, dest.x, dest.y, src.width, src.height);
		g_system->updateScreen();
		break;
	default:
		GFXERROR("Invalid buffer %d in update!\n", buffer);
		return GFX_ERROR;
	}

	return GFX_OK;
}

static int scummvm_set_static_buffer(gfx_driver_t *drv, gfx_pixmap_t *pic, gfx_pixmap_t *priority) {
	memcpy(S->visual[1], pic->data, S->xsize * S->ysize);
	gfx_copy_pixmap_box_i(S->priority[1], priority, gfx_rect(0, 0, S->xsize, S->ysize));

	return GFX_OK;
}

// Mouse pointer operations

// Scale cursor and map its colors to the global palette
static uint8 *create_cursor(gfx_driver_t *drv, gfx_pixmap_t *pointer, int mode)
{
	int linewidth = pointer->width;
	int lines = pointer->height;
	uint8 *data = new uint8[linewidth*lines];
	uint8 *linebase = data, *pos;
	uint8 *src = pointer->index_data;

	for (int yc = 0; yc < pointer->index_height; yc++) {
		pos = linebase;

		for (int xc = 0; xc < pointer->index_width; xc++) {
			uint8 color = *src;
			// FIXME: The palette size check is a workaround for cursors using non-palette colour GFX_CURSOR_TRANSPARENT
			// Note that some cursors don't have a palette in SQ5
			if (pointer->palette && color < pointer->palette->size())
				color = pointer->palette->getColor(color).parent_index;
			for (int scalectr = 0; scalectr < drv->mode->xfact; scalectr++) {
				*pos++ = color;
			}
			src++;
		}
		for (int scalectr = 1; scalectr < drv->mode->yfact; scalectr++)
			memcpy(linebase + linewidth * scalectr, linebase, linewidth);
		linebase += linewidth * drv->mode->yfact;
	}
	return data;
}


static int scummvm_set_pointer(gfx_driver_t *drv, gfx_pixmap_t *pointer, Common::Point *hotspot) {
	if ((pointer == NULL) || (hotspot == NULL)) {
		g_system->showMouse(false);
	} else {
		delete[] S->pointer_data;
		S->pointer_data = create_cursor(drv, pointer, 1);

		// FIXME: The palette size check is a workaround for cursors using non-palette colour GFX_CURSOR_TRANSPARENT
		// Note that some cursors don't have a palette in SQ5
		uint8 color_key = GFX_CURSOR_TRANSPARENT;
		if ((pointer->color_key != GFX_PIXMAP_COLOR_KEY_NONE) && (pointer->palette && (unsigned int)pointer->color_key < pointer->palette->size()))
			color_key = pointer->palette->getColor(pointer->color_key).parent_index;
		// Some cursors in SQ5 don't have a palette. The cursor palette seems to use 64 colors, so setting the color key to 63 works
		// TODO: Is this correct?
		if (!pointer->palette)
			color_key = 63;

		g_system->setMouseCursor(S->pointer_data, pointer->width, pointer->height, hotspot->x, hotspot->y, color_key);
		g_system->showMouse(true);
	}

	return GFX_OK;
}

gfx_driver_t gfx_driver_scummvm = {
	NULL,
	0, 0,
	0,		// flags here
	0,
	NULL,
	scummvm_init,
	scummvm_exit,
	scummvm_draw_line,
	scummvm_draw_filled_rect,
	scummvm_draw_pixmap,
	scummvm_grab_pixmap,
	scummvm_update,
	scummvm_set_static_buffer,
	scummvm_set_pointer,
	NULL
};

} // End of namespace Sci
