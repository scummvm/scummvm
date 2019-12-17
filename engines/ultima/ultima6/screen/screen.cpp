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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cmath>
#include <SDL.h>

#include "ultima/ultima6/core/nuvie_defs.h"
#include "U6misc.h"
#include "ultima/ultima6/conf/configuration.h"

#include "Surface.h"
#include "Scale.h"
#include "Screen.h"
#include "MapWindow.h"
#include "Background.h"

namespace Ultima {
namespace Ultima6 {

#define sqr(a) ((a)*(a))

//Ultima 6 light globe sizes.
#define NUM_GLOBES 5
#define SHADING_BORDER 2 // should be the same as MapWindow's TMP_MAP_BORDER
static const sint32 globeradius[]   = { 36, 112, 148, 192, 448 };
static const sint32 globeradius_2[] = { 18, 56, 74, 96, 224 };

Screen::Screen(Configuration *cfg) {
	config = cfg;

	sdl_surface = NULL;
	surface = NULL;
	scaler = NULL;
	update_rects = NULL;
	shading_data = NULL;
	scaler_index = 0;
	scale_factor = 2;
	fullscreen = false;
	doubleBuffer = false;
	is_no_darkness = false;
	non_square_pixels = false;
	shading_ambient = 255;
	width = 320;
	height = 200;

	std::string str_lighting_style;
	config->value("config/general/lighting", str_lighting_style);

	if (str_lighting_style == "none")
		lighting_style = LIGHTING_STYLE_NONE;
	else if (str_lighting_style == "smooth")
		lighting_style = LIGHTING_STYLE_SMOOTH;
	else
		lighting_style = LIGHTING_STYLE_ORIGINAL;
	old_lighting_style = lighting_style;
	max_update_rects = 10;
	num_update_rects = 0;
	memset(shading_globe, 0, sizeof(shading_globe));
}

Screen::~Screen() {
	delete surface;
	if (update_rects) free(update_rects);
	if (shading_data) free(shading_data);

	for (int i = 0; i < NUM_GLOBES; i++) {
		if (shading_globe[i])
			free(shading_globe[i]);
	}

	SDL_Quit();
}

bool Screen::init() {
	std::string str;

	int new_width, new_height;
	config->value("config/video/screen_width", new_width, 320);
	config->value("config/video/screen_height", new_height, 200);

	if (new_width < 320)
		new_width = 320;

	if (new_height < 200)
		new_height = 200;

	width = (uint16)new_width;
	height = (uint16)new_height;

	/* Initialize the SDL library */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		DEBUG(0, LEVEL_EMERGENCY, "Couldn't initialize SDL: %s\n",
		      SDL_GetError());
		return false;
	}

	update_rects = (SDL_Rect *)malloc(sizeof(SDL_Rect) * max_update_rects);
	if (update_rects == NULL)
		return false;

	config->value("config/video/scale_method", str, "---");
	scaler_index = scaler_reg.GetIndexForName(str);
	if (scaler_index == -1) {
		//config.set("config/video/scale_method","SuperEagle",true);
		scaler_index = scaler_reg.GetIndexForName("SuperEagle");
	}

	config->value("config/video/scale_factor", scale_factor, 1);

	config->value("config/video/fullscreen", fullscreen, false);
	config->value("config/video/non_square_pixels", non_square_pixels, false);

	set_screen_mode();

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderPresent(sdlRenderer);
#endif
	return true;
}

void Screen::set_lighting_style(int lighting) {
	lighting_style = lighting;
	old_lighting_style = lighting;
}

bool Screen::toggle_darkness_cheat() {
	is_no_darkness = !is_no_darkness;
	if (is_no_darkness) {
		old_lighting_style = lighting_style;
		lighting_style = LIGHTING_STYLE_NONE;
	} else
		lighting_style = old_lighting_style;
	return is_no_darkness;
}


bool Screen::set_palette(uint8 *p) {
	if (surface == NULL || p == NULL)
		return false;

//SDL_SetColors(scaled_surface,palette,0,256);
	for (int i = 0; i < 256; ++i) {
		uint32  r = p[i * 3];
		uint32  g = p[i * 3 + 1];
		uint32  b = p[i * 3 + 2];

		uint32  c = ((r >> RenderSurface::Rloss) << RenderSurface::Rshift) | ((g >> RenderSurface::Gloss) << RenderSurface::Gshift) | ((b >> RenderSurface::Bloss) << RenderSurface::Bshift);

		surface->colour32[i] = c;
	}

	return true;
}

bool Screen::set_palette_entry(uint8 idx, uint8 r, uint8 g, uint8 b) {
	if (surface == NULL)
		return false;

	uint32 c = ((((uint32)r) >> RenderSurface::Rloss) << RenderSurface::Rshift) | ((((uint32)g) >> RenderSurface::Gloss) << RenderSurface::Gshift) | ((((uint32)b) >> RenderSurface::Bloss) << RenderSurface::Bshift);

	surface->colour32[idx] = c;

	return true;
}

bool Screen::rotate_palette(uint8 pos, uint8 length) {
	uint32 tmp_colour;
	uint8 i;

	tmp_colour = surface->colour32[pos + length - 1];

	for (i = length - 1; i > 0; i--)
		surface->colour32[pos + i] = surface->colour32[pos + i - 1];

	surface->colour32[pos] = tmp_colour;

	return true;
}

uint16 Screen::get_translated_x(uint16 x) {
	if (scale_factor != 1)
		x /= scale_factor;

	return x;
}

uint16 Screen::get_translated_y(uint16 y) {
	if (scale_factor != 1)
		y /= scale_factor;

	return y;
}
bool Screen::clear(sint16 x, sint16 y, sint16 w, sint16 h, SDL_Rect *clip_rect) {
	uint8 *pixels;
	uint16 i;
	uint16 x1, y1;

	pixels = (uint8 *)surface->pixels;

	if (x >= width || y >= height)
		return false;

	if (x < 0) {
		if (x + w <= 0)
			return false;
		else
			w += x;

		x = 0;
	}

	if (y < 0) {
		if (y + h <= 0)
			return false;
		else
			h += y;

		y = 0;
	}

	if (x + w >= width)
		w = width - x;

	if (y + h >= height)
		h = height - y;

	if (clip_rect) {
		x1 = x;
		y1 = y;
		if (x < clip_rect->x)
			x = clip_rect->x;

		if (y < clip_rect->y)
			y = clip_rect->y;

		if (x1 + w > clip_rect->x + clip_rect->w) {
			w -= (x1 + w) - (clip_rect->x + clip_rect->w);
			if (w <= 0)
				return false;
		}

		if (y1 + h > clip_rect->y + clip_rect->h) {
			h -= (y1 + h) - (clip_rect->y + clip_rect->h);
			if (h <= 0)
				return false;
		}
	}

	pixels += y * surface->pitch + (x * surface->bytes_per_pixel);

	for (i = 0; i < h; i++) {
		memset(pixels, 0, w * surface->bytes_per_pixel);
		pixels += surface->pitch;
	}

	return true;
}

bool Screen::fill(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h) {
	if (x >= surface->w || y >= surface->h) {
		return true;
	}

	if (y + (uint16)h > surface->h) {
		h = surface->h - y;
	}

	if (x + (uint16)w > surface->w) {
		w = surface->w - x;
	}

	if (surface->bits_per_pixel == 16)
		return fill16(colour_num, x, y, w, h);

	return fill32(colour_num, x, y, w, h);
}

bool Screen::fill16(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h) {
	uint16 *pixels;
	uint16 i, j;

	pixels = (uint16 *)surface->pixels;

	pixels += y * surface->w + x;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++)
			pixels[j] = (uint16)surface->colour32[colour_num];

		pixels += surface->w;
	}

	return true;
}

