/* 
 * Gamecube/Wii VIDEO/GX subsystem wrapper
 *
 * Copyright (C) 2008, 2009		Andre Heider "dhewg" <dhewg@wiibrew.org>
 *
 * This code is licensed to you under the terms of the GNU GPL, version 2;
 * see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 */

#ifndef __GFX_H__
#define __GFX_H__

#include <gccore.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	GFX_STANDARD_AUTO = -1,
	GFX_STANDARD_PROGRESSIVE = 0,
	GFX_STANDARD_NTSC,
	GFX_STANDARD_PAL,
	GFX_STANDARD_EURGB60,
	GFX_STANDARD_MPAL
} gfx_video_standard_t;

typedef enum {
	GFX_MODE_DEFAULT = 0,
	GFX_MODE_DS,
} gfx_video_mode_t;

typedef enum {
	GFX_TF_RGB565 = 0,
	GFX_TF_RGB5A3,
	GFX_TF_PALETTE_RGB565,
	GFX_TF_PALETTE_RGB5A3
} gfx_tex_format_t;

typedef struct {
	void *pixels;
	u16 *palette;

	gfx_tex_format_t format;
	u16 width;
	u16 height;
	u8 bpp;
	GXTexObj obj;
	GXTlutObj tlut;
	u32 tlut_name;
} gfx_tex_t;

typedef enum {
	GFX_COORD_FULLSCREEN = 0,
	GFX_COORD_CENTER
} gfx_coord_t;

typedef struct {
	f32 x, y;
	f32 w, h;
} gfx_coords_t;

gfx_video_standard_t gfx_video_get_standard(void);

void gfx_video_init(gfx_video_standard_t standard, gfx_video_mode_t mode);
void gfx_video_deinit(void);

u16 gfx_video_get_width(void);
u16 gfx_video_get_height(void);

void gfx_init(void);
void gfx_deinit(void);

void gfx_set_underscan(u16 underscan_x, u16 underscan_y);
void gfx_set_ar(f32 ar);
void gfx_set_pillarboxing(bool enable);

bool gfx_tex_init(gfx_tex_t *tex, gfx_tex_format_t format, u32 tlut_name,
					u16 width, u16 height);
void gfx_tex_deinit(gfx_tex_t *tex);

void gfx_coords(gfx_coords_t *coords, gfx_tex_t *tex, gfx_coord_t type);

bool gfx_tex_set_bilinear_filter(gfx_tex_t *tex, bool enable);
bool gfx_tex_flush_texture(gfx_tex_t *tex);
bool gfx_tex_flush_palette(gfx_tex_t *tex);
bool gfx_tex_clear_palette(gfx_tex_t *tex);

bool gfx_tex_convert(gfx_tex_t *tex, const void *src);

void gfx_frame_start(void);
void gfx_frame_end(void);

void gfx_draw_tex(gfx_tex_t *tex, gfx_coords_t *coords);

#ifdef __cplusplus
}
#endif

#endif

