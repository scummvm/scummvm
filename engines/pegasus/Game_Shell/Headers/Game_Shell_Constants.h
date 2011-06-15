/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios
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
 
#ifndef PEGASUS_GAMESHELL_HEADERS_GAMESHELLCONSTANTS
#define PEGASUS_GAMESHELL_HEADERS_GAMESHELLCONSTANTS

#include "engines/pegasus/MMShell/MMConstants.h"
#include "engines/pegasus/Game_Shell/Headers/Game_Shell_Types.h"

namespace Pegasus {

const tGameID kGameIDNothing = -1;

const tActorID kNoActorID = kGameIDNothing;
const tActorID kPlayerID = 0;
const tItemID kNoItemID = kGameIDNothing;
const tRoomID kNoRoomID = kGameIDNothing;
const tExtraID kNoExtraID = 0xFFFFFFFF;
const tNeighborhoodID kNoNeighborhoodID = kGameIDNothing;
const tAlternateID kNoAlternateID = 0;
const tGameMenuCommand kMenuCmdNoCommand = 0;

const tHotSpotActivationID kActivateHotSpotAlways = 0;
const tHotSpotActivationID kActivateHotSpotNever = -1;

const tItemState kNoItemState = -1;

const tDirectionConstant kNoDirection = 0xFF;

const tTurnDirection kNoTurn = 0xFF;
const tTurnDirection kTurnLeft = 0;
const tTurnDirection kTurnRight = 1;
const tTurnDirection kTurnUp = 2;
const tTurnDirection kTurnDown = 3;
const tTurnDirection kMaxTurns = 4;

const tGameMode kNoMode = -1;
const tGameMode kModeNavigation = 0;
const tGameMode kLastGameShellMode = kModeNavigation;

const tCanMoveForwardReason kCanMoveForward = 0;
const tCanMoveForwardReason kCantMoveBlocked = kCanMoveForward + 1;
const tCanMoveForwardReason kCantMoveDoorClosed = kCantMoveBlocked + 1;
const tCanMoveForwardReason kCantMoveDoorLocked = kCantMoveDoorClosed + 1;
const tCanMoveForwardReason kCantMoveLastReason = kCantMoveDoorLocked;

const tCanTurnReason kCanTurn = 0;
const tCanTurnReason kCantTurnNoTurn = kCanTurn + 1;
const tCanTurnReason kCantTurnLastReason = kCantTurnNoTurn;

const tCanOpenDoorReason kCanOpenDoor = 0;
const tCanOpenDoorReason kCantOpenNoDoor = kCanOpenDoor + 1;
const tCanOpenDoorReason kCantOpenLocked = kCantOpenNoDoor + 1;
const tCanOpenDoorReason kCantOpenAlreadyOpen = kCantOpenLocked + 1;
const tCanOpenDoorReason kCantOpenLastReason = kCantOpenAlreadyOpen;

} // End of namespace Pegasus

#endif