bool Screen::fill32(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h) {
	uint32 *pixels;
	uint16 i, j;


	pixels = (uint32 *)surface->pixels;

	pixels += y * surface->w + x;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++)
			pixels[j] = surface->colour32[colour_num];

		pixels += surface->w;
	}

	return true;
}
void Screen::fade(uint16 dest_x, uint16 dest_y, uint16 src_w, uint16 src_h, uint8 opacity, uint8 fade_bg_color) {
	if (surface->bits_per_pixel == 16)
		fade16(dest_x, dest_y, src_w, src_h, opacity, fade_bg_color);
	else
		fade32(dest_x, dest_y, src_w, src_h, opacity, fade_bg_color);
}

void Screen::fade16(uint16 dest_x, uint16 dest_y, uint16 src_w, uint16 src_h, uint8 opacity, uint8 fade_bg_color) {
	uint16 bg = (uint16)surface->colour32[fade_bg_color];
	uint16 *pixels;
	uint16 i, j;

	pixels = (uint16 *)surface->pixels;

	pixels += dest_y * surface->w + dest_x;

	for (i = 0; i < src_h; i++) {
		for (j = 0; j < src_w; j++) {
			pixels[j] = blendpixel16(bg, pixels[j], opacity);
		}

		pixels += surface->w; //surface->pitch;
	}

	return;
}

void Screen::fade32(uint16 dest_x, uint16 dest_y, uint16 src_w, uint16 src_h, uint8 opacity, uint8 fade_bg_color) {
	uint32 bg = surface->colour32[fade_bg_color];
	uint32 *pixels;
	uint16 i, j;

	pixels = (uint32 *)surface->pixels;

	pixels += dest_y * surface->w + dest_x;

	for (i = 0; i < src_h; i++) {
		for (j = 0; j < src_w; j++) {
			pixels[j] = blendpixel32(bg, pixels[j], opacity);
		}

		pixels += surface->w; //surface->pitch;
	}

	return;
}

void Screen::stipple_8bit(uint8 color_num) {
	stipple_8bit(color_num, 0, 0, surface->w, surface->h);
}

void Screen::stipple_8bit(uint8 color_num, uint16 x, uint16 y, uint16 w, uint16 h) {
	uint32 i, j;

	if (x >= surface->w || y >= surface->h) {
		return;
	}

	if (y + h > surface->h) {
		h = surface->h - y;
	}

	if (x + w > surface->w) {
		w = surface->w - x;
	}

	if (surface->bits_per_pixel == 16) {
		uint16 color = (uint16)surface->colour32[color_num];
		uint16 *pixels = (uint16 *)surface->pixels;

		pixels += y * surface->w + x;

		for (i = y; i < y + h; i++) {
			for (j = x; j < x + w; j += 2) {
				*pixels = color;
				pixels += 2;
			}
			pixels += (surface->w - j) + x;
			if (i % 2) {
				pixels--;
			} else {
				pixels++;
			}
		}
	} else {
		uint32 color = surface->colour32[color_num];
		uint32 *pixels = (uint32 *)surface->pixels;

		pixels += y * surface->w + x;

		for (i = 0; i < h; i++) {
			for (j = x; j < x + w; j += 2) {
				*pixels = color;
				pixels += 2;
			}
			pixels += (surface->w - j) + x;
			if (i % 2) {
				pixels--;
			} else {
				pixels++;
			}
		}
	}
}
void Screen::put_pixel(uint8 colour_num, uint16 x, uint16 y) {
	if (surface->bits_per_pixel == 16) {
		uint16 *pixel = (uint16 *)surface->pixels + y * surface->w + x;
		*pixel = (uint16)surface->colour32[colour_num];
	} else {
		uint32 *pixel = (uint32 *)surface->pixels + y * surface->w + x;
		*pixel = (uint32)surface->colour32[colour_num];
	}
}

void *Screen::get_pixels() {
//if(scaled_surface == NULL)
//   return NULL;

//return scaled_surface->pixels;
	return NULL;
}

SDL_Surface *Screen::get_sdl_surface() {
	if (surface)
		return surface->get_sdl_surface();

	return NULL;
}

bool Screen::blit(sint32 dest_x, sint32 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans, SDL_Rect *clip_rect, uint8 opacity) {
	uint16 src_x = 0;
	uint16 src_y = 0;

// clip to screen.

	if (dest_x >= width || dest_y >= height)
		return false;

	if (dest_x < 0) {
		if (dest_x + src_w <= 0)
			return false;
		else
			src_w += dest_x;

		src_buf += -dest_x;
		dest_x = 0;
	}

	if (dest_y < 0) {
		if (dest_y + src_h <= 0)
			return false;
		else
			src_h += dest_y;

		src_buf += src_pitch * -dest_y;
		dest_y = 0;
	}

	if (dest_x + src_w >= width)
		src_w = width - dest_x;

	if (dest_y + src_h >= height)
		src_h = height - dest_y;

//clip to rect if required.

	if (clip_rect) {
		if (dest_x + src_w < clip_rect->x || dest_y + src_h < clip_rect->y)
			return false;

		if (clip_rect->x > dest_x) {
			src_x = clip_rect->x - dest_x;
			src_w -= src_x;
			dest_x = clip_rect->x;
		}

		if (clip_rect->y > dest_y) {
			src_y = clip_rect->y - dest_y;
			src_h -= src_y;
			dest_y = clip_rect->y;
		}

		if (dest_x + src_w > clip_rect->x + clip_rect->w) {
			if (clip_rect->x + clip_rect->w - dest_x <= 0)
				return false;

			src_w = clip_rect->x + clip_rect->w - dest_x;
		}

		if (dest_y + src_h > clip_rect->y + clip_rect->h) {
			if (clip_rect->y + clip_rect->h - dest_y <= 0)
				return false;

			src_h = clip_rect->y + clip_rect->h - dest_y;
		}

		src_buf += src_y * src_pitch + src_x;
	}

	if (surface->bits_per_pixel == 16) {
		if (opacity < 255)
			return blit16WithOpacity(dest_x, dest_y, src_buf, src_bpp, src_w, src_h, src_pitch, trans, opacity);

		return blit16(dest_x, dest_y, src_buf, src_bpp, src_w, src_h, src_pitch, trans);
	}

	if (opacity < 255)
		return blit32WithOpacity(dest_x, dest_y, src_buf, src_bpp, src_w, src_h, src_pitch, trans, opacity);

	return blit32(dest_x, dest_y, src_buf, src_bpp, src_w, src_h, src_pitch, trans);
}


inline uint16 Screen::blendpixel16(uint16 p, uint16 p1, uint8 opacity) {
	return (((uint8)(((float)((p1 & surface->Rmask) >> surface->Rshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & surface->Rmask) >> surface->Rshift)) * (float)(255 - opacity) / 255.0f)) << surface->Rshift) |     //R
	       (((uint8)(((float)((p1 & surface->Gmask) >> surface->Gshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & surface->Gmask) >> surface->Gshift)) * (float)(255 - opacity) / 255.0f)) << surface->Gshift) |    //G
	       (((uint8)(((float)((p1 & surface->Bmask) >> surface->Bshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & surface->Bmask) >> surface->Bshift)) * (float)(255 - opacity) / 255.0f)) << surface->Bshift);     //B
}

inline uint32 Screen::blendpixel32(uint32 p, uint32 p1, uint8 opacity) {
	return (((uint8)(((float)((p1 & surface->Rmask) >> surface->Rshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & surface->Rmask) >> surface->Rshift)) * (float)(255 - opacity) / 255.0f)) << surface->Rshift) |     //R
	       (((uint8)(((float)((p1 & surface->Gmask) >> surface->Gshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & surface->Gmask) >> surface->Gshift)) * (float)(255 - opacity) / 255.0f)) << surface->Gshift) |    //G
	       (((uint8)(((float)((p1 & surface->Bmask) >> surface->Bshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & surface->Bmask) >> surface->Bshift)) * (float)(255 - opacity) / 255.0f)) << surface->Bshift);     //B
}

