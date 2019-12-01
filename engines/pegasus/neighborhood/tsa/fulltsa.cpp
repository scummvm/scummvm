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

#include "pegasus/cursor.h"
#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/aichip.h"
#include "pegasus/items/biochips/opticalchip.h"
#include "pegasus/neighborhood/caldoria/caldoria.h"
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/prehistoric/prehistoric.h"
#include "pegasus/neighborhood/mars/constants.h"
#include "pegasus/neighborhood/tsa/fulltsa.h"
#include "pegasus/neighborhood/wsc/wsc.h"

namespace Pegasus {

// TSA PICTs:

enum {
	kTBPCloseBoxPICTID = 800,
	kTBPRewindPICTID = 801,
	kUnresolvedPICTID = 802,
	kResolvedPICTID = 803,
	kJumpMenuPICTID = 804,
	kJumpMenuHilitedPICTID = 805,
	kExitPICTID = 806,
	kExitHilitedPICTID = 807,
	kLeftRipPICTID = 808,
	kComparisonCloseBoxPICTID = 809,
	kComparisonLeftRewindPICTID = 810,
	kComparisonRightRewindPICTID = 811,
	kComparisonHiliteNoradPICTID = 812,
	kComparisonHiliteMarsPICTID = 813,
	kComparisonHiliteCaldoriaPICTID = 814,
	kComparisonHiliteWSCPICTID = 815,
	kComparisonChancesNoradPICTID = 816,
	kComparisonChancesMarsPICTID = 817,
	kComparisonChancesCaldoriaPICTID = 818,
	kComparisonChancesWSCPICTID = 819,
	kRedirectionCCRolloverPICTID = 820,
	kRedirectionRRRolloverPICTID = 821,
	kRedirectionFDRolloverPICTID = 822,
	kRedirectionCCDoorPICTID = 823,
	kRedirectionRRDoorPICTID = 824,
	kRedirectionFDDoorPICTID = 825,
	kRedirectionSecuredPICTID = 826,
	kRedirectionNewTargetPICTID = 827,
	kRedirectionClosePICTID = 828
};

static const int16 kCompassShift = 15;

enum {
	kFullTSAMovieScale = 600,
	kFullTSAFramesPerSecond = 15,
	kFullTSAFrameDuration = 40
};

// Alternate IDs.
static const AlternateID kAltTSANormal = 0;
static const AlternateID kAltTSARobotsAtReadyRoom = 1;
static const AlternateID kAltTSARobotsAtFrontDoor = 2;
static const AlternateID kAltTSARedAlert = 3;

// Room IDs.
enum {
	kTSA01 = 1,
	kTSA02 = 2,
	kTSA03 = 3,
	kTSA04 = 4,
	kTSA05 = 5,
	kTSA0A = 6,
	kTSA06 = 7,
	kTSA07 = 8,
	kTSA08 = 9,
	kTSA09 = 10,
	kTSA10 = 11,
	kTSA11 = 12,
	kTSA12 = 13,
	kTSA13 = 14,
	kTSA14 = 15,
	kTSA15 = 16,
	kTSA16 = 17,
	kTSA17 = 18,
	kTSA18 = 19,
	kTSA19 = 20,
	kTSA0B = 21,
	kTSA21Cyan = 22,
	kTSA22Cyan = 23,
	kTSA23Cyan = 24,
	kTSA24Cyan = 25,
	kTSA25Cyan = 26,
	kTSA21Red = 27,
	kTSA23Red = 29,
	kTSA24Red = 30,
	kTSA25Red = 31,
	kTSA26 = 32,
	kTSA27 = 33,
	kTSA28 = 34,
	kTSA29 = 35,
	kTSA30 = 36,
	kTSA31 = 37,
	kTSA32 = 38,
	kTSA33 = 39,
	kTSA34 = 40,
	kTSA35 = 41,
	kTSADeathRoom = 43
};

// Hot Spot Activation IDs.
enum {
	kActivateTSAReadyForCard = 1,
	kActivateTSAReadyToTransport = 2,
	kActivateTSARobotsAwake = 3,
	kActivateTSA0BZoomedOut = 4,
	kActivateTSA0BZoomedIn = 5,
	kActivateTSA0BComparisonVideo = 6,
	kActivationLogReaderOpen = 7,
	kActivateTSA0BTBPVideo = 8,
	kActivationDoesntHaveKey = 9,
	kActivationKeyVaultOpen = 10,
	kActivationDoesntHaveChips = 11,
	kActivationChipVaultOpen = 12,
	kActivationJumpToPrehistoric = 13,
	kActivationJumpToNorad = 14,
	kActivationJumpToMars = 15,
	kActivationJumpToWSC = 16,
	kActivationReadyToExit = 17,
	kActivationReadyForJumpMenu = 18,
	kActivationMainJumpMenu = 19
};

// Hot Spot IDs.
enum {
	kTSAGTCardDropSpotID = 5000,
	kTSAGTTokyoSpotID = 5001,
	kTSAGTCaldoriaSpotID = 5002,
	kTSAGTBeachSpotID = 5003,
	kTSAGTOtherSpotID = 5004,
	kTSA02DoorSpotID = 5005,
	kTSA03EastJimenezSpotID = 5006,
	kTSA03WestCrenshawSpotID = 5007,
	kTSA04EastMatsumotoSpotID = 5008,
	kTSA04WestCastilleSpotID = 5009,
	kTSA05EastSinclairSpotID = 5010,
	kTSA05WestWhiteSpotID = 5011,
	kTSA0AEastSpotID = 5012,
	kTSA0AWastSpotID = 5013,
	kTSA0BEastMonitorSpotID = 5014,
	kTSA0BEastMonitorOutSpotID = 5015,
	kTSA0BEastCompareNoradSpotID = 5016,
	kTSA0BEastCompareMarsSpotID = 5017,
	kTSA0BEastCompareCaldoriaSpotID = 5018,
	kTSA0BEastCompareWSCSpotID = 5019,
	kTSA0BEastLeftRewindSpotID = 5020,
	kTSA0BEastLeftPlaySpotID = 5021,
	kTSA0BEastRightRewindSpotID = 5022,
	kTSA0BEastRightPlaySpotID = 5023,
	kTSA0BEastCloseVideoSpotID = 5024,
	kTSA0BNorthMonitorSpotID = 5025,
	kTSA0BNorthMonitorOutSpotID = 5026,
	kTSA0BNorthHistLogSpotID = 5027,
	kTSA0BNorthRobotsToCommandCenterSpotID = 5028,
	kTSA0BNorthRobotsToReadyRoomSpotID = 5029,
	kTSA0BNorthRobotsToFrontDoorSpotID = 5030,
	kTSA0BWestMonitorSpotID = 5031,
	kTSA0BWestMonitorOutSpotID = 5032,
	kTSA0BWestTheorySpotID = 5033,
	kTSA0BWestBackgroundSpotID = 5034,
	kTSA0BWestProcedureSpotID = 5035,
	kTSA0BWestCloseVideoSpotID = 5036,
	kTSA0BWestPlayVideoSpotID = 5037,
	kTSA0BWestRewindVideoSpotID = 5038,
	kTSA22EastMonitorSpotID = 5039,
	kTSA22EastKeySpotID = 5040,
	kTSA23WestMonitorSpotID = 5041,
	kTSA23WestChipsSpotID = 5042,
	kTSA34NorthDoorSpotID = 5043,
	kTSA37NorthJumpToPrehistoricSpotID = 5044,
	kTSA37NorthJumpToNoradSpotID = 5045,
	kTSA37NorthCancelNoradSpotID = 5046,
	kTSA37NorthJumpToMarsSpotID = 5047,
	kTSA37NorthCancelMarsSpotID = 5048,
	kTSA37NorthJumpToWSCSpotID = 5049,
	kTSA37NorthCancelWSCSpotID = 5050,
	kTSA37NorthExitSpotID = 5051,
	kTSA37NorthJumpMenuSpotID = 5052,
	kTSA37NorthNoradMenuSpotID = 5053,
	kTSA37NorthMarsMenuSpotID = 5054,
	kTSA37NorthWSCMenuSpotID = 5055
};

// Extra sequence IDs.
enum {
	kTSATransporterArrowLoop = 0,
	kTSAArriveFromCaldoria = 1,
	kTSAGTOtherChoice = 2,
	kTSAGTCardSwipe = 3,
	kTSAGTSelectCaldoria = 4,
	kTSAGTGoToCaldoria = 5,
	kTSAGTSelectBeach = 6,
	kTSAGTGoToBeach = 7,
	kTSAGTArriveAtBeach = 8,
	kTSAGTSelectTokyo = 9,
	kTSAGTGoToTokyo = 10,
	kTSAGTArriveAtTokyo = 11,
	kTSA02NorthZoomIn = 12,
	kTSA02NorthTenSecondDoor = 13,
	kTSA02NorthZoomOut = 14,
	kTSA02NorthDoorWithAgent3 = 15,
	kTSA03JimenezZoomIn = 16,
	kTSA03JimenezSpeech = 17,
	kTSA03JimenezZoomOut = 18,
	kTSA03CrenshawZoomIn = 19,
	kTSA03CrenshawSpeech = 20,
	kTSA03CrenshawZoomOut = 21,
	kTSA03SouthRobotDeath = 22,
	kTSA04NorthRobotGreeting = 23,
	kTSA04MatsumotoZoomIn = 24,
	kTSA04MatsumotoSpeech = 25,
	kTSA04MatsumotoZoomOut = 26,
	kTSA04CastilleZoomIn = 27,
	kTSA04CastilleSpeech = 28,
	kTSA04CastilleZoomOut = 29,
	kTSA05SinclairZoomIn = 30,
	kTSA05SinclairSpeech = 31,
	kTSA05SinclairZoomOut = 32,
	kTSA05WhiteZoomIn = 33,
	kTSA05WhiteSpeech = 34,
	kTSA05WhiteZoomOut = 35,
	kTSA0AEastRobot = 36,
	kTSA0AWestRobot = 37,
	kTSA16NorthRobotDeath = 38,
	kTSA0BEastZoomIn = 39,
	kTSA0BEastZoomedView = 40,
	kTSA0BEastZoomOut = 41,
	kTSA0BEastTurnLeft = 42,
	kTSA0BComparisonStartup = 43,
	kTSA0BComparisonView0000 = 44,
	kTSA0BComparisonView0002 = 45,
	kTSA0BComparisonView0020 = 46,
	kTSA0BComparisonView0022 = 47,
	kTSA0BComparisonView0200 = 48,
	kTSA0BComparisonView0202 = 49,
	kTSA0BComparisonView0220 = 50,
	kTSA0BComparisonView0222 = 51,
	kTSA0BComparisonView2000 = 52,
	kTSA0BComparisonView2002 = 53,
	kTSA0BComparisonView2020 = 54,
	kTSA0BComparisonView2022 = 55,
	kTSA0BComparisonView2200 = 56,
	kTSA0BComparisonView2202 = 57,
	kTSA0BComparisonView2220 = 58,
	kTSA0BComparisonView2222 = 59,
	kTSA0BNoradComparisonView = 60,
	kTSA0BNoradUnaltered = 61,
	kTSA0BNoradAltered = 62,
	kTSA0BMarsComparisonView = 63,
	kTSA0BMarsUnaltered = 64,
	kTSA0BMarsAltered = 65,
	kTSA0BWSCComparisonView = 66,
	kTSA0BWSCUnaltered = 67,
	kTSA0BWSCAltered = 68,
	kTSA0BCaldoriaComparisonView = 69,
	kTSA0BCaldoriaUnaltered = 70,
	kTSA0BCaldoriaAltered = 71,
	kTSA0BNorthZoomIn = 72,
	kTSA0BNorthZoomedView = 73,
	kTSA0BNorthZoomOut = 74,
	kTSA0BNorthTurnLeft = 75,
	kTSA0BNorthTurnRight = 76,
	kTSA0BNorthHistLogOpen = 77,
	kTSA0BNorthHistLogClose = 78,
	kTSA0BNorthHistLogCloseWithLog = 79,
	kTSA0BNorthCantChangeHistory = 80,
	kTSA0BNorthYoureBusted = 81,
	kTSA0BNorthFinallyHappened = 82,
	kTSA0BShowRip1 = 83,
	kTSA0BNorthRipView1 = 84,
	kTSA0BShowRip2 = 85,
	kTSA0BShowGuardRobots = 86,
	kTSA0BAIInterruption = 87,
	kTSA0BRobotsToCommandCenter = 88,
	kTSA0BNorthRobotsAtCCView = 89,
	kTSA0BNorthRobotsAtRRView = 90,
	kTSA0BNorthRobotsAtFDView = 91,
	kTSA0BRobotsFromCommandCenterToReadyRoom = 92,
	kTSA0BRobotsFromReadyRoomToCommandCenter = 93,
	kTSA0BRobotsFromCommandCenterToFrontDoor = 94,
	kTSA0BRobotsFromFrontDoorToCommandCenter = 95,
	kTSA0BRobotsFromFrontDoorToReadyRoom = 96,
	kTSA0BRobotsFromReadyRoomToFrontDoor = 97,
	kTSA0BWestZoomIn = 98,
	kTSA0BWestZoomedView = 99,
	kTSA0BWestZoomOut = 100,
	kTSA0BWestTurnRight = 101,
	kTSA0BTBPTheoryHighlight = 102,
	kTSA0BTBPBackgroundHighlight = 103,
	kTSA0BTBPProcedureHighlight = 104,
	kTSA0BTBPTheory = 105,
	kTSA0BTBPBackground = 106,
	kTSA0BTBPProcedure = 107,
	kTSA0BRipAlarmScreen = 108,
	kTSA22RedEastZoomInSequence = 109,
	kTSA22RedEastVaultViewWithKey = 110,
	kTSA22RedEastVaultViewNoKey = 111,
	kTSA23RedWestVaultZoomInSequence = 112,
	kTSA23RedWestVaultViewWithChips = 113,
	kTSA23RedWestVaultViewNoChips = 114,
	kTSA25NorthDeniedNoKey = 115,
	kTSA25NorthDeniedNoChip = 116,
	kTSA25NorthPutOnSuit = 117,
	kTSA25NorthAlreadyHaveSuit = 118,
	kTSA25NorthDescending1 = 119,
	kTSA25NorthDescending2 = 120,
	kTSA37HorseToAI1 = 121,
	kTSA37PegasusAI1 = 122,
	kTSA37AI1ToCommissioner1 = 123,
	kTSA37Commissioner1 = 124,
	kTSA37Commissioner1ToZoom = 125,
	kTSA37ZoomToPrehistoric = 126,
	kTSA37PrehistoricToAI2 = 127,
	kTSA37PegasusAI2 = 128,
	kTSA37AI2ToPrehistoric = 129,
	kTSA37PrehistoricToDepart = 130,
	kTSA37PegasusDepart = 131,
	kTSA37TimeJumpToPegasus = 132,
	kTSA37RecallToDownload = 133,
	kTSA37DownloadToColonel1 = 134,
	kTSA37Colonel1 = 135,
	kTSA37Colonel1ToReviewRequired = 136,
	kTSA37ReviewRequiredToExit = 137,
	kTSA37ExitHilited = 138,
	kTSA37ExitToHorse = 139,
	kTSA37HorseToColonel2 = 140,
	kTSA37Colonel2 = 141,
	kTSA37PegasusAI3 = 142,
	kTSA37AI3ToHorse = 143,
	kTSA37HorseToZoom = 144,
	kTSA37ZoomToMainMenu = 145,
	kTSA37MainMenuToAI4 = 146,
	kTSA37PegasusAI4 = 147,
	kTSA37AI4ToMainMenu = 148,
	kTSA37JumpMenu000 = 149,
	kTSA37JumpMenu001 = 150,
	kTSA37JumpMenu010 = 151,
	kTSA37JumpMenu011 = 152,
	kTSA37JumpMenu100 = 153,
	kTSA37JumpMenu101 = 154,
	kTSA37JumpMenu110 = 155,
	kTSA37JumpMenu111 = 156,
	kTSA37JumpToWSCMenu = 157,
	kTSA37CancelWSC = 158,
	kTSA37JumpToWSC = 159,
	kTSA37WSCToAI5 = 160,
	kTSA37PegasusAI5 = 161,
	kTSA37AI5ToWSC = 162,
	kTSA37WSCToDepart = 163,
	kTSA37JumpToMarsMenu = 164,
	kTSA37CancelMars = 165,
	kTSA37JumpToMars = 166,
	kTSA37MarsToAI6 = 167,
	kTSA37PegasusAI6 = 168,
	kTSA37AI6ToMars = 169,
	kTSA37MarsToDepart = 170,
	kTSA37JumpToNoradMenu = 171,
	kTSA37CancelNorad = 172,
	kTSA37JumpToNorad = 173,
	kTSA37NoradToAI7 = 174,
	kTSA37PegasusAI7 = 175,
	kTSA37AI7ToNorad = 176,
	kTSA37NoradToDepart = 177,
	kTSA37EnvironmentalScan = 178,
	kTSA37DownloadToMainMenu = 179,
	kTSA37DownloadToOpMemReview = 180,
	kTSA37OpMemReviewToMainMenu = 181,
	kTSA37OpMemReviewToAllClear = 182,
	kTSA37AllClearToCongratulations = 183,
	kTSA37Congratulations = 184,
	kTSA37CongratulationsToExit = 185
};

const DisplayOrder kRipTimerOrder = kMonitorLayer;

enum {
	kUnresolvedLeft = kNavAreaLeft + 14,
	kUnresolvedTop = kNavAreaTop + 236,

