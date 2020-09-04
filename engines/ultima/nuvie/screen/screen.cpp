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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/screen/surface.h"
#include "ultima/nuvie/screen/scale.h"
#include "ultima/nuvie/screen/screen.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/gui/widgets/background.h"
#include "common/system.h"
#include "engines/util.h"

namespace Ultima {
namespace Nuvie {

#define sqr(a) ((a)*(a))

//Ultima 6 light globe sizes.
#define NUM_GLOBES 5
#define SHADING_BORDER 2 // should be the same as MapWindow's TMP_MAP_BORDER
static const sint32 globeradius[]   = { 36, 112, 148, 192, 448 };
static const sint32 globeradius_2[] = { 18, 56, 74, 96, 224 };

Screen::Screen(Configuration *cfg) {
	config = cfg;

	_rawSurface = NULL;
	_renderSurface = NULL;
	scaler = NULL;
	shading_data = NULL;
	scaler_index = 0;
	scale_factor = 2;
	doubleBuffer = false;
	is_no_darkness = false;
	non_square_pixels = false;
	shading_ambient = 255;
	width = 320;
	height = 200;

	Std::string str_lighting_style;
	config->value("config/general/lighting", str_lighting_style);

	if (str_lighting_style == "none")
		lighting_style = LIGHTING_STYLE_NONE;
	else if (str_lighting_style == "smooth")
		lighting_style = LIGHTING_STYLE_SMOOTH;
	else
		lighting_style = LIGHTING_STYLE_ORIGINAL;
	old_lighting_style = lighting_style;
	memset(shading_globe, 0, sizeof(shading_globe));
}

Screen::~Screen() {
	delete _renderSurface;
	delete _rawSurface;

	if (shading_data)
		free(shading_data);

	for (int i = 0; i < NUM_GLOBES; i++) {
		if (shading_globe[i])
			free(shading_globe[i]);
	}
}

bool Screen::init() {
	Std::string str;

	int new_width, new_height;
	config->value("config/video/screen_width", new_width, 320);
	config->value("config/video/screen_height", new_height, 200);

	if (new_width < 320)
		new_width = 320;

	if (new_height < 200)
		new_height = 200;

	width = (uint16)new_width;
	height = (uint16)new_height;

	config->value("config/video/scale_method", str, "---");
	scaler_index = scaler_reg.GetIndexForName(str);
	if (scaler_index == -1) {
		//config.set("config/video/scale_method","SuperEagle",true);
		scaler_index = scaler_reg.GetIndexForName("SuperEagle");
	}

	config->value("config/video/scale_factor", scale_factor, 1);

	config->value("config/video/non_square_pixels", non_square_pixels, false);

	set_screen_mode();

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
	if (_renderSurface == NULL || p == NULL)
		return false;

//SDL_SetColors(scaled_surface,palette,0,256);
	for (int i = 0; i < 256; ++i) {
		uint32  r = p[i * 3];
		uint32  g = p[i * 3 + 1];
		uint32  b = p[i * 3 + 2];

		uint32  c = ((r >> RenderSurface::Rloss) << RenderSurface::Rshift) | ((g >> RenderSurface::Gloss) << RenderSurface::Gshift) | ((b >> RenderSurface::Bloss) << RenderSurface::Bshift);

		_renderSurface->colour32[i] = c;
	}

	return true;
}

bool Screen::set_palette_entry(uint8 idx, uint8 r, uint8 g, uint8 b) {
	if (_renderSurface == NULL)
		return false;

	uint32 c = ((((uint32)r) >> RenderSurface::Rloss) << RenderSurface::Rshift) | ((((uint32)g) >> RenderSurface::Gloss) << RenderSurface::Gshift) | ((((uint32)b) >> RenderSurface::Bloss) << RenderSurface::Bshift);

	_renderSurface->colour32[idx] = c;

	return true;
}

bool Screen::rotate_palette(uint8 pos, uint8 length) {
	uint32 tmp_colour;
	uint8 i;

	tmp_colour = _renderSurface->colour32[pos + length - 1];

	for (i = length - 1; i > 0; i--)
		_renderSurface->colour32[pos + i] = _renderSurface->colour32[pos + i - 1];

	_renderSurface->colour32[pos] = tmp_colour;

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
bool Screen::clear(sint16 x, sint16 y, sint16 w, sint16 h, Common::Rect *clip_rect) {
	uint8 *pixels;
	uint16 i;
	uint16 x1, y1;

	pixels = (uint8 *)_renderSurface->pixels;

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
		if (x < clip_rect->left)
			x = clip_rect->left;

		if (y < clip_rect->top)
			y = clip_rect->top;

		if (x1 + w > clip_rect->left + clip_rect->width()) {
			w -= (x1 + w) - (clip_rect->left + clip_rect->width());
			if (w <= 0)
				return false;
		}

		if (y1 + h > clip_rect->top + clip_rect->height()) {
			h -= (y1 + h) - (clip_rect->top + clip_rect->height());
			if (h <= 0)
				return false;
		}
	}

	pixels += y * _renderSurface->pitch + (x * _renderSurface->bytes_per_pixel);

	for (i = 0; i < h; i++) {
		memset(pixels, 0, w * _renderSurface->bytes_per_pixel);
		pixels += _renderSurface->pitch;
	}

	return true;
}

bool Screen::fill(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h) {
	if (x >= _renderSurface->w || y >= _renderSurface->h) {
		return true;
	}

	if ((uint32)(y + h) > _renderSurface->h) {
		h = _renderSurface->h - y;
	}

	if ((uint32)(x + w) > _renderSurface->w) {
		w = _renderSurface->w - x;
	}

	if (_renderSurface->bits_per_pixel == 16)
		return fill16(colour_num, x, y, w, h);

	return fill32(colour_num, x, y, w, h);
}

bool Screen::fill16(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h) {
	uint16 *pixels;
	uint16 i, j;

	pixels = (uint16 *)_renderSurface->pixels;

	pixels += y * _renderSurface->w + x;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++)
			pixels[j] = (uint16)_renderSurface->colour32[colour_num];

