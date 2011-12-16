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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_CONSTANTS_H
#define PEGASUS_NEIGHBORHOOD_NORAD_CONSTANTS_H

#include "pegasus/constants.h"

namespace Pegasus {

// Norad Alpha spot constants

const TimeValue kAlphaBumpIntoWallIn = 0;
const TimeValue kAlphaBumpIntoWallOut = 303;

const TimeValue kAlphaAccessDeniedIn = 303;
const TimeValue kAlphaAccessDeniedOut = 3045;

const TimeValue kAlphaRegDoorCloseIn = 3045;
const TimeValue kAlphaRegDoorCloseOut = 4476;

const TimeValue kAlphaElevatorDoorCloseIn = 4476;
const TimeValue kAlphaElevatorDoorCloseOut = 5071;

const TimeValue kAlphaCantTransportIn = 5071;
const TimeValue kAlphaCantTransportOut = 9348;

const TimeValue kAlphaPressureDoorIntro1In = 9348;
const TimeValue kAlphaPressureDoorIntro1Out = 11061;

const TimeValue kAlphaPressureDoorIntro2In = 11061;
const TimeValue kAlphaPressureDoorIntro2Out = 14098;

const TimeValue kN22ReplyIn = 14098;
const TimeValue kN22ReplyOut = 18442;

const TimeValue kAlphaLoadClawIntroIn = 18442;
const TimeValue kAlphaLoadClawIntroOut = 20698;

// Norad Delta spot constants

const TimeValue kDeltaBumpIntoWallIn = 0;
const TimeValue kDeltaBumpIntoWallOut = 303;

const TimeValue kDeltaAccessDeniedIn = 303;
const TimeValue kDeltaAccessDeniedOut = 3045;

const TimeValue kDeltaRegDoorCloseIn = 3045;
const TimeValue kDeltaRegDoorCloseOut = 4476;

const TimeValue kDeltaElevatorDoorCloseIn = 4476;
const TimeValue kDeltaElevatorDoorCloseOut = 5071;

const TimeValue kPressureDoorIntro1In = 5071;
const TimeValue kPressureDoorIntro1Out = 6784;

const TimeValue kPressureDoorIntro2In = 6784;
const TimeValue kPressureDoorIntro2Out = 9821;

const TimeValue kLoadClawIntroIn = 9821;
const TimeValue kLoadClawIntroOut = 12077;

const TimeValue kHoldForRetinalIn = 12077;
const TimeValue kHoldForRetinalOut = 14104;

const TimeValue kRetinalScanFailedIn = 14104;
const TimeValue kRetinalScanFailedOut = 17538;

const TimeValue kAddisAbabaIn = 17538;
const TimeValue kAddisAbabaOut = 19263;

const TimeValue kBangkokIn = 19263;
const TimeValue kBangkokOut = 20201;

const TimeValue kBonnIn = 20201;
const TimeValue kBonnOut = 20915;

const TimeValue kDublinIn = 20915;
const TimeValue kDublinOut = 21660;

const TimeValue kHonoluluIn = 21660;
const TimeValue kHonoluluOut = 22498;

const TimeValue kMadridIn = 22498;
const TimeValue kMadridOut = 23474;

const TimeValue kReykjavikIn = 23474;
const TimeValue kReykjavikOut = 24488;

const TimeValue kSanAntonioIn = 24488;
const TimeValue kSanAntonioOut = 25561;

const TimeValue kSeoulIn = 25561;
const TimeValue kSeoulOut = 26461;

const TimeValue kSvortalskIn = 26461;
const TimeValue kSvortalskOut = 27582;

const TimeValue kSiloBeepIn = 27582;
const TimeValue kSiloBeepOut = 27721;

const TimeValue kAllSilosDeactivatedIn = 27721;
const TimeValue kAllSilosDeactivatedOut = 28928;

const TimeValue kGlobalLaunchOverrideIn = 28928;
const TimeValue kGlobalLaunchOverrideOut = 30736;

const TimeValue kLaunchSiloSelectedIn = 30736;
const TimeValue kLaunchSiloSelectedOut = 31660;

const TimeValue kLaunchToProceedIn = 31660;
const TimeValue kLaunchToProceedOut = 32536;

const TimeValue kMaximumDeactivationIn = 32536;
const TimeValue kMaximumDeactivationOut = 34337;

const TimeValue kMissileLaunchedIn = 34337;
const TimeValue kMissileLaunchedOut = 35082;

const TimeValue kNewLaunchSiloIn = 35082;
const TimeValue kNewLaunchSiloOut = 36320;

const TimeValue kStrikeAuthorizedIn = 36320;
const TimeValue kStrikeAuthorizedOut = 37393;

const TimeValue kPrimaryTargetIn = 37393;
const TimeValue kPrimaryTargetOut = 38628;

const TimeValue kSiloDeactivatedIn = 38628;
const TimeValue kSiloDeactivatedOut = 39566;

const TimeValue kStrikeCodeRejectedIn = 39566;
const TimeValue kStrikeCodeRejectedOut = 41056;

const TimeValue kToDeactivateIn = 41056;
const TimeValue kToDeactivateOut = 46494;

const TimeValue kTwoMinutesIn = 46494;
const TimeValue kTwoMinutesOut = 47166;

const TimeValue kOneMinuteIn = 47166;
const TimeValue kOneMinuteOut = 47856;

const TimeValue kFiftySecondsIn = 47856;
const TimeValue kFiftySecondsOut = 48691;

const TimeValue kFortySecondsIn = 48691;
const TimeValue kFortySecondsOut = 49500;

const TimeValue kThirtySecondsIn = 49500;
const TimeValue kThirtySecondsOut = 50362;

const TimeValue kTwentySecondsIn = 50362;
const TimeValue kTwentySecondsOut = 51245;

const TimeValue kTenSecondsIn = 51245;
const TimeValue kTenSecondsOut = 52069;

const TimeValue kGiveUpHumanIn = 52069;
const TimeValue kGiveUpHumanOut = 55023;

const TimeValue kIJustBrokeIn = 55023;
const TimeValue kIJustBrokeOut = 59191;

const TimeValue kTheOnlyGoodHumanIn = 59191;
const TimeValue kTheOnlyGoodHumanOut = 62379;

const TimeValue kYouAreRunningIn = 62379;
const TimeValue kYouAreRunningOut = 64201;

const TimeValue kYouCannotPossiblyIn = 64201;
const TimeValue kYouCannotPossiblyOut = 65740;

const TimeValue kYouWillFailIn = 65740;
const TimeValue kYouWillFailOut = 67217;

const CanOpenDoorReason kCantOpenBadPressure = kCantOpenLastReason + 1;

const NotificationFlags kAirTimerExpiredFlag = kLastNeighborhoodNotificationFlag << 1;

const uint16 kNoradWarningVolume = 0x100 / 3;
const uint16 kNoradSuckWindVolume = 0x100 / 2;

const int16 kElevatorCompassAngle = -40;
const int16 kSubPlatformCompassAngle = 45;
const int16 kSubControlCompassAngle = -10;

//	Norad interactions.

const InteractionID kNoradGlobeGameInteractionID = 0;
const InteractionID kNoradECRMonitorInteractionID = 1;
const InteractionID kNoradFillingStationInteractionID = 2;
const InteractionID kNoradElevatorInteractionID = 3;
const InteractionID kNoradPressureDoorInteractionID = 4;
const InteractionID kNoradSubControlRoomInteractionID = 5;
const InteractionID kNoradSubPlatformInteractionID = 6;

/////////////////////////////////////////////
//
//	Norad Alpha

const CoordType kECRSlideShowLeft = kNavAreaLeft + 78;
const CoordType kECRSlideShowTop = kNavAreaTop + 1;

const CoordType kECRPanLeft = kNavAreaLeft + 78 + 5;
const CoordType kECRPanTop = kNavAreaTop + 1 + 4;
const CoordType kECRPanRight = kECRPanLeft + 213;
const CoordType kECRPanBottom = kECRPanTop + 241;

const CoordType kNoradAlphaElevatorControlsLeft = kNavAreaLeft + 332;
const CoordType kNoradAlphaElevatorControlsTop = kNavAreaTop + 127;

const CoordType kNoradAlpha01LeftSideLeft = kNavAreaLeft + 0;
const CoordType kNoradAlpha01LeftSideTop = kNavAreaTop + 0;

const CoordType kNoradAlpha01RightSideLeft = kNavAreaLeft + 240;
const CoordType kNoradAlpha01RightSideTop = kNavAreaTop + 12;

const CoordType kNoradUpperLevelsLeft = kNavAreaLeft + 98;
const CoordType kNoradUpperLevelsTop = kNavAreaTop + 31;

const CoordType kNoradUpperTypeLeft = kNoradUpperLevelsLeft + 114;
const CoordType kNoradUpperTypeTop = kNoradUpperLevelsTop + 8;

const CoordType kNoradUpperUpLeft = kNavAreaLeft + 361;
const CoordType kNoradUpperUpTop = kNavAreaTop + 32;

const CoordType kNoradUpperDownLeft = kNavAreaLeft + 367;
const CoordType kNoradUpperDownTop = kNavAreaTop + 66;

const CoordType kNoradLowerLevelsLeft = kNavAreaLeft + 74;
const CoordType kNoradLowerLevelsTop = kNavAreaTop + 157;

const CoordType kNoradLowerTypeLeft = kNoradLowerLevelsLeft + 144;
const CoordType kNoradLowerTypeTop = kNoradLowerLevelsTop + 9;

const CoordType kNoradLowerUpLeft = kNavAreaLeft + 380;
const CoordType kNoradLowerUpTop = kNavAreaTop + 164;

const CoordType kNoradLowerDownLeft = kNavAreaLeft + 388;
const CoordType kNoradLowerDownTop = kNavAreaTop + 212;

const CoordType kNoradPlatformLeft = kNavAreaLeft + 36;
const CoordType kNoradPlatformTop = kNavAreaTop + 87;

const CoordType kNoradSubControlLeft = kNavAreaLeft + 0;
const CoordType kNoradSubControlTop = kNavAreaTop + 84;

const CoordType kNoradSubControlPinchLeft = kNoradSubControlLeft + 106;
const CoordType kNoradSubControlPinchTop = kNoradSubControlTop + 86;

const CoordType kNoradSubControlDownLeft = kNoradSubControlLeft + 66;
const CoordType kNoradSubControlDownTop = kNoradSubControlTop + 106;

const CoordType kNoradSubControlRightLeft = kNoradSubControlLeft + 83;
const CoordType kNoradSubControlRightTop = kNoradSubControlTop + 90;

const CoordType kNoradSubControlLeftLeft = kNoradSubControlLeft + 56;
const CoordType kNoradSubControlLeftTop = kNoradSubControlTop + 91;

const CoordType kNoradSubControlUpLeft = kNoradSubControlLeft + 66;
const CoordType kNoradSubControlUpTop = kNoradSubControlTop + 81;

const CoordType kNoradSubControlCCWLeft = kNoradSubControlLeft + 29;
const CoordType kNoradSubControlCCWTop = kNoradSubControlTop + 88;

const CoordType kNoradSubControlCWLeft = kNoradSubControlLeft + 0;
const CoordType kNoradSubControlCWTop = kNoradSubControlTop + 89;

const CoordType kNoradClawMonitorLeft = kNavAreaLeft + 288;
const CoordType kNoradClawMonitorTop = kNavAreaTop + 97;

const CoordType kNoradGreenBallAtALeft = kNoradClawMonitorLeft + 179;
const CoordType kNoradGreenBallAtATop = kNoradClawMonitorTop + 82;

const CoordType kNoradGreenBallAtBLeft = kNoradClawMonitorLeft + 130;
const CoordType kNoradGreenBallAtBTop = kNoradClawMonitorTop + 73;

const CoordType kNoradGreenBallAtCLeft = kNoradClawMonitorLeft + 110;
const CoordType kNoradGreenBallAtCTop = kNoradClawMonitorTop + 26;

const CoordType kNoradGreenBallAtDLeft = kNoradClawMonitorLeft + 21;
const CoordType kNoradGreenBallAtDTop = kNoradClawMonitorTop + 49;

/////////////////////////////////////////////
//
//	Norad Delta

const CoordType kGlobeMonitorLeft = kNavAreaLeft + 360;
const CoordType kGlobeMonitorTop = kNavAreaTop + 144;

const CoordType kGlobeLeft = kNavAreaLeft + 172;
const CoordType kGlobeTop = kNavAreaTop;

const CoordType kGlobeCircleLeftLeft = kNavAreaLeft + 186;
const CoordType kGlobeCircleLeftTop = kNavAreaTop + 41;

const CoordType kGlobeCircleRightLeft = kNavAreaLeft + 321;
const CoordType kGlobeCircleRightTop = kNavAreaTop + 41;

const CoordType kGlobeCircleUpLeft = kNavAreaLeft + 220;
const CoordType kGlobeCircleUpTop = kNavAreaTop + 7;

const CoordType kGlobeCircleDownLeft = kNavAreaLeft + 220;
const CoordType kGlobeCircleDownTop = kNavAreaTop + 142;

const CoordType kGlobeUpperLeftHiliteLeft = kNavAreaLeft + 207;
const CoordType kGlobeUpperLeftHiliteTop = kNavAreaTop + 28;

const CoordType kGlobeUpperRightHiliteLeft = kNavAreaLeft + 307;
const CoordType kGlobeUpperRightHiliteTop = kNavAreaTop + 28;

const CoordType kGlobeLowerLeftHiliteLeft = kNavAreaLeft + 207;
const CoordType kGlobeLowerLeftHiliteTop = kNavAreaTop + 128;

const CoordType kGlobeLowerRightHiliteLeft = kNavAreaLeft + 307;
const CoordType kGlobeLowerRightHiliteTop = kNavAreaTop + 128;

const CoordType kGlobeLeftMotionHiliteLeft = kNavAreaLeft + 182;
const CoordType kGlobeLeftMotionHiliteTop = kNavAreaTop + 60;

const CoordType kGlobeRightMotionHiliteLeft = kNavAreaLeft + 331;
const CoordType kGlobeRightMotionHiliteTop = kNavAreaTop + 60;

const CoordType kGlobeUpMotionHiliteLeft = kNavAreaLeft + 239;
const CoordType kGlobeUpMotionHiliteTop = kNavAreaTop + 3;

const CoordType kGlobeDownMotionHiliteLeft = kNavAreaLeft + 239;
const CoordType kGlobeDownMotionHiliteTop = kNavAreaTop + 152;

const CoordType kGlobeUpperNamesLeft = kNavAreaLeft + 368;
const CoordType kGlobeUpperNamesTop = kNavAreaTop + 188;

const CoordType kGlobeLowerNamesLeft = kNavAreaLeft + 368;
const CoordType kGlobeLowerNamesTop = kNavAreaTop + 212;

const CoordType kGlobeCountdownLeft = kNavAreaLeft + 478;
const CoordType kGlobeCountdownTop = kNavAreaTop + 164;

//	Norad Alpha display IDs.

const DisplayElementID kECRSlideShowMovieID = kNeighborhoodDisplayID;
const DisplayElementID kECRPanID = kECRSlideShowMovieID + 1;
const DisplayElementID kNoradAlphaDeathMovieID = kECRPanID + 1;
const DisplayElementID kNoradElevatorControlsID = kNoradAlphaDeathMovieID + 1;
const DisplayElementID kN01LeftSideID = kNoradElevatorControlsID + 1;
const DisplayElementID kN01RightSideID = kN01LeftSideID + 1;
const DisplayElementID kPressureDoorLevelsID = kN01RightSideID + 1;
const DisplayElementID kPressureDoorTypeID = kPressureDoorLevelsID + 1;
const DisplayElementID kPressureDoorUpButtonID = kPressureDoorTypeID + 1;
const DisplayElementID kPressureDoorDownButtonID = kPressureDoorUpButtonID + 1;
const DisplayElementID kPlatformMonitorID = kPressureDoorDownButtonID + 1;
const DisplayElementID kSubControlMonitorID = kPlatformMonitorID + 1;
const DisplayElementID kClawMonitorID = kSubControlMonitorID + 1;
const DisplayElementID kSubControlPinchID = kClawMonitorID + 1;
const DisplayElementID kSubControlDownID = kSubControlPinchID + 1;
const DisplayElementID kSubControlRightID = kSubControlDownID + 1;
const DisplayElementID kSubControlLeftID = kSubControlRightID + 1;
const DisplayElementID kSubControlUpID = kSubControlLeftID + 1;
const DisplayElementID kSubControlCCWID = kSubControlUpID + 1;
const DisplayElementID kSubControlCWID = kSubControlCCWID + 1;
const DisplayElementID kClawMonitorGreenBallID = kSubControlCWID + 1;

//	Norad Delta display IDs.

const DisplayElementID kGlobeMonitorID = kNeighborhoodDisplayID;
const DisplayElementID kGlobeMovieID = kGlobeMonitorID + 14;
const DisplayElementID kGlobeCircleLeftID = kGlobeMovieID + 1;
const DisplayElementID kGlobeCircleRightID = kGlobeCircleLeftID + 1;
const DisplayElementID kGlobeCircleUpID = kGlobeCircleRightID + 1;
const DisplayElementID kGlobeCircleDownID = kGlobeCircleUpID + 1;
const DisplayElementID kMotionHiliteLeftID = kGlobeCircleDownID + 1;
const DisplayElementID kMotionHiliteRightID = kMotionHiliteLeftID + 1;
const DisplayElementID kMotionHiliteUpID = kMotionHiliteRightID + 1;
const DisplayElementID kMotionHiliteDownID = kMotionHiliteUpID + 1;
const DisplayElementID kTargetHiliteUpperLeftID = kMotionHiliteDownID + 1;
const DisplayElementID kTargetHiliteUpperRightID = kTargetHiliteUpperLeftID + 1;
const DisplayElementID kTargetHiliteLowerLeftID = kTargetHiliteUpperRightID + 1;
const DisplayElementID kTargetHiliteLowerRightID = kTargetHiliteLowerLeftID + 1;
const DisplayElementID kGlobeUpperNamesID = kTargetHiliteLowerRightID + 1;
const DisplayElementID kGlobeLowerNamesID = kGlobeUpperNamesID + 1;
const DisplayElementID kGlobeCountdownID = kGlobeLowerNamesID + 1;

//	Norad Alpha:

const DisplayOrder kECRMonitorOrder = kMonitorLayer;
const DisplayOrder kECRPanOrder = kECRMonitorOrder + 1;

const DisplayOrder kN01LeftSideOrder = kMonitorLayer;
const DisplayOrder kN01RightSideOrder = kN01LeftSideOrder + 1;

const DisplayOrder kElevatorControlsOrder = kMonitorLayer;

const DisplayOrder kPressureLevelsOrder = kMonitorLayer;
const DisplayOrder kPressureTypeOrder = kPressureLevelsOrder + 1;
const DisplayOrder kPressureUpOrder = kPressureTypeOrder + 1;
const DisplayOrder kPressureDownOrder = kPressureUpOrder + 1;

const DisplayOrder kPlatformOrder = kMonitorLayer;

const DisplayOrder kSubControlOrder = kMonitorLayer;
const DisplayOrder kClawMonitorOrder = kSubControlOrder + 1;
const DisplayOrder kSubControlPinchOrder = kClawMonitorOrder + 1;
const DisplayOrder kSubControlDownOrder = kSubControlPinchOrder + 1;
const DisplayOrder kSubControlRightOrder = kSubControlDownOrder + 1;
const DisplayOrder kSubControlLeftOrder = kSubControlRightOrder + 1;
const DisplayOrder kSubControlUpOrder = kSubControlLeftOrder + 1;
const DisplayOrder kSubControlCCWOrder = kSubControlUpOrder + 1;
const DisplayOrder kSubControlCWOrder = kSubControlCCWOrder + 1;
const DisplayOrder kClawMonitorGreenBallOrder = kSubControlCWOrder + 1;

//	Norad Delta:

const DisplayOrder kGlobeMonitorLayer = kMonitorLayer;
const DisplayOrder kGlobeMovieLayer = kGlobeMonitorLayer + 1;
const DisplayOrder kGlobeCircleLayer = kGlobeMovieLayer + 1;
const DisplayOrder kGlobeHilitesLayer = kGlobeCircleLayer + 1;
const DisplayOrder kGlobeUpperNamesLayer = kGlobeHilitesLayer + 1;
const DisplayOrder kGlobeLowerNamesLayer = kGlobeUpperNamesLayer + 1;
const DisplayOrder kGlobeCountdownLayer = kGlobeLowerNamesLayer + 1;

//	Norad Alpha Tables

const TimeScale kNoradAlphaMovieScale = 600;
const TimeScale kNoradAlphaFramesPerSecond = 15;
const TimeScale kNoradAlphaFrameDuration = 40;

//	Alternate IDs.

const AlternateID kAltNoradAlphaNormal = 0;

//	Room IDs.

const RoomID kNorad01 = 0;
const RoomID kNorad01East = 1;
const RoomID kNorad01West = 2;
const RoomID kNorad02 = 3;
const RoomID kNorad03 = 4;
const RoomID kNorad04 = 5;
const RoomID kNorad05 = 6;
const RoomID kNorad06 = 7;
const RoomID kNorad07 = 8;
const RoomID kNorad07North = 9;
const RoomID kNorad08 = 10;
const RoomID kNorad09 = 11;
const RoomID kNorad10 = 12;
const RoomID kNorad10East = 13;
const RoomID kNorad11 = 14;
const RoomID kNorad11South = 15;
const RoomID kNorad12 = 16;
const RoomID kNorad12South = 17;
const RoomID kNorad13 = 18;
const RoomID kNorad14 = 19;
const RoomID kNorad15 = 20;
const RoomID kNorad16 = 21;
const RoomID kNorad17 = 22;
const RoomID kNorad18 = 23;
const RoomID kNorad19 = 24;
const RoomID kNorad19West = 25;
const RoomID kNorad21 = 26;
const RoomID kNorad21West = 27;
const RoomID kNorad22 = 28;
const RoomID kNorad22West = 29;

//	Hot Spot Activation IDs.


//	Hot Spot IDs.

const HotSpotID kNorad01ECRSpotID = 5000;
const HotSpotID kNorad01GasSpotID = 5001;
const HotSpotID kNorad01ECROutSpotID = 5002;
const HotSpotID kNorad01GasOutSpotID = 5003;
const HotSpotID kNorad01MonitorSpotID = 5004;
const HotSpotID kNorad01IntakeSpotID = 5005;
const HotSpotID kNorad01DispenseSpotID = 5006;
const HotSpotID kNorad01ArSpotID = 5007;
const HotSpotID kNorad01CO2SpotID = 5008;
const HotSpotID kNorad01HeSpotID = 5009;
const HotSpotID kNorad01OSpotID = 5010;
const HotSpotID kNorad01NSpotID = 5011;
const HotSpotID kN01GasCanisterSpotID = 5012;
const HotSpotID kN01ArgonCanisterSpotID = 5013;
const HotSpotID kN01AirMaskSpotID = 5014;
const HotSpotID kN01NitrogenCanisterSpotID = 5015;
const HotSpotID kN01GasOutletSpotID = 5016;
const HotSpotID kNorad07DoorSpotID = 5017;
const HotSpotID kNorad07DoorOutSpotID = 5018;
const HotSpotID kNorad10DoorSpotID = 5019;
const HotSpotID kNorad10EastOutSpotID = 5020;
const HotSpotID kAlphaUpperPressureDoorUpSpotID = 5021;
const HotSpotID kAlphaUpperPressureDoorDownSpotID = 5022;
const HotSpotID kNorad11ElevatorSpotID = 5023;
const HotSpotID kNorad11ElevatorOutSpotID = 5024;
const HotSpotID kNorad11ElevatorDownSpotID = 5025;
const HotSpotID kNorad12ElevatorSpotID = 5026;
const HotSpotID kNorad12ElevatorOutSpotID = 5027;
const HotSpotID kNorad12ElevatorUpSpotID = 5028;
const HotSpotID kNorad19MonitorSpotID = 5029;
const HotSpotID kNorad19MonitorOutSpotID = 5030;
const HotSpotID kNorad19ActivateMonitorSpotID = 5031;
const HotSpotID kNorad21WestSpotID = 5032;
const HotSpotID kNorad21WestOutSpotID = 5033;
const HotSpotID kAlphaLowerPressureDoorUpSpotID = 5034;
const HotSpotID kAlphaLowerPressureDoorDownSpotID = 5035;
const HotSpotID kNorad22MonitorSpotID = 5036;
const HotSpotID kNorad22MonitorOutSpotID = 5037;
const HotSpotID kNorad22LaunchPrepSpotID = 5038;
const HotSpotID kNorad22ClawControlSpotID = 5039;
const HotSpotID kNorad22ClawPinchSpotID = 5040;
const HotSpotID kNorad22ClawDownSpotID = 5041;
const HotSpotID kNorad22ClawRightSpotID = 5042;
const HotSpotID kNorad22ClawLeftSpotID = 5043;
const HotSpotID kNorad22ClawUpSpotID = 5044;
const HotSpotID kNorad22ClawCCWSpotID = 5045;
const HotSpotID kNorad22ClawCWSpotID = 5046;

//	Extra sequence IDs.

const ExtraID kNoradArriveFromTSA = 0;
const ExtraID kNorad01RobotTaunt = 1;
const ExtraID kNorad01ZoomInWithGasCanister = 2;
const ExtraID kN01WGasCanister = 3;
const ExtraID kNorad01ZoomOutWithGasCanister = 4;
const ExtraID kN01WZEmptyLit = 5;
const ExtraID kN01WZGasCanisterDim = 6;
const ExtraID kN01WZGasCanisterLit = 7;
const ExtraID kN01WZArgonCanisterDim = 8;
const ExtraID kN01WZArgonCanisterLit = 9;
const ExtraID kN01WZAirMaskDim = 10;
const ExtraID kN01WZAirMaskLit = 11;
const ExtraID kN01WZNitrogenCanisterDim = 12;
const ExtraID kN01WZNitrogenCanisterLit = 13;
const ExtraID kNorad04EastDeath = 14;
const ExtraID kNorad19PrepSub = 15;
const ExtraID kNorad19ExitToSub = 16;
const ExtraID kNorad22SouthIntro = 17;
const ExtraID kNorad22SouthReply = 18;
const ExtraID kNorad22SouthFinish = 19;
const ExtraID kN22ClawFromAToB = 20;
const ExtraID kN22ClawALoop = 21;
const ExtraID kN22ClawAPinch = 22;
const ExtraID kN22ClawACounterclockwise = 23;
const ExtraID kN22ClawAClockwise = 24;
const ExtraID kN22ClawFromBToA = 25;
const ExtraID kN22ClawFromBToC = 26;
const ExtraID kN22ClawFromBToD = 27;
const ExtraID kN22ClawBLoop = 28;
const ExtraID kN22ClawBPinch = 29;
const ExtraID kN22ClawBCounterclockwise = 30;
const ExtraID kN22ClawBClockwise = 31;
const ExtraID kN22ClawFromCToB = 32;
const ExtraID kN22ClawCLoop = 33;
const ExtraID kN22ClawCPinch = 34;
const ExtraID kN22ClawCCounterclockwise = 35;
const ExtraID kN22ClawCClockwise = 36;
const ExtraID kN22ClawFromDToB = 37;
const ExtraID kN22ClawDLoop = 38;
const ExtraID kN22ClawDPinch = 39;
const ExtraID kN22ClawDCounterclockwise = 40;
const ExtraID kN22ClawDClockwise = 41;


//	Norad Delta Extra sequence IDs.

const ExtraID kArriveFromSubChase = 0;
const ExtraID kN59ZoomWithRobot = 1;
const ExtraID kN59RobotApproaches = 2;
const ExtraID kN59RobotPunchLoop = 3;
const ExtraID kN59PlayerWins1 = 4;
const ExtraID kN59PlayerWins2 = 5;
const ExtraID kN59RobotWins = 6;
const ExtraID kN59RobotHeadOpens = 7;
const ExtraID kN59Biochips111 = 8;
const ExtraID kN59Biochips011 = 9;
const ExtraID kN59Biochips101 = 10;
const ExtraID kN59Biochips001 = 11;
const ExtraID kN59Biochips110 = 12;
const ExtraID kN59Biochips010 = 13;
const ExtraID kN59Biochips100 = 14;
const ExtraID kN59Biochips000 = 15;
const ExtraID kN59RobotDisappears = 16;
const ExtraID kN60ClawFromAToB = 17;
const ExtraID kN60ClawALoop = 18;
const ExtraID kN60ClawAPinch = 19;
const ExtraID kN60ClawACounterclockwise = 20;
const ExtraID kN60ClawAClockwise = 21;
const ExtraID kN60ClawFromBToA = 22;
const ExtraID kN60ClawFromBToC = 23;
const ExtraID kN60ClawFromBToD = 24;
const ExtraID kN60ClawBLoop = 25;
const ExtraID kN60ClawBPinch = 26;
const ExtraID kN60ClawBCounterclockwise = 27;
const ExtraID kN60ClawBClockwise = 28;
const ExtraID kN60ClawFromCToB = 29;
const ExtraID kN60ClawCLoop = 30;
const ExtraID kN60ClawCPinch = 31;
const ExtraID kN60ClawCCounterclockwise = 32;
const ExtraID kN60ClawCClockwise = 33;
const ExtraID kN60ClawFromDToB = 34;
const ExtraID kN60ClawDLoop = 35;
const ExtraID kN60ClawDPinch = 36;
const ExtraID kN60ClawDCounterclockwise = 37;
const ExtraID kN60ClawDClockwise = 38;
const ExtraID kN60RobotApproaches = 39;
const ExtraID kN60FirstMistake = 40;
const ExtraID kN60ArmActivated = 41;
const ExtraID kN60SecondMistake = 42;
const ExtraID kN60ArmToPositionB = 43;
const ExtraID kN60ThirdMistake = 44;
const ExtraID kN60ArmGrabsRobot = 45;
const ExtraID kN60FourthMistake = 46;
const ExtraID kN60ArmCarriesRobotToPositionA = 47;
const ExtraID kN60PlayerFollowsRobotToDoor = 48;
const ExtraID kN60RobotHeadOpens = 49;
const ExtraID kN60Biochips111 = 50;
const ExtraID kN60Biochips011 = 51;
const ExtraID kN60Biochips101 = 52;
const ExtraID kN60Biochips001 = 53;
const ExtraID kN60Biochips110 = 54;
const ExtraID kN60Biochips010 = 55;
const ExtraID kN60Biochips100 = 56;
const ExtraID kN60Biochips000 = 57;
const ExtraID kN60RobotDisappears = 58;
const ExtraID kNoradDeltaRetinalScanBad = 59;
const ExtraID kNoradDeltaRetinalScanGood = 60;
const ExtraID kN79BrightView = 61;

// Norad Delta Tables

const TimeScale kNoradDeltaMovieScale = 600;
const TimeScale kNoradDeltaFramesPerSecond = 15;
const TimeScale kNoradDeltaFrameDuration = 40;

//	Alternate IDs.

const AlternateID kAltNoradDeltaNormal = 0;

//	Room IDs.

const RoomID kNorad41 = 0;
const RoomID kNorad42 = 1;
const RoomID kNorad43 = 2;
const RoomID kNorad44 = 3;
const RoomID kNorad45 = 4;
const RoomID kNorad46 = 5;
const RoomID kNorad47 = 6;
const RoomID kNorad48 = 7;
const RoomID kNorad48South = 8;
const RoomID kNorad49 = 9;
const RoomID kNorad49South = 10;
const RoomID kNorad50 = 11;
const RoomID kNorad50East = 12;
const RoomID kNorad51 = 13;
const RoomID kNorad52 = 14;
const RoomID kNorad53 = 15;
const RoomID kNorad54 = 16;
const RoomID kNorad54North = 17;
const RoomID kNorad55 = 18;
const RoomID kNorad56 = 19;
const RoomID kNorad57 = 20;
const RoomID kNorad58 = 21;
const RoomID kNorad59 = 22;
const RoomID kNorad59West = 23;
const RoomID kNorad60 = 24;
const RoomID kNorad60West = 25;
const RoomID kNorad61 = 26;
const RoomID kNorad62 = 27;
const RoomID kNorad63 = 28;
const RoomID kNorad64 = 29;
const RoomID kNorad65 = 30;
const RoomID kNorad66 = 31;
const RoomID kNorad67 = 32;
const RoomID kNorad68 = 33;
const RoomID kNorad68West = 34;
const RoomID kNorad69 = 35;
const RoomID kNorad78 = 36;
const RoomID kNorad79 = 37;
const RoomID kNorad79West = 38;

//	Hot Spot Activation IDs.


//	Hot Spot IDs.

const HotSpotID kNorad48ElevatorSpotID = 5000;
const HotSpotID kNorad48ElevatorOutSpotID = 5001;
const HotSpotID kNorad48ElevatorUpSpotID = 5002;
const HotSpotID kNorad49ElevatorSpotID = 5003;
const HotSpotID kNorad49ElevatorOutSpotID = 5004;
const HotSpotID kNorad49ElevatorDownSpotID = 5005;
const HotSpotID kNorad50DoorSpotID = 5006;
const HotSpotID kNorad50DoorOutSpotID = 5007;
const HotSpotID kDeltaUpperPressureDoorUpSpotID = 5008;
const HotSpotID kDeltaUpperPressureDoorDownSpotID = 5009;
const HotSpotID kNorad54DoorSpotID = 5010;
const HotSpotID kNorad54DoorOutSpotID = 5011;
const HotSpotID kNorad59WestSpotID = 5012;
const HotSpotID kNorad59WestOutSpotID = 5013;
const HotSpotID kDeltaLowerPressureDoorUpSpotID = 5014;
const HotSpotID kDeltaLowerPressureDoorDownSpotID = 5015;
const HotSpotID kDelta59RobotHeadSpotID = 5016;
const HotSpotID kDelta59RobotShieldBiochipSpotID = 5017;
const HotSpotID kDelta59RobotOpMemBiochipSpotID = 5018;
const HotSpotID kDelta59RobotRetinalBiochipSpotID = 5019;
const HotSpotID kNorad60MonitorSpotID = 5020;
const HotSpotID kNorad60MonitorOutSpotID = 5021;
const HotSpotID kNorad60LaunchPrepSpotID = 5022;
const HotSpotID kNorad60ClawControlSpotID = 5023;
const HotSpotID kNorad60ClawPinchSpotID = 5024;
const HotSpotID kNorad60ClawDownSpotID = 5025;
const HotSpotID kNorad60ClawRightSpotID = 5026;
const HotSpotID kNorad60ClawLeftSpotID = 5027;
const HotSpotID kNorad60ClawUpSpotID = 5028;
const HotSpotID kNorad60ClawCCWSpotID = 5029;
const HotSpotID kNorad60ClawCWSpotID = 5030;
const HotSpotID kDelta60RobotHeadSpotID = 5031;
const HotSpotID kDelta60RobotShieldBiochipSpotID = 5032;
const HotSpotID kDelta60RobotOpMemBiochipSpotID = 5033;
const HotSpotID kDelta60RobotRetinalBiochipSpotID = 5034;
const HotSpotID kNorad68WestSpotID = 5035;
const HotSpotID kNorad68WestOutSpotID = 5036;
const HotSpotID kNorad79WestSpotID = 5037;
const HotSpotID kNorad79WestOutSpotID = 5038;
const HotSpotID kNorad79SpinLeftSpotID = 5039;
const HotSpotID kNorad79SpinRightSpotID = 5040;
const HotSpotID kNorad79SpinUpSpotID = 5041;
const HotSpotID kNorad79SpinDownSpotID = 5042;
const HotSpotID kNorad79SiloAreaSpotID = 5043;

} // End of namespace Pegasus

#endif