	kResolvedLeft = kNavAreaLeft + 36,
	kResolvedTop = kNavAreaTop + 236,

	kJumpMenuLeft = kNavAreaLeft + 360,
	kJumpMenuTop = kNavAreaTop + 202,

	kJumpMenuHilitedLeft = kNavAreaLeft + 354,
	kJumpMenuHilitedTop = kNavAreaTop + 196,

	kExitLeft = kNavAreaLeft + 360,
	kExitTop = kNavAreaTop + 216,

	kExitHilitedLeft = kNavAreaLeft + 354,
	kExitHilitedTop = kNavAreaTop + 210,

	kRipTimerLeft = kNavAreaLeft + 95,
	kRipTimerTop = kNavAreaTop + 87,

	kTBPCloseLeft = kNavAreaLeft + 30,
	kTBPCloseTop = kNavAreaTop + 16,

	kTBPRewindLeft = kNavAreaLeft + 86,
	kTBPRewindTop = kNavAreaTop + 218,

	kComparisonCloseLeft = kNavAreaLeft + 50,
	kComparisonCloseTop = kNavAreaTop + 14,

	kComparisonLeftRewindLeft = kNavAreaLeft + 96,
	kComparisonLeftRewindTop = kNavAreaTop + 190,

	kComparisonRightRewindLeft = kNavAreaLeft + 282,
	kComparisonRightRewindTop = kNavAreaTop + 190,

