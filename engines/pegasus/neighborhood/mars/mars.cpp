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

#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/opticalchip.h"
#include "pegasus/items/biochips/shieldchip.h"
#include "pegasus/items/inventory/airmask.h"
#include "pegasus/neighborhood/mars/mars.h"

namespace Pegasus {

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
const tRoomID kMars32 = 34;
const tRoomID kMars33 = 35;
const tRoomID kMars33North = 36;
const tRoomID kMars36 = 39;
const tRoomID kMars37 = 40;
const tRoomID kMars38 = 41;
const tRoomID kMars41 = 43;
const tRoomID kMars42 = 44;
const tRoomID kMars43 = 45;
const tRoomID kMars44 = 46;
const tRoomID kMars45 = 47;
const tRoomID kMars46 = 48;
const tRoomID kMars47 = 49;
const tRoomID kMars48 = 50;
const tRoomID kMars50 = 52;
const tRoomID kMars51 = 53;
const tRoomID kMars52 = 54;
const tRoomID kMars54 = 55;
const tRoomID kMars56 = 56;
const tRoomID kMars58 = 57;
const tRoomID kMarsRobotShuttle = 59;
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

const tCoordType kUndoHiliteLeft = kNavAreaLeft + 140;
const tCoordType kUndoHiliteTop = kNavAreaTop + 36;

const int16 kMars52Compass = 90;
const int16 kMars54Compass = 180;
const int16 kMars56Compass = 270;
const int16 kMars58Compass = 0;

// This should really be 22.5.
// Probably no one will know the difference.
const int16 kMarsShieldPanelOffsetAngle = 22;

const tCanMoveForwardReason kCantMoveRobotBlocking = kCantMoveLastReason + 1;

const tNotificationFlags kTimeForCanyonChaseFlag = kLastNeighborhoodNotificationFlag << 1;
const tNotificationFlags kExplosionFinishedFlag = kTimeForCanyonChaseFlag << 1;
const tNotificationFlags kTimeToTransportFlag = kExplosionFinishedFlag << 1;

const tNotificationFlags kMarsNotificationFlags = kTimeForCanyonChaseFlag |
													kExplosionFinishedFlag |
													kTimeToTransportFlag;

const TimeValue kLittleExplosionStart = 0 * 40;
const TimeValue kLittleExplosionStop = 24 * 40;

const TimeValue kBigExplosionStart = 24 * 40;
const TimeValue kBigExplosionStop = 62 * 40;

enum {
	kMaze007RobotLoopingEvent,
	kMaze015RobotLoopingEvent,
	kMaze101RobotLoopingEvent,
	kMaze104RobotLoopingEvent,
	kMaze133RobotLoopingEvent,
	kMaze136RobotLoopingEvent,
	kMaze184RobotLoopingEvent
};

enum {
	kMaze007RobotLoopingTime = (64 + 96) * kMarsFrameDuration,
	kMaze015RobotLoopingTime = (64 + 93) * kMarsFrameDuration,
	kMaze101RobotLoopingTime = (64 + 45) * kMarsFrameDuration,
	kMaze104RobotLoopingTime = 96 * kMarsFrameDuration,
	kMaze133RobotLoopingTime = (64 + 96) * kMarsFrameDuration,
	kMaze136RobotLoopingTime = (64 + 96) * kMarsFrameDuration,
	kMaze184RobotLoopingTime = 96 * kMarsFrameDuration
};

void robotTimerExpiredFunction(FunctionPtr *, void *mars) {
	((Mars *)mars)->robotTiredOfWaiting();
}

void lockThawTimerExpiredFunction(FunctionPtr *, void *mars) {
	((Mars *)mars)->lockThawed();
}

void bombTimerExpiredFunction(FunctionPtr *, void *mars) {
	((Mars *)mars)->didntFindBomb();
}

void bombTimerExpiredInGameFunction(FunctionPtr *, void *mars) {
	((Mars *)mars)->bombExplodesInGame();
}

void airStageExpiredFunction(FunctionPtr *, void *mars) {
	((Mars *)mars)->airStageExpired();
}

Mars::Mars(InputHandler *nextHandler, PegasusEngine *owner) : Neighborhood(nextHandler, owner, "Mars", kMarsID),
		_guessObject(kNoDisplayElement), _undoPict(kNoDisplayElement), _guessHistory(kNoDisplayElement),
		_choiceHighlight(kNoDisplayElement), _shuttleInterface1(kNoDisplayElement), _shuttleInterface2(kNoDisplayElement),
		_shuttleInterface3(kNoDisplayElement), _shuttleInterface4(kNoDisplayElement), _canyonChaseMovie(kNoDisplayElement) {
	_noAirFuse.setFunctionPtr(&airStageExpiredFunction, this);
	setIsItemTaken(kMarsCard);
	setIsItemTaken(kAirMask);
	setIsItemTaken(kCrowbar);
	setIsItemTaken(kCardBomb);
}

Mars::~Mars() {
}

void Mars::init() {	
	Neighborhood::init();
	
	Hotspot *attackSpot = g_allHotspots.findHotspotByID(kAttackRobotHotSpotID);
	attackSpot->setMaskedHotspotFlags(kDropItemSpotFlag, kDropItemSpotFlag);
	_attackingItem = NULL;
	
	forceStridingStop(kMars08, kNorth, kAltMarsNormal);

	_neighborhoodNotification.notifyMe(this, kMarsNotificationFlags, kMarsNotificationFlags);
}

void Mars::flushGameState() {
	g_energyMonitor->saveCurrentEnergyValue();
}

void Mars::start() {
	g_energyMonitor->stopEnergyDraining();
	g_energyMonitor->restoreLastEnergyValue();
	_vm->resetEnergyDeathReason();
	g_energyMonitor->startEnergyDraining();
	Neighborhood::start();
}

class AirMaskCondition : public AICondition {
public:
	AirMaskCondition(const uint32);
	
	virtual bool fireCondition();

protected:
	uint32 _airThreshold;
	uint32 _lastAirLevel;
};

AirMaskCondition::AirMaskCondition(const uint32 airThreshold) {
	_airThreshold = airThreshold;
	_lastAirLevel = g_airMask->getAirLeft();
}

bool AirMaskCondition::fireCondition() {
	bool result = g_airMask && g_airMask->isAirMaskOn() &&
			g_airMask->getAirLeft() <= _airThreshold && _lastAirLevel > _airThreshold;

	_lastAirLevel = g_airMask->getAirLeft();
	return result;
}

void Mars::setUpAIRules() {	
	Neighborhood::setUpAIRules();

	// Don't add these rules if we're going to the robot's shuttle...
	if (g_AIArea && !GameState.getMarsReadyForShuttleTransport()) {
		AIPlayMessageAction *messageAction = new AIPlayMessageAction("Images/AI/Globals/XGLOB1E", false);
		AILocationCondition *locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars47, kSouth));
		AIRule *rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Mars/XM27NB", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars27, kNorth));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Mars/XM27NB", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars28, kNorth));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Mars/XM41ED", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars19, kEast));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		AIDeactivateRuleAction *deactivate = new AIDeactivateRuleAction(rule);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars35, kWest));
		rule = new AIRule(locCondition, deactivate);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Mars/XM41ED", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kMars48, kWest));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		AirMaskCondition *airMask50Condition = new AirMaskCondition(50);
		messageAction = new AIPlayMessageAction("Images/AI/Mars/XMMAZB1", false);
		AIRule *rule50 = new AIRule(airMask50Condition, messageAction);

		AirMaskCondition *airMask25Condition = new AirMaskCondition(25);
		AICompoundAction *compound = new AICompoundAction();
		messageAction = new AIPlayMessageAction("Images/AI/Mars/XMMAZB2", false);
		compound->addAction(messageAction);
		deactivate = new AIDeactivateRuleAction(rule50);
		compound->addAction(deactivate);
		AIRule *rule25 = new AIRule(airMask25Condition, compound);

		AirMaskCondition *airMask5Condition = new AirMaskCondition(5);
		compound = new AICompoundAction;
		messageAction = new AIPlayMessageAction("Images/AI/Mars/XMMAZB3", false);
		compound->addAction(messageAction);
		deactivate = new AIDeactivateRuleAction(rule50);
		compound->addAction(deactivate);
		deactivate = new AIDeactivateRuleAction(rule25);
		compound->addAction(deactivate);
		AIRule *rule5 = new AIRule(airMask5Condition, compound);

		g_AIArea->addAIRule(rule5);
		g_AIArea->addAIRule(rule25);
		g_AIArea->addAIRule(rule50);

		messageAction = new AIPlayMessageAction("Images/AI/Mars/XM51ND", false);
		AIDoorOpenedCondition *doorOpen = new AIDoorOpenedCondition(MakeRoomView(kMars51, kEast));
		rule = new AIRule(doorOpen, messageAction);
		g_AIArea->addAIRule(rule);
	}
}

uint16 Mars::getDateResID() const {
	return kDate2185ID;
}

TimeValue Mars::getViewTime(const tRoomID room, const tDirectionConstant direction) {
	ExtraTable::Entry extra;
	SpotTable::Entry spotEntry;
	uint32 extraID = 0xffffffff;

	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kMars0A, kNorth):
		if (!GameState.getMarsSeenTimeStream()) {
			getExtraEntry(kMarsArrivalFromTSA, extra);
			return extra.movieStart;
		}
		break;
	case MakeRoomView(kMars31South, kSouth):
		if (GameState.isTakenItemID(kMarsCard))
			extraID = kMars31SouthZoomViewNoCard;
		break;
	case MakeRoomView(kMars31, kSouth):
		if (GameState.isTakenItemID(kMarsCard))
			extraID = kMars31SouthViewNoCard;
		break;
	case MakeRoomView(kMars34, kSouth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			if (GameState.isTakenItemID(kCrowbar))
				extraID = kMars34ViewOpenNoBar;
			else
				extraID = kMars34ViewOpenWithBar;
		}
		break;
	case MakeRoomView(kMars36, kSouth):
	case MakeRoomView(kMars37, kSouth):
	case MakeRoomView(kMars38, kSouth):
		findSpotEntry(room, direction, kSpotOnTurnMask | kSpotLoopsMask, spotEntry);
		return spotEntry.movieStart;
	case MakeRoomView(kMars45, kNorth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			if (GameState.isTakenItemID(kCrowbar))
				extraID = kMars45ViewOpenNoBar;
			else
				extraID = kMars45ViewOpenWithBar;
		}
		break;
	case MakeRoomView(kMars48, kEast):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot())
			extraID = kMars48RobotView;
		break;
	case MakeRoomView(kMars56, kEast):
		if (_privateFlags.getFlag(kMarsPrivateBombExposedFlag)) {
			if (_privateFlags.getFlag(kMarsPrivateDraggingBombFlag))
				extraID = kMars57ViewOpenNoBomb;
			else
				extraID = kMars57ExposeBomb;
		} else if (GameState.getMarsLockBroken()) {
			extraID = kMars57OpenPanelChoices;
		} else if (GameState.getMarsLockFrozen()) {
			extraID = kMars57LockFrozenView;
		}
		break;
	case MakeRoomView(kMarsRobotShuttle, kEast):
		if (getCurrentActivation() == kActivationRobotHeadOpen) {
			extraID = kMarsRobotHead111;

			if (_privateFlags.getFlag(kMarsPrivateGotMapChipFlag))
				extraID -= 1;
			if (_privateFlags.getFlag(kMarsPrivateGotOpticalChipFlag))
				extraID -= 2;
			if (_privateFlags.getFlag(kMarsPrivateGotShieldChipFlag))
				extraID -= 4;
		}
		break;
	}

	if (extraID == 0xffffffff)
		return Neighborhood::getViewTime(room, direction);

	getExtraEntry(extraID, extra);
	return extra.movieEnd - 1;
}

void Mars::getZoomEntry(const tHotSpotID spotID, ZoomTable::Entry &entry) {
	Neighborhood::getZoomEntry(spotID, entry);

	uint32 extraID = 0xffffffff;

	switch (spotID) {
	case kMars31SouthSpotID:
		if (GameState.getCurrentDirection() == kSouth && GameState.isTakenItemID(kMarsCard))
			extraID = kMars31SouthZoomInNoCard;
		break;
	case kMars31SouthOutSpotID:
		if (GameState.getCurrentDirection() == kSouth && GameState.isTakenItemID(kMarsCard))
			extraID = kMars31SouthZoomOutNoCard;
		break;
	}

	if (extraID != 0xffffffff) {
		ExtraTable::Entry extra;
		getExtraEntry(extraID, extra);
		entry.movieStart = extra.movieStart;
		entry.movieEnd = extra.movieEnd;
	}
}

