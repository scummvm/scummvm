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

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/portrait.h"
#include "chamber/resdata.h"
#include "chamber/room.h"
#include "chamber/cga.h"
#include "chamber/script.h"
#include "chamber/dialog.h"
#include "chamber/input.h"
#include "chamber/sound.h"

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

void MakePortraitFrame(byte index, byte *target) {
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
void MergeImageAndSpriteData(byte *target, int16 pitch, byte *source, uint16 w, uint16 h) {
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
void MergeImageAndSpriteDataFlip(byte *target, int16 pitch, byte *source, uint16 w, uint16 h) {
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
byte *LoadPortrait(byte **pinfo, byte *end) {
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

		sprite = LoadPersSprit(index);
		sprw = *sprite++;
		sprh = *sprite++;

		if (flags & 0x8000) { /*vertical flip*/
			buffer += pitch * (sprh - 1);
			pitch = -pitch;
		}
		if (flags & 0x4000) /*horizontal flip*/
			MergeImageAndSpriteDataFlip(buffer, pitch, sprite, sprw, sprh);
		else
			MergeImageAndSpriteData(buffer, pitch, sprite, sprw, sprh);
	}
	return sprit_load_buffer + 2;
}

byte *LoadPortraitWithFrame(byte index) {
	byte *pinfo, *end;
	pinfo = SeekToEntry(icone_data, index, &end);
	MakePortraitFrame(*pinfo++, sprit_load_buffer + 2);
	return LoadPortrait(&pinfo, end);
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

byte SelectCurrentAnim(byte *x, byte *y, byte *index) {
	int16 i;
	byte aniidx = ((pers_t *)(script_vars[ScrPool8_CurrentPers]))->index & ~7;
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
	printf("SelectCurrentAnim: not found for %d\n", aniidx);
	return 0;
}

void DrawBoxAroundSpot(void) {
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
	/*TODO: this is CGA-only!*/
	y = (ofs & CGA_ODD_LINES_OFS) ? 1 : 0;
	ofs &= ~CGA_ODD_LINES_OFS;
	x = (ofs % CGA_BYTES_PER_LINE) * CGA_PIXELS_PER_BYTE;
	y += (ofs / CGA_BYTES_PER_LINE) * 2;
	w *= CGA_PIXELS_PER_BYTE;   /*TODO: this will overflow on large sprite*/

	CGA_DrawVLine(x, y, h - 1, 0, CGA_SCREENBUFFER);
	CGA_DrawHLine(x, y, w - 1, 0, CGA_SCREENBUFFER);
	CGA_DrawVLine(x + w - 1, y, h - 1, 0, CGA_SCREENBUFFER);
	CGA_DrawHLine(x, y + h - 1, w - 1, 0, CGA_SCREENBUFFER);

	CGA_RefreshImageData(*spot_sprite);
}

/*Get on-screen image as specified by script to temp buffer and register it with dirty rect of kind 2
If rmb is pressed, draw it immediately and return 0
*/
int16 DrawPortrait(byte **desc, byte *x, byte *y, byte *width, byte *height) {
	byte index;
	byte xx, yy;
	byte *image;

	index = *((*desc)++);
	if (index == 0xFF) {
		if (script_byte_vars.dirty_rect_kind != 0)
			return 0;
		DrawBoxAroundSpot();
		if (!SelectCurrentAnim(&xx, &yy, &index))
			return 0;
	} else {
		xx = *((*desc)++);
		yy = *((*desc)++);
	}
	cur_image_coords_x = xx;
	cur_image_coords_y = yy;
	cur_image_idx = index;

	image = LoadPortraitWithFrame(index - 1);
	cur_image_size_h = *image++;
	cur_image_size_w = *image++;
	cur_image_pixels = image;
	cur_image_offs = CGA_CalcXY_p(cur_image_coords_x, cur_image_coords_y);
	AddDirtyRect(DirtyRectSprite, cur_image_coords_x, cur_image_coords_y, cur_image_size_w, cur_image_size_h, cur_image_offs);

	/*TODO: remove and use only globals?*/
	*x = cur_image_coords_x;
	*y = cur_image_coords_y;
	*width = cur_image_size_w;
	*height = cur_image_size_h;

	if (right_button) {
		CGA_BlitAndWait(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);
		return 0;
	}

	return 1;
}

void BlinkWithSound(byte color) {
	CGA_ColorSelect(color);
	PlaySound(144);
	SelectPalette();
}

void BlinkToRed(void) {
	BlinkWithSound(0x3C);
}

void BlinkToWhite(void) {
	BlinkWithSound(0x3F);
}

void AnimPortrait(byte layer, byte index, byte delay) {
	byte *ani, *ani_end;
	byte temp;

	SelectCurrentAnim(&temp, &temp, &temp);

	if (index == 0xFF)
		index = cur_image_anim1;
	if (index == 0xFE)
		index = cur_image_anim2;

	ani = SeekToEntry(anico_data, index - 1, &ani_end);
	cur_image_pixels = sprit_load_buffer + 2 + 2;

	while (ani != ani_end) {
		byte kind;
		byte x, y;
		byte width, height;
		uint16 offs;

		byte portrait = *ani++;
		LoadPortraitWithFrame(portrait - 1);
		if (*ani == 0xFF) {
			ani++;
			LoadPortrait(&ani, ani + 3);
		}
		GetDirtyRectAndSetSprite(layer, &kind, &x, &y, &width, &height, &offs);
		WaitVBlank();
		CGA_BlitAndWait(cur_image_pixels, width, width, height, CGA_SCREENBUFFER, offs);
		if (delay) {
			if (ani[-1] == 37) { /*TODO: what is it?*/
				if (script_byte_vars.extreme_violence)
					BlinkToRed();
				else
					BlinkToWhite();
			} else {
				int16 i;
				while (delay--) for (i = 0; i < cpu_speed_delay; i++) ; /*TODO: FIXME weak delay*/
			}
		}
	}
}

} // End of namespace Chamber