	kComparisonHiliteSpriteLeft = kNavAreaLeft + 45,
	kComparisonHiliteSpriteTop = kNavAreaTop + 65,

	kComparisonHiliteNoradLeft = kNavAreaLeft + 45,
	kComparisonHiliteNoradTop = kNavAreaTop + 65,

	kComparisonHiliteMarsLeft = kNavAreaLeft + 45 + 4,
	kComparisonHiliteMarsTop = kNavAreaTop + 65 + 23,

	kComparisonHiliteCaldoriaLeft = kNavAreaLeft + 45 + 7,
	kComparisonHiliteCaldoriaTop = kNavAreaTop + 65 + 46,

	kComparisonHiliteWSCLeft = kNavAreaLeft + 45 + 11,
	kComparisonHiliteWSCTop = kNavAreaTop + 65 + 68,

	kComparisonChancesSpriteLeft = kNavAreaLeft + 148,
	kComparisonChancesSpriteTop = kNavAreaTop + 162,

	kComparisonChancesNoradLeft = kNavAreaLeft + 148,
	kComparisonChancesNoradTop = kNavAreaTop + 162,

	kComparisonChancesMarsLeft = kNavAreaLeft + 148,
	kComparisonChancesMarsTop = kNavAreaTop + 162,

	kComparisonChancesCaldoriaLeft = kNavAreaLeft + 148,
	kComparisonChancesCaldoriaTop = kNavAreaTop + 162 + 1,