void Mars::findSpotEntry(const tRoomID room, const tDirectionConstant direction, tSpotFlags flags, SpotTable::Entry &entry) {
	Neighborhood::findSpotEntry(room, direction, flags, entry);

	if ((flags & (kSpotOnArrivalMask | kSpotOnTurnMask)) != 0) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kMars27, kNorth):
			if (GameState.getMarsSeenThermalScan())
				entry.clear();
			else
				GameState.setMarsSeenThermalScan(true);
			break;
		case MakeRoomView(kMars28, kNorth):
			if (GameState.getMarsSeenThermalScan())
				entry.clear();
			else
				GameState.setMarsSeenThermalScan(true);
			break;
		}
	}
}

tCanMoveForwardReason Mars::canMoveForward(ExitTable::Entry &entry) {
	tCanMoveForwardReason reason = Neighborhood::canMoveForward(entry);

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars48, kEast):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot())
			reason = kCantMoveRobotBlocking;
		break;
	case MakeRoomView(kMars48, kSouth):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot())
			_utilityFuse.stopFuse();
		break;
	}

	return reason;
}

void Mars::cantMoveThatWay(tCanMoveForwardReason reason) {
	if (reason == kCantMoveRobotBlocking) {
		startExtraSequence(kMars48RobotKillsPlayer, kExtraCompletedFlag, kFilterNoInput);
		loadLoopSound2("");
	} else {
		Neighborhood::cantMoveThatWay(reason);
	}
}

void Mars::moveForward() {
	if (GameState.getCurrentRoom() == kMars02 || (GameState.getCurrentRoom() >= kMars05 && GameState.getCurrentRoom() <= kMars08))
		loadLoopSound2("");

	Neighborhood::moveForward();
}

void Mars::bumpIntoWall() {
	requestSpotSound(kMarsBumpIntoWallIn, kMarsBumpIntoWallOut, kFilterNoInput, 0);
	Neighborhood::bumpIntoWall();
}

tCanOpenDoorReason Mars::canOpenDoor(DoorTable::Entry &entry) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars05, kEast):
	case MakeRoomView(kMars06, kEast):
	case MakeRoomView(kMars07, kEast):
		if (!GameState.getMarsSecurityDown())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze037, kWest):
	case MakeRoomView(kMarsMaze038, kEast):
		if (GameState.getMarsMazeDoorPair1())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze050, kNorth):
	case MakeRoomView(kMarsMaze058, kSouth):
		if (!GameState.getMarsMazeDoorPair1())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze047, kNorth):
	case MakeRoomView(kMarsMaze142, kSouth):
		if (GameState.getMarsMazeDoorPair2())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze057, kNorth):
	case MakeRoomView(kMarsMaze136, kSouth):
		if (!GameState.getMarsMazeDoorPair2())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze120, kWest):
	case MakeRoomView(kMarsMaze121, kEast):
		if (GameState.getMarsMazeDoorPair3())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kMarsMaze081, kNorth):
	case MakeRoomView(kMarsMaze083, kSouth):
		if (!GameState.getMarsMazeDoorPair3())
			return kCantOpenLocked;
		break;
	}

	return Neighborhood::canOpenDoor(entry);
}

void Mars::cantOpenDoor(tCanOpenDoorReason reason) {
	switch (GameState.getCurrentRoom()) {
	case kMars05:
	case kMars06:
	case kMars07:
		playSpotSoundSync(kMarsCantOpenShuttleIn, kMarsCantOpenShuttleOut);
		break;
	default:
		Neighborhood::cantOpenDoor(reason);
		break;
	}
}

void Mars::openDoor() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars06, kEast):
	case MakeRoomView(kMars07, kEast):
		if (GameState.getMarsSecurityDown())
			playSpotSoundSync(kMarsNoShuttleIn, kMarsNoShuttleOut);
		break;
	case MakeRoomView(kMars47, kSouth):
		if (GameState.isTakenItemID(kAirMask))
			setCurrentAlternate(kAltMarsTookMask);
		else
			setCurrentAlternate(kAltMarsNormal);
		break;
	case MakeRoomView(kMars48, kNorth):
		if (GameState.getMarsPodAtUpperPlatform())
			setCurrentAlternate(kAltMarsNormal);
		else
			setCurrentAlternate(kAltMarsPodAtMars45);
		break;
	case MakeRoomView(kMars48, kEast):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot()) {
			die(kDeathDidntGetOutOfWay);
			return;
		}
		break;
	}

	Neighborhood::openDoor();
}

void Mars::doorOpened() {
	switch (GameState.getCurrentRoom()) {
	case kMars27:
	case kMars28:
		if (GameState.getCurrentDirection() == kNorth)
			_vm->die(kDeathArrestedInMars);
		else
			Neighborhood::doorOpened();
		break;
	case kMars41:
	case kMars42:
		if (GameState.getCurrentDirection() == kEast)
			_vm->die(kDeathWrongShuttleLock);
		else
			Neighborhood::doorOpened();
		break;
	case kMars51:
		Neighborhood::doorOpened();
		setUpReactorEnergyDrain();

		if (g_AIArea)
			g_AIArea->checkRules();
		break;
	case kMars19:
		if (GameState.getCurrentDirection() == kEast)
			GameState.setMarsAirlockOpen(true);

		Neighborhood::doorOpened();
		break;
	case kMars48:
		if (GameState.getCurrentDirection() == kWest)
			GameState.setMarsAirlockOpen(true);

		Neighborhood::doorOpened();
		break;
	default:
		Neighborhood::doorOpened();
		break;
	}
}

void Mars::setUpReactorEnergyDrain() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars51, kEast):
		if (GameState.isCurrentDoorOpen()) {
			if (g_energyMonitor->getEnergyDrainRate() == kEnergyDrainNormal) {
				if (GameState.getShieldOn()) {
					g_shield->setItemState(kShieldRadiation);
					g_energyMonitor->setEnergyDrainRate(kMarsReactorEnergyDrainWithShield);
				} else {
					g_energyMonitor->setEnergyDrainRate(kMarsReactorEnergyDrainNoShield);
				}
				_vm->setEnergyDeathReason(kDeathReactorBurn);
			}
		} else {
			if (g_energyMonitor->getEnergyDrainRate() != kEnergyDrainNormal) {
				if (GameState.getShieldOn())
					g_shield->setItemState(kShieldNormal);
				g_energyMonitor->setEnergyDrainRate(kEnergyDrainNormal);
				_vm->resetEnergyDeathReason();
			}
		}
		break;
	case MakeRoomView(kMars52, kNorth):
	case MakeRoomView(kMars52, kSouth):
	case MakeRoomView(kMars52, kEast):
	case MakeRoomView(kMars52, kWest):
	case MakeRoomView(kMars54, kNorth):
	case MakeRoomView(kMars54, kSouth):
	case MakeRoomView(kMars54, kEast):
	case MakeRoomView(kMars54, kWest):
	case MakeRoomView(kMars56, kNorth):
	case MakeRoomView(kMars56, kSouth):
	case MakeRoomView(kMars56, kEast):
	case MakeRoomView(kMars56, kWest):
	case MakeRoomView(kMars58, kNorth):
	case MakeRoomView(kMars58, kSouth):
	case MakeRoomView(kMars58, kEast):
	case MakeRoomView(kMars58, kWest):
		if (g_energyMonitor->getEnergyDrainRate() == kEnergyDrainNormal) {
			if (GameState.getShieldOn()) {
				g_shield->setItemState(kShieldRadiation);
				g_energyMonitor->setEnergyDrainRate(kMarsReactorEnergyDrainWithShield);
			} else {
				g_energyMonitor->setEnergyDrainRate(kMarsReactorEnergyDrainNoShield);
			}
			_vm->setEnergyDeathReason(kDeathReactorBurn);
		}
		break;
	default:
		if (g_energyMonitor->getEnergyDrainRate() != kEnergyDrainNormal) {
			if (GameState.getShieldOn())
				g_shield->setItemState(kShieldNormal);
			g_energyMonitor->setEnergyDrainRate(kEnergyDrainNormal);
			_vm->resetEnergyDeathReason();
		}
		break;
	}
}

void Mars::closeDoorOffScreen(const tRoomID room, const tDirectionConstant direction) {
	switch (room) {
	case kMars51:
		playSpotSoundSync(kMarsGantryDoorCloseIn, kMarsGantryDoorCloseOut);
		if (GameState.getShieldOn())
			g_shield->setItemState(kShieldNormal);
		g_energyMonitor->setEnergyDrainRate(kEnergyDrainNormal);
		_vm->resetEnergyDeathReason();
		break;
	case kMars05:
	case kMars06:
	case kMars07:
	case kMars13:
	case kMars22:
	case kMars47:
	case kMars52:
		playSpotSoundSync(kMarsGantryDoorCloseIn, kMarsGantryDoorCloseOut);
		break;
	case kMars18:
	case kMars32:
		playSpotSoundSync(kMarsTransportDoorCloseIn, kMarsTransportDoorCloseOut);
		break;
	case kMars19:
		if (GameState.getCurrentRoom() != kMars35) {
			playSpotSoundSync(kMarsBigAirlockDoorCloseIn, kMarsBigAirlockDoorCloseOut);
			GameState.setMarsAirlockOpen(false);
		}
		break;
	case kMars36:
		if (GameState.getCurrentRoom() != kMars35)
			playSpotSoundSync(kMarsSmallAirlockDoorCloseIn, kMarsSmallAirlockDoorCloseOut);
		break;
	case kMars48:
		if (direction == kWest) {
			if (GameState.getCurrentRoom() != kMars60) {
				playSpotSoundSync(kMarsSmallAirlockDoorCloseIn, kMarsSmallAirlockDoorCloseOut);
				GameState.setMarsAirlockOpen(false);
			}
		} else {
			playSpotSoundSync(kMarsGantryDoorCloseIn, kMarsGantryDoorCloseOut);
		}
		break;
	case kMars41:
	case kMars42:
	case kMars43:
		if (direction == kWest)
			playSpotSoundSync(kMarsGantryDoorCloseIn, kMarsGantryDoorCloseOut);
		break;
	case kMarsMaze037:
	case kMarsMaze038:
	case kMarsMaze012:
	case kMarsMaze066:
	case kMarsMaze050:
	case kMarsMaze058:
	case kMarsMaze057:
	case kMarsMaze136:
	case kMarsMaze047:
	case kMarsMaze142:
	case kMarsMaze133:
	case kMarsMaze132:
	case kMarsMaze113:
	case kMarsMaze114:
	case kMarsMaze120:
	case kMarsMaze121:
	case kMarsMaze081:
	case kMarsMaze083:
	case kMarsMaze088:
	case kMarsMaze089:
	case kMarsMaze179:
	case kMarsMaze180:
		playSpotSoundSync(kMarsMazeDoorCloseIn, kMarsMazeDoorCloseOut);
		break;
	}
}

void Mars::checkAirlockDoors() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars19, kWest):
	case MakeRoomView(kMars18, kWest):
	case MakeRoomView(kMars17, kWest):
	case MakeRoomView(kMars16, kWest):
	case MakeRoomView(kMars15, kWest):
	case MakeRoomView(kMars14, kWest):
	case MakeRoomView(kMars12, kWest):
	case MakeRoomView(kMars11, kWest):
	case MakeRoomView(kMars10, kWest):
		if (GameState.getMarsInAirlock()) {
			playSpotSoundSync(kMarsBigAirlockDoorCloseIn, kMarsBigAirlockDoorCloseOut);
			GameState.setMarsInAirlock(false);
		}
		break;
	case MakeRoomView(kMars36, kEast):
	case MakeRoomView(kMars37, kEast):
	case MakeRoomView(kMars38, kEast):
	case MakeRoomView(kMars39, kEast):
	case MakeRoomView(kMars48, kEast):
	case MakeRoomView(kMars50, kEast):
	case MakeRoomView(kMars51, kEast):
	case MakeRoomView(kMars52, kEast):
		if (GameState.getMarsInAirlock()) {
			playSpotSoundSync(kMarsSmallAirlockDoorCloseIn, kMarsSmallAirlockDoorCloseOut);
			GameState.setMarsInAirlock(false);
		}
		break;
	case MakeRoomView(kMars35, kWest):
	case MakeRoomView(kMars35, kEast):
	case MakeRoomView(kMars60, kWest):
	case MakeRoomView(kMars60, kEast):
		GameState.setMarsInAirlock(true);
		break;
	default:
		GameState.setMarsInAirlock(false);
		break;
	}
}

