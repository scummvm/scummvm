/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "saga.h"
#include "reinherit.h"

#include "sysgfx.h"

namespace Saga {

static R_SYSGFX_MODULE SGfxModule;
static OSystem *_system;

static byte cur_pal[R_PAL_ENTRIES * 4];

int SYSGFX_Init(OSystem *system, int width, int height) {
	R_SURFACE r_back_buf;

	_system = system;
	_system->initSize(width, height);

	debug(0, "Init screen %dx%d", width, height);
	// Convert sdl surface data to R surface data
	r_back_buf.buf = (byte *)calloc(1, width * height);
	r_back_buf.buf_w = width;
	r_back_buf.buf_h = height;
	r_back_buf.buf_pitch = width;
	r_back_buf.bpp = 8;

	r_back_buf.clip_rect.left = 0;
	r_back_buf.clip_rect.top = 0;
	r_back_buf.clip_rect.right = width - 1;
	r_back_buf.clip_rect.bottom = height - 1;

	// Set module data
	SGfxModule.r_back_buf = r_back_buf;
	SGfxModule.init = 1;

	return R_SUCCESS;
}

/*
~SysGfx() {
  free(SGfxModule.r_back_buf->buf);
}
 */

R_SURFACE *SYSGFX_GetBackBuffer() {
	return &SGfxModule.r_back_buf;
}

int SYSGFX_LockSurface(R_SURFACE *surface) {
	return 0;
}

int SYSGFX_UnlockSurface(R_SURFACE *surface) {
	return 0;
}

int SYSGFX_GetWhite(void) {
	return SGfxModule.white_index;
}

int SYSGFX_GetBlack(void) {
	return SGfxModule.black_index;
}

int SYSGFX_MatchColor(unsigned long colormask) {
	int i;
	int red = (colormask & 0x0FF0000UL) >> 16;
	int green = (colormask & 0x000FF00UL) >> 8;
	int blue = colormask & 0x00000FFUL;
	int dr;
	int dg;
	int db;
	long color_delta;
	long best_delta = LONG_MAX;
	int best_index = 0;
	byte *ppal;

	for (i = 0, ppal = cur_pal; i < R_PAL_ENTRIES; i++, ppal += 4) {
		dr = ppal[0] - red;
		dr = ABS(dr);
		dg = ppal[1] - green;
		dg = ABS(dg);
		db = ppal[2] - blue;
		db = ABS(db);
		ppal[3] = 0;

		color_delta = (long)(dr * R_RED_WEIGHT + dg * R_GREEN_WEIGHT + db * R_BLUE_WEIGHT);

		if (color_delta == 0) {
			return i;
		}

		if (color_delta < best_delta) {
			best_delta = color_delta;
			best_index = i;
		}
	}

	return best_index;
}

int SYSGFX_SetPalette(R_SURFACE *surface, PALENTRY *pal) {
	byte red;
	byte green;
	byte blue;
	int color_delta;
	int best_wdelta = 0;
	int best_windex = 0;
	int best_bindex = 0;
	int best_bdelta = 1000;
	int i;
	byte *ppal;

	for (i = 0, ppal = cur_pal; i < R_PAL_ENTRIES; i++, ppal += 4) {
		red = pal[i].red;
		ppal[0] = red;
		color_delta = red;
		green = pal[i].green;
		ppal[1] = green;
		color_delta += green;
		blue = pal[i].blue;
		ppal[2] = blue;
		color_delta += blue;
		ppal[3] = 0;

		if (color_delta < best_bdelta) {
			best_bindex = i;
			best_bdelta = color_delta;
		}

		if (color_delta > best_wdelta) {
			best_windex = i;
			best_wdelta = color_delta;
		}
	}

	// Set whitest and blackest color indices
	SGfxModule.white_index = best_windex;
	SGfxModule.black_index = best_bindex;

	_system->setPalette(cur_pal, 0, R_PAL_ENTRIES);

	return R_SUCCESS;
}

int SYSGFX_GetCurrentPal(PALENTRY *src_pal) {
	int i;
	byte *ppal;

	for (i = 0, ppal = cur_pal; i < R_PAL_ENTRIES; i++, ppal += 4) {
		src_pal[i].red = ppal[0];
		src_pal[i].green = ppal[1];
		src_pal[i].blue = ppal[2];
	}

	return R_SUCCESS;
}

int SYSGFX_PalToBlack(R_SURFACE *surface, PALENTRY *src_pal, double percent) {
	int i;
	//int fade_max = 255;
	int new_entry;
	byte *ppal;

	double fpercent;

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry 
	for (i = 0, ppal = cur_pal; i < R_PAL_ENTRIES; i++, ppal += 4) {
		new_entry = (int)(src_pal[i].red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
		ppal[3] = 0;
	}

	_system->setPalette(cur_pal, 0, R_PAL_ENTRIES);

	return R_SUCCESS;
}

int SYSGFX_BlackToPal(R_SURFACE *surface, PALENTRY *src_pal, double percent) {
	int new_entry;
	double fpercent;
	int color_delta;
	int best_wdelta = 0;
	int best_windex = 0;
	int best_bindex = 0;
	int best_bdelta = 1000;
	byte *ppal;
	int i;

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry
	for (i = 0, ppal = cur_pal; i < R_PAL_ENTRIES; i++, ppal += 4) {
		new_entry = (int)(src_pal[i].red - src_pal[i].red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].green - src_pal[i].green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].blue - src_pal[i].blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
		ppal[3] = 0;
	}

	// Find the best white and black color indices again
	if (percent >= 1.0) {
		for (i = 0, ppal = cur_pal; i < R_PAL_ENTRIES; i++, ppal += 4) {
			color_delta = ppal[0];
			color_delta += ppal[1];
			color_delta += ppal[2];

			if (color_delta < best_bdelta) {
				best_bindex = i;
				best_bdelta = color_delta;
			}

			if (color_delta > best_wdelta) {
				best_windex = i;
				best_wdelta = color_delta;
			}
		}
	}

	_system->setPalette(cur_pal, 0, R_PAL_ENTRIES);

	return R_SUCCESS;
}

} // End of namespace Saga

