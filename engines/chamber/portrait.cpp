/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/portrait.h"
#include "chamber/resdata.h"
#include "chamber/room.h"
#include "chamber/cga.h"
#include "chamber/ega.h"
#include "chamber/renderer.h"
#include "chamber/ega_resource.h"
#include "chamber/script.h"
#include "chamber/dialog.h"
#include "chamber/input.h"
#include "chamber/sound.h"
#include "chamber/ifgm.h"

namespace Chamber {


extern uint16 cpu_speed_delay;

byte *cur_image_pixels;
byte cur_image_size_w;
byte cur_image_size_h;
byte cur_image_coords_x;
byte cur_image_coords_y;
uint16 cur_image_offs;
uint16 cur_image_end;
byte cur_image_idx;
byte cur_image_anim1;
byte cur_image_anim2;
uint16 cur_frame_width;

typedef struct persframe_t {
	byte height;
	byte width;
	byte topbot;   /*border and fill colors*/
	byte fill;
	byte left;
	byte right;
} persframe_t;

persframe_t pers_frames[] = {
	{65, 16, 0xFF, 0xAA, 0xEA, 0xAB},
	{70, 16, 0xFF, 0xAA, 0xEA, 0xAB},
	{65, 17, 0xFF, 0xAA, 0xEA, 0xAB},
	{75, 17, 0xFF, 0xAA, 0xEA, 0xAB},
	{85, 16, 0xFF, 0xAA, 0xEA, 0xAB},
	{47, 13, 0xFF,    0, 0xC0,    3},
	{65, 18, 0xFF, 0xAA, 0xEA, 0xAB},
	{38, 11, 0xFF,    0, 0xC0,    3},
	{27, 34,    0,    0,    0,    0}
};

void makePortraitFrame(byte index, byte *target) {
	uint16 i;
	persframe_t *pframe = &pers_frames[index];
	*target++ = pframe->height;
	*target++ = pframe->width;
	cur_frame_width = pframe->width;
	memset(target, pframe->topbot, pframe->width);
	target += pframe->width;
	for (i = 0; i < pframe->height - 2; i++) {
		*target++ = pframe->left;
		memset(target, pframe->fill, pframe->width - 2);
		target += pframe->width - 2;
		*target++ = pframe->right;
	}
	memset(target, pframe->topbot, pframe->width);
}

/*TODO: move this to CGA ?*/
/*
Superimpose source sprite data over target image data
*/
void mergeImageAndSpriteData(byte *target, int16 pitch, byte *source, uint16 w, uint16 h) {
	uint16 x;
	while (h--) {
		for (x = 0; x < w; x++) {
			byte m = *source++;
			*target &= m;
			*target++ |= *source++;
		}
		target -= w;
		target += pitch;
	}
}

/*
Superimpose horizontally-flipped source sprite data over target image data
*/
void mergeImageAndSpriteDataFlip(byte *target, int16 pitch, byte *source, uint16 w, uint16 h) {
	uint16 x;
	target += w - 1;
	while (h--) {
		for (x = 0; x < w; x++) {
			byte m = cga_pixel_flip[*source++];
			*target &= m;
			*target |= cga_pixel_flip[*source++];
			target -= 1;
		}
		target += w;
		target += pitch;
	}
}

/*
Build portrait from multiple pers sprites
*/
byte *loadPortrait(byte **pinfo, byte *end) {
	while (*pinfo != end) {
		byte index;
		uint16 flags;
		int16 pitch;
		byte *buffer, *sprite;
		byte sprw, sprh;

		index = *((*pinfo)++);
		flags = *((*pinfo)++);
		flags |= (*((*pinfo)++)) << 8;
		buffer = sprit_load_buffer + 2 + 2 + (flags & 0x3FFF);
		pitch = cur_frame_width;

		sprite = g_vm->_renderer->loadPersSprit(index);
		sprw = *sprite++;
		sprh = *sprite++;

		if (flags & 0x8000) { /*vertical flip*/
			buffer += pitch * (sprh - 1);
			pitch = -pitch;
		}
		if (flags & 0x4000) /*horizontal flip*/
			mergeImageAndSpriteDataFlip(buffer, pitch, sprite, sprw, sprh);
		else
			mergeImageAndSpriteData(buffer, pitch, sprite, sprw, sprh);
	}
	return sprit_load_buffer + 2;
}

static void ega_expandCgaByte(byte cgaByte, byte *dst) {
	for (int p = 3; p >= 0; p--)
		*dst++ = cga_to_ega_color[(cgaByte >> (p * 2)) & 0x03];
}

static void ega_makePortraitFrame(byte index, byte *target) {
	persframe_t *pf = &pers_frames[index];
	uint16 pw = pf->width * 4;
	byte fillCol = cga_to_ega_color[(pf->fill >> 6) & 0x03];

	*target++ = pf->height;
	*target++ = pf->width;
	cur_frame_width = pf->width;

	for (uint16 x = 0; x < pf->width; x++)
		ega_expandCgaByte(pf->topbot, target + x * 4);
	target += pw;

	for (uint16 i = 0; i < pf->height - 2; i++) {
		ega_expandCgaByte(pf->left, target);
		memset(target + 4, fillCol, (pf->width - 2) * 4);
		ega_expandCgaByte(pf->right, target + (pf->width - 1) * 4);
		target += pw;
	}

	for (uint16 x = 0; x < pf->width; x++)
		ega_expandCgaByte(pf->topbot, target + x * 4);
}

static byte *ega_loadPortrait(byte **pinfo, byte *end) {
	uint16 frame_pw = cur_frame_width * 4;

	while (*pinfo != end) {
		byte index;
		uint16 flags;

		index = *((*pinfo)++);
		flags = *((*pinfo)++);
		flags |= (*((*pinfo)++)) << 8;

		Graphics::Surface *surf = ega_perso_res->getSprite(index);
		byte *src = (byte *)surf->getPixels();
		uint16 sw = surf->w;
		uint16 sh = surf->h;
		int16 spitch = surf->pitch;

		uint16 cga_ofs = flags & 0x3FFF;
		uint16 row = cga_ofs / cur_frame_width;
		uint16 col_cga = cga_ofs % cur_frame_width;
		byte *dst = sprit_load_buffer + 2 + 2 + row * frame_pw + col_cga * 4;

		if (flags & 0x8000) { /*vertical flip*/
			src += spitch * (sh - 1);
			spitch = -spitch;
		}

		if (flags & 0x4000) { /*horizontal flip*/
			for (uint16 y = 0; y < sh; y++) {
				for (uint16 x = 0; x < sw; x++) {
					byte p = src[sw - 1 - x];
					if (p != 0)
						dst[x] = p;
				}
				src += spitch;
				dst += frame_pw;
			}
		} else {
			for (uint16 y = 0; y < sh; y++) {
				for (uint16 x = 0; x < sw; x++) {
					byte p = src[x];
					if (p != 0)
						dst[x] = p;
				}
				src += spitch;
				dst += frame_pw;
			}
		}
	}
	return sprit_load_buffer + 2;
}

byte *CGARenderer::loadPortraitWithFrame(byte index) {
	byte *pinfo, *end;
	pinfo = seekToEntry(icone_data, index, &end);
	makePortraitFrame(*pinfo++, sprit_load_buffer + 2);
	return loadPortrait(&pinfo, end);
}

byte *EGARenderer::loadPortraitWithFrame(byte index) {
	byte *pinfo, *end;
	pinfo = seekToEntry(icone_data, index, &end);
	ega_makePortraitFrame(*pinfo++, sprit_load_buffer + 2);
	return ega_loadPortrait(&pinfo, end);
}


#define STATIC_ANIMS_MAX 24

struct {
	byte   index;
	byte   image;
	byte   x;
	byte   y;
	byte   anim1;
	byte   anim2;
} static_anims[] = {
	{ 24,  13, 35, 10,  4,  5},
	{ 88,  42, 35, 10, 11, 12},
	{152,  50, 35, 10, 13, 14},
	{216,  58, 35, 10, 15, 16},
	{ 40,   9, 30, 20,  3,  3},
	{ 48,   1, 35, 20,  1,  2},
	{ 32,  66, 35, 20, 17, 18},
	{128,  21, 20, 10,  6,  6},
	{192,  25,  2, 70,  7,  7},
	{ 56,  85, 25, 20, 26, 27},
	{ 64,  74, 56, 85, 23, 23},
	{ 72,  74, 56, 85, 23, 23},
	{ 80,  78, 27, 20, 24, 24},
	{144,  80, 27, 20, 25, 25},
	{ 96, 100, 27, 20, 29, 29},
	{104,  92, 27, 20, 28, 28},
	{112, 100, 27, 20, 29, 53},
	{224,  96, 27, 20, 48, 48},
	{232,  92, 27, 20, 47, 47},
	{184, 160, 27, 20, 50, 52},
	{200,  78, 27, 20, 24, 24},
	{160, 106, 33,  2, 49, 49},
	{168, 147, 16,  2, 32, 32},
	{248, 117, 16,  2, 33, 33}
};

byte selectCurrentAnim(byte *x, byte *y, byte *index) {
	int16 i;
	byte aniidx = ((pers_t *)(script_vars[kScrPool8_CurrentPers]))->index & ~7;
	for (i = 0; i < STATIC_ANIMS_MAX; i++) {
		if (static_anims[i].index == aniidx) {
			*x = static_anims[i].x;
			*y = static_anims[i].y;
			*index = static_anims[i].image;
			cur_image_anim1 = static_anims[i].anim1;
			cur_image_anim2 = static_anims[i].anim2;
			return 1;
		}
	}
	warning("SelectCurrentAnim: not found for %d", aniidx);
	return 0;
}

void drawBoxAroundSpot(void) {
	byte *buffer;
	uint16 w, h;
	uint16 ofs;
	uint16 x, y;

	if (*spot_sprite == 0)
		return;
	zone_spots_cur = found_spot;
	zone_spr_index = script_byte_vars.cur_spot_idx - 1;

	buffer = *spot_sprite;

	h = *(byte *)(buffer + 0);
	w = *(byte *)(buffer + 1);
	ofs = *(uint16 *)(buffer + 2);

	/*decode ofs back to x:y*/
	if (g_vm->_videoMode == Common::kRenderEGA) {
		y = ofs / EGA_BYTES_PER_LINE;
		x = ofs % EGA_BYTES_PER_LINE;
		w *= 4; /* w was stored in CGA byte units */
	} else {
		y = (ofs & g_vm->_line_offset) ? 1 : 0;
		ofs &= ~g_vm->_line_offset;
		x = (ofs % g_vm->_screenBPL) * g_vm->_screenPPB;
		y += (ofs / g_vm->_screenBPL) * 2;
		w *= g_vm->_screenPPB;
	}

	g_vm->_renderer->drawVLine(x, y, h - 1, 0, SCREENBUFFER);
	g_vm->_renderer->drawHLine(x, y, w - 1, 0, SCREENBUFFER);
	g_vm->_renderer->drawVLine(x + w - 1, y, h - 1, 0, SCREENBUFFER);
	g_vm->_renderer->drawHLine(x, y + h - 1, w - 1, 0, SCREENBUFFER);

	g_vm->_renderer->refreshImageData(*spot_sprite);
}

/*Get on-screen image as specified by script to temp buffer and register it with dirty rect of kind 2
If rmb is pressed, draw it immediately and return 0
*/
int16 drawPortrait(byte **desc, byte *x, byte *y, byte *width, byte *height) {
	byte index;
	byte xx, yy;
	byte *image;

	index = *((*desc)++);
	if (index == 0xFF) {
		if (script_byte_vars.dirty_rect_kind != 0)
			return 0;
		drawBoxAroundSpot();
		if (!selectCurrentAnim(&xx, &yy, &index))
			return 0;
	} else {
		xx = *((*desc)++);
		yy = *((*desc)++);
	}
	cur_image_coords_x = xx;
	cur_image_coords_y = yy;
	cur_image_idx = index;
	image = g_vm->_renderer->loadPortraitWithFrame(index - 1);
	cur_image_size_h = *image++;
	cur_image_size_w = *image++;
	cur_image_pixels = image;
	cur_image_offs = g_vm->_renderer->calcXY_p(cur_image_coords_x, cur_image_coords_y);
	addDirtyRect(DirtyRectSprite, cur_image_coords_x, cur_image_coords_y, cur_image_size_w, cur_image_size_h, cur_image_offs);

	/*TODO: remove and use only globals?*/
	*x = cur_image_coords_x;
	*y = cur_image_coords_y;
	*width = cur_image_size_w;
	*height = cur_image_size_h;

	if (right_button) {
		g_vm->_renderer->blitAndWait(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, SCREENBUFFER, cur_image_offs);
		return 0;
	}

	return 1;
}

void playHurtSound() {
	if (!ifgm_loaded)
		playSound(144);
	else
		playSound(144 + (getRand() / 4) % 4);
}

void blinkWithSound(byte color) {
	g_vm->_renderer->colorSelect(color);
	playHurtSound();
	selectPalette();
}

void blinkToRed(void) {
	blinkWithSound(0x3C);
}

void blinkToWhite(void) {
	if (g_vm->getLanguage() == Common::EN_USA)
		playHurtSound();	/*TODO: play here and later? looks like a bug, original code will trash palette selection if pcspeaker is used*/

	blinkWithSound(0x3F);
}

volatile byte vblank_ticks;

void waitVBlankTimer(void) {
	if (g_vm->getLanguage() == Common::EN_USA) {
		/*Simulate 3 vblank ticks (~50ms) since vblank_ticks is never incremented by the timer*/
		g_system->delayMillis(50);
	}
	waitVBlank();
}

void animPortrait(byte layer, byte index, byte delay) {
	byte *ani, *ani_end;
	byte temp;

	selectCurrentAnim(&temp, &temp, &temp);

	if (index == 0xFF)
		index = cur_image_anim1;
	if (index == 0xFE)
		index = cur_image_anim2;

	IFGM_PlaySfx(index);

	ani = seekToEntry(anico_data, index - 1, &ani_end);
	cur_image_pixels = sprit_load_buffer + 2 + 2;

	while (ani != ani_end) {
		byte kind;
		byte x, y;
		byte width, height;
		uint16 offs;

		byte portrait = *ani++;
		g_vm->_renderer->loadPortraitWithFrame(portrait - 1);
		if (*ani == 0xFF) {
			ani++;
			if (g_vm->_videoMode == Common::kRenderEGA)
				ega_loadPortrait(&ani, ani + 3);
			else
				loadPortrait(&ani, ani + 3);
		}
		getDirtyRectAndSetSprite(layer, &kind, &x, &y, &width, &height, &offs);
		waitVBlank();
		g_vm->_renderer->blitAndWait(cur_image_pixels, width, width, height, SCREENBUFFER, offs);
		waitVBlankTimer();
		if (delay) {
			if (ani[-1] == 37) { /*TODO: what is it?*/
				if (script_byte_vars.extreme_violence)
					blinkToRed();
				else
					blinkToWhite();
			} else {
				int16 i;
				while (delay--) for (i = 0; i < cpu_speed_delay; i++) ; /*TODO: FIXME weak delay*/
			}
		}
	}
}

} // End of namespace Chamber
