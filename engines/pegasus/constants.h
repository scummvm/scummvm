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
 
#ifndef PEGASUS_CONSTANTS_H
#define PEGASUS_CONSTANTS_H

#include "common/endian.h"
#include "common/rect.h"

#include "pegasus/types.h"

namespace Pegasus {

// TODO: Organize these

const GameID kGameIDNothing = -1;

const ActorID kNoActorID = kGameIDNothing;
const ActorID kPlayerID = 0;
const ItemID kNoItemID = kGameIDNothing;
const RoomID kNoRoomID = kGameIDNothing;
const ExtraID kNoExtraID = 0xFFFFFFFF;
const NeighborhoodID kNoNeighborhoodID = kGameIDNothing;
const AlternateID kNoAlternateID = 0;
const GameMenuCommand kMenuCmdNoCommand = 0;

const HotSpotActivationID kActivateHotSpotAlways = 0;
const HotSpotActivationID kActivateHotSpotNever = -1;

const ItemState kNoItemState = -1;

const DirectionConstant kNoDirection = 0xFF;
static const DirectionConstant kNorth = 0;
static const DirectionConstant kSouth = 1;
static const DirectionConstant kEast = 2;
static const DirectionConstant kWest = 3;

const TurnDirection kNoTurn = 0xFF;
const TurnDirection kTurnLeft = 0;
const TurnDirection kTurnRight = 1;
const TurnDirection kTurnUp = 2;
const TurnDirection kTurnDown = 3;
const TurnDirection kMaxTurns = 4;

const GameMode kNoMode = -1;
const GameMode kModeNavigation = 0;
const GameMode kLastGameShellMode = kModeNavigation;

const CanMoveForwardReason kCanMoveForward = 0;
const CanMoveForwardReason kCantMoveBlocked = kCanMoveForward + 1;
const CanMoveForwardReason kCantMoveDoorClosed = kCantMoveBlocked + 1;
const CanMoveForwardReason kCantMoveDoorLocked = kCantMoveDoorClosed + 1;
const CanMoveForwardReason kCantMoveLastReason = kCantMoveDoorLocked;

const CanTurnReason kCanTurn = 0;
const CanTurnReason kCantTurnNoTurn = kCanTurn + 1;
const CanTurnReason kCantTurnLastReason = kCantTurnNoTurn;

const CanOpenDoorReason kCanOpenDoor = 0;
const CanOpenDoorReason kCantOpenNoDoor = kCanOpenDoor + 1;
const CanOpenDoorReason kCantOpenLocked = kCantOpenNoDoor + 1;
const CanOpenDoorReason kCantOpenAlreadyOpen = kCantOpenLocked + 1;
const CanOpenDoorReason kCantOpenLastReason = kCantOpenAlreadyOpen;

const DisplayElementID kNoDisplayElement = -1;
const DisplayElementID kHighestReservedElementID = -2;

const DisplayElementID kCursorID = kHighestReservedElementID;
const DisplayElementID kLoadScreenID = kCursorID - 1;

const DisplayOrder kMinAvailableOrder = 0;
const DisplayOrder kMaxAvailableOrder = 999998;
const DisplayOrder kLoadScreenOrder = 900000;
const DisplayOrder kCursorOrder = 1000000;

const HotSpotID kNoHotSpotID = -1;
const HotSpotFlags kNoHotSpotFlags = 0;
const HotSpotFlags kAllHotSpotFlags = ~kNoHotSpotFlags;

const NotificationFlags kNoNotificationFlags = 0;

const DisplayElementID kCurrentDragSpriteID = 1000;

const TimeScale kDefaultTimeScale = 600;

//	Ticks per second.

const TimeScale kOneTickPerSecond = 1;
const TimeScale kTwoTicksPerSecond = 2;
const TimeScale kFifteenTicksPerSecond = 15;
const TimeScale kThirtyTicksPerSecond = 30;
const TimeScale kSixtyTicksPerSecond = 60;
const TimeScale kMovieTicksPerSecond = 600;

//	These times are in seconds.

const TimeValue kOneSecond = 1;
const TimeValue kTwoSeconds = 2;
const TimeValue kThreeSeconds = 3;
const TimeValue kFourSeconds = 4;
const TimeValue kFiveSeconds = 5;
const TimeValue kSixSeconds = 6;
const TimeValue kSevenSeconds = 7;
const TimeValue kEightSeconds = 8;
const TimeValue kNineSeconds = 9;
const TimeValue kTenSeconds = 10;
const TimeValue kElevenSeconds = 11;
const TimeValue kTwelveSeconds = 12;
const TimeValue kThirteenSeconds = 13;
const TimeValue kFourteenSeconds = 14;
const TimeValue kFifteenSeconds = 15;
const TimeValue kSixteenSeconds = 16;
const TimeValue kSeventeenSeconds = 17;
const TimeValue kEighteenSeconds = 18;
const TimeValue kNineteenSeconds = 19;
const TimeValue kTwentySeconds = 20;
const TimeValue kThirtySeconds = 30;
const TimeValue kFortySeconds = 40;
const TimeValue kFiftySeconds = 50;
const TimeValue kSixtySeconds = 60;
const TimeValue kOneMinute = 60;
const TimeValue kTwoMinutes = kOneMinute * 2;
const TimeValue kThreeMinutes = kOneMinute * 3;
const TimeValue kFourMinutes = kOneMinute * 4;
const TimeValue kFiveMinutes = kOneMinute * 5;
const TimeValue kSixMinutes = kOneMinute * 6;
const TimeValue kSevenMinutes = kOneMinute * 7;
const TimeValue kEightMinutes = kOneMinute * 8;
const TimeValue kNineMinutes = kOneMinute * 9;
const TimeValue kTenMinutes = kOneMinute * 10;
const TimeValue kElevenMinutes = kOneMinute * 11;
const TimeValue kTwelveMinutes = kOneMinute * 12;
const TimeValue kThirteenMinutes = kOneMinute * 13;
const TimeValue kFourteenMinutes = kOneMinute * 14;
const TimeValue kFifteenMinutes = kOneMinute * 15;
const TimeValue kSixteenMinutes = kOneMinute * 16;
const TimeValue kSeventeenMinutes = kOneMinute * 17;
const TimeValue kEighteenMinutes = kOneMinute * 18;
const TimeValue kNineteenMinutes = kOneMinute * 19;
const TimeValue kTwentyMinutes = kOneMinute * 20;
const TimeValue kThirtyMinutes = kOneMinute * 30;
const TimeValue kFortyMinutes = kOneMinute * 40;
const TimeValue kFiftyMinutes = kOneMinute * 50;
const TimeValue kOneHour = kOneMinute * 60;
const TimeValue kTwoHours = kOneHour * 2;

//	Common times.

const TimeValue kHalfSecondPerTwoTicks = kTwoTicksPerSecond / 2;
const TimeValue kHalfSecondPerThirtyTicks = kThirtyTicksPerSecond / 2;
const TimeValue kHalfSecondPerSixtyTicks = kSixtyTicksPerSecond / 2;

const TimeValue kOneSecondPerTwoTicks = kTwoTicksPerSecond;
const TimeValue kOneSecondPerThirtyTicks = kThirtyTicksPerSecond;
const TimeValue kOneSecondPerSixtyTicks = kSixtyTicksPerSecond;

const TimeValue kOneMinutePerFifteenTicks = kOneMinute * kFifteenTicksPerSecond;
const TimeValue kFiveMinutesPerFifteenTicks = kFiveMinutes * kFifteenTicksPerSecond;
const TimeValue kTenMinutesPerFifteenTicks = kTenMinutes * kFifteenTicksPerSecond;

const TimeValue kOneMinutePerThirtyTicks = kOneMinute * kThirtyTicksPerSecond;
const TimeValue kFiveMinutesPerThirtyTicks = kFiveMinutes * kThirtyTicksPerSecond;
const TimeValue kTenMinutesPerThirtyTicks = kTenMinutes * kThirtyTicksPerSecond;

const TimeValue kOneMinutePerSixtyTicks = kOneMinute * kSixtyTicksPerSecond;
const TimeValue kFiveMinutesPerSixtyTicks = kFiveMinutes * kSixtyTicksPerSecond;
const TimeValue kTenMinutesPerSixtyTicks = kTenMinutes * kSixtyTicksPerSecond;

//	Time in seconds you can hang around Caldoria without going to work...
const TimeValue kLateWarning2TimeLimit = kFiveMinutes;
const TimeValue kLateWarning3TimeLimit = kTenMinutes;

const TimeValue kSinclairShootsTimeLimit = kThreeMinutes;
const TimeValue kCardBombCountDownTime = kTwelveSeconds;

const TimeValue kOxyMaskFullTime = kThirtyMinutes;

const TimeValue kTSAUncreatedTimeLimit = kFiveMinutes;
const TimeValue kRipTimeLimit = kTenMinutesPerFifteenTicks;
const TimeScale kRipTimeScale = kFifteenTicksPerSecond;

const TimeValue kIntroTimeOut = kThirtySeconds;

const TimeValue kMarsRobotPatienceLimit = kFifteenSeconds;
const TimeValue kLockFreezeTimeLmit = kFifteenSeconds;
const TimeValue kSpaceChaseTimeLimit = kTenMinutes;
const TimeValue kVacuumSurvivalTimeLimit = kThirtySeconds;
const TimeValue kColorMatchingTimeLimit = kFourMinutes;
const TimeScale kJunkTimeScale = kFifteenTicksPerSecond;
const TimeValue kJunkDropBaseTime = kFiveSeconds;
const TimeValue kJunkDropSlopTime = kThreeSeconds;
const TimeValue kJunkTravelTime = kTenSeconds * kJunkTimeScale;
const TimeValue kCollisionReboundTime = kOneSecond * kJunkTimeScale;
const TimeValue kWeaponReboundTime = kTwoSeconds * kJunkTimeScale;

const TimeValue kGawkAtRobotTime = kTenSeconds;
const TimeValue kGawkAtRobotTime2 = kThirteenSeconds;
const TimeValue kPlasmaImpactTime = kTwoSeconds;

const TimeValue kNoradAirMaskTimeLimit = kOneMinute + kFifteenSeconds;

const NotificationID kNeighborhoodNotificationID = 1;
const NotificationID kLastNeighborhoodNotificationID = kNeighborhoodNotificationID;

const NotificationFlags kNeighborhoodMovieCompletedFlag = 1;
const NotificationFlags kMoveForwardCompletedFlag = kNeighborhoodMovieCompletedFlag << 1;
const NotificationFlags kStrideCompletedFlag = kMoveForwardCompletedFlag << 1;
const NotificationFlags kTurnCompletedFlag = kStrideCompletedFlag << 1;
const NotificationFlags kSpotCompletedFlag = kTurnCompletedFlag << 1;
const NotificationFlags kDoorOpenCompletedFlag = kSpotCompletedFlag << 1;
const NotificationFlags kExtraCompletedFlag = kDoorOpenCompletedFlag << 1;
const NotificationFlags kSpotSoundCompletedFlag = kExtraCompletedFlag << 1;
const NotificationFlags kDelayCompletedFlag = kSpotSoundCompletedFlag << 1;
const NotificationFlags kActionRequestCompletedFlag = kDelayCompletedFlag << 1;
const NotificationFlags kDeathExtraCompletedFlag = kActionRequestCompletedFlag << 1;
const NotificationFlags kLastNeighborhoodNotificationFlag = kDeathExtraCompletedFlag;

const NotificationFlags kNeighborhoodFlags =	kNeighborhoodMovieCompletedFlag |
												kMoveForwardCompletedFlag |
												kStrideCompletedFlag |
												kTurnCompletedFlag |
												kSpotCompletedFlag |
												kDoorOpenCompletedFlag |
												kExtraCompletedFlag |
												kSpotSoundCompletedFlag |
												kDelayCompletedFlag |
												kActionRequestCompletedFlag |
												kDeathExtraCompletedFlag;

const uint32 kPegasusPrimeCreator = MKTAG('J', 'P', 'P', 'P');
const uint32 kPegasusPrimeContinueType = MKTAG('P', 'P', 'C', 'T');

const uint32 kPegasusPrimeDisk1GameType = MKTAG('P', 'P', 'G', '1');
const uint32 kPegasusPrimeDisk2GameType = MKTAG('P', 'P', 'G', '2');
const uint32 kPegasusPrimeDisk3GameType = MKTAG('P', 'P', 'G', '3');
const uint32 kPegasusPrimeDisk4GameType = MKTAG('P', 'P', 'G', '4');

// We only support one of the save versions; the rest are from betas
// and we are not supporting them.
const uint32 kPegasusPrimeVersion = 0x00009019;

const char kNormalSave = 0;
const char kContinueSave = 1;

//	Display IDs.

const DisplayElementID kNavMovieID = 1;
const DisplayElementID kTurnPushID = 2;

const DisplayElementID kMaxGameShellDisplayID = kTurnPushID;

//	Display ordering.

const DisplayOrder kNavLayer = 10000;
const DisplayOrder kNavMovieOrder = kNavLayer;
const DisplayOrder kTurnPushOrder = kNavMovieOrder + 1;

/////////////////////////////////////////////
//
//	Display IDs.

const DisplayElementID kScreenDimmerID = kMaxGameShellDisplayID + 1;
const DisplayElementID kInterface1ID = kScreenDimmerID + 1;
const DisplayElementID kInterface2ID = kInterface1ID + 1;
const DisplayElementID kInterface3ID = kInterface2ID + 1;
const DisplayElementID kInterface4ID = kInterface3ID + 1;
const DisplayElementID kDateID = kInterface4ID + 1;
const DisplayElementID kCompassID = kDateID + 1;
const DisplayElementID kInventoryPushID = kCompassID + 1;
const DisplayElementID kInventoryLidID = kInventoryPushID + 1;
const DisplayElementID kBiochipPushID = kInventoryLidID + 1;
const DisplayElementID kBiochipLidID = kBiochipPushID + 1;
const DisplayElementID kEnergyBarID = kBiochipLidID + 1;
const DisplayElementID kWarningLightID = kEnergyBarID + 1;
const DisplayElementID kAILeftAreaID = kWarningLightID + 1;
const DisplayElementID kAIMiddleAreaID = kAILeftAreaID + 1;
const DisplayElementID kAIRightAreaID = kAIMiddleAreaID + 1;
const DisplayElementID kAIMovieID = kAIRightAreaID + 1;
const DisplayElementID kInventoryDropHighlightID = kAIMovieID + 1;
const DisplayElementID kBiochipDropHighlightID = kInventoryDropHighlightID + 1;

const DisplayElementID kDraggingSpriteID = 1000;

const DisplayElementID kCroppedMovieID = 2000;

const DisplayElementID kNeighborhoodDisplayID = 3000;

const DisplayElementID kItemPictureBaseID = 5000;

const CoordType kNavAreaLeft = 64;
const CoordType kNavAreaTop = 64;

const CoordType kBackground1Left = 0;
const CoordType kBackground1Top = 64;

const CoordType kBackground2Left = 0;
const CoordType kBackground2Top = 0;

const CoordType kBackground3Left = 576;
const CoordType kBackground3Top = 64;

const CoordType kBackground4Left = 0;
const CoordType kBackground4Top = 320;

const CoordType kOverviewControllerLeft = 540;
const CoordType kOverviewControllerTop = 348;

const CoordType kSwapLeft = 194;
const CoordType kSwapTop = 116;

const CoordType kSwapHiliteLeft = 200;
const CoordType kSwapHiliteTop = 206;

const CoordType kDateLeft = 136;
const CoordType kDateTop = 44;

const CoordType kCompassLeft = 222;
const CoordType kCompassTop = 42;
const CoordType kCompassWidth = 92;

const CoordType kInventoryPushLeft = 74;
const CoordType kInventoryPushTop = 92;

const CoordType kInventoryLidLeft = 74;
const CoordType kInventoryLidTop = 316;

const CoordType kBiochipPushLeft = 362;
const CoordType kBiochipPushTop = 192;

const CoordType kBiochipLidLeft = 362;
const CoordType kBiochipLidTop = 316;

const CoordType kInventoryDropLeft = 0;
const CoordType kInventoryDropTop = 320;
const CoordType kInventoryDropRight = 232;
const CoordType kInventoryDropBottom = 480;

const CoordType kBiochipDropLeft = 302;
const CoordType kBiochipDropTop = 320;
const CoordType kBiochipDropRight = 640;
const CoordType kBiochipDropBottom = 480;

const CoordType kFinalMessageLeft = kInventoryPushLeft + 1;
const CoordType kFinalMessageTop = kInventoryPushTop + 24;

/////////////////////////////////////////////
//
//	Notifications.

const NotificationID kJMPDCShellNotificationID = kLastNeighborhoodNotificationID + 1;
const NotificationID kInterfaceNotificationID = kJMPDCShellNotificationID + 1;
const NotificationID kAINotificationID = kInterfaceNotificationID + 1;
const NotificationID kNoradNotificationID = kAINotificationID + 1;
const NotificationID kNoradECRNotificationID = kNoradNotificationID + 1;
const NotificationID kNoradFillingStationNotificationID = kNoradECRNotificationID + 1;
const NotificationID kNoradPressureNotificationID = kNoradFillingStationNotificationID + 1;
const NotificationID kNoradUtilityNotificationID = kNoradPressureNotificationID + 1;
const NotificationID kNoradElevatorNotificationID = kNoradUtilityNotificationID + 1;
const NotificationID kNoradSubPlatformNotificationID = kNoradElevatorNotificationID + 1;
const NotificationID kSubControlNotificationID = kNoradSubPlatformNotificationID + 1;
const NotificationID kNoradGreenBallNotificationID = kSubControlNotificationID + 1;
const NotificationID kNoradGlobeNotificationID = kNoradGreenBallNotificationID + 1;
const NotificationID kCaldoriaVidPhoneNotificationID = kNoradGlobeNotificationID + 1;
const NotificationID kCaldoriaMessagesNotificationID = kCaldoriaVidPhoneNotificationID + 1;
const NotificationID kCaldoriaBombTimerNotificationID = kCaldoriaMessagesNotificationID + 1;

//	Sent to the shell by fShellNotification.
const NotificationFlags kGameStartingFlag = 1;
const NotificationFlags kNeedNewJumpFlag = kGameStartingFlag << 1;
const NotificationFlags kPlayerDiedFlag = kNeedNewJumpFlag << 1;

const NotificationFlags kJMPShellNotificationFlags = kGameStartingFlag |
														kNeedNewJumpFlag |
														kPlayerDiedFlag;

//	Sent to the interface.
const NotificationFlags kInventoryLidOpenFlag = 1;
const NotificationFlags kInventoryLidClosedFlag = kInventoryLidOpenFlag << 1;
const NotificationFlags kInventoryDrawerUpFlag = kInventoryLidClosedFlag << 1;
const NotificationFlags kInventoryDrawerDownFlag = kInventoryDrawerUpFlag << 1;
const NotificationFlags kBiochipLidOpenFlag = kInventoryDrawerDownFlag << 1;
const NotificationFlags kBiochipLidClosedFlag = kBiochipLidOpenFlag << 1;
const NotificationFlags kBiochipDrawerUpFlag = kBiochipLidClosedFlag << 1;
const NotificationFlags kBiochipDrawerDownFlag = kBiochipDrawerUpFlag << 1;

const NotificationFlags kInterfaceNotificationFlags =	kInventoryLidOpenFlag |
														kInventoryLidClosedFlag |
														kInventoryDrawerUpFlag |
														kInventoryDrawerDownFlag |
														kBiochipLidOpenFlag |
														kBiochipLidClosedFlag |
														kBiochipDrawerUpFlag |
														kBiochipDrawerDownFlag;

//	Hot spots.

//	Neighborhood hot spots.

const HotSpotID kFirstNeighborhoodSpotID = 5000;

//	kShellSpotFlag is a flag which marks all hot spots which belong to the shell, like
//	the current item and current biochip spots.
const HotSpotFlags kShellSpotFlag = 1;
//	kNeighborhoodSpotFlag is a flag which marks all hot spots which belong to a
//	neighborhood, like buttons on walls and so on.
const HotSpotFlags kNeighborhoodSpotFlag = kShellSpotFlag << 1;
//	kZoomInSpotFlag is a flag which marks all hot spots which indicate a zoom.
const HotSpotFlags kZoomInSpotFlag = kNeighborhoodSpotFlag << 1;
//	kZoomOutSpotFlag is a flag which marks all hot spots which indicate a zoom.
const HotSpotFlags kZoomOutSpotFlag = kZoomInSpotFlag << 1;

const HotSpotFlags kClickSpotFlag = kZoomOutSpotFlag << 1;
const HotSpotFlags kPlayExtraSpotFlag = kClickSpotFlag << 1;
const HotSpotFlags kPickUpItemSpotFlag = kPlayExtraSpotFlag << 1;
const HotSpotFlags kDropItemSpotFlag = kPickUpItemSpotFlag << 1;
const HotSpotFlags kOpenDoorSpotFlag = kDropItemSpotFlag << 1;

const HotSpotFlags kZoomSpotFlags = kZoomInSpotFlag | kZoomOutSpotFlag;

const HotSpotFlags kHighestGameShellSpotFlag = kOpenDoorSpotFlag;

/////////////////////////////////////////////
//
//	Hot spots.

//	Shell hot spots.
//	The shell reserves all hot spot IDs from 0 to 999

const HotSpotID kCurrentItemSpotID = 0;
const HotSpotID kCurrentBiochipSpotID = kCurrentItemSpotID + 1;

const HotSpotID kInventoryDropSpotID = kCurrentBiochipSpotID + 1;
const HotSpotID kBiochipDropSpotID = kInventoryDropSpotID + 1;

const HotSpotID kInfoReturnSpotID = kBiochipDropSpotID + 1;

const HotSpotID kAIHint1SpotID = kInfoReturnSpotID + 1;
const HotSpotID kAIHint2SpotID = kAIHint1SpotID + 1;
const HotSpotID kAIHint3SpotID = kAIHint2SpotID + 1;
const HotSpotID kAISolveSpotID = kAIHint3SpotID + 1;
const HotSpotID kAIBriefingSpotID = kAISolveSpotID + 1;
const HotSpotID kAIScanSpotID = kAIBriefingSpotID + 1;

const HotSpotID kPegasusRecallSpotID = kAIScanSpotID + 1;

const HotSpotID kAriesSpotID = kPegasusRecallSpotID + 1;
const HotSpotID kMercurySpotID = kAriesSpotID + 1;
const HotSpotID kPoseidonSpotID = kMercurySpotID + 1;

const HotSpotID kAirMaskToggleSpotID = kPoseidonSpotID + 1;

const HotSpotID kShuttleEnergySpotID = kAirMaskToggleSpotID + 1;
const HotSpotID kShuttleGravitonSpotID = kShuttleEnergySpotID + 1;
const HotSpotID kShuttleTractorSpotID = kShuttleGravitonSpotID + 1;
const HotSpotID kShuttleViewSpotID = kShuttleTractorSpotID + 1;
const HotSpotID kShuttleTransportSpotID = kShuttleViewSpotID + 1;

//	Most of these are obsolete:

//	kInventoryDropSpotFlag is a flag which marks hot spots which are valid drop spots
//	for inventory items.
//	const HotSpotFlags kInventoryDropSpotFlag = kHighestGameShellSpotFlag << 1;

//	kBiochipDropSpotFlag is a flag which marks hot spots which are valid drop spots
//	for biochips.
//	const HotSpotFlags kBiochipDropSpotFlag = kInventoryDropSpotFlag << 1;

//	kInventorySpotFlag is a flag which marks hot spots which indicate inventory items
//	in the environment.
//	const HotSpotFlags kInventorySpotFlag = kBiochipDropSpotFlag << 1;

//	kBiochipSpotFlag is a flag which marks hot spots which indicate biochips
//	in the environment.
const HotSpotFlags kPickUpBiochipSpotFlag = kHighestGameShellSpotFlag << 1;
const HotSpotFlags kDropBiochipSpotFlag = kPickUpBiochipSpotFlag << 1;

const HotSpotFlags kInfoReturnSpotFlag = kDropBiochipSpotFlag << 1;

//	Biochip and inventory hot spot flags...

const HotSpotFlags kAIBiochipSpotFlag = kInfoReturnSpotFlag << 1;
const HotSpotFlags kPegasusBiochipSpotFlag = kAIBiochipSpotFlag << 1;
const HotSpotFlags kOpticalBiochipSpotFlag = kPegasusBiochipSpotFlag << 1;
const HotSpotFlags kAirMaskSpotFlag = kOpticalBiochipSpotFlag << 1;

const HotSpotFlags kJMPClickingSpotFlags = kClickSpotFlag |
											kPlayExtraSpotFlag |
											kOpenDoorSpotFlag |
											kInfoReturnSpotFlag |
											kAIBiochipSpotFlag |
											kPegasusBiochipSpotFlag |
											kOpticalBiochipSpotFlag |
											kAirMaskSpotFlag;

const int32 kMainMenuID = 1;
const int32 kPauseMenuID = 2;
const int32 kCreditsMenuID = 3;
const int32 kDeathMenuID = 4;

/////////////////////////////////////////////
//
//	Menu commands.

const GameMenuCommand kMenuCmdOverview = kMenuCmdNoCommand + 1;
const GameMenuCommand kMenuCmdStartAdventure = kMenuCmdOverview + 1;
const GameMenuCommand kMenuCmdStartWalkthrough = kMenuCmdStartAdventure + 1;
const GameMenuCommand kMenuCmdRestore = kMenuCmdStartWalkthrough + 1;
const GameMenuCommand kMenuCmdCredits = kMenuCmdRestore + 1;
const GameMenuCommand kMenuCmdQuit = kMenuCmdCredits + 1;

const GameMenuCommand kMenuCmdDeathContinue = kMenuCmdQuit + 1;

const GameMenuCommand kMenuCmdDeathQuitDemo = kMenuCmdDeathContinue + 1;
const GameMenuCommand kMenuCmdDeathMainMenuDemo = kMenuCmdDeathQuitDemo + 1;

const GameMenuCommand kMenuCmdDeathRestore = kMenuCmdDeathMainMenuDemo + 1;
const GameMenuCommand kMenuCmdDeathMainMenu = kMenuCmdDeathRestore + 1;

const GameMenuCommand kMenuCmdPauseSave = kMenuCmdDeathMainMenu + 1;
const GameMenuCommand kMenuCmdPauseContinue = kMenuCmdPauseSave + 1;
const GameMenuCommand kMenuCmdPauseRestore = kMenuCmdPauseContinue + 1;
const GameMenuCommand kMenuCmdPauseQuit = kMenuCmdPauseRestore + 1;

const GameMenuCommand kMenuCmdCreditsMainMenu = kMenuCmdPauseQuit + 1;

const GameMenuCommand kMenuCmdCancelRestart = kMenuCmdCreditsMainMenu + 1;
const GameMenuCommand kMenuCmdEjectRestart = kMenuCmdCancelRestart + 1;

const TimeValue kMenuButtonHiliteTime = 20;
const TimeScale kMenuButtonHiliteScale = kSixtyTicksPerSecond;

//	PICT resources:

//	Warning light PICTs:

const ResIDType kLightOffID = 128;
const ResIDType kLightYellowID = 129;
const ResIDType kLightOrangeID = 130;
const ResIDType kLightRedID = 131;

//	Date PICTs:

const ResIDType kDatePrehistoricID = 138;
const ResIDType kDate2112ID = 139;
const ResIDType kDate2185ID = 140;
const ResIDType kDate2310ID = 141;
const ResIDType kDate2318ID = 142;

/////////////////////////////////////////////
//
//	Display Order

const DisplayOrder kCroppedMovieLayer = 11000;

const DisplayOrder kMonitorLayer = 12000;

const DisplayOrder kDragSpriteLayer = 15000;
const DisplayOrder kDragSpriteOrder = kDragSpriteLayer;

const DisplayOrder kInterfaceLayer = 20000;
const DisplayOrder kBackground1Order = kInterfaceLayer;
const DisplayOrder kBackground2Order = kBackground1Order + 1;
const DisplayOrder kBackground3Order = kBackground2Order + 1;
const DisplayOrder kBackground4Order = kBackground3Order + 1;
const DisplayOrder kDateOrder = kBackground4Order + 1;
const DisplayOrder kCompassOrder = kDateOrder + 1;
const DisplayOrder kEnergyBarOrder = kCompassOrder + 1;
const DisplayOrder kEnergyLightOrder = kEnergyBarOrder + 1;

const DisplayOrder kAILayer = 22000;
const DisplayOrder kAILeftAreaOrder = kAILayer;
const DisplayOrder kAIMiddleAreaOrder = kAILeftAreaOrder + 1;
const DisplayOrder kAIRightAreaOrder = kAIMiddleAreaOrder + 1;
const DisplayOrder kAIMovieOrder = kAIRightAreaOrder + 1;

const DisplayOrder kHilitesLayer = 23000;
const DisplayOrder kInventoryHiliteOrder = kHilitesLayer;
const DisplayOrder kBiochipHiliteOrder = kInventoryHiliteOrder + 1;

const DisplayOrder kPanelsLayer = 25000;
const DisplayOrder kInventoryPushOrder = kPanelsLayer;
const DisplayOrder kInventoryLidOrder = kInventoryPushOrder + 1;
const DisplayOrder kBiochipPushOrder = kInventoryLidOrder + 1;
const DisplayOrder kBiochipLidOrder = kBiochipPushOrder + 1;
const DisplayOrder kFinalMessageOrder = kBiochipLidOrder + 1;

const DisplayOrder kInfoLayer = 26000;
const DisplayOrder kInfoBackgroundOrder = kInfoLayer;
const DisplayOrder kInfoSpinOrder = kInfoBackgroundOrder + 1;

const DisplayOrder kScreenDimmerOrder = 30000;

const DisplayOrder kPauseScreenLayer = 31000;
const DisplayOrder kPauseMenuOrder = kPauseScreenLayer;
const DisplayOrder kSaveGameOrder = kPauseMenuOrder + 1;
const DisplayOrder kContinueOrder = kSaveGameOrder + 1;
const DisplayOrder kRestoreOrder = kContinueOrder + 1;
const DisplayOrder kSoundFXOrder = kRestoreOrder + 1;
const DisplayOrder kAmbienceOrder = kSoundFXOrder + 1;
const DisplayOrder kWalkthruOrder = kAmbienceOrder + 1;
const DisplayOrder kQuitToMainMenuOrder = kWalkthruOrder + 1;
const DisplayOrder kPauseLargeHiliteOrder = kQuitToMainMenuOrder + 1;
const DisplayOrder kPauseSmallHiliteOrder = kPauseLargeHiliteOrder + 1;

/////////////////////////////////////////////
//
//	Death reasons.
enum {
	// Caldoria
	kDeathUncreatedInCaldoria = 1,
	kDeathCardBomb,
	kDeathShotBySinclair,
	kDeathSinclairShotDelegate,
	kDeathNuclearExplosion,