int16 Mars::getStaticCompassAngle(const tRoomID room, const tDirectionConstant dir) {
	int16 angle = Neighborhood::getStaticCompassAngle(room, dir);

	switch (MakeRoomView(room, dir)) {
	case MakeRoomView(kMars0A, kNorth):
		angle -= 20;
		break;
	case MakeRoomView(kMars23, kNorth):
	case MakeRoomView(kMars23, kSouth):
	case MakeRoomView(kMars23, kEast):
	case MakeRoomView(kMars23, kWest):
	case MakeRoomView(kMars26, kNorth):
	case MakeRoomView(kMars26, kSouth):
	case MakeRoomView(kMars26, kEast):
	case MakeRoomView(kMars26, kWest):
		angle += 30;
		break;
	case MakeRoomView(kMars24, kNorth):
	case MakeRoomView(kMars24, kSouth):
	case MakeRoomView(kMars24, kEast):
	case MakeRoomView(kMars24, kWest):
	case MakeRoomView(kMars25, kNorth):
	case MakeRoomView(kMars25, kSouth):
	case MakeRoomView(kMars25, kEast):
	case MakeRoomView(kMars25, kWest):
		angle -= 30;
		break;
	case MakeRoomView(kMars54, kNorth):
	case MakeRoomView(kMars54, kSouth):
	case MakeRoomView(kMars54, kEast):
	case MakeRoomView(kMars54, kWest):
		angle += 90;
		break;
	case MakeRoomView(kMars56, kNorth):
	case MakeRoomView(kMars56, kSouth):
	case MakeRoomView(kMars56, kEast):
	case MakeRoomView(kMars56, kWest):
		angle += 180;
		break;
	case MakeRoomView(kMars58, kNorth):
	case MakeRoomView(kMars58, kSouth):
	case MakeRoomView(kMars58, kEast):
	case MakeRoomView(kMars58, kWest):
		angle -= 90;
		break;
	}

	return angle;
}

void Mars::getExitCompassMove(const ExitTable::Entry &exitEntry, FaderMoveSpec &compassMove) {
	Neighborhood::getExitCompassMove(exitEntry, compassMove);

	if (exitEntry.room == kMars43 && exitEntry.direction == kEast) {
		compassMove.insertFaderKnot(exitEntry.movieStart + 16 * kMarsFrameDuration, 90);
		compassMove.insertFaderKnot(exitEntry.movieStart + 32 * kMarsFrameDuration, 270);
	} else if (exitEntry.room == kMars46 && exitEntry.direction == kWest && exitEntry.altCode != kAltMarsPodAtMars45) {
		compassMove.makeTwoKnotFaderSpec(kMarsMovieScale, exitEntry.movieStart, 270, exitEntry.movieEnd, 360);
		compassMove.insertFaderKnot(exitEntry.movieStart + 43 * kMarsFrameDuration, 270);
		compassMove.insertFaderKnot(exitEntry.movieStart + 58 * kMarsFrameDuration, 360);
	}
}

void Mars::getExtraCompassMove(const ExtraTable::Entry &entry, FaderMoveSpec &compassMove) {
	switch (entry.extra) {
	case kMarsTakePodToMars45:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, 0, entry.movieEnd, 180);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 3), 30);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 11), 10);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 14), 40);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 16), 30);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 23), 100);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 31), 70);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 34), 100);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 37), 85);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 42), 135);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 44), 125);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 46), 145);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 49), 160);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * (kMarsFramesPerSecond * 51), 180);
		break;
	case kMars35WestSpinAirlockToEast:
	case kMars60WestSpinAirlockToEast:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, 90, entry.movieEnd, 270);
		compassMove.insertFaderKnot(entry.movieStart + kMarsMovieScale, 90);
		compassMove.insertFaderKnot(entry.movieStart + kMarsMovieScale * 3, 270);
		break;
	case kMars35EastSpinAirlockToWest:
	case kMars60EastSpinAirlockToWest:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, 270, entry.movieEnd, 90);
		compassMove.insertFaderKnot(entry.movieStart + kMarsMovieScale, 270);
		compassMove.insertFaderKnot(entry.movieStart + kMarsMovieScale * 3, 90);
		break;
	case kMars52SpinLeft:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars52Compass, entry.movieEnd, kMars54Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars54Compass);
		break;
	case kMars52SpinRight:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars52Compass, entry.movieEnd, kMars58Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars58Compass);
		break;
	case kMars52Extend:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars52Compass,
				entry.movieEnd, kMars52Compass + kMarsShieldPanelOffsetAngle);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 60, kMars52Compass + kMarsShieldPanelOffsetAngle);
		break;
	case kMars53Retract:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart,
				kMars52Compass + kMarsShieldPanelOffsetAngle, entry.movieEnd, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars52Compass + kMarsShieldPanelOffsetAngle);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 60, kMars52Compass);
		break;
	case kMars56ExtendWithBomb:
	case kMars56ExtendNoBomb:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars56Compass,
				entry.movieEnd, kMars56Compass - kMarsShieldPanelOffsetAngle);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 60, kMars56Compass - kMarsShieldPanelOffsetAngle);
		break;
	case kMars57RetractWithBomb:
	case kMars57RetractNoBomb:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart,
				kMars56Compass - kMarsShieldPanelOffsetAngle, entry.movieEnd, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars56Compass - kMarsShieldPanelOffsetAngle);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 60, kMars56Compass);
		break;
	case kMars54SpinLeft:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars54Compass, entry.movieEnd, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars54Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars56Compass);
		break;
	case kMars54SpinRight:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars54Compass, entry.movieEnd, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars54Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars52Compass);
		break;
	case kMars56SpinLeft:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars56Compass,
				entry.movieEnd, kMars58Compass + 360);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars58Compass + 360);
		break;
	case kMars56SpinRight:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars56Compass, entry.movieEnd, kMars54Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars54Compass);
		break;
	case kMars58SpinLeft:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart, kMars58Compass,
				entry.movieEnd, kMars52Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars58Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars52Compass);
		break;
	case kMars58SpinRight:
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), entry.movieStart,
				kMars58Compass + 360, entry.movieEnd, kMars56Compass);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 10, kMars58Compass + 360);
		compassMove.insertFaderKnot(entry.movieStart + kMarsFrameDuration * 110, kMars56Compass);
		break;
	default:
		Neighborhood::getExtraCompassMove(entry, compassMove);
	}
}

void Mars::loadAmbientLoops() {
	tRoomID room = GameState.getCurrentRoom();

	if ((room >= kMars0A && room <= kMars21) || (room >= kMars41 && room <= kMars43)) {
		if (GameState.getMarsSeenTimeStream())
			loadLoopSound1("Sounds/Mars/Gantry Ambient.22K.8.AIFF");
	} else if (room >= kMars22 && room <= kMars31South) {
		loadLoopSound1("Sounds/Mars/Reception.02.22K.8.AIFF", 0x100 / 4);
	} else if (room >= kMars32 && room <= kMars34) {
		loadLoopSound1("Sounds/Mars/Pod Room Ambient.22K.8.AIFF");
	} else if (room == kMars35) {
		if (getAirQuality(room) == kAirQualityVacuum)
			loadLoopSound1("Sounds/Mars/Gear Room Ambient.22K.8.AIFF");
		else
			loadLoopSound1("Sounds/Mars/Gantry Ambient.22K.8.AIFF", 0x100 / 2);
	} else if (room >= kMars36 && room <= kMars39) {
		loadLoopSound1("Sounds/Mars/Gear Room Ambient.22K.8.AIFF");
	} else if (room >= kMars45 && room <= kMars51) {
		loadLoopSound1("Sounds/Mars/Lower Mars Ambient.22K.8.AIFF");
	} else if (room >= kMars52 && room <= kMars58) {
		loadLoopSound1("Sounds/Mars/ReactorLoop.22K.8.AIFF");
	} else if (room == kMars60) {
		if (getAirQuality(room) == kAirQualityVacuum)
			loadLoopSound1("Sounds/Mars/Mars Maze Ambient.22K.8.AIFF");
		else
			loadLoopSound1("Sounds/Mars/Lower Mars Ambient.22K.8.AIFF", 0x100 / 2);
	} else if (room >= kMarsMaze004 && room <= kMarsMaze200) {
		loadLoopSound1("Sounds/Mars/Mars Maze Ambient.22K.8.AIFF");
	} else if (room == kMarsRobotShuttle) {
		loadLoopSound1("Sounds/Mars/Robot Shuttle.22K.8.AIFF");
	}
	
	if (!_noAirFuse.isFuseLit()) {
		switch (room) {
		case kMars02:
		case kMars05:
		case kMars06:
		case kMars07:
		case kMars08:
			loadLoopSound2("Sounds/Mars/Gantry Loop.aiff", 0x100, 0, 0);
			break;
		// Robot at maze 48
		case kMarsMaze037:
			if (GameState.isCurrentDoorOpen())
				loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			else
				loadLoopSound2("");
			break;
		case kMarsMaze038:
		case kMarsMaze039:
		case kMarsMaze049:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100);
			break;
		case kMarsMaze050:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			break;
		case kMarsMaze051:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		case kMarsMaze052:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 4);
			break;
		case kMarsMaze042:
		case kMarsMaze053:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 8);
			break;
		case kMarsMaze058:
			if (GameState.isCurrentDoorOpen())
				loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 4);
			else
				loadLoopSound2("");
			break;
		// Robot at 151
		case kMarsMaze148:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100);
			break;
		case kMarsMaze147:
		case kMarsMaze149:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			break;
		case kMarsMaze146:
		case kMarsMaze152:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		case kMarsMaze145:
		case kMarsMaze153:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 4);
			break;
		// Robots at 80 and 82.
		case kMarsMaze079:
		case kMarsMaze081:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100);
			break;
		case kMarsMaze078:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			break;
		case kMarsMaze083:
			if (GameState.isCurrentDoorOpen())
				loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			else
				loadLoopSound2("");
			break;
		case kMarsMaze118:
		case kMarsMaze076:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		case kMarsMaze074:
		case kMarsMaze117:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 4);
			break;
		// Robot at 94
		case kMarsMaze093:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100);
			break;
		case kMarsMaze091:
		case kMarsMaze092:
		case kMarsMaze098:
		case kMarsMaze101:
		case kMarsMaze100:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			break;
		case kMarsMaze090:
		case kMarsMaze099:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		case kMarsMaze089:
			if (GameState.isCurrentDoorOpen())
				loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		case kMarsMaze178:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 4);
			break;
		// Robot at 197
		case kMarsMaze191:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100);
			break;
		case kMarsMaze190:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 * 3 / 4);
			break;
		case kMarsMaze198:
		case kMarsMaze189:
			loadLoopSound2("Sounds/Mars/Maze Sparks.22K.AIFF", 0x100 / 2);
			break;
		default:
			loadLoopSound2("");
			break;
		}
	}
}

void Mars::checkContinuePoint(const tRoomID room, const tDirectionConstant direction) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kMars02, kSouth):
	case MakeRoomView(kMars19, kEast):
	case MakeRoomView(kMars22, kNorth):
	case MakeRoomView(kMars43, kEast):
	case MakeRoomView(kMars51, kEast):
	case MakeRoomView(kMars56, kEast):
	case MakeRoomView(kMars60, kWest):
	case MakeRoomView(kMarsMaze004, kWest):
	case MakeRoomView(kMarsMaze009, kWest):
	case MakeRoomView(kMarsMaze012, kWest):
	case MakeRoomView(kMarsMaze037, kWest):
	case MakeRoomView(kMarsMaze047, kNorth):
	case MakeRoomView(kMarsMaze052, kWest):
	case MakeRoomView(kMarsMaze057, kNorth):
	case MakeRoomView(kMarsMaze071, kWest):
	case MakeRoomView(kMarsMaze081, kNorth):
	case MakeRoomView(kMarsMaze088, kWest):
	case MakeRoomView(kMarsMaze093, kWest):
	case MakeRoomView(kMarsMaze115, kNorth):
	case MakeRoomView(kMarsMaze120, kWest):
	case MakeRoomView(kMarsMaze126, kEast):
	case MakeRoomView(kMarsMaze133, kNorth):
	case MakeRoomView(kMarsMaze144, kNorth):
	case MakeRoomView(kMarsMaze156, kEast):
	case MakeRoomView(kMarsMaze162, kNorth):
	case MakeRoomView(kMarsMaze177, kWest):
	case MakeRoomView(kMarsMaze180, kNorth):
	case MakeRoomView(kMarsMaze187, kWest):
	case MakeRoomView(kMarsMaze199, kWest):
		makeContinuePoint();
		break;
	case MakeRoomView(kMars05, kEast):
	case MakeRoomView(kMars06, kEast):
	case MakeRoomView(kMars07, kEast):
		if (GameState.getMarsSecurityDown())
			makeContinuePoint();
		break;
	case MakeRoomView(kMars46, kSouth):
		if (!GameState.getMarsSeenRobotAtReactor())
			makeContinuePoint();
		break;
	case MakeRoomView(kMars46, kWest):
		if (GameState.getMarsAvoidedReactorRobot())
			makeContinuePoint();
		break;
	}
}

void Mars::launchMaze007Robot() {
	startExtraLongSequence(kMarsMaze007RobotApproach, kMarsMaze007RobotDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze007RobotLoopingTime, kMarsMovieScale, kMaze007RobotLoopingEvent);
}

