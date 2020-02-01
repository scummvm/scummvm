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

#ifndef ULTIMA6_SCREEN_SCREEN_H
#define ULTIMA6_SCREEN_SCREEN_H

#include "ultima/shared/std/string.h"

#include "SDL.h"

#include "Game.h"
#include "Surface.h"
#include "Scale.h"

namespace Ultima {
namespace Ultima6 {

#define LIGHTING_STYLE_NONE 0
#define LIGHTING_STYLE_SMOOTH 1
#define LIGHTING_STYLE_ORIGINAL 2

class Configuration;

class Screen {
	Configuration *config;
	SDL_Surface *sdl_surface;
	RenderSurface *surface;
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_Window *sdlWindow;
	SDL_Renderer *sdlRenderer;
	SDL_Texture *sdlTexture;
	float window_scale_w = 1.0, window_scale_h = 1.0;
#endif
	ScalerRegistry     scaler_reg;     // Scaler Registry
	const ScalerStruct *scaler;        // Scaler
	int scaler_index;  // Index of Current Scaler
	int scale_factor;  // Scale factor

	bool fullscreen;
	bool doubleBuffer;
	bool is_no_darkness;
	bool non_square_pixels;

	uint8 palette[768];
	uint16 width;
	uint16 height;
	SDL_Rect *update_rects;
	uint16 num_update_rects;
	uint16 max_update_rects;

	SDL_Rect shading_rect;
	uint8 *shading_data;
	uint8 *shading_globe[6];
	uint8 shading_ambient;
	uint8 *shading_tile[4];

public:
	Screen(Configuration *cfg);
	~Screen();

	bool init();

	bool is_fullscreen() {
		return fullscreen;
	}
	bool is_non_square_pixels() {
		return non_square_pixels;
	}
	int get_scaler_index() {
		return scaler_index;
	}
	ScalerRegistry *get_scaler_reg() {
		return &scaler_reg;
	}
	bool toggle_darkness_cheat();
	bool toggle_fullscreen();
	bool set_palette(uint8 *palette);
	bool set_palette_entry(uint8 idx, uint8 r, uint8 g, uint8 b);
	bool rotate_palette(uint8 pos, uint8 length);
	bool clear(sint16 x, sint16 y, sint16 w, sint16 h, SDL_Rect *clip_rect = NULL);
	void *get_pixels();
	const unsigned char *get_surface_pixels() {
		return (surface->get_pixels());
	}
	uint16 get_pitch();
	SDL_Surface *create_sdl_surface_from(unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch);
	SDL_Surface *create_sdl_surface_8(unsigned char *src_buf, uint16 src_w, uint16 src_h);
	uint16 get_bpp();
	int get_scale_factor() {
		return scale_factor;
	}
	SDL_Surface *get_sdl_surface();
	uint16 get_width() {
		return width;
	}
	uint16 get_height() {
		return height;
	}
	uint16 get_translated_x(uint16 x);
	uint16 get_translated_y(uint16 y);

	bool fill(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h);
	void fade(uint16 dest_x, uint16 dest_y, uint16 src_w, uint16 src_h, uint8 opacity, uint8 fade_bg_color = 0);
	void stipple_8bit(uint8 color_num);
	void stipple_8bit(uint8 color_num, uint16 x, uint16 y, uint16 w, uint16 h);
	void put_pixel(uint8 colour_num, uint16 x, uint16 y);

	bool blit(sint32 dest_x, sint32 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans = false, SDL_Rect *clip_rect = NULL, uint8 opacity = 255);
	void blitbitmap(uint16 dest_x, uint16 dest_y, const unsigned char *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color);

	void buildalphamap8();
	void clearalphamap8(uint16 x, uint16 y, uint16 w, uint16 h, uint8 opacity, bool party_light_source);
	void drawalphamap8globe(sint16 x, sint16 y, uint16 radius);
	void blitalphamap8(sint16 x, sint16 y, SDL_Rect *clip_rect);

	int get_lighting_style() {
		return lighting_style;
	}
	int get_old_lighting_style() {
		return old_lighting_style;    // return the lighting_style before cheats applied
	}
	void set_lighting_style(int lighting);

	uint8 get_ambient() {
		return shading_ambient;
	}
	void set_ambient(uint8 ambient) {
		shading_ambient = ambient;
	}

	void update();
	void update(sint32 x, sint32 y, uint16 w, uint16 h);
	void preformUpdate();
	void lock();
	void unlock();

	bool initScaler();

	unsigned char *copy_area(SDL_Rect *area = NULL, unsigned char *buf = NULL);
	unsigned char *copy_area(SDL_Rect *area, uint16 down_scale);

	void restore_area(unsigned char *pixels, SDL_Rect *area = NULL, unsigned char *target = NULL, SDL_Rect *target_area = NULL, bool free_src = true);

	void draw_line(int sx, int sy, int ex, int ey, uint8 color);

	void get_mouse_location(sint32 *x, sint32 *y);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	void scale_sdl_window_coords(sint32 *x, sint32 *y);
#endif

	void set_non_square_pixels(bool value);

protected:
	int lighting_style, old_lighting_style;
	bool fill16(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h);

	bool fill32(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h);

	void fade16(uint16 dest_x, uint16 dest_y, uint16 src_w, uint16 src_h, uint8 opacity, uint8 fade_bg_color);
	void fade32(uint16 dest_x, uint16 dest_y, uint16 src_w, uint16 src_h, uint8 opacity, uint8 fade_bg_color);

	inline uint16 blendpixel16(uint16 p, uint16 p1, uint8 opacity);
	inline uint32 blendpixel32(uint32 p, uint32 p1, uint8 opacity);

	inline bool blit16(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans);
	inline bool blit16WithOpacity(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans, uint8 opacity);

	inline bool blit32(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans);
	inline bool blit32WithOpacity(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans, uint8 opacity);

	inline void blitbitmap16(uint16 dest_x, uint16 dest_y, const unsigned char *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color);

	inline void blitbitmap32(uint16 dest_x, uint16 dest_y, const unsigned char *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color);

	unsigned char *copy_area16(SDL_Rect *area, uint16 down_scale);
	unsigned char *copy_area32(SDL_Rect *area, uint16 down_scale);

	unsigned char *copy_area16(SDL_Rect *area, unsigned char *buf);
	unsigned char *copy_area32(SDL_Rect *area, unsigned char *buf);
	void restore_area16(unsigned char *pixels, SDL_Rect *area, unsigned char *target = NULL, SDL_Rect *target_area = NULL, bool free_src = true);
	void restore_area32(unsigned char *pixels, SDL_Rect *area, unsigned char *target = NULL, SDL_Rect *target_area = NULL, bool free_src = true);

	void set_screen_mode();
	bool try_scaler(int w, int h, uint32 flags, int hwdepth);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	bool SDL_VideoModeOK(int scaled_width, int scaled_height, int bbp, int flags);
#endif

private:
	int get_screen_bpp();

#if SDL_VERSION_ATLEAST(2, 0, 0)
	bool init_sdl2_window(uint16 scale);
	bool create_sdl_surface_and_texture(sint32 w, sint32 h, Uint32 format);
#else
	bool sdl1_toggle_fullscreen();
#endif

	bool set_fullscreen(bool value);
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