inline bool Screen::blit16(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans) {
	uint16 *pixels;
	uint16 i, j;

	pixels = (uint16 *)surface->pixels;

	pixels += dest_y * surface->w + dest_x;

	if (trans) {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				if (src_buf[j] != 0xff)
					pixels[j] = (uint16)surface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += surface->w; //pitch;
		}
	} else {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = (uint16)surface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += surface->w; //surface->pitch;
		}
	}

	return true;
}

inline bool Screen::blit16WithOpacity(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans, uint8 opacity) {
	uint16 *pixels;
	uint16 i, j;

	pixels = (uint16 *)surface->pixels;

	pixels += dest_y * surface->w + dest_x;

	if (trans) {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				if (src_buf[j] != 0xff)
					pixels[j] = blendpixel16(pixels[j], (uint16)surface->colour32[src_buf[j]], opacity);
			}
			src_buf += src_pitch;
			pixels += surface->w; //pitch;
		}
	} else {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = blendpixel16(pixels[j], (uint16)surface->colour32[src_buf[j]], opacity);
			}
			src_buf += src_pitch;
			pixels += surface->w; //surface->pitch;
		}
	}

	return true;
}

bool Screen::blit32(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans) {
	uint32 *pixels;
	uint16 i, j;

	pixels = (uint32 *)surface->pixels;

	pixels += dest_y * surface->w + dest_x;


	if (trans) {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				if (src_buf[j] != 0xff)
					pixels[j] = surface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += surface->w; //pitch;
		}
	} else {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = surface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += surface->w; //surface->pitch;
		}
	}

	return true;
}

bool Screen::blit32WithOpacity(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans, uint8 opacity) {
	uint32 *pixels;
	uint16 i, j;

	pixels = (uint32 *)surface->pixels;

	pixels += dest_y * surface->w + dest_x;


	if (trans) {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				if (src_buf[j] != 0xff)
					pixels[j] = blendpixel32(pixels[j], surface->colour32[src_buf[j]], opacity);
			}
			src_buf += src_pitch;
			pixels += surface->w; //pitch;
		}
	} else {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = blendpixel32(pixels[j], surface->colour32[src_buf[j]], opacity);
			}
			src_buf += src_pitch;
			pixels += surface->w; //surface->pitch;
		}
	}

	return true;
}

void Screen::blitbitmap(uint16 dest_x, uint16 dest_y, const unsigned char *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color) {
	if (surface->bits_per_pixel == 16)
		blitbitmap16(dest_x, dest_y, src_buf, src_w, src_h, fg_color, bg_color);
	else
		blitbitmap32(dest_x, dest_y, src_buf, src_w, src_h, fg_color, bg_color);

	return;
}

void Screen::blitbitmap16(uint16 dest_x, uint16 dest_y, const unsigned char *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color) {
	uint16 *pixels;
	uint16 i, j;

	pixels = (uint16 *)surface->pixels;

	pixels += dest_y * surface->w + dest_x;

	for (i = 0; i < src_h; i++) {
		for (j = 0; j < src_w; j++) {
			if (src_buf[j])
				pixels[j] = (uint16)surface->colour32[fg_color];
			else
				pixels[j] = (uint16)surface->colour32[bg_color];
		}
		src_buf += src_w;
		pixels += surface->w; //surface->pitch;
	}

	return;
}

void Screen::blitbitmap32(uint16 dest_x, uint16 dest_y, const unsigned char *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color) {
	uint32 *pixels;
	uint16 i, j;

	pixels = (uint32 *)surface->pixels;

	pixels += dest_y * surface->w + dest_x;

	for (i = 0; i < src_h; i++) {
		for (j = 0; j < src_w; j++) {
			if (src_buf[j])
				pixels[j] = surface->colour32[fg_color];
			else
				pixels[j] = surface->colour32[bg_color];
		}
		src_buf += src_w;
		pixels += surface->w; //surface->pitch;
	}

	return;
}

//4 is pure-light
//0 is pitch-black
//Globe of r 1 is just a single tile of 2

static const char TileGlobe[][11 * 11] = {
	{
		// 1 - magic items
		1
	},
	{
		// 2- candle, heatsource, fire field, cookfire, stove
		1, 1, 1,
		1, 2, 1,
		1, 1, 1
	},
	{
		// 3 - torch, brazier, campfire, lamppost, candelabra, moongate
		0, 1, 1, 1, 0,
		1, 2, 2, 2, 1,
		1, 2, 3, 2, 1,
		1, 2, 2, 2, 1,
		0, 1, 1, 1, 0
	},
	{
		// 4 - 20:00, eclipse, dungeon
		0, 0, 1, 1, 1, 0, 0,
		0, 1, 2, 2, 2, 1, 0,
		1, 2, 3, 3, 3, 2, 1,
		1, 2, 3, 4, 3, 2, 1,
		1, 2, 3, 3, 3, 2, 1,
		0, 1, 2, 2, 2, 1, 0,
		0, 0, 1, 1, 1, 0, 0
	},
	{
		// 5 - 5:00, 19:50
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 1, 1, 2, 2, 2, 1, 1, 0,
		0, 1, 2, 3, 3, 3, 2, 1, 0,
		1, 2, 3, 4, 4, 4, 3, 2, 1,
		1, 2, 3, 4, 4, 4, 3, 2, 1,
		1, 2, 3, 4, 4, 4, 3, 2, 1,
		0, 1, 2, 3, 3, 3, 2, 1, 0,
		0, 1, 1, 2, 2, 2, 1, 1, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
	},
	{
		// 6 - 5:10, 19:40
		0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0,
		0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0,
		0, 1, 2, 3, 4, 4, 4, 3, 2, 1, 0,
		1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1,
		1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1,
		1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1,
		0, 1, 2, 3, 4, 4, 4, 3, 2, 1, 0,
		0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0,
		0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0
	},
	{
		// 7 - 5:20, 19:30
		0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0,
		0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0,
		1, 2, 3, 3, 4, 4, 4, 3, 3, 2, 1,
		1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1,
		2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2,
		2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2,
		2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2,
		1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1,
		1, 2, 3, 3, 4, 4, 4, 3, 3, 2, 1,
		0, 1, 2, 2, 3, 3, 3, 2, 2, 1, 0,
		0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0
	},
	{
		// 8- 5:30, 19:20, torch equipped, light spell
		2, 2, 2, 2, 3, 3, 3, 2, 2, 2, 2,
		2, 2, 3, 3, 4, 4, 4, 3, 3, 2, 2,
		2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2,
		2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2,
		2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2,
		2, 2, 3, 3, 4, 4, 4, 3, 3, 2, 2,
		2, 2, 2, 2, 3, 3, 3, 2, 2, 2, 2
	},
	{
		// 9 - 5:40, 19:10
		3, 3, 3, 3, 4, 4, 4, 3, 3, 3, 3,
		3, 3, 4, 4, 4, 4, 4, 4, 4, 3, 3,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		3, 3, 4, 4, 4, 4, 4, 4, 4, 3, 3,
		3, 3, 3, 3, 4, 4, 4, 3, 3, 3, 3
	},
	{
		// 10 - 5:50 19:00
		3, 3, 3, 4, 4, 4, 4, 4, 3, 3, 3,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
		3, 3, 3, 4, 4, 4, 4, 4, 3, 3, 3
	}
};

