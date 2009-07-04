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
#include "graphics/cursorman.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/gfx/gfx_driver.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

GfxDriver::GfxDriver(int xfact, int yfact, int bytespp) {
	int i;

	Graphics::PixelFormat format = { bytespp, 0, 0, 0, 0, 0, 0, 0, 0 };
	_mode = gfx_new_mode(xfact, yfact, format, new Palette(256), 0);
	_mode->xsize = xfact * 320;
	_mode->ysize = yfact * 200;

	for (i = 0; i < 2; i++) {
		_priority[i] = gfx_pixmap_alloc_index_data(gfx_new_pixmap(_mode->xsize, _mode->ysize, GFX_RESID_NONE, -i, -777));
		if (!_priority[i]) {
			error("Out of memory: Could not allocate priority maps! (%dx%d)\n", _mode->xsize, _mode->ysize);
		}
	}
	// create the visual buffers
	for (i = 0; i < 2; i++) {
		_visual[i] = NULL;
		_visual[i] = new byte[_mode->xsize * _mode->ysize];
		if (!_visual[i]) {
			error("Out of memory: Could not allocate visual buffers! (%dx%d)\n", _mode->xsize, _mode->ysize);
		}
		memset(_visual[i], 0, _mode->xsize * _mode->ysize);
	}

	_mode->palette->name = "global";
}

GfxDriver::~GfxDriver() {
	int i;
	for (i = 0; i < 2; i++) {
		gfx_free_pixmap(_priority[i]);
		_priority[i] = NULL;
	}

	for (i = 0; i < 2; i++) {
		delete[] _visual[i];
		_visual[i] = NULL;
	}
}


// Drawing operations

static void drawProc(int x, int y, int c, void *data) {
	GfxDriver *drv = (GfxDriver *)data;
	byte *p = drv->getVisual0();
	p[y * 320* drv->getMode()->xfact + x] = c;
}

int GfxDriver::drawLine(Common::Point start, Common::Point end, gfx_color_t color, 
						gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	uint32 scolor = color.visual.parent_index;
	int xfact = (line_mode == GFX_LINE_MODE_FINE)? 1: _mode->xfact;
	int yfact = (line_mode == GFX_LINE_MODE_FINE)? 1: _mode->yfact;
	int xsize = _mode->xsize;
	int ysize = _mode->ysize;

	if (color.mask & GFX_MASK_VISUAL) {
		Common::Point nstart, nend;

		for (int xc = 0; xc < xfact; xc++) {
			for (int yc = 0; yc < yfact; yc++) {

				nstart.x = CLIP<int16>(start.x + xc, 0, xsize);
				nstart.y = CLIP<int16>(start.y + yc, 0, ysize);
				nend.x = CLIP<int16>(end.x + xc, 0, xsize - 1);
				nend.y = CLIP<int16>(end.y + yc, 0, ysize - 1);

				Graphics::drawLine(nstart.x, nstart.y, nend.x, nend.y, scolor, drawProc, this);

				if (color.mask & GFX_MASK_PRIORITY) {
					gfx_draw_line_pixmap_i(_priority[0], nstart, nend, color.priority);
				}
			}
		}
	}

	return GFX_OK;
}

int GfxDriver::drawFilledRect(rect_t rect, gfx_color_t color1, gfx_color_t color2,
	gfx_rectangle_fill_t shade_mode) {
	if (color1.mask & GFX_MASK_VISUAL) {
		for (int i = rect.y; i < rect.y + rect.height; i++) {
			memset(_visual[0] + i * _mode->xsize + rect.x, color1.visual.parent_index, rect.width);
		}
	}

	if (color1.mask & GFX_MASK_PRIORITY)
		gfx_draw_box_pixmap_i(_priority[0], rect, color1.priority);

	return GFX_OK;
}

// Pixmap operations

int GfxDriver::drawPixmap(gfx_pixmap_t *pxm, int priority, rect_t src, rect_t dest, gfx_buffer_t buffer) {
	int bufnr = (buffer == GFX_BUFFER_STATIC) ? 1 : 0;

	if (dest.width != src.width || dest.height != src.height) {
		printf("Attempt to scale pixmap (%dx%d)->(%dx%d): Not supported\n", src.width, src.height, dest.width, dest.height);
		return GFX_ERROR;
	}

	gfx_crossblit_pixmap(_mode, pxm, priority, src, dest, _visual[bufnr], _mode->xsize,
	                     _priority[bufnr]->index_data, _priority[bufnr]->index_width, 1, 0);

	return GFX_OK;
}

