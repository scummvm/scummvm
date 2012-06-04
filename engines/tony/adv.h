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
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_ADV_H
#define TONY_ADV_H

#include "common/coroutines.h"
#include "tony/mpal/memory.h"
#include "tony/gfxcore.h"


namespace Tony {

// X & Y dimensions of the adventure
#define RM_SX       640
#define RM_SY       480

// X & Y dimensions of bigbuf
#define RM_BBX      (RM_SX)
#define RM_BBY      (RM_SY)

// Skipping X & Y
#define RM_SKIPY    ((RM_BBY - RM_SY) / 2)
#define RM_SKIPX    0

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

// Global Functions
uint32 mainLoadLocation(int nLoc, RMPoint pt, RMPoint start);
void mainUnloadLocation(CORO_PARAM, bool bDoOnExit, uint32 *result);
void mainLinkGraphicTask(RMGfxTask *task);
void mainFreeze(void);
void mainUnfreeze(void);
void mainWaitFrame(CORO_PARAM);
void mainShowMouse(void);
void mainHideMouse(void);
void mainEnableInput(void);
void mainDisableInput(void);
void mainPlayMusic(int nChannel, const char *filename, int nFX, bool bLoop, int nSync);
void mainInitWipe(int type);
void mainCloseWipe(void);
void mainWaitWipeEnd(CORO_PARAM);
void mainEnableGUI(void);
void mainDisableGUI(void);
void mainSetPalesati(bool bPalesati);

} // End of namespace Tony

#endif
