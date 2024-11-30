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

#ifndef GOT_DEFINES_H
#define GOT_DEFINES_H

#include "common/scummsys.h"

namespace Got {

/**
 * Describes one alignment of a mask - image pair
 */
struct ALIGNED_MASK_IMAGE {
	int image_width;		// Image width in addresses in display memory
	uint image_ptr;			// Offset of image bitmap in display mem
	const char *mask_ptr;   // Pointer to mask bitmap
};

struct MASK_IMAGE {
	// ptrs to AlignedMaskedImage
	// structs for four possible destination
	// image alignments
	ALIGNED_MASK_IMAGE *alignments[4];
};

struct LEVEL {               //size=512
	char icon[12][20];       //0   grid of icons
	char bg_color;           //240 background color
	char type;               //241 music
	char actor_type[16];     //242 type of enemies (12 max)
	char actor_loc[16];      //254 location of enemies
	char actor_value[16];    //pass value
	char pal_colors[3];      //change 251,253,254 to these three
	char actor_invis[16];
	char extra[13];
	char static_obj[30];     //302 static objects (treasure, keys,etc)
	int  static_x[30];       //332 X coor of static objects
	int  static_y[30];       //392 Y coor of static objects
	char new_level[10];      //452 level jump for icon 200-204
	char new_level_loc[10];  //462 grid location to jump in to
	char area;               //472 game area (1=forest,etc)
	char actor_dir[16];      //initial dir
	char future[3];          //473
};

struct ACTOR {                    //size=256
	// first part loaded from disk    (size=40)
	char move;                  //movement pattern (0=none)
	char width;                 //physical width
	char height;                //physical height
	char directions;            //1,2 or 4 (1=uni-directional)
	char frames;                //# frames per direction
	char frame_speed;           //# cycles between frame changes
	char frame_sequence[4];     //sequence
	char speed;                 //move every Nth cycle
	char size_x;                 //non-physical padding on X coor
	char size_y;                 //non-phsyical padding on Y coor
	char strength;              //hit strength
	char health;                //
	char num_moves;             //# of moves every <speed> cycles
	char shot_type;             //actor # of shot
	char shot_pattern;          //func number to decide to shoot
	char shots_allowed;         //# shots allowed on screen
	char solid;                 //1=solid (not ghost,etc)
	char flying;                //
	char rating;                //rnd(100) < rating = jewel
	char type;                  //actor (0=thor,1=hammer,2=enemy,3=shot)
	char name[9];               //actors name
	char func_num;              //special function when thor touches
	char func_pass;             //value to pass to func
	int  magic_hurts;           //bitwise magic hurts flags
	char future1[4];

	//the rest is dynamic    //size=216
	MASK_IMAGE pic[4][4];       //mask image pointers
	char frame_count;           //count to switch frames
	char dir;                   //direction of travel
	char last_dir;              //last direction of travel
	int  x;                     //actual X coor
	int  y;                     //actual Y coor
	int  center;                //center of object
	int  last_x[2];             //last X coor on each page
	int  last_y[2];             //last Y coor on each page
	char used;                  //1=active, 0=not active
	char next;                  //next frame to be shown
	char speed_count;           //count down to movement
	char vunerable;             //count down to vunerability
	char shot_cnt;              //count down to another shot
	char num_shots;             //# of shots currently on screen
	char creator;               //which actor # created this actor
	char pause;                 //pause must be 0 to move
	char actor_num;
	char move_count;
	char dead;
	char toggle;
	char center_x;
	char center_y;
	char show;                  //display or not (for blinking)
	char temp1;
	char temp2;
	char counter;
	char move_counter;
	char edge_counter;
	char temp3;
	char temp4;
	char temp5;
	char hit_thor;
	int  rand;
	char init_dir;
	char pass_value;
	char shot_actor;
	char magic_hit;
	char temp6;
	int  i1, i2, i3, i4, i5, i6;
	char init_health;
	char talk_counter;
	char etype;
	char future2[25];
};

struct ACTOR_NFO {				//size=40
	char move;                  //movement pattern (0=none)
	char width;                 //physical width
	char height;                //physical height
	char directions;            //1,2 or 4 (1=uni-directional)
	char frames;                //# frames per direction
	char frame_speed;           //# cycles between frame changes
	char frame_sequence[4];     //sequence
	char speed;                 //move every Nth cycle
	char size_x;                 //non-physical padding on X coor
	char size_y;                 //non-phsyical padding on Y coor
	char strength;              //hit strength
	char health;                //
	char num_moves;             //# of moves every <speed> cycles
	char shot_type;             //actor # of shot
	char shot_pattern;          //func number to decide to shoot
	char shots_allowed;         //# shots allowed on screen
	char solid;                 //1=solid (not ghost,etc)
	char flying;                //
	char rating;                //rnd(100) < rating = jewel
	char type;                  //actor (0=thor,1=hammer,2=enemy,3=shot)
	char name[9];               //actors name
	char func_num;              //special function when thor touches
	char func_pass;             //value to pass to func
	char future1[6];
};

struct ACTOR_DATA {               //5200
	char pic[16][256];            //4096
	char shot[4][256];            //1024
	ACTOR_NFO actor_info;         //40
	ACTOR_NFO shot_info;          //40
} ;

struct SETUP {
	unsigned int  f00 : 1;
	unsigned int  f01 : 1;
	unsigned int  f02 : 1;
	unsigned int  f03 : 1;
	unsigned int  f04 : 1;
	unsigned int  f05 : 1;
	unsigned int  f06 : 1;
	unsigned int  f07 : 1;

