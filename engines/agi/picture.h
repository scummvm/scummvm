/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#ifndef AGI_PICTURE_H
#define AGI_PICTURE_H

#include "agi/agi.h"

namespace Agi {

/**
 * AGI picture resource.
 */
struct agi_picture {
	uint32 flen;			/**< size of raw data */
	uint8 *rdata;			/**< raw vector image data */
};

class AgiEngine;
class GfxMgr;

class PictureMgr {
	AgiEngine *_vm;
	GfxMgr *_gfx;

private:

	void draw_line(int x1, int y1, int x2, int y2);
	void put_virt_pixel(int x, int y);
	void dynamic_draw_line();
	void absolute_draw_line();
	INLINE int is_ok_fill_here(int x, int y);
	void fill_scanline(int x, int y);
	void agi_fill(unsigned int x, unsigned int y);
	void x_corner();
	void y_corner();
	void fill();
	int plot_pattern_point(int x, int y, int bitpos);
	void plot_pattern(int x, int y);
	void plot_brush();
	void draw_picture();

public:
	PictureMgr(AgiEngine *agi, GfxMgr *gfx) {
		_vm = agi;
		_gfx = gfx;
	}

	int decode_picture(int, int);
	int unload_picture(int);
	void show_pic();
	uint8 *convert_v3_pic(uint8 *src, uint32 len);
};

}                             // End of namespace Agi

#endif				/* AGI_PICTURE_H */
