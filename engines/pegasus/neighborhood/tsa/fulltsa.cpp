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

#include "pegasus/cursor.h"
#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/aichip.h"
#include "pegasus/items/biochips/opticalchip.h"
#include "pegasus/neighborhood/caldoria/caldoria.h"
#include "pegasus/neighborhood/norad/alpha/noradalpha.h"
#include "pegasus/neighborhood/prehistoric/prehistoric.h"
#include "pegasus/neighborhood/mars/constants.h"
#include "pegasus/neighborhood/tsa/fulltsa.h"
#include "pegasus/neighborhood/wsc/wsc.h"

namespace Pegasus {

// TSA PICTs:

const tResIDType kTBPCloseBoxPICTID = 800;
const tResIDType kTBPRewindPICTID = 801;
const tResIDType kUnresolvedPICTID = 802;
const tResIDType kResolvedPICTID = 803;
const tResIDType kJumpMenuPICTID = 804;
const tResIDType kJumpMenuHilitedPICTID = 805;
const tResIDType kExitPICTID = 806;
const tResIDType kExitHilitedPICTID = 807;
const tResIDType kLeftRipPICTID = 808;
const tResIDType kComparisonCloseBoxPICTID = 809;
const tResIDType kComparisonLeftRewindPICTID = 810;
const tResIDType kComparisonRightRewindPICTID = 811;
const tResIDType kComparisonHiliteNoradPICTID = 812;
const tResIDType kComparisonHiliteMarsPICTID = 813;
const tResIDType kComparisonHiliteCaldoriaPICTID = 814;
const tResIDType kComparisonHiliteWSCPICTID = 815;
const tResIDType kComparisonChancesNoradPICTID = 816;
const tResIDType kComparisonChancesMarsPICTID = 817;
const tResIDType kComparisonChancesCaldoriaPICTID = 818;
const tResIDType kComparisonChancesWSCPICTID = 819;
const tResIDType kRedirectionCCRolloverPICTID = 820;
const tResIDType kRedirectionRRRolloverPICTID = 821;
const tResIDType kRedirectionFDRolloverPICTID = 822;
const tResIDType kRedirectionCCDoorPICTID = 823;
const tResIDType kRedirectionRRDoorPICTID = 824;
const tResIDType kRedirectionFDDoorPICTID = 825;
const tResIDType kRedirectionSecuredPICTID = 826;
const tResIDType kRedirectionNewTargetPICTID = 827;
const tResIDType kRedirectionClosePICTID = 828;

const short kCompassShift = 15;

const TimeScale kFullTSAMovieScale = 600;
const TimeScale kFullTSAFramesPerSecond = 15;
const TimeScale kFullTSAFrameDuration = 40;

//	Alternate IDs.

const tAlternateID kAltTSANormal = 0;
const tAlternateID kAltTSARobotsAtReadyRoom = 1;
const tAlternateID kAltTSARobotsAtFrontDoor = 2;
const tAlternateID kAltTSARedAlert = 3;

//	Room IDs.

const tRoomID kTSA01 = 1;
const tRoomID kTSA02 = 2;
const tRoomID kTSA03 = 3;
const tRoomID kTSA04 = 4;
const tRoomID kTSA05 = 5;
const tRoomID kTSA0A = 6;
const tRoomID kTSA06 = 7;
const tRoomID kTSA07 = 8;
const tRoomID kTSA08 = 9;
const tRoomID kTSA09 = 10;
const tRoomID kTSA10 = 11;
const tRoomID kTSA11 = 12;
const tRoomID kTSA12 = 13;
const tRoomID kTSA13 = 14;
const tRoomID kTSA14 = 15;
const tRoomID kTSA15 = 16;
const tRoomID kTSA16 = 17;
const tRoomID kTSA17 = 18;
const tRoomID kTSA18 = 19;
const tRoomID kTSA19 = 20;
const tRoomID kTSA0B = 21;
const tRoomID kTSA21Cyan = 22;
const tRoomID kTSA22Cyan = 23;
const tRoomID kTSA23Cyan = 24;
const tRoomID kTSA24Cyan = 25;
const tRoomID kTSA25Cyan = 26;
const tRoomID kTSA21Red = 27;
const tRoomID kTSA23Red = 29;
const tRoomID kTSA24Red = 30;
const tRoomID kTSA25Red = 31;
const tRoomID kTSA26 = 32;
const tRoomID kTSA27 = 33;
const tRoomID kTSA28 = 34;
const tRoomID kTSA29 = 35;
const tRoomID kTSA30 = 36;
const tRoomID kTSA31 = 37;
const tRoomID kTSA32 = 38;
const tRoomID kTSA33 = 39;
const tRoomID kTSA34 = 40;
const tRoomID kTSA35 = 41;
const tRoomID kTSADeathRoom = 43;

//	Hot Spot Activation IDs.

const tHotSpotActivationID kActivateTSAReadyForCard = 1;
const tHotSpotActivationID kActivateTSAReadyToTransport = 2;
const tHotSpotActivationID kActivateTSARobotsAwake = 3;
const tHotSpotActivationID kActivateTSA0BZoomedOut = 4;
const tHotSpotActivationID kActivateTSA0BZoomedIn = 5;
const tHotSpotActivationID kActivateTSA0BComparisonVideo = 6;
const tHotSpotActivationID kActivationLogReaderOpen = 7;
const tHotSpotActivationID kActivateTSA0BTBPVideo = 8;
const tHotSpotActivationID kActivationDoesntHaveKey = 9;
const tHotSpotActivationID kActivationKeyVaultOpen = 10;
const tHotSpotActivationID kActivationDoesntHaveChips = 11;
const tHotSpotActivationID kActivationChipVaultOpen = 12;
const tHotSpotActivationID kActivationJumpToPrehistoric = 13;
const tHotSpotActivationID kActivationJumpToNorad = 14;
const tHotSpotActivationID kActivationJumpToMars = 15;
const tHotSpotActivationID kActivationJumpToWSC = 16;
const tHotSpotActivationID kActivationReadyToExit = 17;
const tHotSpotActivationID kActivationReadyForJumpMenu = 18;
const tHotSpotActivationID kActivationMainJumpMenu = 19;

//	Hot Spot IDs.

const tHotSpotID kTSAGTCardDropSpotID = 5000;
const tHotSpotID kTSAGTTokyoSpotID = 5001;
const tHotSpotID kTSAGTCaldoriaSpotID = 5002;
const tHotSpotID kTSAGTBeachSpotID = 5003;
const tHotSpotID kTSAGTOtherSpotID = 5004;
const tHotSpotID kTSA02DoorSpotID = 5005;
const tHotSpotID kTSA03EastJimenezSpotID = 5006;
const tHotSpotID kTSA03WestCrenshawSpotID = 5007;
const tHotSpotID kTSA04EastMatsumotoSpotID = 5008;
const tHotSpotID kTSA04WestCastilleSpotID = 5009;
const tHotSpotID kTSA05EastSinclairSpotID = 5010;
const tHotSpotID kTSA05WestWhiteSpotID = 5011;
const tHotSpotID kTSA0AEastSpotID = 5012;
const tHotSpotID kTSA0AWastSpotID = 5013;
const tHotSpotID kTSA0BEastMonitorSpotID = 5014;
const tHotSpotID kTSA0BEastMonitorOutSpotID = 5015;
const tHotSpotID kTSA0BEastCompareNoradSpotID = 5016;
const tHotSpotID kTSA0BEastCompareMarsSpotID = 5017;
const tHotSpotID kTSA0BEastCompareCaldoriaSpotID = 5018;
const tHotSpotID kTSA0BEastCompareWSCSpotID = 5019;
const tHotSpotID kTSA0BEastLeftRewindSpotID = 5020;
const tHotSpotID kTSA0BEastLeftPlaySpotID = 5021;
const tHotSpotID kTSA0BEastRightRewindSpotID = 5022;
const tHotSpotID kTSA0BEastRightPlaySpotID = 5023;
const tHotSpotID kTSA0BEastCloseVideoSpotID = 5024;
const tHotSpotID kTSA0BNorthMonitorSpotID = 5025;
const tHotSpotID kTSA0BNorthMonitorOutSpotID = 5026;
const tHotSpotID kTSA0BNorthHistLogSpotID = 5027;
const tHotSpotID kTSA0BNorthRobotsToCommandCenterSpotID = 5028;
const tHotSpotID kTSA0BNorthRobotsToReadyRoomSpotID = 5029;
const tHotSpotID kTSA0BNorthRobotsToFrontDoorSpotID = 5030;
const tHotSpotID kTSA0BWestMonitorSpotID = 5031;
const tHotSpotID kTSA0BWestMonitorOutSpotID = 5032;
const tHotSpotID kTSA0BWestTheorySpotID = 5033;
const tHotSpotID kTSA0BWestBackgroundSpotID = 5034;
const tHotSpotID kTSA0BWestProcedureSpotID = 5035;
const tHotSpotID kTSA0BWestCloseVideoSpotID = 5036;
const tHotSpotID kTSA0BWestPlayVideoSpotID = 5037;
const tHotSpotID kTSA0BWestRewindVideoSpotID = 5038;
const tHotSpotID kTSA22EastMonitorSpotID = 5039;
const tHotSpotID kTSA22EastKeySpotID = 5040;
const tHotSpotID kTSA23WestMonitorSpotID = 5041;
const tHotSpotID kTSA23WestChipsSpotID = 5042;
const tHotSpotID kTSA34NorthDoorSpotID = 5043;
const tHotSpotID kTSA37NorthJumpToPrehistoricSpotID = 5044;
const tHotSpotID kTSA37NorthJumpToNoradSpotID = 5045;
const tHotSpotID kTSA37NorthCancelNoradSpotID = 5046;
const tHotSpotID kTSA37NorthJumpToMarsSpotID = 5047;
const tHotSpotID kTSA37NorthCancelMarsSpotID = 5048;
const tHotSpotID kTSA37NorthJumpToWSCSpotID = 5049;
const tHotSpotID kTSA37NorthCancelWSCSpotID = 5050;
const tHotSpotID kTSA37NorthExitSpotID = 5051;
const tHotSpotID kTSA37NorthJumpMenuSpotID = 5052;
const tHotSpotID kTSA37NorthNoradMenuSpotID = 5053;
const tHotSpotID kTSA37NorthMarsMenuSpotID = 5054;
const tHotSpotID kTSA37NorthWSCMenuSpotID = 5055;

//	Extra sequence IDs.

const tExtraID kTSATransporterArrowLoop = 0;
const tExtraID kTSAArriveFromCaldoria = 1;
const tExtraID kTSAGTOtherChoice = 2;
const tExtraID kTSAGTCardSwipe = 3;
const tExtraID kTSAGTSelectCaldoria = 4;
const tExtraID kTSAGTGoToCaldoria = 5;
const tExtraID kTSAGTSelectBeach = 6;
const tExtraID kTSAGTGoToBeach = 7;
const tExtraID kTSAGTArriveAtBeach = 8;
const tExtraID kTSAGTSelectTokyo = 9;
const tExtraID kTSAGTGoToTokyo = 10;
const tExtraID kTSAGTArriveAtTokyo = 11;
const tExtraID kTSA02NorthZoomIn = 12;
const tExtraID kTSA02NorthTenSecondDoor = 13;
const tExtraID kTSA02NorthZoomOut = 14;
const tExtraID kTSA02NorthDoorWithAgent3 = 15;
const tExtraID kTSA03JimenezZoomIn = 16;
const tExtraID kTSA03JimenezSpeech = 17;
const tExtraID kTSA03JimenezZoomOut = 18;
const tExtraID kTSA03CrenshawZoomIn = 19;
const tExtraID kTSA03CrenshawSpeech = 20;
const tExtraID kTSA03CrenshawZoomOut = 21;
const tExtraID kTSA03SouthRobotDeath = 22;
const tExtraID kTSA04NorthRobotGreeting = 23;
const tExtraID kTSA04MatsumotoZoomIn = 24;
const tExtraID kTSA04MatsumotoSpeech = 25;
const tExtraID kTSA04MatsumotoZoomOut = 26;
const tExtraID kTSA04CastilleZoomIn = 27;
const tExtraID kTSA04CastilleSpeech = 28;
const tExtraID kTSA04CastilleZoomOut = 29;
const tExtraID kTSA05SinclairZoomIn = 30;
const tExtraID kTSA05SinclairSpeech = 31;
const tExtraID kTSA05SinclairZoomOut = 32;
const tExtraID kTSA05WhiteZoomIn = 33;
const tExtraID kTSA05WhiteSpeech = 34;
const tExtraID kTSA05WhiteZoomOut = 35;
const tExtraID kTSA0AEastRobot = 36;
const tExtraID kTSA0AWestRobot = 37;
const tExtraID kTSA16NorthRobotDeath = 38;
const tExtraID kTSA0BEastZoomIn = 39;
const tExtraID kTSA0BEastZoomedView = 40;
const tExtraID kTSA0BEastZoomOut = 41;
const tExtraID kTSA0BEastTurnLeft = 42;
const tExtraID kTSA0BComparisonStartup = 43;
const tExtraID kTSA0BComparisonView0000 = 44;
const tExtraID kTSA0BComparisonView0002 = 45;
const tExtraID kTSA0BComparisonView0020 = 46;
const tExtraID kTSA0BComparisonView0022 = 47;
const tExtraID kTSA0BComparisonView0200 = 48;
const tExtraID kTSA0BComparisonView0202 = 49;
const tExtraID kTSA0BComparisonView0220 = 50;
const tExtraID kTSA0BComparisonView0222 = 51;
const tExtraID kTSA0BComparisonView2000 = 52;
const tExtraID kTSA0BComparisonView2002 = 53;
const tExtraID kTSA0BComparisonView2020 = 54;
const tExtraID kTSA0BComparisonView2022 = 55;
const tExtraID kTSA0BComparisonView2200 = 56;
const tExtraID kTSA0BComparisonView2202 = 57;
const tExtraID kTSA0BComparisonView2220 = 58;
const tExtraID kTSA0BComparisonView2222 = 59;
const tExtraID kTSA0BNoradComparisonView = 60;
const tExtraID kTSA0BNoradUnaltered = 61;
const tExtraID kTSA0BNoradAltered = 62;
const tExtraID kTSA0BMarsComparisonView = 63;
const tExtraID kTSA0BMarsUnaltered = 64;
const tExtraID kTSA0BMarsAltered = 65;
const tExtraID kTSA0BWSCComparisonView = 66;
const tExtraID kTSA0BWSCUnaltered = 67;
const tExtraID kTSA0BWSCAltered = 68;
const tExtraID kTSA0BCaldoriaComparisonView = 69;
const tExtraID kTSA0BCaldoriaUnaltered = 70;
const tExtraID kTSA0BCaldoriaAltered = 71;
const tExtraID kTSA0BNorthZoomIn = 72;
const tExtraID kTSA0BNorthZoomedView = 73;
const tExtraID kTSA0BNorthZoomOut = 74;
const tExtraID kTSA0BNorthTurnLeft = 75;
const tExtraID kTSA0BNorthTurnRight = 76;
const tExtraID kTSA0BNorthHistLogOpen = 77;
const tExtraID kTSA0BNorthHistLogClose = 78;
const tExtraID kTSA0BNorthHistLogCloseWithLog = 79;
const tExtraID kTSA0BNorthCantChangeHistory = 80;
const tExtraID kTSA0BNorthYoureBusted = 81;
const tExtraID kTSA0BNorthFinallyHappened = 82;
const tExtraID kTSA0BShowRip1 = 83;
const tExtraID kTSA0BNorthRipView1 = 84;
const tExtraID kTSA0BShowRip2 = 85;
const tExtraID kTSA0BShowGuardRobots = 86;
const tExtraID kTSA0BAIInterruption = 87;
const tExtraID kTSA0BRobotsToCommandCenter = 88;
const tExtraID kTSA0BNorthRobotsAtCCView = 89;
const tExtraID kTSA0BNorthRobotsAtRRView = 90;
const tExtraID kTSA0BNorthRobotsAtFDView = 91;
const tExtraID kTSA0BRobotsFromCommandCenterToReadyRoom = 92;
const tExtraID kTSA0BRobotsFromReadyRoomToCommandCenter = 93;
const tExtraID kTSA0BRobotsFromCommandCenterToFrontDoor = 94;
const tExtraID kTSA0BRobotsFromFrontDoorToCommandCenter = 95;
const tExtraID kTSA0BRobotsFromFrontDoorToReadyRoom = 96;
const tExtraID kTSA0BRobotsFromReadyRoomToFrontDoor = 97;
const tExtraID kTSA0BWestZoomIn = 98;
const tExtraID kTSA0BWestZoomedView = 99;
const tExtraID kTSA0BWestZoomOut = 100;
const tExtraID kTSA0BWestTurnRight = 101;
const tExtraID kTSA0BTBPTheoryHighlight = 102;
const tExtraID kTSA0BTBPBackgroundHighlight = 103;
const tExtraID kTSA0BTBPProcedureHighlight = 104;
const tExtraID kTSA0BTBPTheory = 105;
const tExtraID kTSA0BTBPBackground = 106;
const tExtraID kTSA0BTBPProcedure = 107;
const tExtraID kTSA0BRipAlarmScreen = 108;
const tExtraID kTSA22RedEastZoomInSequence = 109;
const tExtraID kTSA22RedEastVaultViewWithKey = 110;
const tExtraID kTSA22RedEastVaultViewNoKey = 111;
const tExtraID kTSA23RedWestVaultZoomInSequence = 112;
const tExtraID kTSA23RedWestVaultViewWithChips = 113;
const tExtraID kTSA23RedWestVaultViewNoChips = 114;
const tExtraID kTSA25NorthDeniedNoKey = 115;
const tExtraID kTSA25NorthDeniedNoChip = 116;
const tExtraID kTSA25NorthPutOnSuit = 117;
const tExtraID kTSA25NorthAlreadyHaveSuit = 118;
const tExtraID kTSA25NorthDescending1 = 119;
const tExtraID kTSA25NorthDescending2 = 120;
const tExtraID kTSA37HorseToAI1 = 121;
const tExtraID kTSA37PegasusAI1 = 122;
const tExtraID kTSA37AI1ToCommissioner1 = 123;
const tExtraID kTSA37Commissioner1 = 124;
const tExtraID kTSA37Commissioner1ToZoom = 125;
const tExtraID kTSA37ZoomToPrehistoric = 126;
const tExtraID kTSA37PrehistoricToAI2 = 127;
const tExtraID kTSA37PegasusAI2 = 128;
const tExtraID kTSA37AI2ToPrehistoric = 129;
const tExtraID kTSA37PrehistoricToDepart = 130;
const tExtraID kTSA37PegasusDepart = 131;
const tExtraID kTSA37TimeJumpToPegasus = 132;
const tExtraID kTSA37RecallToDownload = 133;
const tExtraID kTSA37DownloadToColonel1 = 134;
const tExtraID kTSA37Colonel1 = 135;
const tExtraID kTSA37Colonel1ToReviewRequired = 136;
const tExtraID kTSA37ReviewRequiredToExit = 137;
const tExtraID kTSA37ExitHilited = 138;
const tExtraID kTSA37ExitToHorse = 139;
const tExtraID kTSA37HorseToColonel2 = 140;
const tExtraID kTSA37Colonel2 = 141;
const tExtraID kTSA37PegasusAI3 = 142;
const tExtraID kTSA37AI3ToHorse = 143;
const tExtraID kTSA37HorseToZoom = 144;
const tExtraID kTSA37ZoomToMainMenu = 145;
const tExtraID kTSA37MainMenuToAI4 = 146;
const tExtraID kTSA37PegasusAI4 = 147;
const tExtraID kTSA37AI4ToMainMenu = 148;
const tExtraID kTSA37JumpMenu000 = 149;
const tExtraID kTSA37JumpMenu001 = 150;
const tExtraID kTSA37JumpMenu010 = 151;
const tExtraID kTSA37JumpMenu011 = 152;
const tExtraID kTSA37JumpMenu100 = 153;
const tExtraID kTSA37JumpMenu101 = 154;
const tExtraID kTSA37JumpMenu110 = 155;
const tExtraID kTSA37JumpMenu111 = 156;
const tExtraID kTSA37JumpToWSCMenu = 157;
const tExtraID kTSA37CancelWSC = 158;
const tExtraID kTSA37JumpToWSC = 159;
const tExtraID kTSA37WSCToAI5 = 160;
const tExtraID kTSA37PegasusAI5 = 161;
const tExtraID kTSA37AI5ToWSC = 162;
const tExtraID kTSA37WSCToDepart = 163;
const tExtraID kTSA37JumpToMarsMenu = 164;
const tExtraID kTSA37CancelMars = 165;
const tExtraID kTSA37JumpToMars = 166;
const tExtraID kTSA37MarsToAI6 = 167;
const tExtraID kTSA37PegasusAI6 = 168;
const tExtraID kTSA37AI6ToMars = 169;
const tExtraID kTSA37MarsToDepart = 170;
const tExtraID kTSA37JumpToNoradMenu = 171;
const tExtraID kTSA37CancelNorad = 172;
const tExtraID kTSA37JumpToNorad = 173;
const tExtraID kTSA37NoradToAI7 = 174;
const tExtraID kTSA37PegasusAI7 = 175;
const tExtraID kTSA37AI7ToNorad = 176;
const tExtraID kTSA37NoradToDepart = 177;
const tExtraID kTSA37EnvironmentalScan = 178;
const tExtraID kTSA37DownloadToMainMenu = 179;
const tExtraID kTSA37DownloadToOpMemReview = 180;
const tExtraID kTSA37OpMemReviewToMainMenu = 181;
const tExtraID kTSA37OpMemReviewToAllClear = 182;
const tExtraID kTSA37AllClearToCongratulations = 183;
const tExtraID kTSA37Congratulations = 184;
const tExtraID kTSA37CongratulationsToExit = 185;

const tDisplayOrder kRipTimerOrder = kMonitorLayer;


const tCoordType kUnresolvedLeft = kNavAreaLeft + 14;
const tCoordType kUnresolvedTop = kNavAreaTop + 236;

const tCoordType kResolvedLeft = kNavAreaLeft + 36;
const tCoordType kResolvedTop = kNavAreaTop + 236;

const tCoordType kJumpMenuLeft = kNavAreaLeft + 360;
const tCoordType kJumpMenuTop = kNavAreaTop + 202;

const tCoordType kJumpMenuHilitedLeft = kNavAreaLeft + 354;
const tCoordType kJumpMenuHilitedTop = kNavAreaTop + 196;

const tCoordType kExitLeft = kNavAreaLeft + 360;
const tCoordType kExitTop = kNavAreaTop + 216;

const tCoordType kExitHilitedLeft = kNavAreaLeft + 354;
const tCoordType kExitHilitedTop = kNavAreaTop + 210;

const tCoordType kRipTimerLeft = kNavAreaLeft + 95;
const tCoordType kRipTimerTop = kNavAreaTop + 87;

const tCoordType kTBPCloseLeft = kNavAreaLeft + 30;
const tCoordType kTBPCloseTop = kNavAreaTop + 16;

const tCoordType kTBPRewindLeft = kNavAreaLeft + 86;
const tCoordType kTBPRewindTop = kNavAreaTop + 218;

const tCoordType kComparisonCloseLeft = kNavAreaLeft + 50;
const tCoordType kComparisonCloseTop = kNavAreaTop + 14;

const tCoordType kComparisonLeftRewindLeft = kNavAreaLeft + 96;
const tCoordType kComparisonLeftRewindTop = kNavAreaTop + 190;

const tCoordType kComparisonRightRewindLeft = kNavAreaLeft + 282;
const tCoordType kComparisonRightRewindTop = kNavAreaTop + 190;

const tCoordType kComparisonHiliteSpriteLeft = kNavAreaLeft + 45;
const tCoordType kComparisonHiliteSpriteTop = kNavAreaTop + 65;

const tCoordType kComparisonHiliteNoradLeft = kNavAreaLeft + 45;
const tCoordType kComparisonHiliteNoradTop = kNavAreaTop + 65;

const tCoordType kComparisonHiliteMarsLeft = kNavAreaLeft + 45 + 4;
const tCoordType kComparisonHiliteMarsTop = kNavAreaTop + 65 + 23;

const tCoordType kComparisonHiliteCaldoriaLeft = kNavAreaLeft + 45 + 7;
const tCoordType kComparisonHiliteCaldoriaTop = kNavAreaTop + 65 + 46;

const tCoordType kComparisonHiliteWSCLeft = kNavAreaLeft + 45 + 11;
const tCoordType kComparisonHiliteWSCTop = kNavAreaTop + 65 + 68;

const tCoordType kComparisonChancesSpriteLeft = kNavAreaLeft + 148;
const tCoordType kComparisonChancesSpriteTop = kNavAreaTop + 162;

const tCoordType kComparisonChancesNoradLeft = kNavAreaLeft + 148;
const tCoordType kComparisonChancesNoradTop = kNavAreaTop + 162;

const tCoordType kComparisonChancesMarsLeft = kNavAreaLeft + 148;
const tCoordType kComparisonChancesMarsTop = kNavAreaTop + 162;

const tCoordType kComparisonChancesCaldoriaLeft = kNavAreaLeft + 148;
const tCoordType kComparisonChancesCaldoriaTop = kNavAreaTop + 162 + 1;

const tCoordType kComparisonChancesWSCLeft = kNavAreaLeft + 148;
const tCoordType kComparisonChancesWSCTop = kNavAreaTop + 162;

const tCoordType kRedirectionSprite1Left = kNavAreaLeft + 58;
const tCoordType kRedirectionSprite1Top = kNavAreaTop + 16;

const tCoordType kRedirectionSprite2Left = kNavAreaLeft + 36;
const tCoordType kRedirectionSprite2Top = kNavAreaTop + 166;

const tCoordType kRedirectionCCRolloverLeft = kNavAreaLeft + 58;
const tCoordType kRedirectionCCRolloverTop = kNavAreaTop + 16;

const tCoordType kRedirectionRRRolloverLeft = kNavAreaLeft + 430;
const tCoordType kRedirectionRRRolloverTop = kNavAreaTop + 30;

const tCoordType kRedirectionFDRolloverLeft = kNavAreaLeft + 278;
const tCoordType kRedirectionFDRolloverTop = kNavAreaTop + 160;

const tCoordType kRedirectionCCDoorLeft = kNavAreaLeft + 174;
const tCoordType kRedirectionCCDoorTop = kNavAreaTop + 36;

const tCoordType kRedirectionRRDoorLeft = kNavAreaLeft + 418;
const tCoordType kRedirectionRRDoorTop = kNavAreaTop + 32;

const tCoordType kRedirectionFDDoorLeft = kNavAreaLeft + 298;
const tCoordType kRedirectionFDDoorTop = kNavAreaTop + 240;

const tCoordType kRedirectionSecuredLeft = kNavAreaLeft + 36;
const tCoordType kRedirectionSecuredTop = kNavAreaTop + 166;

const tCoordType kRedirectionNewTargetLeft = kNavAreaLeft + 36;
const tCoordType kRedirectionNewTargetTop = kNavAreaTop + 166;

const tCoordType kRedirectionCloseLeft = kNavAreaLeft + 56;
const tCoordType kRedirectionCloseTop = kNavAreaTop + 220;

const TimeValue kTSABumpIntoWallIn = 0;
const TimeValue kTSABumpIntoWallOut = 148;

const TimeValue kTSAGTDoorCloseIn = 148;
const TimeValue kTSAGTDoorCloseOut = 1570;

const TimeValue kTSANoOtherDestinationIn = 1570;
const TimeValue kTSANoOtherDestinationOut = 3601;

const TimeValue kTSAEntryDoorCloseIn = 3601;
const TimeValue kTSAEntryDoorCloseOut = 4200;

const TimeValue kTSAInsideDoorCloseIn = 4200;
const TimeValue kTSAInsideDoorCloseOut = 4800;

const TimeValue kTSAVaultCloseIn = 4800;
const TimeValue kTSAVaultCloseOut = 5388;

const TimeValue kTSAPegasusDoorCloseIn = 5388;
const TimeValue kTSAPegasusDoorCloseOut = 6457;

const bool kPegasusUnresolved = false;
const bool kPegasusResolved = true;
const bool kPegasusCantExit = false;
const bool kPegasusCanExit = true;

// Monitor modes
enum {
	kMonitorNeutral = 0,
	kMonitorTheory = 1,
	kMonitorProcedure = 2,
	kMonitorBackground = 3,
	kMonitorNoradComparison = 4,
	kMonitorMarsComparison = 5,
	kMonitorCaldoriaComparison = 6,
	kMonitorWSCComparison = 7,
	
