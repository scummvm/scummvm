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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_CONSTANTS_H
#define PEGASUS_NEIGHBORHOOD_MARS_CONSTANTS_H

#include "pegasus/constants.h"

namespace Pegasus {

//	Element Coordinates

const tCoordType kUndoHiliteLeft = kNavAreaLeft + 140;
const tCoordType kUndoHiliteTop = kNavAreaTop + 36;

const tCoordType kCurrentGuessLeft = kNavAreaLeft + 146;
const tCoordType kCurrentGuessTop = kNavAreaTop + 90;

const tCoordType kReactorChoiceHiliteLeft = kNavAreaLeft + 116;
const tCoordType kReactorChoiceHiliteTop = kNavAreaTop + 158;

const tCoordType kReactorHistoryLeft = kNavAreaLeft + 302;
const tCoordType kReactorHistoryTop = kNavAreaTop + 39;

const tCoordType kAnswerLeft = kNavAreaLeft + 304;
const tCoordType kAnswerTop = kNavAreaTop + 180;

const tCoordType kShuttle1Left = 0;
const tCoordType kShuttle1Top = 0;

const tCoordType kShuttle2Left = 0;
const tCoordType kShuttle2Top = 96;

const tCoordType kShuttle3Left = 500;
const tCoordType kShuttle3Top = 96;

const tCoordType kShuttle4Left = 0;
const tCoordType kShuttle4Top = 320;

const tCoordType kShuttleWindowLeft = 140;
const tCoordType kShuttleWindowTop = 96;
const tCoordType kShuttleWindowWidth = 360;
const tCoordType kShuttleWindowHeight = 224;

const tCoordType kShuttleWindowMidH = (kShuttleWindowLeft * 2 + kShuttleWindowWidth) / 2;
const tCoordType kShuttleWindowMidV = (kShuttleWindowTop * 2 + kShuttleWindowHeight) / 2;

const tCoordType kShuttleLeftLeft = 0;
const tCoordType kShuttleLeftTop = 128;

const tCoordType kShuttleRightLeft = 506;
const tCoordType kShuttleRightTop = 128;

const tCoordType kShuttleLowerLeftLeft = 74;
const tCoordType kShuttleLowerLeftTop = 358;

const tCoordType kShuttleLowerRightLeft = 486;
const tCoordType kShuttleLowerRightTop = 354;

const tCoordType kShuttleCenterLeft = 260;
const tCoordType kShuttleCenterTop = 336;

const tCoordType kShuttleUpperLeftLeft = 30;
const tCoordType kShuttleUpperLeftTop = 32;

const tCoordType kShuttleUpperRightLeft = 506;
const tCoordType kShuttleUpperRightTop = 52;

const tCoordType kShuttleLeftEnergyLeft = 110;
const tCoordType kShuttleLeftEnergyTop = 186;

const tCoordType kShuttleRightEnergyLeft = 510;
const tCoordType kShuttleRightEnergyTop = 186;

const tCoordType kShuttleEnergyLeft = 186;
const tCoordType kShuttleEnergyTop = 60;
const tCoordType kShuttleEnergyWidth = 252;
const tCoordType kShuttleEnergyHeight = 22;

const tCoordType kPlanetStartLeft = kShuttleWindowLeft;
const tCoordType kPlanetStartTop = kShuttleWindowTop + kShuttleWindowHeight;

const tCoordType kPlanetStopLeft = kShuttleWindowLeft;
const tCoordType kPlanetStopTop = kShuttleWindowTop + kShuttleWindowHeight - 100;

const tCoordType kShuttleTractorLeft = kShuttleWindowLeft + 6;
const tCoordType kShuttleTractorTop = kShuttleWindowTop + 56;
const tCoordType kShuttleTractorWidth = 348;
const tCoordType kShuttleTractorHeight = 112;

const tCoordType kShuttleJunkLeft = kShuttleWindowLeft + 6;
const tCoordType kShuttleJunkTop = kShuttleWindowTop + 6;

const tDisplayOrder kShuttlePlanetOrder = kInterfaceLayer;
const tDisplayOrder kShuttleAlienShipOrder = kShuttlePlanetOrder + 1;
const tDisplayOrder kShuttleRobotShipOrder = kShuttleAlienShipOrder + 1;
const tDisplayOrder kShuttleTractorBeamMovieOrder = kShuttleRobotShipOrder + 1;
const tDisplayOrder kShuttleWeaponBackOrder = kShuttleTractorBeamMovieOrder + 1;
const tDisplayOrder kShuttleJunkOrder = kShuttleWeaponBackOrder + 1;
const tDisplayOrder kShuttleWeaponFrontOrder = kShuttleJunkOrder + 1;
const tDisplayOrder kShuttleTractorBeamOrder = kShuttleWeaponFrontOrder + 1;
const tDisplayOrder kShuttleHUDOrder = kShuttleTractorBeamOrder + 1;
const tDisplayOrder kShuttleBackgroundOrder = kShuttleHUDOrder + 1;
const tDisplayOrder kShuttleMonitorOrder = kShuttleBackgroundOrder + 1;
const tDisplayOrder kShuttleStatusOrder = kShuttleMonitorOrder + 1;

const TimeValue kShuttleSwingStart = 0;
const TimeValue kShuttleSwingStop = 5 * 600;

const TimeValue kCanyonChaseStart = kShuttleSwingStop;
const TimeValue kCanyonChaseStop = 60 * 600 + 43 * 600 + 14 * 40;

const TimeValue kLaunchTubeReachedTime = 60 * 600 + 38 * 600 - kCanyonChaseStart;
const TimeValue kCanyonChaseFinishedTime = kCanyonChaseStop - kCanyonChaseStart -
											kLaunchTubeReachedTime;

//	Left shuttle.

const TimeValue kShuttleLeftIntroStart = 0;
const TimeValue kShuttleLeftIntroStop = 400;

const TimeValue kShuttleLeftBlankTime = 400;

const TimeValue kShuttleLeftNormalTime = 440;

const TimeValue kShuttleLeftAutoTestTime = 480;

const TimeValue kShuttleLeftDamagedTime = 520;

const TimeValue kShuttleLeftDampingTime = 560;

const TimeValue kShuttleLeftGravitonTime = 600;

const TimeValue kShuttleLeftTractorTime = 640;

//	Right shuttle.

const TimeValue kShuttleRightIntroStart = 0;
const TimeValue kShuttleRightIntroStop = 400;

const TimeValue kShuttleRightDestroyedStart = 400;
const TimeValue kShuttleRightDestroyedStop = 840;

const TimeValue kShuttleRightBlankTime = 840;

const TimeValue kShuttleRightNormalTime = 880;

const TimeValue kShuttleRightDamagedTime = 920;

const TimeValue kShuttleRightTargetLockTime = 960;

const TimeValue kShuttleRightGravitonTime = 1000;

const TimeValue kShuttleRightOverloadTime = 1040;

//	Lower Left shuttle.

const TimeValue kShuttleLowerLeftCollisionTime = 0;

const TimeValue kShuttleLowerLeftTubeTime = 40;

const TimeValue kShuttleLowerLeftAutopilotTime = 80;

//	Lower Right shuttle.

const TimeValue kShuttleLowerRightOffTime = 0;

const TimeValue kShuttleLowerRightTrackingTime = 40;

const TimeValue kShuttleLowerRightTransportTime = 80;

const TimeValue kShuttleLowerRightTransportHiliteTime = 120;

//	Center shuttle.

const TimeValue kShuttleCenterBoardingTime = 0;

const TimeValue kShuttleCenterCheckTime = 40;

const TimeValue kShuttleCenterNavCompTime = 80;

const TimeValue kShuttleCenterCommTime = 120;

const TimeValue kShuttleCenterWeaponsTime = 160;

const TimeValue kShuttleCenterAllSystemsTime = 200;

const TimeValue kShuttleCenterSecureLooseTime = 240;

const TimeValue kShuttleCenterAutoTestTime = 280;

const TimeValue kShuttleCenterLaunchTime = 320;

const TimeValue kShuttleCenterEnterTubeTime = 360;

const TimeValue kShuttleCenterTargetSightedTime = 400;

const TimeValue kShuttleCenterVerifyingTime = 440;

const TimeValue kShuttleCenterScanningTime = 480;

const TimeValue kShuttleCenterSafeTime = 520;

//	Upper Left shuttle.

const TimeValue kShuttleUpperLeftDimTime = 0;

const TimeValue kShuttleUpperLeftDampingTime = 40;

const TimeValue kShuttleUpperLeftGravitonTime = 80;

const TimeValue kShuttleUpperLeftTractorTime = 120;

//	Upper Right shuttle.

const TimeValue kShuttleUpperRightLockedTime = 0;

const TimeValue kShuttleUpperRightArmedTime = 40;

const TimeValue kShuttleUpperRightAlienDestroyedTime = 80;

const TimeValue kShuttleUpperRightOverloadTime = 120;

const TimeValue kShuttleUpperRightTargetDestroyedTime = 160;

//	Shuttle distance

const int kShuttleDistance = 500;

const int kJunkMaxDistance = kShuttleDistance;
const int kJunkMinDistance = 40;

const int kEnergyBeamMaxDistance = kShuttleDistance;
const int kEnergyBeamMinDistance = 40;

const int kGravitonMaxDistance = kShuttleDistance;
const int kGravitonMinDistance = 40;

const TimeValue kMarsOxyMaskOnIn = 0;
const TimeValue kMarsOxyMaskOnOut = 1560;

const TimeValue kMarsAirlockButtonBeepIn = 1560;
const TimeValue kMarsAirlockButtonBeepOut = 1620;

const TimeValue kMarsColorMatchingButtonBeepIn = 1620;
const TimeValue kMarsColorMatchingButtonBeepOut = 1680;

const TimeValue kMarsKioskBeepIn = 1680;
const TimeValue kMarsKioskBeepOut = 1740;

const TimeValue kMarsBumpIntoWallIn = 1740;
const TimeValue kMarsBumpIntoWallOut = 1888;

const TimeValue kMarsGantryDoorCloseIn = 1888;
const TimeValue kMarsGantryDoorCloseOut = 2866;

const TimeValue kMarsTransportDoorCloseIn = 2866;
const TimeValue kMarsTransportDoorCloseOut = 3593;

const TimeValue kMarsAirlockPressurizeIn = 3593;
const TimeValue kMarsAirlockPressurizeOut = 4766;

const TimeValue kMarsBigAirlockDoorCloseIn = 4766;
const TimeValue kMarsBigAirlockDoorCloseOut = 7872;

const TimeValue kMarsSmallAirlockDoorCloseIn = 7872;
const TimeValue kMarsSmallAirlockDoorCloseOut = 10000;

const TimeValue kMarsMazeDoorCloseIn = 10000;
const TimeValue kMarsMazeDoorCloseOut = 10969;

const TimeValue kMarsRobotTakesTransportIn = 10969;
const TimeValue kMarsRobotTakesTransportOut = 12802;

const TimeValue kMarsPodDepartedUpperPlatformIn = 12802;
const TimeValue kMarsPodDepartedUpperPlatformOut = 15783;

const TimeValue kMarsPodDepartedLowerPlatformIn = 15783;
const TimeValue kMarsPodDepartedLowerPlatformOut = 18736;

const TimeValue kMarsPodArrivedUpperPlatformIn = 18736;
const TimeValue kMarsPodArrivedUpperPlatformOut = 21605;

const TimeValue kMarsCheckInRequiredIn = 21605;
const TimeValue kMarsCheckInRequiredOut = 27463;

const TimeValue kMarsCantOpenShuttleIn = 27463;
const TimeValue kMarsCantOpenShuttleOut = 29214;

const TimeValue kMarsShuttleLockOverrideIn = 29214;
const TimeValue kMarsShuttleLockOverrideOut = 30330;

const TimeValue kMarsNoShuttleIn = 30330;
const TimeValue kMarsNoShuttleOut = 31502;

const TimeValue kMustBeUnlockedIn = 31502;
const TimeValue kMustBeUnlockedOut = 33960;

const TimeValue kColorMatchBlueIn = 33960;
const TimeValue kColorMatchBlueOut = 34240;

const TimeValue kColorMatchRedIn = 34240;
const TimeValue kColorMatchRedOut = 34538;

const TimeValue kColorMatchGreenIn = 34538;
const TimeValue kColorMatchGreenOut = 34827;

const TimeValue kColorMatchYellowIn = 34827;
const TimeValue kColorMatchYellowOut = 35162;

const TimeValue kColorMatchPurpleIn = 35162;
const TimeValue kColorMatchPurpleOut = 35426;

const TimeValue kColorMatchZeroNodesIn = 35426;
const TimeValue kColorMatchZeroNodesOut = 36376;

const TimeValue kColorMatchOneNodeIn = 36376;
const TimeValue kColorMatchOneNodeOut = 37209;

const TimeValue kColorMatchTwoNodesIn = 37209;
const TimeValue kColorMatchTwoNodesOut = 37983;

const TimeValue kColorMatchThreeNodesIn = 37983;
const TimeValue kColorMatchThreeNodesOut = 38784;

const TimeValue kMarsShuttle1DepartedIn = 38784;
const TimeValue kMarsShuttle1DepartedOut = 40323;

const TimeValue kMarsShuttle2DepartedIn = 40323;
const TimeValue kMarsShuttle2DepartedOut = 41824;

const TimeValue kShuttleCockpitIn = 41824;
const TimeValue kShuttleCockpitOut = 43126;

const TimeValue kShuttleOnboardIn = 43126;
const TimeValue kShuttleOnboardOut = 44284;

const TimeValue kShuttleNavigationIn = 44284;
const TimeValue kShuttleNavigationOut = 46049;

const TimeValue kShuttleCommunicationIn = 46049;
const TimeValue kShuttleCommunicationOut = 47288;

const TimeValue kShuttleAutoTestingIn = 47288;
const TimeValue kShuttleAutoTestingOut = 48179;

const TimeValue kMarsThrusterAutoTestIn = 48179;
const TimeValue kMarsThrusterAutoTestOut = 49979;

const TimeValue kShuttleAllSystemsIn = 49979;
const TimeValue kShuttleAllSystemsOut = 51065;

const TimeValue kShuttleSecureLooseIn = 51065;
const TimeValue kShuttleSecureLooseOut = 52346;

const TimeValue kShuttlePrepareForDropIn = 52346;
const TimeValue kShuttlePrepareForDropOut = 53216;

const TimeValue kShuttleAllClearIn = 53216;
const TimeValue kShuttleAllClearOut = 54031;

const TimeValue kShuttleConfiguringIn = 54031;
const TimeValue kShuttleConfiguringOut = 54994;

const TimeValue kShuttleGeneratingIn = 54994;
const TimeValue kShuttleGeneratingOut = 56033;

const TimeValue kShuttleBreakawayIn = 56033;
const TimeValue kShuttleBreakawayOut = 57346;

const TimeValue kMarsAtmosphericBreakawayIn = 57346;
const TimeValue kMarsAtmosphericBreakawayOut = 59237;

const TimeValue kMarsCockpitChatterIn = 59237;
const TimeValue kMarsCockpitChatterOut = 70344;

const TimeValue kShuttleDamperDescIn = 70344;
const TimeValue kShuttleDamperDescOut = 73262;

const TimeValue kShuttleGravitonDescIn = 73262;
const TimeValue kShuttleGravitonDescOut = 75296;

const TimeValue kShuttleTractorDescIn = 75296;
const TimeValue kShuttleTractorDescOut = 78381;

const TimeValue kShuttleTargetSightedIn = 78381;
const TimeValue kShuttleTargetSightedOut = 79074;

const TimeValue kShuttleAutopilotEngagedIn = 79074;
const TimeValue kShuttleAutopilotEngagedOut = 80414;

const TimeValue kMarsEDBBlastIn = 80414;
const TimeValue kMarsEDBBlastOut = 80705;

const TimeValue kMarsGravitonBlastIn = 80705;
const TimeValue kMarsGravitonBlastOut = 81199;

const TimeValue kMarsJunkCollisionIn = 81199;
const TimeValue kMarsJunkCollisionOut = 81961;

const TimeValue kShuttleGravitonIn = 81961;
const TimeValue kShuttleGravitonOut = 82587;

const TimeValue kShuttleDampingBeamIn = 82587;
const TimeValue kShuttleDampingBeamOut = 83331;

const TimeValue kShuttleTractorBeamIn = 83331;
const TimeValue kShuttleTractorBeamOut = 83802;

const TimeValue kShuttleHullBreachIn = 83802;
const TimeValue kShuttleHullBreachOut = 84721;

const TimeValue kShuttleWingDamageIn = 84721;
const TimeValue kShuttleWingDamageOut = 85640;

const TimeValue kShuttleHullDamageIn = 85640;
const TimeValue kShuttleHullDamageOut = 86513;

const TimeValue kShuttleEnergyTooLowIn = 86513;
const TimeValue kShuttleEnergyTooLowOut = 87578;

const TimeValue kShuttleTractorLimitedIn = 87578;
const TimeValue kShuttleTractorLimitedOut = 89164;

const TimeValue kShuttleCantHoldIn = 89164;
const TimeValue kShuttleCantHoldOut = 90945;

const TimeValue kShuttleBrokeFreeIn = 90945;
const TimeValue kShuttleBrokeFreeOut = 92322;

const TimeValue kShuttleDestroyedIn = 92322;
const TimeValue kShuttleDestroyedOut = 93189;

const TimeValue kShuttleCoordinatesIn = 93189;
const TimeValue kShuttleCoordinatesOut = 94018;

const TimeValue kShuttleScanningIn = 94018;
const TimeValue kShuttleScanningOut = 94975;

const TimeValue kShuttleSafeIn = 94975;
const TimeValue kShuttleSafeOut = 96176;

const TimeValue kShuttleOverloadedIn = 96176;
const TimeValue kShuttleOverloadedOut = 101308;

const TimeScale kMarsMovieScale = 600;
const TimeScale kMarsFramesPerSecond = 15;
const TimeScale kMarsFrameDuration = 40;

//	Alternate IDs.

const tAlternateID kAltMarsNormal = 0;
const tAlternateID kAltMarsPodAtMars34 = 1;
const tAlternateID kAltMarsTookCard = 2;
const tAlternateID kAltMars35AirlockEast = 3;
const tAlternateID kAltMars35AirlockWest = 4;
const tAlternateID kAltMarsPodAtMars45 = 5;
const tAlternateID kAltMarsTookMask = 6;
const tAlternateID kAltMarsMaskOnFiller = 7;
const tAlternateID kAltMars60AirlockEast = 8;
const tAlternateID kAltMars60AirlockWest = 9;

//	Room IDs.

const tRoomID kMars0A = 0;
const tRoomID kMars00 = 1;
const tRoomID kMars01 = 2;
const tRoomID kMars02 = 3;
const tRoomID kMars03 = 4;
const tRoomID kMars04 = 5;
const tRoomID kMars05 = 6;
const tRoomID kMars06 = 7;
const tRoomID kMars07 = 8;
const tRoomID kMars08 = 9;
const tRoomID kMars09 = 10;
const tRoomID kMars10 = 11;
const tRoomID kMars11 = 12;
const tRoomID kMars12 = 13;
const tRoomID kMars13 = 14;
const tRoomID kMars14 = 15;
const tRoomID kMars15 = 16;
const tRoomID kMars16 = 17;
const tRoomID kMars17 = 18;
const tRoomID kMars18 = 19;
const tRoomID kMars19 = 20;
const tRoomID kMars20 = 21;
const tRoomID kMars21 = 22;
const tRoomID kMars22 = 23;
const tRoomID kMars23 = 24;
const tRoomID kMars24 = 25;
const tRoomID kMars25 = 26;
const tRoomID kMars26 = 27;
const tRoomID kMars27 = 28;
const tRoomID kMars28 = 29;
const tRoomID kMars29 = 30;
const tRoomID kMars30 = 31;
const tRoomID kMars31 = 32;
const tRoomID kMars31South = 33;
const tRoomID kMars32 = 34;
const tRoomID kMars33 = 35;
const tRoomID kMars33North = 36;
const tRoomID kMars34 = 37;
const tRoomID kMars35 = 38;
const tRoomID kMars36 = 39;
const tRoomID kMars37 = 40;
const tRoomID kMars38 = 41;
const tRoomID kMars39 = 42;
const tRoomID kMars41 = 43;
const tRoomID kMars42 = 44;
const tRoomID kMars43 = 45;
const tRoomID kMars44 = 46;
const tRoomID kMars45 = 47;
const tRoomID kMars46 = 48;
const tRoomID kMars47 = 49;
const tRoomID kMars48 = 50;
const tRoomID kMars49 = 51;
const tRoomID kMars50 = 52;
const tRoomID kMars51 = 53;
const tRoomID kMars52 = 54;
const tRoomID kMars54 = 55;
const tRoomID kMars56 = 56;
const tRoomID kMars58 = 57;
const tRoomID kMars60 = 58;
const tRoomID kMarsRobotShuttle = 59;
const tRoomID kMarsMaze004 = 60;
const tRoomID kMarsMaze005 = 61;
const tRoomID kMarsMaze006 = 62;
const tRoomID kMarsMaze007 = 63;
const tRoomID kMarsMaze008 = 64;
const tRoomID kMarsMaze009 = 65;
const tRoomID kMarsMaze010 = 66;
const tRoomID kMarsMaze011 = 67;
const tRoomID kMarsMaze012 = 68;
const tRoomID kMarsMaze015 = 69;
const tRoomID kMarsMaze016 = 70;
const tRoomID kMarsMaze017 = 71;
const tRoomID kMarsMaze018 = 72;
const tRoomID kMarsMaze019 = 73;
const tRoomID kMarsMaze020 = 74;
const tRoomID kMarsMaze021 = 75;
const tRoomID kMarsMaze022 = 76;
const tRoomID kMarsMaze023 = 77;
const tRoomID kMarsMaze024 = 78;
const tRoomID kMarsMaze025 = 79;
const tRoomID kMarsMaze026 = 80;
const tRoomID kMarsMaze027 = 81;
const tRoomID kMarsMaze028 = 82;
const tRoomID kMarsMaze031 = 83;
const tRoomID kMarsMaze032 = 84;
const tRoomID kMarsMaze033 = 85;
const tRoomID kMarsMaze034 = 86;
const tRoomID kMarsMaze035 = 87;
const tRoomID kMarsMaze036 = 88;
const tRoomID kMarsMaze037 = 89;
const tRoomID kMarsMaze038 = 90;
const tRoomID kMarsMaze039 = 91;
const tRoomID kMarsMaze042 = 92;
const tRoomID kMarsMaze043 = 93;
const tRoomID kMarsMaze044 = 94;
const tRoomID kMarsMaze045 = 95;
const tRoomID kMarsMaze046 = 96;
const tRoomID kMarsMaze047 = 97;
const tRoomID kMarsMaze049 = 98;
const tRoomID kMarsMaze050 = 99;
const tRoomID kMarsMaze051 = 100;
const tRoomID kMarsMaze052 = 101;
const tRoomID kMarsMaze053 = 102;
const tRoomID kMarsMaze054 = 103;
const tRoomID kMarsMaze055 = 104;
const tRoomID kMarsMaze056 = 105;
const tRoomID kMarsMaze057 = 106;
const tRoomID kMarsMaze058 = 107;
const tRoomID kMarsMaze059 = 108;
const tRoomID kMarsMaze060 = 109;
const tRoomID kMarsMaze061 = 110;
const tRoomID kMarsMaze063 = 111;
const tRoomID kMarsMaze064 = 112;
const tRoomID kMarsMaze065 = 113;
const tRoomID kMarsMaze066 = 114;
const tRoomID kMarsMaze067 = 115;
const tRoomID kMarsMaze068 = 116;
const tRoomID kMarsMaze069 = 117;
const tRoomID kMarsMaze070 = 118;
const tRoomID kMarsMaze071 = 119;
const tRoomID kMarsMaze072 = 120;
const tRoomID kMarsMaze074 = 121;
const tRoomID kMarsMaze076 = 122;
const tRoomID kMarsMaze078 = 123;
const tRoomID kMarsMaze079 = 124;
const tRoomID kMarsMaze081 = 125;
const tRoomID kMarsMaze083 = 126;
const tRoomID kMarsMaze084 = 127;
const tRoomID kMarsMaze085 = 128;
const tRoomID kMarsMaze086 = 129;
const tRoomID kMarsMaze087 = 130;
const tRoomID kMarsMaze088 = 131;
const tRoomID kMarsMaze089 = 132;
const tRoomID kMarsMaze090 = 133;
const tRoomID kMarsMaze091 = 134;
const tRoomID kMarsMaze092 = 135;
const tRoomID kMarsMaze093 = 136;
const tRoomID kMarsMaze098 = 137;
const tRoomID kMarsMaze099 = 138;
const tRoomID kMarsMaze100 = 139;
const tRoomID kMarsMaze101 = 140;
const tRoomID kMarsMaze104 = 141;
const tRoomID kMarsMaze105 = 142;
const tRoomID kMarsMaze106 = 143;
const tRoomID kMarsMaze107 = 144;
const tRoomID kMarsMaze108 = 145;
const tRoomID kMarsMaze111 = 146;
const tRoomID kMarsMaze113 = 147;
const tRoomID kMarsMaze114 = 148;
const tRoomID kMarsMaze115 = 149;
const tRoomID kMarsMaze116 = 150;
const tRoomID kMarsMaze117 = 151;
const tRoomID kMarsMaze118 = 152;
const tRoomID kMarsMaze119 = 153;
const tRoomID kMarsMaze120 = 154;
const tRoomID kMarsMaze121 = 155;
const tRoomID kMarsMaze122 = 156;
const tRoomID kMarsMaze123 = 157;
const tRoomID kMarsMaze124 = 158;
const tRoomID kMarsMaze125 = 159;
const tRoomID kMarsMaze126 = 160;
const tRoomID kMarsMaze127 = 161;
const tRoomID kMarsMaze128 = 162;
const tRoomID kMarsMaze129 = 163;
const tRoomID kMarsMaze130 = 164;
const tRoomID kMarsMaze131 = 165;
const tRoomID kMarsMaze132 = 166;
const tRoomID kMarsMaze133 = 167;
const tRoomID kMarsMaze136 = 168;
const tRoomID kMarsMaze137 = 169;
const tRoomID kMarsMaze138 = 170;
const tRoomID kMarsMaze139 = 171;
const tRoomID kMarsMaze140 = 172;
const tRoomID kMarsMaze141 = 173;
const tRoomID kMarsMaze142 = 174;
const tRoomID kMarsMaze143 = 175;
const tRoomID kMarsMaze144 = 176;
const tRoomID kMarsMaze145 = 177;
const tRoomID kMarsMaze146 = 178;
const tRoomID kMarsMaze147 = 179;
const tRoomID kMarsMaze148 = 180;
const tRoomID kMarsMaze149 = 181;
const tRoomID kMarsMaze152 = 182;
const tRoomID kMarsMaze153 = 183;
const tRoomID kMarsMaze154 = 184;
const tRoomID kMarsMaze155 = 185;
const tRoomID kMarsMaze156 = 186;
const tRoomID kMarsMaze157 = 187;
const tRoomID kMarsMaze159 = 188;
const tRoomID kMarsMaze160 = 189;
const tRoomID kMarsMaze161 = 190;
const tRoomID kMarsMaze162 = 191;
const tRoomID kMarsMaze163 = 192;
const tRoomID kMarsMaze164 = 193;
const tRoomID kMarsMaze165 = 194;
const tRoomID kMarsMaze166 = 195;
const tRoomID kMarsMaze167 = 196;
const tRoomID kMarsMaze168 = 197;
const tRoomID kMarsMaze169 = 198;
const tRoomID kMarsMaze170 = 199;
const tRoomID kMarsMaze171 = 200;
const tRoomID kMarsMaze172 = 201;
const tRoomID kMarsMaze173 = 202;
const tRoomID kMarsMaze174 = 203;
const tRoomID kMarsMaze175 = 204;
const tRoomID kMarsMaze177 = 205;
const tRoomID kMarsMaze178 = 206;
const tRoomID kMarsMaze179 = 207;
const tRoomID kMarsMaze180 = 208;
const tRoomID kMarsMaze181 = 209;
const tRoomID kMarsMaze182 = 210;
const tRoomID kMarsMaze183 = 211;
const tRoomID kMarsMaze184 = 212;
const tRoomID kMarsMaze187 = 213;
const tRoomID kMarsMaze188 = 214;
const tRoomID kMarsMaze189 = 215;
const tRoomID kMarsMaze190 = 216;
const tRoomID kMarsMaze191 = 217;
const tRoomID kMarsMaze192 = 218;
const tRoomID kMarsMaze193 = 219;
const tRoomID kMarsMaze194 = 220;
const tRoomID kMarsMaze195 = 221;
const tRoomID kMarsMaze198 = 222;
const tRoomID kMarsMaze199 = 223;
const tRoomID kMarsMaze200 = 224;
const tRoomID kMarsDeathRoom = 225;

//	Hot Spot Activation IDs.

const tHotSpotActivationID kActivationReadyForKiosk = 1;
const tHotSpotActivationID kActivationKioskChoice = 2;
const tHotSpotActivationID kActivationTunnelMapReady = 3;
const tHotSpotActivationID kActivateMarsPodClosed = 4;
const tHotSpotActivationID kActivateMarsPodOpen = 5;
const tHotSpotActivationID kActivateReadyToPressurizeAirlock = 6;
const tHotSpotActivationID kActivateAirlockPressurized = 7;
const tHotSpotActivationID kActivateMaskOnHolder = 8;
const tHotSpotActivationID kActivateMaskOnFiller = 9;
const tHotSpotActivationID kActivateReactorPlatformOut = 10;
const tHotSpotActivationID kActivateReactorPlatformIn = 11;
const tHotSpotActivationID kActivateReactorAskLowerScreen = 12;
const tHotSpotActivationID kActivateReactorReadyForNitrogen = 13;
const tHotSpotActivationID kActivateReactorReadyForCrowBar = 14;
const tHotSpotActivationID kActivateReactorAskOperation = 15;
const tHotSpotActivationID kActivateReactorRanEvaluation = 16;
const tHotSpotActivationID kActivateReactorRanDiagnostics = 17;
const tHotSpotActivationID kActivateReactorAnalyzed = 18;
const tHotSpotActivationID kActivateReactorInstructions = 19;
const tHotSpotActivationID kActivateReactorInGame = 20;
const tHotSpotActivationID kActivateReactorBombSafe = 21;
const tHotSpotActivationID kActivateReactorBombExposed = 22;
const tHotSpotActivationID kActivationRobotHeadClosed = 23;
const tHotSpotActivationID kActivationRobotHeadOpen = 24;

//	Hot Spot IDs.

const tHotSpotID kMars11NorthKioskSpotID = 5000;
const tHotSpotID kMars11NorthKioskSightsSpotID = 5001;
const tHotSpotID kMars11NorthKioskColonySpotID = 5002;
const tHotSpotID kMars12NorthKioskSpotID = 5003;
const tHotSpotID kMars12NorthKioskSightsSpotID = 5004;
const tHotSpotID kMars12NorthKioskColonySpotID = 5005;
const tHotSpotID kMars31SouthSpotID = 5006;
const tHotSpotID kMars31SouthOutSpotID = 5007;
const tHotSpotID kMars31SouthCardSpotID = 5008;
const tHotSpotID kMars33NorthSpotID = 5009;
const tHotSpotID kMars33NorthOutSpotID = 5010;
const tHotSpotID kMars33NorthMonitorSpotID = 5011;
const tHotSpotID kMars34NorthCardDropSpotID = 5012;
const tHotSpotID kMars34SouthOpenStorageSpotID = 5013;
const tHotSpotID kMars34SouthCloseStorageSpotID = 5014;
const tHotSpotID kMars34SouthCrowbarSpotID = 5015;
const tHotSpotID kMars35EastPressurizeSpotID = 5016;
const tHotSpotID kMars35EastSpinSpotID = 5017;
const tHotSpotID kMars35WestPressurizeSpotID = 5018;
const tHotSpotID kMars35WestSpinSpotID = 5019;
const tHotSpotID kMars45NorthOpenStorageSpotID = 5020;
const tHotSpotID kMars45NorthCloseStorageSpotID = 5021;
const tHotSpotID kMars45NorthCrowbarSpotID = 5022;
const tHotSpotID kAttackRobotHotSpotID = 5023;
const tHotSpotID kMars49AirMaskSpotID = 5024;
const tHotSpotID kMars49AirMaskFilledSpotID = 5025;
const tHotSpotID kMars49AirFillingDropSpotID = 5026;
const tHotSpotID kMars52MoveLeftSpotID = 5027;
const tHotSpotID kMars52MoveRightSpotID = 5028;
const tHotSpotID kMars52ExtractSpotID = 5029;
const tHotSpotID kMars53RetractSpotID = 5030;
const tHotSpotID kMars54MoveLeftSpotID = 5031;
const tHotSpotID kMars54MoveRightSpotID = 5032;
const tHotSpotID kMars54ExtractSpotID = 5033;
const tHotSpotID kMars55RetractSpotID = 5034;
const tHotSpotID kMars56MoveLeftSpotID = 5035;
const tHotSpotID kMars56MoveRightSpotID = 5036;
const tHotSpotID kMars56ExtractSpotID = 5037;
const tHotSpotID kMars57RetractSpotID = 5038;
const tHotSpotID kMars57LowerScreenSpotID = 5039;
const tHotSpotID kMars57Retract2SpotID = 5040;
const tHotSpotID kMars57DropNitrogenSpotID = 5041;
const tHotSpotID kMars57DropCrowBarSpotID = 5042;
const tHotSpotID kMars57CantOpenPanelSpotID = 5043;
const tHotSpotID kMars57ShieldEvaluationSpotID = 5044;
const tHotSpotID kMars57MeasureOutputSpotID = 5045;
const tHotSpotID kMars57RunDiagnosticsSpotID = 5046;
const tHotSpotID kMars57BackToOperationMenuSpotID = 5047;
const tHotSpotID kMars57AnalyzeObjectSpotID = 5048;
const tHotSpotID kMars57RemoveObjectMenuSpotID = 5049;
const tHotSpotID kMars57CircuitLinkSpotID = 5050;
const tHotSpotID kMars57CancelCircuitLinkSpotID = 5051;
const tHotSpotID kMars57GameInstructionsSpotID = 5052;
const tHotSpotID kMars57UndoMoveSpotID = 5053;
const tHotSpotID kMars57RedMoveSpotID = 5054;
const tHotSpotID kMars57YellowMoveSpotID = 5055;
const tHotSpotID kMars57GreenMoveSpotID = 5056;
const tHotSpotID kMars57BlueMoveSpotID = 5057;
const tHotSpotID kMars57PurpleMoveSpotID = 5058;
const tHotSpotID kMars57LowerScreenSafelySpotID = 5059;
const tHotSpotID kMars57GrabBombSpotID = 5060;
const tHotSpotID kMars58MoveLeftSpotID = 5061;
const tHotSpotID kMars58MoveRightSpotID = 5062;
const tHotSpotID kMars58ExtractSpotID = 5063;
const tHotSpotID kMars59RetractSpotID = 5064;
const tHotSpotID kMars60EastPressurizeSpotID = 5065;
const tHotSpotID kMars60EastSpinSpotID = 5066;
const tHotSpotID kMars60WestPressurizeSpotID = 5067;
const tHotSpotID kMars60WestSpinSpotID = 5068;
const tHotSpotID kRobotShuttleOpenHeadSpotID = 5069;
const tHotSpotID kRobotShuttleMapChipSpotID = 5070;
const tHotSpotID kRobotShuttleOpticalChipSpotID = 5071;
const tHotSpotID kRobotShuttleShieldChipSpotID = 5072;

//	Extra sequence IDs.

const tExtraID kMarsArrivalFromTSA = 0;
const tExtraID kMars0AWatchShuttleDepart = 1;
const tExtraID kRobotThrowsPlayer = 2;
const tExtraID kMarsInfoKioskIntro = 3;
const tExtraID kMarsColonyInfo = 4;
const tExtraID kMarsSightsInfo = 5;
const tExtraID kRobotOnWayToShuttle = 6;
const tExtraID kMars31SouthZoomInNoCard = 7;
const tExtraID kMars31SouthViewNoCard = 8;
const tExtraID kMars31SouthZoomOutNoCard = 9;
const tExtraID kMars31SouthZoomViewNoCard = 10;
const tExtraID kMars33SlideShow1 = 11;
const tExtraID kMars33SlideShow2 = 12;
const tExtraID kMars33SlideShow3 = 13;
const tExtraID kMars33SlideShow4 = 14;
const tExtraID kMars34SpotOpenWithBar = 15;
const tExtraID kMars34SpotCloseWithBar = 16;
const tExtraID kMars34SpotOpenNoBar = 17;
const tExtraID kMars34SpotCloseNoBar = 18;
const tExtraID kMars34ViewOpenWithBar = 19;
const tExtraID kMars34ViewOpenNoBar = 20;
const tExtraID kMars34NorthPodGreeting = 21;
const tExtraID kMarsTurnOnPod = 22;
const tExtraID kMarsTakePodToMars45 = 23;
const tExtraID kMars35WestSpinAirlockToEast = 24;
const tExtraID kMars35EastSpinAirlockToWest = 25;
const tExtraID kMars45SpotOpenWithBar = 26;
const tExtraID kMars45SpotCloseWithBar = 27;
const tExtraID kMars45SpotOpenNoBar = 28;
const tExtraID kMars45SpotCloseNoBar = 29;
const tExtraID kMars45ViewOpenWithBar = 30;
const tExtraID kMars45ViewOpenNoBar = 31;
const tExtraID kMars48RobotApproaches = 32;
const tExtraID kMars48RobotKillsPlayer = 33;
const tExtraID kMars48RobotLoops = 34;
const tExtraID kMars48RobotView = 35;
const tExtraID kMars48RobotDefends = 36;
const tExtraID kMars49SouthViewMaskFilling = 37;
const tExtraID kMars52SpinLeft = 38;
const tExtraID kMars52SpinRight = 39;
const tExtraID kMars52Extend = 40;
const tExtraID kMars53Retract = 41;
const tExtraID kMars54SpinLeft = 42;
const tExtraID kMars54SpinRight = 43;
const tExtraID kMars54Extend = 44;
const tExtraID kMars55Retract = 45;
const tExtraID kMars56SpinLeft = 46;
const tExtraID kMars56SpinRight = 47;
const tExtraID kMars56ExtendWithBomb = 48;
const tExtraID kMars56ExtendNoBomb = 49;
const tExtraID kMars57RetractWithBomb = 50;
const tExtraID kMars57RetractNoBomb = 51;
const tExtraID kMars57LowerScreenClosed = 52;
const tExtraID kMars57CantOpenPanel = 53;
const tExtraID kMars57FreezeLock = 54;
const tExtraID kMars57BreakLock = 55;
const tExtraID kMars57LockFrozenView = 56;
const tExtraID kMars57ThawLock = 57;
const tExtraID kMars57OpenPanel = 58;
const tExtraID kMars57OpenPanelChoices = 59;
const tExtraID kMars57ShieldEvaluation = 60;
const tExtraID kMars57MeasureOutput = 61;
const tExtraID kMars57ShieldOkayLoop = 62;
const tExtraID kMars57RunDiagnostics = 63;
const tExtraID kMars57BombExplodes = 64;
const tExtraID kMars57BombAnalysis = 65;
const tExtraID kMars57DontLink = 66;
const tExtraID kMars57CircuitLink = 67;
const tExtraID kMars57GameLevel1 = 68;
const tExtraID kMars57GameLevel2 = 69;
const tExtraID kMars57GameLevel3 = 70;
const tExtraID kMars57BombExplodesInGame = 71;
const tExtraID kMars57GameSolved = 72;
const tExtraID kMars57ExposeBomb = 73;
const tExtraID kMars57BackToNormal = 74;
const tExtraID kMars57ViewOpenNoBomb = 75;
const tExtraID kMars58SpinLeft = 76;
const tExtraID kMars58SpinRight = 77;
const tExtraID kMars58Extend = 78;
const tExtraID kMars59Retract = 79;
const tExtraID kMars60WestSpinAirlockToEast = 80;
const tExtraID kMars60EastSpinAirlockToWest = 81;
const tExtraID kMarsRobotHeadOpen = 82;
const tExtraID kMarsRobotHeadClose = 83;
const tExtraID kMarsRobotHead000 = 84;
const tExtraID kMarsRobotHead001 = 85;
const tExtraID kMarsRobotHead010 = 86;
const tExtraID kMarsRobotHead011 = 87;
const tExtraID kMarsRobotHead100 = 88;
const tExtraID kMarsRobotHead101 = 89;
const tExtraID kMarsRobotHead110 = 90;
const tExtraID kMarsRobotHead111 = 91;
const tExtraID kMarsMaze007RobotApproach = 92;
const tExtraID kMarsMaze007RobotLoop = 93;
const tExtraID kMarsMaze007RobotDeath = 94;
const tExtraID kMarsMaze015SouthRobotApproach = 95;
const tExtraID kMarsMaze015SouthRobotLoop = 96;
const tExtraID kMarsMaze015SouthRobotDeath = 97;
const tExtraID kMarsMaze101EastRobotApproach = 98;
const tExtraID kMarsMaze101EastRobotLoop = 99;
const tExtraID kMarsMaze101EastRobotDeath = 100;
const tExtraID kMarsMaze104WestLoop = 101;
const tExtraID kMarsMaze104WestDeath = 102;
const tExtraID kMarsMaze133SouthApproach = 103;
const tExtraID kMarsMaze133SouthLoop = 104;
const tExtraID kMarsMaze133SouthDeath = 105;
const tExtraID kMarsMaze136NorthApproach = 106;
const tExtraID kMarsMaze136NorthLoop = 107;
const tExtraID kMarsMaze136NorthDeath = 108;
const tExtraID kMarsMaze184WestLoop = 109;
const tExtraID kMarsMaze184WestDeath = 110;
const tExtraID kMars200DeathInBucket = 111;

const tResIDType kReactorUndoHilitePICTID = 900;

const int16 kMars52Compass = 90;
const int16 kMars54Compass = 180;
const int16 kMars56Compass = 270;
const int16 kMars58Compass = 0;

} // End of namespace Pegasus

#endif