	unsigned int  f08 : 1;
	unsigned int  f09 : 1;
	unsigned int  f10 : 1;
	unsigned int  f11 : 1;
	unsigned int  f12 : 1;
	unsigned int  f13 : 1;
	unsigned int  f14 : 1;
	unsigned int  f15 : 1;

	unsigned int  f16 : 1;
	unsigned int  f17 : 1;
	unsigned int  f18 : 1;
	unsigned int  f19 : 1;
	unsigned int  f20 : 1;
	unsigned int  f21 : 1;
	unsigned int  f22 : 1;
	unsigned int  f23 : 1;

	unsigned int  f24 : 1;
	unsigned int  f25 : 1;
	unsigned int  f26 : 1;
	unsigned int  f27 : 1;
	unsigned int  f28 : 1;
	unsigned int  f29 : 1;
	unsigned int  f30 : 1;
	unsigned int  f31 : 1;

	unsigned int  f32 : 1;
	unsigned int  f33 : 1;
	unsigned int  f34 : 1;
	unsigned int  f35 : 1;
	unsigned int  f36 : 1;
	unsigned int  f37 : 1;
	unsigned int  f38 : 1;
	unsigned int  f39 : 1;

	unsigned int  f40 : 1;
	unsigned int  f41 : 1;
	unsigned int  f42 : 1;
	unsigned int  f43 : 1;
	unsigned int  f44 : 1;
	unsigned int  f45 : 1;
	unsigned int  f46 : 1;
	unsigned int  f47 : 1;

	unsigned int  f48 : 1;
	unsigned int  f49 : 1;
	unsigned int  f50 : 1;
	unsigned int  f51 : 1;
	unsigned int  f52 : 1;
	unsigned int  f53 : 1;
	unsigned int  f54 : 1;
	unsigned int  f55 : 1;

	unsigned int  f56 : 1;
	unsigned int  f57 : 1;
	unsigned int  f58 : 1;
	unsigned int  f59 : 1;
	unsigned int  f60 : 1;
	unsigned int  f61 : 1;
	unsigned int  f62 : 1;
	unsigned int  f63 : 1;

