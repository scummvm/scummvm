/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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
 
#ifndef PEGASUS_TYPES_H
#define PEGASUS_TYPES_H

#include "common/scummsys.h"

namespace Pegasus {

// TODO: Probably all of these don't really need to be typedef'd...

typedef int32 tDisplayElementID;
typedef int32 tDisplayOrder;

typedef int16 tHotSpotID;
typedef uint32 tHotSpotFlags;

typedef byte tButtonState;
typedef uint32 tInputBits;

typedef int32 tNotificationID;
typedef uint32 tNotificationFlags;

// Mac types.
typedef int16 tResIDType;
typedef int16 tCoordType;

enum tSlideDirection {
	kSlideLeftMask = 1,
	kSlideRightMask = kSlideLeftMask << 1,
	kSlideUpMask = kSlideRightMask << 1 << 1,
	kSlideDownMask = kSlideUpMask << 1,
	
	kSlideHorizMask = kSlideLeftMask | kSlideRightMask,
	kSlideVertMask = kSlideUpMask | kSlideDownMask,

	kSlideUpLeftMask = kSlideLeftMask | kSlideUpMask,
	kSlideUpRightMask = kSlideRightMask | kSlideUpMask,
	kSlideDownLeftMask = kSlideLeftMask | kSlideDownMask,
	kSlideDownRightMask = kSlideRightMask | kSlideDownMask
};

// ScummVM QuickTime/QuickDraw replacement types
typedef uint TimeValue;
typedef uint TimeScale;

typedef int16 tGameID;

typedef tGameID tItemID;
typedef tGameID tActorID;
typedef tGameID tRoomID;
typedef tGameID tNeighborhoodID;
typedef byte tAlternateID;
typedef int8 tHotSpotActivationID;

typedef int16 tWeightType;

typedef byte tDirectionConstant;
typedef byte tTurnDirection;

// Meant to be room in low 16 bits and direction in high 16 bits.
typedef uint32 tRoomViewID;

#define MakeRoomView(room, direction) (((tRoomViewID) (room)) | (((tRoomViewID) (direction)) << 16))

typedef uint32 tExtraID;

typedef int16 tGameMode;

typedef int16 tWeightType;

typedef int16 tItemState;

typedef int8 tDeathReason;

typedef int32 tGameMenuCommand;

typedef int32 tGameScoreType;

typedef long tCanMoveForwardReason;

typedef long tCanTurnReason;

typedef long tCanOpenDoorReason;

enum tInventoryResult {
	kInventoryOK,
	kTooMuchWeight,
	kItemNotInInventory
};

typedef int32 tInteractionID;

typedef int32 tAIConditionID;

enum tEnergyStage {
	kStageNoStage,
	kStageCasual,				//	more than 50% energy
	kStageWorried,				//	more than 25% energy
	kStageNervous,				//	more than 5% energy
	kStagePanicStricken			//	less than 5% energy
};

enum tNoradSubPrepState {
	kSubNotPrepped,
	kSubPrepped,
	kSubDamaged
};

enum tLowerClientSignature {
	kNoClientSignature,
	kInventorySignature,
	kBiochipSignature,
	kAISignature
};

enum tLowerAreaSignature {
	kLeftAreaSignature,
	kMiddleAreaSignature,
	kRightAreaSignature
};

enum tAirQuality {
	kAirQualityGood,
	kAirQualityDirty,
	kAirQualityVacuum
};

enum tDragType {
	kDragNoDrag,
	kDragInventoryPickup,
	kDragBiochipPickup,
	kDragInventoryUse
};

} // End of namespace Pegasus

#endif
