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

const tDisplayElementID kNoDisplayElement = -1;
const tDisplayElementID kHighestReservedElementID = -2;

const tDisplayElementID kCursorID = kHighestReservedElementID;
const tDisplayElementID kLoadScreenID = kCursorID - 1;

const tDisplayOrder kMinAvailableOrder = 0;
const tDisplayOrder kMaxAvailableOrder = 999998;
const tDisplayOrder kLoadScreenOrder = 900000;
const tDisplayOrder kCursorOrder = 1000000;

const tHotSpotID kNoHotSpotID = -1;
const tHotSpotFlags kNoHotSpotFlags = 0;
const tHotSpotFlags kAllHotSpotFlags = ~kNoHotSpotFlags;

const tNotificationFlags kNoNotificationFlags = 0;

const tDisplayElementID kCurrentDragSpriteID = 1000;

// TODO
//const Fixed kFixed1 = 1 << 16;
//const Fixed kFixedMinus1 = -1 << 16;

const TimeScale kDefaultTimeScale = 600;

// TODO
//const RGBColor kWhiteRGB = {0xFFFF, 0xFFFF, 0xFFFF};

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

const tNotificationID kNeighborhoodNotificationID = 1;
const tNotificationID kLastNeighborhoodNotificationID = kNeighborhoodNotificationID;

const tNotificationFlags kNeighborhoodMovieCompletedFlag = 1;
const tNotificationFlags kMoveForwardCompletedFlag = kNeighborhoodMovieCompletedFlag << 1;
const tNotificationFlags kStrideCompletedFlag = kMoveForwardCompletedFlag << 1;
const tNotificationFlags kTurnCompletedFlag = kStrideCompletedFlag << 1;
const tNotificationFlags kSpotCompletedFlag = kTurnCompletedFlag << 1;
const tNotificationFlags kDoorOpenCompletedFlag = kSpotCompletedFlag << 1;
const tNotificationFlags kExtraCompletedFlag = kDoorOpenCompletedFlag << 1;
const tNotificationFlags kSpotSoundCompletedFlag = kExtraCompletedFlag << 1;
const tNotificationFlags kDelayCompletedFlag = kSpotSoundCompletedFlag << 1;
const tNotificationFlags kActionRequestCompletedFlag = kDelayCompletedFlag << 1;
const tNotificationFlags kDeathExtraCompletedFlag = kActionRequestCompletedFlag << 1;
const tNotificationFlags kLastNeighborhoodNotificationFlag = kDeathExtraCompletedFlag;

const tNotificationFlags kNeighborhoodFlags =	kNeighborhoodMovieCompletedFlag |
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

const tDisplayElementID kNavMovieID = 1;
const tDisplayElementID kTurnPushID = 2;

const tDisplayElementID kMaxGameShellDisplayID = kTurnPushID;

//	Display ordering.

const tDisplayOrder kNavLayer = 10000;
const tDisplayOrder kNavMovieOrder = kNavLayer;
const tDisplayOrder kTurnPushOrder = kNavMovieOrder + 1;

/////////////////////////////////////////////
//
//	Display IDs.

const tDisplayElementID kScreenDimmerID = kMaxGameShellDisplayID + 1;
const tDisplayElementID kInterface1ID = kScreenDimmerID + 1;
const tDisplayElementID kInterface2ID = kInterface1ID + 1;
const tDisplayElementID kInterface3ID = kInterface2ID + 1;
const tDisplayElementID kInterface4ID = kInterface3ID + 1;
const tDisplayElementID kDateID = kInterface4ID + 1;
const tDisplayElementID kCompassID = kDateID + 1;
const tDisplayElementID kInventoryPushID = kCompassID + 1;
const tDisplayElementID kInventoryLidID = kInventoryPushID + 1;
const tDisplayElementID kBiochipPushID = kInventoryLidID + 1;
const tDisplayElementID kBiochipLidID = kBiochipPushID + 1;
const tDisplayElementID kEnergyBarID = kBiochipLidID + 1;
const tDisplayElementID kWarningLightID = kEnergyBarID + 1;
const tDisplayElementID kAILeftAreaID = kWarningLightID + 1;
const tDisplayElementID kAIMiddleAreaID = kAILeftAreaID + 1;
const tDisplayElementID kAIRightAreaID = kAIMiddleAreaID + 1;
const tDisplayElementID kAIMovieID = kAIRightAreaID + 1;
const tDisplayElementID kInventoryDropHighlightID = kAIMovieID + 1;
const tDisplayElementID kBiochipDropHighlightID = kInventoryDropHighlightID + 1;

const tDisplayElementID kDraggingSpriteID = 1000;