		pixels += _renderSurface->w;
	}

	return true;
}

bool Screen::fill32(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h) {
	uint32 *pixels;
	uint16 i, j;


	pixels = (uint32 *)_renderSurface->pixels;

	pixels += y * _renderSurface->w + x;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++)
			pixels[j] = _renderSurface->colour32[colour_num];

		pixels += _renderSurface->w;
	}

	return true;
}
void Screen::fade(uint16 dest_x, uint16 dest_y, uint16 src_w, uint16 src_h, uint8 opacity, uint8 fade_bg_color) {
	if (_renderSurface->bits_per_pixel == 16)
		fade16(dest_x, dest_y, src_w, src_h, opacity, fade_bg_color);
	else
		fade32(dest_x, dest_y, src_w, src_h, opacity, fade_bg_color);
}

void Screen::fade16(uint16 dest_x, uint16 dest_y, uint16 src_w, uint16 src_h, uint8 opacity, uint8 fade_bg_color) {
	uint16 bg = (uint16)_renderSurface->colour32[fade_bg_color];
	uint16 *pixels;
	uint16 i, j;

	pixels = (uint16 *)_renderSurface->pixels;

	pixels += dest_y * _renderSurface->w + dest_x;

	for (i = 0; i < src_h; i++) {
		for (j = 0; j < src_w; j++) {
			pixels[j] = blendpixel16(bg, pixels[j], opacity);
		}

		pixels += _renderSurface->w; //_renderSurface->pitch;
	}

	return;
}

void Screen::fade32(uint16 dest_x, uint16 dest_y, uint16 src_w, uint16 src_h, uint8 opacity, uint8 fade_bg_color) {
	uint32 bg = _renderSurface->colour32[fade_bg_color];
	uint32 *pixels;
	uint16 i, j;

	pixels = (uint32 *)_renderSurface->pixels;

	pixels += dest_y * _renderSurface->w + dest_x;

	for (i = 0; i < src_h; i++) {
		for (j = 0; j < src_w; j++) {
			pixels[j] = blendpixel32(bg, pixels[j], opacity);
		}

		pixels += _renderSurface->w; //_renderSurface->pitch;
	}

	return;
}

void Screen::stipple_8bit(uint8 color_num) {
	stipple_8bit(color_num, 0, 0, _renderSurface->w, _renderSurface->h);
}

void Screen::stipple_8bit(uint8 color_num, uint16 x, uint16 y, uint16 w, uint16 h) {
	uint32 i, j;

	if (x >= _renderSurface->w || y >= _renderSurface->h) {
		return;
	}

	if ((uint32)(y + h) > _renderSurface->h) {
		h = _renderSurface->h - y;
	}

	if ((uint32)(x + w) > _renderSurface->w) {
		w = _renderSurface->w - x;
	}

	if (_renderSurface->bits_per_pixel == 16) {
		uint16 color = (uint16)_renderSurface->colour32[color_num];
		uint16 *pixels = (uint16 *)_renderSurface->pixels;

		pixels += y * _renderSurface->w + x;

		for (i = y; i < (uint32)(y + h); i++) {
			for (j = x; j < (uint32)(x + w); j += 2) {
				*pixels = color;
				pixels += 2;
			}
			pixels += (_renderSurface->w - j) + x;
			if (i % 2) {
				pixels--;
			} else {
				pixels++;
			}
		}
	} else {
		uint32 color = _renderSurface->colour32[color_num];
		uint32 *pixels = (uint32 *)_renderSurface->pixels;

		pixels += y * _renderSurface->w + x;

		for (i = 0; i < h; i++) {
			for (j = x; j < (uint32)(x + w); j += 2) {
				*pixels = color;
				pixels += 2;
			}
			pixels += (_renderSurface->w - j) + x;
			if (i % 2) {
				pixels--;
			} else {
				pixels++;
			}
		}
	}
}
void Screen::put_pixel(uint8 colour_num, uint16 x, uint16 y) {
	if (_renderSurface->bits_per_pixel == 16) {
		uint16 *pixel = (uint16 *)_renderSurface->pixels + y * _renderSurface->w + x;
		*pixel = (uint16)_renderSurface->colour32[colour_num];
	} else {
		uint32 *pixel = (uint32 *)_renderSurface->pixels + y * _renderSurface->w + x;
		*pixel = (uint32)_renderSurface->colour32[colour_num];
	}
}

