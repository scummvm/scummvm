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

const tCanOpenDoorReason kCantOpenBadPressure = kCantOpenLastReason + 1;

const tNotificationFlags kAirTimerExpiredFlag = kLastNeighborhoodNotificationFlag << 1;

const uint16 kNoradWarningVolume = 0x100 / 3;
const uint16 kNoradSuckWindVolume = 0x100 / 2;

const int16 kElevatorCompassAngle = -40;
const int16 kSubPlatformCompassAngle = 45;
const int16 kSubControlCompassAngle = -10;

//	Norad interactions.

const tInteractionID kNoradGlobeGameInteractionID = 0;
const tInteractionID kNoradECRMonitorInteractionID = 1;
const tInteractionID kNoradFillingStationInteractionID = 2;
const tInteractionID kNoradElevatorInteractionID = 3;
const tInteractionID kNoradPressureDoorInteractionID = 4;
const tInteractionID kNoradSubControlRoomInteractionID = 5;
const tInteractionID kNoradSubPlatformInteractionID = 6;

/////////////////////////////////////////////
//
//	Norad Alpha

const tCoordType kECRSlideShowLeft = kNavAreaLeft + 78;
const tCoordType kECRSlideShowTop = kNavAreaTop + 1;

const tCoordType kECRPanLeft = kNavAreaLeft + 78 + 5;
const tCoordType kECRPanTop = kNavAreaTop + 1 + 4;
const tCoordType kECRPanRight = kECRPanLeft + 213;
const tCoordType kECRPanBottom = kECRPanTop + 241;

const tCoordType kNoradAlphaElevatorControlsLeft = kNavAreaLeft + 332;
const tCoordType kNoradAlphaElevatorControlsTop = kNavAreaTop + 127;

const tCoordType kNoradAlpha01LeftSideLeft = kNavAreaLeft + 0;
const tCoordType kNoradAlpha01LeftSideTop = kNavAreaTop + 0;

const tCoordType kNoradAlpha01RightSideLeft = kNavAreaLeft + 240;
const tCoordType kNoradAlpha01RightSideTop = kNavAreaTop + 12;

const tCoordType kNoradUpperLevelsLeft = kNavAreaLeft + 98;
const tCoordType kNoradUpperLevelsTop = kNavAreaTop + 31;

const tCoordType kNoradUpperTypeLeft = kNoradUpperLevelsLeft + 114;
const tCoordType kNoradUpperTypeTop = kNoradUpperLevelsTop + 8;

const tCoordType kNoradUpperUpLeft = kNavAreaLeft + 361;
const tCoordType kNoradUpperUpTop = kNavAreaTop + 32;

const tCoordType kNoradUpperDownLeft = kNavAreaLeft + 367;
const tCoordType kNoradUpperDownTop = kNavAreaTop + 66;

const tCoordType kNoradLowerLevelsLeft = kNavAreaLeft + 74;
const tCoordType kNoradLowerLevelsTop = kNavAreaTop + 157;

const tCoordType kNoradLowerTypeLeft = kNoradLowerLevelsLeft + 144;
const tCoordType kNoradLowerTypeTop = kNoradLowerLevelsTop + 9;

const tCoordType kNoradLowerUpLeft = kNavAreaLeft + 380;
const tCoordType kNoradLowerUpTop = kNavAreaTop + 164;

const tCoordType kNoradLowerDownLeft = kNavAreaLeft + 388;
const tCoordType kNoradLowerDownTop = kNavAreaTop + 212;

const tCoordType kNoradPlatformLeft = kNavAreaLeft + 36;
const tCoordType kNoradPlatformTop = kNavAreaTop + 87;

const tCoordType kNoradSubControlLeft = kNavAreaLeft + 0;
const tCoordType kNoradSubControlTop = kNavAreaTop + 84;

const tCoordType kNoradSubControlPinchLeft = kNoradSubControlLeft + 106;
const tCoordType kNoradSubControlPinchTop = kNoradSubControlTop + 86;