const tDisplayElementID kCroppedMovieID = 2000;

const tDisplayElementID kNeighborhoodDisplayID = 3000;

const tDisplayElementID kItemPictureBaseID = 5000;

const tCoordType kNavAreaLeft = 64;
const tCoordType kNavAreaTop = 64;

const tCoordType kBackground1Left = 0;
const tCoordType kBackground1Top = 64;

const tCoordType kBackground2Left = 0;
const tCoordType kBackground2Top = 0;

const tCoordType kBackground3Left = 576;
const tCoordType kBackground3Top = 64;

const tCoordType kBackground4Left = 0;
const tCoordType kBackground4Top = 320;

const tCoordType kOverviewControllerLeft = 540;
const tCoordType kOverviewControllerTop = 348;

const tCoordType kSwapLeft = 194;
const tCoordType kSwapTop = 116;

const tCoordType kSwapHiliteLeft = 200;
const tCoordType kSwapHiliteTop = 206;

const tCoordType kDateLeft = 136;
const tCoordType kDateTop = 44;

const tCoordType kCompassLeft = 222;
const tCoordType kCompassTop = 42;
const tCoordType kCompassWidth = 92;

const tCoordType kInventoryPushLeft = 74;
const tCoordType kInventoryPushTop = 92;

const tCoordType kInventoryLidLeft = 74;
const tCoordType kInventoryLidTop = 316;

const tCoordType kBiochipPushLeft = 362;
const tCoordType kBiochipPushTop = 192;

const tCoordType kBiochipLidLeft = 362;
const tCoordType kBiochipLidTop = 316;

// TODO: Remove global variable needs
//const Common::Rect kInventoryHiliteBounds(334, 76, 430, 172);
//const Common::Rect kBiochipHiliteBounds (334, 364, 430, 460);

const tCoordType kInventoryDropLeft = 0;
const tCoordType kInventoryDropTop = 320;
const tCoordType kInventoryDropRight = 232;
const tCoordType kInventoryDropBottom = 480;

const tCoordType kBiochipDropLeft = 302;
const tCoordType kBiochipDropTop = 320;
const tCoordType kBiochipDropRight = 640;
const tCoordType kBiochipDropBottom = 480;

const tCoordType kFinalMessageLeft = kInventoryPushLeft + 1;
const tCoordType kFinalMessageTop = kInventoryPushTop + 24;

/////////////////////////////////////////////
//
//	Notifications.

const tNotificationID kJMPDCShellNotificationID = kLastNeighborhoodNotificationID + 1;
const tNotificationID kInterfaceNotificationID = kJMPDCShellNotificationID + 1;
const tNotificationID kAINotificationID = kInterfaceNotificationID + 1;
const tNotificationID kNoradNotificationID = kAINotificationID + 1;
const tNotificationID kNoradECRNotificationID = kNoradNotificationID + 1;
const tNotificationID kNoradFillingStationNotificationID = kNoradECRNotificationID + 1;
const tNotificationID kNoradPressureNotificationID = kNoradFillingStationNotificationID + 1;
const tNotificationID kNoradUtilityNotificationID = kNoradPressureNotificationID + 1;
const tNotificationID kNoradElevatorNotificationID = kNoradUtilityNotificationID + 1;
const tNotificationID kNoradSubPlatformNotificationID = kNoradElevatorNotificationID + 1;
const tNotificationID kSubControlNotificationID = kNoradSubPlatformNotificationID + 1;
const tNotificationID kNoradGreenBallNotificationID = kSubControlNotificationID + 1;
const tNotificationID kNoradGlobeNotificationID = kNoradGreenBallNotificationID + 1;
const tNotificationID kCaldoriaVidPhoneNotificationID = kNoradGlobeNotificationID + 1;
const tNotificationID kCaldoriaMessagesNotificationID = kCaldoriaVidPhoneNotificationID + 1;
const tNotificationID kCaldoriaBombTimerNotificationID = kCaldoriaMessagesNotificationID + 1;

//	Sent to the shell by fShellNotification.
const tNotificationFlags kGameStartingFlag = 1;
const tNotificationFlags kNeedNewJumpFlag = kGameStartingFlag << 1;
const tNotificationFlags kPlayerDiedFlag = kNeedNewJumpFlag << 1;

const tNotificationFlags kJMPShellNotificationFlags = kGameStartingFlag |
														kNeedNewJumpFlag |
														kPlayerDiedFlag;