void *Screen::get_pixels() {
//if(scaled_surface == NULL)
//   return NULL;

//return scaled_surface->pixels;
	return NULL;
}

Graphics::ManagedSurface *Screen::get_sdl_surface() {
	if (_renderSurface)
		return _renderSurface->get_sdl_surface();

	return NULL;
}

bool Screen::blit(int32 dest_x, int32 dest_y, const byte *src_buf, uint16 src_bpp,
		uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans,
		Common::Rect *clip_rect, uint8 opacity) {
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
		if (dest_x + src_w < clip_rect->left || dest_y + src_h < clip_rect->top)
			return false;

		if (clip_rect->left > dest_x) {
			src_x = clip_rect->left - dest_x;
			src_w -= src_x;
			dest_x = clip_rect->left;
		}

		if (clip_rect->top > dest_y) {
			src_y = clip_rect->top - dest_y;
			src_h -= src_y;
			dest_y = clip_rect->top;
		}

		if (dest_x + src_w > clip_rect->left + clip_rect->width()) {
			if (clip_rect->left + clip_rect->width() - dest_x <= 0)
				return false;

			src_w = clip_rect->left + clip_rect->width() - dest_x;
		}

		if (dest_y + src_h > clip_rect->top + clip_rect->height()) {
			if (clip_rect->top + clip_rect->height() - dest_y <= 0)
				return false;

			src_h = clip_rect->top + clip_rect->height() - dest_y;
		}

		src_buf += src_y * src_pitch + src_x;
	}

	if (_renderSurface->bits_per_pixel == 16) {
		if (opacity < 255)
			return blit16WithOpacity(dest_x, dest_y, src_buf, src_bpp, src_w, src_h, src_pitch, trans, opacity);

		return blit16(dest_x, dest_y, src_buf, src_bpp, src_w, src_h, src_pitch, trans);
	}

	if (opacity < 255)
		return blit32WithOpacity(dest_x, dest_y, src_buf, src_bpp, src_w, src_h, src_pitch, trans, opacity);

	return blit32(dest_x, dest_y, src_buf, src_bpp, src_w, src_h, src_pitch, trans);
}


inline uint16 Screen::blendpixel16(uint16 p, uint16 p1, uint8 opacity) {
	return (((uint8)(((float)((p1 & _renderSurface->Rmask) >> _renderSurface->Rshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & _renderSurface->Rmask) >> _renderSurface->Rshift)) * (float)(255 - opacity) / 255.0f)) << _renderSurface->Rshift) |     //R
	       (((uint8)(((float)((p1 & _renderSurface->Gmask) >> _renderSurface->Gshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & _renderSurface->Gmask) >> _renderSurface->Gshift)) * (float)(255 - opacity) / 255.0f)) << _renderSurface->Gshift) |    //G
	       (((uint8)(((float)((p1 & _renderSurface->Bmask) >> _renderSurface->Bshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & _renderSurface->Bmask) >> _renderSurface->Bshift)) * (float)(255 - opacity) / 255.0f)) << _renderSurface->Bshift);     //B
}

inline uint32 Screen::blendpixel32(uint32 p, uint32 p1, uint8 opacity) {
	return (((uint8)(((float)((p1 & _renderSurface->Rmask) >> _renderSurface->Rshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & _renderSurface->Rmask) >> _renderSurface->Rshift)) * (float)(255 - opacity) / 255.0f)) << _renderSurface->Rshift) |     //R
	       (((uint8)(((float)((p1 & _renderSurface->Gmask) >> _renderSurface->Gshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & _renderSurface->Gmask) >> _renderSurface->Gshift)) * (float)(255 - opacity) / 255.0f)) << _renderSurface->Gshift) |    //G
	       (((uint8)(((float)((p1 & _renderSurface->Bmask) >> _renderSurface->Bshift)) * (float)(opacity) / 255.0f) + (uint8)(((float)((p & _renderSurface->Bmask) >> _renderSurface->Bshift)) * (float)(255 - opacity) / 255.0f)) << _renderSurface->Bshift);     //B
}