const tCoordType kNoradSubControlDownLeft = kNoradSubControlLeft + 66;
const tCoordType kNoradSubControlDownTop = kNoradSubControlTop + 106;

const tCoordType kNoradSubControlRightLeft = kNoradSubControlLeft + 83;
const tCoordType kNoradSubControlRightTop = kNoradSubControlTop + 90;

const tCoordType kNoradSubControlLeftLeft = kNoradSubControlLeft + 56;
const tCoordType kNoradSubControlLeftTop = kNoradSubControlTop + 91;

const tCoordType kNoradSubControlUpLeft = kNoradSubControlLeft + 66;
const tCoordType kNoradSubControlUpTop = kNoradSubControlTop + 81;

const tCoordType kNoradSubControlCCWLeft = kNoradSubControlLeft + 29;
const tCoordType kNoradSubControlCCWTop = kNoradSubControlTop + 88;

const tCoordType kNoradSubControlCWLeft = kNoradSubControlLeft + 0;
const tCoordType kNoradSubControlCWTop = kNoradSubControlTop + 89;

const tCoordType kNoradClawMonitorLeft = kNavAreaLeft + 288;
const tCoordType kNoradClawMonitorTop = kNavAreaTop + 97;

const tCoordType kNoradGreenBallAtALeft = kNoradClawMonitorLeft + 179;
const tCoordType kNoradGreenBallAtATop = kNoradClawMonitorTop + 82;

const tCoordType kNoradGreenBallAtBLeft = kNoradClawMonitorLeft + 130;
const tCoordType kNoradGreenBallAtBTop = kNoradClawMonitorTop + 73;

const tCoordType kNoradGreenBallAtCLeft = kNoradClawMonitorLeft + 110;
const tCoordType kNoradGreenBallAtCTop = kNoradClawMonitorTop + 26;

const tCoordType kNoradGreenBallAtDLeft = kNoradClawMonitorLeft + 21;
const tCoordType kNoradGreenBallAtDTop = kNoradClawMonitorTop + 49;

/////////////////////////////////////////////
//
//	Norad Delta

const tCoordType kGlobeMonitorLeft = kNavAreaLeft + 360;
const tCoordType kGlobeMonitorTop = kNavAreaTop + 144;

const tCoordType kGlobeLeft = kNavAreaLeft + 172;
const tCoordType kGlobeTop = kNavAreaTop;

const tCoordType kGlobeCircleLeftLeft = kNavAreaLeft + 186;
const tCoordType kGlobeCircleLeftTop = kNavAreaTop + 41;

const tCoordType kGlobeCircleRightLeft = kNavAreaLeft + 321;
const tCoordType kGlobeCircleRightTop = kNavAreaTop + 41;

const tCoordType kGlobeCircleUpLeft = kNavAreaLeft + 220;
const tCoordType kGlobeCircleUpTop = kNavAreaTop + 7;

const tCoordType kGlobeCircleDownLeft = kNavAreaLeft + 220;
const tCoordType kGlobeCircleDownTop = kNavAreaTop + 142;

const tCoordType kGlobeUpperLeftHiliteLeft = kNavAreaLeft + 207;
const tCoordType kGlobeUpperLeftHiliteTop = kNavAreaTop + 28;

const tCoordType kGlobeUpperRightHiliteLeft = kNavAreaLeft + 307;
const tCoordType kGlobeUpperRightHiliteTop = kNavAreaTop + 28;

const tCoordType kGlobeLowerLeftHiliteLeft = kNavAreaLeft + 207;
const tCoordType kGlobeLowerLeftHiliteTop = kNavAreaTop + 128;

const tCoordType kGlobeLowerRightHiliteLeft = kNavAreaLeft + 307;
const tCoordType kGlobeLowerRightHiliteTop = kNavAreaTop + 128;