void Screen::clearalphamap8(uint16 x, uint16 y, uint16 w, uint16 h, uint8 opacity, bool party_light_source) {
	switch (lighting_style) {
	default:
	case LIGHTING_STYLE_NONE:
		return;
	case LIGHTING_STYLE_SMOOTH:
		shading_ambient = opacity;
		break;
	case LIGHTING_STYLE_ORIGINAL:
		if (opacity < 0xFF)
			shading_ambient = 0;
		else
			shading_ambient = 0xFF;
		break;
	}

	if (shading_data == NULL) {
		shading_rect.x = x;
		shading_rect.y = y;
		if (lighting_style == LIGHTING_STYLE_ORIGINAL) {
			shading_rect.w = w + (SHADING_BORDER * 2);
			shading_rect.h = h + (SHADING_BORDER * 2);
		} else { // LIGHTING_STYLE_SMOOTH
			shading_rect.w = (w + (SHADING_BORDER * 2)) * 16 + 8;
			shading_rect.h = (h + (SHADING_BORDER * 2)) * 16 + 8;
		}
		shading_data = (unsigned char *)malloc(sizeof(char) * shading_rect.w * shading_rect.h);
		if (shading_data == NULL) {
			/* We couldn't allocate memory for the opacity map, so just disable lighting */
			shading_ambient = 0xFF;
			return;
		}
		buildalphamap8();
	}
	if (shading_ambient == 0xFF) {
	} else {
		memset(shading_data, shading_ambient, sizeof(char)*shading_rect.w * shading_rect.h);
	}
	sint16 x_off;
	if (Game::get_game()->is_original_plus_full_map())
		x_off = - Game::get_game()->get_background()->get_border_width();
	else
		x_off = 0;
	//Light globe around the avatar
	if (lighting_style == LIGHTING_STYLE_ORIGINAL)
		drawalphamap8globe((shading_rect.w - 1 + x_off / 16) / 2 - SHADING_BORDER, (shading_rect.h - 1) / 2 - SHADING_BORDER, opacity / 20 + 4); //range 4 - 10
	else if (lighting_style == LIGHTING_STYLE_SMOOTH)
		drawalphamap8globe((((shading_rect.w - 8 + x_off) / 16) - 1) / 2 - SHADING_BORDER, (((shading_rect.h - 8) / 16) - 1) / 2 - SHADING_BORDER, party_light_source ? 5 : 4);
}

void Screen::buildalphamap8() {
	//Build three globes for 5 intensities
	for (int i = 0; i < NUM_GLOBES; i++) {
		shading_globe[i] = (uint8 *)malloc(sqr(globeradius[i]));
		for (int y = 0; y < globeradius[i]; y++) {
			for (int x = 0; x < globeradius[i]; x++) {
				float r;
				//Distance from center
				r  = sqrtf(sqr((y - globeradius_2[i])) + sqr((x - globeradius_2[i])));
				//Unitize
				r /= sqrtf(sqr(globeradius_2[i]) + sqr(globeradius_2[i]));
				//Calculate brightness
				r  = (float)exp(-(10 * r * r));
				//Fit into a byte
				r *= 255;
				//Place it
				shading_globe[i][y * globeradius[i] + x] = (uint8)r;
			}
		}
	}

	//Get the three shading tiles (for original-style dithered lighting)
	Game *game = Game::get_game();
	int game_type;
	config->value("config/GameType", game_type);

	if (game_type == NUVIE_GAME_U6 || game_type == NUVIE_GAME_SE) {
		shading_tile[0] = game->get_map_window()->get_tile_manager()->get_tile(444)->data;
		shading_tile[1] = game->get_map_window()->get_tile_manager()->get_tile(445)->data;
		shading_tile[2] = game->get_map_window()->get_tile_manager()->get_tile(446)->data;
		shading_tile[3] = game->get_map_window()->get_tile_manager()->get_tile(447)->data;
	} else { //NUVIE_GAME_MD
		shading_tile[0] = game->get_map_window()->get_tile_manager()->get_tile(268)->data;
		shading_tile[1] = game->get_map_window()->get_tile_manager()->get_tile(269)->data;
		shading_tile[2] = game->get_map_window()->get_tile_manager()->get_tile(270)->data;
		shading_tile[3] = game->get_map_window()->get_tile_manager()->get_tile(271)->data;
	}
}


void Screen::drawalphamap8globe(sint16 x, sint16 y, uint16 r) {
	sint16 i, j;
// check shouldn't be needed since items only have 3 intensites
	//Clamp lighting globe size to 0-4 (5 levels) // 4 - 10 (7 levels) now in orig_style now like original
//    if( r > NUM_GLOBES && lighting_style != LIGHTING_STYLE_ORIGINAL)
//        r = NUM_GLOBES;
	if (r < 1)
		return;
	if (shading_ambient == 0xFF)
		return;
	if (lighting_style == LIGHTING_STYLE_NONE)
		return;
	if (lighting_style == LIGHTING_STYLE_ORIGINAL) {
		uint8 rad;
		if (r < 6)
			rad = r - 1;
		else
			rad = 5;
		x += SHADING_BORDER;
		y += SHADING_BORDER;
		//Draw using "original" lighting
		for (j = 0; j <= rad * 2; j++)
			for (i = 0; i <= rad * 2; i++) {
				if (x + i - rad < 0 || x + i - rad >= shading_rect.w)
					continue;
				if (y + j - rad < 0 || y + j - rad >= shading_rect.h)
					continue;
				shading_data[(y + j - rad)*shading_rect.w + (x + i - rad)] = MIN(shading_data[(y + j - rad) * shading_rect.w + (x + i - rad)] + TileGlobe[r - 1][j * (rad * 2 + 1) + i], 4);
			}
		return;
	}
	x = (x + SHADING_BORDER) * 16 + 8;
	y = (y + SHADING_BORDER) * 16 + 8;

	//Draw using "smooth" lighting
	//The x and y are relative to (0,0) of the mapwindow itself, and are absolute coordinates, so are i and j
	r--;
	for (i = -globeradius_2[r]; i < globeradius_2[r]; i++)
		for (j = -globeradius_2[r]; j < globeradius_2[r]; j++) {
			if ((y + i) - 1 < 0 ||
			        (x + j) - 1 < 0 ||
			        (y + i) + 1 > shading_rect.h ||
			        (x + j) + 1 > shading_rect.w)
				continue;
			shading_data[(y + i)*shading_rect.w + (x + j)] = MIN(shading_data[(y + i) * shading_rect.w + (x + j)] + shading_globe[r][(i + globeradius_2[r]) * globeradius[r] + (j + globeradius_2[r])], 255);
		}
}


