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

#ifndef __JBW__
#define __JBW__

#include "common/scummsys.h"

namespace CGE {

// Defines found in cge.mak
#define VOL
#define INI_FILE VFILE // Or is it CFILE?
#define PIC_FILE VFILE
#define BMP_MODE 0
//

#define BEL      7
#define BS       8
#define HT       9
#define LF      10
#define FF      12
#define CR      13
#define MAXFILE 128

#define IsWhite(c)  ((c) == ' ' || (c) == '\t' || (c) == '\n')
#define IsUpper(c)  ((c) >= 'A' && (c) <= 'Z')
#define IsLower(c)  ((c) >= 'a' && (c) <= 'z')
#define IsDigit(c)  ((c) >= '0' && (c) <= '9')
#define IsAlpha(c)  (IsLower(c) || IsUpper(c) || (c) == '_')
#define IsAlNum(c)  (IsAlpha(c) || IsDigit(c))
#define IsHxDig(c)  (IsDigit(c) || ((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))

#define farnew(t, n) ((t *) malloc(sizeof(t) * (n)))
#define ArrayCount(a)   (sizeof(a) / sizeof((a)[0]))
#define MAX_TIMER   0x1800B0L

typedef void (MouseFunType)(void);

#define Lo(d)       (((int *) &d)[0])
#define Hi(d)       (((int *) &d)[1])
#define LoWord(d)   ((uint16) Lo(d))
#define HiWord(d)   ((uint16) Hi(d))
#define K(n)        (1024 * (n))
#define MASK(n)     ((1 << n) - 1)

typedef enum {
    NoKey   = 0, CtrlA, CtrlB, CtrlC, CtrlD, CtrlE, CtrlF, CtrlG, CtrlH,
    CtrlI, CtrlJ, CtrlK, CtrlL, CtrlM, CtrlN, CtrlO, CtrlP,
    CtrlQ, CtrlR, CtrlS, CtrlT, CtrlU, CtrlV, CtrlW, CtrlX,
    CtrlY, CtrlZ,
    BSp     = 8,
    Tab     = 9,
    Enter   = 13,
    Eof     = 26,
    Esc     = 27,
    AltQ    = 256 + 16, AltW, AltE, AltR, AltT, AltY, AltU, AltI, AltO, AltP,
    AltA    = 256 + 30, AltS, AltD, AltF, AltG, AltH, AltJ, AltK, AltL,
    AltZ    = 256 + 44, AltX, AltC, AltV, AltB, AltN, AltM,
    F11     = 256 + 87, F12,
    F1      = 256 + 59, F2, F3, F4, F5, F6, F7, F8, F9, F10,
    ShiftTab    = 256 + 15,
    ShiftF1 = 256 + 84, ShiftF2, ShiftF3, ShiftF4, ShiftF5,
    ShiftF6, ShiftF7, ShiftF8, ShiftF9, ShiftF10,
    CtrlF1  = 256 + 94, CtrlF2,  CtrlF3,  CtrlF4,  CtrlF5,
    CtrlF6,  CtrlF7,  CtrlF8,  CtrlF9,  CtrlF10,
    AltF1   = 256 + 104, AltF2,  AltF3,   AltF4,   AltF5,
    AltF6,  AltF7,   AltF8,   AltF9,   AltF10,
    Home    = 256 + 71,
    Up,
    PgUp,
    Left    = 256 + 75,
    Ctr,
    Right,
    End     = 256 + 79,
    Down,
    PgDn,
    Ins,
    Del,
    CtrlLeft    = 256 + 115,
    CtrlRight,
    CtrlEnd,
    CtrlPgDn,
    CtrlHome,
    CtrlPgUp    = 256 + 132,

    MouseLeft   = 512 + 1,
    MouseRight,
    TwiceLeft   = 512 + 256 + 1,
    TwiceRight
}  Keys;

struct  KeyStatStruct {
	int RShift      : 1;
	int LShift      : 1;
	int Ctrl        : 1;
	int Alt         : 1;

	int ScrollLock  : 1;
	int NumLock     : 1;
	int CapsLock    : 1;
	int Ins         : 1;

	int LeftCtrl    : 1;
	int LeftAlt     : 1;
	int Unused      : 6;
};

#define HGC_Cursor  0x0B0C
#define CGA_Cursor  0x0607
#define OFF_Cursor  0x2000

#define TimerCount  (*((volatile long *) ((void _seg *) 0x40 + (void *) 0x6C)))
#define KeyStat     (*((volatile struct KeyStatStruct *) ((void _seg *) 0x40 + (void *) 0x17)))
#define BreakFlag   (*((volatile uint8 *) ((void _seg *) 0x40 + (void *) 0x71)))
#define PostFlag    (*((volatile uint16 *) ((void _seg *) 0x40 + (void *) 0x72)))
#define POST        ((void (*)(void)) ((void _seg *) 0xF000 + (void *) 0xFFF0))


#ifdef  __cplusplus
#define   EC      extern "C"
#else
#define   EC
#endif


extern  uint16  _stklen;
extern  uint16  _heaplen;

} // End of namespace CGE

#endif