	kComparisonChancesWSCLeft = kNavAreaLeft + 148,
	kComparisonChancesWSCTop = kNavAreaTop + 162,

	kRedirectionSprite1Left = kNavAreaLeft + 58,
	kRedirectionSprite1Top = kNavAreaTop + 16,

	kRedirectionSprite2Left = kNavAreaLeft + 36,
	kRedirectionSprite2Top = kNavAreaTop + 166,

	kRedirectionCCRolloverLeft = kNavAreaLeft + 58,
	kRedirectionCCRolloverTop = kNavAreaTop + 16,

	kRedirectionRRRolloverLeft = kNavAreaLeft + 430,
	kRedirectionRRRolloverTop = kNavAreaTop + 30,

	kRedirectionFDRolloverLeft = kNavAreaLeft + 278,
	kRedirectionFDRolloverTop = kNavAreaTop + 160,

	kRedirectionCCDoorLeft = kNavAreaLeft + 174,
	kRedirectionCCDoorTop = kNavAreaTop + 36,

	kRedirectionRRDoorLeft = kNavAreaLeft + 418,
	kRedirectionRRDoorTop = kNavAreaTop + 32,

	kRedirectionFDDoorLeft = kNavAreaLeft + 298,
	kRedirectionFDDoorTop = kNavAreaTop + 240,

