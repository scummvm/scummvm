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
#include "common/serializer.h"
#include "graphics/managed_surface.h"

namespace Got {

#ifdef DEPRECATED
/**
 * Describes one alignment of a mask - image pair
 */
struct ALIGNED_MASK_IMAGE {
	int image_width = 0;		// Image width in addresses in display memory
	uint image_ptr = 0;			// Offset of image bitmap in display mem
	byte *mask_ptr = nullptr;   // Pointer to mask bitmap
};

struct MASK_IMAGE {
	// ptrs to AlignedMaskedImage
	// structs for four possible destination
	// image alignments
	ALIGNED_MASK_IMAGE *alignments[4] = {};
};
#endif

#define OBJECTS_COUNT 30

struct LEVEL {                    // size=512
	byte icon[12][20] = {};       // 0   grid of icons
	byte bg_color = 0;            // 240 background color
	byte type = 0;                // 241 music
	byte actor_type[16] = {};     // 242 type of enemies (12 max)
	byte actor_loc[16] = {};      // 254 location of enemies
	byte actor_value[16] = {};    // pass value
	byte pal_colors[3] = {};      // change 251,253,254 to these three
	byte actor_invis[16] = {};
	byte extra[13] = {};

	byte static_obj[OBJECTS_COUNT] = {};     // 302 static objects (treasure, keys,etc)
	int  static_x[OBJECTS_COUNT] = {};       // 332 X coor of static objects
	int  static_y[OBJECTS_COUNT] = {};       // 392 Y coor of static objects
	byte new_level[10] = {};      // 452 level jump for icon 200-204
	byte new_level_loc[10] = {};  // 462 grid location to jump in to
	byte area = 0;                // 472 game area (1=forest,etc)
	byte actor_dir[16] = {};      // initial dir
	byte future[3] = {};          // 473

	void sync(Common::Serializer &s);
	void load(Common::SeekableReadStream *src) {
		Common::Serializer s(src, nullptr);
		sync(s);
	}
	void save(Common::WriteStream *dest) {
		Common::Serializer s(nullptr, dest);
		sync(s);
	}
};

#define DIRECTION_COUNT 4
#define FRAME_COUNT 4

struct ACTOR {                      // Size=256
	// First part loaded from disk  (size=40)
	byte move = 0;                  // Movement pattern (0=none)
	byte width = 0;                 // Physical width
	byte height = 0;                // Physical height
	byte directions = 0;            // 1,2 or 4 (1=uni-directional)
	byte frames = 0;                // # frames per direction
	byte frame_speed = 0;           // # cycles between frame changes
	byte frame_sequence[4] = {};    // Sequence
	byte speed = 0;                 // Move every Nth cycle
	byte size_x = 0;                // Non-physical padding on X coor
	byte size_y = 0;                // Non-phsyical padding on Y coor
	byte strength = 0;              // Hit strength
	byte health = 0;                // 
	byte num_moves = 0;             // # of moves every <speed> cycles
	byte shot_type = 0;             // Actor # of shot
	byte shot_pattern = 0;          // Func number to decide to shoot
	byte shots_allowed = 0;         // # shots allowed on screen
	byte solid = 0;                 // 1=solid (not ghost,etc)
	byte flying = 0;                // 
	byte rating = 0;                // rnd(100) < rating = jewel
	byte type = 0;                  // Actor (0=thor,1=hammer,2=enemy,3=shot)
	byte name[9] = {};              // Actors name
	byte func_num = 0;              // Special function when thor touches
	byte func_pass = 0;             // Value to pass to func
	int  magic_hurts = 0;           // Bitwise magic hurts flags
	byte future1[4] = {};

	// The rest is dynamic    //size=216
	// Direction/frame surfaces
	Graphics::ManagedSurface pic[DIRECTION_COUNT][FRAME_COUNT];

