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
#include "chamber/room.h"
#include "chamber/enums.h"
#include "chamber/resdata.h"
#include "chamber/cga.h"
#include "chamber/print.h"
#include "chamber/anim.h"
#include "chamber/cursor.h"
#include "chamber/script.h"
#include "chamber/print.h"
#include "chamber/input.h"
#include "chamber/dialog.h"
#include "chamber/portrait.h"
#include "chamber/sound.h"
#include "chamber/ifgm.h"

namespace Chamber {

byte scratch_mem1[8010];
byte *scratch_mem2 = scratch_mem1 + 1500;

rect_t room_bounds_rect = {0, 0, 0, 0};
byte last_object_hint = 0;
byte object_hint = 0;
byte command_hint = 0;
byte zone_name = 0;
byte room_hint_bar_width = 0;
byte last_command_hint = 0;
byte zone_spr_index = 0;
byte zone_obj_count = 0;

byte cmd_hint_bar_width = 32;
byte cmd_hint_bar_coords_x = 188 / 4;
byte cmd_hint_bar_coords_y = 193;
byte room_hint_bar_coords_x = 0;
byte room_hint_bar_coords_y = 0;

byte *sprites_list[MAX_SPRITES];

spot_t *zone_spots;
spot_t *zone_spots_end;
spot_t *zone_spots_cur;

vortanims_t *vortanims_ptr;
turkeyanims_t *turkeyanims_ptr;
pers_t *aspirant_ptr;
spot_t *aspirant_spot;
spot_t *found_spot;
byte **spot_sprite;

byte zone_palette;

uint16 zsprite_draw_ofs;
byte zsprite_w;
byte zsprite_h;

byte *lutin_mem;

uint16 drops_cleanup_time = 0;

byte in_de_profundis = 0;  /*TODO: useless?*/

uint16 next_vorts_cmd = 0;
uint16 next_vorts_ticks = 0;
uint16 next_turkey_cmd = 0;
uint16 next_turkey_ticks = 0;
uint16 next_protozorqs_ticks = 0;

byte skip_zone_transition;

#define VORTANIMS_MAX 25

/*
Vorts room enter/leave animations
*/
vortanims_t vortsanim_list[VORTANIMS_MAX] = {
	{ 2, { 3, {{52, 113}}}, { 8, {{43, 113}}}, {12, {{43, 113}}}, {16, {{43, 113}}}},
	{ 3, { 6, {{58, 120}}}, { 7, {{33, 120}}}, {11, {{33, 120}}}, {15, {{33, 120}}}},
	{ 4, { 2, {{26, 121}}}, { 9, {{43, 121}}}, {13, {{43, 121}}}, {17, {{43, 121}}}},
	{ 5, { 1, {{32, 119}}}, {10, {{33, 119}}}, {14, {{33, 119}}}, {18, {{33, 119}}}},
	{ 6, { 3, {{36, 115}}}, { 8, {{27, 115}}}, {12, {{27, 115}}}, {16, {{27, 115}}}},
	{ 7, { 1, {{40, 123}}}, {10, {{41, 123}}}, {14, {{41, 123}}}, {18, {{41, 123}}}},
	{ 8, {21, {{64, 132}}}, { 9, {{33, 132}}}, {13, {{33, 132}}}, {17, {{33, 132}}}},
	{90, { 1, {{27, 129}}}, {10, {{28, 129}}}, {14, {{28, 129}}}, {18, {{28, 129}}}},
	{91, { 3, {{44, 129}}}, { 8, {{35, 129}}}, {12, {{35, 129}}}, {16, {{35, 129}}}},
	{10, {29, {{22, 156}}}, {30, {{22, 156}}}, {31, {{22, 156}}}, {32, {{22, 156}}}},
	{11, {29, {{22, 156}}}, {30, {{22, 156}}}, {31, {{22, 156}}}, {32, {{22, 156}}}},
	{12, {29, {{22, 156}}}, {30, {{22, 156}}}, {31, {{22, 156}}}, {32, {{22, 156}}}},
	{13, {29, {{22, 156}}}, {30, {{22, 156}}}, {31, {{22, 156}}}, {32, {{22, 156}}}},
	{18, {29, {{22, 156}}}, {30, {{22, 156}}}, {31, {{22, 156}}}, {32, {{22, 156}}}},
	{35, {29, {{20, 156}}}, {30, {{20, 156}}}, {31, {{20, 156}}}, {32, {{20, 156}}}},
	{42, {29, {{18, 156}}}, {30, {{18, 156}}}, {31, {{18, 156}}}, {32, {{18, 156}}}},
	{51, {29, {{57, 144}}}, {30, {{57, 144}}}, {31, {{57, 144}}}, {32, {{57, 144}}}},
	{53, {29, {{48, 156}}}, {30, {{48, 156}}}, {31, {{48, 156}}}, {32, {{48, 156}}}},
	{54, {29, {{55, 139}}}, {30, {{55, 139}}}, {31, {{55, 139}}}, {32, {{55, 139}}}},
	{56, {29, {{47, 115}}}, {30, {{47, 115}}}, {31, {{47, 115}}}, {32, {{47, 115}}}},
	{57, {29, {{38, 136}}}, {30, {{38, 136}}}, {31, {{38, 136}}}, {32, {{38, 136}}}},
	{58, {29, {{28, 125}}}, {30, {{28, 125}}}, {31, {{28, 125}}}, {32, {{28, 125}}}},
	{59, {29, {{45, 132}}}, {30, {{45, 132}}}, {31, {{45, 132}}}, {32, {{45, 132}}}},
	{60, {29, {{21, 150}}}, {30, {{21, 150}}}, {31, {{21, 150}}}, {32, {{21, 150}}}},
	{61, {29, {{23, 123}}}, {30, {{23, 123}}}, {31, {{23, 123}}}, {32, {{23, 123}}}}
};

#define TURKEYANIMS_MAX 10

turkeyanims_t turkeyanim_list[TURKEYANIMS_MAX] = {
	{50, {61, {{14, 140}}}, {62, {{14, 140}}}},
	{51, {61, {{14, 143}}}, {62, {{14, 143}}}},
	{53, {61, {{20, 153}}}, {62, {{20, 153}}}},
	{54, {61, {{16, 139}}}, {62, {{16, 139}}}},
	{56, {61, {{24, 118}}}, {62, {{24, 118}}}},
	{57, {61, {{27, 129}}}, {62, {{27, 129}}}},
	{58, {61, {{39, 125}}}, {62, {{39, 125}}}},
	{59, {61, {{29, 130}}}, {62, {{29, 130}}}},
	{60, {61, {{49, 150}}}, {62, {{49, 150}}}},
	{61, {61, {{56, 141}}}, {62, {{56, 141}}}}
};

static const byte cga_color_sels[] = {
	0x30, 0x10, 0x30, 0x10, 0x30, 0x10, 0x10, 0x30, 0x10, 0x10, 0x10, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x10, 0x10, 0x10
};

void SelectSpecificPalette(byte index) {
	cga_ColorSelect(cga_color_sels[index]);
}


void SelectPalette(void) {
	cga_ColorSelect(cga_color_sels[script_byte_vars.palette_index]);
}

/*
Blit sprites to backbuffer
*/
void BlitSpritesToBackBuffer(void) {
	int16 i;
	for (i = 0; i < MAX_SPRITES; i++) {
		byte *sprite = sprites_list[i];
		cga_RestoreImage(sprite, backbuffer);
	}
}

/*
Copy data at sprite's rect from screen to backbuffer
*/
void RefreshSpritesData(void) {
	int16 i;
	for (i = 0; i < MAX_SPRITES; i++) {
		byte *sprite = sprites_list[i];
		cga_RefreshImageData(sprite);
	}
}

/*
Check if packed x/y coordinates are in rect
*/
int16 IsInRect(byte x, byte y, rect_t *rect) {
	if (x < rect->sx) return 0;
	if (x >= rect->ex) return 0;
	if (y < rect->sy) return 0;
	if (y >= rect->ey) return 0;
	return 1;
}

/*
Check if cursor is in rect
*/
int16 IsCursorInRect(rect_t *rect) {
	return IsInRect(cursor_x / CGA_PIXELS_PER_BYTE, cursor_y, rect);
}

/*
Find person for a current spot
*/
void FindPerson(void) {
	int16 i;
	pers_t *pers = pers_list;
	for (i = 0; i < PERS_MAX; i++, pers++) {
		if ((pers->flags & 15) == script_byte_vars.cur_spot_idx) {
			script_vars[ScrPool8_CurrentPers] = pers;
			script_byte_vars.cur_pers = i + 1;
			return;
		}
	}
	script_byte_vars.cur_pers = 0;
}

/*
Select a spot under cursor if its flags are matched given criteria
*/
void CheckHotspots(byte m, byte v) {
	int16 i;
	spot_t *spot = zone_spots;
	for (i = 0; spot != zone_spots_end; i++, spot++) {
		if (IsCursorInRect((rect_t *)spot) && (spot->flags & SPOTFLG_80) && ((spot->flags & m) == v)) {
			script_byte_vars.cur_spot_idx = i + 1;
			spot_sprite = sprites_list + i;
			found_spot = spot;
			script_byte_vars.cur_spot_flags = spot->flags;
			object_hint = spot->hint;
			cursor_color = 0xAA;
			the_command = Swap16(spot->command);
			FindPerson();
			return;
		}
	}
	cursor_color = 0xFF;
	object_hint = zone_name;    /*room name*/
	script_byte_vars.cur_spot_idx = 0;
}

/*
Select cursor shape for current spot
*/
void SelectSpotCursor(void) {
	int16 curs = CURSOR_TARGET;
	CheckHotspots(script_byte_vars.spot_m, script_byte_vars.spot_v);
	if (cursor_color == 0xAA) {
		curs = CURSOR_BODY;
		if ((script_byte_vars.cur_spot_flags & (SPOTFLG_20 | SPOTFLG_10 | SPOTFLG_8)) != SPOTFLG_10) {
			curs = CURSOR_ARROWS;
			if ((script_byte_vars.cur_spot_flags & SPOTFLG_20) != 0)
				curs = CURSOR_CROSSHAIR;
		}
	}
	cursor_shape = souri_data + curs * CURSOR_WIDTH * CURSOR_HEIGHT * 2 / CGA_PIXELS_PER_BYTE;
}

#define kBgW 8
#define kBgH 30

/*blocks draw order (clockwise inward spiral)*/
static const int16 background_draw_steps[] = {
	kBgW, kBgW, kBgW, kBgW, kBgW, kBgW, kBgW,
	kBgH / 2 * CGA_BYTES_PER_LINE, kBgH / 2 * CGA_BYTES_PER_LINE, kBgH / 2 * CGA_BYTES_PER_LINE, kBgH / 2 * CGA_BYTES_PER_LINE, kBgH / 2 * CGA_BYTES_PER_LINE,
	-kBgW, -kBgW, -kBgW, -kBgW, -kBgW, -kBgW, -kBgW, -kBgW,
	-kBgH / 2 * CGA_BYTES_PER_LINE, -kBgH / 2 * CGA_BYTES_PER_LINE, -kBgH / 2 * CGA_BYTES_PER_LINE, -kBgH / 2 * CGA_BYTES_PER_LINE,
	kBgW, kBgW, kBgW, kBgW, kBgW, kBgW, kBgW,
	kBgH / 2 * CGA_BYTES_PER_LINE, kBgH / 2 * CGA_BYTES_PER_LINE, kBgH / 2 * CGA_BYTES_PER_LINE,
	-kBgW, -kBgW, -kBgW, -kBgW, -kBgW, -kBgW,
	-kBgH / 2 * CGA_BYTES_PER_LINE, -kBgH / 2 * CGA_BYTES_PER_LINE,
	kBgW, kBgW, kBgW, kBgW, kBgW,
	kBgH / 2 * CGA_BYTES_PER_LINE,
	-kBgW, -kBgW, -kBgW, -kBgW, 0
};

/*
Draw main backgound pattern, in spiral-like order
*/
void DrawBackground(byte *target, byte vblank) {
	int16 i;
	uint16 offs = (2 / 2) * CGA_BYTES_PER_LINE + 8;   /*TODO: calcxy?*/
	byte *pixels = gauss_data + 0x3C8; /*TODO: better const*/
	for (i = 0; i < 53; i++) {
		/*draw a tile, alternating between two variants*/
		cga_Blit(pixels + (i & 1 ? 0 : kBgW * kBgH), kBgW, kBgW, kBgH, target, offs);
		if (vblank)
			waitVBlank();
		offs += background_draw_steps[i];
	}

	offs = (182 / 2) * CGA_BYTES_PER_LINE;  /*TODO: calcxy?*/
	for (i = 0; i < 9; i++) {
		cga_Blit(pixels, kBgW, kBgW, 9, target, offs);
		offs += kBgW;
	}
}

/*
Load and initialize zone data
*/
void LoadZone(void) {
	byte *zptr, *zend;

	zptr = seekToEntry(zones_data, script_byte_vars.zone_index - 1, &zend);
	script_byte_vars.zone_area = *zptr++;
	script_byte_vars.zone_room = *zptr++;
	zone_name = object_hint = *zptr++;
	zone_palette = script_byte_vars.palette_index = *zptr++;
	zone_obj_count = *zptr++;
	if (zone_obj_count != 0) {
		uint16 i;
		uint16 *zcmds = script_word_vars.zone_obj_cmds;
		memset(script_word_vars.zone_obj_cmds, 0, 15 * 5);  /*half of list: TODO: bug? wipe whole list?*/
		for (i = 0; i < zone_obj_count; i++) {
			/*load spot's reactions*/
			uint16 flags = (*zptr++) << 8;
			flags |= *zptr++;
			if (flags & 0x10) {
				zcmds[0] = zptr[0] | (zptr[1] << 8);    /*TODO: big-endian, but loaded here as le and converted later*/
				zptr += 2;
			}
			if (flags & 8) {
				zcmds[1] = zptr[0] | (zptr[1] << 8);
				zptr += 2;
			}
			if (flags & 4) {
				zcmds[2] = zptr[0] | (zptr[1] << 8);
				zptr += 2;
			}
			if (flags & 2) {
				zcmds[3] = zptr[0] | (zptr[1] << 8);
				zptr += 2;
			}
			if (flags & 1) {
				zcmds[4] = zptr[0] | (zptr[1] << 8);
				zptr += 2;
			}
			zcmds += 5;
		}
	}

	zone_spots = (spot_t *)zptr;
	script_vars[ScrPool4_ZoneSpots] = (spot_t *)zptr;
	zone_spots_end = (spot_t *)zend;
	zone_spots_cur = (spot_t *)zptr;
	zone_spr_index = 0;
	script_byte_vars.dead_flag = 0;
	script_byte_vars.bvar_25 = 0;
	script_word_vars.next_aspirant_cmd = BE(0);
	next_turkey_cmd = 0;
	next_vorts_cmd = 0;
	script_byte_vars.used_commands = 0;
}

void ResetZone(void) {
	script_byte_vars.bvar_43 = 0;
	script_byte_vars.bvar_4B = 0;
	script_byte_vars.bvar_61 = 0;
	script_byte_vars.bvar_45 = 0;
	script_byte_vars.bvar_0B = 0;
	script_byte_vars.bvar_33 = 0;
	script_byte_vars.bvar_34 = 0;
	script_byte_vars.bvar_08 = 0;
	script_byte_vars.aspirant_flags = 0;
	script_word_vars.psi_cmds[0] = BE(0x9048);
	script_word_vars.psi_cmds[1] = BE(0xA01D);
	script_word_vars.psi_cmds[2] = BE(0);
	script_word_vars.psi_cmds[3] = BE(0x00F7);
	script_word_vars.psi_cmds[4] = BE(0x9048);
	script_word_vars.psi_cmds[5] = BE(0x9048);
	script_word_vars.wvar_0C = BE(0xA01C);
	script_word_vars.wvar_0E = BE(0);
	script_word_vars.wvar_AA = BE(0);
}

/*
Load puzzl sprite to buffer, return next free buffer ptr
*/
byte *LoadPuzzl(byte index, byte *buffer) {
	if (script_byte_vars.palette_index == 14)
		return LoadSprite(index, puzzl_data + 4, buffer, 1);
	else
		return LoadSprite(index, puzzl_data + 4, buffer, 0);
}

/*
Load puzzl sprite to scratch buffer, return sprite ptr
*/
byte *LoadPuzzlToScratch(byte index) {
	byte *buffer = scratch_mem2;
	LoadPuzzl(index, buffer);
	return buffer;
}

#define kNumDoorSprites 3

typedef struct doorinfo_t {
	byte flipped;
	struct {
		byte width;
		byte height;
		byte *pixels;
		uint16 offs;
	} layer[kNumDoorSprites];
	byte width;
	byte height;
	uint16 offs;
	byte sprites[1];   /*variable size*/
} doorinfo_t;

byte *doors_list[MAX_DOORS];
byte arpla_y_step;

/*
Fill in sliding door animation information
*/
void InitRoomDoorInfo(byte index) {
	int16 i;
	byte *aptr;
	byte *sprbuf;
	doorinfo_t *info = (doorinfo_t *)scratch_mem2;
	rect_t bounds = {0xFF, 0, 0xFF, 0};

	aptr = doors_list[index - 1];
	info->flipped = (aptr[1] & 0x80) ? ~0 : 0;
	sprbuf = info->sprites;
	for (i = 0; i < kNumDoorSprites; i++) {
		byte x, y, w, h, ox;
		byte *sprite = sprbuf;
		sprbuf = LoadPuzzl(aptr[0], sprbuf);

		x = aptr[1];
		y = aptr[2];
		w = sprite[0];
		h = sprite[1];

		ox = x;
		if (x & 0x80) {
			/*horizontal flip*/
			x = (x + w - 1) & 0x7F;
			ox &= 0x7F;
		}

		y = (y * 2) & 0xFF; /*TODO: disregard vertical flip?*/

		if (ox < bounds.sx)
			bounds.sx = ox;
		if (ox + w >= bounds.ex)
			bounds.ex = ox + w;

		if (y < bounds.sy)
			bounds.sy = y;
		if (y + h >= bounds.ey)
			bounds.ey = y + h;

		info->layer[i].width = w;
		info->layer[i].height = h;
		info->layer[i].pixels = sprite + 2;
		info->layer[i].offs = cga_CalcXY_p(x, y);

		aptr += 3;
	}

	info->width = bounds.ex - bounds.sx;
	info->height = bounds.ey - bounds.sy;
	info->offs = cga_CalcXY_p(bounds.sx, bounds.sy);
}

/*
Draw sliding door
*/
void DrawRoomDoor(void) {
	int16 i;
	doorinfo_t *info = (doorinfo_t *)scratch_mem2;
	for (i = 0; i < kNumDoorSprites; i++) {
		byte w = info->layer[i].width;
		byte h = info->layer[i].height;
		byte *pixels = info->layer[i].pixels;
		uint16 offs = info->layer[i].offs;

		if (!info->flipped)
			cga_BlitSprite(pixels, w * 2, w, h, backbuffer, offs);
		else
			cga_BlitSpriteFlip(pixels, w * 2, w, h, backbuffer, offs);
	}
	waitVBlank();
	waitVBlank();
	cga_CopyScreenBlock(backbuffer, info->width, info->height, frontbuffer, info->offs);
}

/*
Animate sliding door open
*/
void AnimRoomDoorOpen(byte index) {
	int16 i;

	byte oldheight;

	doorinfo_t *info = (doorinfo_t *)scratch_mem2;

	InitRoomDoorInfo(index);

	IFGM_PlaySample(29);

	oldheight = info->layer[1].height;

	for (i = 0; i < oldheight / 2; i++) {
#if 1
		DrawRoomDoor();
#endif
		info->layer[1].height -= 2;
		info->layer[1].pixels += info->layer[1].width * 2 * 2;
	}

	playSound(31);
}

/*
Animate sliding door close
*/
void AnimRoomDoorClose(byte index) {
	int16 i;

	byte oldheight;
	byte *oldpixels;

	doorinfo_t *info = (doorinfo_t *)scratch_mem2;
	InitRoomDoorInfo(index);

	IFGM_PlaySample(29);

	oldheight = info->layer[1].height;
	oldpixels = info->layer[1].pixels;

	info->layer[1].pixels += info->layer[1].width * 2 * (info->layer[1].height - 1);
	info->layer[1].height = 1;

	for (i = 0; i < oldheight / 2; i++) {
#if 1
		DrawRoomDoor();
#endif
		info->layer[1].height += 2;
		info->layer[1].pixels -= info->layer[1].width * 2 * 2;
	}

	info->layer[1].height = oldheight;
	info->layer[1].pixels = oldpixels;
	DrawRoomDoor();

	playSound(31);
}

/*Maybe FindRoomDoor?*/
byte FindInitialSpot(void) {
	spot_t *spot;
	byte index;
	byte flags = script_byte_vars.last_door;
	if (flags == 0)
		return 0;
	flags |= SPOTFLG_80 | SPOTFLG_8;
	for (index = 1, spot = zone_spots; spot != zone_spots_end; index++, spot++) {
		if (spot->flags == flags)
			return index;
	}
	return 0;
}

/*
Find first spot index that matches given flags
*/
byte FindSpotByFlags(byte mask, byte value) {
	spot_t *spot;
	byte index;
	for (index = 1, spot = zone_spots; spot != zone_spots_end; spot++, index++) {
		if ((spot->flags & mask) == value)
			return index;
	}
	return 0xFF;
}

/*
Select person and its spot (if available)
*/
byte SelectPerson(byte offset) {
	/*TODO: replace offset arg with index?*/
	byte index = offset / 5;   /* / sizeof(pers_t) */

	script_vars[ScrPool8_CurrentPers] = &pers_list[index];

	index = FindSpotByFlags(0x3F, (pers_list[index].index & 7) | SPOTFLG_10);   /*TODO: return 0 if not found?*/
	if (index == 0xFF)
		return 0;

	found_spot = &zone_spots[index - 1];
	script_byte_vars.cur_spot_idx = index;
	spot_sprite = &sprites_list[index - 1];
	return 1;
}

/*
Play animation at the selected spot or specified coordinates
*/
void AnimateSpot(const animdesc_t *info) {
	byte *sprite = *spot_sprite;
	cga_RestoreImage(sprite, backbuffer);
	if (info->index & ANIMFLG_USESPOT) {
		/*at selected spot*/
		cursor_x = found_spot->sx * 4;
		cursor_y = found_spot->sy;
		if (info->params.desc)
			drawMessage(seekToString(desci_data, info->params.desc), frontbuffer);

		playAnim(info->index & ~ANIMFLG_USESPOT, found_spot->sx, found_spot->sy);
	} else {
		/*at specified coords*/
		playAnim(info->index, info->params.coords.x, info->params.coords.y);
	}
}

typedef struct lutinanim_t {
	byte phase;
	byte sprites[8];
} lutinanim_t;

lutinanim_t lutins_table[] = {
	{0, { 55, 55, 55, 55, 55, 55, 55, 55} },
	{0, { 41, 41, 41, 41, 41, 41, 41, 41} },
	{0, {  6,  6,  6,  6,  6,  6,  6,  6} },
	{0, { 33, 34, 33, 35, 33, 36, 33, 35} },
	{0, { 10, 11, 10, 13, 10, 12, 10, 14} },
	{0, { 15, 16, 17, 18, 19, 15, 20, 21} },
	{0, { 80, 81, 80, 83, 80, 82, 80, 84} },
	{0, {139, 140, 141, 142, 139, 140, 141, 142} },
	{0, {133, 134, 133, 135, 133, 134, 133, 135} },
	{0, {136, 137, 136, 138, 136, 137, 136, 138} },
	{0, {144, 145, 144, 145, 144, 145, 144, 145} },
	{0, { 33, 34, 33, 35, 33, 36, 33, 35} },
	{0, {181, 182, 181, 183, 181, 182, 181, 183} },
	{0, {178, 179, 178, 180, 178, 179, 178, 180} },
	{0, {181, 182, 181, 183, 181, 182, 181, 183} },
	{0, {185, 186, 187, 188, 185, 186, 187, 188} },
	{0, {185, 186, 187, 188, 185, 186, 187, 188} },
	{0, {164, 132, 164, 132, 164, 132, 164, 132} },
	{0, {144, 145, 144, 145, 144, 145, 144, 145} },
	{0, { 33, 34, 33, 35, 33, 36, 33, 35} },
	{0, {199, 200, 201, 202, 199, 200, 201, 202} },
	{0, {195, 196, 197, 198, 195, 196, 197, 198} },
	{0, {203, 203, 203, 203, 203, 203, 203, 203} },
	{0, {148, 148, 148, 148, 148, 148, 148, 148} },
	{0, {185, 186, 187, 188, 185, 186, 187, 188} },
	{0, {193, 193, 193, 193, 193, 193, 193, 193} },
	{0, {165, 166, 167, 168, 165, 166, 167, 168} },
	{0, { 33, 34, 33, 35, 33, 36, 33, 35} },
	{0, {181, 182, 181, 183, 181, 182, 181, 183} },
	{0, {178, 179, 178, 180, 178, 179, 178, 180} },
	{0, { 91, 91, 91, 91, 91, 91, 91, 91} },
	{0, {221, 221, 221, 221, 221, 221, 221, 221} },
	{0, {  0,  0,  0,  0,  0,  0,  0,  0} }
};

void BeforeChangeZone(byte index) {
	byte oldspot;
	static const animdesc_t anim57 = {ANIMFLG_USESPOT | 57, { { 0, 0 } }};
	static const animdesc_t anim58 = {ANIMFLG_USESPOT | 58, { { 0, 0 } }};

	script_byte_vars.need_draw_spots = 0;
	if (pers_list[kPersScifi].area != script_byte_vars.zone_area)
		return;
	if (index < 59 || index >= 63)
		return;

	oldspot = script_byte_vars.cur_spot_idx;

	script_byte_vars.need_draw_spots = ~0;

	SelectPerson(PersonOffset(kPersScifi));
	AnimateSpot(&anim57);

	if (pers_list[kPersMonkey].area != 0) {
		SelectPerson(PersonOffset(kPersMonkey));
		AnimateSpot(&anim58);
	}

	script_byte_vars.cur_spot_idx = oldspot;
}

void ChangeZone(byte index) {
	byte spridx = 0;

	script_byte_vars.prev_zone_index = script_byte_vars.zone_index;
	script_byte_vars.zone_index = index;

	if (script_byte_vars.bvar_6C != 0)
		spridx = 229;
	else if (script_byte_vars.zone_index == 129)
		spridx = 221;
	else if (script_byte_vars.zone_index == 130)
		spridx = 222;

	if (spridx != 0) {
		int16 i;
		lutinanim_t *la = &lutins_table[31];
		for (i = 0; i < 8; i++)
			la->sprites[i] = spridx;
	}

	LoadZone();
	ResetZone();
}


void DrawPersons(void) {
	int16 i;
	byte index, pidx;
	spot_t *spot;

	for (spot = zone_spots; spot != zone_spots_end; spot++) {
		if ((spot->flags & 0x38) == SPOTFLG_10)
			spot->flags &= ~SPOTFLG_80;
	}

	for (i = 0; i < PERS_MAX; i++) {
		pers_list[i].flags &= ~0xF;

		if (pers_list[i].area != script_byte_vars.zone_area)
			continue;
		if (pers_list[i].flags & PERSFLG_40)
			continue;

		pidx = (pers_list[i].index & 7) | SPOTFLG_10;
		for (index = 1, spot = zone_spots; spot != zone_spots_end; spot++, index++) {
			if ((spot->flags & ~SPOTFLG_40) == pidx) {
				spot->flags |= SPOTFLG_80;
				spot->hint = pers_list[i].name;
				pers_list[i].flags |= index;
				if (spot->flags & SPOTFLG_40)
					DrawZoneAniSprite((rect_t *)spot, index, backbuffer);
				break;
			}
		}
	}
}

/*
Draw room's static object to backbuffer
*/
void DrawRoomStaticObject(byte *aptr, byte *rx, byte *ry, byte *rw, byte *rh) {
	byte x, y, w, h;
	int16 pitch;
	byte *sprite = LoadPuzzlToScratch(aptr[0]);
	x = aptr[1];
	y = aptr[2];
	w = sprite[0];
	h = sprite[1];

	sprite += 2;

	*rx = x & 0x7F;
	*ry = (y & 0x7F) * 2;
	*rw = w;
	*rh = h;

	pitch = w * 2;

	if (x & 0x80) {
		/*horizontal flip*/
		x = (x + w - 1) & 0x7F;
	}

	if (y & 0x80) {
		/*vertical flip*/
		sprite += pitch * (h - 1);
		pitch = -pitch;
	}
	y = (y * 2) & 0xFF;

	if (aptr[0] == 83) { /*Hand sprite from Who Will Be Saved room*/
		if (arpla_y_step & 1)
			y -= 8;
		arpla_y_step >>= 1;
	}

	/*TODO: adjust ry accordingly? SCR_11_DrawRoomObject uses offs from adjusted y, but DrawRoomStatics relies on original y*/
	/*TODO: check if this results in any glitches in Who Will Be Saved*/

	if (aptr[1] & 0x80)
		cga_BlitSpriteFlip(sprite, pitch, w, h, backbuffer, cga_CalcXY_p(x, y));
	else
		cga_BlitSprite(sprite, pitch, w, h, backbuffer, cga_CalcXY_p(x, y));
}

/*
Draw all room's static objects (decorations) to backbuffer
Initialize room bounds rect to room's dimensions
Draw room's name box and text
*/
void DrawRoomStatics(void) {
	byte *aptr, *aend;
	byte doorcount = 0;
	byte x, y, w, h;
	uint16 xx, ww;

	DrawBackground(backbuffer, 0);
	arpla_y_step = script_byte_vars.hands;

	aptr = seekToEntry(arpla_data, script_byte_vars.zone_room - 1, &aend);
	room_bounds_rect.sx = 0xFF;
	room_bounds_rect.ex = 0;
	room_bounds_rect.sy = 0xFF;
	room_bounds_rect.ey = 0;

	/*load and draw room decor*/
	for (; aptr != aend; aptr += 3) {
		byte index = *aptr;
		/*a door ?*/
		if (index >= 50 && index < 61) {
			doors_list[doorcount++] = aptr - 3; /*TODO: check for list overflow?*/
			if (doorcount == script_byte_vars.cur_spot_idx)
				continue;
			if (script_byte_vars.zone_room == 32 && index == 91 && (script_byte_vars.bvar_27 & 8))
				continue;   /*TODO: error? index may never be that high here*/
		}
		/*draw decor object*/
		DrawRoomStaticObject(aptr, &x, &y, &w, &h);

		/*update room's bounding rect*/
		if (x < room_bounds_rect.sx)
			room_bounds_rect.sx = x;
		if (x + w > room_bounds_rect.ex)
			room_bounds_rect.ex = x + w;
		if (y < room_bounds_rect.sy)
			room_bounds_rect.sy = y;
		if (y + h > room_bounds_rect.ey)
			room_bounds_rect.ey = y + h;
	}

	/*place object hint box right under the room*/
	room_hint_bar_width = room_bounds_rect.ex - room_bounds_rect.sx - 2;
	char_draw_coords_x = room_bounds_rect.sx;
	room_hint_bar_coords_x = room_bounds_rect.sx + 1;
	char_draw_coords_y = room_bounds_rect.ey + 2;
	room_hint_bar_coords_y = room_bounds_rect.ey + 2;
	char_xlat_table = chars_color_wonb;

	/*print room name*/
	cga_PrintChar(0x3B, backbuffer);
	DrawObjectHint();                       /* area name */
	cga_PrintChar(0x3C, backbuffer);

	/*draw border around hint text*/
	xx = (room_hint_bar_coords_x - 1) * 4;
	y = room_hint_bar_coords_y;
	ww = (room_hint_bar_width + 2) * 4;

	cga_DrawHLine(xx, y - 2, ww, 2, backbuffer);
	cga_DrawHLine(xx, y - 1, ww, 0, backbuffer);
	cga_DrawHLine(xx, y + 6, ww, 2, backbuffer);
	cga_DrawVLine(xx, y - 2, 9, 2, backbuffer);
	cga_DrawVLine(xx + ww - 1, y - 2, 9, 2, backbuffer);
}

/*
Redraw all room's static objects (decorations) to backbuffer
*/
void RedrawRoomStatics(byte index, byte y_step) {
	byte *aptr, *aend;
	byte x, y, w, h;
	arpla_y_step = y_step;

	aptr = seekToEntry(arpla_data, index - 1, &aend);
	for (; aptr != aend; aptr += 3) {
		/*load room's bg objs*/
		DrawRoomStaticObject(aptr, &x, &y, &w, &h);
	}
}

/*
Draw "some item in the room" icon
*/
void DrawRoomItemsIndicator(void) {
	byte spridx = 172;
	int16 i;
	for (i = 0; i < MAX_INV_ITEMS; i++) {
		if (inventory_items[i].flags == ITEMFLG_ROOM
		        && inventory_items[i].area == script_byte_vars.zone_area) {
			spridx = 173;
			break;
		}
	}
	DrawSpriteN(spridx, 296 / CGA_PIXELS_PER_BYTE, 14, CGA_SCREENBUFFER);
	DrawSpriteN(spridx, 296 / CGA_PIXELS_PER_BYTE, 14, backbuffer);

	/*recalculate the number of zapstiks we have*/
	script_byte_vars.zapstiks_owned = 0;
	for (i = 0; i < 14; i++) {
		if (inventory_items[i + kItemZapstik1 - 1].flags == ITEMFLG_OWNED)
			script_byte_vars.zapstiks_owned++;
	}
}

void DrawZoneSpots(void) {
	static const animdesc_t anim59 = {ANIMFLG_USESPOT | 59, { { 0, 0 } }};
	static const animdesc_t anim60 = {ANIMFLG_USESPOT | 60, { { 0, 0 } }};

	byte oldspot = script_byte_vars.cur_spot_idx;

	if (!script_byte_vars.need_draw_spots)
		return;

	SelectPerson(PersonOffset(kPersScifi));
	AnimateSpot(&anim59);

	pers_list[kPersScifi].area = script_byte_vars.zone_area;

	if (pers_list[kPersMonkey].area != 0) {
		pers_list[kPersMonkey].area = script_byte_vars.zone_area;
		SelectPerson(PersonOffset(kPersMonkey));
		AnimateSpot(&anim60);
	}

	script_byte_vars.cur_spot_idx = oldspot;

	DrawPersons();
}

void RefreshZone(void) {
	popDirtyRects(DirtyRectSprite);
	popDirtyRects(DirtyRectBubble);
	popDirtyRects(DirtyRectText);

	if (!skip_zone_transition && !right_button)
		DrawBackground(CGA_SCREENBUFFER, 1);

	cga_BackBufferToRealFull();

	in_de_profundis = 0;
	IFGM_StopSample();
	if (script_byte_vars.zone_area == 22) {
		IFGM_PlaySample(131);
		in_de_profundis = 1;
	}

	DrawTheWallDoors();
	SelectPalette();
	DrawRoomItemsIndicator();
	DrawZoneSpots();
	if (script_byte_vars.cur_spot_idx != 0)
		AnimRoomDoorClose(script_byte_vars.cur_spot_idx);
	BlitSpritesToBackBuffer();
}

/*
Draw object hint or zone name text to backbuffer
*/
void DrawObjectHint(void) {
	if (script_byte_vars.zone_index == 135)
		return;
	char_draw_max_width = room_hint_bar_width;
	char_draw_coords_x = room_hint_bar_coords_x;
	char_draw_coords_y = room_hint_bar_coords_y;
	char_xlat_table = chars_color_wonb;
	printStringCentered(seekToString(motsi_data, object_hint), backbuffer);

#ifdef DEBUG_ZONE
	cga_PrintChar(0x20, backbuffer);
	cga_PrintChar(0x10 + script_byte_vars.zone_index / 100, backbuffer);
	cga_PrintChar(0x10 + (script_byte_vars.zone_index / 10) % 10, backbuffer);
	cga_PrintChar(0x10 + script_byte_vars.zone_index % 10, backbuffer);
#endif
}

/*
Copy object hint from backbuffer to screen
*/
void ShowObjectHint(byte *target) {
	if (script_byte_vars.zone_index == 135)
		return;
	cga_CopyScreenBlock(backbuffer, room_hint_bar_width + 2, 9, target, cga_CalcXY_p(room_hint_bar_coords_x - 1, room_hint_bar_coords_y - 2));
}

/*
Draw command hint text to backbuffer
*/
void DrawCommandHint(void) {
	char_draw_max_width = cmd_hint_bar_width;
	char_draw_coords_x = cmd_hint_bar_coords_x;
	char_draw_coords_y = cmd_hint_bar_coords_y;
	char_xlat_table = chars_color_wonb;
	printStringCentered(seekToString(vepci_data, command_hint), backbuffer);
}

/*
Copy command hint from backbuffer to screen
*/
void ShowCommandHint(byte *target) {
	cga_CopyScreenBlock(backbuffer, cmd_hint_bar_width + 2, 9, target, cga_CalcXY_p(cmd_hint_bar_coords_x - 1, cmd_hint_bar_coords_y - 2));
}

void LoadLutinSprite(uint16 lutidx) {
	byte spridx;
	uint16 flags;
	byte *lutin_entry, *lutin_entry_end;
	byte *buffer;
	byte *sprite;
	byte sprw, sprh;

	uint16 i;

	buffer = lutin_mem;

	for (i = 0; i < 800; i++) { /*TODO: fix size*/
		buffer[i * 2] = 0xFF;   /*mask*/
		buffer[i * 2 + 1] = 0;  /*pixels*/
	}

	lutin_entry = seekToEntry(lutin_data, lutidx, &lutin_entry_end);

	*buffer++ = *lutin_entry++; /*width*/
	*buffer++ = *lutin_entry++; /*height*/

	for (; lutin_entry != lutin_entry_end;) {
		spridx = *lutin_entry++;
		flags = *lutin_entry++;
		flags |= (*lutin_entry++) << 8;

		sprite = LoadSprit(spridx);
		sprw = *sprite++;
		sprh = *sprite++;

		buffer = lutin_mem + 2 + (flags & 0x7FFF) * 2;
		if (flags & 0x8000)
			MergeSpritesDataFlip(buffer, lutin_mem[0] * 2, sprite, sprw, sprh);
		else
			MergeSpritesData(buffer, lutin_mem[0] * 2, sprite, sprw, sprh);
	}
}

/*
Draw specific room's person idle sprite
*/
void DrawCharacterSprite(byte spridx, byte x, byte y, byte *target) {
	lutin_mem = scratch_mem2;

	LoadLutinSprite(spridx);

	DrawSprite(scratch_mem2, target, cga_CalcXY_p(x, y));
}

/*
Draw room's person idle sprite and advance sprite's animation
Return true if a sprite was drawn
*/
char DrawZoneAniSprite(rect_t *rect, uint16 index, byte *target) {
	int16 i;
	byte spridx;
	pers_t *pers = pers_list;
	for (i = 0; i < PERS_MAX; i++, pers++) {
		if ((pers->flags & 15) == index) {
			lutinanim_t *la = &lutins_table[pers->index >> 3];
			spridx = la->sprites[la->phase];
			la->phase = (la->phase + 1) % 8;

			lutin_mem = scratch_mem2;

			LoadLutinSprite(spridx);

			zsprite_w = scratch_mem2[0];
			zsprite_h = scratch_mem2[1];
			zsprite_draw_ofs = cga_CalcXY_p(rect->sx, rect->sy);

			DrawSprite(scratch_mem2, target, zsprite_draw_ofs);

			return ~0;
		}
	}
	return 0;
}

/*
Initialize Aspirant
*/
void PrepareAspirant(void) {
	byte index;
	byte hostility, appearance;
	byte flags;

	if (script_byte_vars.zone_area == kAreaDreamsOfSlime) {
		pers_list[kPersAspirant1].area = kAreaDreamsOfSlime;
		pers_list[kPersAspirant2].area = kAreaDreamsOfSlime;
		pers_list[kPersAspirant3].area = kAreaDreamsOfSlime;
		pers_list[kPersAspirant1].name = 50; /*DIVO*/
		pers_list[kPersAspirant2].name = 50;
		pers_list[kPersAspirant3].name = 50;
		return;
	}

	pers_list[kPersAspirant1].area = kAreaNone;
	pers_list[kPersAspirant2].area = kAreaNone;
	pers_list[kPersAspirant3].area = kAreaNone;
	pers_list[kPersAspirant4].area = kAreaNone;

	if (script_byte_vars.bvar_26 >= 63)
		return;

	if (script_byte_vars.zone_area >= kAreaPassage1)
		return;

	index = FindSpotByFlags(0x3F, 17);
	if (index == 0xFF)
		return;
	aspirant_spot = &zone_spots[index - 1];

	script_byte_vars.aspirant_pers_ofs += 5;   /*FIXME: this is sizeof(pers_t), but it's used in scripts, so hardcoded*/
	if (script_byte_vars.aspirant_pers_ofs >= PersonOffset(kPersAspirant4 + 1))
		script_byte_vars.aspirant_pers_ofs = PersonOffset(kPersAspirant1);
	aspirant_ptr = &pers_list[script_byte_vars.aspirant_pers_ofs / 5];

	if (aspirant_ptr->flags & PERSFLG_40)
		return;

	hostility = script_byte_vars.rand_value;
	appearance = Rand();
	flags = 0;
	/*
	flags values:
	     0 - passive
		 1 - willing to trade
		 2 - hostile
		|4 - ?
	*/

	if (script_byte_vars.zone_area <= kAreaTheMastersOrbit3) {
		/*at The Master's Orbit*/
		if (hostility < 90 && aspirant_ptr->item) {
			/* ROPE, STONE FLY, GOBLET, LANTERN, but not DAGGER*/
			if ((aspirant_ptr->item >= kItemRope1 && aspirant_ptr->item <= kItemLantern4)
			&& !(aspirant_ptr->item >= kItemDagger1 && aspirant_ptr->item <= kItemDagger4))
				flags = 1;	/*willing to trade*/
		}
		if (appearance < 23) {
			/*spawn in this room*/
			aspirant_ptr->area = script_byte_vars.zone_area;
			script_word_vars.next_aspirant_cmd = BE(0xA018);	/*leave*/
			script_byte_vars.check_used_commands = 3;			/*after 3 actions*/
			script_byte_vars.aspirant_flags = flags;
			script_vars[ScrPool8_CurrentPers] = aspirant_ptr;
		} else if (appearance < 52) {
			script_word_vars.next_aspirant_cmd = BE(0xA019);
			flags |= 4;
			script_byte_vars.check_used_commands = 3;
			script_byte_vars.aspirant_flags = flags;
			script_vars[ScrPool8_CurrentPers] = aspirant_ptr;
		} else {
			/*do not spawn*/
			script_byte_vars.aspirant_flags = 0;
			return;
		}
	} else {
		/*at other places*/
		if (hostility < 39 && aspirant_ptr->item)
			flags = 1;	/*willing to trade*/
		if (hostility >= 166)
			flags = 2;	/*hostile*/

		if (appearance < 26) {
			/*spawn in this room*/
			aspirant_ptr->area = script_byte_vars.zone_area;
			script_word_vars.next_aspirant_cmd = BE(0xA018);	/*leave*/
			script_byte_vars.check_used_commands = 3;			/*after 3 actions*/
			script_byte_vars.aspirant_flags = flags;
			script_vars[ScrPool8_CurrentPers] = aspirant_ptr;
		} else if (appearance < 52) {
			script_word_vars.next_aspirant_cmd = BE(0xA019);
			flags |= 4;
			script_byte_vars.check_used_commands = 3;
			script_byte_vars.aspirant_flags = flags;
			script_vars[ScrPool8_CurrentPers] = aspirant_ptr;
		} else {
			/*do not spawn*/
			script_byte_vars.aspirant_flags = 0;
			return;
		}
	}
}

/*
Initialize Vorts
*/
void PrepareVorts(void) {
	spot_t *spot;

	if ((script_byte_vars.zone_area != kAreaTheReturn) || !(script_byte_vars.bvar_36 & 0x80)) {
		pers_list[kPersVort].flags &= ~PERSFLG_40;
		pers_list[kPersVort2].flags &= ~PERSFLG_40;
		pers_list[kPersVort3].flags &= ~PERSFLG_40;

		for (spot = zone_spots; spot != zone_spots_end; spot++) {
			if ((spot->flags & ~SPOTFLG_80) == (SPOTFLG_40 | SPOTFLG_10)) {
				int16 i;
				for (i = 0; i < VORTANIMS_MAX; i++) {
					if (vortsanim_list[i].room == script_byte_vars.zone_room) {
						vortanims_ptr = &vortsanim_list[i];
						if (script_byte_vars.zone_area == pers_list[kPersVort].area
						        || script_byte_vars.zone_area == pers_list[kPersVort2].area
						        || script_byte_vars.zone_area == pers_list[kPersVort3].area) {
							next_vorts_cmd = 0xA015;                 /*VortLeave*/
						} else {
							pers_list[kPersVort].area = 0;
							pers_list[kPersVort2].area = 0;
							pers_list[kPersVort3].area = 0;
							script_byte_vars.bvar_36 &= 0x80;    /*TODO: is this correct? |= ?*/
							if (script_byte_vars.rand_value < 39) {
								pers_list[kPersVort].area = script_byte_vars.zone_area;
								next_vorts_cmd = 0xA015;             /*VortLeave*/
							} else if (script_byte_vars.rand_value < 78)
								next_vorts_cmd = 0xA014;             /*VortAppear*/
							else
								return;
						}
						script_byte_vars.bvar_36 |= 1;
						next_vorts_ticks = Swap16(script_word_vars.timer_ticks2) + 5;
						return;
					}
				}
				break;
			}
		}
	}
	pers_list[kPersVort].area = 0;
	pers_list[kPersVort2].area = 0;
	pers_list[kPersVort3].area = 0;
	script_byte_vars.bvar_36 &= 0x80;    /*TODO: is this correct? |= ?*/
}

/*
Initialize Turkey
*/
void PrepareTurkey(void) {
	spot_t *spot;

	if (script_byte_vars.zone_area == kAreaPlacatingThePowers && script_byte_vars.bvar_4E == 0) {
		pers_list[kPersTurkey].area = kAreaPlacatingThePowers;
		return;
	}

	pers_list[kPersTurkey].flags &= ~PERSFLG_40;

	for (spot = zone_spots; spot != zone_spots_end; spot++) {
		if ((spot->flags & ~SPOTFLG_80) == (SPOTFLG_40 | SPOTFLG_10 | SPOTFLG_1)) {
			int16 i;

			for (i = 0; i < TURKEYANIMS_MAX; i++) {
				if (turkeyanim_list[i].room == script_byte_vars.zone_room) {
					turkeyanims_ptr = &turkeyanim_list[i];
					if (script_byte_vars.zone_area == pers_list[kPersTurkey].area) {
						next_turkey_cmd = 0xA01F;	/*leave*/
						next_turkey_ticks = Swap16(script_word_vars.timer_ticks2) + 5;
					} else {
						pers_list[kPersTurkey].area = 0;
						if (script_byte_vars.rand_value >= 217) {
							pers_list[kPersTurkey].area = script_byte_vars.zone_area;
							next_turkey_cmd = 0xA01F;	/*leave*/
							next_turkey_ticks = Swap16(script_word_vars.timer_ticks2) + 5;
						} else if (script_byte_vars.rand_value >= 178) {
							next_turkey_cmd = 0xA01E;	/*appear*/
							next_turkey_ticks = Swap16(script_word_vars.timer_ticks2) + 5;
						}
					}
					return;
				}
			}
			break;
		}
	}

	pers_list[kPersTurkey].area = 0;
}


/*
Load puzzl sprite to scratch and init draw params
*/
uint16 GetPuzzlSprite(byte index, byte x, byte y, uint16 *w, uint16 *h, uint16 *ofs) {
	byte *spr = LoadPuzzlToScratch(index);
	*w = spr[0];
	*h = spr[1];
	*ofs = cga_CalcXY_p(x, y);
	return 0;   /*sprite offset in scratch buf*/
}

/*
Save specific fully drawn rooms to backbuffer
*/
void BackupScreenOfSpecialRoom(void) {
	switch (script_byte_vars.zone_room) {
	case 41: /* THE POWERS OF THE ABYSS */
	case 22: /* DE PROFUNDIS */
	case 23: /* DE PROFUNDIS */
	case 24: /* THE WALL */
		cga_RealBufferToBackFull();
		break;
	}
}

/*
Fully redraw specific rooms
*/
void RestoreScreenOfSpecialRoom(void) {
	switch (script_byte_vars.zone_room) {
	case 23: /* DE PROFUNDIS */
	case 24: /* DE PROFUNDIS */
		RedrawRoomStatics(script_byte_vars.zone_room, 0);
		break;
	}
}

/*
Modify anim sprite 127
*/
void SetAnim127Sprite(byte flags, byte spridx) {
	byte *lutin_entry, *lutin_entry_end;
	lutin_entry = seekToEntry(lutin_data, 127, &lutin_entry_end);
	lutin_entry[2] = spridx;
	switch (spridx) {
	case 37:	/*DAGGER*/
	case 58:	/*CHOPPER*/
	case 40:	/*SACRIFICIAL BLADE*/
		if (flags == ITEMFLG_OWNED)
			script_byte_vars.bvar_66 += 1;
		else
			script_byte_vars.bvar_66 -= 1;
		break;
	}
}

/*
Bounce current item to the room/inventory
*/
void BounceCurrentItem(byte flags, byte y) {
	item_t *item = (item_t *)(script_vars[ScrPool3_CurrentItem]);

	SetAnim127Sprite(flags, item->sprite);
	item->flags = flags;
	item->area = script_byte_vars.zone_area;
	BackupScreenOfSpecialRoom();
	playAnim(41, 176 / 4, y);
	DrawRoomItemsIndicator();
	RestoreScreenOfSpecialRoom();
}

/*
Load The Wall gate sprites
*/
byte *LoadMursmSprite(byte index) {
	byte *pinfo, *end;
	pinfo = seekToEntry(mursm_data, index, &end);

	while (pinfo != end) {
		uint16 flags;
		int16 pitch;
		byte *buffer, *sprite;
		byte sprw, sprh;

		index = *pinfo++;
		flags = *pinfo++;
		flags |= (*pinfo++) << 8;
		buffer = sprit_load_buffer + (flags & 0x3FFF);
		pitch = 20;

		sprite = LoadPuzzlToScratch(index & 0x7F);
		sprw = *sprite++;
		sprh = *sprite++;

		if (index & 0x80)     /*horizontal flip*/
			mergeImageAndSpriteDataFlip(buffer, pitch, sprite, sprw, sprh);
		else
			mergeImageAndSpriteData(buffer, pitch, sprite, sprw, sprh);
	}

	/*TODO: move this from globals to args/consts?*/
	cur_frame_width = 20;
	cur_image_coords_y = 32;
	cur_image_size_w = 80 / 4;
	cur_image_size_h = 59;

	return sprit_load_buffer;
}

thewalldoor_t the_wall_doors[2];

/*
Open The Wall's right gate
TODO: move this to CGA?
*/
void TheWallOpenRightDoor(byte x, byte y, byte width, byte height, byte limit) {
	uint16 offs = cga_CalcXY_p(x + width - 2, y);

	while (--width) {
		cga_HideScreenBlockLiftToRight(1, CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);
		if (width == limit)
			return;
	}

	offs++;

	/*hide remaining column*/
	uint16 ooffs = offs;
	byte oh = height;
	while (height--) {
		memcpy(frontbuffer + offs, backbuffer + offs, 1);

		offs ^= CGA_ODD_LINES_OFS;
		if ((offs & CGA_ODD_LINES_OFS) == 0)
			offs += CGA_BYTES_PER_LINE;
	}

	cga_blitToScreen(ooffs, 1, oh);
}

/*
Open The Wall's left gate
TODO: move this to CGA?
*/
void TheWallOpenLeftDoor(byte x, byte y, byte width, byte height, byte limit) {
	uint16 offs = cga_CalcXY_p(x + 1, y);

	while (--width) {
		cga_HideScreenBlockLiftToLeft(1, CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);
		if (width == limit)
			return;
	}

	offs--;

	/*hide remaining column*/
	uint16 ooffs = offs;
	byte oh = height;
	while (height--) {
		memcpy(frontbuffer + offs, backbuffer + offs, 1);

		offs ^= CGA_ODD_LINES_OFS;
		if ((offs & CGA_ODD_LINES_OFS) == 0)
			offs += CGA_BYTES_PER_LINE;
	}
	cga_blitToScreen(ooffs, 1, oh);
}

/*
Animate The Wall doors
Phase 3: Fully closed -> Half opened
*/
void TheWallPhase3_DoorOpen1(void) {
	script_byte_vars.zone_index = (script_byte_vars.zone_index == 95) ? 9 : 102;
	LoadZone();

	TheWallOpenRightDoor(144 / 4, 32, 80 / 4, 59, 40 / 4);
	TheWallOpenLeftDoor(64 / 4, 32, 80 / 4, 59, 40 / 4);

	IFGM_StopSample();

	/*TODO: fill in the_wall_door_* structures, as they are used by the original code and appear in savefile*/
}

/*
Animate The Wall doors
Phase 0: Half opened -> Fully opened
*/
void TheWallPhase0_DoorOpen2(void) {
	script_byte_vars.zone_index = (script_byte_vars.zone_index == 9) ? 24 : 30;
	LoadZone();

	TheWallOpenRightDoor((144 + 40) / 4, 32, (80 - 40) / 4, 59, 0);
	TheWallOpenLeftDoor(64 / 4, 32, (80 - 40) / 4, 59, 0);

	IFGM_StopSample();

	/*TODO: fill in the_wall_door_* structures, as they are used by the original code and appear in savefile*/
}

/*
Animate The Wall doors
Phase 1: Opened -> Half closed
*/
void TheWallPhase1_DoorClose1(void) {
	byte *spr;

	script_byte_vars.zone_index = (script_byte_vars.zone_index == 24) ? 9 : 102;
	LoadZone();

	spr = LoadMursmSprite(0);
	spr += cur_frame_width - 1;
	cur_image_coords_x = 64 / 4;
	cga_AnimLiftToRight(10, spr, cur_frame_width, 1, cur_image_size_h, frontbuffer, cga_CalcXY_p(cur_image_coords_x, cur_image_coords_y));

	spr = LoadMursmSprite(1);
	cur_image_coords_x = 220 / 4;
	cga_AnimLiftToLeft(10, spr, cur_frame_width, 1, cur_image_size_h, frontbuffer, cga_CalcXY_p(cur_image_coords_x, cur_image_coords_y));

	IFGM_StopSample();

	/*TODO: fill in the_wall_door_* structures, as they are used by the original code and appear in savefile*/
}

/*
Animate The Wall doors
Phase 2: Half closed -> Fully closed
*/
void TheWallPhase2_DoorClose2(void) {
	byte *spr;

	script_byte_vars.zone_index = (script_byte_vars.zone_index == 9) ? 95 : 103;
	LoadZone();

	spr = LoadMursmSprite(0);
	spr += cur_frame_width - 1;
	cur_image_coords_x = 64 / 4;
	cga_AnimLiftToRight(10, spr - 10, cur_frame_width, 1 + 10, cur_image_size_h, frontbuffer, cga_CalcXY_p(cur_image_coords_x, cur_image_coords_y));

	spr = LoadMursmSprite(1);
	cur_image_coords_x = 220 / 4;
	cga_AnimLiftToLeft(10, spr, cur_frame_width, 1 + 10, cur_image_size_h, frontbuffer, cga_CalcXY_p(cur_image_coords_x, cur_image_coords_y) - 10);

	IFGM_PlaySample(30);

	/*TODO: fill in the_wall_door_* structures, as they are used by the original code and appear in savefile*/
}

/*
Draw default The Wall doors
*/
void DrawTheWallDoors(void) {
	switch (script_byte_vars.zone_index) {
	case 9:
	case 102:
		cga_Blit(LoadMursmSprite(0) + 10, 20, 10, 59, CGA_SCREENBUFFER, cga_CalcXY_p(64 / CGA_PIXELS_PER_BYTE, 32));
		if (g_vm->getLanguage() == Common::EN_USA) {
			/*This fixes odd black patch on the right gate door*/
			cga_Blit(LoadMursmSprite(1)     , 20, 10, 59, CGA_SCREENBUFFER, cga_CalcXY_p(184 / CGA_PIXELS_PER_BYTE, 32));
		} else {
			cga_Blit(LoadMursmSprite(1)     , 20, 10, 59, CGA_SCREENBUFFER, cga_CalcXY_p(180 / CGA_PIXELS_PER_BYTE, 32));
		}
		break;
	case 95:
	case 103:
		cga_Blit(LoadMursmSprite(0), 20, 20, 59, CGA_SCREENBUFFER, cga_CalcXY_p(64 / CGA_PIXELS_PER_BYTE, 32));
		cga_Blit(LoadMursmSprite(1), 20, 20, 59, CGA_SCREENBUFFER, cga_CalcXY_p(144 / CGA_PIXELS_PER_BYTE, 32));
		break;
	}
}

/*
Superimpose source sprite data over target sprite data
*/
void MergeSpritesData(byte *target, uint16 pitch, byte *source, uint16 w, uint16 h) {
	uint16 x;
	while (h--) {
		for (x = 0; x < w; x++) {
			byte m = *source++;
			*target++ &= m;
			*target &= m;
			*target++ |= *source++;
		}
		target -= w * 2;
		target += pitch;
	}
}

/*
Superimpose horizontally-flipped source sprite data over target sprite data
*/
void MergeSpritesDataFlip(byte *target, uint16 pitch, byte *source, uint16 w, uint16 h) {
	uint16 x;
	target += w * 2 - 2;
	while (h--) {
		for (x = 0; x < w; x++) {
			byte m = cga_pixel_flip[*source++];
			*target++ &= m;
			*target &= m;
			*target |= cga_pixel_flip[*source++];
			target -= 3;
		}
		target += w * 2;
		target += pitch;
	}
}

/*
Save image at the rect to buffer
Return current and next free buffer ptr
*/
byte *BackupSpotImage(spot_t *spot, byte **spotback, byte *buffer) {
	*spotback = buffer;
	buffer = cga_BackupImage(backbuffer, cga_CalcXY_p(spot->sx, spot->sy), spot->ex - spot->sx, spot->ey - spot->sy, buffer);
	return buffer;
}

/*
Save zone spot images to sprites list
*/
void BackupSpotsImages(void) {
	spot_t *spot = zone_spots;
	byte *buffer = scratch_mem1;
	int16 i;
	for (i = 0; i < MAX_SPRITES; i++)
		sprites_list[i] = 0;
	for (i = 0; spot != zone_spots_end; spot++, i++) { /*TODO: maybe don't advance it if spot is skipped?*/
		if (spot->flags & SPOTFLG_40)
			buffer = BackupSpotImage(spot, &sprites_list[i], buffer);
	}
}

/*
Animate all room's persons, one per call
TODO: rename me
*/
void DrawSpots(byte *target) {
	spot_t *spot = zone_spots_cur;
	byte spridx = zone_spr_index;
	if (spot == zone_spots_end) {
		spot = zone_spots;
		spridx = 0;
	}

	do {
		spridx++;
		if ((spot->flags & SPOTFLG_40) && ((spot->flags & ~7) == (SPOTFLG_80 | SPOTFLG_40 | SPOTFLG_10))) {
			zone_spots_cur = spot + 1;
			zone_spr_index = spridx;
			/*TODO: subclass spot_t from rect_t*/
			if (DrawZoneAniSprite((rect_t *)spot, (spot - zone_spots) + 1, backbuffer)) {
				updateCursor();
				waitVBlank();
				undrawCursor(target);
				cga_CopyScreenBlock(backbuffer, zsprite_w, zsprite_h, target, zsprite_draw_ofs);
				drawCursor(target);
				cga_RestoreImage(sprites_list[zone_spr_index - 1], backbuffer);
				return;
			}
			spridx = zone_spr_index;    /*TODO: not neded?*/
		}
		spot++;
	} while (spot != zone_spots_end);
	zone_spots_cur = spot;
	zone_spr_index = spridx;
}

/*
Animate room's persons at fixed rate
TODO: rename me
*/
void AnimateSpots(byte *target) {
	if (script_byte_vars.timer_ticks % 32 == 31)
		DrawSpots(target);
}

/*
Draw cursor and hints text on screen
*/
void DrawHintsAndCursor(byte *target) {
	updateCursor();
	waitVBlank();
	undrawCursor(target);
	if (object_hint != last_object_hint) {
		ShowObjectHint(target);
		last_object_hint = object_hint;
	}

	if (command_hint != last_command_hint) {
		ShowCommandHint(target);
		last_command_hint = command_hint;
	}

	drawCursor(target);
}

/*
Hide a person
*/
void HidePerson(byte offset) {
	SelectPerson(offset);
	found_spot->flags &= ~SPOTFLG_80;
}

const byte patrol_route[] = {
	kAreaGuardRoom, kAreaTheConcourse,
	kAreaPassage1, kAreaTheRing2,
	kAreaTheRing1, kAreaTheRing6,
	kAreaTheRing5, kAreaTheRing4,
	kAreaTheRing3, kAreaTheRing2,
	kAreaPassage1, kAreaTheConcourse};
const byte *timed_seq_ptr = patrol_route;

/*
Update Protozorqs locations
*/
void UpdateProtozorqs(void) {
	uint16 elapsed;

	if (script_byte_vars.bvar_45 != 0)
		return;

	if (script_byte_vars.bvar_26 >= 63)
		return;

	script_word_vars.next_protozorqs_cmd = BE(0);

	if (pers_list[kPersProtozorq5].flags & PERSFLG_40) {
		pers_list[kPersProtozorq5].area = kAreaGuardRoom;
		pers_list[kPersProtozorq5].flags &= ~(PERSFLG_10 | PERSFLG_20 | PERSFLG_40 | PERSFLG_80);
		pers_list[kPersProtozorq5].index = 52;
	}

	if (pers_list[kPersProtozorq14].flags & PERSFLG_20)
		script_byte_vars.bvar_30 |= 4;

	elapsed = Swap16(script_word_vars.timer_ticks2);
	if (elapsed < next_protozorqs_ticks)
		return;

	next_protozorqs_ticks = elapsed + 30;

	script_byte_vars.bvar_3F = *timed_seq_ptr++;
	if (timed_seq_ptr == patrol_route + sizeof(patrol_route))
		timed_seq_ptr = patrol_route;
	script_byte_vars.bvar_40 = *timed_seq_ptr++;

	if (script_byte_vars.bvar_3F == kAreaGuardRoom) {
		script_byte_vars.bvar_30 &= ~4;
		pers_list[kPersProtozorq14].flags &= ~(PERSFLG_20 | PERSFLG_40 | PERSFLG_80);
		pers_list[kPersProtozorq14].index = 55;
		if (pers_list[kPersProtozorq5].area == kAreaGuardRoom) {
			pers_list[kPersProtozorq5].flags &= ~(PERSFLG_20 | PERSFLG_40 | PERSFLG_80);
			script_byte_vars.bvar_41 = 1;
		}
	}

	if (script_byte_vars.bvar_3F == kAreaTheRing2 && script_byte_vars.bvar_40 == kAreaTheRing1) {
		script_byte_vars.bvar_41 = 0;
		if (pers_list[kPersProtozorq5].area == kAreaGuardRoom) {
			script_byte_vars.bvar_40 = kAreaPassage1;
			timed_seq_ptr = patrol_route + 4;
		}

	}

	pers_list[kPersProtozorq14].area = script_byte_vars.bvar_40;

	if (script_byte_vars.bvar_41 != 0) {
		pers_list[kPersProtozorq5].area = script_byte_vars.bvar_40;
		pers_list[kPersProtozorq12].area = script_byte_vars.bvar_40;
		pers_list[kPersProtozorq13].area = script_byte_vars.bvar_40;

		if (script_byte_vars.zone_area == script_byte_vars.bvar_3F) {
			static const animdesc_t anim35 = {ANIMFLG_USESPOT | 35, { { 0, 0 } }};
			updateUndrawCursor(frontbuffer);
			RefreshSpritesData();
			HidePerson(PersonOffset(kPersProtozorq14));
			HidePerson(PersonOffset(kPersProtozorq5));
			HidePerson(PersonOffset(kPersProtozorq12));
			HidePerson(PersonOffset(kPersProtozorq13));
			AnimateSpot(&anim35);
			BlitSpritesToBackBuffer();
			drawCursor(frontbuffer);
			return;
		}

		if (script_byte_vars.zone_area == script_byte_vars.bvar_40) {
			static const animdesc_t anim34 = {ANIMFLG_USESPOT | 34, { { 0, 0 } }};
			updateUndrawCursor(frontbuffer);
			RefreshSpritesData();

			SelectPerson(PersonOffset(kPersProtozorq14));
			AnimateSpot(&anim34);

			if (SelectPerson(PersonOffset(kPersProtozorq12))) {
				AnimateSpot(&anim34);

				SelectPerson(PersonOffset(kPersProtozorq5));
				AnimateSpot(&anim34);

				SelectPerson(PersonOffset(kPersProtozorq13));
				AnimateSpot(&anim34);
			}

			DrawPersons();
			cga_BackBufferToRealFull();
			BlitSpritesToBackBuffer();
			drawCursor(frontbuffer);

			if (script_byte_vars.zapstiks_owned != 0)
				script_word_vars.next_protozorqs_cmd = BE(0xC1FD);
			else if (pers_list[kPersProtozorq14].flags & PERSFLG_20)
				script_word_vars.next_protozorqs_cmd = BE(0xC1E5);
			else if (script_byte_vars.zone_area == kAreaPassage1)
				script_word_vars.next_protozorqs_cmd = BE(0xC060);

			return;
		}

	} else {
		if (pers_list[kPersProtozorq14].flags & PERSFLG_40)
			return;

		if (script_byte_vars.zone_area == script_byte_vars.bvar_3F) {
			static const animdesc_t anim35 = {ANIMFLG_USESPOT | 35, { { 0, 0 } }};
			updateUndrawCursor(frontbuffer);
			RefreshSpritesData();
			HidePerson(PersonOffset(kPersProtozorq14));
			AnimateSpot(&anim35);
			BlitSpritesToBackBuffer();
			drawCursor(frontbuffer);
			return;
		}

		if (script_byte_vars.zone_area == script_byte_vars.bvar_40) {
			static const animdesc_t anim34 = {ANIMFLG_USESPOT | 34, { { 0, 0 } }};
			updateUndrawCursor(frontbuffer);
			RefreshSpritesData();

			SelectPerson(PersonOffset(kPersProtozorq14));
			AnimateSpot(&anim34);

			DrawPersons();
			cga_BackBufferToRealFull();
			BlitSpritesToBackBuffer();
			drawCursor(frontbuffer);

			return;
		}
	}
}

/*
Check how many time have passed and call the guards
*/
void CheckGameTimeLimit(void) {
	uint16 elapsed = Swap16(script_word_vars.timer_ticks2);

	if (elapsed < 60 * 60)
		return;

	if (script_byte_vars.bvar_26 < 63) {
		script_byte_vars.bvar_37 = 3;
		script_byte_vars.bvar_26 = (script_byte_vars.zone_area < kAreaPassage1) ? 255 : 63;
		pers_list[kPersProtozorq5].area = 0;
		pers_list[kPersProtozorq6].area = 0;
		pers_list[kPersProtozorq7].area = 0;
		pers_list[kPersProtozorq8].area = 0;
		pers_list[kPersProtozorq10].area = 0;
		pers_list[kPersProtozorq11].area = kAreaGuardRoom;
		pers_list[kPersProtozorq12].area = kAreaGuardRoom;
		pers_list[kPersProtozorq13].area = kAreaGuardRoom;
		pers_list[kPersProtozorq14].area = kAreaGuardRoom;
		if (script_byte_vars.zone_area < kAreaDeProfundis)
			script_word_vars.next_protozorqs_cmd = BE(0xC012);
		return;
	}

	if (elapsed < 105 * 60)
		return;

	if (elapsed < 120 * 60) {
		script_byte_vars.bvar_37 = 2;
		script_byte_vars.bvar_4E = 1;
		if ((script_byte_vars.zone_area != kAreaBirthOfADivineRace)
		        && (script_byte_vars.zone_area >= kAreaPlacatingThePowers && script_byte_vars.zone_area <= kAreaCell4))
			script_word_vars.next_protozorqs_cmd = BE(0xC012);
		return;
	}

	script_word_vars.next_protozorqs_cmd = BE(0xC318);	/*game over*/
}

/*
Clean up dropped items after some time
*/
void CleanupDroppedItems(void) {
	int16 i;

	if (Swap16(script_word_vars.timer_ticks2) - drops_cleanup_time < 180)
		return;
	drops_cleanup_time = Swap16(script_word_vars.timer_ticks2);

	for (i = 0; i < MAX_INV_ITEMS; i++) {
		if (inventory_items[i].flags & ITEMFLG_ROOM) {
			if (inventory_items[i].area != script_byte_vars.zone_area && inventory_items[i].area < kAreaPassage4)
				inventory_items[i].flags &= ~ITEMFLG_ROOM;
		}
	}
}

void ResetAllPersons(void) {
	int16 i;
	for (i = 0; i < PERS_MAX; i++)
		pers_list[i].flags &= ~PERSFLG_80;
	script_byte_vars.dead_flag = 0;
}

} // End of namespace Chamber