//	Sent to the interface.
const tNotificationFlags kInventoryLidOpenFlag = 1;
const tNotificationFlags kInventoryLidClosedFlag = kInventoryLidOpenFlag << 1;
const tNotificationFlags kInventoryDrawerUpFlag = kInventoryLidClosedFlag << 1;
const tNotificationFlags kInventoryDrawerDownFlag = kInventoryDrawerUpFlag << 1;
const tNotificationFlags kBiochipLidOpenFlag = kInventoryDrawerDownFlag << 1;
const tNotificationFlags kBiochipLidClosedFlag = kBiochipLidOpenFlag << 1;
const tNotificationFlags kBiochipDrawerUpFlag = kBiochipLidClosedFlag << 1;
const tNotificationFlags kBiochipDrawerDownFlag = kBiochipDrawerUpFlag << 1;

const tNotificationFlags kInterfaceNotificationFlags =	kInventoryLidOpenFlag |
														kInventoryLidClosedFlag |
														kInventoryDrawerUpFlag |
														kInventoryDrawerDownFlag |
														kBiochipLidOpenFlag |
														kBiochipLidClosedFlag |
														kBiochipDrawerUpFlag |
														kBiochipDrawerDownFlag;

//	Hot spots.

//	Neighborhood hot spots.

const tHotSpotID kFirstNeighborhoodSpotID = 5000;

//	kShellSpotFlag is a flag which marks all hot spots which belong to the shell, like
//	the current item and current biochip spots.
const tHotSpotFlags kShellSpotFlag = 1;
//	kNeighborhoodSpotFlag is a flag which marks all hot spots which belong to a
//	neighborhood, like buttons on walls and so on.
const tHotSpotFlags kNeighborhoodSpotFlag = kShellSpotFlag << 1;
//	kZoomInSpotFlag is a flag which marks all hot spots which indicate a zoom.
const tHotSpotFlags kZoomInSpotFlag = kNeighborhoodSpotFlag << 1;
//	kZoomOutSpotFlag is a flag which marks all hot spots which indicate a zoom.
const tHotSpotFlags kZoomOutSpotFlag = kZoomInSpotFlag << 1;

const tHotSpotFlags kClickSpotFlag = kZoomOutSpotFlag << 1;
const tHotSpotFlags kPlayExtraSpotFlag = kClickSpotFlag << 1;
const tHotSpotFlags kPickUpItemSpotFlag = kPlayExtraSpotFlag << 1;
const tHotSpotFlags kDropItemSpotFlag = kPickUpItemSpotFlag << 1;
const tHotSpotFlags kOpenDoorSpotFlag = kDropItemSpotFlag << 1;

const tHotSpotFlags kZoomSpotFlags = kZoomInSpotFlag | kZoomOutSpotFlag;

const tHotSpotFlags kHighestGameShellSpotFlag = kOpenDoorSpotFlag;

/////////////////////////////////////////////
//
//	Hot spots.

//	Shell hot spots.
//	The shell reserves all hot spot IDs from 0 to 999

const tHotSpotID kCurrentItemSpotID = 0;
const tHotSpotID kCurrentBiochipSpotID = kCurrentItemSpotID + 1;

const tHotSpotID kInventoryDropSpotID = kCurrentBiochipSpotID + 1;
const tHotSpotID kBiochipDropSpotID = kInventoryDropSpotID + 1;

const tHotSpotID kInfoReturnSpotID = kBiochipDropSpotID + 1;

const tHotSpotID kAIHint1SpotID = kInfoReturnSpotID + 1;
const tHotSpotID kAIHint2SpotID = kAIHint1SpotID + 1;
const tHotSpotID kAIHint3SpotID = kAIHint2SpotID + 1;
const tHotSpotID kAISolveSpotID = kAIHint3SpotID + 1;
const tHotSpotID kAIBriefingSpotID = kAISolveSpotID + 1;
const tHotSpotID kAIScanSpotID = kAIBriefingSpotID + 1;

const tHotSpotID kPegasusRecallSpotID = kAIScanSpotID + 1;

const tHotSpotID kAriesSpotID = kPegasusRecallSpotID + 1;
const tHotSpotID kMercurySpotID = kAriesSpotID + 1;
const tHotSpotID kPoseidonSpotID = kMercurySpotID + 1;

const tHotSpotID kAirMaskToggleSpotID = kPoseidonSpotID + 1;

const tHotSpotID kShuttleEnergySpotID = kAirMaskToggleSpotID + 1;
const tHotSpotID kShuttleGravitonSpotID = kShuttleEnergySpotID + 1;
const tHotSpotID kShuttleTractorSpotID = kShuttleGravitonSpotID + 1;
const tHotSpotID kShuttleViewSpotID = kShuttleTractorSpotID + 1;
const tHotSpotID kShuttleTransportSpotID = kShuttleViewSpotID + 1;