	kRawModeMask = 0x0F,
	kPlayingTBPMask = 0x10,
	kPlayingLeftComparisonMask = 0x20,
	kPlayingRightComparisonMask = 0x40,
	
	kPlayingAnyMask = kPlayingTBPMask |
						kPlayingLeftComparisonMask |
						kPlayingRightComparisonMask,
	
	kMonitorPlayingTheory = kMonitorTheory | kPlayingTBPMask,
	kMonitorPlayingProcedure = kMonitorProcedure | kPlayingTBPMask,
	kMonitorPlayingBackground = kMonitorBackground | kPlayingTBPMask,
	
	kMonitorPlayingLeftNoradComparison = kMonitorNoradComparison |
											kPlayingLeftComparisonMask,
	kMonitorPlayingRightNoradComparison = kMonitorNoradComparison |
											kPlayingRightComparisonMask,
	kMonitorPlayingLeftMarsComparison = kMonitorMarsComparison |
										kPlayingLeftComparisonMask,
	kMonitorPlayingRightMarsComparison = kMonitorMarsComparison |
											kPlayingRightComparisonMask,
	kMonitorPlayingLeftCaldoriaComparison = kMonitorCaldoriaComparison |
											kPlayingLeftComparisonMask,
	kMonitorPlayingRightCaldoriaComparison = kMonitorCaldoriaComparison |
												kPlayingRightComparisonMask,
	kMonitorPlayingLeftWSCComparison = kMonitorWSCComparison |
										kPlayingLeftComparisonMask,
	kMonitorPlayingRightWSCComparison = kMonitorWSCComparison |
										kPlayingRightComparisonMask
};

static const tExtraID s_historicalLogViews[16] = {
	kTSA0BComparisonView0000,
	kTSA0BComparisonView0002,
	kTSA0BComparisonView0020,
	kTSA0BComparisonView0022,
	kTSA0BComparisonView0200,
	kTSA0BComparisonView0202,
	kTSA0BComparisonView0220,
	kTSA0BComparisonView0222,
	kTSA0BComparisonView2000,
	kTSA0BComparisonView2002,
	kTSA0BComparisonView2020,
	kTSA0BComparisonView2022,
	kTSA0BComparisonView2200,
	kTSA0BComparisonView2202,
	kTSA0BComparisonView2220,
	kTSA0BComparisonView2222
};

const long kRedirectionCCRolloverSprite = 0;
const long kRedirectionRRRolloverSprite = 1;
const long kRedirectionFDRolloverSprite = 2;
const long kRedirectionCCDoorSprite = 3;
const long kRedirectionRRDoorSprite = 4;
const long kRedirectionFDDoorSprite = 5;
const long kRedirectionCloseSprite = 6;
const long kRedirectionSecuredSprite = 0;
const long kRedirectionNewTargetSprite = 1;

void RipTimer::initImage() {
	_middle = -1;
	
	_timerImage.getImageFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kLeftRipPICTID);

	Common::Rect r;
	_timerImage.getSurfaceBounds(r);
	setBounds(r);
}

void RipTimer::releaseImage() {
	_timerImage.deallocateSurface();
}

void RipTimer::draw(const Common::Rect &updateRect) {
	Common::Rect bounds;
	getBounds(bounds);

	Common::Rect r1 = bounds;
	r1.right = _middle;
	r1 = updateRect.findIntersectingRect(r1);

	if (!r1.isEmpty()) {
		Common::Rect r2 = r1;
		r2.moveTo(r1.left - _bounds.left, r1.top - bounds.top);
		_timerImage.copyToCurrentPort(r2, r1);
	}
}

void RipTimer::timeChanged(const TimeValue newTime) {
	Common::Rect bounds;
	getBounds(bounds);

	tCoordType newMiddle = bounds.left + bounds.width() * newTime / getDuration();

	if (newMiddle != _middle) {
		_middle = newMiddle;
		triggerRedraw();
	}

	if (newTime == getStop())
		((PegasusEngine *)g_engine)->die(kDeathUncreatedInTSA);
}