void Mars::launchMaze015Robot() {
	startExtraLongSequence(kMarsMaze015SouthRobotApproach, kMarsMaze015SouthRobotDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze015RobotLoopingTime, kMarsMovieScale, kMaze015RobotLoopingEvent);
}

void Mars::launchMaze101Robot() {
	startExtraLongSequence(kMarsMaze101EastRobotApproach, kMarsMaze101EastRobotDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze101RobotLoopingTime, kMarsMovieScale, kMaze101RobotLoopingEvent);
}

void Mars::launchMaze104Robot() {
	startExtraLongSequence(kMarsMaze104WestLoop, kMarsMaze104WestDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze104RobotLoopingTime, kMarsMovieScale, kMaze104RobotLoopingEvent);
}

void Mars::launchMaze133Robot() {
	startExtraLongSequence(kMarsMaze133SouthApproach, kMarsMaze133SouthDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze133RobotLoopingTime, kMarsMovieScale, kMaze133RobotLoopingEvent);
}

void Mars::launchMaze136Robot() {
	startExtraLongSequence(kMarsMaze136NorthApproach, kMarsMaze136NorthDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze136RobotLoopingTime, kMarsMovieScale, kMaze136RobotLoopingEvent);
}

void Mars::launchMaze184Robot() {
	startExtraLongSequence(kMarsMaze184WestLoop, kMarsMaze184WestDeath, kExtraCompletedFlag, kFilterAllInput);
	scheduleEvent(kMaze184RobotLoopingTime, kMarsMovieScale, kMaze184RobotLoopingEvent);
}

void Mars::timerExpired(const uint32 eventType) {
	switch (eventType) {
	case kMaze007RobotLoopingEvent:
	case kMaze015RobotLoopingEvent:
	case kMaze101RobotLoopingEvent:
	case kMaze104RobotLoopingEvent:
	case kMaze133RobotLoopingEvent:
	case kMaze136RobotLoopingEvent:
	case kMaze184RobotLoopingEvent:
		_interruptionFilter = kFilterNoInput;
		break;
	}
}

void Mars::arriveAt(const tRoomID room, const tDirectionConstant direction) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kMars18, kNorth):
		if (GameState.getMarsPodAtUpperPlatform())
			setCurrentAlternate(kAltMarsPodAtMars34);
		break;
	case MakeRoomView(kMars27, kEast):
	case MakeRoomView(kMars29, kEast):
		if (GameState.isTakenItemID(kMarsCard))
			setCurrentAlternate(kAltMarsTookCard);
		else
			setCurrentAlternate(kAltMarsNormal);
		break;
	case MakeRoomView(kMars35, kEast):
	case MakeRoomView(kMars35, kWest):
		if (GameState.getMarsAirlockOpen())
			setCurrentAlternate(kAltMars35AirlockWest);
		else
			setCurrentAlternate(kAltMars35AirlockEast);
		break;
	case MakeRoomView(kMars60, kEast):
	case MakeRoomView(kMars60, kWest):
		if (GameState.getMarsAirlockOpen())
			setCurrentAlternate(kAltMars60AirlockEast);
		else
			setCurrentAlternate(kAltMars60AirlockWest);
		break;
	case MakeRoomView(kMars45, kNorth):
	case MakeRoomView(kMars45, kSouth):
	case MakeRoomView(kMars45, kEast):
	case MakeRoomView(kMars45, kWest):
		GameState.setMarsPodAtUpperPlatform(false);
		setCurrentAlternate(kAltMarsPodAtMars45);
		break;
	case MakeRoomView(kMars46, kNorth):
	case MakeRoomView(kMars46, kSouth):
	case MakeRoomView(kMars46, kEast):
	case MakeRoomView(kMars46, kWest):
	case MakeRoomView(kMars47, kNorth):
	case MakeRoomView(kMars47, kSouth):
	case MakeRoomView(kMars47, kEast):
	case MakeRoomView(kMars47, kWest):
		if (GameState.getMarsPodAtUpperPlatform())
			setCurrentAlternate(kAltMarsNormal);
		else
			setCurrentAlternate(kAltMarsPodAtMars45);
		break;
	case MakeRoomView(kMars48, kNorth):
	case MakeRoomView(kMars48, kSouth):
	case MakeRoomView(kMars48, kEast):
	case MakeRoomView(kMars48, kWest):
	case MakeRoomView(kMars49, kNorth):
	case MakeRoomView(kMars49, kEast):
	case MakeRoomView(kMars49, kWest):
		if (GameState.isTakenItemID(kAirMask))
			setCurrentAlternate(kAltMarsTookMask);
		else
			setCurrentAlternate(kAltMarsNormal);
		break;
	case MakeRoomView(kMars49, kSouth):
		if (GameState.getMarsMaskOnFiller())
			setCurrentAlternate(kAltMarsMaskOnFiller);
		else if (GameState.isTakenItemID(kAirMask))
			setCurrentAlternate(kAltMarsTookMask);
		else
			setCurrentAlternate(kAltMarsNormal);
		break;
	}

	Neighborhood::arriveAt(room, direction);
	checkAirlockDoors();
	setUpReactorEnergyDrain();

	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kMars0A, kNorth):
		if (!GameState.getMarsSeenTimeStream())
			startExtraLongSequence(kMarsArrivalFromTSA, kMars0AWatchShuttleDepart, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kMars07, kSouth):
	case MakeRoomView(kMars13, kNorth):
		if (!GameState.getMarsHeardCheckInMessage()) {
			playSpotSoundSync(kMarsCheckInRequiredIn, kMarsCheckInRequiredOut);
			GameState.setMarsHeardCheckInMessage(true);
		}
		break;
	case MakeRoomView(kMars44, kWest):
		// TODO: Space Chase
		_neighborhoodNotification.setNotificationFlags(kTimeForCanyonChaseFlag, kTimeForCanyonChaseFlag);
		break;
	case MakeRoomView(kMars10, kNorth):
		if (!GameState.getMarsRobotThrownPlayer())
			startExtraSequence(kRobotThrowsPlayer, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kMars11, kSouth):
	case MakeRoomView(kMars12, kSouth):
		setCurrentActivation(kActivationReadyForKiosk);
		break;
	case MakeRoomView(kMars15, kWest):
		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSecurityDown()) {
			playSpotSoundSync(kMarsShuttle2DepartedIn, kMarsShuttle2DepartedOut);
			restoreStriding(kMars17, kWest, kAltMarsNormal);
			GameState.setMarsSecurityDown(true);
		}
		break;
	case MakeRoomView(kMars17, kNorth):
	case MakeRoomView(kMars17, kSouth):
	case MakeRoomView(kMars17, kEast):
	case MakeRoomView(kMars17, kWest):
		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSecurityDown())
			forceStridingStop(kMars17, kWest, kAltMarsNormal);

		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSawRobotLeave()) {
			startExtraSequence(kRobotOnWayToShuttle, kExtraCompletedFlag, kFilterNoInput);
			restoreStriding(kMars19, kWest, kAltMarsNormal);
			GameState.setMarsSawRobotLeave(true);
		}
		break;
	case MakeRoomView(kMars19, kNorth):
	case MakeRoomView(kMars19, kSouth):
	case MakeRoomView(kMars19, kWest):
		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSawRobotLeave())
			forceStridingStop(kMars19, kWest, kAltMarsNormal);

		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSecurityDown())
			forceStridingStop(kMars17, kWest, kAltMarsNormal);
		break;
	case MakeRoomView(kMars19, kEast):
		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSawRobotLeave())
			forceStridingStop(kMars19, kWest, kAltMarsNormal);

		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSecurityDown())
			forceStridingStop(kMars17, kWest, kAltMarsNormal);
		break;
	case MakeRoomView(kMars32, kNorth):
		if (!GameState.getMarsPodAtUpperPlatform()) {
			playSpotSoundSync(kMarsPodArrivedUpperPlatformIn, kMarsPodArrivedUpperPlatformOut);
			GameState.setMarsPodAtUpperPlatform(true);
		}
		break;
	case MakeRoomView(kMars33North, kNorth):
		setCurrentActivation(kActivationTunnelMapReady);
		// Fall through...
	case MakeRoomView(kMars33, kSouth):
	case MakeRoomView(kMars33, kEast):
	case MakeRoomView(kMars33, kWest):
	case MakeRoomView(kMars32, kSouth):
	case MakeRoomView(kMars32, kEast):
	case MakeRoomView(kMars32, kWest):
		if (!GameState.getMarsPodAtUpperPlatform())
			GameState.setMarsPodAtUpperPlatform(true);
		break;
	case MakeRoomView(kMars34, kNorth):
		startExtraSequence(kMars34NorthPodGreeting, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kMars34, kSouth):
	case MakeRoomView(kMars45, kNorth):
		setCurrentActivation(kActivateMarsPodClosed);
		break;
	case MakeRoomView(kMars35, kWest):
		if (GameState.getMarsThreadedMaze() && !GameState.getMarsSecurityDown())
			forceStridingStop(kMars19, kWest, kAltMarsNormal);
		// Fall through...
	case MakeRoomView(kMars60, kEast):
		if (!GameState.getMarsAirlockOpen())
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
		break;
	case MakeRoomView(kMars35, kEast):
	case MakeRoomView(kMars60, kWest):
		if (GameState.getMarsAirlockOpen())
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
		break;
	case MakeRoomView(kMars39, kWest):
		if (GameState.getLastRoom() == kMarsMaze200)
			GameState.setMarsPodAtUpperPlatform(false);
		break;
	case MakeRoomView(kMars45, kSouth):
		// Set up maze doors here.
		// Doing it here makes sure that it will be the same if the player comes
		// back out of the maze and goes back in, but will vary if
		// the player comes back down to the maze a second time.
		GameState.setMarsMazeDoorPair1(_vm->getRandomBit());
		GameState.setMarsMazeDoorPair2(_vm->getRandomBit());
		GameState.setMarsMazeDoorPair3(_vm->getRandomBit());
		GameState.setMarsArrivedBelow(true);
		break;
	case MakeRoomView(kMars48, kEast):
		if (!GameState.getMarsSeenRobotAtReactor()) {
			// Preload the looping sound...
			loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0, 0, 0);
			startExtraSequence(kMars48RobotApproaches, kExtraCompletedFlag, kFilterNoInput);
		} else if (!GameState.getMarsAvoidedReactorRobot()) {
			loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0x100, 0, 0);
			loopExtraSequence(kMars48RobotLoops);
			_utilityFuse.primeFuse(kMarsRobotPatienceLimit);
			_utilityFuse.setFunctionPtr(&robotTimerExpiredFunction, (void *)this);
			_utilityFuse.lightFuse();
		}
		break;
	case MakeRoomView(kMars48, kSouth):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot()) {
			loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0x100, 0, 0);
			_utilityFuse.primeFuse(kMarsRobotPatienceLimit);
			_utilityFuse.setFunctionPtr(&robotTimerExpiredFunction, (void *)this);
			_utilityFuse.lightFuse();
		}
		break;
	case MakeRoomView(kMars49, kSouth):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot()) {
			playSpotSoundSync(kMarsRobotTakesTransportIn, kMarsRobotTakesTransportOut);
			playSpotSoundSync(kMarsPodDepartedLowerPlatformIn, kMarsPodDepartedLowerPlatformOut);
			GameState.setMarsAvoidedReactorRobot(true);
			GameState.setMarsPodAtUpperPlatform(true);
			GameState.getScoringAvoidedRobot();
		}

		if (GameState.isTakenItemID(kAirMask))
			setCurrentActivation(kActivateHotSpotAlways);
		else if (GameState.getMarsMaskOnFiller())
			setCurrentActivation(kActivateMaskOnFiller);
		else
			setCurrentActivation(kActivateMaskOnHolder);
		break;
	case MakeRoomView(kMars51, kWest):
	case MakeRoomView(kMars50, kWest):
	case MakeRoomView(kMars48, kWest):
		if (GameState.getShieldOn())
			g_shield->setItemState(kShieldNormal);
		g_energyMonitor->setEnergyDrainRate(kEnergyDrainNormal);
		_vm->resetEnergyDeathReason();
		break;
	case MakeRoomView(kMars52, kNorth):
	case MakeRoomView(kMars52, kSouth):
	case MakeRoomView(kMars52, kEast):
	case MakeRoomView(kMars52, kWest):
	case MakeRoomView(kMars54, kNorth):
	case MakeRoomView(kMars54, kSouth):
	case MakeRoomView(kMars54, kEast):
	case MakeRoomView(kMars54, kWest):
	case MakeRoomView(kMars56, kNorth):
	case MakeRoomView(kMars56, kSouth):
	case MakeRoomView(kMars56, kWest):
	case MakeRoomView(kMars58, kNorth):
	case MakeRoomView(kMars58, kSouth):
	case MakeRoomView(kMars58, kEast):
	case MakeRoomView(kMars58, kWest):
		setCurrentActivation(kActivateReactorPlatformOut);
		break;
	case MakeRoomView(kMars56, kEast):
		if (GameState.getMarsLockBroken()) {
			setCurrentActivation(kActivateReactorAskOperation);
			_privateFlags.setFlag(kMarsPrivatePlatformZoomedInFlag, true);
		} else if (GameState.getMarsLockFrozen()) {
			setCurrentActivation(kActivateReactorReadyForCrowBar);
			_privateFlags.setFlag(kMarsPrivatePlatformZoomedInFlag, true);
			_utilityFuse.primeFuse(kLockFreezeTimeLmit);
			_utilityFuse.setFunctionPtr(&lockThawTimerExpiredFunction, (void *)this);
			_utilityFuse.lightFuse();
		} else {
			setCurrentActivation(kActivateReactorPlatformOut);
		}
		break;
	case MakeRoomView(kMarsRobotShuttle, kEast):
		setCurrentActivation(kActivationRobotHeadClosed);
		break;
	case MakeRoomView(kMarsMaze007, kNorth):
		launchMaze007Robot();
		break;
	case MakeRoomView(kMarsMaze015, kSouth):
		launchMaze015Robot();
		break;
	case MakeRoomView(kMarsMaze101, kEast):
		launchMaze101Robot();
		break;
	case MakeRoomView(kMarsMaze104, kWest):
		launchMaze104Robot();
		break;
	case MakeRoomView(kMarsMaze133, kSouth):
		launchMaze133Robot();
		break;
	case MakeRoomView(kMarsMaze136, kNorth):
		launchMaze136Robot();
		break;
	case MakeRoomView(kMarsMaze184, kWest):
		launchMaze184Robot();
		break;
	case MakeRoomView(kMarsMaze199, kSouth):
		GameState.setScoringThreadedMaze();
		GameState.setMarsThreadedMaze(true);
		break;
	case MakeRoomView(kMarsDeathRoom, kNorth):
	case MakeRoomView(kMarsDeathRoom, kSouth):
	case MakeRoomView(kMarsDeathRoom, kEast):
	case MakeRoomView(kMarsDeathRoom, kWest):
		switch (GameState.getLastRoom()) {
		case kMars39:
			die(kDeathDidntLeaveBucket);
			break;
		case kMars46:
			die(kDeathRunOverByPod);
			break;
		}
		break;
	}

	checkAirMask();
}