inline bool Screen::blit16(uint16 dest_x, uint16 dest_y, const byte *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans) {
	uint16 *pixels;
	uint16 i, j;

	pixels = (uint16 *)_renderSurface->pixels;

	pixels += dest_y * _renderSurface->w + dest_x;

	if (trans) {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				if (src_buf[j] != 0xff)
					pixels[j] = (uint16)_renderSurface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += _renderSurface->w; //pitch;
		}
	} else {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = (uint16)_renderSurface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += _renderSurface->w; //_renderSurface->pitch;
		}
	}

	return true;
}

inline bool Screen::blit16WithOpacity(uint16 dest_x, uint16 dest_y, const byte *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans, uint8 opacity) {
	uint16 *pixels;
	uint16 i, j;

	pixels = (uint16 *)_renderSurface->pixels;

	pixels += dest_y * _renderSurface->w + dest_x;

	if (trans) {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				if (src_buf[j] != 0xff)
					pixels[j] = blendpixel16(pixels[j], (uint16)_renderSurface->colour32[src_buf[j]], opacity);
			}
			src_buf += src_pitch;
			pixels += _renderSurface->w; //pitch;
		}
	} else {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = blendpixel16(pixels[j], (uint16)_renderSurface->colour32[src_buf[j]], opacity);
			}
			src_buf += src_pitch;
			pixels += _renderSurface->w; //_renderSurface->pitch;
		}
	}

	return true;
}

bool Screen::blit32(uint16 dest_x, uint16 dest_y, const byte *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans) {
	uint32 *pixels;
	uint16 i, j;

	pixels = (uint32 *)_renderSurface->pixels;

	pixels += dest_y * _renderSurface->w + dest_x;


	if (trans) {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				if (src_buf[j] != 0xff)
					pixels[j] = _renderSurface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += _renderSurface->w; //pitch;
		}
	} else {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = _renderSurface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += _renderSurface->w; //_renderSurface->pitch;
		}
	}

	return true;
}

bool Screen::blit32WithOpacity(uint16 dest_x, uint16 dest_y, const byte *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans, uint8 opacity) {
	uint32 *pixels;
	uint16 i, j;

	pixels = (uint32 *)_renderSurface->pixels;

	pixels += dest_y * _renderSurface->w + dest_x;


	if (trans) {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				if (src_buf[j] != 0xff)
					pixels[j] = blendpixel32(pixels[j], _renderSurface->colour32[src_buf[j]], opacity);
			}
			src_buf += src_pitch;
			pixels += _renderSurface->w; //pitch;
		}
	} else {
		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = blendpixel32(pixels[j], _renderSurface->colour32[src_buf[j]], opacity);
			}
			src_buf += src_pitch;
			pixels += _renderSurface->w; //_renderSurface->pitch;
		}
	}

	return true;
}

void Screen::blitbitmap(uint16 dest_x, uint16 dest_y, const byte *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color) {
	if (_renderSurface->bits_per_pixel == 16)
		blitbitmap16(dest_x, dest_y, src_buf, src_w, src_h, fg_color, bg_color);
	else
		blitbitmap32(dest_x, dest_y, src_buf, src_w, src_h, fg_color, bg_color);

	return;
}

void Screen::blitbitmap16(uint16 dest_x, uint16 dest_y, const byte *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color) {
	uint16 *pixels;
	uint16 i, j;

	pixels = (uint16 *)_renderSurface->pixels;

	pixels += dest_y * _renderSurface->w + dest_x;

	for (i = 0; i < src_h; i++) {
		for (j = 0; j < src_w; j++) {
			if (src_buf[j])
				pixels[j] = (uint16)_renderSurface->colour32[fg_color];
			else
				pixels[j] = (uint16)_renderSurface->colour32[bg_color];
		}
		src_buf += src_w;
		pixels += _renderSurface->w; //_renderSurface->pitch;
	}

	return;
}