	byte frame_count = 0;           // Count to switch frames
	byte dir = 0;                   // Direction of travel
	byte last_dir = 0;              // Last direction of travel
	int  x = 0;                     // Actual X coor
	int  y = 0;                     // Actual Y coor
	int  center = 0;                // Center of object
	int  last_x[2] = {};            // Last X coor on each page
	int  last_y[2] = {};            // Last Y coor on each page
	byte used = 0;                  // 1=active, 0=not active
	byte next = 0;                  // Next frame to be shown
	byte speed_count = 0;           // Count down to movement
	byte vunerable = 0;             // Count down to vunerability
	byte shot_cnt = 0;              // Count down to another shot
	byte num_shots = 0;             // # of shots currently on screen
	byte creator = 0;               // Which actor # created this actor
	byte pause = 0;                 // Pause must be 0 to move
	byte actor_num = 0;
	byte move_count = 0;
	byte dead = 0;
	byte toggle = 0;
	byte center_x = 0;
	byte center_y = 0;
	byte show = 0;                  // Display or not (for blinking)
	byte temp1 = 0;
	byte temp2 = 0;
	byte counter = 0;
	byte move_counter = 0;
	byte edge_counter = 0;
	byte temp3 = 0;
	byte temp4 = 0;
	byte temp5 = 0;
	byte hit_thor = 0;
	int  rand = 0;
	byte init_dir = 0;
	byte pass_value = 0;
	byte shot_actor = 0;
	byte magic_hit = 0;
	byte temp6 = 0;
	int  i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, i6 = 0;
	byte init_health = 0;
	byte talk_counter = 0;
	byte etype = 0;
	byte future2[25] = {};
};

struct ACTOR_NFO {				//size=40
	byte move = 0;                  //movement pattern (0=none)
	byte width = 0;                 //physical width
	byte height = 0;                //physical height
	byte directions = 0;            //1,2 or 4 (1=uni-directional)
	byte frames = 0;                //# frames per direction
	byte frame_speed = 0;           //# cycles between frame changes
	byte frame_sequence[4] = {};    //sequence
	byte speed = 0;                 //move every Nth cycle
	byte size_x = 0;                //non-physical padding on X coor
	byte size_y = 0;                //non-phsyical padding on Y coor
	byte strength = 0;              //hit strength
	byte health = 0;                //
	byte num_moves = 0;             //# of moves every <speed> cycles
	byte shot_type = 0;             //actor # of shot
	byte shot_pattern = 0;          //func number to decide to shoot
	byte shots_allowed = 0;         //# shots allowed on screen
	byte solid = 0;                 //1=solid (not ghost,etc)
	byte flying = 0;                //
	byte rating = 0;                //rnd(100) < rating = jewel
	byte type = 0;                  //actor (0=thor,1=hammer,2=enemy,3=shot)
	byte name[9] = {};              //actors name
	byte func_num = 0;              //special function when thor touches
	byte func_pass = 0;             //value to pass to func
	byte future1[6] = {};
};

struct ACTOR_DATA {                 //5200
	byte pic[16][256] = {};         //4096
	byte shot[4][256] = {};         //1024
	ACTOR_NFO actor_info = {};      //40
	ACTOR_NFO shot_info = {};       //40
};

struct SETUP {
	SETUP() :
		f00(0), f01(0), f02(0), f03(0), f04(0), f05(0), f06(0), f07(0), f08(0), f09(0),
		f10(0), f11(0), f12(0), f13(0), f14(0), f15(0), f16(0), f17(0), f18(0), f19(0),
		f20(0), f21(0), f22(0), f23(0), f24(0), f25(0), f26(0), f27(0), f28(0), f29(0),
		f30(0), f31(0), f32(0), f33(0), f34(0), f35(0), f36(0), f37(0), f38(0), f39(0),
		f40(0), f41(0), f42(0), f43(0), f44(0), f45(0), f46(0), f47(0), f48(0), f49(0),
		f50(0), f51(0), f52(0), f53(0), f54(0), f55(0), f56(0), f57(0), f58(0), f59(0),
		f60(0), f61(0), f62(0), f63(0) {
	}

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

	byte value[16] = {};
	byte junk = 0;
	byte game = 0;
	byte area = 0;          //1,2,3
	byte pc_sound = 0;      //1=enabled
	byte dig_sound = 0;     //1 & !pc_sound = enabled
	byte music = 0;         //1=enabled
	byte speed = 0;         //1=slow mode (for slower 286's)
	byte scroll_flag = 0;   //unused
	byte boss_dead[3] = {};
	byte skill = 0;         //0=easy, 1=normal, 2=hard
	byte game_over = 0;
	byte future[19] = {};   //probably not needed
};

struct PIC_HEADER {
	byte width = 0;
	byte height = 0;
};

struct THOR_INFO {
	byte magic = 0;
	byte keys = 0;
	int  jewels = 0;
	byte last_area = 0;
	byte last_screen = 0;
	byte last_icon = 0;
	byte last_dir = 0;
	int  inventory = 0;
	byte item = 0;         //currently selected item
	byte last_health = 0;
	byte last_magic = 0;
	int  last_jewels = 0;
	byte last_keys = 0;
	byte last_item = 0;
	int  last_inventory = 0;
	byte level = 0;         //current level (1,2,3)
	long score = 0;
	long last_score = 0;
	byte object = 0;
	byte *object_name = 0;
	byte last_object = 0;
	byte *last_object_name = 0;
	byte armor = 0;
	byte future[65] = {};
};

struct HEADER {
	long offset = 0;
	long length = 0;

	void load(Common::SeekableReadStream *src) {
		offset = src->readUint32LE();
		length = src->readUint32LE();
	}
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

#define GAME1 (_G(area)==1)
#define GAME2 (_G(area)==2)
#define GAME3 (_G(area)==3)
#define BP    (_G(key_flag)[_B])

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