//	Most of these are obsolete:

//	kInventoryDropSpotFlag is a flag which marks hot spots which are valid drop spots
//	for inventory items.
//	const tHotSpotFlags kInventoryDropSpotFlag = kHighestGameShellSpotFlag << 1;

//	kBiochipDropSpotFlag is a flag which marks hot spots which are valid drop spots
//	for biochips.
//	const tHotSpotFlags kBiochipDropSpotFlag = kInventoryDropSpotFlag << 1;

//	kInventorySpotFlag is a flag which marks hot spots which indicate inventory items
//	in the environment.
//	const tHotSpotFlags kInventorySpotFlag = kBiochipDropSpotFlag << 1;

//	kBiochipSpotFlag is a flag which marks hot spots which indicate biochips
//	in the environment.
const tHotSpotFlags kPickUpBiochipSpotFlag = kHighestGameShellSpotFlag << 1;
const tHotSpotFlags kDropBiochipSpotFlag = kPickUpBiochipSpotFlag << 1;

const tHotSpotFlags kInfoReturnSpotFlag = kDropBiochipSpotFlag << 1;

//	Biochip and inventory hot spot flags...

const tHotSpotFlags kAIBiochipSpotFlag = kInfoReturnSpotFlag << 1;
const tHotSpotFlags kPegasusBiochipSpotFlag = kAIBiochipSpotFlag << 1;
const tHotSpotFlags kOpticalBiochipSpotFlag = kPegasusBiochipSpotFlag << 1;
const tHotSpotFlags kAirMaskSpotFlag = kOpticalBiochipSpotFlag << 1;

const tHotSpotFlags kJMPClickingSpotFlags = kClickSpotFlag |
											kPlayExtraSpotFlag |
											kOpenDoorSpotFlag |
											kInfoReturnSpotFlag |
											kAIBiochipSpotFlag |
											kPegasusBiochipSpotFlag |
											kOpticalBiochipSpotFlag |
											kAirMaskSpotFlag;

const tMM32BitID kMainMenuID = 1;
const tMM32BitID kPauseMenuID = 2;
const tMM32BitID kCreditsMenuID = 3;
const tMM32BitID kDeathMenuID = 4;

/////////////////////////////////////////////
//
//	Menu commands.

const tGameMenuCommand kMenuCmdOverview = kMenuCmdNoCommand + 1;
const tGameMenuCommand kMenuCmdStartAdventure = kMenuCmdOverview + 1;
const tGameMenuCommand kMenuCmdStartWalkthrough = kMenuCmdStartAdventure + 1;
const tGameMenuCommand kMenuCmdRestore = kMenuCmdStartWalkthrough + 1;
const tGameMenuCommand kMenuCmdCredits = kMenuCmdRestore + 1;
const tGameMenuCommand kMenuCmdQuit = kMenuCmdCredits + 1;

const tGameMenuCommand kMenuCmdDeathContinue = kMenuCmdQuit + 1;

const tGameMenuCommand kMenuCmdDeathQuitDemo = kMenuCmdDeathContinue + 1;
const tGameMenuCommand kMenuCmdDeathMainMenuDemo = kMenuCmdDeathQuitDemo + 1;

const tGameMenuCommand kMenuCmdDeathRestore = kMenuCmdDeathMainMenuDemo + 1;
const tGameMenuCommand kMenuCmdDeathMainMenu = kMenuCmdDeathRestore + 1;

const tGameMenuCommand kMenuCmdPauseSave = kMenuCmdDeathMainMenu + 1;
const tGameMenuCommand kMenuCmdPauseContinue = kMenuCmdPauseSave + 1;
const tGameMenuCommand kMenuCmdPauseRestore = kMenuCmdPauseContinue + 1;
const tGameMenuCommand kMenuCmdPauseQuit = kMenuCmdPauseRestore + 1;

const tGameMenuCommand kMenuCmdCreditsMainMenu = kMenuCmdPauseQuit + 1;

const tGameMenuCommand kMenuCmdCancelRestart = kMenuCmdCreditsMainMenu + 1;
const tGameMenuCommand kMenuCmdEjectRestart = kMenuCmdCancelRestart + 1;

const TimeValue kMenuButtonHiliteTime = 20;
const TimeScale kMenuButtonHiliteScale = kSixtyTicksPerSecond;

//	PICT resources:

//	Warning light PICTs:

const tResIDType kLightOffID = 128;
const tResIDType kLightYellowID = 129;
const tResIDType kLightOrangeID = 130;
const tResIDType kLightRedID = 131;

//	Date PICTs:

const tResIDType kDatePrehistoricID = 138;
const tResIDType kDate2112ID = 139;
const tResIDType kDate2185ID = 140;
const tResIDType kDate2310ID = 141;
const tResIDType kDate2318ID = 142;

/////////////////////////////////////////////
//
//	Display Order

const tDisplayOrder kCroppedMovieLayer = 11000;

const tDisplayOrder kMonitorLayer = 12000;

const tDisplayOrder kDragSpriteLayer = 15000;
const tDisplayOrder kDragSpriteOrder = kDragSpriteLayer;

const tDisplayOrder kInterfaceLayer = 20000;
const tDisplayOrder kBackground1Order = kInterfaceLayer;
const tDisplayOrder kBackground2Order = kBackground1Order + 1;
const tDisplayOrder kBackground3Order = kBackground2Order + 1;
const tDisplayOrder kBackground4Order = kBackground3Order + 1;
const tDisplayOrder kDateOrder = kBackground4Order + 1;
const tDisplayOrder kCompassOrder = kDateOrder + 1;
const tDisplayOrder kEnergyBarOrder = kCompassOrder + 1;
const tDisplayOrder kEnergyLightOrder = kEnergyBarOrder + 1;

const tDisplayOrder kAILayer = 22000;
const tDisplayOrder kAILeftAreaOrder = kAILayer;
const tDisplayOrder kAIMiddleAreaOrder = kAILeftAreaOrder + 1;
const tDisplayOrder kAIRightAreaOrder = kAIMiddleAreaOrder + 1;
const tDisplayOrder kAIMovieOrder = kAIRightAreaOrder + 1;

const tDisplayOrder kHilitesLayer = 23000;
const tDisplayOrder kInventoryHiliteOrder = kHilitesLayer;
const tDisplayOrder kBiochipHiliteOrder = kInventoryHiliteOrder + 1;

const tDisplayOrder kPanelsLayer = 25000;
const tDisplayOrder kInventoryPushOrder = kPanelsLayer;
const tDisplayOrder kInventoryLidOrder = kInventoryPushOrder + 1;
const tDisplayOrder kBiochipPushOrder = kInventoryLidOrder + 1;
const tDisplayOrder kBiochipLidOrder = kBiochipPushOrder + 1;
const tDisplayOrder kFinalMessageOrder = kBiochipLidOrder + 1;

const tDisplayOrder kInfoLayer = 26000;
const tDisplayOrder kInfoBackgroundOrder = kInfoLayer;
const tDisplayOrder kInfoSpinOrder = kInfoBackgroundOrder + 1;

const tDisplayOrder kScreenDimmerOrder = 30000;

const tDisplayOrder kPauseScreenLayer = 31000;
const tDisplayOrder kPauseMenuOrder = kPauseScreenLayer;
const tDisplayOrder kSaveGameOrder = kPauseMenuOrder + 1;
const tDisplayOrder kContinueOrder = kSaveGameOrder + 1;
const tDisplayOrder kRestoreOrder = kContinueOrder + 1;
const tDisplayOrder kSoundFXOrder = kRestoreOrder + 1;
const tDisplayOrder kAmbienceOrder = kSoundFXOrder + 1;
const tDisplayOrder kWalkthruOrder = kAmbienceOrder + 1;
const tDisplayOrder kQuitToMainMenuOrder = kWalkthruOrder + 1;
const tDisplayOrder kPauseLargeHiliteOrder = kQuitToMainMenuOrder + 1;
const tDisplayOrder kPauseSmallHiliteOrder = kPauseLargeHiliteOrder + 1;

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

static const tCoordType kAILeftAreaLeft = 76;
static const tCoordType kAILeftAreaTop = 334;

static const tCoordType kAILeftAreaWidth = 96;
static const tCoordType kAILeftAreaHeight = 96;

static const tCoordType kAIMiddleAreaLeft = 172;
static const tCoordType kAIMiddleAreaTop = 334;

static const tCoordType kAIMiddleAreaWidth = 192;
static const tCoordType kAIMiddleAreaHeight = 96;

static const tCoordType kAIRightAreaLeft = 364;
static const tCoordType kAIRightAreaTop = 334;

static const tCoordType kAIRightAreaWidth = 96;
static const tCoordType kAIRightAreaHeight = 96;

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

static const tDirectionConstant kNorth = 0;
static const tDirectionConstant kSouth = 1;
static const tDirectionConstant kEast = 2;
static const tDirectionConstant kWest = 3;

// TODO: Remove me
const tRoomID kTSA37 = 42;
const tRoomID kTinyTSA37 = 0;

} // End of namespace Pegasus

#endif