void Screen::blitalphamap8(sint16 x, sint16 y, SDL_Rect *clip_rect) {
	//pixel = (dst*(1-alpha))+(src*alpha)   for an interpolation
	//pixel = pixel * alpha                 for a reduction
	//We use a reduction here

	if (shading_ambient == 0xFF)
		return;
	if (lighting_style == LIGHTING_STYLE_NONE)
		return;

	uint16 i, j;
	Game *game = Game::get_game();

	if (lighting_style == LIGHTING_STYLE_ORIGINAL) {

		for (j = SHADING_BORDER; j < shading_rect.h - SHADING_BORDER; j++) {
			for (i = SHADING_BORDER; i < shading_rect.w - SHADING_BORDER; i++) {
				if (shading_data[j * shading_rect.w + i] < 4)
					blit(x + (i - SHADING_BORDER) * 16, y + (j - SHADING_BORDER) * 16, shading_tile[shading_data[j * shading_rect.w + i]], 8, 16, 16, 16, true, game->get_map_window()->get_clip_rect());
			}
		}
		return;
	}

	uint16 src_w = shading_rect.w - (SHADING_BORDER * 2 * 16);
	uint16 src_h = shading_rect.h - (SHADING_BORDER * 2 * 16);

	uint16 src_x = SHADING_BORDER * 16;
	uint16 src_y = SHADING_BORDER * 16;

	uint8 *src_buf = shading_data;

	// clip to screen.

	//if(x >= width || y >= height)
	//  return;

	if (x < 0) {
		//if(x + src_w <= 0)
		//  return;
		//else
		src_w += x;

		src_buf += -x;
		x = 0;
	}

	if (y < 0) {
		//if(y + src_h <= 0)
		//  return;
		//else
		src_h += y;

		src_buf += shading_rect.w * -y;
		y = 0;
	}

	if (x + src_w >= width)
		src_w = width - x;

	if (y + src_h >= height)
		src_h = height - y;

	//clip to rect if required.

	if (clip_rect) {
		//if(x + src_w < clip_rect->x || y + src_h < clip_rect->y)
		//   return;

		if (clip_rect->x > x) {
			src_x += (clip_rect->x - x);
			src_w -= (clip_rect->x - x);
			x = clip_rect->x;
		}

		if (clip_rect->y > y) {
			src_y += (clip_rect->y - y);
			src_h -= (clip_rect->y - y);
			y = clip_rect->y;
		}

		if (x + src_w > clip_rect->x + clip_rect->w) {
			//if(clip_rect->x + clip_rect->w - x <= 0)
			//  return;

			src_w = clip_rect->x + clip_rect->w - x;
		}

		if (y + src_h > clip_rect->y + clip_rect->h) {
			//if(clip_rect->y + clip_rect->h - y <= 0)
			//  return;

			src_h = clip_rect->y + clip_rect->h - y;
		}

		src_buf += src_y * shading_rect.w + src_x;
	}


	switch (surface->bits_per_pixel) {
	case 16:
		uint16 *pixels16;
		pixels16 = (uint16 *)surface->pixels;

		pixels16 += y * surface->w + x;

		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels16[j] = (((unsigned char)(((float)((pixels16[j] & surface->Rmask) >> surface->Rshift)) * (float)(src_buf[j]) / 255.0f)) << surface->Rshift) |      //R
				              (((unsigned char)(((float)((pixels16[j] & surface->Gmask) >> surface->Gshift)) * (float)(src_buf[j]) / 255.0f)) << surface->Gshift) |      //G
				              (((unsigned char)(((float)((pixels16[j] & surface->Bmask) >> surface->Bshift)) * (float)(src_buf[j]) / 255.0f)) << surface->Bshift);       //B
			}
			pixels16 += surface->w;
			src_buf += shading_rect.w;
		}
		return;
		break;
	case 24:
	case 32:
		uint32 *pixels;
		pixels = (uint32 *)surface->pixels;

		pixels += y * surface->w + x;

		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = (((unsigned char)(((float)((pixels[j] & surface->Rmask) >> surface->Rshift)) * (float)(src_buf[j]) / 255.0f)) << surface->Rshift) |      //R
				            (((unsigned char)(((float)((pixels[j] & surface->Gmask) >> surface->Gshift)) * (float)(src_buf[j]) / 255.0f)) << surface->Gshift) |      //G
				            (((unsigned char)(((float)((pixels[j] & surface->Bmask) >> surface->Bshift)) * (float)(src_buf[j]) / 255.0f)) << surface->Bshift);       //B
			}
			pixels += surface->w;
			src_buf += shading_rect.w;
		}
		return;
		break;
	default:
		DEBUG(0, LEVEL_ERROR, "Screen::blitalphamap8() cannot handle your screen surface depth of %d\n", surface->bits_per_pixel);
		break;
		return;
	}

}


/* Return an 8bit surface. Source format is assumed to be identical to screen. */
SDL_Surface *Screen::create_sdl_surface_8(unsigned char *src_buf, uint16 src_w, uint16 src_h) {
	SDL_Surface *new_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, src_w, src_h, 8, 0, 0, 0, 0);
	unsigned char *pixels = (unsigned char *)new_surface->pixels;

	if (surface->bits_per_pixel == 16) {
		uint16 *src = (uint16 *)src_buf;
		for (int p = 0; p < (src_w * src_h); p++)
			for (int i = 0; i < 256; i++) // convert to 8bpp
				if (src[p] == (uint16)surface->colour32[i]) {
					pixels[p] = i;
					break;
				}
	} else {
		uint32 *src = (uint32 *)src_buf;
		for (int p = 0; p < (src_w * src_h); p++)
			for (int i = 0; i < 256; i++)
				if (src[p] == surface->colour32[i]) {
					pixels[p] = i;
					break;
				}
	}
	return (new_surface);
}


SDL_Surface *Screen::create_sdl_surface_from(unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch) {
	SDL_Surface *new_surface;
	uint16 i, j;

	new_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, src_w, src_h, surface->bits_per_pixel,
	                                   surface->Rmask, surface->Gmask, surface->Bmask, 0);

	if (surface->bits_per_pixel == 16) {
		uint16 *pixels = (uint16 *)new_surface->pixels;

		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = (uint16)surface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += src_pitch; //surface->pitch;
		}
	} else {
		uint32 *pixels = (uint32 *)new_surface->pixels;

		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = surface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += src_w;
		}
	}

	return new_surface;

}

uint16 Screen::get_pitch() {
	return (uint16)surface->pitch;
}

uint16 Screen::get_bpp() {
	return surface->bits_per_pixel;
}

void Screen::update() {
	if (scaler) {
		scaler->Scale(surface->format_type, surface->pixels,     // type, source
		              0, 0, surface->w, surface->h,                          // x, y, w, h
		              surface->pitch / surface->bytes_per_pixel, surface->h, // pixels/line, pixels/col
		              sdl_surface->pixels,                                   // dest
		              sdl_surface->pitch / sdl_surface->format->BytesPerPixel, // destpixels/line
		              scale_factor);
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_UpdateTexture(sdlTexture, NULL, sdl_surface->pixels, sdl_surface->pitch);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
#else
	SDL_UpdateRect(sdl_surface, 0, 0, 0, 0);
#endif

	return;
}


void Screen::update(sint32 x, sint32 y, uint16 w, uint16 h) {
//if(scaled_surface)
	/*
	 if(x >= 2)
	   x -= 2;
	 if(y >= 2)
	   y -= 2;
	 if(w <= surface->w-2)
	   w += 2;
	 if(h <= surface->h-2)
	   h += 2;
	*/

	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > width)
		return;
	if (y > height)
		return;
	if ((x + w) > width) w = width - x;
	if ((y + h) > height) h = height - y;

	if (scaler) {
		scaler->Scale(surface->format_type, surface->pixels,     // type, source
		              x, y, w, h,                            // x, y, w, h
		              surface->pitch / surface->bytes_per_pixel, surface->h, // pixels/line, pixels/col
		              sdl_surface->pixels,                                   // dest
		              sdl_surface->pitch / sdl_surface->format->BytesPerPixel, // destpixels/line
		              scale_factor);
	}

	if (num_update_rects == max_update_rects) {
		update_rects = (SDL_Rect *)nuvie_realloc(update_rects, sizeof(SDL_Rect) * (max_update_rects + 10));
		max_update_rects += 10;
	}

	update_rects[num_update_rects].x = x * scale_factor;
	update_rects[num_update_rects].y = y * scale_factor;
	update_rects[num_update_rects].w = w * scale_factor;
	update_rects[num_update_rects].h = h * scale_factor;

	num_update_rects++;

//SDL_UpdateRect(sdl_surface,x*scale_factor,y*scale_factor,w*scale_factor,h*scale_factor);

//DEBUG(0,LEVEL_DEBUGGING,"update rect(%d,%d::%d,%d)\n", update_rects[num_update_rects].x, update_rects[num_update_rects].y, update_rects[num_update_rects].w, update_rects[num_update_rects].h);

	return;
}