void Screen::blitbitmap32(uint16 dest_x, uint16 dest_y, const byte *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color) {
	uint32 *pixels;
	uint16 i, j;

	pixels = (uint32 *)_renderSurface->pixels;

	pixels += dest_y * _renderSurface->w + dest_x;

	for (i = 0; i < src_h; i++) {
		for (j = 0; j < src_w; j++) {
			if (src_buf[j])
				pixels[j] = _renderSurface->colour32[fg_color];
			else
				pixels[j] = _renderSurface->colour32[bg_color];
		}
		src_buf += src_w;
		pixels += _renderSurface->w; //_renderSurface->pitch;
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
		shading_rect.left = x;
		shading_rect.top = y;
		if (lighting_style == LIGHTING_STYLE_ORIGINAL) {
			shading_rect.setWidth(w + (SHADING_BORDER * 2));
			shading_rect.setHeight(h + (SHADING_BORDER * 2));
		} else { // LIGHTING_STYLE_SMOOTH
			shading_rect.setWidth((w + (SHADING_BORDER * 2)) * 16 + 8);
			shading_rect.setHeight((h + (SHADING_BORDER * 2)) * 16 + 8);
		}
		shading_data = (byte *)malloc(sizeof(byte) * shading_rect.width() * shading_rect.height());
		if (shading_data == NULL) {
			/* We couldn't allocate memory for the opacity map, so just disable lighting */
			shading_ambient = 0xFF;
			return;
		}
		buildalphamap8();
	}
	if (shading_ambient == 0xFF) {
	} else {
		memset(shading_data, shading_ambient, sizeof(char)*shading_rect.width() * shading_rect.height());
	}
	sint16 x_off;
	if (Game::get_game()->is_original_plus_full_map())
		x_off = - Game::get_game()->get_background()->get_border_width();
	else
		x_off = 0;
	//Light globe around the avatar
	if (lighting_style == LIGHTING_STYLE_ORIGINAL)
		drawalphamap8globe((shading_rect.width() - 1 + x_off / 16) / 2 - SHADING_BORDER, (shading_rect.height() - 1) / 2 - SHADING_BORDER, opacity / 20 + 4); //range 4 - 10
	else if (lighting_style == LIGHTING_STYLE_SMOOTH)
		drawalphamap8globe((((shading_rect.width() - 8 + x_off) / 16) - 1) / 2 - SHADING_BORDER, (((shading_rect.height() - 8) / 16) - 1) / 2 - SHADING_BORDER, party_light_source ? 5 : 4);
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
				if (x + i - rad < 0 || x + i - rad >= shading_rect.width())
					continue;
				if (y + j - rad < 0 || y + j - rad >= shading_rect.height())
					continue;
				shading_data[(y + j - rad)*shading_rect.width() + (x + i - rad)] = MIN(shading_data[(y + j - rad) * shading_rect.width() + (x + i - rad)] + TileGlobe[r - 1][j * (rad * 2 + 1) + i], 4);
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
			        (y + i) + 1 > shading_rect.height() ||
			        (x + j) + 1 > shading_rect.width())
				continue;
			shading_data[(y + i)*shading_rect.width() + (x + j)] = MIN(shading_data[(y + i) * shading_rect.width() + (x + j)] + shading_globe[r][(i + globeradius_2[r]) * globeradius[r] + (j + globeradius_2[r])], 255);
		}
}


void Screen::blitalphamap8(sint16 x, sint16 y, Common::Rect *clip_rect) {
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

		for (j = SHADING_BORDER; j < shading_rect.height() - SHADING_BORDER; j++) {
			for (i = SHADING_BORDER; i < shading_rect.width() - SHADING_BORDER; i++) {
				if (shading_data[j * shading_rect.width() + i] < 4)
					blit(x + (i - SHADING_BORDER) * 16, y + (j - SHADING_BORDER) * 16, shading_tile[shading_data[j * shading_rect.width() + i]], 8, 16, 16, 16, true, game->get_map_window()->get_clip_rect());
			}
		}
		return;
	}

	uint16 src_w = shading_rect.width() - (SHADING_BORDER * 2 * 16);
	uint16 src_h = shading_rect.height() - (SHADING_BORDER * 2 * 16);

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

		src_buf += shading_rect.width() * -y;
		y = 0;
	}

	if (x + src_w >= width)
		src_w = width - x;

	if (y + src_h >= height)
		src_h = height - y;

	//clip to rect if required.

	if (clip_rect) {
		//if(x + src_w < clip_rect->left || y + src_h < clip_rect->top)
		//   return;

		if (clip_rect->left > x) {
			src_x += (clip_rect->left - x);
			src_w -= (clip_rect->left - x);
			x = clip_rect->left;
		}

		if (clip_rect->top > y) {
			src_y += (clip_rect->top - y);
			src_h -= (clip_rect->top - y);
			y = clip_rect->top;
		}

		if (x + src_w > clip_rect->left + clip_rect->width()) {
			//if(clip_rect->left + clip_rect->width() - x <= 0)
			//  return;

			src_w = clip_rect->left + clip_rect->width() - x;
		}

		if (y + src_h > clip_rect->top + clip_rect->height()) {
			//if(clip_rect->top + clip_rect->height() - y <= 0)
			//  return;

			src_h = clip_rect->top + clip_rect->height() - y;
		}

		src_buf += src_y * shading_rect.width() + src_x;
	}


	switch (_renderSurface->bits_per_pixel) {
	case 16:
		uint16 *pixels16;
		pixels16 = (uint16 *)_renderSurface->pixels;

		pixels16 += y * _renderSurface->w + x;

		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels16[j] = (((unsigned char)(((float)((pixels16[j] & _renderSurface->Rmask) >> _renderSurface->Rshift)) * (float)(src_buf[j]) / 255.0f)) << _renderSurface->Rshift) |      //R
				              (((unsigned char)(((float)((pixels16[j] & _renderSurface->Gmask) >> _renderSurface->Gshift)) * (float)(src_buf[j]) / 255.0f)) << _renderSurface->Gshift) |      //G
				              (((unsigned char)(((float)((pixels16[j] & _renderSurface->Bmask) >> _renderSurface->Bshift)) * (float)(src_buf[j]) / 255.0f)) << _renderSurface->Bshift);       //B
			}
			pixels16 += _renderSurface->w;
			src_buf += shading_rect.width();
		}
		return;
		break;
	case 24:
	case 32:
		uint32 *pixels;
		pixels = (uint32 *)_renderSurface->pixels;

		pixels += y * _renderSurface->w + x;

		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = (((unsigned char)(((float)((pixels[j] & _renderSurface->Rmask) >> _renderSurface->Rshift)) * (float)(src_buf[j]) / 255.0f)) << _renderSurface->Rshift) |      //R
				            (((unsigned char)(((float)((pixels[j] & _renderSurface->Gmask) >> _renderSurface->Gshift)) * (float)(src_buf[j]) / 255.0f)) << _renderSurface->Gshift) |      //G
				            (((unsigned char)(((float)((pixels[j] & _renderSurface->Bmask) >> _renderSurface->Bshift)) * (float)(src_buf[j]) / 255.0f)) << _renderSurface->Bshift);       //B
			}
			pixels += _renderSurface->w;
			src_buf += shading_rect.width();
		}
		return;
		break;
	default:
		DEBUG(0, LEVEL_ERROR, "Screen::blitalphamap8() cannot handle your screen _renderSurface depth of %d\n", _renderSurface->bits_per_pixel);
		break;
		return;
	}

}


