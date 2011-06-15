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
 
#ifndef PEGASUS_GAMESHELL_HEADERS_GAMESHELLTYPES
#define PEGASUS_GAMESHELL_HEADERS_GAMESHELLTYPES

#include "pegasus/MMShell/MMTypes.h"

namespace Pegasus {

typedef tMM16BitID tGameID;

typedef tGameID tItemID;
typedef tGameID tActorID;
typedef tGameID tRoomID;
typedef tGameID tNeighborhoodID;
typedef tMM8BitU tAlternateID;
typedef tMM8BitS tHotSpotActivationID;

typedef tMM16BitS tWeightType;

typedef tMM8BitU tDirectionConstant;
typedef tMM8BitU tTurnDirection;

// Meant to be room in low 16 bits and direction in high 16 bits.
typedef tMM32BitU tRoomViewID;

#define MakeRoomView(room, direction) (((tRoomViewID) (room)) | (((tRoomViewID) (direction)) << 16))

typedef tMM32BitU tExtraID;

typedef tMM16BitS tGameMode;

typedef tMM16BitS tWeightType;

typedef tMM16BitS tItemState;

typedef tMM8BitS tDeathReason;

typedef tMM32BitS tGameMenuCommand;

typedef tMM32BitS tGameScoreType;

typedef long tCanMoveForwardReason;

typedef long tCanTurnReason;

typedef long tCanOpenDoorReason;

enum tInventoryResult {
	kInventoryOK,
	kTooMuchWeight,
	kItemNotInInventory
};

} // End of namespace Pegasus

#endif
