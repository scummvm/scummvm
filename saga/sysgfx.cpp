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
#include "reinherit.h"

#include <SDL.h>
#include <limits.h>

/*
 * Begin module component
\*--------------------------------------------------------------------------*/
#include "sysgfx.h"

namespace Saga {

R_SYSGFX_MODULE SGfxModule;

static SDL_Color cur_pal[R_PAL_ENTRIES];

int SYSGFX_Init(R_SYSGFX_INIT *gfx_init) {
	SDL_Surface *sdl_screen;
	R_SURFACE r_screen;

	SDL_Surface *sdl_back_buf;
	R_SURFACE r_back_buf;

	int result;
	Uint32 flags;

	assert(gfx_init != NULL);

	if (gfx_init->fullscreen) {
		flags = SDL_FULLSCREEN | SDL_HWPALETTE;
	} else {
		flags = SDL_HWPALETTE;
	}

	/* Test video mode availability */
	result = SDL_VideoModeOK(gfx_init->screen_w,
	    gfx_init->screen_h, gfx_init->screen_bpp, flags);
	if (result == 0) {
		R_printf(R_STDERR,
		    "Requested video mode (%d x %d @ %d bpp) "
		    "is unavailable.\n",
		    gfx_init->screen_w,
		    gfx_init->screen_h, gfx_init->screen_bpp);

		return R_FAILURE;
	}

	/* Set the video mode */
	sdl_screen = SDL_SetVideoMode(gfx_init->screen_w,
	    gfx_init->screen_h, gfx_init->screen_bpp, flags);
	if (sdl_screen == NULL) {
		R_printf(R_STDERR,
		    "Unable to set video mode (%d x %d @ %d bpp).\n",
		    gfx_init->screen_w,
		    gfx_init->screen_h, gfx_init->screen_bpp);

		R_printf(R_STDERR, "SDL reports: %s\n", SDL_GetError());

		return R_FAILURE;
	}

	R_printf(R_STDOUT,
	    "Set video mode: (%d x %d @ %d bpp)\n",
	    sdl_screen->w, sdl_screen->h, sdl_screen->format->BitsPerPixel);

	/* Convert sdl surface data to R surface data */
	r_screen.buf = (uchar *)sdl_screen->pixels;
	r_screen.buf_w = sdl_screen->w;
	r_screen.buf_h = sdl_screen->h;
	r_screen.buf_pitch = sdl_screen->pitch;
	r_screen.bpp = gfx_init->screen_bpp;

	r_screen.clip_rect.left = 0;
	r_screen.clip_rect.top = 0;
	r_screen.clip_rect.right = sdl_screen->w - 1;
	r_screen.clip_rect.bottom = sdl_screen->h - 1;

	r_screen.impl_src = sdl_screen;

	/* Create the back buffer */
	sdl_back_buf = SDL_CreateRGBSurface(SDL_SWSURFACE,
	    gfx_init->backbuf_w,
	    gfx_init->backbuf_h, gfx_init->backbuf_bpp, 0, 0, 0, 0);
	if (sdl_back_buf == NULL) {

		R_printf(R_STDERR,
		    "Unable to create back buffer (%d x %d @ %d bpp).\n",
		    gfx_init->backbuf_w,
		    gfx_init->backbuf_h, gfx_init->backbuf_bpp);

		R_printf(R_STDERR, "SDL reports: %s.\n", SDL_GetError());

		return R_FAILURE;
	}

	/* Convert sdl surface data to R surface data */
	r_back_buf.buf = (uchar *)sdl_back_buf->pixels;
	r_back_buf.buf_w = sdl_back_buf->w;
	r_back_buf.buf_h = sdl_back_buf->h;
	r_back_buf.buf_pitch = sdl_back_buf->pitch;
	r_back_buf.bpp = gfx_init->backbuf_bpp;

	r_back_buf.clip_rect.left = 0;
	r_back_buf.clip_rect.top = 0;
	r_back_buf.clip_rect.right = sdl_back_buf->w - 1;
	r_back_buf.clip_rect.bottom = sdl_back_buf->h - 1;

	r_back_buf.impl_src = sdl_back_buf;

	/* Set module data */
	SGfxModule.sdl_screen = sdl_screen;
	SGfxModule.r_screen = r_screen;
	SGfxModule.sdl_back_buf = sdl_back_buf;
	SGfxModule.r_back_buf = r_back_buf;

	SGfxModule.init = 1;

	return R_SUCCESS;
}

R_SURFACE *SYSGFX_GetScreenSurface(void) {
	return &SGfxModule.r_screen;
}

R_SURFACE *SYSGFX_GetBackBuffer(void) {
	return &SGfxModule.r_back_buf;
}

int SYSGFX_LockSurface(R_SURFACE *surface) {
	int result;

	assert(surface != NULL);

	result = SDL_LockSurface((SDL_Surface *) surface->impl_src);

	return (result == 0) ? R_SUCCESS : R_FAILURE;
}

int SYSGFX_UnlockSurface(R_SURFACE *surface) {
	assert(surface != NULL);

	SDL_UnlockSurface((SDL_Surface *) surface->impl_src);

	return R_SUCCESS;
}

R_SURFACE *SYSGFX_FormatToDisplay(R_SURFACE *surface) {
	R_SURFACE *new_r_surface;
	SDL_Surface *new_sdl_surface;

	new_r_surface = (R_SURFACE *)malloc(sizeof *new_r_surface);
	if (new_r_surface == NULL) {
		return NULL;
	}

	new_sdl_surface = SDL_DisplayFormat((SDL_Surface *)surface->impl_src);
	if (new_sdl_surface == NULL) {
		free(new_r_surface);
		return NULL;
	}

	new_r_surface->buf = (uchar *)new_sdl_surface->pixels;
	new_r_surface->buf_w = new_sdl_surface->w;
	new_r_surface->buf_h = new_sdl_surface->h;
	new_r_surface->buf_pitch = new_sdl_surface->pitch;
	new_r_surface->bpp = new_sdl_surface->format->BitsPerPixel;

	new_r_surface->clip_rect.left = 0;
	new_r_surface->clip_rect.top = 0;
	new_r_surface->clip_rect.right = new_sdl_surface->w - 1;
	new_r_surface->clip_rect.bottom = new_sdl_surface->h - 1;

	new_r_surface->impl_src = new_sdl_surface;

	return new_r_surface;
}

R_SURFACE *SYSGFX_CreateSurface(int w, int h, int bpp) {
	R_SURFACE *new_surface;
	SDL_Surface *new_sdl_surface;

	assert(bpp == 8);	/* 16bpp not supported, maybe not necessary? */
	assert((w > 0) && (h > 0));

	new_surface = (R_SURFACE *)malloc(sizeof *new_surface);
	if (new_surface == NULL) {
		return NULL;
	}

	new_sdl_surface =
	    SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, bpp, 0, 0, 0, 0);
	if (new_sdl_surface == NULL) {

		free(new_surface);
		return NULL;
	}

