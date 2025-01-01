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

#ifndef GOT_DATA_DEFINES_H
#define GOT_DATA_DEFINES_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "got/data/actor.h"
#include "got/data/level.h"
#include "got/data/sd_data.h"
#include "got/data/setup.h"
#include "got/data/thor_info.h"

namespace Got {

#define REPEAT(a) for(rep=0;rep<a;rep++)
#define IN_RANGE(v,l,h) (v>=l && v<=h)

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

#define APPLE_MAGIC      1
#define LIGHTNING_MAGIC  2
#define BOOTS_MAGIC      4
#define WIND_MAGIC       8
#define SHIELD_MAGIC    16
#define THUNDER_MAGIC   32

#define BOSS_LEVEL1  59
#define BOSS_LEVEL21 200
#define BOSS_LEVEL22 118

//#define DEBUG 1
//#define IDE 1
//#define CRIPPLED 1
#define DEMO_LEN 3600

} // namespace Got

#endif
