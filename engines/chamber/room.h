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

#ifndef CHAMBER_ROOM_H
#define CHAMBER_ROOM_H

namespace Chamber {

#define SPOTFLG_1  0x01
#define SPOTFLG_8  0x08
#define SPOTFLG_10 0x10
#define SPOTFLG_20 0x20
#define SPOTFLG_40 0x40
#define SPOTFLG_80 0x80

/*static room object*/
/*TODO: manipulated from script, do not change*/
#include "common/pack-start.h"
typedef struct spot_t {
	unsigned char sx;
	unsigned char ex;
	unsigned char sy;
	unsigned char ey;
	unsigned char flags;
	unsigned char hint;
	unsigned short command;
} spot_t;
#include "common/pack-end.h"

#define PERSFLAGS  0xF0
#define PERSFLG_10 0x10
#define PERSFLG_20 0x20
#define PERSFLG_40 0x40
#define PERSFLG_80 0x80

/*person*/
/*TODO: manipulated from script, do not change*/
#include "common/pack-start.h"
typedef struct pers_t {
	unsigned char area;     /*location*/
	unsigned char flags;    /*flags in bits 7..4 and room index in bits 3..0*/
	unsigned char name;     /*name index*/
	unsigned char index;    /*animations index (in lutins_table) in bits 7..3 , spot index in bits 2..0*/
	unsigned char item;     /*inventory item index (1-based)*/
} pers_t;
#include "common/pack-end.h"

#define ANIMFLG_USESPOT 0x80

typedef struct animdesc_t {
	unsigned char index;    /*flag in bit 7, animation index in bits 6..0*/
	union {
		struct {
			unsigned char x, y;
		} coords;
		unsigned short desc;
	} params;
} animdesc_t;

typedef struct vortanims_t {
	unsigned char room;
	animdesc_t field_1;
	animdesc_t field_4;
	animdesc_t field_7;
	animdesc_t field_A;
} vortanims_t;

typedef struct rec7_t {
	unsigned char room;
	animdesc_t field_1;
	animdesc_t field_4;
} rec7_t;

extern unsigned char scratch_mem1[8010];
extern unsigned char *scratch_mem2;

extern rect_t room_bounds_rect;

extern unsigned char last_object_hint;
extern unsigned char object_hint;
extern unsigned char command_hint;
extern unsigned char last_command_hint;

extern unsigned short next_ticks2;
extern unsigned short next_ticks3;
extern unsigned short next_command3;
extern unsigned short next_ticks4;
extern unsigned short next_command4;

#define MAX_SPRITES 16

extern unsigned char *sprites_list[MAX_SPRITES];

#define MAX_DOORS 5

extern unsigned char *doors_list[MAX_DOORS];

extern unsigned char zone_palette;

extern spot_t *zone_spots;
extern spot_t *zone_spots_end;
extern spot_t *zone_spots_cur;

extern vortanims_t vortsanim_list[];
extern vortanims_t *vortanims_ptr;

extern rec7_t recs7_list[];
extern rec7_t *rec7_ptr;
extern pers_t *pers_ptr;
extern spot_t *spot_ptr;
extern spot_t *found_spot;
extern unsigned char **spot_sprite;

extern unsigned char *lutin_mem;

extern unsigned char zone_drawn;

extern unsigned char in_de_profundis;

extern unsigned char zone_name;
extern unsigned char room_hint_bar_width;
extern unsigned char zone_spr_index;
extern unsigned char zone_obj_count;
extern unsigned char room_hint_bar_coords_x;
extern unsigned char room_hint_bar_coords_y;

extern unsigned short inv_update_time;

extern const unsigned char timed_seq[];
extern const unsigned char *timed_seq_ptr;

typedef struct thewalldoor_t {
	unsigned char   height;
	unsigned char   width;
	unsigned int    pitch;
	unsigned int    offs;
	unsigned char   *pixels;
} thewalldoor_t;

extern thewalldoor_t the_wall_doors[2];

int IsInRect(unsigned char x, unsigned char y, rect_t *rect);
int IsCursorInRect(rect_t *rect);
void SelectSpotCursor(void);

void CheckHotspots(unsigned char m, unsigned char v);

void AnimateSpot(const animdesc_t *info);
unsigned char *LoadPuzzlToScratch(unsigned char index);

void DrawObjectHint(void);
void ShowObjectHint(unsigned char *target);
void DrawCommandHint(void);
void ShowCommandHint(unsigned char *target);

char DrawZoneAniSprite(rect_t *rect, unsigned int index, unsigned char *target);

void DrawHintsAndCursor(unsigned char *target);

void DrawTheWallDoors(void);
void MergeSpritesData(unsigned char *target, unsigned int pitch, unsigned char *source, unsigned int w, unsigned int h);
void MergeSpritesDataFlip(unsigned char *target, unsigned int pitch, unsigned char *source, unsigned int w, unsigned int h);

void RefreshSpritesData(void);
void BlitSpritesToBackBuffer(void);
void BackupSpotsImages(void);

void SelectPalette(void);

unsigned char FindSpotByFlags(unsigned char mask, unsigned char value);
unsigned char FindAndSelectSpot(unsigned char offset);

void FindPerson(void);

void UpdateZoneSpot(unsigned char index);
void DrawRoomStaticObject(unsigned char *aptr, unsigned char *rx, unsigned char *ry, unsigned char *rw, unsigned char *rh);
void DrawRoomStatics(void);
void RedrawRoomStatics(unsigned char index, unsigned char y_step);
void DrawZoneObjs(void);
void RefreshZone(void);
void ChangeZone(unsigned char index);

void DrawSpots(unsigned char *target);
void AnimateSpots(unsigned char *target);

unsigned char FindInitialSpot(void);
void AnimRoomDoorOpen(unsigned char index);
void AnimRoomDoorClose(unsigned char index);

unsigned int GetPuzzlSprite(unsigned char index, unsigned char x, unsigned char y, unsigned int *w, unsigned int *h, unsigned int *ofs);

void BounceCurrentItem(unsigned char flags, unsigned char y);

void BackupScreenOfSpecialRoom(void);
void RestoreScreenOfSpecialRoom(void);

void TheWallPhase3_DoorOpen1(void);
void TheWallPhase0_DoorOpen2(void);
void TheWallPhase1_DoorClose1(void);
void TheWallPhase2_DoorClose2(void);

void PrepareCommand1(void);
void PrepareCommand3(void);
void PrepareCommand4(void);

void UpdateTimedRects1(void);
void UpdateTimedRects2(void);
void UpdateTimedInventoryItems(void);

void ResetAllPersons(void);

} // End of namespace Chamber

#endif
