/** @file lbaengine.h
	@brief
	This file contains the main game engine routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "sys.h"
#include "actor.h"

int32 quitGame;
volatile int32 lbaTime;

int16 leftMouse;
int16 rightMouse;

/** Work video buffer */
uint8 *workVideoBuffer;
/** Main game video buffer */
uint8 *frontVideoBuffer;
/** Auxiliar game video buffer */
uint8 *frontVideoBufferbis;

/** temporary screen table */
int32 screenLookupTable[2000];

ActorMoveStruct loopMovePtr; // mainLoopVar1

int32 loopPressedKey;         // mainLoopVar5
int32 previousLoopPressedKey; // mainLoopVar6
int32 loopCurrentKey;    // mainLoopVar7
int32 loopInventoryItem; // mainLoopVar9

int32 loopActorStep; // mainLoopVar17

/** Disable screen recenter */
int16 disableScreenRecenter;

int32 zoomScreen;

void freezeTime();
void unfreezeTime();

int32 gameEngineLoop();

#endif
