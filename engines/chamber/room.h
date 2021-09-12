#ifndef _ROOM_H_
#define _ROOM_H_

#define SPOTFLG_1  0x01
#define SPOTFLG_8  0x08
#define SPOTFLG_10 0x10
#define SPOTFLG_20 0x20
#define SPOTFLG_40 0x40
#define SPOTFLG_80 0x80

/*static room object*/
/*TODO: manipulated from script, do not change*/
typedef struct spot_t {
	unsigned char sx;
	unsigned char ex;
	unsigned char sy;
	unsigned char ey;
	unsigned char flags;
	unsigned char hint;
	unsigned short command;
} spot_t;

#if sizeof(spot_t) != 8
#error "spot_t must be 8 bytes long"
#endif

#define PERSFLAGS  0xF0
#define PERSFLG_10 0x10
#define PERSFLG_20 0x20
#define PERSFLG_40 0x40
#define PERSFLG_80 0x80

/*person*/
/*TODO: manipulated from script, do not change*/
typedef struct pers_t {
	unsigned char area;     /*location*/
	unsigned char flags;    /*flags in bits 7..4 and room index in bits 3..0*/
	unsigned char name;     /*name index*/
	unsigned char index;    /*animations index (in lutins_table) in bits 7..3 , spot index in bits 2..0*/
	unsigned char item;     /*inventory item index (1-based)*/
} pers_t;

#if sizeof(pers_t) != 5
#error "pers_t must be 5 bytes long"
#endif

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

extern unsigned char zone_palette;

extern spot_t *zone_spots;
extern spot_t *zone_spots_end;
extern spot_t *zone_spots_cur;

extern vortanims_t *vortanims_ptr;
extern rec7_t *rec7_ptr;
extern pers_t *pers_ptr;
extern spot_t *spot_ptr;
extern spot_t *found_spot;
extern unsigned char **spot_sprite;

extern unsigned char *lutin_mem;

extern unsigned char zone_drawn;

extern unsigned char in_de_profundis;

extern unsigned char zone_spr_index;

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

void TheWallPhase3(void);
void TheWallPhase0(void);
void TheWallPhase1_DoorClose1(void);
void TheWallPhase2_DoorClose2(void);

void PrepareCommand1(void);
void PrepareCommand3(void);
void PrepareCommand4(void);

void UpdateTimedRects1(void);
void UpdateTimedRects2(void);
void UpdateTimedInventoryItems(void);

void ResetAllPersons(void);

#endif
