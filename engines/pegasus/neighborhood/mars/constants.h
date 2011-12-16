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

const CoordType kUndoHiliteLeft = kNavAreaLeft + 140;
const CoordType kUndoHiliteTop = kNavAreaTop + 36;

const CoordType kCurrentGuessLeft = kNavAreaLeft + 146;
const CoordType kCurrentGuessTop = kNavAreaTop + 90;

const CoordType kReactorChoiceHiliteLeft = kNavAreaLeft + 116;
const CoordType kReactorChoiceHiliteTop = kNavAreaTop + 158;

const CoordType kReactorHistoryLeft = kNavAreaLeft + 302;
const CoordType kReactorHistoryTop = kNavAreaTop + 39;

const CoordType kAnswerLeft = kNavAreaLeft + 304;
const CoordType kAnswerTop = kNavAreaTop + 180;

const CoordType kShuttle1Left = 0;
const CoordType kShuttle1Top = 0;

const CoordType kShuttle2Left = 0;
const CoordType kShuttle2Top = 96;

const CoordType kShuttle3Left = 500;
const CoordType kShuttle3Top = 96;

const CoordType kShuttle4Left = 0;
const CoordType kShuttle4Top = 320;

const CoordType kShuttleWindowLeft = 140;
const CoordType kShuttleWindowTop = 96;
const CoordType kShuttleWindowWidth = 360;
const CoordType kShuttleWindowHeight = 224;

const CoordType kShuttleWindowMidH = (kShuttleWindowLeft * 2 + kShuttleWindowWidth) / 2;
const CoordType kShuttleWindowMidV = (kShuttleWindowTop * 2 + kShuttleWindowHeight) / 2;

const CoordType kShuttleLeftLeft = 0;
const CoordType kShuttleLeftTop = 128;

const CoordType kShuttleRightLeft = 506;
const CoordType kShuttleRightTop = 128;

const CoordType kShuttleLowerLeftLeft = 74;
const CoordType kShuttleLowerLeftTop = 358;

const CoordType kShuttleLowerRightLeft = 486;
const CoordType kShuttleLowerRightTop = 354;

const CoordType kShuttleCenterLeft = 260;
const CoordType kShuttleCenterTop = 336;

const CoordType kShuttleUpperLeftLeft = 30;
const CoordType kShuttleUpperLeftTop = 32;

const CoordType kShuttleUpperRightLeft = 506;
const CoordType kShuttleUpperRightTop = 52;

const CoordType kShuttleLeftEnergyLeft = 110;
const CoordType kShuttleLeftEnergyTop = 186;

const CoordType kShuttleRightEnergyLeft = 510;
const CoordType kShuttleRightEnergyTop = 186;

const CoordType kShuttleEnergyLeft = 186;
const CoordType kShuttleEnergyTop = 60;
const CoordType kShuttleEnergyWidth = 252;
const CoordType kShuttleEnergyHeight = 22;

const CoordType kPlanetStartLeft = kShuttleWindowLeft;
const CoordType kPlanetStartTop = kShuttleWindowTop + kShuttleWindowHeight;

const CoordType kPlanetStopLeft = kShuttleWindowLeft;
const CoordType kPlanetStopTop = kShuttleWindowTop + kShuttleWindowHeight - 100;

const CoordType kShuttleTractorLeft = kShuttleWindowLeft + 6;
const CoordType kShuttleTractorTop = kShuttleWindowTop + 56;
const CoordType kShuttleTractorWidth = 348;
const CoordType kShuttleTractorHeight = 112;

const CoordType kShuttleJunkLeft = kShuttleWindowLeft + 6;
const CoordType kShuttleJunkTop = kShuttleWindowTop + 6;