int GfxDriver::grabPixmap(rect_t src, gfx_pixmap_t *pxm, gfx_map_mask_t map) {
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
			memcpy(pxm->data + i * src.width, _visual[0] + (i + src.y) * _mode->xsize + src.x, src.width);
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

int GfxDriver::update(rect_t src, Common::Point dest, gfx_buffer_t buffer) {
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
			memcpy(_visual[0] + (dest.y + i) * _mode->xsize + dest.x,
			       _visual[1] + (src.y + i) * _mode->xsize + src.x, src.width);
		}

		if ((src.x == dest.x) && (src.y == dest.y))
			gfx_copy_pixmap_box_i(_priority[0], _priority[1], src);
		break;
	case GFX_BUFFER_FRONT:
		g_system->copyRectToScreen(_visual[0] + src.x + src.y * _mode->xsize, _mode->xsize, dest.x, dest.y, src.width, src.height);
		g_system->updateScreen();
		break;
	default:
		GFXERROR("Invalid buffer %d in update!\n", buffer);
		return GFX_ERROR;
	}

	return GFX_OK;
}

int GfxDriver::setStaticBuffer(gfx_pixmap_t *pic, gfx_pixmap_t *priority) {
	memcpy(_visual[1], pic->data, _mode->xsize * _mode->ysize);
	gfx_copy_pixmap_box_i(_priority[1], priority, gfx_rect(0, 0, _mode->xsize, _mode->ysize));

	return GFX_OK;
}

// Mouse pointer operations

// Scale cursor and map its colors to the global palette
byte *GfxDriver::createCursor(gfx_pixmap_t *pointer) {
	int linewidth = pointer->width;
	int lines = pointer->height;
	byte *data = new byte[linewidth*lines];
	byte *linebase = data, *pos;
	byte *src = pointer->index_data;

	for (int yc = 0; yc < pointer->index_height; yc++) {
		pos = linebase;

		for (int xc = 0; xc < pointer->index_width; xc++) {
			byte color = *src;
			// FIXME: The palette size check is a workaround for cursors using non-palette colour GFX_CURSOR_TRANSPARENT
			// Note that some cursors don't have a palette in SQ5
			if (pointer->palette && color < pointer->palette->size())
				color = pointer->palette->getColor(color).parent_index;
			for (int scalectr = 0; scalectr < _mode->xfact; scalectr++) {
				*pos++ = color;
			}
			src++;
		}
		for (int scalectr = 1; scalectr < _mode->yfact; scalectr++)
			memcpy(linebase + linewidth * scalectr, linebase, linewidth);
		linebase += linewidth * _mode->yfact;
	}
	return data;
}


int GfxDriver::setPointer(gfx_pixmap_t *pointer, Common::Point *hotspot) {
	if ((pointer == NULL) || (hotspot == NULL)) {
		CursorMan.showMouse(false);
	} else {
		byte *cursorData = createCursor(pointer);

		// FIXME: The palette size check is a workaround for cursors using non-palette color GFX_CURSOR_TRANSPARENT
		// Note that some cursors don't have a palette (e.g. in SQ5 and QFG3)
		byte color_key = GFX_CURSOR_TRANSPARENT;
		if ((pointer->color_key != GFX_PIXMAP_COLOR_KEY_NONE) && (pointer->palette && (unsigned int)pointer->color_key < pointer->palette->size()))
			color_key = pointer->palette->getColor(pointer->color_key).parent_index;
		// Some cursors don't have a palette, so we set the color key directly
		if (!pointer->palette)
			color_key = pointer->color_key;

		CursorMan.replaceCursor(cursorData, pointer->width, pointer->height, hotspot->x, hotspot->y, color_key);
		CursorMan.showMouse(true);

		delete[] cursorData;
		cursorData = 0;
	}

	return GFX_OK;
}

} // End of namespace Sci
