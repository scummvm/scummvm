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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef __CGE__
#define __CGE__

#include "cge\wav.h"
#include "cge\vga13h.h"

namespace CGE {

#define TSEQ           96
#define HTALK          (TSEQ+4)
#define TOO_FAR        (TSEQ+5)
#define NO_WAY         (TSEQ+5)
#define POC_FUL        (TSEQ+5)
#define OFF_USE        (TSEQ+6)

#define EXIT_OK_TEXT   40
#define NOMUSIC_TEXT   98
#define BADSVG_TEXT    99
#define OFF_USE_COUNT  600
#define OFF_USE_TEXT   601
#define NO_WAY_TEXT    671
#define TOO_FAR_TEXT   681
#define POC_FUL_TEXT   691
#define A_C_D_TEXT     777

#define GETNAME_PROMPT 50
#define GETNAME_TITLE  51

#define QUIT_TITLE     200
#define QUIT_TEXT      201
#define NOQUIT_TEXT    202
#define DEMO_TEXT      300
#define NOSOUND_TEXT   310
                       
#define PAN_HIG        40
#define WORLD_HIG      (SCR_HIG-PAN_HIG)
                       
#define INFO_X         177
#define INFO_Y         164
#define INFO_W         140

#if defined(DEMO)
#define CAVE_X         4
#define CAVE_Y         166
#define CAVE_SX        0
#define CAVE_SY        0
#define CAVE_DX        23
#define CAVE_DY        29
#define CAVE_NX        3
#define CAVE_NY        1
#else                  
#define CAVE_X         4
#define CAVE_Y         166
#define CAVE_SX        0
#define CAVE_SY        0
#define CAVE_DX        9
#define CAVE_DY        10
#define CAVE_NX        8
#define CAVE_NY        3
#endif

#define BUTTON_X       151
#define BUTTON_Y       164
#define BUTTON_DX      19
#define BUTTON_DY      11
#define BUTTON_NX      1
#define BUTTON_NY      3
                       
#define MINI_X         86
#define MINI_Y         162

//#define       MAP_XCNT    16
//#define       MAP_ZCNT    4
#define MAP_XCNT       40
#define MAP_ZCNT       20
#define MAP_TOP        80
#define MAP_HIG        80
#define MAP_XGRID      (SCR_WID / MAP_XCNT)
#define MAP_ZGRID      (MAP_HIG / MAP_ZCNT)

#define LINE_MAX       512
#define USER_MAX       100
#define SHP_MAX        1024
#define STD_DELAY      3
#define LEV_MAX        5
#define CAVE_MAX       (CAVE_NX*CAVE_NY)
#define MAX_FIND_LEVEL 3
#define MAX_DISTANCE   3

#define INI_EXT        ".INI"
#define IN0_EXT        ".IN0"
#define LGO_EXT        ".LGO"
#define SVG_EXT        ".SVG"
                       
#define WALKSIDE       10
                       
#define BUSY_REF       500
                       
#define SYSTIMERATE    6   // 12 Hz
#define HEROFUN0       (40*12)
#define HEROFUN1       ( 2*12)
#define PAIN           (Flag[0])
#define FINIS          (Flag[3])


class SYSTEM : public SPRITE {
	int lum;
public:
	static int FunDel;
	static void SetPal(void);
	static void FunTouch(void);
	SYSTEM(void) : SPRITE(NULL) {
		SetPal();
		Tick();
	}
	void Touch(uint16 mask, int x, int y);
	void Tick(void);
};


class CLUSTER : public COUPLE {
public:
	static uint8 Map[MAP_ZCNT][MAP_XCNT];
	uint8 &Cell(void);
	CLUSTER(void) : COUPLE() { }
	CLUSTER(int a, int b) : COUPLE(a, b) { }
	bool Protected(void);
};


class WALK : public SPRITE {
public:
	CLUSTER Here;
	enum DIR { NO_DIR = -1, NN, EE, SS, WW } Dir;
	int TracePtr;
	WALK(BMP_PTR *shpl);
	void Tick(void);
	void FindWay(CLUSTER c);
	void FindWay(SPRITE *spr);
	int Distance(SPRITE *spr);
	void Turn(DIR d);
	void Park(void);
	bool Lower(SPRITE *spr);
	void Reach(SPRITE *spr, int mode = -1);
};


CLUSTER XZ(int x, int y);
CLUSTER XZ(COUPLE xy);


extern WALK *Hero;


void ExpandSprite(SPRITE *spr);
void ContractSprite(SPRITE *spr);
void cge_main(void);

} // End of namespace CGE

#endif