const DisplayOrder kShuttlePlanetOrder = kInterfaceLayer;
const DisplayOrder kShuttleAlienShipOrder = kShuttlePlanetOrder + 1;
const DisplayOrder kShuttleRobotShipOrder = kShuttleAlienShipOrder + 1;
const DisplayOrder kShuttleTractorBeamMovieOrder = kShuttleRobotShipOrder + 1;
const DisplayOrder kShuttleWeaponBackOrder = kShuttleTractorBeamMovieOrder + 1;
const DisplayOrder kShuttleJunkOrder = kShuttleWeaponBackOrder + 1;
const DisplayOrder kShuttleWeaponFrontOrder = kShuttleJunkOrder + 1;
const DisplayOrder kShuttleTractorBeamOrder = kShuttleWeaponFrontOrder + 1;
const DisplayOrder kShuttleHUDOrder = kShuttleTractorBeamOrder + 1;
const DisplayOrder kShuttleBackgroundOrder = kShuttleHUDOrder + 1;
const DisplayOrder kShuttleMonitorOrder = kShuttleBackgroundOrder + 1;
const DisplayOrder kShuttleStatusOrder = kShuttleMonitorOrder + 1;

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

const AlternateID kAltMarsNormal = 0;
const AlternateID kAltMarsPodAtMars34 = 1;
const AlternateID kAltMarsTookCard = 2;
const AlternateID kAltMars35AirlockEast = 3;
const AlternateID kAltMars35AirlockWest = 4;
const AlternateID kAltMarsPodAtMars45 = 5;
const AlternateID kAltMarsTookMask = 6;
const AlternateID kAltMarsMaskOnFiller = 7;
const AlternateID kAltMars60AirlockEast = 8;
const AlternateID kAltMars60AirlockWest = 9;

//	Room IDs.

