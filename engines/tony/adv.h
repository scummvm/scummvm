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
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    様様様様様様様様様様様様様様様様様 *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  ADV.CPP..............  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c                                  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c                                 *
 *      "#$e z$*"   .  `.   ^*Nc e$""                                     *
 *         "$$".  .r"   ^4.  .^$$"                                        *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#ifndef TONY_ADV_H
#define TONY_ADV_H

#include "tony/mpal/memory.h"
#include "tony/gfxcore.h"


namespace Tony {

// X & Y dimensions of the adventure
#define RM_SX			640
#define RM_SY			480

// Dimensioni X&Y del bigbuf
#define RM_BBX		(RM_SX)
#define RM_BBY		(RM_SY)

// Skipping X&Y 
#define RM_SKIPY  ((RM_BBY - RM_SY) / 2)
#define RM_SKIPX  0

// Name lengths
// FIXME: Some of these can likely be removed when refactoring is completed
#define MAX_DRIVE 10
#define MAX_DIR 255
#define MAX_FNAME 100
#define MAX_EXT 10

// Tony's actions
enum RMTonyAction {
	TA_GOTO = 0,
	TA_TAKE,
	TA_USE,
	TA_EXAMINE,
	TA_TALK,
	TA_PALESATI,

	TA_COMBINE = 10,
	TA_RECEIVECOMBINE,
	TA_COMBINEGIVE,
	TA_RECEIVECOMBINEGIVE
};

// Funzioni globali
HANDLE MainLoadLocation(int nLoc, RMPoint pt, RMPoint start);
HANDLE MainUnloadLocation(bool bDoOnExit);
void MainLinkGraphicTask(RMGfxTask *task);
void MainFreeze(void);
void MainUnfreeze(void);
void MainWaitFrame(void);
void MainShowMouse(void);
void MainHideMouse(void);
void MainEnableInput(void);
void MainDisableInput(void);
void MainPlayMusic(int nChannel, const char *filename, int nFX, bool bLoop, int nSync);
void MainInitWipe(int type);
void MainCloseWipe(void);
void MainWaitWipeEnd(void);
void MainEnableGUI(void);
void MainDisableGUI(void);
void MainSetPalesati(bool bPalesati);

} // End of namespace Tony

#endif