FullTSA::FullTSA(InputHandler *nextHandler, PegasusEngine *owner) : Neighborhood(nextHandler, owner, "Full TSA", kFullTSAID),
		_ripTimer(kNoDisplayElement), _sprite1(kNoDisplayElement), _sprite2(kNoDisplayElement), _sprite3(kNoDisplayElement) {
	setIsItemTaken(kJourneymanKey);
	setIsItemTaken(kPegasusBiochip);
	setIsItemTaken(kMapBiochip);
}

void FullTSA::init() {	
	Neighborhood::init();
	_ripTimer.setDisplayOrder(kRipTimerOrder);
	_ripTimer.startDisplaying();

	if (!GameState.getTSASeenRobotGreeting())
		forceStridingStop(kTSA03, kNorth, kNoAlternateID);

	_sprite1.setDisplayOrder(kMonitorLayer);
	_sprite1.startDisplaying();
	_sprite2.setDisplayOrder(kMonitorLayer);
	_sprite2.startDisplaying();
	_sprite3.setDisplayOrder(kMonitorLayer);
	_sprite3.startDisplaying();
	
	// Fix a mistake in the world builder tables.
	HotspotInfoTable::Entry *entry = findHotspotEntry(kTSA23WestChipsSpotID);
	entry->hotspotItem = kPegasusBiochip;
}

void uncreatedInTSAFunction(FunctionPtr *, void *tsa) {
	((FullTSA *)tsa)->die(kDeathUncreatedInTSA);
}

void FullTSA::start() {
	g_energyMonitor->stopEnergyDraining();

	if (!GameState.getScoringEnterTSA()) {
		_utilityFuse.primeFuse(GameState.getTSAFuseTimeLimit());
		_utilityFuse.setFunctionPtr(&uncreatedInTSAFunction, (void *)this);
		_utilityFuse.lightFuse();
	} else if (GameState.getTSAState() == kTSAPlayerDetectedRip || GameState.getTSAState() == kTSAPlayerNeedsHistoricalLog) {
		_ripTimer.initImage();
		_ripTimer.moveElementTo(kRipTimerLeft, kRipTimerTop);
		_ripTimer.setSegment(0, kRipTimeLimit, kRipTimeScale);
		_ripTimer.setTime(GameState.getRipTimerTime());
		_ripTimer.start();
	}

	Neighborhood::start();
}

void FullTSA::flushGameState() {
	GameState.setRipTimerTime(_ripTimer.getTime());
	GameState.setTSAFuseTimeLimit(_utilityFuse.getTimeRemaining());
}

Common::String FullTSA::getBriefingMovie() {	
	Common::String movieName = Neighborhood::getBriefingMovie();

	if (movieName.empty()) {
		tRoomID room = GameState.getCurrentRoom();

		switch (GameState.getTSAState()) {
		case kTSAPlayerNotArrived:
		case kTSAPlayerForcedReview:
			if (room >= kTSA16 && room <= kTSA0B)
				return "Images/AI/TSA/XT01A";

			return "Images/AI/TSA/XT01";
		case kTSAPlayerDetectedRip:
		case kTSAPlayerNeedsHistoricalLog:
			return "Images/AI/TSA/XT02";
		case kTSAPlayerGotHistoricalLog:
		case kTSAPlayerInstalledHistoricalLog:
			return "Images/AI/TSA/XT03";
		default:
			switch (getCurrentActivation()) {
			case kActivationJumpToPrehistoric:
				g_AIChip->showBriefingClicked();
				startExtraSequenceSync(kTSA37PegasusAI2, kHintInterruption);
				startExtraSequenceSync(kTSA37AI2ToPrehistoric, kFilterNoInput);
				g_AIChip->clearClicked();
				break;
			case kActivationJumpToNorad:
				g_AIChip->showBriefingClicked();
				startExtraSequenceSync(kTSA37PegasusAI7, kHintInterruption);
				startExtraSequenceSync(kTSA37AI7ToNorad, kFilterNoInput);
				g_AIChip->clearClicked();
				break;
			case kActivationJumpToMars:
				g_AIChip->showBriefingClicked();
				startExtraSequenceSync(kTSA37PegasusAI6, kHintInterruption);
				startExtraSequenceSync(kTSA37AI6ToMars, kFilterNoInput);
				g_AIChip->clearClicked();
				break;
			case kActivationJumpToWSC:
				g_AIChip->showBriefingClicked();
				startExtraSequenceSync(kTSA37PegasusAI5, kHintInterruption);
				startExtraSequenceSync(kTSA37AI5ToWSC, kFilterNoInput);
				g_AIChip->clearClicked();
				break;
			default:
				if (GameState.allTimeZonesFinished())
					return "Images/AI/TSA/XT05";

				return "Images/AI/TSA/XT04";
			}
			break;
		}
	}

	return movieName;
}

Common::String FullTSA::getEnvScanMovie() {
	Common::String movieName = Neighborhood::getEnvScanMovie();

	if (movieName.empty()) {
		switch (GameState.getTSAState()) {
		case kTSAPlayerNotArrived:
		case kTSAPlayerForcedReview:
		case kTSAPlayerDetectedRip:
		case kTSAPlayerNeedsHistoricalLog:
			return "Images/AI/TSA/XTE1";
		default:
			if (GameState.getCurrentRoom() == kTSA37) {
				g_AIChip->showEnvScanClicked();
				startExtraSequenceSync(kTSA37EnvironmentalScan, kHintInterruption);
	
				switch (getCurrentActivation()) {
				case kActivationJumpToPrehistoric:
					startExtraSequenceSync(kTSA37AI2ToPrehistoric, kFilterNoInput);
					break;
				case kActivationJumpToNorad:
					startExtraSequenceSync(kTSA37AI7ToNorad, kFilterNoInput);
					showExtraView(kTSA37JumpToNoradMenu);
					break;
				case kActivationJumpToMars:
					startExtraSequenceSync(kTSA37AI6ToMars, kFilterNoInput);
					showExtraView(kTSA37JumpToMarsMenu);
					break;
				case kActivationJumpToWSC:
					startExtraSequenceSync(kTSA37AI5ToWSC, kFilterNoInput);
					showExtraView(kTSA37JumpToWSCMenu);
					break;
				default:
					startExtraSequenceSync(kTSA37AI4ToMainMenu, kFilterNoInput);
					break;
				}

				g_AIChip->clearClicked();
			} else if (GameState.allTimeZonesFinished()) {
				return "Images/AI/TSA/XTE1";
			} else {
				return "Images/AI/TSA/XTE2";
			}
			break;
		}
	}

	return movieName;
}

uint FullTSA::getNumHints() {
	uint numHints = Neighborhood::getNumHints();

	if (numHints == 0) {
		switch (GameState.getTSAState()) {
		case kRobotsAtCommandCenter:
		case kRobotsAtFrontDoor:
		case kRobotsAtReadyRoom:
			if (GameState.getCurrentRoom() == kTSA0B && GameState.getTSA0BZoomedIn())
				numHints = 3;
			break;
		}
	}

	return numHints;
}

Common::String FullTSA::getHintMovie(uint hintNum) {
	Common::String movieName = Neighborhood::getHintMovie(hintNum);

	if (movieName.empty())
		movieName = Common::String::format("Images/AI/TSA/XT20NH%d", hintNum);
		
	return movieName;
}

void FullTSA::loadAmbientLoops() {
	tRoomID room = GameState.getCurrentRoom();

	switch (GameState.getTSAState()) {
	case kTSAPlayerDetectedRip:
	case kTSAPlayerNeedsHistoricalLog:
		if ((room >= kTSA16 && room <= kTSA0B) || (room >= kTSA21Cyan && room <= kTSA24Cyan) || (room >= kTSA21Red && room <= kTSA24Red))
			loadLoopSound1("Sounds/TSA/TSA CLAXON.22K.AIFF", 0x100 / 4, 0, 0);
		else if (room == kTSA25Cyan || room == kTSA25Red)
			loadLoopSound1("Sounds/TSA/TSA CLAXON.22K.AIFF", 0x100 / 6, 0, 0);
		else
			loadLoopSound1("Sounds/TSA/TSA EchoClaxon.22K.AIFF", 0x100 / 4, 0, 0);
		break;
	default:
		if (room >= kTSA00 && room <= kTSA02)
			loadLoopSound1("Sounds/TSA/T01NAE.NEW.22K.AIFF");
		else if (room >= kTSA03 && room <= kTSA15)
			loadLoopSound1("Sounds/TSA/T01NAE.NEW.22K.AIFF");
		else if (room >= kTSA16 && room <= kTSA0B)
			loadLoopSound1("Sounds/TSA/T14SAEO1.22K.AIFF");
		else if (room >= kTSA21Cyan && room <= kTSA25Red)
			loadLoopSound1("Sounds/TSA/T15SAE01.22K.AIFF");
		else if (room >= kTSA26 && room <= kTSA37)
			loadLoopSound1("Sounds/TSA/T01NAE.NEW.22K.AIFF");
		break;
	}
}

short FullTSA::getStaticCompassAngle(const tRoomID room, const tDirectionConstant dir) {
	int16 result = Neighborhood::getStaticCompassAngle(room, dir);

	switch (room) {
	case kTSA08:
		result += kCompassShift;
		break;
	case kTSA09:
		result -= kCompassShift;
		break;
	case kTSA10:
		result += kCompassShift * 2;
		break;
	case kTSA11:
	case kTSA22Cyan:
	case kTSA22Red:
		result -= kCompassShift * 2;
		break;
	case kTSA12:
		result += kCompassShift * 3;
		break;
	case kTSA13:
		result -= kCompassShift * 3;
		break;
	case kTSA14:
	case kTSA16:
	case kTSA17:
	case kTSA18:
	case kTSA19:
		result += kCompassShift * 4;
		break;
	case kTSA0B:
		result += kCompassShift * 4;

		if (dir == kWest)
			result += 30;
		else if (dir == kEast)
			result -= 30;
		break;
	case kTSA33:
		result += kCompassShift * 4;
		break;
	case kTSA15:
	case kTSA21Cyan:
	case kTSA24Cyan:
	case kTSA25Cyan:
	case kTSA21Red:
	case kTSA24Red:
	case kTSA25Red:
	case kTSA26:
	case kTSA27:
	case kTSA28:
	case kTSA29:
	case kTSA30:
		result -= kCompassShift * 4;
		break;
	case kTSA23Cyan:
	case kTSA23Red:
		result -= kCompassShift * 6;
		break;
	case kTSA32:
		result -= kCompassShift * 8;
		break;
	case kTSA34:
		result -= kCompassShift * 12;
		break;
	case kTSA35:
		result += kCompassShift * 8;
		break;
	case kTSA37:
		result -= kCompassShift * 2;
		break;
	}

	return result;
}

void FullTSA::getExitCompassMove(const ExitTable::Entry &exitEntry, FaderMoveSpec &compassMove) {
	Neighborhood::getExitCompassMove(exitEntry, compassMove);

	switch (MakeRoomView(exitEntry.room, exitEntry.direction)) {
	case MakeRoomView(kTSA01, kSouth):
		compassMove.insertFaderKnot(exitEntry.movieStart, -180);
		compassMove.insertFaderKnot(exitEntry.movieStart + kFullTSAFrameDuration * 3, -180);
		compassMove.insertFaderKnot(exitEntry.movieStart + kFullTSAFrameDuration * 33,
				getStaticCompassAngle(exitEntry.exitRoom, exitEntry.exitDirection));
		break;
	case MakeRoomView(kTSA11, kEast):
		if (getCurrentAlternate() == kAltTSARobotsAtReadyRoom) {
			compassMove.makeTwoKnotFaderSpec(kFullTSAMovieScale, exitEntry.movieStart,
					getStaticCompassAngle(kTSA11, kEast), exitEntry.movieEnd,
					getStaticCompassAngle(kTSA13, kEast));
			compassMove.insertFaderKnot(exitEntry.movieStart + kFullTSAFrameDuration * 13, compassMove.getNthKnotValue(1));
		}
		break;
	case MakeRoomView(kTSA34, kNorth):
		compassMove.insertFaderKnot(exitEntry.movieStart + kFullTSAFrameDuration * 48,
				getStaticCompassAngle(exitEntry.room, exitEntry.direction));
		compassMove.insertFaderKnot(exitEntry.movieStart + kFullTSAFrameDuration * 68,
				getStaticCompassAngle(exitEntry.exitRoom, exitEntry.exitDirection));
		break;
	case MakeRoomView(kTSA37, kNorth):
		compassMove.insertFaderKnot(exitEntry.movieStart + kFullTSAFrameDuration * 38,
				getStaticCompassAngle(exitEntry.room, exitEntry.direction));
		compassMove.insertFaderKnot(exitEntry.movieStart + kFullTSAFrameDuration * 64,
				getStaticCompassAngle(exitEntry.room, exitEntry.direction) + kCompassShift * 3 / 2);
		compassMove.insertFaderKnot(exitEntry.movieStart + kFullTSAFrameDuration * 105,
				getStaticCompassAngle(exitEntry.exitRoom, exitEntry.exitDirection));
		break;
	}
}

void FullTSA::getExtraCompassMove(const ExtraTable::Entry &extraEntry, FaderMoveSpec &compassMove) {
	int16 angle;
	
	switch (extraEntry.extra) {
	case kTSA0BEastTurnLeft:
	case kTSA0BNorthTurnLeft:
		angle =getStaticCompassAngle(GameState.getCurrentRoom(), GameState.getCurrentDirection());
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), extraEntry.movieStart, angle,
				extraEntry.movieEnd, angle - 60);
		break;
	case kTSA0BNorthTurnRight:
	case kTSA0BWestTurnRight:
		angle = getStaticCompassAngle(GameState.getCurrentRoom(), GameState.getCurrentDirection());
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), extraEntry.movieStart, angle,
				extraEntry.movieEnd, angle + 60);
		break;
	case kTSA22RedEastZoomInSequence:
		angle = getStaticCompassAngle(GameState.getCurrentRoom(), GameState.getCurrentDirection());
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), extraEntry.movieStart, angle,
				extraEntry.movieEnd, angle);
		compassMove.insertFaderKnot(extraEntry.movieStart + 1200, angle - kCompassShift * 2);
		compassMove.insertFaderKnot(extraEntry.movieStart + 8160, angle - kCompassShift * 2);
		compassMove.insertFaderKnot(extraEntry.movieStart + 9840, angle);
		break;
	case kTSA23RedWestVaultZoomInSequence:
		angle = getStaticCompassAngle(GameState.getCurrentRoom(), GameState.getCurrentDirection());
		compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), extraEntry.movieStart, angle,
				extraEntry.movieEnd, angle);
		compassMove.insertFaderKnot(extraEntry.movieStart + 1200, angle - kCompassShift * 2);
		compassMove.insertFaderKnot(extraEntry.movieStart + 10100, angle - kCompassShift * 2);
		compassMove.insertFaderKnot(extraEntry.movieStart + 11880, angle);
		break;
	default:
		Neighborhood::getExtraCompassMove(extraEntry, compassMove);
		break;
	}
}

uint16 FullTSA::getDateResID() const {
	return kDate2318ID;
}

TimeValue FullTSA::getViewTime(const tRoomID room, const tDirectionConstant direction) {
	tExtraID extraID = 0xffffffff;
	
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kTSA0B, kEast):
		if (GameState.getTSA0BZoomedIn())
			switch (GameState.getTSAState()) {
			case kTSAPlayerInstalledHistoricalLog:
			case kTSABossSawHistoricalLog:
			case kRobotsAtCommandCenter:
			case kRobotsAtFrontDoor:
			case kRobotsAtReadyRoom:
				extraID = s_historicalLogViews[getHistoricalLogIndex()];
				break;
			default:
				extraID = kTSA0BEastZoomedView;
				break;
			}
		break;
	case MakeRoomView(kTSA0B, kNorth):
		if (GameState.getTSA0BZoomedIn())
			switch (GameState.getTSAState()) {
			case kTSAPlayerNeedsHistoricalLog:
				extraID = kTSA0BNorthRipView1;
				break;
			default:
				extraID = kTSA0BNorthZoomedView;
				break;
			}
		break;
	case MakeRoomView(kTSA0B, kWest):
		if (GameState.getTSA0BZoomedIn())
			extraID = kTSA0BWestZoomedView;
		break;
	case MakeRoomView(kTSA22Red, kEast):
		if (_privateFlags.getFlag(kTSAPrivateKeyVaultOpenFlag)) {
			if (_vm->itemInLocation(kJourneymanKey, kFullTSAID, kTSA22Red, kEast))
				extraID = kTSA22RedEastVaultViewWithKey;
			else
				extraID = kTSA22RedEastVaultViewNoKey;
		}
		break;
	case MakeRoomView(kTSA23Red, kWest):
		if (_privateFlags.getFlag(kTSAPrivateChipVaultOpenFlag)) {
			if (_vm->itemInLocation(kPegasusBiochip, kFullTSAID, kTSA23Red, kWest))
				extraID = kTSA23RedWestVaultViewWithChips;
			else
				extraID = kTSA23RedWestVaultViewNoChips;
		}
		break;
	case MakeRoomView(kTSA37, kNorth):
		switch (GameState.getTSAState()) {
		case kTSAPlayerGotHistoricalLog:
			extraID = kTSA37ReviewRequiredToExit;
			break;
		case kPlayerFinishedWithTSA:
			extraID = kTSA37CongratulationsToExit;
			break;
		default:
			extraID = kTSA37AI3ToHorse;
			break;
		}
		break;
	}

	if (extraID != 0xffffffff) {
		ExtraTable::Entry entry;
		getExtraEntry(extraID, entry);
		return entry.movieEnd - 1;
	}

	return Neighborhood::getViewTime(room, direction);
}