const RoomID kMars0A = 0;
const RoomID kMars00 = 1;
const RoomID kMars01 = 2;
const RoomID kMars02 = 3;
const RoomID kMars03 = 4;
const RoomID kMars04 = 5;
const RoomID kMars05 = 6;
const RoomID kMars06 = 7;
const RoomID kMars07 = 8;
const RoomID kMars08 = 9;
const RoomID kMars09 = 10;
const RoomID kMars10 = 11;
const RoomID kMars11 = 12;
const RoomID kMars12 = 13;
const RoomID kMars13 = 14;
const RoomID kMars14 = 15;
const RoomID kMars15 = 16;
const RoomID kMars16 = 17;
const RoomID kMars17 = 18;
const RoomID kMars18 = 19;
const RoomID kMars19 = 20;
const RoomID kMars20 = 21;
const RoomID kMars21 = 22;
const RoomID kMars22 = 23;
const RoomID kMars23 = 24;
const RoomID kMars24 = 25;
const RoomID kMars25 = 26;
const RoomID kMars26 = 27;
const RoomID kMars27 = 28;
const RoomID kMars28 = 29;
const RoomID kMars29 = 30;
const RoomID kMars30 = 31;
const RoomID kMars31 = 32;
const RoomID kMars31South = 33;
const RoomID kMars32 = 34;
const RoomID kMars33 = 35;
const RoomID kMars33North = 36;
const RoomID kMars34 = 37;
const RoomID kMars35 = 38;
const RoomID kMars36 = 39;
const RoomID kMars37 = 40;
const RoomID kMars38 = 41;
const RoomID kMars39 = 42;
const RoomID kMars41 = 43;
const RoomID kMars42 = 44;
const RoomID kMars43 = 45;
const RoomID kMars44 = 46;
const RoomID kMars45 = 47;
const RoomID kMars46 = 48;
const RoomID kMars47 = 49;
const RoomID kMars48 = 50;
const RoomID kMars49 = 51;
const RoomID kMars50 = 52;
const RoomID kMars51 = 53;
const RoomID kMars52 = 54;
const RoomID kMars54 = 55;
const RoomID kMars56 = 56;
const RoomID kMars58 = 57;
const RoomID kMars60 = 58;
const RoomID kMarsRobotShuttle = 59;
const RoomID kMarsMaze004 = 60;
const RoomID kMarsMaze005 = 61;
const RoomID kMarsMaze006 = 62;
const RoomID kMarsMaze007 = 63;
const RoomID kMarsMaze008 = 64;
const RoomID kMarsMaze009 = 65;
const RoomID kMarsMaze010 = 66;
const RoomID kMarsMaze011 = 67;
const RoomID kMarsMaze012 = 68;
const RoomID kMarsMaze015 = 69;
const RoomID kMarsMaze016 = 70;
const RoomID kMarsMaze017 = 71;
const RoomID kMarsMaze018 = 72;
const RoomID kMarsMaze019 = 73;
const RoomID kMarsMaze020 = 74;
const RoomID kMarsMaze021 = 75;
const RoomID kMarsMaze022 = 76;
const RoomID kMarsMaze023 = 77;
const RoomID kMarsMaze024 = 78;
const RoomID kMarsMaze025 = 79;
const RoomID kMarsMaze026 = 80;
const RoomID kMarsMaze027 = 81;
const RoomID kMarsMaze028 = 82;
const RoomID kMarsMaze031 = 83;
const RoomID kMarsMaze032 = 84;
const RoomID kMarsMaze033 = 85;
const RoomID kMarsMaze034 = 86;
const RoomID kMarsMaze035 = 87;
const RoomID kMarsMaze036 = 88;
const RoomID kMarsMaze037 = 89;
const RoomID kMarsMaze038 = 90;
const RoomID kMarsMaze039 = 91;
const RoomID kMarsMaze042 = 92;
const RoomID kMarsMaze043 = 93;
const RoomID kMarsMaze044 = 94;
const RoomID kMarsMaze045 = 95;
const RoomID kMarsMaze046 = 96;
const RoomID kMarsMaze047 = 97;
const RoomID kMarsMaze049 = 98;
const RoomID kMarsMaze050 = 99;
const RoomID kMarsMaze051 = 100;
const RoomID kMarsMaze052 = 101;
const RoomID kMarsMaze053 = 102;
const RoomID kMarsMaze054 = 103;
const RoomID kMarsMaze055 = 104;
const RoomID kMarsMaze056 = 105;
const RoomID kMarsMaze057 = 106;
const RoomID kMarsMaze058 = 107;
const RoomID kMarsMaze059 = 108;
const RoomID kMarsMaze060 = 109;
const RoomID kMarsMaze061 = 110;
const RoomID kMarsMaze063 = 111;
const RoomID kMarsMaze064 = 112;
const RoomID kMarsMaze065 = 113;
const RoomID kMarsMaze066 = 114;
const RoomID kMarsMaze067 = 115;
const RoomID kMarsMaze068 = 116;
const RoomID kMarsMaze069 = 117;
const RoomID kMarsMaze070 = 118;
const RoomID kMarsMaze071 = 119;
const RoomID kMarsMaze072 = 120;
const RoomID kMarsMaze074 = 121;
const RoomID kMarsMaze076 = 122;
const RoomID kMarsMaze078 = 123;
const RoomID kMarsMaze079 = 124;
const RoomID kMarsMaze081 = 125;
const RoomID kMarsMaze083 = 126;
const RoomID kMarsMaze084 = 127;
const RoomID kMarsMaze085 = 128;
const RoomID kMarsMaze086 = 129;
const RoomID kMarsMaze087 = 130;
const RoomID kMarsMaze088 = 131;
const RoomID kMarsMaze089 = 132;
const RoomID kMarsMaze090 = 133;
const RoomID kMarsMaze091 = 134;
const RoomID kMarsMaze092 = 135;
const RoomID kMarsMaze093 = 136;
const RoomID kMarsMaze098 = 137;
const RoomID kMarsMaze099 = 138;
const RoomID kMarsMaze100 = 139;
const RoomID kMarsMaze101 = 140;
const RoomID kMarsMaze104 = 141;
const RoomID kMarsMaze105 = 142;
const RoomID kMarsMaze106 = 143;
const RoomID kMarsMaze107 = 144;
const RoomID kMarsMaze108 = 145;
const RoomID kMarsMaze111 = 146;
const RoomID kMarsMaze113 = 147;
const RoomID kMarsMaze114 = 148;
const RoomID kMarsMaze115 = 149;
const RoomID kMarsMaze116 = 150;
const RoomID kMarsMaze117 = 151;
const RoomID kMarsMaze118 = 152;
const RoomID kMarsMaze119 = 153;
const RoomID kMarsMaze120 = 154;
const RoomID kMarsMaze121 = 155;
const RoomID kMarsMaze122 = 156;
const RoomID kMarsMaze123 = 157;
const RoomID kMarsMaze124 = 158;
const RoomID kMarsMaze125 = 159;
const RoomID kMarsMaze126 = 160;
const RoomID kMarsMaze127 = 161;
const RoomID kMarsMaze128 = 162;
const RoomID kMarsMaze129 = 163;
const RoomID kMarsMaze130 = 164;
const RoomID kMarsMaze131 = 165;
const RoomID kMarsMaze132 = 166;
const RoomID kMarsMaze133 = 167;
const RoomID kMarsMaze136 = 168;
const RoomID kMarsMaze137 = 169;
const RoomID kMarsMaze138 = 170;
const RoomID kMarsMaze139 = 171;
const RoomID kMarsMaze140 = 172;
const RoomID kMarsMaze141 = 173;
const RoomID kMarsMaze142 = 174;
const RoomID kMarsMaze143 = 175;
const RoomID kMarsMaze144 = 176;
const RoomID kMarsMaze145 = 177;
const RoomID kMarsMaze146 = 178;
const RoomID kMarsMaze147 = 179;
const RoomID kMarsMaze148 = 180;
const RoomID kMarsMaze149 = 181;
const RoomID kMarsMaze152 = 182;
const RoomID kMarsMaze153 = 183;
const RoomID kMarsMaze154 = 184;
const RoomID kMarsMaze155 = 185;
const RoomID kMarsMaze156 = 186;
const RoomID kMarsMaze157 = 187;
const RoomID kMarsMaze159 = 188;
const RoomID kMarsMaze160 = 189;
const RoomID kMarsMaze161 = 190;
const RoomID kMarsMaze162 = 191;
const RoomID kMarsMaze163 = 192;
const RoomID kMarsMaze164 = 193;
const RoomID kMarsMaze165 = 194;
const RoomID kMarsMaze166 = 195;
const RoomID kMarsMaze167 = 196;
const RoomID kMarsMaze168 = 197;
const RoomID kMarsMaze169 = 198;
const RoomID kMarsMaze170 = 199;
const RoomID kMarsMaze171 = 200;
const RoomID kMarsMaze172 = 201;
const RoomID kMarsMaze173 = 202;
const RoomID kMarsMaze174 = 203;
const RoomID kMarsMaze175 = 204;
const RoomID kMarsMaze177 = 205;
const RoomID kMarsMaze178 = 206;
const RoomID kMarsMaze179 = 207;
const RoomID kMarsMaze180 = 208;
const RoomID kMarsMaze181 = 209;
const RoomID kMarsMaze182 = 210;
const RoomID kMarsMaze183 = 211;
const RoomID kMarsMaze184 = 212;
const RoomID kMarsMaze187 = 213;
const RoomID kMarsMaze188 = 214;
const RoomID kMarsMaze189 = 215;
const RoomID kMarsMaze190 = 216;
const RoomID kMarsMaze191 = 217;
const RoomID kMarsMaze192 = 218;
const RoomID kMarsMaze193 = 219;
const RoomID kMarsMaze194 = 220;
const RoomID kMarsMaze195 = 221;
const RoomID kMarsMaze198 = 222;
const RoomID kMarsMaze199 = 223;
const RoomID kMarsMaze200 = 224;
const RoomID kMarsDeathRoom = 225;