void Mars::shieldOn() {
	setUpReactorEnergyDrain();
}

void Mars::shieldOff() {
	setUpReactorEnergyDrain();
}

void Mars::turnTo(const tDirectionConstant direction) {
	switch (MakeRoomView(GameState.getCurrentRoom(), direction)) {
	case MakeRoomView(kMars27, kNorth):
	case MakeRoomView(kMars27, kSouth):
	case MakeRoomView(kMars27, kEast):
	case MakeRoomView(kMars29, kNorth):
	case MakeRoomView(kMars29, kSouth):
	case MakeRoomView(kMars29, kEast):
		if (GameState.isTakenItemID(kMarsCard))
			setCurrentAlternate(kAltMarsTookCard);
		break;
	case MakeRoomView(kMars35, kNorth):
	case MakeRoomView(kMars35, kSouth):
	case MakeRoomView(kMars60, kNorth):
	case MakeRoomView(kMars60, kSouth):
		if (getCurrentActivation() == kActivateAirlockPressurized)
			playSpotSoundSync(kMarsAirlockPressurizeIn, kMarsAirlockPressurizeOut);
		break;
	}

	Neighborhood::turnTo(direction);

	switch (MakeRoomView(GameState.getCurrentRoom(), direction)) {
	case MakeRoomView(kMars11, kSouth):
	case MakeRoomView(kMars12, kSouth):
		setCurrentActivation(kActivationReadyForKiosk);
		break;
	case MakeRoomView(kMars18, kNorth):
		if (GameState.getMarsPodAtUpperPlatform())
			setCurrentAlternate(kAltMarsPodAtMars34);
		break;
	case MakeRoomView(kMars22, kSouth):
		if (!GameState.getMarsHeardCheckInMessage()) {
			playSpotSoundSync(kMarsCheckInRequiredIn, kMarsCheckInRequiredOut);
			GameState.setMarsHeardCheckInMessage(true);
		}
		break;
	case MakeRoomView(kMars34, kSouth):
	case MakeRoomView(kMars45, kNorth):
		setCurrentActivation(kActivateMarsPodClosed);
		break;
	case MakeRoomView(kMars34, kNorth):
		startExtraSequence(kMars34NorthPodGreeting, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kMars35, kEast):
	case MakeRoomView(kMars60, kWest):
		if (GameState.getMarsAirlockOpen())
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
		break;
	case MakeRoomView(kMars60, kEast):
		if (!GameState.getMarsAirlockOpen())
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
		break;
	case MakeRoomView(kMars35, kWest):
		if (!GameState.getMarsAirlockOpen())
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
		
		// Do this here because this will be called after spinning the airlock after
		// going through the gear room.
		if (GameState.getMarsThreadedMaze())
			GameState.setScoringThreadedGearRoom();
		break;
	case MakeRoomView(kMars48, kNorth):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot())
			die(kDeathDidntGetOutOfWay);
		break;
	case MakeRoomView(kMars48, kEast):
		if (!GameState.getMarsSeenRobotAtReactor()) {
			// Preload the looping sound...
			loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0, 0, 0);
			startExtraSequence(kMars48RobotApproaches, kExtraCompletedFlag, kFilterNoInput);
		} else if (!GameState.getMarsAvoidedReactorRobot()) {
			loopExtraSequence(kMars48RobotLoops);
		} else if (GameState.isTakenItemID(kAirMask)) {
			setCurrentAlternate(kAltMarsTookMask);
		} else {
			setCurrentAlternate(kAltMarsNormal);
		}
		break;
	case MakeRoomView(kMars48, kWest):
		if (GameState.getMarsSeenRobotAtReactor() && !GameState.getMarsAvoidedReactorRobot())
			die(kDeathDidntGetOutOfWay);
		else if (GameState.isTakenItemID(kAirMask))
			setCurrentAlternate(kAltMarsTookMask);
		else
			setCurrentAlternate(kAltMarsNormal);
		break;
	case MakeRoomView(kMars49, kSouth):
		if (GameState.isTakenItemID(kAirMask))
			setCurrentActivation(kActivateHotSpotAlways);
		else
			setCurrentActivation(kActivateMaskOnHolder);
		break;
	case MakeRoomView(kMars52, kNorth):
	case MakeRoomView(kMars52, kSouth):
	case MakeRoomView(kMars52, kEast):
	case MakeRoomView(kMars52, kWest):
	case MakeRoomView(kMars54, kNorth):
	case MakeRoomView(kMars54, kSouth):
	case MakeRoomView(kMars54, kEast):
	case MakeRoomView(kMars54, kWest):
	case MakeRoomView(kMars56, kNorth):
	case MakeRoomView(kMars56, kSouth):
	case MakeRoomView(kMars56, kEast):
	case MakeRoomView(kMars56, kWest):
	case MakeRoomView(kMars58, kNorth):
	case MakeRoomView(kMars58, kSouth):
	case MakeRoomView(kMars58, kEast):
	case MakeRoomView(kMars58, kWest):
		setCurrentActivation(kActivateReactorPlatformOut);
		break;
	case MakeRoomView(kMarsMaze007, kNorth):
		launchMaze007Robot();
		break;
	case MakeRoomView(kMarsMaze015, kSouth):
		launchMaze015Robot();
		break;
	case MakeRoomView(kMarsMaze101, kEast):
		launchMaze101Robot();
		break;
	case MakeRoomView(kMarsMaze104, kWest):
		launchMaze104Robot();
		break;
	case MakeRoomView(kMarsMaze133, kSouth):
		launchMaze133Robot();
		break;
	case MakeRoomView(kMarsMaze136, kNorth):
		launchMaze136Robot();
		break;
	case MakeRoomView(kMarsMaze184, kWest):
		launchMaze184Robot();
		break;
	}
}

void Mars::activateOneHotspot(HotspotInfoTable::Entry &entry, Hotspot *hotspot) {
	switch (hotspot->getObjectID()) {
	case kMars57RedMoveSpotID:
	case kMars57YellowMoveSpotID:
	case kMars57GreenMoveSpotID:
		if (!_choiceHighlight.choiceHighlighted(hotspot->getObjectID() - kMars57RedMoveSpotID))
			hotspot->setActive();
		break;
	case kMars57BlueMoveSpotID:
		if (_reactorStage >= 2 && !_choiceHighlight.choiceHighlighted(3))
			hotspot->setActive();
		break;
	case kMars57PurpleMoveSpotID:
		if (_reactorStage == 3 && !_choiceHighlight.choiceHighlighted(4))
			hotspot->setActive();
		break;
	default:
		Neighborhood::activateOneHotspot(entry, hotspot);
		break;
	}
}

void Mars::activateHotspots() {
	InventoryItem *item;

	Neighborhood::activateHotspots();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars48, kEast):
		if ((_navMovie.getFlags() & kLoopTimeBase) != 0 && _vm->getDragType() == kDragInventoryUse)
			g_allHotspots.activateOneHotspot(kAttackRobotHotSpotID);
		break;
	case MakeRoomView(kMars56, kEast):
		switch (getCurrentActivation()) {
		case kActivateReactorReadyForNitrogen:
			item = (InventoryItem *)g_allItems.findItemByID(kNitrogenCanister);
			if (item->getItemState() != kNitrogenFull)
				g_allHotspots.deactivateOneHotspot(kMars57DropNitrogenSpotID);
			// Fall through...
		case kActivateReactorReadyForCrowBar:
			g_allHotspots.activateOneHotspot(kMars57CantOpenPanelSpotID);
			break;
		}
		break;
	case MakeRoomView(kMarsRobotShuttle, kEast):
		if (_privateFlags.getFlag(kMarsPrivateGotMapChipFlag))
			g_allHotspots.deactivateOneHotspot(kRobotShuttleMapChipSpotID);
		else
			g_allHotspots.activateOneHotspot(kRobotShuttleMapChipSpotID);

		if (_privateFlags.getFlag(kMarsPrivateGotOpticalChipFlag))
			g_allHotspots.deactivateOneHotspot(kRobotShuttleOpticalChipSpotID);
		else
			g_allHotspots.activateOneHotspot(kRobotShuttleOpticalChipSpotID);

		if (_privateFlags.getFlag(kMarsPrivateGotShieldChipFlag))
			g_allHotspots.deactivateOneHotspot(kRobotShuttleShieldChipSpotID);
		else
			g_allHotspots.activateOneHotspot(kRobotShuttleShieldChipSpotID);
		break;
	default:
		if (_privateFlags.getFlag(kMarsPrivateInSpaceChaseFlag)) {
			// TODO
		}
		break;
	}
}