const tCoordType kGlobeLeftMotionHiliteLeft = kNavAreaLeft + 182;
const tCoordType kGlobeLeftMotionHiliteTop = kNavAreaTop + 60;

const tCoordType kGlobeRightMotionHiliteLeft = kNavAreaLeft + 331;
const tCoordType kGlobeRightMotionHiliteTop = kNavAreaTop + 60;

const tCoordType kGlobeUpMotionHiliteLeft = kNavAreaLeft + 239;
const tCoordType kGlobeUpMotionHiliteTop = kNavAreaTop + 3;

const tCoordType kGlobeDownMotionHiliteLeft = kNavAreaLeft + 239;
const tCoordType kGlobeDownMotionHiliteTop = kNavAreaTop + 152;

const tCoordType kGlobeUpperNamesLeft = kNavAreaLeft + 368;
const tCoordType kGlobeUpperNamesTop = kNavAreaTop + 188;

const tCoordType kGlobeLowerNamesLeft = kNavAreaLeft + 368;
const tCoordType kGlobeLowerNamesTop = kNavAreaTop + 212;

const tCoordType kGlobeCountdownLeft = kNavAreaLeft + 478;
const tCoordType kGlobeCountdownTop = kNavAreaTop + 164;

//	Norad Alpha display IDs.

const tDisplayElementID kECRSlideShowMovieID = kNeighborhoodDisplayID;
const tDisplayElementID kECRPanID = kECRSlideShowMovieID + 1;
const tDisplayElementID kNoradAlphaDeathMovieID = kECRPanID + 1;
const tDisplayElementID kNoradElevatorControlsID = kNoradAlphaDeathMovieID + 1;
const tDisplayElementID kN01LeftSideID = kNoradElevatorControlsID + 1;
const tDisplayElementID kN01RightSideID = kN01LeftSideID + 1;
const tDisplayElementID kPressureDoorLevelsID = kN01RightSideID + 1;
const tDisplayElementID kPressureDoorTypeID = kPressureDoorLevelsID + 1;
const tDisplayElementID kPressureDoorUpButtonID = kPressureDoorTypeID + 1;
const tDisplayElementID kPressureDoorDownButtonID = kPressureDoorUpButtonID + 1;
const tDisplayElementID kPlatformMonitorID = kPressureDoorDownButtonID + 1;
const tDisplayElementID kSubControlMonitorID = kPlatformMonitorID + 1;
const tDisplayElementID kClawMonitorID = kSubControlMonitorID + 1;
const tDisplayElementID kSubControlPinchID = kClawMonitorID + 1;
const tDisplayElementID kSubControlDownID = kSubControlPinchID + 1;
const tDisplayElementID kSubControlRightID = kSubControlDownID + 1;
const tDisplayElementID kSubControlLeftID = kSubControlRightID + 1;
const tDisplayElementID kSubControlUpID = kSubControlLeftID + 1;
const tDisplayElementID kSubControlCCWID = kSubControlUpID + 1;
const tDisplayElementID kSubControlCWID = kSubControlCCWID + 1;
const tDisplayElementID kClawMonitorGreenBallID = kSubControlCWID + 1;

//	Norad Delta display IDs.