	kRedirectionSecuredLeft = kNavAreaLeft + 36,
	kRedirectionSecuredTop = kNavAreaTop + 166,

	kRedirectionNewTargetLeft = kNavAreaLeft + 36,
	kRedirectionNewTargetTop = kNavAreaTop + 166,

	kRedirectionCloseLeft = kNavAreaLeft + 56,
	kRedirectionCloseTop = kNavAreaTop + 220
};

static const TimeValue kTSABumpIntoWallIn = 0;
static const TimeValue kTSABumpIntoWallOut = 148;

static const TimeValue kTSAGTDoorCloseIn = 148;
static const TimeValue kTSAGTDoorCloseOut = 1570;

static const TimeValue kTSANoOtherDestinationIn = 1570;
static const TimeValue kTSANoOtherDestinationOut = 3601;

static const TimeValue kTSAEntryDoorCloseIn = 3601;
static const TimeValue kTSAEntryDoorCloseOut = 4200;

static const TimeValue kTSAInsideDoorCloseIn = 4200;
static const TimeValue kTSAInsideDoorCloseOut = 4800;

static const TimeValue kTSAVaultCloseIn = 4800;
static const TimeValue kTSAVaultCloseOut = 5388;

static const TimeValue kTSAPegasusDoorCloseIn = 5388;
static const TimeValue kTSAPegasusDoorCloseOut = 6457;

enum {
	kPegasusUnresolved = false,
	kPegasusResolved = true,
	kPegasusCantExit = false,
	kPegasusCanExit = true
};

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

static const ExtraID s_historicalLogViews[16] = {
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

enum {
	kRedirectionCCRolloverSprite = 0,
	kRedirectionRRRolloverSprite = 1,
	kRedirectionFDRolloverSprite = 2,
	kRedirectionCCDoorSprite = 3,
	kRedirectionRRDoorSprite = 4,
	kRedirectionFDDoorSprite = 5,
	kRedirectionCloseSprite = 6,
	kRedirectionSecuredSprite = 0,
	kRedirectionNewTargetSprite = 1
};

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
	// WORKAROUND: If the timer isn't running, don't run the following code.
	// Fixes use of the code when it shouldn't be running (since this is an
	// IdlerAnimation, this is called on useIdleTime() but this specific
	// timer only makes sense when used as an actual timer).
	if (!isRunning())
		return;

	Common::Rect bounds;
	getBounds(bounds);

	CoordType newMiddle = bounds.left + bounds.width() * newTime / getDuration();

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

void FullTSA::dieUncreatedInTSA() {
	die(kDeathUncreatedInTSA);
}

void FullTSA::start() {
	g_energyMonitor->stopEnergyDraining();

	if (!GameState.getScoringEnterTSA()) {
		_utilityFuse.primeFuse(GameState.getTSAFuseTimeLimit());
		_utilityFuse.setFunctor(new Common::Functor0Mem<void, FullTSA>(this, &FullTSA::dieUncreatedInTSA));
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
		RoomID room = GameState.getCurrentRoom();

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
		default:
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
	RoomID room = GameState.getCurrentRoom();

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

short FullTSA::getStaticCompassAngle(const RoomID room, const DirectionConstant dir) {
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
	default:
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
	default:
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

TimeValue FullTSA::getViewTime(const RoomID room, const DirectionConstant direction) {
	ExtraID extraID = 0xffffffff;

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
	default:
		break;
	}

	if (extraID != 0xffffffff) {
		ExtraTable::Entry entry;
		getExtraEntry(extraID, entry);
		return entry.movieEnd - 1;
	}

	return Neighborhood::getViewTime(room, direction);
}

void FullTSA::findSpotEntry(const RoomID room, const DirectionConstant direction, SpotFlags flags, SpotTable::Entry &entry) {
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
		biochip = (BiochipItem *)_vm->getAllItems().findItemByID(kMapBiochip);
		_vm->addItemToBiochips(biochip);
		GameState.setScoringGotPegasusBiochip(true);
		break;
	default:
		break;
	}
}

void FullTSA::playExtraMovie(const ExtraTable::Entry &extraEntry, const NotificationFlags flags, const InputBits interruptionInput) {
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
	default:
		break;
	}

	Neighborhood::startDoorOpenMovie(startTime, stopTime);
}

InputBits FullTSA::getInputFilter() {
	InputBits result = Neighborhood::getInputFilter();

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
	default:
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
	default:
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
	default:
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
	default:
		break;
	}

	Neighborhood::openDoor();
}

CanMoveForwardReason FullTSA::canMoveForward(ExitTable::Entry &entry) {
	if (GameState.getCurrentRoomAndView() == MakeRoomView(kTSA25Red, kNorth))
		return kCantMoveBlocked;

	return Neighborhood::canMoveForward(entry);
}

CanOpenDoorReason FullTSA::canOpenDoor(DoorTable::Entry &entry) {
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
	default:
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
		break;
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
			_vm->getAllHotspots().activateOneHotspot(kTSA02DoorSpotID);
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
					_vm->getAllHotspots().activateOneHotspot(kTSA0BEastCompareNoradSpotID);
					_vm->getAllHotspots().activateOneHotspot(kTSA0BEastCompareMarsSpotID);
					_vm->getAllHotspots().activateOneHotspot(kTSA0BEastCompareCaldoriaSpotID);
					_vm->getAllHotspots().activateOneHotspot(kTSA0BEastCompareWSCSpotID);
				}
				break;
			default:
				break;
			}
		break;
	case MakeRoomView(kTSA0B, kNorth):
		if (GameState.getTSA0BZoomedIn())
			switch (GameState.getTSAState()) {
			case kRobotsAtCommandCenter:
			case kRobotsAtFrontDoor:
			case kRobotsAtReadyRoom:
				_vm->getAllHotspots().activateOneHotspot(kTSA0BNorthRobotsToCommandCenterSpotID);
				_vm->getAllHotspots().activateOneHotspot(kTSA0BNorthRobotsToReadyRoomSpotID);
				_vm->getAllHotspots().activateOneHotspot(kTSA0BNorthRobotsToFrontDoorSpotID);
				break;
			default:
				break;
			}
		break;
	default:
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
		default:
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
		default:
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
		default:
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
	ExtraID jumpMenuView = kTSA37JumpMenu000;

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
	InputDevice.waitInput(kFilterAllButtons);

	if ((GameState.getT0BMonitorMode() & kPlayingTBPMask) == 0) {
		ExtraID extra;

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
		default:
			error("Invalid monitor mode");
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

void FullTSA::initializeTBPMonitor(const int newMode, const ExtraID highlightExtra) {
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

	// Only allow input if we're not in the middle of series of queue requests.
	if (actionQueueEmpty())
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

void FullTSA::initializeComparisonMonitor(const int newMode, const ExtraID comparisonView) {
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
	InputDevice.waitInput(kFilterAllButtons);

	if ((GameState.getT0BMonitorMode() & kPlayingLeftComparisonMask) == 0) {
		ExtraID extra;

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
		default:
			error("Invalid monitor mode");
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
	InputDevice.waitInput(kFilterAllButtons);

	if ((GameState.getT0BMonitorMode() & kPlayingRightComparisonMask) == 0) {
		ExtraID extra;

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
		default:
			error("Invalid monitor mode");
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
	default:
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

void FullTSA::checkContinuePoint(const RoomID room, const DirectionConstant direction) {
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
	default:
		break;
	}
}

void FullTSA::arriveAt(const RoomID room, const DirectionConstant direction) {
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
			default:
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
			default:
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
			default:
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
	default:
		break;
	}
}

void FullTSA::checkRobotLocations(const RoomID room, const DirectionConstant dir) {
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
		default:
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
			default:
				break;
			}
		}
		break;
	default:
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
		initializePegasusButtons(false);
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
		initializePegasusButtons(true);
		break;
	default:
		break;
	}
}