void FullTSA::findSpotEntry(const tRoomID room, const tDirectionConstant direction, tSpotFlags flags, SpotTable::Entry &entry) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kTSA0B, kNorth):
	case MakeRoomView(kTSA0B, kEast):
	case MakeRoomView(kTSA0B, kWest):
		if (!GameState.getTSA0BZoomedIn())
			Neighborhood::findSpotEntry(room, direction, flags, entry);
		break;
	default:
		Neighborhood::findSpotEntry(room, direction, flags, entry);
		break;
	}
}

void FullTSA::getExtraEntry(const uint32 id, ExtraTable::Entry &extraEntry) {
	Neighborhood::getExtraEntry(id, extraEntry);

	if (id == kTSA0BShowGuardRobots)
		extraEntry.movieStart += kFullTSAFrameDuration * 3;
}

void FullTSA::pickedUpItem(Item *item) {
	BiochipItem *biochip;
	
	switch (item->getObjectID()) {
	case kJourneymanKey:
		GameState.setScoringGotJourneymanKey(true);
		break;
	case kPegasusBiochip:
		biochip = (BiochipItem *)g_allItems.findItemByID(kMapBiochip);
		_vm->addItemToBiochips(biochip);
		GameState.setScoringGotPegasusBiochip(true);
		break;
	}
}

void FullTSA::playExtraMovie(const ExtraTable::Entry &extraEntry, const tNotificationFlags flags, const tInputBits interruptionInput) {
	switch (extraEntry.extra) {
	case kTSA0BNorthZoomIn:
		if (_privateFlags.getFlag(kTSAPrivateLogReaderOpenFlag)) {
			_privateFlags.setFlag(kTSAPrivateLogReaderOpenFlag, false);
			requestExtraSequence(kTSA0BNorthHistLogClose, 0, kFilterNoInput);
			requestExtraSequence(kTSA0BNorthZoomIn, kExtraCompletedFlag, kFilterNoInput);
		} else {
			Neighborhood::playExtraMovie(extraEntry, flags, interruptionInput);
		}
		break;
	case kTSA0BNorthZoomOut:
		if (_ripTimer.isVisible())
			_ripTimer.hide();

		shutDownRobotMonitor();
		Neighborhood::playExtraMovie(extraEntry, flags, interruptionInput);
		break;
	case kTSA0BEastZoomOut:
		shutDownComparisonMonitor();
		Neighborhood::playExtraMovie(extraEntry, flags, interruptionInput);
		break;
	default:
		Neighborhood::playExtraMovie(extraEntry, flags, interruptionInput);
		break;
	}
}

void FullTSA::startDoorOpenMovie(const TimeValue startTime, const TimeValue stopTime) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kTSA00, kNorth):
		if (GameState.getLastNeighborhood() != kFullTSAID) {
			startExtraSequence(kTSAArriveFromCaldoria, kDoorOpenCompletedFlag, kFilterNoInput);
			return;
		}
		break;
	case MakeRoomView(kTSA02, kNorth):
		if (!GameState.getTSAIDedAtDoor()) {
			GameState.setTSAIDedAtDoor(true);
			requestExtraSequence(kTSA02NorthZoomIn, 0, kFilterNoInput);
			requestExtraSequence(kTSA02NorthTenSecondDoor, 0, kFilterNoInput);

			if (GameState.getTSASeenAgent3AtDoor()) {
				requestExtraSequence(kTSA02NorthZoomOut, kExtraCompletedFlag, kFilterNoInput);
			} else {
				GameState.setTSASeenAgent3AtDoor(true);
				requestExtraSequence(kTSA02NorthZoomOut, 0, kFilterNoInput);
				requestExtraSequence(kTSA02NorthDoorWithAgent3, kDoorOpenCompletedFlag, kFilterNoInput);
			}
			return;
		}
		break;
	case MakeRoomView(kTSA03, kSouth):
		if (GameState.getTSAState() == kRobotsAtFrontDoor) {
			playDeathExtra(kTSA03SouthRobotDeath, kDeathShotByTSARobots);
			return;
		}
		break;
	case MakeRoomView(kTSA16, kNorth):
		if (GameState.getTSAState() == kRobotsAtCommandCenter) {
			playDeathExtra(kTSA16NorthRobotDeath, kDeathShotByTSARobots);
			return;
		}
		break;
	}

	Neighborhood::startDoorOpenMovie(startTime, stopTime);
}

tInputBits FullTSA::getInputFilter() {
	tInputBits result = Neighborhood::getInputFilter();

	switch (GameState.getCurrentRoom()) {
	case kTSA0B:
		if (GameState.getT0BMonitorMode() != kMonitorNeutral)
			// Only allow a click.
			result &= JMPPPInput::getClickInputFilter();
		break;
	case kTSA37:
		// Can't move forward in Pegasus. Only press the exit button.
		result &= ~(kFilterUpButton | kFilterUpAuto);
		break;
	}

	return result;
}

void FullTSA::turnLeft() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kTSA15, kNorth):
		if (GameState.getTSAState() == kTSAPlayerNeedsHistoricalLog)
			setCurrentAlternate(kAltTSANormal);
		break;
	case MakeRoomView(kTSA0B, kNorth):
		if (_ripTimer.isVisible())
			_ripTimer.hide();
		releaseSprites();
		break;
	case MakeRoomView(kTSA0B, kEast):
		shutDownComparisonMonitor();
		break;
	}

	Neighborhood::turnLeft();
}

void FullTSA::turnRight() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kTSA15, kSouth):
		if (GameState.getTSAState() == kTSAPlayerNeedsHistoricalLog)
			setCurrentAlternate(kAltTSANormal);
		break;
	case MakeRoomView(kTSA0B, kNorth):
		if (_ripTimer.isVisible())
			_ripTimer.hide();
		releaseSprites();
		break;
	case MakeRoomView(kTSA0B, kEast):
		shutDownComparisonMonitor();
		break;
	}

	Neighborhood::turnRight();
}

void FullTSA::openDoor() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kTSA15, kSouth):
		if (GameState.getTSAState() == kTSAPlayerNeedsHistoricalLog || GameState.getTSAState() == kRobotsAtFrontDoor)
			setCurrentAlternate(kAltTSARedAlert);
		break;
	}

	Neighborhood::openDoor();
}

tCanMoveForwardReason FullTSA::canMoveForward(ExitTable::Entry &entry) {
	if (GameState.getCurrentRoomAndView() == MakeRoomView(kTSA25Red, kNorth))
		return kCantMoveBlocked;

	return Neighborhood::canMoveForward(entry);
}

tCanOpenDoorReason FullTSA::canOpenDoor(DoorTable::Entry &entry) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kTSA02, kNorth):
		if (!GameState.getTSAFrontDoorUnlockedOutside())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kTSA03, kSouth):
		if (!GameState.getTSAFrontDoorUnlockedInside())
			return kCantOpenLocked;
		break;
	case MakeRoomView(kTSA16, kNorth):
		if (GameState.getTSACommandCenterLocked())
			return kCantOpenLocked;
		break;
	}

	return Neighborhood::canOpenDoor(entry);
}

void FullTSA::bumpIntoWall() {
	requestSpotSound(kTSABumpIntoWallIn, kTSABumpIntoWallOut, kFilterAllInput, 0);
	Neighborhood::bumpIntoWall();
}

void FullTSA::downButton(const Input &input) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kTSA0B, kEast):
		if (GameState.getTSA0BZoomedIn())
			startExtraSequence(kTSA0BEastZoomOut, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kTSA0B, kNorth):
		if (GameState.getTSA0BZoomedIn())
			startExtraSequence(kTSA0BNorthZoomOut, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kTSA0B, kWest):
		if (GameState.getTSA0BZoomedIn() && GameState.getT0BMonitorMode() == kMonitorNeutral)
			startExtraSequence(kTSA0BWestZoomOut, kExtraCompletedFlag, kFilterNoInput);
		break;
	default:
		Neighborhood::downButton(input);
	}
}

void FullTSA::activateOneHotspot(HotspotInfoTable::Entry &entry, Hotspot *spot) {
	switch (spot->getObjectID()) {
	case kTSA0BEastLeftRewindSpotID:
	case kTSA0BEastLeftPlaySpotID:
		if (_privateFlags.getFlag(kTSAPrivatePlayingRightComparisonFlag))
			spot->setInactive();
		else
			Neighborhood::activateOneHotspot(entry, spot);
		break;
	case kTSA0BEastRightRewindSpotID:
	case kTSA0BEastRightPlaySpotID:
		if (_privateFlags.getFlag(kTSAPrivatePlayingLeftComparisonFlag))
			spot->setInactive();
		else
			Neighborhood::activateOneHotspot(entry, spot);
		break;
	default:
		Neighborhood::activateOneHotspot(entry, spot);
		break;
	}
}

void FullTSA::activateHotspots() {
	Neighborhood::activateHotspots();

	switch (MakeRoomView(GameState.getCurrentRoom(), GameState.getCurrentDirection())) {
	case MakeRoomView(kTSA02, kNorth):
		if (!GameState.getTSAFrontDoorUnlockedOutside())
			g_allHotspots.activateOneHotspot(kTSA02DoorSpotID);
		break;
	case MakeRoomView(kTSA0B, kEast):
		if (GameState.getTSA0BZoomedIn())
			switch (GameState.getTSAState()) {
			case kTSAPlayerInstalledHistoricalLog:
			case kTSABossSawHistoricalLog:
			case kRobotsAtCommandCenter:
			case kRobotsAtFrontDoor:
			case kRobotsAtReadyRoom:
				if (getCurrentActivation() != kActivateTSA0BComparisonVideo) {
					g_allHotspots.activateOneHotspot(kTSA0BEastCompareNoradSpotID);
					g_allHotspots.activateOneHotspot(kTSA0BEastCompareMarsSpotID);
					g_allHotspots.activateOneHotspot(kTSA0BEastCompareCaldoriaSpotID);
					g_allHotspots.activateOneHotspot(kTSA0BEastCompareWSCSpotID);
				}
				break;
			}
		break;
	case MakeRoomView(kTSA0B, kNorth):
		if (GameState.getTSA0BZoomedIn())
			switch (GameState.getTSAState()) {
			case kRobotsAtCommandCenter:
			case kRobotsAtFrontDoor:
			case kRobotsAtReadyRoom:
				g_allHotspots.activateOneHotspot(kTSA0BNorthRobotsToCommandCenterSpotID);
				g_allHotspots.activateOneHotspot(kTSA0BNorthRobotsToReadyRoomSpotID);
				g_allHotspots.activateOneHotspot(kTSA0BNorthRobotsToFrontDoorSpotID);
				break;
			}
		break;
	}
}