const tDisplayElementID kGlobeMonitorID = kNeighborhoodDisplayID;
const tDisplayElementID kGlobeMovieID = kGlobeMonitorID + 14;
const tDisplayElementID kGlobeCircleLeftID = kGlobeMovieID + 1;
const tDisplayElementID kGlobeCircleRightID = kGlobeCircleLeftID + 1;
const tDisplayElementID kGlobeCircleUpID = kGlobeCircleRightID + 1;
const tDisplayElementID kGlobeCircleDownID = kGlobeCircleUpID + 1;
const tDisplayElementID kMotionHiliteLeftID = kGlobeCircleDownID + 1;
const tDisplayElementID kMotionHiliteRightID = kMotionHiliteLeftID + 1;
const tDisplayElementID kMotionHiliteUpID = kMotionHiliteRightID + 1;
const tDisplayElementID kMotionHiliteDownID = kMotionHiliteUpID + 1;
const tDisplayElementID kTargetHiliteUpperLeftID = kMotionHiliteDownID + 1;
const tDisplayElementID kTargetHiliteUpperRightID = kTargetHiliteUpperLeftID + 1;
const tDisplayElementID kTargetHiliteLowerLeftID = kTargetHiliteUpperRightID + 1;
const tDisplayElementID kTargetHiliteLowerRightID = kTargetHiliteLowerLeftID + 1;
const tDisplayElementID kGlobeUpperNamesID = kTargetHiliteLowerRightID + 1;
const tDisplayElementID kGlobeLowerNamesID = kGlobeUpperNamesID + 1;
const tDisplayElementID kGlobeCountdownID = kGlobeLowerNamesID + 1;

//	Norad Alpha:

const tDisplayOrder kECRMonitorOrder = kMonitorLayer;
const tDisplayOrder kECRPanOrder = kECRMonitorOrder + 1;

const tDisplayOrder kN01LeftSideOrder = kMonitorLayer;
const tDisplayOrder kN01RightSideOrder = kN01LeftSideOrder + 1;

const tDisplayOrder kElevatorControlsOrder = kMonitorLayer;

const tDisplayOrder kPressureLevelsOrder = kMonitorLayer;
const tDisplayOrder kPressureTypeOrder = kPressureLevelsOrder + 1;
const tDisplayOrder kPressureUpOrder = kPressureTypeOrder + 1;
const tDisplayOrder kPressureDownOrder = kPressureUpOrder + 1;

const tDisplayOrder kPlatformOrder = kMonitorLayer;

const tDisplayOrder kSubControlOrder = kMonitorLayer;
const tDisplayOrder kClawMonitorOrder = kSubControlOrder + 1;
const tDisplayOrder kSubControlPinchOrder = kClawMonitorOrder + 1;
const tDisplayOrder kSubControlDownOrder = kSubControlPinchOrder + 1;
const tDisplayOrder kSubControlRightOrder = kSubControlDownOrder + 1;
const tDisplayOrder kSubControlLeftOrder = kSubControlRightOrder + 1;
const tDisplayOrder kSubControlUpOrder = kSubControlLeftOrder + 1;
const tDisplayOrder kSubControlCCWOrder = kSubControlUpOrder + 1;
const tDisplayOrder kSubControlCWOrder = kSubControlCCWOrder + 1;
const tDisplayOrder kClawMonitorGreenBallOrder = kSubControlCWOrder + 1;

//	Norad Delta:

const tDisplayOrder kGlobeMonitorLayer = kMonitorLayer;
const tDisplayOrder kGlobeMovieLayer = kGlobeMonitorLayer + 1;
const tDisplayOrder kGlobeCircleLayer = kGlobeMovieLayer + 1;
const tDisplayOrder kGlobeHilitesLayer = kGlobeCircleLayer + 1;
const tDisplayOrder kGlobeUpperNamesLayer = kGlobeHilitesLayer + 1;
const tDisplayOrder kGlobeLowerNamesLayer = kGlobeUpperNamesLayer + 1;
const tDisplayOrder kGlobeCountdownLayer = kGlobeLowerNamesLayer + 1;

//	Norad Alpha Tables

const TimeScale kNoradAlphaMovieScale = 600;
const TimeScale kNoradAlphaFramesPerSecond = 15;
const TimeScale kNoradAlphaFrameDuration = 40;

//	Alternate IDs.

const tAlternateID kAltNoradAlphaNormal = 0;

//	Room IDs.

