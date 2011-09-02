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

// TODO: All of the "tMM"-prefixed defines should be replaced eventually
// TODO: Probably all of these don't really need to be typedef'd...
typedef int8 tMM8BitS;
typedef uint8 tMM8BitU;

typedef int16 tMM16BitS;
typedef uint16 tMM16BitU;

typedef int32 tMM32BitS;
typedef uint32 tMM32BitU;

typedef tMM8BitS tMM8BitID;
typedef tMM16BitS tMM16BitID;
typedef tMM32BitS tMM32BitID;

typedef tMM8BitU tMM8BitFlags;
typedef tMM16BitU tMM16BitFlags;
typedef tMM32BitU tMM32BitFlags;

typedef tMM32BitID tDisplayElementID;
typedef tMM32BitS tDisplayOrder;

typedef tMM16BitID tHotSpotID;
typedef tMM32BitFlags tHotSpotFlags;

typedef tMM8BitFlags tButtonState;
typedef tMM32BitFlags tInputBits;

typedef tMM8BitU tKeyMapType[16];
typedef tMM8BitU tKeyType;
typedef tMM8BitU tKeyMapIndexType;
typedef tMM8BitU tKeyMapBitType;

typedef tMM32BitID tNotificationID;
typedef tMM32BitFlags tNotificationFlags;

//	Mac types.
typedef tMM16BitS tResIDType;
typedef tMM16BitS tCoordType;
typedef tMM16BitS tQDCopyMode;
typedef tMM16BitS tResItemCountType;

enum tCopyMode {
	kNoMask,
	kUseClipArea,
	kUseTransparency
};

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
// TODO: Fixed and RGBColor

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

typedef tMM32BitID tInteractionID;

typedef tMM32BitID tAIConditionID;

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

} // End of namespace Pegasus

#endif