/* Return an 8bit _renderSurface. Source format is assumed to be identical to screen. */
Graphics::ManagedSurface *Screen::create_sdl_surface_8(byte *src_buf, uint16 src_w, uint16 src_h) {
	Graphics::ManagedSurface *new_surface = new Graphics::ManagedSurface(src_w, src_h,
		Graphics::PixelFormat::createFormatCLUT8());
	byte *pixels = (byte *)new_surface->getPixels();

	if (_renderSurface->bits_per_pixel == 16) {
		uint16 *src = (uint16 *)src_buf;
		for (int p = 0; p < (src_w * src_h); p++)
			for (int i = 0; i < 256; i++) // convert to 8bpp
				if (src[p] == (uint16)_renderSurface->colour32[i]) {
					pixels[p] = i;
					break;
				}
	} else {
		uint32 *src = (uint32 *)src_buf;
		for (int p = 0; p < (src_w * src_h); p++)
			for (int i = 0; i < 256; i++)
				if (src[p] == _renderSurface->colour32[i]) {
					pixels[p] = i;
					break;
				}
	}
	return (new_surface);
}


Graphics::ManagedSurface *Screen::create_sdl_surface_from(byte *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch) {
	Graphics::ManagedSurface *new_surface;
	uint16 i, j;

	new_surface = RenderSurface::createSurface(src_w, src_h,
		_renderSurface->getFormat());

	if (_renderSurface->bits_per_pixel == 16) {
		uint16 *pixels = (uint16 *)new_surface->getPixels();

		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = (uint16)_renderSurface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += src_pitch; //_renderSurface->pitch;
		}
	} else {
		uint32 *pixels = (uint32 *)new_surface->getPixels();

		for (i = 0; i < src_h; i++) {
			for (j = 0; j < src_w; j++) {
				pixels[j] = _renderSurface->colour32[src_buf[j]];
			}
			src_buf += src_pitch;
			pixels += src_w;
		}
	}

	return new_surface;

}

uint16 Screen::get_pitch() {
	return (uint16)_renderSurface->pitch;
}

uint16 Screen::get_bpp() {
	return _renderSurface->bits_per_pixel;
}

void Screen::update() {
	_rawSurface->markAllDirty();
	_rawSurface->update();
}


void Screen::update(int x, int y, uint16 w, uint16 h) {
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x > width)
		return;
	if (y > height)
		return;
	if ((x + w) > width)
		w = width - x;
	if ((y + h) > height)
		h = height - y;

	// Get the subarea, which internally adds a dirty rect for the given area
	_rawSurface->getSubArea(Common::Rect(x, y, x + width, y + height));
}

void Screen::preformUpdate() {
	_rawSurface->update();
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
	Std::string scaler_name;

	return true;
}

int Screen::get_screen_bpp() {
	// Get info. about video.
	Graphics::PixelFormat pf = g_system->getScreenFormat();
	return pf.bpp();
}