const tRoomID kNorad01 = 0;
const tRoomID kNorad01East = 1;
const tRoomID kNorad01West = 2;
const tRoomID kNorad02 = 3;
const tRoomID kNorad03 = 4;
const tRoomID kNorad04 = 5;
const tRoomID kNorad05 = 6;
const tRoomID kNorad06 = 7;
const tRoomID kNorad07 = 8;
const tRoomID kNorad07North = 9;
const tRoomID kNorad08 = 10;
const tRoomID kNorad09 = 11;
const tRoomID kNorad10 = 12;
const tRoomID kNorad10East = 13;
const tRoomID kNorad11 = 14;
const tRoomID kNorad11South = 15;
const tRoomID kNorad12 = 16;
const tRoomID kNorad12South = 17;
const tRoomID kNorad13 = 18;
const tRoomID kNorad14 = 19;
const tRoomID kNorad15 = 20;
const tRoomID kNorad16 = 21;
const tRoomID kNorad17 = 22;
const tRoomID kNorad18 = 23;
const tRoomID kNorad19 = 24;
const tRoomID kNorad19West = 25;
const tRoomID kNorad21 = 26;
const tRoomID kNorad21West = 27;
const tRoomID kNorad22 = 28;
const tRoomID kNorad22West = 29;

//	Hot Spot Activation IDs.


//	Hot Spot IDs.

const tHotSpotID kNorad01ECRSpotID = 5000;
const tHotSpotID kNorad01GasSpotID = 5001;
const tHotSpotID kNorad01ECROutSpotID = 5002;
const tHotSpotID kNorad01GasOutSpotID = 5003;
const tHotSpotID kNorad01MonitorSpotID = 5004;
const tHotSpotID kNorad01IntakeSpotID = 5005;
const tHotSpotID kNorad01DispenseSpotID = 5006;
const tHotSpotID kNorad01ArSpotID = 5007;
const tHotSpotID kNorad01CO2SpotID = 5008;
const tHotSpotID kNorad01HeSpotID = 5009;
const tHotSpotID kNorad01OSpotID = 5010;
const tHotSpotID kNorad01NSpotID = 5011;
const tHotSpotID kN01GasCanisterSpotID = 5012;
const tHotSpotID kN01ArgonCanisterSpotID = 5013;
const tHotSpotID kN01AirMaskSpotID = 5014;
const tHotSpotID kN01NitrogenCanisterSpotID = 5015;
const tHotSpotID kN01GasOutletSpotID = 5016;
const tHotSpotID kNorad07DoorSpotID = 5017;
const tHotSpotID kNorad07DoorOutSpotID = 5018;
const tHotSpotID kNorad10DoorSpotID = 5019;
const tHotSpotID kNorad10EastOutSpotID = 5020;
const tHotSpotID kAlphaUpperPressureDoorUpSpotID = 5021;
const tHotSpotID kAlphaUpperPressureDoorDownSpotID = 5022;
const tHotSpotID kNorad11ElevatorSpotID = 5023;
const tHotSpotID kNorad11ElevatorOutSpotID = 5024;
const tHotSpotID kNorad11ElevatorDownSpotID = 5025;
const tHotSpotID kNorad12ElevatorSpotID = 5026;
const tHotSpotID kNorad12ElevatorOutSpotID = 5027;
const tHotSpotID kNorad12ElevatorUpSpotID = 5028;
const tHotSpotID kNorad19MonitorSpotID = 5029;
const tHotSpotID kNorad19MonitorOutSpotID = 5030;
const tHotSpotID kNorad19ActivateMonitorSpotID = 5031;
const tHotSpotID kNorad21WestSpotID = 5032;
const tHotSpotID kNorad21WestOutSpotID = 5033;
const tHotSpotID kAlphaLowerPressureDoorUpSpotID = 5034;
const tHotSpotID kAlphaLowerPressureDoorDownSpotID = 5035;
const tHotSpotID kNorad22MonitorSpotID = 5036;
const tHotSpotID kNorad22MonitorOutSpotID = 5037;
const tHotSpotID kNorad22LaunchPrepSpotID = 5038;
const tHotSpotID kNorad22ClawControlSpotID = 5039;
const tHotSpotID kNorad22ClawPinchSpotID = 5040;
const tHotSpotID kNorad22ClawDownSpotID = 5041;
const tHotSpotID kNorad22ClawRightSpotID = 5042;
const tHotSpotID kNorad22ClawLeftSpotID = 5043;
const tHotSpotID kNorad22ClawUpSpotID = 5044;
const tHotSpotID kNorad22ClawCCWSpotID = 5045;
const tHotSpotID kNorad22ClawCWSpotID = 5046;

