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

static const char * const kPinkGame = "PinkGame";

static const char * const kPeril = "peril";

static const char * const kCloseAction = "Close";
static const char * const kIdleAction = "Idle";
static const char * const kOpenAction = "Open";
static const char * const kShowAction = "Show";
static const char * const kHideAction = "Hide";
static const char * const kInactiveAction = "Inactive";

static const char * const kInventoryWindowActor = "InventoryWindow";
static const char * const kInventoryItemActor = "InventoryItem";
static const char * const kInventoryRightArrowActor = "InventoryRightArrow";
static const char * const kInventoryLeftArrowActor = "InventoryLeftArrow";

static const char * const kAudioInfoActor = "AudioInfo";
static const char * const kPdaButtonActor = "PDAButton";

static const char * const kCursorNameExit = "Exit";
static const char * const kCursorNameExitUp = "ExitUp";
static const char * const kCursorNameExitLeft = "ExitLeft";
static const char * const kCursorNameExitRight = "ExitRight";
static const char * const kCursorNameExitForward = "ExitForward";
static const char * const kCursorNameExitBackWards = "ExitBackwards";

static const char * const kClickable = "Clickable";
static const char * const kCursor = "Cursor";

static const char * const kFoodPuzzle = "FoodPuzzle";
static const char * const kJackson = "Jackson";
static const char * const kBolted = "Bolted";
static const char * const kDrunkLocation = "DrunkLocation";
static const char * const kDrunk = "Drunk";

static const char * const kBoy = "Boy";
static const char * const kSirBaldley = "SirBaldley";
static const char * const kBoyBlocked = "BoyBlocked";

static const char * const kUndefinedValue = "UNDEFINED";
static const char * const kTrueValue = "TRUE";

static const char * const kCountryWheel = "CountryWheel";
static const char * const kDomainWheel = "DomainWheel";

static const char * const kLocator = "Locator";

static const char * const kPreviousPageButton = "PreviousPageButton";
static const char * const kDomainButton = "DomainButton";
static const char * const kNavigatorButton = "NavigatorButton";

static const char * const kNavigatePage = "NAVIGATE";

static const char * const kSfx = "SFX";

static const char * const kRightHand = "RightHand";
static const char * const kLeftHand = "LeftHand";

static const char * const kLeft1Name = "Left1";
static const char * const kLeft2Name = "Left2";
static const char * const kLeft3Name = "Left3";
static const char * const kLeft4Name = "Left4";

} // End of namespace Pink

#endif