	new_surface->buf_w = new_sdl_surface->w;
	new_surface->buf_h = new_sdl_surface->h;
	new_surface->buf_pitch = new_sdl_surface->pitch;
	new_surface->bpp = new_sdl_surface->format->BitsPerPixel;

	new_surface->clip_rect.left = 0;
	new_surface->clip_rect.top = 0;
	new_surface->clip_rect.right = w - 1;
	new_surface->clip_rect.bottom = h - 1;

	new_surface->impl_src = new_sdl_surface;

	return new_surface;
}

int SYSGFX_DestroySurface(R_SURFACE *surface) {
	SDL_FreeSurface((SDL_Surface *) surface->impl_src);

	free(surface);

	return R_SUCCESS;
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

	for (i = 0; i < R_PAL_ENTRIES; i++) {
		dr = cur_pal[i].r - red;
		dr = ABS(dr);

		dg = cur_pal[i].g - green;
		dg = ABS(dg);

		db = cur_pal[i].b - blue;
		db = ABS(db);

#if R_COLORSEARCH_SQUARE
		color_delta = (long)((dr * dr) * R_RED_WEIGHT +
		    (dg * dg) * R_GREEN_WEIGHT + (db * db) * R_BLUE_WEIGHT);
#else
		color_delta = (long)(dr * R_RED_WEIGHT +
		    dg * R_GREEN_WEIGHT + db * R_BLUE_WEIGHT);
#endif
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

	uchar red;
	uchar green;
	uchar blue;

	int color_delta;
	int best_wdelta = 0;
	int best_windex = 0;
	int best_bindex = 0;
	int best_bdelta = 1000;

	int i;

	for (i = 0; i < R_PAL_ENTRIES; i++) {
		red = pal[i].red;
		cur_pal[i].r = red;

		color_delta = red;

		green = pal[i].green;
		cur_pal[i].g = green;

		color_delta += green;

		blue = pal[i].blue;
		cur_pal[i].b = blue;

		color_delta += blue;

		if (color_delta < best_bdelta) {
			best_bindex = i;
			best_bdelta = color_delta;
		}

		if (color_delta > best_wdelta) {
			best_windex = i;
			best_wdelta = color_delta;
		}
	}

	/* Set whitest and blackest color indices */
	SGfxModule.white_index = best_windex;
	SGfxModule.black_index = best_bindex;

	/* If the screen surface is palettized, set the screen palette.
	 * If the screen surface is not palettized, set the palette of 
	 * the surface parameter */
	if (SGfxModule.r_screen.bpp < 16) {
		SDL_SetColors(SGfxModule.sdl_screen, cur_pal, 0,
		    R_PAL_ENTRIES);
	} else {
		SDL_SetColors((SDL_Surface *) surface->impl_src,
		    cur_pal, 0, R_PAL_ENTRIES);
	}

	return R_SUCCESS;
}

int SYSGFX_GetCurrentPal(PALENTRY *src_pal) {
	int i;

	for (i = 0; i < R_PAL_ENTRIES; i++) {
		src_pal[i].red = cur_pal[i].r;
		src_pal[i].green = cur_pal[i].g;
		src_pal[i].blue = cur_pal[i].b;
	}

	return R_SUCCESS;
}

int SYSGFX_PalToBlack(R_SURFACE *surface, PALENTRY *src_pal, double percent) {
	int i;

	/*int fade_max = 255; */
	int new_entry;

	double fpercent;

	if (percent > 1.0) {
		percent = 1.0;
	}

	/* Exponential fade */
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	/* Use the correct percentage change per frame for each palette entry */
	for (i = 0; i < R_PAL_ENTRIES; i++) {
		new_entry = (int)(src_pal[i].red * fpercent);

		if (new_entry < 0) {
			cur_pal[i].r = 0;
		} else {
			cur_pal[i].r = (uchar) new_entry;
		}

		new_entry = (int)(src_pal[i].green * fpercent);

		if (new_entry < 0) {
			cur_pal[i].g = 0;
		} else {
			cur_pal[i].g = (uchar) new_entry;
		}

		new_entry = (int)(src_pal[i].blue * fpercent);

		if (new_entry < 0) {
			cur_pal[i].b = 0;
		} else {
			cur_pal[i].b = (uchar) new_entry;
		}
	}

	/* If the screen surface is palettized, set the screen palette.
	 * If the screen surface is not palettized, set the palette of 
	 * the surface parameter */
	if (SGfxModule.r_screen.bpp < 16) {
		SDL_SetColors(SGfxModule.sdl_screen, cur_pal, 0,
		    R_PAL_ENTRIES);
	} else {
		SDL_SetColors((SDL_Surface *) surface->impl_src,
		    cur_pal, 0, R_PAL_ENTRIES);
	}

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

	int i;

	if (percent > 1.0) {
		percent = 1.0;
	}

	/* Exponential fade */
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	/* Use the correct percentage change per frame for each palette entry */
	for (i = 0; i < R_PAL_ENTRIES; i++) {
		new_entry = (int)(src_pal[i].red - src_pal[i].red * fpercent);

		if (new_entry < 0) {
			cur_pal[i].r = 0;
		} else {
			cur_pal[i].r = (uchar) new_entry;
		}

		new_entry =
		    (int)(src_pal[i].green - src_pal[i].green * fpercent);

		if (new_entry < 0) {
			cur_pal[i].g = 0;
		} else {
			cur_pal[i].g = (uchar) new_entry;
		}

		new_entry =
		    (int)(src_pal[i].blue - src_pal[i].blue * fpercent);

		if (new_entry < 0) {
			cur_pal[i].b = 0;
		} else {
			cur_pal[i].b = (uchar) new_entry;
		}
	}

	/* Find the best white and black color indices again */
	if (percent >= 1.0) {
		for (i = 0; i < R_PAL_ENTRIES; i++) {
			color_delta = cur_pal[i].r;
			color_delta += cur_pal[i].g;
			color_delta += cur_pal[i].b;

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

	/* If the screen surface is palettized, set the screen palette.
	 * If the screen surface is not palettized, set the palette of 
	 * the surface parameter */
	if (SGfxModule.r_screen.bpp < 16) {
		SDL_SetColors(SGfxModule.sdl_screen, cur_pal, 0,
		    R_PAL_ENTRIES);
	} else {
		SDL_SetColors((SDL_Surface *) surface->impl_src,
		    cur_pal, 0, R_PAL_ENTRIES);
	}

	return R_SUCCESS;
}

} // End of namespace Saga