//	Extra sequence IDs.

const tExtraID kNoradArriveFromTSA = 0;
const tExtraID kNorad01RobotTaunt = 1;
const tExtraID kNorad01ZoomInWithGasCanister = 2;
const tExtraID kN01WGasCanister = 3;
const tExtraID kNorad01ZoomOutWithGasCanister = 4;
const tExtraID kN01WZEmptyLit = 5;
const tExtraID kN01WZGasCanisterDim = 6;
const tExtraID kN01WZGasCanisterLit = 7;
const tExtraID kN01WZArgonCanisterDim = 8;
const tExtraID kN01WZArgonCanisterLit = 9;
const tExtraID kN01WZAirMaskDim = 10;
const tExtraID kN01WZAirMaskLit = 11;
const tExtraID kN01WZNitrogenCanisterDim = 12;
const tExtraID kN01WZNitrogenCanisterLit = 13;
const tExtraID kNorad04EastDeath = 14;
const tExtraID kNorad19PrepSub = 15;
const tExtraID kNorad19ExitToSub = 16;
const tExtraID kNorad22SouthIntro = 17;
const tExtraID kNorad22SouthReply = 18;
const tExtraID kNorad22SouthFinish = 19;
const tExtraID kN22ClawFromAToB = 20;
const tExtraID kN22ClawALoop = 21;
const tExtraID kN22ClawAPinch = 22;
const tExtraID kN22ClawACounterclockwise = 23;
const tExtraID kN22ClawAClockwise = 24;
const tExtraID kN22ClawFromBToA = 25;
const tExtraID kN22ClawFromBToC = 26;
const tExtraID kN22ClawFromBToD = 27;
const tExtraID kN22ClawBLoop = 28;
const tExtraID kN22ClawBPinch = 29;
const tExtraID kN22ClawBCounterclockwise = 30;
const tExtraID kN22ClawBClockwise = 31;
const tExtraID kN22ClawFromCToB = 32;
const tExtraID kN22ClawCLoop = 33;
const tExtraID kN22ClawCPinch = 34;
const tExtraID kN22ClawCCounterclockwise = 35;
const tExtraID kN22ClawCClockwise = 36;
const tExtraID kN22ClawFromDToB = 37;
const tExtraID kN22ClawDLoop = 38;
const tExtraID kN22ClawDPinch = 39;
const tExtraID kN22ClawDCounterclockwise = 40;
const tExtraID kN22ClawDClockwise = 41;


//	Norad Delta Extra sequence IDs.

