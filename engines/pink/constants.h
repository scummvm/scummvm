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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PINK_CONSTANTS_H
#define PINK_CONSTANTS_H

namespace Pink {

enum {
    kMaxClassLength = 32,
    kMaxStringLength = 64,
    kNullObject = 0
};

enum {
    kActionHide,
    kActionLoop,
    kActionPlay,
    kActionPlayWithSfx,
    kActionSfx,
    kActionSound,
    kActionStill,
    kActionTalk,
    kActionText,
    kActor,
    kAudioInfoPDAButton,
    kConditionGameVariable,
    kConditionInventoryItemOwner,
    kConditionModuleVariable,
    kConditionNotInventoryItemOwner,
    kConditionNotModuleVariable,
    kConditionNotPageVariable,
    kConditionPageVariable,
    kCursorActor,
    kGamePage,
    kHandlerLeftClick,
    kHandlerStartPage,
    kHandlerTimer,
    kHandlerTimerActions,
    kHandlerTimerSequences,
    kHandlerUseClick,
    kInventoryActor,
    kInventoryItem,
    kLeadActor,
    kModuleProxy,
    kPDAButtonActor,
    kParlSqPink,
    kPubPink,
    kSeqTimer,
    kSequence,
    kSequenceAudio,
    kSequenceItem,
    kSequenceItemDefaultAction,
    kSequenceItemLeader,
    kSequenceItemLeaderAudio,
    kSideEffectExit,
    kSideEffectGameVariable,
    kSideEffectInventoryItemOwner,
    kSideEffectLocation,
    kSideEffectModuleVariable,
    kSideEffectPageVariable,
    kSideEffectRandomPageVariable,
    kSupportingActor,
    kWalkAction,
    kWalkLocation
};

enum {
    kCursorsCount = 11
};

enum {
    kLoadingCursor = 0,
    kExitForwardCursor = 1,
    kExitLeftCursor = 2,
    kExitRightCursor = 3,
    kDefaultCursor = 4,
    kClickableFirstFrameCursor = 5,
    kClickableSecondFrameCursor = 6,
    kNotClickableCursor = 7,
    kHoldingItemCursor = 8,
    kPDAFirstCursor = 9,
    kPDASecondCursor = 10
};


// values are from Hokus Pokus
enum {
    kPokusLoadingCursorID = 135,
    kPokusExitForwardCursorID = 138,
    kPokusExitLeftCursorID = 133,
    kPokusExitRightCursorID = 134,
    kPokusClickableFirstCursorID = 137,
    kPokusClickableSecondCursorID = 136,
    kPokusClickableThirdCursorID = 145,
    kPokusNotClickableCursorID = 140,
    kPokusHoldingItemCursorID = 147,
    kPokusPDAFirstCursorID = 141,
    kPokusPDASecondCursorID = 144
};

// from Peril
// it contains cursors whose ids differ
enum {
    kPerilClickableThirdCursorID = 140,
    kPerilNotClickableCursorID = 139,
    kPerilPDASecondCursorID = 142
};

enum {
    kLoadingSave = 1,
    kLoadingNewGame = 0
};

} // End of namespace Pink

#endif
