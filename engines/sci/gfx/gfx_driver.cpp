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

#include "sci/sci.h"	// for INCLUDE_OLDGFX
#ifdef INCLUDE_OLDGFX

#include "common/scummsys.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/primitives.h"
#include "graphics/surface.h"

#include "sci/gui/gui_screen.h"
#include "sci/gfx/gfx_driver.h"
#include "sci/gfx/gfx_tools.h"

#include "sci/gui/gui_screen.h"

namespace Sci {


GfxDriver::GfxDriver(SciGuiScreen *screen, int scaleFactor) : _screen(screen) {
	_mode = gfx_new_mode(scaleFactor, new Palette(256));

	if (_mode->palette)
		_mode->palette->name = "global";
}

GfxDriver::~GfxDriver() {
}


// Drawing operations

static void drawProc(int x, int y, int c, void *data) {
	GfxDriver *drv = (GfxDriver *)data;
	byte *p = drv->_screen->_displayScreen;
	uint8 col = c;
	memcpy(p + (y * drv->_screen->_width * drv->getMode()->scaleFactor + x), &col, 1);
}

void GfxDriver::drawLine(Common::Point start, Common::Point end, gfx_color_t color,
						gfx_line_mode_t line_mode, gfx_line_style_t line_style) {
	uint32 scolor = color.visual.getParentIndex();
	int scaleFactor = (line_mode == GFX_LINE_MODE_FINE)? 1: _mode->scaleFactor;
	int xsize = _mode->xsize;
	int ysize = _mode->ysize;

	if (color.mask & GFX_MASK_VISUAL) {
		Common::Point nstart, nend;

		for (int xc = 0; xc < scaleFactor; xc++) {
			for (int yc = 0; yc < scaleFactor; yc++) {

				nstart.x = CLIP<int16>(start.x + xc, 0, xsize);
				nstart.y = CLIP<int16>(start.y + yc, 0, ysize);
				nend.x = CLIP<int16>(end.x + xc, 0, xsize - 1);
				nend.y = CLIP<int16>(end.y + yc, 0, ysize - 1);

				Graphics::drawLine(nstart.x, nstart.y, nend.x, nend.y, scolor, drawProc, this);

				if (color.mask & GFX_MASK_PRIORITY) {
					gfx_draw_line_buffer(_screen->_priorityScreen, 1, 1, nstart, nend, color.priority);
				}
			}
		}
	}
}

void GfxDriver::drawFilledRect(rect_t rect, gfx_color_t color1, gfx_color_t color2,
	gfx_rectangle_fill_t shade_mode) {
	if (color1.mask & GFX_MASK_VISUAL) {
		for (int i = rect.y; i < rect.y + rect.height; i++) {
			memset(_screen->_displayScreen + (i * _mode->xsize + rect.x),
			       color1.visual.getParentIndex(), rect.width);
		}
	}

	if (color1.mask & GFX_MASK_PRIORITY) {
		gfx_clip_box_basic(&rect, _screen->_width - 1, _screen->_height - 1);
		gfx_draw_box_buffer(_screen->_priorityScreen, _screen->_width, rect, color1.priority);
	}
}

// Pixmap operations

void GfxDriver::drawPixmap(gfx_pixmap_t *pxm, int priority, rect_t src, rect_t dest, gfx_buffer_t buffer) {
	byte *destBuffer = (buffer == GFX_BUFFER_STATIC) ? _screen->_visualScreen : _screen->_displayScreen;
	byte *destPriority = (buffer == GFX_BUFFER_STATIC) ? _screen->_controlScreen : _screen->_priorityScreen;
	if (dest.width != src.width || dest.height != src.height) {
		warning("Attempt to scale pixmap (%dx%d)->(%dx%d): Not supported\n", src.width, src.height, dest.width, dest.height);
		return;
	}

	gfx_crossblit_pixmap(_mode, pxm, priority, src, dest, destBuffer,
	                     _mode->xsize,
	                     destPriority,
	                     _screen->_width, 1);
}

void GfxDriver::grabPixmap(rect_t src, gfx_pixmap_t *pxm, gfx_map_mask_t map) {
	if (src.x < 0 || src.y < 0)
		error("Attempt to grab pixmap from invalid coordinates (%d,%d)", src.x, src.y);

	if (!pxm->data)
		error("Attempt to grab pixmap to unallocated memory");

	switch (map) {

	case GFX_MASK_VISUAL:
		pxm->width = src.width;
		pxm->height = src.height;
		for (int i = 0; i < src.height; i++) {
			memcpy(pxm->data + i * src.width,
			       _screen->_displayScreen + ((i + src.y) * _mode->xsize + src.x),
			       src.width);
		}
		break;

	case GFX_MASK_PRIORITY:
		warning("FIXME: priority map grab not implemented yet");
		break;

	default:
		error("Attempt to grab pixmap from invalid map 0x%02x", map);
	}
}

// Buffer operations

void GfxDriver::update(rect_t src, Common::Point dest, gfx_buffer_t buffer) {
	switch (buffer) {
	case GFX_BUFFER_BACK:
		for (int i = 0; i < src.height; i++) {
			memcpy(_screen->_displayScreen + ( (dest.y + i) * _mode->xsize + dest.x),
			       _screen->_visualScreen + ( (src.y + i) * _mode->xsize + src.x), src.width );
		}

		if ((src.x == dest.x) && (src.y == dest.y)) {
			int offset = src.x + (src.y * _screen->_width);

			gfx_clip_box_basic(&src, _screen->_width, _screen->_height);

			while (src.height--) {
				memcpy(_screen->_priorityScreen + offset, _screen->_controlScreen + offset, _screen->_width);
				offset += _screen->_width;
			}
		}
		break;
	case GFX_BUFFER_FRONT: {
		// TODO: we need to call SciGuiCursor::refreshPosition() before each screen update to limit the mouse cursor position
		g_system->copyRectToScreen(_screen->_displayScreen + (src.x + src.y * _mode->xsize), _mode->xsize, dest.x, dest.y, src.width, src.height);
		g_system->updateScreen();
		break;
	}
	default:
		error("Invalid buffer %d in update", buffer);
	}
}

void GfxDriver::setStaticBuffer(gfx_pixmap_t *pic, gfx_pixmap_t *priority) {
	memcpy(_screen->_visualScreen, pic->data, _mode->xsize * _mode->ysize);
	memcpy(_screen->_controlScreen, priority->index_data, _mode->xsize * _mode->ysize);
}

void GfxDriver::animatePalette(int fromColor, int toColor, int stepCount) {
	int i;
	PaletteEntry firstColor = _mode->palette->getColor(fromColor);
	PaletteEntry loopColor;
	for (i = fromColor + 1; i <= toColor; i++) {
		loopColor = _mode->palette->getColor(i);
		loopColor.r = 0;
		loopColor.g = 0;
		loopColor.b = 0;
		_mode->palette->makeSystemColor(i-1, loopColor); // loopColor.r, loopColor.g, loopColor.b);
	}
//	_mode->palette->setColor(toColor, firstColor.r, firstColor.g, firstColor.b);
	_mode->palette->makeSystemColor(toColor, firstColor);
}

} // End of namespace Sci

#endif