//	Hot Spot Activation IDs.

const HotSpotActivationID kActivationReadyForKiosk = 1;
const HotSpotActivationID kActivationKioskChoice = 2;
const HotSpotActivationID kActivationTunnelMapReady = 3;
const HotSpotActivationID kActivateMarsPodClosed = 4;
const HotSpotActivationID kActivateMarsPodOpen = 5;
const HotSpotActivationID kActivateReadyToPressurizeAirlock = 6;
const HotSpotActivationID kActivateAirlockPressurized = 7;
const HotSpotActivationID kActivateMaskOnHolder = 8;
const HotSpotActivationID kActivateMaskOnFiller = 9;
const HotSpotActivationID kActivateReactorPlatformOut = 10;
const HotSpotActivationID kActivateReactorPlatformIn = 11;
const HotSpotActivationID kActivateReactorAskLowerScreen = 12;
const HotSpotActivationID kActivateReactorReadyForNitrogen = 13;
const HotSpotActivationID kActivateReactorReadyForCrowBar = 14;
const HotSpotActivationID kActivateReactorAskOperation = 15;
const HotSpotActivationID kActivateReactorRanEvaluation = 16;
const HotSpotActivationID kActivateReactorRanDiagnostics = 17;
const HotSpotActivationID kActivateReactorAnalyzed = 18;
const HotSpotActivationID kActivateReactorInstructions = 19;
const HotSpotActivationID kActivateReactorInGame = 20;
const HotSpotActivationID kActivateReactorBombSafe = 21;
const HotSpotActivationID kActivateReactorBombExposed = 22;
const HotSpotActivationID kActivationRobotHeadClosed = 23;
const HotSpotActivationID kActivationRobotHeadOpen = 24;