void Screen::set_screen_mode() {
	if (scale_factor == 0) scale_factor = 1;
	scaler = 0;
	scale_factor = 1;
		
	Graphics::PixelFormat SCREEN_FORMAT(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(width, height, &SCREEN_FORMAT);
	_rawSurface = new Graphics::Screen(width, height, SCREEN_FORMAT);

	_renderSurface = CreateRenderSurface(_rawSurface);
}

bool Screen::is_fullscreen() const {
	return g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
}

bool Screen::toggle_fullscreen() {
	set_fullscreen(!is_fullscreen());
	return true;
}

bool Screen::set_fullscreen(bool value) {
	g_system->beginGFXTransaction();
	g_system->setFeatureState(OSystem::kFeatureFullscreenMode, value);
	g_system->endGFXTransaction();

	return true;
}

//Note! assumes area divides evenly by down_scale factor
byte *Screen::copy_area(Common::Rect *area, uint16 down_scale) {
	if (_renderSurface->bits_per_pixel == 16)
		return (copy_area16(area, down_scale));

	return (copy_area32(area, down_scale));
}

byte *Screen::copy_area16(Common::Rect *area, uint16 down_scale) {
	Graphics::PixelFormat *fmt;
	Graphics::ManagedSurface *main_surface = get_sdl_surface();
	byte *dst_pixels = NULL;
	byte *ptr;
	const uint16 *src_pixels;
	uint32 r, g, b;
	uint32 ra, ga, ba;
	uint16 x, y;
	uint8 x1, y1;

	dst_pixels = new unsigned char[((area->width() / down_scale) * (area->height() / down_scale)) * 3];
	ptr = dst_pixels;

	fmt = &main_surface->format;

	for (y = 0; y < area->height(); y += down_scale) {
		for (x = 0; x < area->width(); x += down_scale) {
			r = 0;
			g = 0;
			b = 0;

			src_pixels = (const uint16 *)main_surface->getPixels();
			src_pixels += ((area->top + y) * _renderSurface->w + (area->left + x));

			for (y1 = 0; y1 < down_scale; y1++) {
				for (x1 = 0; x1 < down_scale; x1++) {
					ra = *src_pixels & fmt->rMax();
					ra >>= fmt->rShift;
					ra <<= fmt->rLoss;

					ga = *src_pixels & fmt->gMax();
					ga >>= fmt->gShift;
					ga <<= fmt->gLoss;

					ba = *src_pixels & fmt->bMax();
					ba >>= fmt->bShift;
					ba <<= fmt->bLoss;

					r += ra;
					g += ga;
					b += ba;

					src_pixels++;
				}
				src_pixels += _renderSurface->w;
			}

			ptr[0] = (uint8)(r / (down_scale * down_scale));
			ptr[1] = (uint8)(g / (down_scale * down_scale));
			ptr[2] = (uint8)(b / (down_scale * down_scale));
			ptr += 3;
		}
	}

	return dst_pixels;
}

byte *Screen::copy_area32(Common::Rect *area, uint16 down_scale) {
	Graphics::PixelFormat *fmt;
	Graphics::ManagedSurface *main_surface = get_sdl_surface();
	byte *dst_pixels = NULL;
	byte *ptr;
	const uint32 *src_pixels;
	uint32 r, g, b;
	uint32 ra, ga, ba;
	uint16 x, y;
	uint8 x1, y1;

	dst_pixels = new unsigned char[((area->width() / down_scale) * (area->height() / down_scale)) * 3];
	ptr = dst_pixels;

	fmt = &main_surface->format;

	for (y = 0; y < area->height(); y += down_scale) {
		for (x = 0; x < area->width(); x += down_scale) {
			r = 0;
			g = 0;
			b = 0;

			src_pixels = (const uint32 *)main_surface->getPixels();
			src_pixels += ((area->top + y) * _renderSurface->w + (area->left + x));

			for (y1 = 0; y1 < down_scale; y1++) {
				for (x1 = 0; x1 < down_scale; x1++) {
					ra = *src_pixels & fmt->rMax();
					ra >>= fmt->rShift;
					ra <<= fmt->rLoss;

					ga = *src_pixels & fmt->gMax();
					ga >>= fmt->gShift;
					ga <<= fmt->gLoss;

					ba = *src_pixels & fmt->bMax();
					ba >>= fmt->bShift;
					ba <<= fmt->bLoss;

					r += ra;
					g += ga;
					b += ba;

					src_pixels++;
				}
				src_pixels += _renderSurface->w;
			}

			ptr[0] = (uint8)(r / (down_scale * down_scale));
			ptr[1] = (uint8)(g / (down_scale * down_scale));
			ptr[2] = (uint8)(b / (down_scale * down_scale));
			ptr += 3;
		}
	}

	return dst_pixels;
}

// _renderSurface -> byte *
// (NULL area = entire screen)
byte *Screen::copy_area(Common::Rect *area, byte *buf) {
	Common::Rect screen_area(0, 0, _renderSurface->w, _renderSurface->h);
	if (!area)
		area = &screen_area;

	if (_renderSurface->bits_per_pixel == 16)
		return (copy_area16(area, buf));
	return (copy_area32(area, buf));
}


// byte * -> _renderSurface
// byte * -> target (src area still means location on screen, not relative to target)
// (NULL area = entire screen)
void Screen::restore_area(byte *pixels, Common::Rect *area,
                          byte *target, Common::Rect *target_area, bool free_src) {
	Common::Rect screen_area(0, 0, _renderSurface->w, _renderSurface->h);
	if (!area)
		area = &screen_area;

	if (_renderSurface->bits_per_pixel == 16)
		restore_area16(pixels, area, target, target_area, free_src);
	else
		restore_area32(pixels, area, target, target_area, free_src);
}


byte *Screen::copy_area32(Common::Rect *area, byte *buf) {
	uint32 *copied = (uint32 *)buf;
	if (buf == NULL) {
		copied = (uint32 *)malloc(area->width() * area->height() * 4);
	}
	uint32 *dest = copied;
	uint32 *src = (uint32 *)_renderSurface->pixels;
	uint16 src_x_off = ABS(area->left);
	uint16 src_y_off = ABS(area->top);
	uint16 src_w = area->width();
	uint16 src_h = area->height();

	if (area->left < 0) {
		src_x_off = 0;
		src_w += area->left;
		dest += ABS(area->left);
	}

	if (area->top < 0) {
		src_y_off = 0;
		src_h += area->top;
		dest += (area->width() * ABS(area->top));
	}

	if (src_x_off + src_w > (int)_renderSurface->w) {
		src_w -= ((src_x_off + src_w) - _renderSurface->w);
	}

	if (src_y_off + src_h > (int)_renderSurface->h) {
		src_h -= ((src_y_off + src_h) - _renderSurface->h);
	}

	src += src_y_off * _renderSurface->w + src_x_off;

	for (uint32 i = 0; i < src_h; i++) {
		for (uint32 j = 0; j < src_w; j++)
			dest[j] = src[j];
		dest += area->width();
		src += _renderSurface->w;
	}
	return ((byte *)copied);
}


void Screen::restore_area32(byte *pixels, Common::Rect *area,
                            byte *target, Common::Rect *target_area, bool free_src) {
	uint32 *src = (uint32 *)pixels;
	uint32 *dest = (uint32 *)_renderSurface->pixels;
	dest += area->top * _renderSurface->w + area->left;
	if (target) { // restore to target instead of screen
		dest = (uint32 *)target;
		dest += (area->top - target_area->top) * target_area->width() + (area->left - target_area->left);
	}
	for (int i = 0; i < area->height(); i++) {
		for (int j = 0; j < area->width(); j++)
			dest[j] = src[j];
		src += area->width();
		dest += target ? target_area->width() : _renderSurface->w;
	}
	if (free_src) {
		free(pixels);
	}
}


byte *Screen::copy_area16(Common::Rect *area, byte *buf) {
	uint16 *copied = (uint16 *)buf;
	if (buf == NULL) {
		copied = (uint16 *)malloc(area->width() * area->height() * 2);
	}
	uint16 *dest = copied;
	uint16 *src = (uint16 *)_renderSurface->pixels;
	uint16 src_x_off = ABS(area->left);
	uint16 src_y_off = ABS(area->top);
	uint16 src_w = area->width();
	uint16 src_h = area->height();

	if (area->left < 0) {
		src_x_off = 0;
		src_w += area->left;
		dest += ABS(area->left);
	}

	if (area->top < 0) {
		src_y_off = 0;
		src_h += area->top;
		dest += (area->width() * ABS(area->top));
	}

	if (src_x_off + src_w > (int)_renderSurface->w) {
		src_w -= ((src_x_off + src_w) - _renderSurface->w);
	}

	if (src_y_off + src_h > (int)_renderSurface->h) {
		src_h -= ((src_y_off + src_h) - _renderSurface->h);
	}

	src += src_y_off * _renderSurface->w + src_x_off;

	for (uint32 i = 0; i < src_h; i++) {
		for (uint32 j = 0; j < src_w; j++)
			dest[j] = src[j];
		dest += area->width();
		src += _renderSurface->w;
	}
	return ((byte *)copied);
}


void Screen::restore_area16(byte *pixels, Common::Rect *area,
                            byte *target, Common::Rect *target_area, bool free_src) {
	uint16 *src = (uint16 *)pixels;
	uint16 *dest = (uint16 *)_renderSurface->pixels;
	dest += area->top * _renderSurface->w + area->left;
	if (target) { // restore to target instead of screen
		dest = (uint16 *)target;
		dest += (area->top - target_area->top) * target_area->width() + (area->left - target_area->left);
	}

	for (int i = 0; i < area->height(); i++) {
		for (int j = 0; j < area->width(); j++)
			dest[j] = src[j];
		src += area->width();
		dest += target ? target_area->width() : _renderSurface->w;
	}
	if (free_src) {
		free(pixels);
	}
}

void Screen::draw_line(int sx, int sy, int ex, int ey, uint8 color) {
	if (_renderSurface == NULL)
		return;

	_renderSurface->draw_line(sx, sy, ex, ey, color);

	return;
}


void Screen::get_mouse_location(int *x, int *y) {
	Common::Point pt = Events::get()->getMousePos();
	*x = pt.x;
	*y = pt.y;
}

void Screen::set_non_square_pixels(bool value) {
	// No implementation
}

} // End of namespace Nuvie
} // End of namespace Ultima