void Mars::clickInHotspot(const Input &input, const Hotspot *clickedSpot) {
	switch (clickedSpot->getObjectID()) {
	case kMars11NorthKioskSpotID:
	case kMars12NorthKioskSpotID:
		playSpotSoundSync(kMarsKioskBeepIn, kMarsKioskBeepOut);
		Neighborhood::clickInHotspot(input, clickedSpot);
		break;
	case kMars11NorthKioskSightsSpotID:
	case kMars12NorthKioskSightsSpotID:
		playSpotSoundSync(kMarsKioskBeepIn, kMarsKioskBeepOut);
		if (!startExtraSequenceSync(kMarsSightsInfo, kFilterAllInput))
			showExtraView(kMarsInfoKioskIntro);
		break;
	case kMars11NorthKioskColonySpotID:
	case kMars12NorthKioskColonySpotID:
		playSpotSoundSync(kMarsKioskBeepIn, kMarsKioskBeepOut);
		if (!startExtraSequenceSync(kMarsColonyInfo, kFilterAllInput))
			showExtraView(kMarsInfoKioskIntro);
		break;
	case kMars33NorthMonitorSpotID:
		switch (_lastExtra) {
		case kMars33SlideShow1:
			startExtraSequence(kMars33SlideShow2, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMars33SlideShow2:
			startExtraSequence(kMars33SlideShow3, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMars33SlideShow3:
			startExtraSequence(kMars33SlideShow4, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMars33SlideShow4:
			// Should never happen...
		default:
			startExtraSequence(kMars33SlideShow1, kExtraCompletedFlag, kFilterNoInput);
			break;
		}
		break;
	case kMars34SouthOpenStorageSpotID:
		if (GameState.isTakenItemID(kCrowbar))
			startExtraSequence(kMars34SpotOpenNoBar, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraSequence(kMars34SpotOpenWithBar, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kMars34SouthCloseStorageSpotID:
		if (GameState.isTakenItemID(kCrowbar))
			startExtraSequence(kMars34SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraSequence(kMars34SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kMars35WestPressurizeSpotID:
	case kMars35EastPressurizeSpotID:
	case kMars60WestPressurizeSpotID:
	case kMars60EastPressurizeSpotID:
		playSpotSoundSync(kMarsAirlockButtonBeepIn, kMarsAirlockButtonBeepOut);
		playSpotSoundSync(kMarsAirlockPressurizeIn, kMarsAirlockPressurizeOut);
		setCurrentActivation(kActivateAirlockPressurized);
		break;
	case kMars45NorthOpenStorageSpotID:
		if (GameState.isTakenItemID(kCrowbar))
			startExtraSequence(kMars45SpotOpenNoBar, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraSequence(kMars45SpotOpenWithBar, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kMars45NorthCloseStorageSpotID:
		if (GameState.isTakenItemID(kCrowbar))
			startExtraSequence(kMars45SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
		else
			startExtraSequence(kMars45SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kMars56ExtractSpotID:
		if (GameState.isTakenItemID(kCardBomb)) {
			startExtraSequence(kMars56ExtendNoBomb, kExtraCompletedFlag, kFilterNoInput);
			setCurrentActivation(kActivateReactorPlatformIn);
		} else {
			startExtraSequence(kMars56ExtendWithBomb, kExtraCompletedFlag, kFilterNoInput);
			setCurrentActivation(kActivateReactorAskLowerScreen);
		}
		break;
	case kMars57UndoMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doUndoOneGuess();
		break;
	case kMars57RedMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doReactorGuess(0);
		break;
	case kMars57YellowMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doReactorGuess(1);
		break;
	case kMars57GreenMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doReactorGuess(2);
		break;
	case kMars57BlueMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doReactorGuess(3);
		break;
	case kMars57PurpleMoveSpotID:
		playSpotSoundSync(kMarsColorMatchingButtonBeepIn, kMarsColorMatchingButtonBeepOut);
		doReactorGuess(4);
		break;
	default:
		Neighborhood::clickInHotspot(input, clickedSpot);
		break;
	}
}

tInputBits Mars::getInputFilter() {
	tInputBits result = Neighborhood::getInputFilter();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars49, kSouth):
		if (GameState.getMarsMaskOnFiller())
			// Can't move when mask is on filler.
			result &= ~kFilterAllDirections;
		break;
	case MakeRoomView(kMars52, kNorth):
	case MakeRoomView(kMars52, kSouth):
	case MakeRoomView(kMars52, kEast):
	case MakeRoomView(kMars52, kWest):
	case MakeRoomView(kMars54, kNorth):
	case MakeRoomView(kMars54, kSouth):
	case MakeRoomView(kMars54, kEast):
	case MakeRoomView(kMars54, kWest):
	case MakeRoomView(kMars56, kNorth):
	case MakeRoomView(kMars56, kSouth):
	case MakeRoomView(kMars56, kEast):
	case MakeRoomView(kMars56, kWest):
	case MakeRoomView(kMars58, kNorth):
	case MakeRoomView(kMars58, kSouth):
	case MakeRoomView(kMars58, kEast):
	case MakeRoomView(kMars58, kWest):
		if (_privateFlags.getFlag(kMarsPrivatePlatformZoomedInFlag))
			// Can't move when platform is extended.
			result &= ~kFilterAllDirections;
		break;
	case MakeRoomView(kMars44, kWest):
		// TODO
		break;
	}

	return result;
}

// Only called when trying to pick up an item and the player can't (because
// the inventory is too full or because the player lets go of the item before
// dropping it into the inventory).
Hotspot *Mars::getItemScreenSpot(Item *item, DisplayElement *element) {
	tHotSpotID destSpotID;

	switch (item->getObjectID()) {
	case kCardBomb:
		destSpotID = kMars57GrabBombSpotID;
		break;
	case kMarsCard:
		destSpotID = kMars31SouthCardSpotID;
		break;
	case kAirMask:
		if (GameState.getMarsMaskOnFiller())
			destSpotID = kMars49AirFillingDropSpotID;
		else
			destSpotID = kMars49AirMaskSpotID;
		break;
	case kCrowbar:
		if (GameState.getCurrentRoom() == kMars34)
			destSpotID = kMars34SouthCrowbarSpotID;
		else
			destSpotID = kMars45NorthCrowbarSpotID;
		break;
	case kMapBiochip:
		destSpotID = kRobotShuttleMapChipSpotID;
		break;
	case kOpticalBiochip:
		destSpotID = kRobotShuttleOpticalChipSpotID;
		break;
	case kShieldBiochip:
		destSpotID = kRobotShuttleShieldChipSpotID;
		break;
	default:
		destSpotID = kNoHotSpotID;
		break;
	}

	if (destSpotID == kNoHotSpotID)
		return Neighborhood::getItemScreenSpot(item, element);

	return g_allHotspots.findHotspotByID(destSpotID);
}

void Mars::takeItemFromRoom(Item *item) {
	switch (item->getObjectID()) {
	case kAirMask:
		setCurrentAlternate(kAltMarsTookMask);
		break;
	case kCardBomb:
		_privateFlags.setFlag(kMarsPrivateDraggingBombFlag, true);
		break;
	case kMapBiochip:
		_privateFlags.setFlag(kMarsPrivateGotMapChipFlag, true);
		break;
	case kShieldBiochip:
		_privateFlags.setFlag(kMarsPrivateGotShieldChipFlag, true);
		break;
	case kOpticalBiochip:
		_privateFlags.setFlag(kMarsPrivateGotOpticalChipFlag, true);
		break;
	}

	Neighborhood::takeItemFromRoom(item);
}

void Mars::pickedUpItem(Item *item) {
	switch (item->getObjectID()) {
	case kAirMask:
		setCurrentActivation(kActivateHotSpotAlways);
		if (!GameState.getScoringGotOxygenMask()) {
			g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Mars/XM48SB", false, kWarningInterruption);
			GameState.setScoringGotOxygenMask();
		}
		break;
	case kCrowbar:
		GameState.setScoringGotCrowBar();
		g_AIArea->checkMiddleArea();
		break;
	case kMarsCard:
		GameState.setScoringGotMarsCard();
		g_AIArea->checkMiddleArea();
		break;
	case kCardBomb:
		GameState.setScoringGotCardBomb();
		if (GameState.getMarsLockBroken()) {
			startExtraSequence(kMars57BackToNormal, kExtraCompletedFlag, kFilterNoInput);
			GameState.setMarsLockBroken(false);
		}

		_privateFlags.setFlag(kMarsPrivateDraggingBombFlag, false);
		break;
	case kMapBiochip:
		if (_privateFlags.getFlag(kMarsPrivateGotMapChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotShieldChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotOpticalChipFlag)) {
			GameState.setMarsFinished(true);
			GameState.setScoringMarsGandhi();
			startExtraSequence(kMarsRobotHeadClose, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kShieldBiochip:
		if (_privateFlags.getFlag(kMarsPrivateGotMapChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotShieldChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotOpticalChipFlag)) {
			GameState.setMarsFinished(true);
			GameState.setScoringMarsGandhi();
			startExtraSequence(kMarsRobotHeadClose, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kOpticalBiochip:
		g_opticalChip->addAries();
		GameState.setScoringGotMarsOpMemChip();
			
		if (_privateFlags.getFlag(kMarsPrivateGotMapChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotShieldChipFlag) &&
				_privateFlags.getFlag(kMarsPrivateGotOpticalChipFlag)) {
			GameState.setMarsFinished(true);
			GameState.setScoringMarsGandhi();
			startExtraSequence(kMarsRobotHeadClose, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	}
}

void Mars::dropItemIntoRoom(Item *item, Hotspot *dropSpot) {
	if (dropSpot->getObjectID() == kAttackRobotHotSpotID) {
		_attackingItem = (InventoryItem *)item;
		startExtraSequence(kMars48RobotDefends, kExtraCompletedFlag, kFilterNoInput);
		loadLoopSound2("");
	} else {
		switch (item->getObjectID()) {
		case kMarsCard:
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			if (dropSpot && dropSpot->getObjectID() == kMars34NorthCardDropSpotID)
				startExtraSequence(kMarsTurnOnPod, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kNitrogenCanister:
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			if (dropSpot && dropSpot->getObjectID() == kMars57DropNitrogenSpotID)
				startExtraSequence(kMars57FreezeLock, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kCrowbar:
			_utilityFuse.stopFuse();
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			if (dropSpot && dropSpot->getObjectID() == kMars57DropCrowBarSpotID)
				startExtraSequence(kMars57BreakLock, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kAirMask:
			if (dropSpot) {
				if (dropSpot->getObjectID() == kMars49AirFillingDropSpotID) {
					if (!GameState.getMarsMaskOnFiller()) {
						Neighborhood::dropItemIntoRoom(item, dropSpot);
						startExtraSequence(kMars49SouthViewMaskFilling, kExtraCompletedFlag, kFilterNoInput);
					} else {
						setCurrentActivation(kActivateMaskOnFiller);
						setCurrentAlternate(kAltMarsMaskOnFiller);
						Neighborhood::dropItemIntoRoom(item, dropSpot);
					}
				} else if (dropSpot->getObjectID() == kMars49AirMaskSpotID) {
					setCurrentAlternate(kAltMarsNormal);
					setCurrentActivation(kActivateMaskOnHolder);
					Neighborhood::dropItemIntoRoom(item, dropSpot);
				}
			}
			break;
		case kCardBomb:
			_privateFlags.setFlag(kMarsPrivateDraggingBombFlag, false);
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			break;
		case kMapBiochip:
			_privateFlags.setFlag(kMarsPrivateGotMapChipFlag, false);
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			break;
		case kShieldBiochip:
			_privateFlags.setFlag(kMarsPrivateGotShieldChipFlag, false);
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			break;
		case kOpticalBiochip:
			_privateFlags.setFlag(kMarsPrivateGotOpticalChipFlag, false);
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			break;
		default:
			Neighborhood::dropItemIntoRoom(item, dropSpot);
			break;
		}
	}
}

void Mars::robotTiredOfWaiting() {
	if (GameState.getCurrentRoomAndView() == MakeRoomView(kMars48, kEast)) {
		if (_attackingItem) {
			startExtraSequence(kMars48RobotKillsPlayer, kExtraCompletedFlag, kFilterNoInput);
			loadLoopSound2("");
		} else {
			_privateFlags.setFlag(kMarsPrivateRobotTiredOfWaitingFlag, true);
		}
	} else {
		die(kDeathDidntGetOutOfWay);
	}
}

void Mars::turnLeft() {
	if (isEventTimerRunning())
		cancelEvent();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars34, kSouth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			_privateFlags.setFlag(kMarsPrivatePodTurnLeftFlag, true);
			if (GameState.isTakenItemID(kCrowbar))
				startExtraSequence(kMars34SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kMars34SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		} else {
			Neighborhood::turnLeft();
		}
		break;
	case MakeRoomView(kMars45, kNorth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			_privateFlags.setFlag(kMarsPrivatePodTurnLeftFlag, true);
			if (GameState.isTakenItemID(kCrowbar))
				startExtraSequence(kMars45SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kMars45SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		} else {
			Neighborhood::turnLeft();
		}
		break;
	default:
		Neighborhood::turnLeft();
		break;
	}
}

void Mars::turnRight() {
	if (isEventTimerRunning())
		cancelEvent();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kMars34, kSouth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			_privateFlags.setFlag(kMarsPrivatePodTurnRightFlag, true);
			if (GameState.isTakenItemID(kCrowbar))
				startExtraSequence(kMars34SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kMars34SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		} else {
			Neighborhood::turnRight();
		}
		break;
	case MakeRoomView(kMars45, kNorth):
		if (_privateFlags.getFlag(kMarsPrivatePodStorageOpenFlag)) {
			_privateFlags.setFlag(kMarsPrivatePodTurnRightFlag, true);
			if (GameState.isTakenItemID(kCrowbar))
				startExtraSequence(kMars45SpotCloseNoBar, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kMars45SpotCloseWithBar, kExtraCompletedFlag, kFilterNoInput);
		} else {
			Neighborhood::turnRight();
		}
		break;
	default:
		Neighborhood::turnRight();
		break;
	}
}

void Mars::receiveNotification(Notification *notification, const tNotificationFlags flag) {
	InventoryItem *item;
	
	Neighborhood::receiveNotification(notification, flag);

	if ((flag & kExtraCompletedFlag) != 0) {
		_interruptionFilter = kFilterAllInput;

		switch (_lastExtra) {
		case kMarsArrivalFromTSA:
			GameState.setMarsSeenTimeStream(true);
			loadAmbientLoops();
			playSpotSoundSync(kMarsShuttle1DepartedIn, kMarsShuttle1DepartedOut);
			makeContinuePoint();
			break;
		case kRobotThrowsPlayer:
			GameState.setMarsRobotThrownPlayer(true);
			GameState.setScoringThrownByRobot();
			restoreStriding(kMars08, kNorth, kAltMarsNormal);
			arriveAt(kMars08, kNorth);
			if (!GameState.getMarsHeardUpperPodMessage()) {
				playSpotSoundSync(kMarsPodDepartedUpperPlatformIn,
											kMarsPodDepartedUpperPlatformOut);
				GameState.setMarsHeardUpperPodMessage(true);
			}
			break;
		case kMarsInfoKioskIntro:
			GameState.setScoringSawMarsKiosk();
			setCurrentActivation(kActivationKioskChoice);
			break;
		case kMars33SlideShow4:
			GameState.setScoringSawTransportMap();
			setCurrentActivation(kActivateHotSpotAlways);
			break;
		case kMars34SpotOpenNoBar:
		case kMars34SpotOpenWithBar:
		case kMars45SpotOpenNoBar:
		case kMars45SpotOpenWithBar:
			_privateFlags.setFlag(kMarsPrivatePodStorageOpenFlag, true);
			setCurrentActivation(kActivateMarsPodOpen);
			break;
		case kMars34SpotCloseNoBar:
		case kMars34SpotCloseWithBar:
		case kMars45SpotCloseNoBar:
		case kMars45SpotCloseWithBar:
			_privateFlags.setFlag(kMarsPrivatePodStorageOpenFlag, false);
			setCurrentActivation(kActivateMarsPodClosed);
			if (_privateFlags.getFlag(kMarsPrivatePodTurnLeftFlag)) {
				_privateFlags.setFlag(kMarsPrivatePodTurnLeftFlag, false);
				turnLeft();
			} else if (_privateFlags.getFlag(kMarsPrivatePodTurnRightFlag)) {
				_privateFlags.setFlag(kMarsPrivatePodTurnRightFlag, false);
				turnRight();
			}
			break;
		case kMarsTurnOnPod:
			item = (InventoryItem *)g_allItems.findItemByID(kMarsCard);
			_vm->addItemToInventory(item);
			GameState.setScoringTurnedOnTransport();
			loadLoopSound1("");
			loadLoopSound2("");
			startExtraSequence(kMarsTakePodToMars45, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMarsTakePodToMars45:
			arriveAt(kMars45, kSouth);
			break;
		case kMars35WestSpinAirlockToEast:
			GameState.setMarsAirlockOpen(false);
			setCurrentAlternate(kAltMars35AirlockEast);
			turnTo(kWest);
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
			g_airMask->airQualityChanged();
			checkAirMask();
			loadAmbientLoops();
			break;
		case kMars35EastSpinAirlockToWest:
			GameState.setMarsAirlockOpen(true);
			setCurrentAlternate(kAltMars35AirlockWest);
			turnTo(kEast);
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
			g_airMask->airQualityChanged();
			checkAirMask();
			loadAmbientLoops();
			break;
		case kMars48RobotApproaches:
			loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0x100, 0, 0);
			GameState.setMarsSeenRobotAtReactor(true);
			loopExtraSequence(kMars48RobotLoops);
			_utilityFuse.primeFuse(kMarsRobotPatienceLimit);
			_utilityFuse.setFunctionPtr(&robotTimerExpiredFunction, (void *)this);
			_utilityFuse.lightFuse();
			break;
		case kMars48RobotDefends:
			_vm->addItemToInventory(_attackingItem);
			_attackingItem = 0;
			if (_privateFlags.getFlag(kMarsPrivateRobotTiredOfWaitingFlag)) {
				startExtraSequence(kMars48RobotKillsPlayer, kExtraCompletedFlag, kFilterNoInput);
				loadLoopSound2("", 0x100, 0, 0);
			} else {
				loadLoopSound2("Sounds/Mars/Robot Loop.aiff", 0x100, 0, 0);
				loopExtraSequence(kMars48RobotLoops, kExtraCompletedFlag);
			}
			break;
		case kMars48RobotKillsPlayer:
			loadLoopSound2("");
			die(kDeathDidntGetOutOfWay);
			break;
		case kMars49SouthViewMaskFilling:
			setCurrentActivation(kActivateMaskOnFiller);
			setCurrentAlternate(kAltMarsMaskOnFiller);
			GameState.setMarsMaskOnFiller(true);
			break;
		case kMars58SpinLeft:
		case kMars54SpinRight:
			GameState.setScoringActivatedPlatform();
			arriveAt(kMars52, kEast);
			break;
		case kMars52SpinLeft:
		case kMars56SpinRight:
			GameState.setScoringActivatedPlatform();
			arriveAt(kMars54, kEast);
			break;
		case kMars54SpinLeft:
		case kMars58SpinRight:
			GameState.setScoringActivatedPlatform();
			arriveAt(kMars56, kEast);
			break;
		case kMars56SpinLeft:
		case kMars52SpinRight:
			GameState.setScoringActivatedPlatform();
			arriveAt(kMars58, kEast);
			break;
		case kMars52Extend:
		case kMars54Extend:
		case kMars56ExtendNoBomb:
		case kMars58Extend:
			GameState.setScoringActivatedPlatform();
			setCurrentActivation(kActivateReactorPlatformIn);
			_privateFlags.setFlag(kMarsPrivatePlatformZoomedInFlag, true);
			break;
		case kMars53Retract:
		case kMars55Retract:
		case kMars57RetractWithBomb:
		case kMars57RetractNoBomb:
		case kMars59Retract:
			GameState.setScoringActivatedPlatform();
			setCurrentActivation(kActivateReactorPlatformOut);
			_privateFlags.setFlag(kMarsPrivatePlatformZoomedInFlag, false);
			break;
		case kMars56ExtendWithBomb:
			playSpotSoundSync(kMustBeUnlockedIn, kMustBeUnlockedOut);
			GameState.setScoringActivatedPlatform();
			_privateFlags.setFlag(kMarsPrivatePlatformZoomedInFlag, true);
			break;
		case kMars57CantOpenPanel:
			GameState.setScoringActivatedPlatform();
			setCurrentActivation(kActivateReactorAskLowerScreen);
			break;
		case kMars57LowerScreenClosed:
		case kMars57ThawLock:
			setCurrentActivation(kActivateReactorReadyForNitrogen);
			GameState.setMarsLockFrozen(false);
			break;
		case kMars57FreezeLock:
			item = (InventoryItem *)g_allItems.findItemByID(kNitrogenCanister);
			item->setItemState(kNitrogenEmpty);
			_vm->addItemToInventory(item);
			setCurrentActivation(kActivateReactorReadyForCrowBar);
			GameState.setScoringUsedLiquidNitrogen();
			GameState.setMarsLockFrozen(true);
			showExtraView(kMars57LockFrozenView);
			_utilityFuse.primeFuse(kLockFreezeTimeLmit);
			_utilityFuse.setFunctionPtr(&lockThawTimerExpiredFunction, (void *)this);
			_utilityFuse.lightFuse();
			break;
		case kMars57BreakLock:
			item = (InventoryItem *)g_allItems.findItemByID(kCrowbar);
			_vm->addItemToInventory(item);
			GameState.setScoringUsedCrowBar();
			GameState.setMarsLockBroken(true);
			GameState.setMarsLockFrozen(false);
			startExtraLongSequence(kMars57OpenPanel, kMars57OpenPanelChoices, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMars57OpenPanel:
		case kMars57OpenPanelChoices:
			setCurrentActivation(kActivateReactorAskOperation);
			break;
		case kMars57ShieldEvaluation:
		case kMars57MeasureOutput:
			setCurrentActivation(kActivateReactorRanEvaluation);
			loopExtraSequence(kMars57ShieldOkayLoop);
			break;
		case kMars57RunDiagnostics:
			setCurrentActivation(kActivateReactorRanDiagnostics);
			GameState.setScoringFoundCardBomb();
			break;
		case kMars57BombExplodes:
		case kMars57BombExplodesInGame:
			die(kDeathDidntDisarmMarsBomb);
			break;
		case kMars57BombAnalysis:
			setCurrentActivation(kActivateReactorAnalyzed);
			break;
		case kMars57DontLink:
			startExtraSequence(kMars57OpenPanelChoices, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kMars57CircuitLink:
			setCurrentActivation(kActivateReactorInstructions);
			break;
		case kMars57GameLevel1:
			setUpReactorLevel1();
			break;
		case kMars57GameLevel2:
		case kMars57GameLevel3:
			setUpNextReactorLevel();
			break;
		case kMars57GameSolved:
			setCurrentActivation(kActivateReactorBombSafe);
			break;
		case kMars57ExposeBomb:
			setCurrentActivation(kActivateReactorBombExposed);
			_privateFlags.setFlag(kMarsPrivateBombExposedFlag, true);
			break;
		case kMars57BackToNormal:
			setCurrentActivation(kActivateReactorPlatformIn);
			_privateFlags.setFlag(kMarsPrivateBombExposedFlag, false);
			g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Mars/XM51SW", false, kWarningInterruption);
			break;
		case kMars60WestSpinAirlockToEast:
			GameState.setMarsAirlockOpen(true);
			setCurrentAlternate(kAltMars60AirlockEast);
			turnTo(kWest);
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
			g_airMask->airQualityChanged();
			checkAirMask();
			loadAmbientLoops();
			break;
		case kMars60EastSpinAirlockToWest:
			GameState.setMarsAirlockOpen(false);
			setCurrentAlternate(kAltMars60AirlockWest);
			turnTo(kEast);
			setCurrentActivation(kActivateReadyToPressurizeAirlock);
			g_airMask->airQualityChanged();
			checkAirMask();
			loadAmbientLoops();
			break;
		case kMarsRobotHeadOpen:
			setCurrentActivation(kActivationRobotHeadOpen);
			break;
		case kMarsRobotHeadClose:
			recallToTSASuccess();
			break;
		case kMarsMaze007RobotApproach:
		case kMarsMaze015SouthRobotApproach:
		case kMarsMaze101EastRobotApproach:
		case kMarsMaze104WestLoop:
		case kMarsMaze133SouthApproach:
		case kMarsMaze136NorthApproach:
		case kMarsMaze184WestLoop:
			die(kDeathGroundByMazebot);
			break;
		}
	} else if ((flag & kTimeForCanyonChaseFlag) != 0) {
		// TODO
	} else if ((flag & kExplosionFinishedFlag) != 0) {
		// TODO
	} else if ((flag & kTimeToTransportFlag) != 0) {
		// TODO
	}

	if (g_AIArea)
		g_AIArea->checkMiddleArea();
}

void Mars::spotCompleted() {
	Neighborhood::spotCompleted();

	if (GameState.getCurrentRoom() == kMarsRobotShuttle)
		g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Mars/XN59WD", false, kWarningInterruption);
}

tAirQuality Mars::getAirQuality(const tRoomID room) {
	if ((room >= kMars36 && room <= kMars39) || (room >= kMarsMaze004 && room <= kMarsMaze200))
		return kAirQualityVacuum;
	if (room == kMars35 && !GameState.getMarsAirlockOpen())
		return kAirQualityVacuum;
	if (room == kMars60 && !GameState.getMarsAirlockOpen())
		return kAirQualityVacuum;

	return Neighborhood::getAirQuality(room);
}

// Start up panting sound if necessary.

void Mars::checkAirMask() {
	Neighborhood::checkAirMask();

	if (getAirQuality(GameState.getCurrentRoom()) == kAirQualityVacuum) {
		if (g_airMask->isAirMaskOn()) {
			if (_noAirFuse.isFuseLit()) {
				_noAirFuse.stopFuse();
				loadLoopSound2("");
				loadAmbientLoops();
				playSpotSoundSync(kMarsOxyMaskOnIn, kMarsOxyMaskOnOut);
			}
		} else {
			if (!_noAirFuse.isFuseLit()) {
				loadLoopSound2("Sounds/Mars/SukWind1.22K.AIFF");
				_noAirFuse.primeFuse(kVacuumSurvivalTimeLimit);
				_noAirFuse.lightFuse();
			}
		}
	} else {
		if (_noAirFuse.isFuseLit()) {
			_noAirFuse.stopFuse();
			loadLoopSound2("");
			loadAmbientLoops();
		}
	}
}

void Mars::airStageExpired() {
	if (((PegasusEngine *)g_engine)->playerHasItemID(kAirMask))
		die(kDeathNoAirInMaze);
	else
		die(kDeathNoMaskInMaze);
}

void Mars::lockThawed() {
	startExtraSequence(kMars57ThawLock, kExtraCompletedFlag, kFilterNoInput);
}

void Mars::setUpReactorLevel1() {
	_reactorStage = 1;
	makeColorSequence();
	_guessObject.initReactorGuess();
	_undoPict.initFromPICTResource(_vm->_resFork, kReactorUndoHilitePICTID);
	_undoPict.setDisplayOrder(kMonitorLayer);
	_undoPict.moveElementTo(kUndoHiliteLeft, kUndoHiliteTop);
	_undoPict.startDisplaying();
	_guessHistory.initReactorHistory();
	_choiceHighlight.initReactorChoiceHighlight();
	setCurrentActivation(kActivateReactorInGame);
	_bombFuse.primeFuse(kColorMatchingTimeLimit);
	_bombFuse.setFunctionPtr(&bombTimerExpiredInGameFunction, (void *)this);
	_bombFuse.lightFuse();
}

void Mars::setUpNextReactorLevel() {
	_guessObject.show();
	_guessHistory.show();
	_guessHistory.clearHistory();
	_choiceHighlight.show();
	_reactorStage++;
	makeColorSequence();
}

void Mars::makeColorSequence() {
	int32 code[5];
	int32 highest = _reactorStage + 2;

	for (int32 i = 0; i < highest; i++)
		code[i] = i;

	_vm->shuffleArray(code, highest);
	_currentGuess[0] = -1;
	_currentGuess[1] = -1;
	_currentGuess[2] = -1;
	_nextGuess = 0;
	_guessObject.setGuess(-1, -1, -1);
	_guessHistory.setAnswer(code[0], code[1], code[2]);
}

void Mars::doUndoOneGuess() {
	if (_nextGuess > 0) {
		_undoPict.show();
		_vm->delayShell(1, 2);
		_undoPict.hide();
		_nextGuess--;
		_currentGuess[_nextGuess] = -1;
		_guessObject.setGuess(_currentGuess[0], _currentGuess[1], _currentGuess[2]);
		_choiceHighlight.resetHighlight();

		if (_currentGuess[0] != -1) {
			_choiceHighlight.highlightChoice(_currentGuess[0]);

			if (_currentGuess[1] != -1) {
				_choiceHighlight.highlightChoice(_currentGuess[1]);

				if (_currentGuess[2] != -1)
					_choiceHighlight.highlightChoice(_currentGuess[2]);
			}
		}
	}
}

void Mars::doReactorGuess(int32 guess) {
	_choiceHighlight.highlightChoice(guess);
	_currentGuess[_nextGuess] = guess;
	_guessObject.setGuess(_currentGuess[0], _currentGuess[1], _currentGuess[2]);

	switch (guess) {
	case 0:
		playSpotSoundSync(kColorMatchRedIn, kColorMatchRedOut);
		break;
	case 1:
		playSpotSoundSync(kColorMatchYellowIn, kColorMatchYellowOut);
		break;
	case 2:
		playSpotSoundSync(kColorMatchGreenIn, kColorMatchGreenOut);
		break;
	case 3:
		playSpotSoundSync(kColorMatchBlueIn, kColorMatchBlueOut);
		break;
	case 4:
		playSpotSoundSync(kColorMatchPurpleIn, kColorMatchPurpleOut);
		break;
	}

	_nextGuess++;

	if (_nextGuess == 3) {
		_vm->delayShell(1, 2);
		_nextGuess = 0;
		_guessHistory.addGuess(_currentGuess[0], _currentGuess[1], _currentGuess[2]);

		switch (_guessHistory.getCurrentNumCorrect()) {
		case 0:
			playSpotSoundSync(kColorMatchZeroNodesIn, kColorMatchZeroNodesOut);
			break;
		case 1:
			playSpotSoundSync(kColorMatchOneNodeIn, kColorMatchOneNodeOut);
			break;
		case 2:
			playSpotSoundSync(kColorMatchTwoNodesIn, kColorMatchTwoNodesOut);
			break;
		case 3:
			playSpotSoundSync(kColorMatchThreeNodesIn, kColorMatchThreeNodesOut);
			break;
		}

		_currentGuess[0] = -1;
		_currentGuess[1] = -1;
		_currentGuess[2] = -1;
		_guessObject.setGuess(-1, -1, -1);
		_choiceHighlight.resetHighlight();

		if (_guessHistory.isSolved()) {
			_guessHistory.showAnswer();
			_vm->delayShell(1, 2);
			_guessObject.hide();
			_guessHistory.hide();
			_choiceHighlight.hide();

			switch (_reactorStage) {
			case 1:
				startExtraSequence(kMars57GameLevel2, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 2:
				startExtraSequence(kMars57GameLevel3, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 3:
				_bombFuse.stopFuse();
				_guessObject.disposeReactorGuess();
				_undoPict.deallocateSurface();
				_guessHistory.disposeReactorHistory();
				_choiceHighlight.disposeReactorChoiceHighlight();
				GameState.setScoringDisarmedCardBomb();
				startExtraSequence(kMars57GameSolved, kExtraCompletedFlag, kFilterNoInput);
				break;
			}
		} else if (_guessHistory.getNumGuesses() >= 5) {
			_vm->delayShell(2, 1);
			bombExplodesInGame();
		}
	}
}

void Mars::bombExplodesInGame() {
	_guessObject.disposeReactorGuess();
	_undoPict.deallocateSurface();
	_guessHistory.disposeReactorHistory();
	_choiceHighlight.disposeReactorChoiceHighlight();
	startExtraSequence(kMars57BombExplodesInGame, kExtraCompletedFlag, kFilterNoInput);
}

void Mars::didntFindBomb() {
	die(kDeathDidntFindMarsBomb);
}

Common::String Mars::getBriefingMovie() {
	Common::String movieName = Neighborhood::getBriefingMovie();

	if (!movieName.empty())
		return movieName;

	return "Images/AI/Mars/XM01";
}

Common::String Mars::getEnvScanMovie() {
	Common::String movieName = Neighborhood::getEnvScanMovie();

	if (movieName.empty()) {
		tRoomID room = GameState.getCurrentRoom();

		if (room >= kMars0A && room <= kMars21)
			return "Images/AI/Mars/XME1";
		else if (room >= kMars22 && room <= kMars31South)
			return "Images/AI/Mars/XME2";
		else if (room >= kMars52 && room <= kMars58)
			return "Images/AI/Mars/XMREACE";

		return "Images/AI/Mars/XME3";
	}

	return movieName;
}

uint Mars::getNumHints() {
	uint numHints = Neighborhood::getNumHints();

	if (numHints == 0) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kMars27, kNorth):
		case MakeRoomView(kMars28, kNorth):
		case MakeRoomView(kMars49, kSouth):
			numHints = 1;
			break;
		case MakeRoomView(kMars31, kSouth):
		case MakeRoomView(kMars31South, kSouth):
			if (!GameState.isTakenItemID(kMarsCard))
				numHints = 1;
			break;
		case MakeRoomView(kMars34, kNorth):
			if (!GameState.isTakenItemID(kMarsCard))
				numHints = 2;
			break;
		case MakeRoomView(kMars34, kSouth):
		case MakeRoomView(kMars45, kNorth):
			if (!GameState.isTakenItemID(kCrowbar))
				numHints = 1;
			break;
		case MakeRoomView(kMars51, kEast):
			if (GameState.isCurrentDoorOpen() && !GameState.getShieldOn()) {
				if (GameState.isTakenItemID(kShieldBiochip))
					numHints = 1;
				else
					numHints = 2;
			}
			break;
		case MakeRoomView(kMars52, kNorth):
		case MakeRoomView(kMars52, kSouth):
		case MakeRoomView(kMars52, kEast):
		case MakeRoomView(kMars52, kWest):
		case MakeRoomView(kMars54, kNorth):
		case MakeRoomView(kMars54, kSouth):
		case MakeRoomView(kMars54, kEast):
		case MakeRoomView(kMars54, kWest):
		case MakeRoomView(kMars56, kNorth):
		case MakeRoomView(kMars56, kSouth):
		case MakeRoomView(kMars56, kWest):
		case MakeRoomView(kMars58, kNorth):
		case MakeRoomView(kMars58, kSouth):
		case MakeRoomView(kMars58, kEast):
		case MakeRoomView(kMars58, kWest):
			if (!GameState.getShieldOn()) {
				if (GameState.isTakenItemID(kShieldBiochip))
					numHints = 1;
				else
					numHints = 2;
			}
			break;
		case MakeRoomView(kMars56, kEast):
			if (getCurrentActivation() == kActivateReactorReadyForNitrogen) {
				if ((tExtraID)_lastExtra == kMars57LowerScreenClosed)
					numHints = 3;
			} else if (getCurrentActivation() == kActivateReactorPlatformOut) {
				if (!GameState.getShieldOn()) {
					if (GameState.isTakenItemID(kShieldBiochip))
						numHints = 1;
					else
						numHints = 2;
				}
			}
			break;
		}
	}

	return numHints;
}

Common::String Mars::getHintMovie(uint hintNum) {
	Common::String movieName = Neighborhood::getHintMovie(hintNum);

	if (movieName.empty()) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kMars27, kNorth):
		case MakeRoomView(kMars28, kNorth):
			return "Images/AI/Globals/XGLOB5C";
		case MakeRoomView(kMars31, kSouth):
		case MakeRoomView(kMars31South, kSouth):
		case MakeRoomView(kMars34, kSouth):
		case MakeRoomView(kMars45, kNorth):
			return "Images/AI/Globals/XGLOB1C";
		case MakeRoomView(kMars34, kNorth):
			if (hintNum == 1)
				return "Images/AI/Globals/XGLOB2C";

			return "Images/AI/Globals/XGLOB3G";
		case MakeRoomView(kMars49, kSouth):
			if (GameState.isTakenItemID(kAirMask))
				return "Images/AI/Globals/XGLOB3E";

			return "Images/AI/Globals/XGLOB1C";
		case MakeRoomView(kMars51, kEast):
			if (GameState.isTakenItemID(kShieldBiochip))
				return "Images/AI/Mars/XM52NW";

			if (hintNum == 1)
				return "Images/AI/Globals/XGLOB2D";

			return "Images/AI/Globals/XGLOB3F";
		case MakeRoomView(kMars52, kNorth):
		case MakeRoomView(kMars52, kSouth):
		case MakeRoomView(kMars52, kEast):
		case MakeRoomView(kMars52, kWest):
		case MakeRoomView(kMars54, kNorth):
		case MakeRoomView(kMars54, kSouth):
		case MakeRoomView(kMars54, kEast):
		case MakeRoomView(kMars54, kWest):
		case MakeRoomView(kMars56, kNorth):
		case MakeRoomView(kMars56, kSouth):
		case MakeRoomView(kMars56, kWest):
		case MakeRoomView(kMars58, kNorth):
		case MakeRoomView(kMars58, kSouth):
		case MakeRoomView(kMars58, kEast):
		case MakeRoomView(kMars58, kWest):
			if (hintNum == 1) {
				if (GameState.isTakenItemID(kShieldBiochip))
					return "Images/AI/Mars/XM52NW";

				return "Images/AI/Globals/XGLOB2D";
			}

			return "Images/AI/Globals/XGLOB3F";
		case MakeRoomView(kMars56, kEast):
			if (getCurrentActivation() == kActivateReactorReadyForNitrogen)
				return Common::String::format("Images/AI/Mars/XM57SD%d", hintNum);

			if (hintNum == 1) {
				if (GameState.isTakenItemID(kShieldBiochip))
					return "Images/AI/Mars/XM52NW";

				return "Images/AI/Globals/XGLOB2D";
			}

			return "Images/AI/Globals/XGLOB3F";
		}
	}

	return movieName;
}

bool Mars::inColorMatchingGame() {
	return _guessObject.isDisplaying();
}

bool Mars::canSolve() {
	return GameState.getCurrentRoomAndView() == MakeRoomView(kMars56, kEast) && (getCurrentActivation() == kActivateReactorReadyForNitrogen ||
			getCurrentActivation() == kActivateReactorReadyForCrowBar || inColorMatchingGame());
}

void Mars::doSolve() {
	if (getCurrentActivation() == kActivateReactorReadyForNitrogen || getCurrentActivation() == kActivateReactorReadyForCrowBar) {
		_utilityFuse.stopFuse();
		GameState.setMarsLockBroken(true);
		GameState.setMarsLockFrozen(false);
		startExtraLongSequence(kMars57OpenPanel, kMars57OpenPanelChoices, kExtraCompletedFlag, kFilterNoInput);
	} else if (inColorMatchingGame()) {
		_bombFuse.stopFuse();
		_guessObject.disposeReactorGuess();
		_undoPict.deallocateSurface();
		_guessHistory.disposeReactorHistory();
		_choiceHighlight.disposeReactorChoiceHighlight();
		startExtraSequence(kMars57GameSolved, kExtraCompletedFlag, kFilterNoInput);
	}
}

Common::String Mars::getSoundSpotsName() {
	return "Sounds/Mars/Mars Spots";
}

Common::String Mars::getNavMovieName() {
	return "Images/Mars/Mars.movie";
}

} // End of namespace Pegasus