const tExtraID kArriveFromSubChase = 0;
const tExtraID kN59ZoomWithRobot = 1;
const tExtraID kN59RobotApproaches = 2;
const tExtraID kN59RobotPunchLoop = 3;
const tExtraID kN59PlayerWins1 = 4;
const tExtraID kN59PlayerWins2 = 5;
const tExtraID kN59RobotWins = 6;
const tExtraID kN59RobotHeadOpens = 7;
const tExtraID kN59Biochips111 = 8;
const tExtraID kN59Biochips011 = 9;
const tExtraID kN59Biochips101 = 10;
const tExtraID kN59Biochips001 = 11;
const tExtraID kN59Biochips110 = 12;
const tExtraID kN59Biochips010 = 13;
const tExtraID kN59Biochips100 = 14;
const tExtraID kN59Biochips000 = 15;
const tExtraID kN59RobotDisappears = 16;
const tExtraID kN60ClawFromAToB = 17;
const tExtraID kN60ClawALoop = 18;
const tExtraID kN60ClawAPinch = 19;
const tExtraID kN60ClawACounterclockwise = 20;
const tExtraID kN60ClawAClockwise = 21;
const tExtraID kN60ClawFromBToA = 22;
const tExtraID kN60ClawFromBToC = 23;
const tExtraID kN60ClawFromBToD = 24;
const tExtraID kN60ClawBLoop = 25;
const tExtraID kN60ClawBPinch = 26;
const tExtraID kN60ClawBCounterclockwise = 27;
const tExtraID kN60ClawBClockwise = 28;
const tExtraID kN60ClawFromCToB = 29;
const tExtraID kN60ClawCLoop = 30;
const tExtraID kN60ClawCPinch = 31;
const tExtraID kN60ClawCCounterclockwise = 32;
const tExtraID kN60ClawCClockwise = 33;
const tExtraID kN60ClawFromDToB = 34;
const tExtraID kN60ClawDLoop = 35;
const tExtraID kN60ClawDPinch = 36;
const tExtraID kN60ClawDCounterclockwise = 37;
const tExtraID kN60ClawDClockwise = 38;
const tExtraID kN60RobotApproaches = 39;
const tExtraID kN60FirstMistake = 40;
const tExtraID kN60ArmActivated = 41;
const tExtraID kN60SecondMistake = 42;
const tExtraID kN60ArmToPositionB = 43;
const tExtraID kN60ThirdMistake = 44;
const tExtraID kN60ArmGrabsRobot = 45;
const tExtraID kN60FourthMistake = 46;
const tExtraID kN60ArmCarriesRobotToPositionA = 47;
const tExtraID kN60PlayerFollowsRobotToDoor = 48;
const tExtraID kN60RobotHeadOpens = 49;
const tExtraID kN60Biochips111 = 50;
const tExtraID kN60Biochips011 = 51;
const tExtraID kN60Biochips101 = 52;
const tExtraID kN60Biochips001 = 53;
const tExtraID kN60Biochips110 = 54;
const tExtraID kN60Biochips010 = 55;
const tExtraID kN60Biochips100 = 56;
const tExtraID kN60Biochips000 = 57;
const tExtraID kN60RobotDisappears = 58;
const tExtraID kNoradDeltaRetinalScanBad = 59;
const tExtraID kNoradDeltaRetinalScanGood = 60;
const tExtraID kN79BrightView = 61;

// Norad Delta Tables

const TimeScale kNoradDeltaMovieScale = 600;
const TimeScale kNoradDeltaFramesPerSecond = 15;
const TimeScale kNoradDeltaFrameDuration = 40;

//	Alternate IDs.

const tAlternateID kAltNoradDeltaNormal = 0;

//	Room IDs.

const tRoomID kNorad41 = 0;
const tRoomID kNorad42 = 1;
const tRoomID kNorad43 = 2;
const tRoomID kNorad44 = 3;
const tRoomID kNorad45 = 4;
const tRoomID kNorad46 = 5;
const tRoomID kNorad47 = 6;
const tRoomID kNorad48 = 7;
const tRoomID kNorad48South = 8;
const tRoomID kNorad49 = 9;
const tRoomID kNorad49South = 10;
const tRoomID kNorad50 = 11;
const tRoomID kNorad50East = 12;
const tRoomID kNorad51 = 13;
const tRoomID kNorad52 = 14;
const tRoomID kNorad53 = 15;
const tRoomID kNorad54 = 16;
const tRoomID kNorad54North = 17;
const tRoomID kNorad55 = 18;
const tRoomID kNorad56 = 19;
const tRoomID kNorad57 = 20;
const tRoomID kNorad58 = 21;
const tRoomID kNorad59 = 22;
const tRoomID kNorad59West = 23;
const tRoomID kNorad60 = 24;
const tRoomID kNorad60West = 25;
const tRoomID kNorad61 = 26;
const tRoomID kNorad62 = 27;
const tRoomID kNorad63 = 28;
const tRoomID kNorad64 = 29;
const tRoomID kNorad65 = 30;
const tRoomID kNorad66 = 31;
const tRoomID kNorad67 = 32;
const tRoomID kNorad68 = 33;
const tRoomID kNorad68West = 34;
const tRoomID kNorad69 = 35;
const tRoomID kNorad78 = 36;
const tRoomID kNorad79 = 37;
const tRoomID kNorad79West = 38;