void Screen::preformUpdate() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_UpdateTexture(sdlTexture, NULL, sdl_surface->pixels, sdl_surface->pitch);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
#else
	SDL_UpdateRects(sdl_surface, num_update_rects, update_rects);
#endif
	num_update_rects = 0;
}

void Screen::lock() {
// SDL_LockSurface(scaled_surface);

	return;
}

void Screen::unlock() {
//SDL_UnlockSurface(scaled_surface);

	return;
}

bool Screen::initScaler() {
	std::string scaler_name;

	return true;
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
bool Screen::SDL_VideoModeOK(int scaled_width, int scaled_height, int bpp, int flags) {
	return (bpp == get_screen_bpp());
}
#endif

int Screen::get_screen_bpp() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_DisplayMode mode;
	if (SDL_GetDisplayMode(0, 0, &mode) != 0) {
		return 0;
	}

	return SDL_BITSPERPIXEL(mode.format);
#else
	// Get info. about video.
	const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
	return vinfo->vfmt->BitsPerPixel;
#endif
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
bool Screen::init_sdl2_window(uint16 scale) {
	uint32 win_width = width;
	uint32 win_height = height;

	window_scale_w = (float)scale;
	window_scale_h = (float)scale;

	if (non_square_pixels)
		window_scale_h *= 1.2;

	SDL_CreateWindowAndRenderer(width * window_scale_w, (int)(height * window_scale_h), SDL_WINDOW_SHOWN, &sdlWindow, &sdlRenderer);
	if (sdlWindow == NULL || sdlRenderer == NULL)
		return false;

	SDL_SetWindowTitle(sdlWindow, "Nuvie");
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(sdlRenderer, width * window_scale_w, (int)(height * window_scale_h)); //VGA non-square pixels.

	set_fullscreen(fullscreen);

	return true;
}

bool Screen::create_sdl_surface_and_texture(sint32 w, sint32 h, Uint32 format) {
	uint32 rmask, gmask, bmask, amask;
	int bpp;

	if (!SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask))
		return false;

	sdl_surface = SDL_CreateRGBSurface(0, w, h, bpp,
	                                   rmask,
	                                   gmask,
	                                   bmask,
	                                   amask);

	if (sdl_surface == NULL) {
		fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
		return false;
	}

	sdlTexture = SDL_CreateTexture(sdlRenderer,
	                               format,
	                               SDL_TEXTUREACCESS_STREAMING,
	                               w, h);

	if (sdlTexture == NULL) {
		SDL_FreeSurface(sdl_surface);
		sdl_surface = NULL;
		fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

#endif

void Screen::set_screen_mode() {
	uint32 flags = 0;

	if (scale_factor == 0) scale_factor = 1;
	scaler = 0;

	// Get BPP
	int bpp = get_screen_bpp();

	// If we can't use the format, force 16 bit
	if (bpp != 16 && bpp != 32) {
		DEBUG(0, LEVEL_DEBUGGING, "BPP %d selected. Using 16 instead.", bpp);
		bpp = 16;
	}



	DEBUG(0, LEVEL_DEBUGGING, "Attempting to set vid mode: %dx%dx%dx%d", width, height, bpp, scale_factor);
#if !SDL_VERSION_ATLEAST(2, 0, 0)
	// Get info. about video.
	const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();

	// Is Fullscreen?
	if (fullscreen) {
		flags |= SDL_FULLSCREEN;
		DEBUG(1, LEVEL_DEBUGGING, " Fullscreen");
	}
	if (vinfo->hw_available && doubleBuffer && fullscreen) {
		flags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
		DEBUG(1, LEVEL_DEBUGGING, " Hardware Double Buffered\n");
	} else {
		flags |= SDL_SWSURFACE;
		DEBUG(1, LEVEL_DEBUGGING, " Software Surface\n");
	}
#endif
	// Old Software rendering. Try a scaler_index first,
	if (!try_scaler(width, height, flags, bpp)) {

		scaler = 0;
#if SDL_VERSION_ATLEAST(2, 0, 0)
		init_sdl2_window(scale_factor);
		scale_factor = 1;
		if (bpp == 32) {
			if (!create_sdl_surface_and_texture(width, height, SDL_PIXELFORMAT_ARGB8888))
				exit(EXIT_FAILURE);
		} else {
			if (!create_sdl_surface_and_texture(width, height, SDL_PIXELFORMAT_RGB565))
				exit(EXIT_FAILURE);
		}

		surface = CreateRenderSurface(sdl_surface);
#else
		scale_factor = 1;
		sdl_surface = SDL_SetVideoMode(width, height, bpp, flags);

		// Couldn't create it, so disable double buffering
		if (!sdl_surface) {
			flags &= ~(SDL_HWSURFACE | SDL_DOUBLEBUF);
			flags |= SDL_SWSURFACE;
			scale_factor = 1;
			scaler = 0;
			sdl_surface = SDL_SetVideoMode(width, height, bpp, flags);
		}

		if (!sdl_surface) {
			DEBUG(0, LEVEL_NOTIFICATION, "Unable to create surface. Attempting 320x200x1 Software Surface\n");
			width = 320;
			height = 200;
			sdl_surface = SDL_SetVideoMode(width, height, bpp, flags);
			if (!sdl_surface) {
				DEBUG(0, LEVEL_EMERGENCY, "Unable to create surface. Exiting\n");
				exit(-1);
			}
		}

		if (sdl_surface->flags & SDL_HWSURFACE) {
			DEBUG(0, LEVEL_DEBUGGING, "Created Double Buffered Surface\n");
			surface = CreateRenderSurface(width, height, bpp);
		} else {
			surface = CreateRenderSurface(sdl_surface);
		}
#endif
	}

	surface->set_format(sdl_surface->format);


//	if (zbuffer) screen->create_zbuffer();
}

bool Screen::toggle_fullscreen() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	return set_fullscreen(!fullscreen);
#else
	return sdl1_toggle_fullscreen();
#endif
}

#if !SDL_VERSION_ATLEAST(2, 0, 0)
bool Screen::sdl1_toggle_fullscreen() {
	uint32 flags = sdl_surface->flags;
	int scaled_height = height * scale_factor;
	int scaled_width = width * scale_factor;
	if (fullscreen) {
		flags &= ~(SDL_HWSURFACE | SDL_DOUBLEBUF);
		flags &= ~SDL_FULLSCREEN;
		flags |= SDL_SWSURFACE;
	} else {
		flags |= SDL_FULLSCREEN;
		if (SDL_GetVideoInfo()->hw_available && doubleBuffer)
			flags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
	}
	uint8 bpp = get_sdl_surface()->format->BitsPerPixel;

	if (!SDL_VideoModeOK(scaled_width, scaled_height, bpp, flags)) {
		if (!fullscreen) { // try software
			flags &= ~(SDL_HWSURFACE | SDL_DOUBLEBUF);
			flags |= SDL_SWSURFACE;
		}
		if (!SDL_VideoModeOK(scaled_width, scaled_height, bpp, flags)) {
			DEBUG(0, LEVEL_ERROR, "Couldn't toggle to %\n", fullscreen ? "a window" : "fullscreen");
			return false;
		}
	}
	fullscreen = !fullscreen;
	return SDL_SetVideoMode(scaled_width, scaled_height, bpp, flags);
}
#endif

bool Screen::set_fullscreen(bool value) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	fullscreen = value;
	Uint32 windowFlags = SDL_GetWindowFlags(sdlWindow);

	if (fullscreen)
		windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	else if ((windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
		windowFlags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;

	if (SDL_SetWindowFullscreen(sdlWindow, windowFlags) < 0) {
		DEBUG(0, LEVEL_NOTIFICATION, "error toggling fullscreen mode %s\n", SDL_GetError());
		return false;
	}
#ifdef WIN32
	if (!fullscreen) {
		SDL_SetWindowSize(sdlWindow, (int)(width * window_scale_w), (int)(height * window_scale_h));
	}
#endif
	return true;
#else
	return false;
#endif
}

bool Screen::try_scaler(int w, int h, uint32 flags, int hwdepth) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (scaler_index == 0) //point scaler
		return false;
#endif
	// Try the universal scalers
	if (scale_factor > 1 && scaler_index >= 0) {
		int scaled_height = h * scale_factor;
		int scaled_width = w * scale_factor;
		scaler = scaler_reg.GetScaler(scaler_index);

		// If the scaler wasn't found, use the Point scaler
		if (!scaler) {
			DEBUG(0, LEVEL_NOTIFICATION, "Couldn't find scaler for scaler_index %d\n", scaler_index);
			scaler = scaler_reg.GetPointScaler();
		}
		// If the scaler selected is 2x only, and we are in a > than 2x mode, use Point
		else if (scale_factor > 2 && (scaler->flags & SCALER_FLAG_2X_ONLY)) {
			DEBUG(0, LEVEL_NOTIFICATION, "Scaler %s only supports 2x. %dx requested\n", scaler->name, scale_factor);
			scaler = scaler_reg.GetPointScaler();
		}
		// If it requires 16 bit, force that. However, if it fails use point
		else if (scaler->flags & SCALER_FLAG_16BIT_ONLY) {
			if (!SDL_VideoModeOK(scaled_width, scaled_height, 16, flags)) {
				DEBUG(0, LEVEL_NOTIFICATION, "%s requires 16 bit colour. Couldn't set mode.\n", scaler->name);
				scaler = scaler_reg.GetPointScaler();
			} else if (hwdepth != 16) {
				DEBUG(0, LEVEL_NOTIFICATION, "%s requires 16 bit colour. Forcing.\n", scaler->name);
				hwdepth = 16;
			}
		}
		// If it requires 32 bit, force that. However, if it fails use point
		else if (scaler->flags & SCALER_FLAG_32BIT_ONLY) {
			if (!SDL_VideoModeOK(scaled_width, scaled_height, 32, flags)) {
				DEBUG(0, LEVEL_NOTIFICATION, "%s requires 32 bit colour. Couldn't set mode.\n", scaler->name);
				scaler = scaler_reg.GetPointScaler();
			} else if (hwdepth != 32) {
				DEBUG(0, LEVEL_NOTIFICATION, "%s requires 32 bit colour. Forcing.\n", scaler->name);
				hwdepth = 32;
			}
		}

		DEBUG(0, LEVEL_NOTIFICATION, "Using scaler: %s\n", scaler->name);

		// Attempt to set Video mode
		if (!SDL_VideoModeOK(scaled_width, scaled_height, hwdepth, flags)) {
			hwdepth = 0;

			// Try 32 bit (but only if allowed)
			if (SDL_VideoModeOK(scaled_width, scaled_height, 32, flags) && !(scaler->flags & SCALER_FLAG_16BIT_ONLY))
				hwdepth = 32;
			// Try 16 bit (but only if allowed)
			else if (SDL_VideoModeOK(scaled_width, scaled_height, 16, flags) && !(scaler->flags & SCALER_FLAG_32BIT_ONLY))
				hwdepth = 16;
		}

		// Oh no, it didn't work
		if (hwdepth != 16 && hwdepth != 32) {
			DEBUG(0, LEVEL_NOTIFICATION, "%s requires 16/32 bit colour. Couldn't set mode.\n", scaler->name);
		} else {
#if SDL_VERSION_ATLEAST(2, 0, 0)
			init_sdl2_window(scale_factor);
			if (hwdepth == 32) {
				if (!create_sdl_surface_and_texture(scaled_width, scaled_height, SDL_PIXELFORMAT_ARGB8888))
					return false;
			} else {
				if (!create_sdl_surface_and_texture(scaled_width, scaled_height, SDL_PIXELFORMAT_RGB565))
					return false;
			}
			surface = CreateRenderSurface(w, h, hwdepth);
			if (surface) {
				return true;
			}
#else
			if ((sdl_surface = SDL_SetVideoMode(scaled_width, scaled_height, hwdepth, flags))) {
				/* Create render surface */
				surface = CreateRenderSurface(w, h, hwdepth);
				return true;
			}
#endif
		}

		// Output that scaled surface creation failed
		DEBUG(0, LEVEL_ERROR, "Couldn't create %s scaled surface\n", scaler->name);
		delete surface;
		scaler = 0;
		surface = 0;
		sdl_surface = 0;
		return false;
	}

	return false;
}

//Note! assumes area divides evenly by down_scale factor
unsigned char *Screen::copy_area(SDL_Rect *area, uint16 down_scale) {
	if (surface->bits_per_pixel == 16)
		return (copy_area16(area, down_scale));

	return (copy_area32(area, down_scale));
}

unsigned char *Screen::copy_area16(SDL_Rect *area, uint16 down_scale) {
	SDL_PixelFormat *fmt;
	SDL_Surface *main_surface = get_sdl_surface();
	unsigned char *dst_pixels = NULL;
	unsigned char *ptr;
	uint16 *src_pixels;
	uint32 r, g, b;
	uint32 ra, ga, ba;
	uint16 x, y;
	uint8 x1, y1;

	dst_pixels = new unsigned char[((area->w / down_scale) * (area->h / down_scale)) * 3];
	ptr = dst_pixels;

	fmt = main_surface->format;

	for (y = 0; y < area->h; y += down_scale) {
		for (x = 0; x < area->w; x += down_scale) {
			r = 0;
			g = 0;
			b = 0;

			src_pixels = (uint16 *)main_surface->pixels;
			src_pixels += ((area->y + y) * surface->w + (area->x + x));

			for (y1 = 0; y1 < down_scale; y1++) {
				for (x1 = 0; x1 < down_scale; x1++) {
					ra = *src_pixels & fmt->Rmask;
					ra >>= fmt->Rshift;
					ra <<= fmt->Rloss;

					ga = *src_pixels & fmt->Gmask;
					ga >>= fmt->Gshift;
					ga <<= fmt->Gloss;

					ba = *src_pixels & fmt->Bmask;
					ba >>= fmt->Bshift;
					ba <<= fmt->Bloss;

					r += ra;
					g += ga;
					b += ba;

					src_pixels++;
				}
				src_pixels += surface->w;
			}

			ptr[0] = (uint8)(r / (down_scale * down_scale));
			ptr[1] = (uint8)(g / (down_scale * down_scale));
			ptr[2] = (uint8)(b / (down_scale * down_scale));
			ptr += 3;
		}
	}

	return dst_pixels;
}

unsigned char *Screen::copy_area32(SDL_Rect *area, uint16 down_scale) {
	SDL_PixelFormat *fmt;
	SDL_Surface *main_surface = get_sdl_surface();
	unsigned char *dst_pixels = NULL;
	unsigned char *ptr;
	uint32 *src_pixels;
	uint32 r, g, b;
	uint32 ra, ga, ba;
	uint16 x, y;
	uint8 x1, y1;

	dst_pixels = new unsigned char[((area->w / down_scale) * (area->h / down_scale)) * 3];
	ptr = dst_pixels;

	fmt = main_surface->format;

	for (y = 0; y < area->h; y += down_scale) {
		for (x = 0; x < area->w; x += down_scale) {
			r = 0;
			g = 0;
			b = 0;

			src_pixels = (uint32 *)main_surface->pixels;
			src_pixels += ((area->y + y) * surface->w + (area->x + x));

			for (y1 = 0; y1 < down_scale; y1++) {
				for (x1 = 0; x1 < down_scale; x1++) {
					ra = *src_pixels & fmt->Rmask;
					ra >>= fmt->Rshift;
					ra <<= fmt->Rloss;

					ga = *src_pixels & fmt->Gmask;
					ga >>= fmt->Gshift;
					ga <<= fmt->Gloss;

					ba = *src_pixels & fmt->Bmask;
					ba >>= fmt->Bshift;
					ba <<= fmt->Bloss;

					r += ra;
					g += ga;
					b += ba;

					src_pixels++;
				}
				src_pixels += surface->w;
			}

			ptr[0] = (uint8)(r / (down_scale * down_scale));
			ptr[1] = (uint8)(g / (down_scale * down_scale));
			ptr[2] = (uint8)(b / (down_scale * down_scale));
			ptr += 3;
		}
	}

	return dst_pixels;
}

// surface -> unsigned char *
// (NULL area = entire screen)
unsigned char *Screen::copy_area(SDL_Rect *area, unsigned char *buf) {
	SDL_Rect screen_area = { 0, 0, (uint16)surface->w, (uint16)surface->h };
	if (!area)
		area = &screen_area;

	if (surface->bits_per_pixel == 16)
		return (copy_area16(area, buf));
	return (copy_area32(area, buf));
}


// unsigned char * -> surface
// unsigned char * -> target (src area still means location on screen, not relative to target)
// (NULL area = entire screen)
void Screen::restore_area(unsigned char *pixels, SDL_Rect *area,
                          unsigned char *target, SDL_Rect *target_area, bool free_src) {
	SDL_Rect screen_area = { 0, 0, (Uint16)surface->w, (Uint16)surface->h };
	if (!area)
		area = &screen_area;

	if (surface->bits_per_pixel == 16)
		restore_area16(pixels, area, target, target_area, free_src);
	else
		restore_area32(pixels, area, target, target_area, free_src);
}


unsigned char *Screen::copy_area32(SDL_Rect *area, unsigned char *buf) {
	uint32 *copied = (uint32 *)buf;
	if (buf == NULL) {
		copied = (uint32 *)malloc(area->w * area->h * 4);
	}
	uint32 *dest = copied;
	uint32 *src = (uint32 *)surface->pixels;
	uint16 src_x_off = abs(area->x);
	uint16 src_y_off = abs(area->y);
	uint16 src_w = area->w;
	uint16 src_h = area->h;

	if (area->x < 0) {
		src_x_off = 0;
		src_w += area->x;
		dest += abs(area->x);
	}

	if (area->y < 0) {
		src_y_off = 0;
		src_h += area->y;
		dest += (area->w * abs(area->y));
	}

	if (src_x_off + src_w > surface->w) {
		src_w -= ((src_x_off + src_w) - surface->w);
	}

	if (src_y_off + src_h > surface->h) {
		src_h -= ((src_y_off + src_h) - surface->h);
	}

	src += src_y_off * surface->w + src_x_off;

	for (uint32 i = 0; i < src_h; i++) {
		for (uint32 j = 0; j < src_w; j++)
			dest[j] = src[j];
		dest += area->w;
		src += surface->w;
	}
	return ((unsigned char *)copied);
}


void Screen::restore_area32(unsigned char *pixels, SDL_Rect *area,
                            unsigned char *target, SDL_Rect *target_area, bool free_src) {
	uint32 *src = (uint32 *)pixels;
	uint32 *dest = (uint32 *)surface->pixels;
	dest += area->y * surface->w + area->x;
	if (target) { // restore to target instead of screen
		dest = (uint32 *)target;
		dest += (area->y - target_area->y) * target_area->w + (area->x - target_area->x);
	}
	for (uint32 i = 0; i < area->h; i++) {
		for (uint32 j = 0; j < area->w; j++)
			dest[j] = src[j];
		src += area->w;
		dest += target ? target_area->w : surface->w;
	}
	if (free_src) {
		free(pixels);
	}
}


unsigned char *Screen::copy_area16(SDL_Rect *area, unsigned char *buf) {
	uint16 *copied = (uint16 *)buf;
	if (buf == NULL) {
		copied = (uint16 *)malloc(area->w * area->h * 2);
	}
	uint16 *dest = copied;
	uint16 *src = (uint16 *)surface->pixels;
	uint16 src_x_off = abs(area->x);
	uint16 src_y_off = abs(area->y);
	uint16 src_w = area->w;
	uint16 src_h = area->h;

	if (area->x < 0) {
		src_x_off = 0;
		src_w += area->x;
		dest += abs(area->x);
	}

	if (area->y < 0) {
		src_y_off = 0;
		src_h += area->y;
		dest += (area->w * abs(area->y));
	}

	if (src_x_off + src_w > surface->w) {
		src_w -= ((src_x_off + src_w) - surface->w);
	}

	if (src_y_off + src_h > surface->h) {
		src_h -= ((src_y_off + src_h) - surface->h);
	}

	src += src_y_off * surface->w + src_x_off;

	for (uint32 i = 0; i < src_h; i++) {
		for (uint32 j = 0; j < src_w; j++)
			dest[j] = src[j];
		dest += area->w;
		src += surface->w;
	}
	return ((unsigned char *)copied);
}


void Screen::restore_area16(unsigned char *pixels, SDL_Rect *area,
                            unsigned char *target, SDL_Rect *target_area, bool free_src) {
	uint16 *src = (uint16 *)pixels;
	uint16 *dest = (uint16 *)surface->pixels;
	dest += area->y * surface->w + area->x;
	if (target) { // restore to target instead of screen
		dest = (uint16 *)target;
		dest += (area->y - target_area->y) * target_area->w + (area->x - target_area->x);
	}

	for (uint32 i = 0; i < area->h; i++) {
		for (uint32 j = 0; j < area->w; j++)
			dest[j] = src[j];
		src += area->w;
		dest += target ? target_area->w : surface->w;
	}
	if (free_src) {
		free(pixels);
	}
}

void Screen::draw_line(int sx, int sy, int ex, int ey, uint8 color) {
	if (surface == NULL)
		return;

	surface->draw_line(sx, sy, ex, ey, color);

	return;
}


void Screen::get_mouse_location(sint32 *x, sint32 *y) {
	SDL_GetMouseState(x, y);
#if SDL_VERSION_ATLEAST(2, 0, 0)
	scale_sdl_window_coords(x, y);
#else
	if (scale_factor != 1) {
		*x /= scale_factor;
		*y /= scale_factor;
	}
#endif
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
void Screen::scale_sdl_window_coords(sint32 *mx, sint32 *my) {
	if (fullscreen) {
		float sx, sy;
		SDL_RenderGetScale(sdlRenderer, &sx, &sy);

		SDL_Rect viewport;
		SDL_RenderGetViewport(sdlRenderer, &viewport);

		*mx = *mx - (sint32)((float)viewport.x * sx);

		sx = ((float)viewport.w / width) * sx;
		sy = ((float)viewport.h / height) * sy;

		*mx = (sint32)((float) * mx / sx) ;
		*my = (sint32)((float) * my / sy) ;
	} else {
		sint32 w, h;
		SDL_RenderGetLogicalSize(sdlRenderer, &w, &h);

		w = w / width;
		h = h / height;

		*mx = (sint32)((float) * mx / window_scale_w);
		*my = (sint32)((float) * my / window_scale_h);
	}
}
#endif

void Screen::set_non_square_pixels(bool value) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (value == non_square_pixels)
		return;

	non_square_pixels = value;

	if (non_square_pixels)
		window_scale_h *= 1.2;
	else
		window_scale_h /= 1.2;

	int sw = (int)(width * window_scale_w);
	int sh = (int)(height * window_scale_h);

	SDL_RenderSetLogicalSize(sdlRenderer, sw, sh); //VGA non-square pixels.
	SDL_SetWindowSize(sdlWindow, sw, sh);
#endif
}

} // End of namespace Ultima6
} // End of namespace Ultima