//	Hot Spot IDs.

const HotSpotID kMars11NorthKioskSpotID = 5000;
const HotSpotID kMars11NorthKioskSightsSpotID = 5001;
const HotSpotID kMars11NorthKioskColonySpotID = 5002;
const HotSpotID kMars12NorthKioskSpotID = 5003;
const HotSpotID kMars12NorthKioskSightsSpotID = 5004;
const HotSpotID kMars12NorthKioskColonySpotID = 5005;
const HotSpotID kMars31SouthSpotID = 5006;
const HotSpotID kMars31SouthOutSpotID = 5007;
const HotSpotID kMars31SouthCardSpotID = 5008;
const HotSpotID kMars33NorthSpotID = 5009;
const HotSpotID kMars33NorthOutSpotID = 5010;
const HotSpotID kMars33NorthMonitorSpotID = 5011;
const HotSpotID kMars34NorthCardDropSpotID = 5012;
const HotSpotID kMars34SouthOpenStorageSpotID = 5013;
const HotSpotID kMars34SouthCloseStorageSpotID = 5014;
const HotSpotID kMars34SouthCrowbarSpotID = 5015;
const HotSpotID kMars35EastPressurizeSpotID = 5016;
const HotSpotID kMars35EastSpinSpotID = 5017;
const HotSpotID kMars35WestPressurizeSpotID = 5018;
const HotSpotID kMars35WestSpinSpotID = 5019;
const HotSpotID kMars45NorthOpenStorageSpotID = 5020;
const HotSpotID kMars45NorthCloseStorageSpotID = 5021;
const HotSpotID kMars45NorthCrowbarSpotID = 5022;
const HotSpotID kAttackRobotHotSpotID = 5023;
const HotSpotID kMars49AirMaskSpotID = 5024;
const HotSpotID kMars49AirMaskFilledSpotID = 5025;
const HotSpotID kMars49AirFillingDropSpotID = 5026;
const HotSpotID kMars52MoveLeftSpotID = 5027;
const HotSpotID kMars52MoveRightSpotID = 5028;
const HotSpotID kMars52ExtractSpotID = 5029;
const HotSpotID kMars53RetractSpotID = 5030;
const HotSpotID kMars54MoveLeftSpotID = 5031;
const HotSpotID kMars54MoveRightSpotID = 5032;
const HotSpotID kMars54ExtractSpotID = 5033;
const HotSpotID kMars55RetractSpotID = 5034;
const HotSpotID kMars56MoveLeftSpotID = 5035;
const HotSpotID kMars56MoveRightSpotID = 5036;
const HotSpotID kMars56ExtractSpotID = 5037;
const HotSpotID kMars57RetractSpotID = 5038;
const HotSpotID kMars57LowerScreenSpotID = 5039;
const HotSpotID kMars57Retract2SpotID = 5040;
const HotSpotID kMars57DropNitrogenSpotID = 5041;
const HotSpotID kMars57DropCrowBarSpotID = 5042;
const HotSpotID kMars57CantOpenPanelSpotID = 5043;
const HotSpotID kMars57ShieldEvaluationSpotID = 5044;
const HotSpotID kMars57MeasureOutputSpotID = 5045;
const HotSpotID kMars57RunDiagnosticsSpotID = 5046;
const HotSpotID kMars57BackToOperationMenuSpotID = 5047;
const HotSpotID kMars57AnalyzeObjectSpotID = 5048;
const HotSpotID kMars57RemoveObjectMenuSpotID = 5049;
const HotSpotID kMars57CircuitLinkSpotID = 5050;
const HotSpotID kMars57CancelCircuitLinkSpotID = 5051;
const HotSpotID kMars57GameInstructionsSpotID = 5052;
const HotSpotID kMars57UndoMoveSpotID = 5053;
const HotSpotID kMars57RedMoveSpotID = 5054;
const HotSpotID kMars57YellowMoveSpotID = 5055;
const HotSpotID kMars57GreenMoveSpotID = 5056;
const HotSpotID kMars57BlueMoveSpotID = 5057;
const HotSpotID kMars57PurpleMoveSpotID = 5058;
const HotSpotID kMars57LowerScreenSafelySpotID = 5059;
const HotSpotID kMars57GrabBombSpotID = 5060;
const HotSpotID kMars58MoveLeftSpotID = 5061;
const HotSpotID kMars58MoveRightSpotID = 5062;
const HotSpotID kMars58ExtractSpotID = 5063;
const HotSpotID kMars59RetractSpotID = 5064;
const HotSpotID kMars60EastPressurizeSpotID = 5065;
const HotSpotID kMars60EastSpinSpotID = 5066;
const HotSpotID kMars60WestPressurizeSpotID = 5067;
const HotSpotID kMars60WestSpinSpotID = 5068;
const HotSpotID kRobotShuttleOpenHeadSpotID = 5069;
const HotSpotID kRobotShuttleMapChipSpotID = 5070;
const HotSpotID kRobotShuttleOpticalChipSpotID = 5071;
const HotSpotID kRobotShuttleShieldChipSpotID = 5072;