void FullTSA::clickInHotspot(const Input &input, const Hotspot *clickedSpot) {	
	switch (clickedSpot->getObjectID()) {
	case kTSAGTOtherSpotID:
		showExtraView(kTSAGTOtherChoice);
		playSpotSoundSync(kTSANoOtherDestinationIn, kTSANoOtherDestinationOut);
		showExtraView(kTSAGTCardSwipe);
		break;
	case kTSA02DoorSpotID:
		GameState.setTSAFrontDoorUnlockedOutside(true);
		Neighborhood::clickInHotspot(input, clickedSpot);
		break;
	case kTSA03EastJimenezSpotID:
		startExtraLongSequence(kTSA03JimenezZoomIn, kTSA03JimenezZoomOut, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA03WestCrenshawSpotID:
		startExtraLongSequence(kTSA03CrenshawZoomIn, kTSA03CrenshawZoomOut, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA04EastMatsumotoSpotID:
		startExtraLongSequence(kTSA04MatsumotoZoomIn, kTSA04MatsumotoZoomOut, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA04WestCastilleSpotID:
		startExtraLongSequence(kTSA04CastilleZoomIn, kTSA04CastilleZoomOut, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA05EastSinclairSpotID:
		startExtraLongSequence(kTSA05SinclairZoomIn, kTSA05SinclairZoomOut, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA05WestWhiteSpotID:
		startExtraLongSequence(kTSA05WhiteZoomIn, kTSA05WhiteZoomOut, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA0BEastCompareNoradSpotID:
		initializeComparisonMonitor(kMonitorNoradComparison, kTSA0BNoradComparisonView);
		break;
	case kTSA0BEastCompareMarsSpotID:
		initializeComparisonMonitor(kMonitorMarsComparison, kTSA0BMarsComparisonView);
		break;
	case kTSA0BEastCompareCaldoriaSpotID:
		initializeComparisonMonitor(kMonitorCaldoriaComparison, kTSA0BCaldoriaComparisonView);
		break;
	case kTSA0BEastCompareWSCSpotID:
		initializeComparisonMonitor(kMonitorWSCComparison, kTSA0BWSCComparisonView);
		break;
	case kTSA0BEastCloseVideoSpotID:
		_navMovie.stop();
		_sprite3.show();
		_vm->delayShell(1, 2);
		_sprite3.hide();
		initializeComparisonMonitor(kMonitorNeutral, 0);
		break;
	case kTSA0BEastLeftPlaySpotID:
		playLeftComparison();
		break;
	case kTSA0BEastRightPlaySpotID:
		playRightComparison();
		break;
		
	// Command center
	case kTSA0BWestTheorySpotID:
		initializeTBPMonitor(kMonitorTheory, kTSA0BTBPTheoryHighlight);
		break;
	case kTSA0BWestBackgroundSpotID:
		initializeTBPMonitor(kMonitorBackground, kTSA0BTBPBackgroundHighlight);
		break;
	case kTSA0BWestProcedureSpotID:
		initializeTBPMonitor(kMonitorProcedure, kTSA0BTBPProcedureHighlight);
		break;
	case kTSA0BWestCloseVideoSpotID:
		_navMovie.stop();
		_sprite2.show();
		_vm->delayShell(1, 2);
		_sprite2.hide();
		initializeTBPMonitor(kMonitorNeutral, 0);
		break;
	case kTSA0BWestPlayVideoSpotID:
		playTBPMonitor();
		break;
	case kTSA0BEastLeftRewindSpotID:
	case kTSA0BEastRightRewindSpotID:
	case kTSA0BWestRewindVideoSpotID:
		if ((GameState.getT0BMonitorMode() & kPlayingAnyMask) != 0) {
			bool playing = _navMovie.isRunning();
			if (playing)
				_navMovie.stop();

			if (clickedSpot->getObjectID() == kTSA0BEastRightRewindSpotID)
				_sprite2.show();
			else
				_sprite1.show();

			_vm->delayShell(1, 2);

			if (clickedSpot->getObjectID() == kTSA0BEastRightRewindSpotID)
				_sprite2.hide();
			else
				_sprite1.hide();

			_navMovie.setTime(GameState.getT0BMonitorStart());

			if (playing) {
				_navMovie.start();
			} else {
				_privateFlags.setFlag(kTSAPrivatePlayingLeftComparisonFlag, false);
				_privateFlags.setFlag(kTSAPrivatePlayingRightComparisonFlag, false);
			}
		}
		break;
	case kTSA22EastMonitorSpotID:
		requestExtraSequence(kTSA22RedEastZoomInSequence, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA23WestMonitorSpotID:
		requestExtraSequence(kTSA23RedWestVaultZoomInSequence, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA0BNorthRobotsToCommandCenterSpotID:
		_sprite1.setCurrentFrameIndex(kRedirectionCCDoorSprite);
		_sprite1.show();
		_vm->delayShell(1, 2);
		_sprite1.hide();

		switch (GameState.getTSAState()) {
		case kRobotsAtCommandCenter:
			// Nothing
			break;
		case kRobotsAtFrontDoor:
			GameState.setTSAState(kRobotsAtCommandCenter);
			_sprite2.setCurrentFrameIndex(kRedirectionNewTargetSprite);
			startExtraSequence(kTSA0BRobotsFromFrontDoorToCommandCenter, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kRobotsAtReadyRoom:
			GameState.setTSAState(kRobotsAtCommandCenter);
			_sprite2.setCurrentFrameIndex(kRedirectionNewTargetSprite);
			startExtraSequence(kTSA0BRobotsFromReadyRoomToCommandCenter, kExtraCompletedFlag, kFilterNoInput);
			break;
		}
		break;
	case kTSA0BNorthRobotsToReadyRoomSpotID:
		_sprite1.setCurrentFrameIndex(kRedirectionRRDoorSprite);
		_sprite1.show();
		_vm->delayShell(1, 2);
		_sprite1.hide();

		switch (GameState.getTSAState()) {
		case kRobotsAtCommandCenter:
			GameState.setTSAState(kRobotsAtReadyRoom);
			_sprite2.setCurrentFrameIndex(kRedirectionNewTargetSprite);
			startExtraSequence(kTSA0BRobotsFromCommandCenterToReadyRoom, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kRobotsAtFrontDoor:
			GameState.setTSAState(kRobotsAtReadyRoom);
			_sprite2.setCurrentFrameIndex(kRedirectionNewTargetSprite);
			startExtraSequence(kTSA0BRobotsFromFrontDoorToReadyRoom, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kRobotsAtReadyRoom:
			// Nothing
			break;
		}
		break;
	case kTSA0BNorthRobotsToFrontDoorSpotID:
		_sprite1.setCurrentFrameIndex(kRedirectionFDDoorSprite);
		_sprite1.show();
		_vm->delayShell(1, 2);
		_sprite1.hide();

		switch (GameState.getTSAState()) {
		case kRobotsAtCommandCenter:
			GameState.setTSAState(kRobotsAtFrontDoor);
			_sprite2.setCurrentFrameIndex(kRedirectionNewTargetSprite);
			startExtraSequence(kTSA0BRobotsFromCommandCenterToFrontDoor, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kRobotsAtFrontDoor:
			// Nothing
			break;
		case kRobotsAtReadyRoom:
			GameState.setTSAState(kRobotsAtFrontDoor);
			_sprite2.setCurrentFrameIndex(kRedirectionNewTargetSprite);
			startExtraSequence(kTSA0BRobotsFromReadyRoomToFrontDoor, kExtraCompletedFlag, kFilterNoInput);
			break;
		}
		break;

	// Pegasus
	case kTSA37NorthJumpToPrehistoricSpotID:
		startExtraSequence(kTSA37PegasusDepart, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA37NorthExitSpotID:
		_sprite2.setCurrentFrameIndex(1);
		_vm->delayShell(1, 2);
		releaseSprites();
		moveForward();
		break;
	case kTSA37NorthJumpMenuSpotID:
		_sprite2.setCurrentFrameIndex(1);
		_vm->delayShell(1, 2);
		releaseSprites();
		break;			
	case kTSA37NorthJumpToNoradSpotID:
		GameState.setTSAState(kPlayerOnWayToNorad);
		requestExtraSequence(kTSA37JumpToNorad, 0, kFilterNoInput);

		if (!GameState.getBeenToNorad()) {
			requestExtraSequence(kTSA37NoradToAI7, 0, kFilterNoInput);
			requestExtraSequence(kTSA37PegasusAI7, 0, kFilterNoInput);
			requestExtraSequence(kTSA37AI7ToNorad, 0, kFilterNoInput);
			GameState.setBeenToNorad(true);
		}

		requestExtraSequence(kTSA37NoradToDepart, 0, kFilterNoInput);
		requestExtraSequence(kTSA37PegasusDepart, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA37NorthJumpToMarsSpotID:
		GameState.setTSAState(kPlayerOnWayToMars);
		requestExtraSequence(kTSA37JumpToMars, 0, kFilterNoInput);

		if (!GameState.getBeenToMars()) {
			requestExtraSequence(kTSA37MarsToAI6, 0, kFilterNoInput);
			requestExtraSequence(kTSA37PegasusAI6, 0, kFilterNoInput);
			requestExtraSequence(kTSA37AI6ToMars, 0, kFilterNoInput);
			GameState.setBeenToMars(true);
		}

		requestExtraSequence(kTSA37MarsToDepart, 0, kFilterNoInput);
		requestExtraSequence(kTSA37PegasusDepart, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTSA37NorthJumpToWSCSpotID:
		GameState.setTSAState(kPlayerOnWayToWSC);
		requestExtraSequence(kTSA37JumpToWSC, 0, kFilterNoInput);

		if (!GameState.getBeenToWSC()) {
			requestExtraSequence(kTSA37WSCToAI5, 0, kFilterNoInput);
			requestExtraSequence(kTSA37PegasusAI5, 0, kFilterNoInput);
			requestExtraSequence(kTSA37AI5ToWSC, 0, kFilterNoInput);
			GameState.setBeenToWSC(true);
		}

		requestExtraSequence(kTSA37WSCToDepart, 0, kFilterNoInput);
		requestExtraSequence(kTSA37PegasusDepart, kExtraCompletedFlag, kFilterNoInput);
		break;
	default:
		Neighborhood::clickInHotspot(input, clickedSpot);
		break;
	}
}

void FullTSA::showMainJumpMenu() {
	tExtraID jumpMenuView = kTSA37JumpMenu000;

	if (GameState.getNoradFinished())
		jumpMenuView += 4;
	if (GameState.getMarsFinished())
		jumpMenuView += 2;
	if (GameState.getWSCFinished())
		jumpMenuView += 1;

	showExtraView(jumpMenuView);
	setCurrentActivation(kActivationMainJumpMenu);
}

void FullTSA::playTBPMonitor() {
	InputHandler::getCurrentInputDevice()->waitInput(kFilterAllButtons);
	
	if ((GameState.getT0BMonitorMode() & kPlayingTBPMask) == 0) {
		tExtraID extra;

		switch (GameState.getT0BMonitorMode() & kRawModeMask) {
		case kMonitorTheory:
			GameState.setTSASeenTheory(true);
			extra = kTSA0BTBPTheory;
			GameState.setScoringSawTheory(true);
			break;
		case kMonitorBackground:
			GameState.setTSASeenBackground(true);
			extra = kTSA0BTBPBackground;
			GameState.setScoringSawBackground(true);
			break;
		case kMonitorProcedure:
			GameState.setTSASeenProcedure(true);
			extra = kTSA0BTBPProcedure;
			GameState.setScoringSawProcedure(true);
			break;
		}

		GameState.setT0BMonitorMode(GameState.getT0BMonitorMode() | kPlayingTBPMask);

		ExtraTable::Entry entry;
		getExtraEntry(extra, entry);
		_lastExtra = extra;

		GameState.setT0BMonitorStart(entry.movieStart + kFullTSAFrameDuration * 5);
		startMovieSequence(GameState.getT0BMonitorStart(), entry.movieEnd, kExtraCompletedFlag, false, kFilterAllInput);
	} else if (_navMovie.isRunning()) {
		_navMovie.stop();
	} else {
		_navMovie.start();
	}
}

void FullTSA::initializeTBPMonitor(const int newMode, const tExtraID highlightExtra) {
	GameState.setT0BMonitorMode(newMode);

	if (newMode != kMonitorNeutral) {
		showExtraView(highlightExtra);
		_vm->delayShell(1, 2);
		setCurrentActivation(kActivateTSA0BTBPVideo);
		_sprite1.addPICTResourceFrame(kTBPRewindPICTID, false, 0, 0);
		_sprite1.moveElementTo(kTBPRewindLeft, kTBPRewindTop);
		_sprite1.setCurrentFrameIndex(0);
		_sprite2.addPICTResourceFrame(kTBPCloseBoxPICTID, false, 0, 0);
		_sprite2.moveElementTo(kTBPCloseLeft, kTBPCloseTop);
		_sprite2.setCurrentFrameIndex(0);
		playTBPMonitor();
	} else {
		if (GameState.getTSAState() == kTSAPlayerForcedReview && GameState.getTSASeenTheory() &&
				GameState.getTSASeenBackground() && GameState.getTSASeenProcedure()) {
			setOffRipAlarm();
		} else {
			setCurrentActivation(kActivateTSA0BZoomedIn);
			updateViewFrame();
		}

		releaseSprites();
	}

	_interruptionFilter = kFilterAllInput;
}

void FullTSA::startUpComparisonMonitor() {
	releaseSprites();

	_sprite1.addPICTResourceFrame(kComparisonHiliteNoradPICTID, false,
			kComparisonHiliteNoradLeft - kComparisonHiliteSpriteLeft,
			kComparisonHiliteNoradTop - kComparisonHiliteSpriteTop);
	_sprite1.addPICTResourceFrame(kComparisonHiliteMarsPICTID, false,
			kComparisonHiliteMarsLeft - kComparisonHiliteSpriteLeft,
			 kComparisonHiliteMarsTop - kComparisonHiliteSpriteTop);
	_sprite1.addPICTResourceFrame(kComparisonHiliteCaldoriaPICTID, false,
			kComparisonHiliteCaldoriaLeft - kComparisonHiliteSpriteLeft,
			kComparisonHiliteCaldoriaTop - kComparisonHiliteSpriteTop);
	_sprite1.addPICTResourceFrame(kComparisonHiliteWSCPICTID, false,
			kComparisonHiliteWSCLeft - kComparisonHiliteSpriteLeft,
			kComparisonHiliteWSCTop - kComparisonHiliteSpriteTop);

	_sprite1.setCurrentFrameIndex(0);
	_sprite1.moveElementTo(kComparisonHiliteSpriteLeft, kComparisonHiliteSpriteTop);
	
	_sprite2.addPICTResourceFrame(kComparisonChancesNoradPICTID, false,
			kComparisonChancesNoradLeft - kComparisonChancesSpriteLeft,
			kComparisonChancesNoradTop - kComparisonChancesSpriteTop);
	_sprite2.addPICTResourceFrame(kComparisonChancesMarsPICTID, false,
			kComparisonChancesMarsLeft - kComparisonChancesSpriteLeft,
			kComparisonChancesMarsTop - kComparisonChancesSpriteTop);
	_sprite2.addPICTResourceFrame(kComparisonChancesCaldoriaPICTID, false,
			kComparisonChancesCaldoriaLeft - kComparisonChancesSpriteLeft,
			kComparisonChancesCaldoriaTop - kComparisonChancesSpriteTop);
	_sprite2.addPICTResourceFrame(kComparisonChancesWSCPICTID, false,
			kComparisonChancesWSCLeft - kComparisonChancesSpriteLeft,
			kComparisonChancesWSCTop - kComparisonChancesSpriteTop);

	_sprite2.setCurrentFrameIndex(0);
	_sprite2.moveElementTo(kComparisonChancesSpriteLeft, kComparisonChancesSpriteTop);
	updateViewFrame();
}

void FullTSA::shutDownComparisonMonitor() {
	releaseSprites();
}

void FullTSA::initializeComparisonMonitor(const int newMode, const tExtraID comparisonView) {
	GameState.setT0BMonitorMode(newMode);
	_privateFlags.setFlag(kTSAPrivatePlayingLeftComparisonFlag, false);
	_privateFlags.setFlag(kTSAPrivatePlayingRightComparisonFlag, false);

	if (newMode != kMonitorNeutral) {
		shutDownComparisonMonitor();
		setCurrentActivation(kActivateTSA0BComparisonVideo);
		_sprite1.addPICTResourceFrame(kComparisonLeftRewindPICTID, false, 0, 0);
		_sprite1.moveElementTo(kComparisonLeftRewindLeft, kComparisonLeftRewindTop);
		_sprite1.setCurrentFrameIndex(0);
		_sprite2.addPICTResourceFrame(kComparisonRightRewindPICTID, false, 0, 0);
		_sprite2.moveElementTo(kComparisonRightRewindLeft, kComparisonRightRewindTop);
		_sprite2.setCurrentFrameIndex(0);
		_sprite3.addPICTResourceFrame(kComparisonCloseBoxPICTID, false, 0, 0);
		_sprite3.moveElementTo(kComparisonCloseLeft, kComparisonCloseTop);
		_sprite3.setCurrentFrameIndex(0);
		showExtraView(comparisonView);
	} else {
		if (GameState.getTSAState() == kTSAPlayerInstalledHistoricalLog &&
				GameState.getTSASeenNoradNormal() &&
				GameState.getTSASeenNoradAltered() &&
				GameState.getTSASeenMarsNormal() &&
				GameState.getTSASeenMarsAltered() &&
				GameState.getTSASeenCaldoriaNormal() &&
				GameState.getTSASeenCaldoriaAltered() &&
				GameState.getTSASeenWSCNormal() &&
				GameState.getTSASeenWSCAltered()) {
			GameState.setTSAState(kTSABossSawHistoricalLog);
			requestExtraSequence(kTSA0BEastZoomOut, kExtraCompletedFlag, kFilterNoInput);
			requestExtraSequence(kTSA0BEastTurnLeft, kExtraCompletedFlag, kFilterNoInput);
			requestExtraSequence(kTSA0BNorthZoomIn, kExtraCompletedFlag, kFilterNoInput);
		} else {
			setCurrentActivation(kActivateTSA0BZoomedIn);
			releaseSprites();
			startUpComparisonMonitor();
		}
	}

	_interruptionFilter = kFilterAllInput;
}

void FullTSA::playLeftComparison() {
	InputHandler::getCurrentInputDevice()->waitInput(kFilterAllButtons);

	if ((GameState.getT0BMonitorMode() & kPlayingLeftComparisonMask) == 0) {
		tExtraID extra;

		switch (GameState.getT0BMonitorMode() & kRawModeMask) {
		case kMonitorNoradComparison:
			GameState.setTSASeenNoradAltered(true);
			extra = kTSA0BNoradAltered;
			GameState.setScoringSawNoradAltered(true);
			break;
		case kMonitorMarsComparison:
			GameState.setTSASeenMarsAltered(true);
			extra = kTSA0BMarsAltered;
			GameState.setScoringSawMarsAltered(true);
			break;
		case kMonitorCaldoriaComparison:
			GameState.setTSASeenCaldoriaAltered(true);
			extra = kTSA0BCaldoriaAltered;
			GameState.setScoringSawCaldoriaAltered(true);
			break;
		case kMonitorWSCComparison:
			GameState.setTSASeenWSCAltered(true);
			extra = kTSA0BWSCAltered;
			GameState.setScoringSawWSCAltered(true);
			break;
		}

		GameState.setT0BMonitorMode(GameState.getT0BMonitorMode() | kPlayingLeftComparisonMask);

		ExtraTable::Entry entry;
		getExtraEntry(extra, entry);
		_lastExtra = extra;

		// skip first five frames of movie
		// (this is a dissolve that doesn't belong...)
		GameState.setT0BMonitorStart(entry.movieStart + kFullTSAFrameDuration * 5);
		_privateFlags.setFlag(kTSAPrivatePlayingLeftComparisonFlag);

		// Allow clicking...
		startMovieSequence(GameState.getT0BMonitorStart(), entry.movieEnd,
				kExtraCompletedFlag, false, JMPPPInput::getClickInputFilter());
	} else if (_navMovie.isRunning()) {
		_navMovie.stop();
	} else {
		_navMovie.start();
	}
}

void FullTSA::playRightComparison() {	
	InputHandler::getCurrentInputDevice()->waitInput(kFilterAllButtons);
	
	if ((GameState.getT0BMonitorMode() & kPlayingRightComparisonMask) == 0) {
		tExtraID extra;

		switch (GameState.getT0BMonitorMode() & kRawModeMask) {
		case kMonitorNoradComparison:
			GameState.setTSASeenNoradNormal(true);
			extra = kTSA0BNoradUnaltered;
			GameState.setScoringSawNoradNormal(true);
			break;
		case kMonitorMarsComparison:
			GameState.setTSASeenMarsNormal(true);
			extra = kTSA0BMarsUnaltered;
			GameState.setScoringSawMarsNormal(true);
			break;
		case kMonitorCaldoriaComparison:
			GameState.setTSASeenCaldoriaNormal(true);
			extra = kTSA0BCaldoriaUnaltered;
			GameState.setScoringSawCaldoriaNormal(true);
			break;
		case kMonitorWSCComparison:
			GameState.setTSASeenWSCNormal(true);
			extra = kTSA0BWSCUnaltered;
			GameState.setScoringSawWSCNormal(true);
			break;
		}

		GameState.setT0BMonitorMode(GameState.getT0BMonitorMode() | kPlayingRightComparisonMask);

		ExtraTable::Entry entry;
		getExtraEntry(extra, entry);
		_lastExtra = extra;

		// skip first five frames of movie
		// (this is a dissolve that doesn't belong...)
		GameState.setT0BMonitorStart(entry.movieStart + kFullTSAFrameDuration * 5);
		_privateFlags.setFlag(kTSAPrivatePlayingRightComparisonFlag);

		// Allow clicking...
		startMovieSequence(GameState.getT0BMonitorStart(), entry.movieEnd,
				kExtraCompletedFlag, false, JMPPPInput::getClickInputFilter());
	} else if (_navMovie.isRunning()) {
		_navMovie.stop();
	} else {
		_navMovie.start();
	}
}

// When this function is called, the player is zoomed up on the center monitor, and the
// TSA state is kTSABossSawHistoricalLog.
void FullTSA::startRobotGame() {
	requestExtraSequence(kTSA0BNorthCantChangeHistory, 0, kFilterNoInput);
	requestExtraSequence(kTSA0BAIInterruption, 0, kFilterNoInput);
	requestExtraSequence(kTSA0BShowGuardRobots, 0, kFilterNoInput);
	requestExtraSequence(kTSA0BRobotsToCommandCenter, kExtraCompletedFlag, kFilterNoInput);
}

void FullTSA::startUpRobotMonitor() {
	releaseSprites();

	_sprite1.addPICTResourceFrame(kRedirectionCCRolloverPICTID, true,
			kRedirectionCCRolloverLeft - kRedirectionSprite1Left,
			kRedirectionCCRolloverTop - kRedirectionSprite1Top);
	_sprite1.addPICTResourceFrame(kRedirectionRRRolloverPICTID, true,
			kRedirectionRRRolloverLeft - kRedirectionSprite1Left,
			kRedirectionRRRolloverTop - kRedirectionSprite1Top);
	_sprite1.addPICTResourceFrame(kRedirectionFDRolloverPICTID, false,
			kRedirectionFDRolloverLeft - kRedirectionSprite1Left,
			kRedirectionFDRolloverTop - kRedirectionSprite1Top);
	_sprite1.addPICTResourceFrame(kRedirectionCCDoorPICTID, true,
			kRedirectionCCDoorLeft - kRedirectionSprite1Left,
			kRedirectionCCDoorTop - kRedirectionSprite1Top);
	_sprite1.addPICTResourceFrame(kRedirectionRRDoorPICTID, true,
			kRedirectionRRDoorLeft - kRedirectionSprite1Left,
			kRedirectionRRDoorTop - kRedirectionSprite1Top);
	_sprite1.addPICTResourceFrame(kRedirectionFDDoorPICTID, false,
			kRedirectionFDDoorLeft - kRedirectionSprite1Left,
			kRedirectionFDDoorTop - kRedirectionSprite1Top);
	_sprite1.addPICTResourceFrame(kRedirectionClosePICTID, false,
			kRedirectionCloseLeft - kRedirectionSprite1Left,
			kRedirectionCloseTop - kRedirectionSprite1Top);
	_sprite1.moveElementTo(kRedirectionSprite1Left, kRedirectionSprite1Top);

	_sprite2.addPICTResourceFrame(kRedirectionSecuredPICTID, false,
			kRedirectionSecuredLeft - kRedirectionSprite2Left,
			kRedirectionSecuredTop - kRedirectionSprite2Top);
	_sprite2.addPICTResourceFrame(kRedirectionNewTargetPICTID, false,
			kRedirectionNewTargetLeft - kRedirectionSprite2Left,
			kRedirectionNewTargetTop - kRedirectionSprite2Top);
	_sprite2.moveElementTo(kRedirectionSprite2Left, kRedirectionSprite2Top);

	switch (GameState.getTSAState()) {
	case kRobotsAtCommandCenter:
		showExtraView(kTSA0BNorthRobotsAtCCView);
		break;
	case kRobotsAtFrontDoor:
		showExtraView(kTSA0BNorthRobotsAtFDView);
		break;
	case kRobotsAtReadyRoom:
		showExtraView(kTSA0BNorthRobotsAtRRView);
		break;
	}
}

void FullTSA::shutDownRobotMonitor() {
	releaseSprites();
}

// Assume this is called only when zoomed in at T0B west
void FullTSA::setOffRipAlarm() {
	GameState.setTSAState(kTSAPlayerDetectedRip);
	_ripTimer.initImage();
	_ripTimer.moveElementTo(kRipTimerLeft, kRipTimerTop);
	_ripTimer.setSegment(0, kRipTimeLimit, kRipTimeScale);
	_ripTimer.start();
	loadAmbientLoops();
	startExtraSequenceSync(kTSA0BRipAlarmScreen, kFilterNoInput);
	_vm->delayShell(2, 1); // Two seconds..
	requestExtraSequence(kTSA0BWestZoomOut, kExtraCompletedFlag, kFilterNoInput);
	requestExtraSequence(kTSA0BWestTurnRight, 0, kFilterNoInput);
	requestExtraSequence(kTSA0BNorthZoomIn, kExtraCompletedFlag, kFilterNoInput);
	requestExtraSequence(kTSA0BNorthFinallyHappened, 0, kFilterNoInput);
	requestExtraSequence(kTSA0BShowRip1, kExtraCompletedFlag, kFilterNoInput);
}

void FullTSA::checkContinuePoint(const tRoomID room, const tDirectionConstant direction) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kTSA04, kNorth):
	case MakeRoomView(kTSA14, kEast):
	case MakeRoomView(kTSA15, kWest):
	case MakeRoomView(kTSA16, kNorth):
	case MakeRoomView(kTSA16, kSouth):
	case MakeRoomView(kTSA21Cyan, kSouth):
	case MakeRoomView(kTSA21Red, kSouth):
	case MakeRoomView(kTSA26, kNorth):
		makeContinuePoint();
		break;
	}
}

void FullTSA::arriveAt(const tRoomID room, const tDirectionConstant direction) {
	checkRobotLocations(room, direction);
	Neighborhood::arriveAt(room, direction);

	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kTSADeathRoom, kNorth):
	case MakeRoomView(kTSADeathRoom, kSouth):
	case MakeRoomView(kTSADeathRoom, kEast):
	case MakeRoomView(kTSADeathRoom, kWest):
		die(kDeathShotByTSARobots);
		break;
	case MakeRoomView(kTSA00, kNorth):
		if (GameState.getLastNeighborhood() != kFullTSAID) {
			makeContinuePoint();
			openDoor();
		} else {
			setCurrentActivation(kActivateTSAReadyForCard);
			loopExtraSequence(kTSATransporterArrowLoop, 0);
		}
		break;
	case MakeRoomView(kTSA03, kNorth):
	case MakeRoomView(kTSA05, kNorth):
	case MakeRoomView(kTSA0A, kNorth):
	case MakeRoomView(kTSA06, kNorth):
	case MakeRoomView(kTSA07, kNorth):
		if (_utilityFuse.isFuseLit())
			_utilityFuse.stopFuse();
		GameState.setScoringEnterTSA(true);
		break;
	case MakeRoomView(kTSA04, kNorth):
		if (_utilityFuse.isFuseLit())
			_utilityFuse.stopFuse();
		if (!GameState.getTSASeenRobotGreeting())
			startExtraSequence(kTSA04NorthRobotGreeting, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kTSA03, kSouth):
		GameState.setTSAFrontDoorUnlockedInside(GameState.getTSAState() == kRobotsAtFrontDoor || GameState.allTimeZonesFinished());
		break;
	case MakeRoomView(kTSA0A, kEast):
	case MakeRoomView(kTSA0A, kWest):
		if (GameState.getTSAState() == kTSAPlayerNotArrived)
			setCurrentActivation(kActivateTSARobotsAwake);
		break;
	case MakeRoomView(kTSA0B, kNorth):
		if (GameState.getTSA0BZoomedIn()) {
			setCurrentActivation(kActivateTSA0BZoomedIn);

			switch (GameState.getTSAState()) {
			case kTSAPlayerNeedsHistoricalLog:
				_ripTimer.show();
				break;
			case kRobotsAtCommandCenter:
			case kRobotsAtFrontDoor:
			case kRobotsAtReadyRoom:
				startUpRobotMonitor();
				break;
			}
		} else {
			setCurrentActivation(kActivateTSA0BZoomedOut);

			switch (GameState.getTSAState()) {
			case kTSAPlayerNotArrived:
				requestExtraSequence(kTSA0BNorthZoomIn, kExtraCompletedFlag, kFilterNoInput);
				requestExtraSequence(kTSA0BNorthYoureBusted, 0, kFilterNoInput);
				requestExtraSequence(kTSA0BNorthZoomOut, kExtraCompletedFlag, kFilterNoInput);
				requestExtraSequence(kTSA0BNorthTurnLeft, 0, kFilterNoInput);
				requestExtraSequence(kTSA0BWestZoomIn, kExtraCompletedFlag, kFilterNoInput);
				break;
			case kTSAPlayerGotHistoricalLog:
				startExtraSequence(kTSA0BNorthHistLogOpen, kExtraCompletedFlag, kFilterNoInput);
				break;
			}
		}
		break;
	case MakeRoomView(kTSA0B, kSouth):
		GameState.setTSA0BZoomedIn(false);
		setCurrentActivation(kActivateTSA0BZoomedOut);
		break;
	case MakeRoomView(kTSA0B, kWest):
		if (GameState.getTSA0BZoomedIn()) {
			setCurrentActivation(kActivateTSA0BZoomedIn);
			initializeTBPMonitor(kMonitorNeutral, 0);
		} else {
			setCurrentActivation(kActivateTSA0BZoomedOut);
		}
		break;
	case MakeRoomView(kTSA0B, kEast):
		if (GameState.getTSA0BZoomedIn()) {
			setCurrentActivation(kActivateTSA0BZoomedIn);

			switch (GameState.getTSAState()) {
			case kTSAPlayerInstalledHistoricalLog:
			case kTSABossSawHistoricalLog:
			case kRobotsAtCommandCenter:
			case kRobotsAtFrontDoor:
			case kRobotsAtReadyRoom:
				initializeComparisonMonitor(kMonitorNeutral, 0);
				break;
			}
		} else {
			setCurrentActivation(kActivateTSA0BZoomedOut);
		}
		break;
	case MakeRoomView(kTSA21Red, kSouth):
		if (GameState.getTSAState() == kRobotsAtFrontDoor)
			GameState.setScoringWentToReadyRoom2(true);
		break;
	case MakeRoomView(kTSA22Red, kEast):
		if (!_vm->playerHasItemID(kJourneymanKey))
			setCurrentActivation(kActivationDoesntHaveKey);
		break;
	case MakeRoomView(kTSA23Red, kWest):
		if (!_vm->playerHasItemID(kPegasusBiochip))
			setCurrentActivation(kActivationDoesntHaveChips);
		break;
	case MakeRoomView(kTSA25Red, kNorth):
		arriveAtTSA25Red();
		break;
	case MakeRoomView(kTSA34, kSouth):
		if (GameState.getLastRoom() == kTSA37)
			closeDoorOffScreen(kTSA37, kNorth);
		break;
	case MakeRoomView(kTSA37, kNorth):
		arriveAtTSA37();
		break;
	}
}

void FullTSA::checkRobotLocations(const tRoomID room, const tDirectionConstant dir) {
	switch (room) {
	case kTSA03:
	case kTSA04:
	case kTSA05:
	case kTSA06:
	case kTSA0A:
	case kTSA07:
	case kTSA08:
	case kTSA09:
	case kTSA10:
	case kTSA11:
	case kTSA12:
	case kTSA13:
	case kTSA14:
	case kTSA15:
		switch (GameState.getTSAState()) {
		case kRobotsAtFrontDoor:
			setCurrentAlternate(kAltTSARobotsAtFrontDoor);
			break;
		case kRobotsAtReadyRoom:
			setCurrentAlternate(kAltTSARobotsAtReadyRoom);
			break;
		}
		break;
	case kTSA16:
		if (dir == kNorth) {
			switch (GameState.getTSAState()) {
			case kRobotsAtCommandCenter:
				if (!_privateFlags.getFlag(kTSAPrivateSeenRobotWarningFlag)) {
					g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/TSA/XT11WB", false, kWarningInterruption);
					_privateFlags.setFlag(kTSAPrivateSeenRobotWarningFlag, true);
				}
				break;
			case kRobotsAtFrontDoor:
				setCurrentAlternate(kAltTSARobotsAtFrontDoor);
				break;
			case kRobotsAtReadyRoom:
				setCurrentAlternate(kAltTSARobotsAtReadyRoom);
				break;
			}
		}
		break;
	}
}

void FullTSA::arriveAtTSA25Red() {
	if (!_vm->playerHasItemID(kJourneymanKey))
		startExtraSequence(kTSA25NorthDeniedNoKey, kExtraCompletedFlag, kFilterNoInput);
	else if (!_vm->playerHasItemID(kPegasusBiochip))
		startExtraSequence(kTSA25NorthDeniedNoChip, kExtraCompletedFlag, kFilterNoInput);
	else if (GameState.getTSABiosuitOn())
		startExtraSequence(kTSA25NorthAlreadyHaveSuit, kExtraCompletedFlag, kFilterNoInput);
	else
		startExtraSequence(kTSA25NorthPutOnSuit, kExtraCompletedFlag, kFilterNoInput);
}

void FullTSA::arriveAtTSA37() {
	_ripTimer.stop();
	_ripTimer.releaseImage();

	switch (GameState.getTSAState()) {
	case kTSAPlayerNeedsHistoricalLog:
		startExtraLongSequence(kTSA37HorseToAI1, kTSA37AI2ToPrehistoric, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kPlayerOnWayToPrehistoric:
		setCurrentActivation(kActivationJumpToPrehistoric);
		showExtraView(kTSA37AI2ToPrehistoric);
		break;
	case kTSAPlayerGotHistoricalLog:
		initializePegasusButtons(false, true);
		break;
	case kPlayerWentToPrehistoric:
	case kPlayerOnWayToNorad:
	case kPlayerOnWayToMars:
	case kPlayerOnWayToWSC:
		startExtraSequence(kTSA37TimeJumpToPegasus, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kRobotsAtFrontDoor:
		startExtraLongSequence(kTSA37HorseToColonel2, kTSA37AI4ToMainMenu, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kPlayerLockedInPegasus:
		showMainJumpMenu();
		break;
	case kPlayerFinishedWithTSA:
		initializePegasusButtons(true, true);
		break;
	}
}

void FullTSA::turnTo(const tDirectionConstant newDirection) {
	Neighborhood::turnTo(newDirection);

	switch (MakeRoomView(GameState.getCurrentRoom(), newDirection)) {
	case MakeRoomView(kTSA03, kSouth):
		if (GameState.getTSAState() == kRobotsAtFrontDoor || GameState.allTimeZonesFinished())
			GameState.setTSAFrontDoorUnlockedInside(true);
		else
			GameState.setTSAFrontDoorUnlockedInside(false);
		break;
	case MakeRoomView(kTSA0A, kEast):
	case MakeRoomView(kTSA0A, kWest):
		setCurrentActivation(kActivateTSARobotsAwake);
		break;
	case MakeRoomView(kTSA0B, kEast):
		if (GameState.getTSA0BZoomedIn())
			setCurrentActivation(kActivateTSA0BZoomedIn);
		else
			setCurrentActivation(kActivateTSA0BZoomedOut);

		GameState.setT0BMonitorMode(GameState.getT0BMonitorMode() & ~kPlayingAnyMask);

		if (_privateFlags.getFlag(kTSAPrivateLogReaderOpenFlag))
			_privateFlags.setFlag(kTSAPrivateLogReaderOpenFlag, false);

		switch (GameState.getTSAState()) {
		case kTSAPlayerInstalledHistoricalLog:
		case kTSABossSawHistoricalLog:
		case kRobotsAtCommandCenter:
		case kRobotsAtFrontDoor:
		case kRobotsAtReadyRoom:
			if (GameState.getTSA0BZoomedIn())
				startUpComparisonMonitor();
			break;
		}
		break;
	case MakeRoomView(kTSA0B, kNorth):
		if (GameState.getTSA0BZoomedIn())
			setCurrentActivation(kActivateTSA0BZoomedIn);
		else
			setCurrentActivation(kActivateTSA0BZoomedOut);

		GameState.setT0BMonitorMode(GameState.getT0BMonitorMode() & ~kPlayingAnyMask);

		switch (GameState.getTSAState()) {
		case kTSAPlayerNeedsHistoricalLog:
			if (GameState.getTSA0BZoomedIn())
				_ripTimer.show();
			break;
		case kTSAPlayerGotHistoricalLog:
			if (!GameState.getTSA0BZoomedIn())
				startExtraSequence(kTSA0BNorthHistLogOpen, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kTSAPlayerInstalledHistoricalLog:
			if (GameState.getTSA0BZoomedIn()) {
				if ((GameState.getTSASeenNoradNormal() || GameState.getTSASeenNoradAltered()) &&
						(GameState.getTSASeenMarsNormal() || GameState.getTSASeenMarsAltered()) &&
						(GameState.getTSASeenCaldoriaNormal() || GameState.getTSASeenCaldoriaAltered()) &&
						(GameState.getTSASeenWSCNormal() || GameState.getTSASeenWSCAltered())) {
					GameState.setTSAState(kTSABossSawHistoricalLog);
					startRobotGame();
				}
			}
			break;
		case kRobotsAtCommandCenter:
		case kRobotsAtFrontDoor:
		case kRobotsAtReadyRoom:
			if (GameState.getTSA0BZoomedIn())
				startExtraSequence(kTSA0BShowGuardRobots, kExtraCompletedFlag, kFilterNoInput);
			break;
		}
		break;
	case MakeRoomView(kTSA0B, kWest):
		if (GameState.getTSA0BZoomedIn())
			setCurrentActivation(kActivateTSA0BZoomedIn);
		else
			setCurrentActivation(kActivateTSA0BZoomedOut);

		GameState.setT0BMonitorMode(GameState.getT0BMonitorMode() & ~kPlayingAnyMask);

		if (_privateFlags.getFlag(kTSAPrivateLogReaderOpenFlag))
			_privateFlags.setFlag(kTSAPrivateLogReaderOpenFlag, false);

		if (GameState.getTSA0BZoomedIn())
			initializeTBPMonitor(kMonitorNeutral, 0);
		break;
	case MakeRoomView(kTSA0B, kSouth):
		GameState.setTSA0BZoomedIn(false);
		setCurrentActivation(kActivateTSA0BZoomedOut);
		break;
	case MakeRoomView(kTSA16, kNorth):
		switch (GameState.getTSAState()) {
		case kRobotsAtCommandCenter:
			if (!_privateFlags.getFlag(kTSAPrivateSeenRobotWarningFlag)) {
				g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/TSA/XT11WB", false, kWarningInterruption);
				_privateFlags.setFlag(kTSAPrivateSeenRobotWarningFlag, true);
			}
			break;
		case kRobotsAtFrontDoor:
			setCurrentAlternate(kAltTSARobotsAtFrontDoor);
			break;
		case kRobotsAtReadyRoom:
			setCurrentAlternate(kAltTSARobotsAtReadyRoom);
			break;
		}
		break;
	case MakeRoomView(kTSA22Red, kEast):
		if (!_vm->playerHasItemID(kJourneymanKey))
			setCurrentActivation(kActivationDoesntHaveKey);
		break;
	case MakeRoomView(kTSA22Red, kNorth):
	case MakeRoomView(kTSA22Red, kSouth):
		if (_privateFlags.getFlag(kTSAPrivateKeyVaultOpenFlag)) {
			playSpotSoundSync(kTSAVaultCloseIn, kTSAVaultCloseOut);
			_privateFlags.setFlag(kTSAPrivateKeyVaultOpenFlag, false);
		}

		setCurrentActivation(kActivateHotSpotAlways);
		break;
	case MakeRoomView(kTSA23Red, kWest):
		if (!_vm->playerHasItemID(kPegasusBiochip))
			setCurrentActivation(kActivationDoesntHaveChips);
		break;
	case MakeRoomView(kTSA23Red, kNorth):
	case MakeRoomView(kTSA23Red, kSouth):
		if (_privateFlags.getFlag(kTSAPrivateChipVaultOpenFlag)) {
			playSpotSoundSync(kTSAVaultCloseIn, kTSAVaultCloseOut);
			_privateFlags.setFlag(kTSAPrivateChipVaultOpenFlag, false);
		}

		setCurrentActivation(kActivateHotSpotAlways);
		break;
	}

	// Make sure the TBP monitor is forced neutral.
	GameState.setT0BMonitorMode(kMonitorNeutral);
}

void FullTSA::closeDoorOffScreen(const tRoomID room, const tDirectionConstant) {
	switch (room) {
	case kTSA00:
	case kTSA01:
		if (GameState.getCurrentRoom() == kTSA01 || GameState.getCurrentRoom() == kTSA02)
			playSpotSoundSync(kTSAGTDoorCloseIn, kTSAGTDoorCloseOut);
		break;
	case kTSA02:
	case kTSA03:
		playSpotSoundSync(kTSAEntryDoorCloseIn, kTSAEntryDoorCloseOut);
		break;
	case kTSA14:
	case kTSA15:
	case kTSA16:
	case kTSA21Cyan:
	case kTSA21Red:
		playSpotSoundSync(kTSAInsideDoorCloseIn, kTSAInsideDoorCloseOut);
		break;
	case kTSA34:
	case kTSA37:
		playSpotSoundSync(kTSAPegasusDoorCloseIn, kTSAPegasusDoorCloseOut);
		break;
	}
}

void FullTSA::receiveNotification(Notification *notification, const tNotificationFlags flags) {	
	tExtraID lastExtra = _lastExtra;

	if ((flags & kExtraCompletedFlag) != 0) {
		switch (lastExtra) {
		case kTSA0BEastTurnLeft:
			// Need to check this here because turnTo will call _navMovie.stop,
			// so it has to happen before Neighborhood::receiveNotification,
			// which may end up starting another sequence...
			turnTo(kNorth);
			break;
		}
	}

	Neighborhood::receiveNotification(notification, flags);

	InventoryItem *item;

	if ((flags & kExtraCompletedFlag) != 0) {
		// Only allow input if we're not in the middle of series of queue requests.
		if (actionQueueEmpty())
			_interruptionFilter = kFilterAllInput;

		switch (lastExtra) {
		case kTSAGTCardSwipe:
			item = (InventoryItem *)g_allItems.findItemByID(kKeyCard);
			_vm->addItemToInventory(item);
			setCurrentActivation(kActivateTSAReadyToTransport);
			break;
		case kTSAGTGoToCaldoria:
			_vm->jumpToNewEnvironment(kCaldoriaID, kCaldoria44, kEast);

			if (GameState.allTimeZonesFinished())
				GameState.setScoringWentAfterSinclair(true);
			break;
		case kTSAGTGoToTokyo:
		case kTSAGTGoToBeach:
			if (GameState.allTimeZonesFinished())
				die(kDeathSinclairShotDelegate);
			else
				die(kDeathUncreatedInTSA);
			break;
		case kTSA02NorthZoomOut:
			openDoor();
			break;

		// Hall of suspects.
		case kTSA04NorthRobotGreeting:
			GameState.setTSASeenRobotGreeting(true);
			restoreStriding(kTSA03, kNorth, kNoAlternateID);
			break;
		case kTSA03JimenezZoomIn:
			GameState.setScoringSawBust1(true);
			break;
		case kTSA03CrenshawZoomIn:
			GameState.setScoringSawBust2(true);
			break;
		case kTSA04MatsumotoZoomIn:
			GameState.setScoringSawBust3(true);
			break;
		case kTSA04CastilleZoomIn:
			GameState.setScoringSawBust4(true);
			break;
		case kTSA05SinclairZoomIn:
			GameState.setScoringSawBust5(true);
			break;
		case kTSA05WhiteZoomIn:
			GameState.setScoringSawBust6(true);
			break;
			
		// Command center
		// Historical comparison...
		case kTSA0BEastZoomIn:
			GameState.setTSA0BZoomedIn(true);
			setCurrentActivation(kActivateTSA0BZoomedIn);
			GameState.setT0BMonitorMode(GameState.getT0BMonitorMode() & ~kPlayingAnyMask);

			switch (GameState.getTSAState()) {
			case kTSAPlayerInstalledHistoricalLog:
			case kTSABossSawHistoricalLog:
			case kRobotsAtCommandCenter:
			case kRobotsAtFrontDoor:
			case kRobotsAtReadyRoom:
				startUpComparisonMonitor();
				break;
			}
			break;
		case kTSA0BEastZoomOut:
			GameState.setTSA0BZoomedIn(false);
			setCurrentActivation(kActivateTSA0BZoomedOut);

			switch (GameState.getTSAState()) {
			case kTSABossSawHistoricalLog:
				// Prevent current view from activating.
				break;
			default:
				activateCurrentView(GameState.getCurrentRoom(), GameState.getCurrentDirection(),
						kSpotOnTurnMask);
				break;
			}
			break;
		case kTSA0BComparisonStartup:
			if ((flags & kActionRequestCompletedFlag) != 0) {
				_privateFlags.setFlag(kTSAPrivateLogReaderOpenFlag, false);
				GameState.setTSAState(kTSAPlayerInstalledHistoricalLog);
				turnTo(kEast);
			}

			startUpComparisonMonitor();
			break;
		case kTSA0BNoradAltered:
		case kTSA0BMarsAltered:
		case kTSA0BCaldoriaAltered:
		case kTSA0BWSCAltered:
		case kTSA0BNoradUnaltered:
		case kTSA0BMarsUnaltered:
		case kTSA0BCaldoriaUnaltered:
		case kTSA0BWSCUnaltered:
			initializeComparisonMonitor(kMonitorNeutral, 0);
			break;
			
		// Center monitor.
		case kTSA0BNorthZoomIn:
			GameState.setTSA0BZoomedIn(true);
			setCurrentActivation(kActivateTSA0BZoomedIn);
			GameState.setT0BMonitorMode(GameState.getT0BMonitorMode() & ~kPlayingAnyMask);

			switch (GameState.getTSAState()) {
			case kTSAPlayerNeedsHistoricalLog:
				startExtraSequence(kTSA0BShowRip1, kExtraCompletedFlag, kFilterNoInput);
				break;
			case kTSABossSawHistoricalLog:
			case kTSAPlayerInstalledHistoricalLog:
				if ((GameState.getTSASeenNoradNormal() || GameState.getTSASeenNoradAltered()) &&
						(GameState.getTSASeenMarsNormal() || GameState.getTSASeenMarsAltered()) &&
						(GameState.getTSASeenCaldoriaNormal() || GameState.getTSASeenCaldoriaAltered()) &&
						(GameState.getTSASeenWSCNormal() || GameState.getTSASeenWSCAltered())) {
					GameState.setTSAState(kTSABossSawHistoricalLog);
					startRobotGame();
				}
				break;
			case kRobotsAtCommandCenter:
			case kRobotsAtFrontDoor:
			case kRobotsAtReadyRoom:
				startExtraSequence(kTSA0BShowGuardRobots, kExtraCompletedFlag, kFilterNoInput);
				break;
			}
			break;
		case kTSA0BNorthZoomOut:
			GameState.setTSA0BZoomedIn(false);
			setCurrentActivation(kActivateTSA0BZoomedOut);
			break;
		case kTSA0BShowRip1:
			GameState.setTSAState(kTSAPlayerNeedsHistoricalLog);
			GameState.setTSACommandCenterLocked(false);

			if ((flags & kActionRequestCompletedFlag) != 0)
				turnTo(kNorth);

			_ripTimer.show();
			break;
		case kTSA0BNorthHistLogOpen:
			setCurrentActivation(kActivationLogReaderOpen);
			_privateFlags.setFlag(kTSAPrivateLogReaderOpenFlag, true);
			break;
		case kTSA0BRobotsToCommandCenter:
			GameState.setTSAState(kRobotsAtCommandCenter);
			// Fall through
		case kTSA0BShowGuardRobots:
			startUpRobotMonitor();
			// Fall through
		case kTSA0BRobotsFromCommandCenterToReadyRoom:
		case kTSA0BRobotsFromReadyRoomToCommandCenter:
		case kTSA0BRobotsFromCommandCenterToFrontDoor:
		case kTSA0BRobotsFromFrontDoorToCommandCenter:
		case kTSA0BRobotsFromFrontDoorToReadyRoom:
		case kTSA0BRobotsFromReadyRoomToFrontDoor:
			_sprite2.setCurrentFrameIndex(kRedirectionSecuredSprite);
			_sprite2.show();
			break;
			
		// TBP monitor.
		case kTSA0BWestZoomIn:
			GameState.setTSA0BZoomedIn(true);
			setCurrentActivation(kActivateTSA0BZoomedIn);

			if (GameState.getTSAState() == kTSAPlayerNotArrived) {
				turnTo(kWest);
				GameState.setTSACommandCenterLocked(true);
				GameState.setTSAState(kTSAPlayerForcedReview);
			}

			initializeTBPMonitor(kMonitorNeutral, 0);
			break;
		case kTSA0BWestZoomOut:
			GameState.setTSA0BZoomedIn(false);
			setCurrentActivation(kActivateTSA0BZoomedOut);
			GameState.setT0BMonitorMode(kMonitorNeutral);

			switch (GameState.getTSAState()) {
			case kTSAPlayerDetectedRip:
				// Keep the current view from activating.
				break;
			default:
				activateCurrentView(GameState.getCurrentRoom(), GameState.getCurrentDirection(),
						kSpotOnTurnMask);
				break;
			}
			break;
		case kTSA0BTBPTheory:
		case kTSA0BTBPBackground:
		case kTSA0BTBPProcedure:
			initializeTBPMonitor(kMonitorNeutral, 0);
			break;
			
		// Ready room
		case kTSA22RedEastZoomInSequence:
			_privateFlags.setFlag(kTSAPrivateKeyVaultOpenFlag, true);
			setCurrentActivation(kActivationKeyVaultOpen);
			break;
		case kTSA23RedWestVaultZoomInSequence:
			_privateFlags.setFlag(kTSAPrivateChipVaultOpenFlag, true);
			setCurrentActivation(kActivationChipVaultOpen);
			break;
		case kTSA25NorthPutOnSuit:
			GameState.setTSABiosuitOn(true);
			GameState.setScoringGotBiosuit(true);
			// Fall through...
		case kTSA25NorthAlreadyHaveSuit:
			requestExtraSequence(kTSA25NorthDescending1, 0, kFilterNoInput);
			requestExtraSequence(kTSA25NorthDescending2, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kTSA25NorthDescending2:
			arriveAt(kTSA26, kNorth);
			break;

		// Pegasus.
		case kTSA37HorseToAI1:
		case kTSA37AI2ToPrehistoric:
			setCurrentActivation(kActivationJumpToPrehistoric);
			GameState.setTSAState(kPlayerOnWayToPrehistoric);
			break;
		case kTSA37PegasusDepart:
			_vm->setLastEnergyValue(kFullEnergy);

			switch (GameState.getTSAState()) {
			case kPlayerOnWayToPrehistoric:
				_vm->jumpToNewEnvironment(kPrehistoricID, kPrehistoric02, kSouth);
				GameState.setPrehistoricSeenTimeStream(false);
				GameState.setPrehistoricSeenFlyer1(false);
				GameState.setPrehistoricSeenFlyer2(false);
				GameState.setPrehistoricSeenBridgeZoom(false);
				GameState.setPrehistoricBreakerThrown(false);
				GameState.setScoringGoToPrehistoric(true);
				GameState.setTSAState(kPlayerWentToPrehistoric);
				break;
			case kPlayerOnWayToNorad:
				_vm->jumpToNewEnvironment(kNoradAlphaID, kNorad01, kSouth);
				GameState.setNoradSeenTimeStream(false);
				GameState.setNoradGassed(true);
				GameState.setNoradFillingStationOn(false);
				GameState.setNoradN22MessagePlayed(false);
				GameState.setNoradPlayedGlobeGame(false);
				GameState.setNoradBeatRobotWithClaw(false);
				GameState.setNoradBeatRobotWithDoor(false);
				GameState.setNoradRetScanGood(false);
				GameState.setNoradWaitingForLaser(false);
				GameState.setNoradSubRoomPressure(9);
				GameState.setNoradSubPrepState(kSubNotPrepped);
				break;
			case kPlayerOnWayToMars:
				_vm->jumpToNewEnvironment(kMarsID, kMars0A, kNorth);
				GameState.setMarsSeenTimeStream(false);
				GameState.setMarsHeardUpperPodMessage(false);
				GameState.setMarsRobotThrownPlayer(false);
				GameState.setMarsHeardCheckInMessage(false);
				GameState.setMarsPodAtUpperPlatform(false);
				GameState.setMarsSeenThermalScan(false);
				GameState.setMarsArrivedBelow(false);
				GameState.setMarsSeenRobotAtReactor(false);
				GameState.setMarsAvoidedReactorRobot(false);
				GameState.setMarsLockFrozen(false);
				GameState.setMarsLockBroken(false);
				GameState.setMarsSecurityDown(false);
				GameState.setMarsAirlockOpen(false);
				GameState.setMarsReadyForShuttleTransport(false);
				GameState.setMarsFinishedCanyonChase(false);
				GameState.setMarsThreadedMaze(false);
				break;
			case kPlayerOnWayToWSC:
				_vm->jumpToNewEnvironment(kWSCID, kWSC01, kWest);
				GameState.setWSCSeenTimeStream(false);
				GameState.setWSCPoisoned(false);
				GameState.setWSCAnsweredAboutDart(false);
				GameState.setWSCRemovedDart(false);
				GameState.setWSCAnalyzerOn(false);
				GameState.setWSCDartInAnalyzer(false);
				GameState.setWSCAnalyzedDart(false);
				GameState.setWSCPickedUpAntidote(false);
				GameState.setWSCSawMorph(false);
				GameState.setWSCDesignedAntidote(false);
				GameState.setWSCOfficeMessagesOpen(false);
				GameState.setWSCSeenNerd(false);
				GameState.setWSCHeardPage1(false);
				GameState.setWSCHeardPage2(false);
				GameState.setWSCHeardCheckIn(false);
				GameState.setWSCDidPlasmaDodge(false);
				GameState.setWSCSeenSinclairLecture(false);
				GameState.setWSCBeenAtWSC93(false);
				GameState.setWSCCatwalkDark(false);
				GameState.setWSCRobotDead(false);
				GameState.setWSCRobotGone(false);
				break;
			};
			break;
		case kTSA37TimeJumpToPegasus:
			if (g_energyMonitor)
				g_energyMonitor->stopEnergyDraining();

			switch (GameState.getTSAState()) {
			case kPlayerWentToPrehistoric:
				arriveFromPrehistoric();
				break;
			case kPlayerOnWayToNorad:
				arriveFromNorad();
				break;
			case kPlayerOnWayToMars:
				arriveFromMars();
				break;
			case kPlayerOnWayToWSC:
				arriveFromWSC();
				break;
			default:
				break;
			}
			break;
		case kTSA37DownloadToOpMemReview:
			switch (GameState.getTSAState()) {
			case kPlayerOnWayToNorad:
				g_opticalChip->playOpMemMovie(kPoseidonSpotID);
				break;
			case kPlayerOnWayToMars:
				g_opticalChip->playOpMemMovie(kAriesSpotID);
				break;
			case kPlayerOnWayToWSC:
				g_opticalChip->playOpMemMovie(kMercurySpotID);
				break;
			}

			if (GameState.allTimeZonesFinished()) {
				requestExtraSequence(kTSA37OpMemReviewToAllClear, 0, kFilterNoInput);
				requestExtraSequence(kTSA37AllClearToCongratulations, 0, kFilterNoInput);
				requestExtraSequence(kTSA37Congratulations, 0, kFilterNoInput);
				requestExtraSequence(kTSA37CongratulationsToExit, kExtraCompletedFlag, kFilterNoInput);
			} else {
				requestExtraSequence(kTSA37OpMemReviewToMainMenu, kExtraCompletedFlag, kFilterNoInput);
			}
			break;
		case kTSA37RecallToDownload:
		case kTSA37ReviewRequiredToExit:
			GameState.setTSAState(kTSAPlayerGotHistoricalLog);
			initializePegasusButtons(kPegasusUnresolved, kPegasusCanExit);
			break;
		case kTSA37ZoomToMainMenu:
		case kTSA37HorseToColonel2:
		case kTSA37DownloadToMainMenu:
		case kTSA37OpMemReviewToMainMenu:
		case kTSA37AI4ToMainMenu:
			GameState.setTSAState(kPlayerLockedInPegasus);
			showMainJumpMenu();
			makeContinuePoint();
			break;
		case kTSA37JumpToNoradMenu:
			setCurrentActivation(kActivationJumpToNorad);
			break;
		case kTSA37JumpToMarsMenu:
			setCurrentActivation(kActivationJumpToMars);
			break;
		case kTSA37JumpToWSCMenu:
			setCurrentActivation(kActivationJumpToWSC);
			break;
		case kTSA37CancelNorad:
		case kTSA37CancelMars:
		case kTSA37CancelWSC:
			showMainJumpMenu();
			break;
		case kTSA37CongratulationsToExit:
			GameState.setTSAState(kPlayerFinishedWithTSA);
			initializePegasusButtons(true, true);
			break;
		}
	}

	g_AIArea->checkMiddleArea();
}

void FullTSA::arriveFromPrehistoric() {
	if (_vm->playerHasItemID(kHistoricalLog)) {
		GameState.setScoringFinishedPrehistoric();
		requestExtraSequence(kTSA37RecallToDownload, 0, kFilterNoInput);
		requestExtraSequence(kTSA37DownloadToColonel1, 0, kFilterNoInput);
		requestExtraSequence(kTSA37Colonel1, 0, kFilterNoInput);
		requestExtraSequence(kTSA37Colonel1ToReviewRequired, 0, kFilterNoInput);
		requestExtraSequence(kTSA37ReviewRequiredToExit, kExtraCompletedFlag, kFilterNoInput);
	} else {
		// Make sure rip timer is going...
		startExtraSequence(kTSA37DownloadToMainMenu, kExtraCompletedFlag, kFilterNoInput);
	}
}

void FullTSA::arriveFromNorad() {
	requestExtraSequence(kTSA37RecallToDownload, 0, kFilterNoInput);

	if (GameState.getNoradFinished() && !GameState.getScoringFinishedNorad()) {
		GameState.setScoringFinishedNorad();
		requestExtraSequence(kTSA37DownloadToOpMemReview, kExtraCompletedFlag, kFilterNoInput);
	} else {
		requestExtraSequence(kTSA37DownloadToMainMenu, kExtraCompletedFlag, kFilterNoInput);
	}
}

void FullTSA::arriveFromMars() {
	requestExtraSequence(kTSA37RecallToDownload, 0, kFilterNoInput);

	if (GameState.getMarsFinished() && !GameState.getScoringFinishedMars()) {
		GameState.setScoringFinishedMars();
		requestExtraSequence(kTSA37DownloadToOpMemReview, kExtraCompletedFlag, kFilterNoInput);
	} else {
		requestExtraSequence(kTSA37DownloadToMainMenu, kExtraCompletedFlag, kFilterNoInput);
	}
}

void FullTSA::arriveFromWSC() {
	requestExtraSequence(kTSA37RecallToDownload, 0, kFilterNoInput);

	if (GameState.getWSCFinished() && !GameState.getScoringFinishedWSC()) {
		GameState.setScoringFinishedWSC();
		requestExtraSequence(kTSA37DownloadToOpMemReview, kExtraCompletedFlag, kFilterNoInput);
	} else {
		requestExtraSequence(kTSA37DownloadToMainMenu, kExtraCompletedFlag, kFilterNoInput);
	}
}

void FullTSA::initializePegasusButtons(bool resolved, bool exit) {
	if (resolved) {
		_sprite1.addPICTResourceFrame(kResolvedPICTID, false, 0, 0);
		_sprite1.moveElementTo(kResolvedLeft, kResolvedTop);
	} else {
		_sprite1.addPICTResourceFrame(kUnresolvedPICTID, false, 0, 0);
		_sprite1.moveElementTo(kUnresolvedLeft, kUnresolvedTop);
	}

	_sprite1.setCurrentFrameIndex(0);
	_sprite1.show();

	if (exit) {
		_sprite2.addPICTResourceFrame(kExitPICTID, false, 0, 0);
		_sprite2.addPICTResourceFrame(kExitHilitedPICTID, false, kExitHilitedLeft - kExitLeft, kExitHilitedTop - kExitTop);
		_sprite2.moveElementTo(kExitLeft, kExitTop);
		setCurrentActivation(kActivationReadyToExit);
	} else {
		_sprite2.addPICTResourceFrame(kJumpMenuPICTID, false, 0, 0);
		_sprite2.addPICTResourceFrame(kJumpMenuHilitedPICTID, false, kJumpMenuHilitedLeft - kJumpMenuLeft, kJumpMenuHilitedTop - kJumpMenuTop);
		_sprite2.moveElementTo(kJumpMenuLeft, kJumpMenuTop);
		setCurrentActivation(kActivationReadyForJumpMenu);
	}

	_sprite2.setCurrentFrameIndex(0);
	_sprite2.show();
}

Hotspot *FullTSA::getItemScreenSpot(Item *item, DisplayElement *element) {
	switch (item->getObjectID()) {
	case kJourneymanKey:
		return g_allHotspots.findHotspotByID(kTSA22EastKeySpotID);
		break;
	case kPegasusBiochip:
		return g_allHotspots.findHotspotByID(kTSA23WestChipsSpotID);
		break;
	}

	return Neighborhood::getItemScreenSpot(item, element);
}

void FullTSA::dropItemIntoRoom(Item *item, Hotspot *dropSpot) {
	Neighborhood::dropItemIntoRoom(item, dropSpot);

	switch (item->getObjectID()) {
	case kKeyCard:
		if (dropSpot->getObjectID() == kTSAGTCardDropSpotID)
			startExtraSequence(kTSAGTCardSwipe, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kHistoricalLog:
		if (dropSpot->getObjectID() == kTSA0BNorthHistLogSpotID) {
			requestExtraSequence(kTSA0BNorthHistLogCloseWithLog, 0, kFilterNoInput);
			requestExtraSequence(kTSA0BNorthTurnRight, 0, kFilterNoInput);
			requestExtraSequence(kTSA0BEastZoomIn, kExtraCompletedFlag, kFilterNoInput);
			requestExtraSequence(kTSA0BComparisonStartup, kExtraCompletedFlag, kFilterNoInput);
			GameState.setScoringPutLogInReader(true);
		}
		break;
	}
}

uint FullTSA::getHistoricalLogIndex() {
	uint index;
	
	if (GameState.getTSASeenNoradNormal() && GameState.getTSASeenNoradAltered())
		index = 8;
	else
		index = 0;

	if (GameState.getTSASeenMarsNormal() && GameState.getTSASeenMarsAltered())
		index += 4;

	if (GameState.getTSASeenCaldoriaNormal() && GameState.getTSASeenCaldoriaAltered())
		index += 2;

	if (GameState.getTSASeenWSCNormal() && GameState.getTSASeenWSCAltered())
		index += 1;

	return index;
}

void FullTSA::handleInput(const Input &input, const Hotspot *cursorSpot) {
	switch (MakeRoomView(GameState.getCurrentRoom(), GameState.getCurrentDirection())) {
	case MakeRoomView(kTSA0B, kEast):
		if (GameState.getTSA0BZoomedIn() && !_navMovie.isRunning() && GameState.getT0BMonitorMode() == kMonitorNeutral) {
			switch (GameState.getTSAState()) {
			case kTSAPlayerInstalledHistoricalLog:
			case kTSABossSawHistoricalLog:
			case kRobotsAtCommandCenter:
			case kRobotsAtFrontDoor:
			case kRobotsAtReadyRoom:
				if (cursorSpot) {
					switch (cursorSpot->getObjectID()) {
					case kTSA0BEastCompareNoradSpotID:
						_sprite1.setCurrentFrameIndex(0);
						_sprite2.setCurrentFrameIndex(0);
						_sprite1.show();
						_sprite2.show();
						break;
					case kTSA0BEastCompareMarsSpotID:
						_sprite1.setCurrentFrameIndex(1);
						_sprite2.setCurrentFrameIndex(1);
						_sprite1.show();
						_sprite2.show();
						break;
					case kTSA0BEastCompareCaldoriaSpotID:
						_sprite1.setCurrentFrameIndex(2);
						_sprite2.setCurrentFrameIndex(2);
						_sprite1.show();
						_sprite2.show();
						break;
					case kTSA0BEastCompareWSCSpotID:
						_sprite1.setCurrentFrameIndex(3);
						_sprite2.setCurrentFrameIndex(3);
						_sprite1.show();
						_sprite2.show();
						break;
					default:
						_sprite1.hide();
						_sprite2.hide();
						break;
					}
				} else {
					_sprite1.hide();
					_sprite2.hide();
				}
				break;
			}
		}
		break;
	case MakeRoomView(kTSA0B, kNorth):
		if (GameState.getTSA0BZoomedIn() && !_navMovie.isRunning()) {
			switch (GameState.getTSAState()) {
			case kRobotsAtCommandCenter:
			case kRobotsAtFrontDoor:
			case kRobotsAtReadyRoom:
				if (cursorSpot) {
					switch (cursorSpot->getObjectID()) {
					case kTSA0BNorthRobotsToCommandCenterSpotID:
						_sprite1.setCurrentFrameIndex(kRedirectionCCRolloverSprite);
						_sprite1.show();
						break;
					case kTSA0BNorthRobotsToReadyRoomSpotID:
						_sprite1.setCurrentFrameIndex(kRedirectionRRRolloverSprite);
						_sprite1.show();
						break;
					case kTSA0BNorthRobotsToFrontDoorSpotID:
						_sprite1.setCurrentFrameIndex(kRedirectionFDRolloverSprite);
						_sprite1.show();
						break;
					default:
						_sprite1.hide();
						break;
					}
				} else {
					_sprite1.hide();
				}
				break;
			}
		}
		break;
	}

	Neighborhood::handleInput(input, cursorSpot);
}

void FullTSA::releaseSprites() {
	_sprite1.hide();
	_sprite2.hide();
	_sprite3.hide();
	_sprite1.discardFrames();
	_sprite2.discardFrames();
	_sprite3.discardFrames();
}

bool FullTSA::canSolve() {
	return GameState.getCurrentRoomAndView() == MakeRoomView(kTSA0B, kNorth) &&
		   GameState.getTSA0BZoomedIn() &&
		   (GameState.getTSAState() == kRobotsAtCommandCenter ||
		   GameState.getTSAState() == kRobotsAtFrontDoor ||
		   GameState.getTSAState() == kRobotsAtReadyRoom);
}

void FullTSA::doSolve() {
	// REROUTING ROBOTS

	_sprite1.setCurrentFrameIndex(kRedirectionFDDoorSprite);
	_sprite1.show();
	_vm->delayShell(1, 2);
	_sprite1.hide();

	switch (GameState.getTSAState()) {
	case kRobotsAtCommandCenter:
		GameState.setTSAState(kRobotsAtFrontDoor);
		_sprite2.setCurrentFrameIndex(kRedirectionNewTargetSprite);
		startExtraSequence(kTSA0BRobotsFromCommandCenterToFrontDoor, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kRobotsAtFrontDoor:
		// Nothing
		break;
	case kRobotsAtReadyRoom:
		GameState.setTSAState(kRobotsAtFrontDoor);
		_sprite2.setCurrentFrameIndex(kRedirectionNewTargetSprite);
		startExtraSequence(kTSA0BRobotsFromReadyRoomToFrontDoor, kExtraCompletedFlag, kFilterNoInput);
		break;
	}
}

void FullTSA::updateCursor(const Common::Point where, const Hotspot *cursorSpot) {
	if (cursorSpot) {
		switch (cursorSpot->getObjectID()) {
		case kTSA0BEastMonitorSpotID:
		case kTSA0BNorthMonitorSpotID:
		case kTSA0BWestMonitorSpotID:
		case kTSA22EastMonitorSpotID:
		case kTSA23WestMonitorSpotID:
			_vm->_cursor->setCurrentFrameIndex(1);
			return;
		case kTSA0BEastMonitorOutSpotID:
		case kTSA0BNorthMonitorOutSpotID:
		case kTSA0BWestMonitorOutSpotID:
			_vm->_cursor->setCurrentFrameIndex(2);
			return;
		}
	}

	Neighborhood::updateCursor(where, cursorSpot);
}

Common::String FullTSA::getNavMovieName() {
	return "Images/TSA/Full TSA.movie";
}

Common::String FullTSA::getSoundSpotsName() {
	return "Sounds/TSA/TSA Spots";
}

} // End of namespace Pegasus