	// TSA
	kDeathUncreatedInTSA,
	kDeathShotByTSARobots,

	// Prehistoric
	kDeathFallOffCliff,
	kDeathEatenByDinosaur,
	kDeathStranded,

	// Norad
	kDeathGassedInNorad,
	kDeathArrestedInNorad,
	kDeathWokeUpNorad,
	kDeathSubDestroyed, // Unused
	kDeathRobotThroughNoradDoor,
	kDeathRobotSubControlRoom,

	// Mars
	kDeathWrongShuttleLock,
	kDeathArrestedInMars,
	kDeathRunOverByPod,
	kDeathDidntGetOutOfWay,
	kDeathReactorBurn,
	kDeathDidntFindMarsBomb,
	kDeathDidntDisarmMarsBomb,
	kDeathNoMaskInMaze,
	kDeathNoAirInMaze,
	kDeathGroundByMazebot,
	kDeathMissedOreBucket,
	kDeathDidntLeaveBucket,
	kDeathRanIntoCanyonWall, // Unused
	kDeathRanIntoSpaceJunk,

	// WSC
	kDeathDidntStopPoison,
	kDeathArrestedInWSC,
	kDeathHitByPlasma,
	kDeathShotOnCatwalk,

	// Winning
	kPlayerWonGame
};

static const CoordType kAILeftAreaLeft = 76;
static const CoordType kAILeftAreaTop = 334;

static const CoordType kAILeftAreaWidth = 96;
static const CoordType kAILeftAreaHeight = 96;

static const CoordType kAIMiddleAreaLeft = 172;
static const CoordType kAIMiddleAreaTop = 334;

static const CoordType kAIMiddleAreaWidth = 192;
static const CoordType kAIMiddleAreaHeight = 96;

static const CoordType kAIRightAreaLeft = 364;
static const CoordType kAIRightAreaTop = 334;

static const CoordType kAIRightAreaWidth = 96;
static const CoordType kAIRightAreaHeight = 96;

enum {
	kTSAPlayerNotArrived,				//	initial state, must be zero
	kTSAPlayerForcedReview,				//	Player must watch TBP before rip occurs.
	kTSAPlayerDetectedRip,				//	Player finished TBP, rip alarm just went off.
	kTSAPlayerNeedsHistoricalLog,		//	Player is instructed to get historical log
	kTSAPlayerGotHistoricalLog,
	kTSAPlayerInstalledHistoricalLog,
	kTSABossSawHistoricalLog,
	kRobotsAtCommandCenter,
	kRobotsAtFrontDoor,
	kRobotsAtReadyRoom,
	kPlayerLockedInPegasus,
	kPlayerOnWayToPrehistoric,
	kPlayerWentToPrehistoric,
	kPlayerOnWayToNorad,
	kPlayerOnWayToMars,
	kPlayerOnWayToWSC,
	kPlayerFinishedWithTSA
};

/////////////////////////////////////////////
//
//	Mode constants.

static const GameMode kModeInventoryPick = kLastGameShellMode + 1;
static const GameMode kModeBiochipPick = kModeInventoryPick + 1;
static const GameMode kModeInfoScreen = kModeBiochipPick + 1;

} // End of namespace Pegasus

#endif