void FullTSA::turnTo(const DirectionConstant newDirection) {
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
		default:
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
		default:
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
		default:
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
	default:
		break;
	}

	// Make sure the TBP monitor is forced neutral.
	GameState.setT0BMonitorMode(kMonitorNeutral);
}

void FullTSA::closeDoorOffScreen(const RoomID room, const DirectionConstant) {
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
	default:
		break;
	}
}

void FullTSA::receiveNotification(Notification *notification, const NotificationFlags flags) {
	ExtraID lastExtra = _lastExtra;

	if ((flags & kExtraCompletedFlag) != 0) {
		switch (lastExtra) {
		case kTSA0BEastTurnLeft:
			// Need to check this here because turnTo will call _navMovie.stop,
			// so it has to happen before Neighborhood::receiveNotification,
			// which may end up starting another sequence...
			turnTo(kNorth);
			break;
		default:
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
			item = (InventoryItem *)_vm->getAllItems().findItemByID(kKeyCard);
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
			default:
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
			default:
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
				GameState.setMarsSawRobotLeave(false);
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
			default:
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
			if (_vm->itemInBiochips(kOpticalBiochip)) {
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
				default:
					break;
				}
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
			initializePegasusButtons(kPegasusUnresolved);
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
			initializePegasusButtons(true);
			break;
		default:
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

void FullTSA::initializePegasusButtons(bool resolved) {
	if (resolved) {
		_sprite1.addPICTResourceFrame(kResolvedPICTID, false, 0, 0);
		_sprite1.moveElementTo(kResolvedLeft, kResolvedTop);
	} else {
		_sprite1.addPICTResourceFrame(kUnresolvedPICTID, false, 0, 0);
		_sprite1.moveElementTo(kUnresolvedLeft, kUnresolvedTop);
	}

	_sprite1.setCurrentFrameIndex(0);
	_sprite1.show();

	_sprite2.addPICTResourceFrame(kExitPICTID, false, kExitLeft - kExitHilitedLeft, kExitTop - kExitHilitedTop);
	_sprite2.addPICTResourceFrame(kExitHilitedPICTID, false, 0, 0);
	_sprite2.moveElementTo(kExitHilitedLeft, kExitHilitedTop);
	setCurrentActivation(kActivationReadyToExit);
	_sprite2.setCurrentFrameIndex(0);
	_sprite2.show();
}

Hotspot *FullTSA::getItemScreenSpot(Item *item, DisplayElement *element) {
	switch (item->getObjectID()) {
	case kJourneymanKey:
		return _vm->getAllHotspots().findHotspotByID(kTSA22EastKeySpotID);
		break;
	case kPegasusBiochip:
		return _vm->getAllHotspots().findHotspotByID(kTSA23WestChipsSpotID);
		break;
	default:
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
	default:
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
			default:
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
			default:
				break;
			}
		}
		break;
	default:
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
	default:
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
		default:
			break;
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
