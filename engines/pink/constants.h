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
	kMaxStringLength = 128,
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
	kCursorsCount = 13
};

enum {
	kLoadingCursor = 0,
	kDefaultCursor = 1,
	kClickableFirstFrameCursor = 2,
	kClickableSecondFrameCursor = 3,
	kNotClickableCursor = 4,
	kHoldingItemCursor = 5,
	kPDADefaultCursor = 6,
	kPDAClickableFirstFrameCursor = 7,
	kPDAClickableSecondFrameCursor = 8,
	kExitLeftCursor = 9,
	kExitRightCursor = 10,
	kExitForwardCursor = 11,
	kExitDownCursor = 12 // only in Hokus Pokus
};


// values are from Hokus Pokus
enum {
	kPokusLoadingCursorID = 135,
	kPokusExitForwardCursorID = 138,
	kPokusExitDownCursorID = 139,
	kPokusExitLeftCursorID = 133,
	kPokusExitRightCursorID = 134,
	kPokusClickableFirstCursorID = 137,
	kPokusClickableSecondCursorID = 136,
	kPokusClickableThirdCursorID = 145,
	kPokusNotClickableCursorID = 140,
	kPokusHoldingItemCursorID = 147,
	kPokusPDADefaultCursorID = 141,
	kPokusPDAClickableFirstFrameCursorID = 144,
	kPokusPDAClickableSecondFrameCursorID = 146
};

// from Peril
// it contains cursors whose ids differ
enum {
	kPerilClickableThirdCursorID = 140,
	kPerilNotClickableCursorID = 139,
	kPerilHoldingItemCursorID = 101,
	kPerilPDAClickableFirstFrameCursorID = 142,
	kPerilPDAClickableSecondFrameCursorID = 143
};

enum {
	kLoadingSave = 1,
	kLoadingNewGame = 0
};

enum {
	kOrbMajorVersion = 2,
	kOrbMinorVersion = 0,
	kBroMajorVersion = 1,
	kBroMinorVersion = 0
};

enum {
	kTimersUpdateTime = 100,
	kCursorsUpdateTime = 200
};

enum {
	kSampleRate = 22050
};

static const char *kPinkGame = "PinkGame";

static const char *kPokus = "pokus";
static const char *kPeril = "peril";

static const char *kCloseAction = "Close";
static const char *kIdleAction = "Idle";
static const char *kOpenAction = "Open";
static const char *kPlayAction = "Play";
static const char *kShowAction = "Show";
static const char *kHideAction = "Hide";

static const char *kInventoryWindowActor = "InventoryWindow";
static const char *kInventoryItemActor = "InventoryItem";
static const char *kInventoryRightArrowActor = "InventoryRightArrow";
static const char *kInventoryLeftArrowActor = "InventoryLeftArrow";

static const char *kCursorNameExitUp = "ExitUp";
static const char *kCursorNameExitLeft = "ExitLeft";
static const char *kCursorNameExitRight = "ExitRight";
static const char *kCursorNameExitForward = "ExitForward";
static const char *kCursorNameExitBackWards = "ExitBackwards";

static const char *kClickable = "Clickable";
static const char *kCursor = "Cursor";

static const char *kFoodPuzzle = "FoodPuzzle";
static const char *kJackson = "Jackson";
static const char *kBolted = "Bolted";
static const char *kDrunkLocation = "DrunkLocation";
static const char *kDrunk = "Drunk";

static const char *kFirstRound = "15.1";
static const char *kSecondRound = "15.2";
static const char *kThirdRound = "15.3";

static const char *kBoy = "Boy";
static const char *kSirBaldley = "SirBaldley";
static const char *kBoyBlocked = "BoyBlocked";

} // End of namespace Pink

#endif