//	Extra sequence IDs.

const ExtraID kMarsArrivalFromTSA = 0;
const ExtraID kMars0AWatchShuttleDepart = 1;
const ExtraID kRobotThrowsPlayer = 2;
const ExtraID kMarsInfoKioskIntro = 3;
const ExtraID kMarsColonyInfo = 4;
const ExtraID kMarsSightsInfo = 5;
const ExtraID kRobotOnWayToShuttle = 6;
const ExtraID kMars31SouthZoomInNoCard = 7;
const ExtraID kMars31SouthViewNoCard = 8;
const ExtraID kMars31SouthZoomOutNoCard = 9;
const ExtraID kMars31SouthZoomViewNoCard = 10;
const ExtraID kMars33SlideShow1 = 11;
const ExtraID kMars33SlideShow2 = 12;
const ExtraID kMars33SlideShow3 = 13;
const ExtraID kMars33SlideShow4 = 14;
const ExtraID kMars34SpotOpenWithBar = 15;
const ExtraID kMars34SpotCloseWithBar = 16;
const ExtraID kMars34SpotOpenNoBar = 17;
const ExtraID kMars34SpotCloseNoBar = 18;
const ExtraID kMars34ViewOpenWithBar = 19;
const ExtraID kMars34ViewOpenNoBar = 20;
const ExtraID kMars34NorthPodGreeting = 21;
const ExtraID kMarsTurnOnPod = 22;
const ExtraID kMarsTakePodToMars45 = 23;
const ExtraID kMars35WestSpinAirlockToEast = 24;
const ExtraID kMars35EastSpinAirlockToWest = 25;
const ExtraID kMars45SpotOpenWithBar = 26;
const ExtraID kMars45SpotCloseWithBar = 27;
const ExtraID kMars45SpotOpenNoBar = 28;
const ExtraID kMars45SpotCloseNoBar = 29;
const ExtraID kMars45ViewOpenWithBar = 30;
const ExtraID kMars45ViewOpenNoBar = 31;
const ExtraID kMars48RobotApproaches = 32;
const ExtraID kMars48RobotKillsPlayer = 33;
const ExtraID kMars48RobotLoops = 34;
const ExtraID kMars48RobotView = 35;
const ExtraID kMars48RobotDefends = 36;
const ExtraID kMars49SouthViewMaskFilling = 37;
const ExtraID kMars52SpinLeft = 38;
const ExtraID kMars52SpinRight = 39;
const ExtraID kMars52Extend = 40;
const ExtraID kMars53Retract = 41;
const ExtraID kMars54SpinLeft = 42;
const ExtraID kMars54SpinRight = 43;
const ExtraID kMars54Extend = 44;
const ExtraID kMars55Retract = 45;
const ExtraID kMars56SpinLeft = 46;
const ExtraID kMars56SpinRight = 47;
const ExtraID kMars56ExtendWithBomb = 48;
const ExtraID kMars56ExtendNoBomb = 49;
const ExtraID kMars57RetractWithBomb = 50;
const ExtraID kMars57RetractNoBomb = 51;
const ExtraID kMars57LowerScreenClosed = 52;
const ExtraID kMars57CantOpenPanel = 53;
const ExtraID kMars57FreezeLock = 54;
const ExtraID kMars57BreakLock = 55;
const ExtraID kMars57LockFrozenView = 56;
const ExtraID kMars57ThawLock = 57;
const ExtraID kMars57OpenPanel = 58;
const ExtraID kMars57OpenPanelChoices = 59;
const ExtraID kMars57ShieldEvaluation = 60;
const ExtraID kMars57MeasureOutput = 61;
const ExtraID kMars57ShieldOkayLoop = 62;
const ExtraID kMars57RunDiagnostics = 63;
const ExtraID kMars57BombExplodes = 64;
const ExtraID kMars57BombAnalysis = 65;
const ExtraID kMars57DontLink = 66;
const ExtraID kMars57CircuitLink = 67;
const ExtraID kMars57GameLevel1 = 68;
const ExtraID kMars57GameLevel2 = 69;
const ExtraID kMars57GameLevel3 = 70;
const ExtraID kMars57BombExplodesInGame = 71;
const ExtraID kMars57GameSolved = 72;
const ExtraID kMars57ExposeBomb = 73;
const ExtraID kMars57BackToNormal = 74;
const ExtraID kMars57ViewOpenNoBomb = 75;
const ExtraID kMars58SpinLeft = 76;
const ExtraID kMars58SpinRight = 77;
const ExtraID kMars58Extend = 78;
const ExtraID kMars59Retract = 79;
const ExtraID kMars60WestSpinAirlockToEast = 80;
const ExtraID kMars60EastSpinAirlockToWest = 81;
const ExtraID kMarsRobotHeadOpen = 82;
const ExtraID kMarsRobotHeadClose = 83;
const ExtraID kMarsRobotHead000 = 84;
const ExtraID kMarsRobotHead001 = 85;
const ExtraID kMarsRobotHead010 = 86;
const ExtraID kMarsRobotHead011 = 87;
const ExtraID kMarsRobotHead100 = 88;
const ExtraID kMarsRobotHead101 = 89;
const ExtraID kMarsRobotHead110 = 90;
const ExtraID kMarsRobotHead111 = 91;
const ExtraID kMarsMaze007RobotApproach = 92;
const ExtraID kMarsMaze007RobotLoop = 93;
const ExtraID kMarsMaze007RobotDeath = 94;
const ExtraID kMarsMaze015SouthRobotApproach = 95;
const ExtraID kMarsMaze015SouthRobotLoop = 96;
const ExtraID kMarsMaze015SouthRobotDeath = 97;
const ExtraID kMarsMaze101EastRobotApproach = 98;
const ExtraID kMarsMaze101EastRobotLoop = 99;
const ExtraID kMarsMaze101EastRobotDeath = 100;
const ExtraID kMarsMaze104WestLoop = 101;
const ExtraID kMarsMaze104WestDeath = 102;
const ExtraID kMarsMaze133SouthApproach = 103;
const ExtraID kMarsMaze133SouthLoop = 104;
const ExtraID kMarsMaze133SouthDeath = 105;
const ExtraID kMarsMaze136NorthApproach = 106;
const ExtraID kMarsMaze136NorthLoop = 107;
const ExtraID kMarsMaze136NorthDeath = 108;
const ExtraID kMarsMaze184WestLoop = 109;
const ExtraID kMarsMaze184WestDeath = 110;
const ExtraID kMars200DeathInBucket = 111;

const ResIDType kReactorUndoHilitePICTID = 900;

const int16 kMars52Compass = 90;
const int16 kMars54Compass = 180;
const int16 kMars56Compass = 270;
const int16 kMars58Compass = 0;

} // End of namespace Pegasus

#endif