//	Hot Spot Activation IDs.


//	Hot Spot IDs.

const tHotSpotID kNorad48ElevatorSpotID = 5000;
const tHotSpotID kNorad48ElevatorOutSpotID = 5001;
const tHotSpotID kNorad48ElevatorUpSpotID = 5002;
const tHotSpotID kNorad49ElevatorSpotID = 5003;
const tHotSpotID kNorad49ElevatorOutSpotID = 5004;
const tHotSpotID kNorad49ElevatorDownSpotID = 5005;
const tHotSpotID kNorad50DoorSpotID = 5006;
const tHotSpotID kNorad50DoorOutSpotID = 5007;
const tHotSpotID kDeltaUpperPressureDoorUpSpotID = 5008;
const tHotSpotID kDeltaUpperPressureDoorDownSpotID = 5009;
const tHotSpotID kNorad54DoorSpotID = 5010;
const tHotSpotID kNorad54DoorOutSpotID = 5011;
const tHotSpotID kNorad59WestSpotID = 5012;
const tHotSpotID kNorad59WestOutSpotID = 5013;
const tHotSpotID kDeltaLowerPressureDoorUpSpotID = 5014;
const tHotSpotID kDeltaLowerPressureDoorDownSpotID = 5015;
const tHotSpotID kDelta59RobotHeadSpotID = 5016;
const tHotSpotID kDelta59RobotShieldBiochipSpotID = 5017;
const tHotSpotID kDelta59RobotOpMemBiochipSpotID = 5018;
const tHotSpotID kDelta59RobotRetinalBiochipSpotID = 5019;
const tHotSpotID kNorad60MonitorSpotID = 5020;
const tHotSpotID kNorad60MonitorOutSpotID = 5021;
const tHotSpotID kNorad60LaunchPrepSpotID = 5022;
const tHotSpotID kNorad60ClawControlSpotID = 5023;
const tHotSpotID kNorad60ClawPinchSpotID = 5024;
const tHotSpotID kNorad60ClawDownSpotID = 5025;
const tHotSpotID kNorad60ClawRightSpotID = 5026;
const tHotSpotID kNorad60ClawLeftSpotID = 5027;
const tHotSpotID kNorad60ClawUpSpotID = 5028;
const tHotSpotID kNorad60ClawCCWSpotID = 5029;
const tHotSpotID kNorad60ClawCWSpotID = 5030;
const tHotSpotID kDelta60RobotHeadSpotID = 5031;
const tHotSpotID kDelta60RobotShieldBiochipSpotID = 5032;
const tHotSpotID kDelta60RobotOpMemBiochipSpotID = 5033;
const tHotSpotID kDelta60RobotRetinalBiochipSpotID = 5034;
const tHotSpotID kNorad68WestSpotID = 5035;
const tHotSpotID kNorad68WestOutSpotID = 5036;
const tHotSpotID kNorad79WestSpotID = 5037;
const tHotSpotID kNorad79WestOutSpotID = 5038;
const tHotSpotID kNorad79SpinLeftSpotID = 5039;
const tHotSpotID kNorad79SpinRightSpotID = 5040;
const tHotSpotID kNorad79SpinUpSpotID = 5041;
const tHotSpotID kNorad79SpinDownSpotID = 5042;
const tHotSpotID kNorad79SiloAreaSpotID = 5043;

} // End of namespace Pegasus

#endif