	char value[16];
	char junk;
	char game;
	char area;          //1,2,3
	char pc_sound;      //1=enabled
	char dig_sound;     //1 & !pc_sound = enabled
	char music;         //1=enabled
	char speed;         //1=slow mode (for slower 286's)
	char scroll_flag;   //unused
	char boss_dead[3];
	char skill;         //0=easy, 1=normal, 2=hard
	char game_over;
	char future[19];    //probably not needed
};

struct PIC_HEADER {
	char width;
	char height;
};

struct THOR_INFO {
	char magic;
	char keys;
	int  jewels;
	char last_area;
	char last_screen;
	char last_icon;
	char last_dir;
	int  inventory;
	char item;         //currently selected item
	char last_health;
	char last_magic;
	int  last_jewels;
	char last_keys;
	char last_item;
	int  last_inventory;
	char level;         //current level (1,2,3)
	long score;
	long last_score;
	char object;
	char *object_name;
	char last_object;
	char *last_object_name;
	char armor;
	char future[65];
};

struct HEADER {
	long offset;
	long length;
};

//==========================================================================
// Defines

#define PAGES 0u
//#define PAGE0 3840u
//#define PAGE1 19200u
//#define PAGE2 34560u
//#define PAGE3 49920u

#define PAGE0 3840u
#define PAGE1 19280u
#define PAGE2 34720u
#define PAGE3 50160u

#define X_MAX  319
#define Y_MAX  191
#define MO_BUFF 56688u
#define MO_OFFSET 55968u
#define ENEMY_OFFSET 59664u
#define ENEMY_SHOT_OFFSET 64272u
#define MAX_ACTORS  35
#define MAX_ENEMIES 16
#define MAX_SHOTS   16
#define STAMINA 20

#define THOR 0
#define UP     72
#define DOWN   80
#define LEFT   75
#define RIGHT  77
#define HOME   71
#define PGUP   73
#define END    79
#define PGDN   81
#define ESC     1
#define SPACE  57
#define ENTER  28
#define ALT    56
#define CTRL   29
#define TAB    15
#define LSHIFT 42
#define _Z     44
#define _ONE   2
#define _TWO   3
#define _THREE 4
#define _FOUR  5
#define _S     31
#define _L     38
#define _K     37
#define _D     32
#define _B     48
#define _F1    59
#define AMI_LEN 1800
#define TMP_SIZE 5800

#define sc_Index 0x3C4
enum {
	sc_Reset,
	sc_Clock,
	sc_MapMask,
	sc_CharMap,
	sc_MemMode
};

#define crtc_Index 0x3D4

enum {
	crtc_H_Total,
	crtc_H_DispEnd,
	crtc_H_Blank,
	crtc_H_EndBlank,
	crtc_H_Retrace,
	crtc_H_EndRetrace,
	crtc_V_Total,
	crtc_OverFlow,
	crtc_RowScan,
	crtc_MaxScanLine,
	crtc_CursorStart,
	crtc_CursorEnd,
	crtc_StartHigh,
	crtc_StartLow,
	crtc_CursorHigh,
	crtc_CursorLow,
	crtc_V_Retrace,
	crtc_V_EndRetrace,
	crtc_V_DispEnd,
	crtc_Offset,
	crtc_Underline,
	crtc_V_Blank,
	crtc_V_EndBlank,
	crtc_Mode,
	crtc_LineCompare
};

#define gc_Index 0x3CE
enum {
	gc_SetReset,
	gc_EnableSetReset,
	gc_ColorCompare,
	gc_DataRotate,
	gc_ReadMap,
	gc_Mode,
	gc_Misc,
	gc_ColorDontCare,
	gc_BitMask
};

#define atr_Index 0x3c0
enum {
	atr_Mode = 16,
	atr_Overscan,
	atr_ColorPlaneEnable,
	atr_PelPan,
	atr_ColorSelect
};
#define	status_Reg1 0x3da

enum {
	OW,
	GULP,
	SWISH,
	YAH,
	ELECTRIC,
	THUNDER,
	DOOR,
	FALL,
	ANGEL,
	WOOP,
	DEAD,
	BRAAPP,
	WIND,
	PUNCH1,
	CLANG,
	EXPLODE,
	BOSS11,
	BOSS12,
	BOSS13,
};

#define	status_Reg1 0x3da

#define GAME1 (area==1)
#define GAME2 (area==2)
#define GAME3 (area==3)
#define BP    (key_flag[_B])

#define NUM_SOUNDS  19
#define NUM_OBJECTS 32

#define APPLE_MAGIC     1
#define HOURGLASS_MAGIC 2
#define LIGHTNING_MAGIC 4

#define BOOTS_MAGIC    8
#define BOMB_MAGIC     16
#define WIND_MAGIC     32

#define QUESTION_MAGIC 64
#define SHIELD_MAGIC   128
#define THUNDER_MAGIC  256

#define BOSS_LEVEL1  59
#define BOSS_LEVEL21 200
#define BOSS_LEVEL22 118

//#define DEBUG 1
//#define IDE 1
//#define CRIPPLED 1
#define DEMO_LEN 3600

} // namespace Got

#endif
