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

#ifndef PEGASUS_SCORING_H
#define PEGASUS_SCORING_H

#include "pegasus/types.h"

namespace Pegasus {

/////////////////////////////////////////////
//
//	Scoring.

const CoordType kDeathScreenScoreLeft = 151;
const CoordType kDeathScreenScoreTop = 212;
const CoordType kDeathScreenScoreWidth = 124;
const CoordType kDeathScreenScoreHeight = 12;
const CoordType kDeathScreenScoreSkipVert = -16;

//	Caldoria & TSA

const GameScoreType kSawINNScore					= 5;
const GameScoreType kTookShowerScore				= 2;
const GameScoreType kFixedHairScore				= 2;
const GameScoreType kGotKeyCardScore				= 5;
const GameScoreType kReadPaperScore				= 2;
const GameScoreType kLookThroughTelescopeScore		= 2;
const GameScoreType kSawCaldoriaKioskScore			= 2;
const GameScoreType kGoToTSAScore					= 3;

const GameScoreType kEnterTSAScore					= 2;
const GameScoreType kSawBust1Score					= 2;
const GameScoreType kSawBust2Score					= 2;
const GameScoreType kSawBust3Score					= 2;
const GameScoreType kSawBust4Score					= 2;
const GameScoreType kSawBust5Score					= 2;
const GameScoreType kSawBust6Score					= 2;
const GameScoreType kSawTheoryScore				= 4;
const GameScoreType kSawBackgroundScore			= 4;
const GameScoreType kSawProcedureScore				= 4;
const GameScoreType kGotJourneymanKeyScore			= 5;
const GameScoreType kGotPegasusBiochipScore		= 5;
const GameScoreType kGotBiosuitScore				= 5;
const GameScoreType kGoToPrehistoricScore			= 5;

const GameScoreType kPutLogInReaderScore			= 5;
const GameScoreType kSawCaldoriaNormalScore		= 2;
const GameScoreType kSawCaldoriaAlteredScore		= 2;
const GameScoreType kSawNoradNormalScore			= 2;
const GameScoreType kSawNoradAlteredScore			= 2;
const GameScoreType kSawMarsNormalScore			= 2;
const GameScoreType kSawMarsAlteredScore			= 2;
const GameScoreType kSawWSCNormalScore				= 2;
const GameScoreType kSawWSCAlteredScore			= 2;
const GameScoreType kWentToReadyRoom2Score			= 5;
const GameScoreType kWentAfterSinclairScore		= 5;
const GameScoreType kUsedCardBombScore				= 10;
const GameScoreType kShieldedCardBombScore			= 5;
const GameScoreType kStunnedSinclairScore			= 10;
const GameScoreType kDisarmedNukeScore				= 10;

const GameScoreType kMaxCaldoriaTSAScoreBefore		=	kSawINNScore +
														kTookShowerScore +
														kFixedHairScore +
														kGotKeyCardScore +
														kReadPaperScore +
														kLookThroughTelescopeScore +
														kSawCaldoriaKioskScore +
														kGoToTSAScore +
														kEnterTSAScore +
														kSawBust1Score +
														kSawBust2Score +
														kSawBust3Score +
														kSawBust4Score +
														kSawBust5Score +
														kSawBust6Score +
														kSawTheoryScore +
														kSawBackgroundScore +
														kSawProcedureScore +
														kGotJourneymanKeyScore +
														kGotPegasusBiochipScore +
														kGotBiosuitScore +
														kGoToPrehistoricScore +
														kPutLogInReaderScore +
														kSawCaldoriaNormalScore +
														kSawCaldoriaAlteredScore +
														kSawNoradNormalScore +
														kSawNoradAlteredScore +
														kSawMarsNormalScore +
														kSawMarsAlteredScore +
														kSawWSCNormalScore +
														kSawWSCAlteredScore +
														kWentToReadyRoom2Score;

const GameScoreType kMaxCaldoriaTSAScoreAfter		=	kWentAfterSinclairScore +
														kUsedCardBombScore +
														kShieldedCardBombScore +
														kStunnedSinclairScore +
														kDisarmedNukeScore;

const GameScoreType kMaxCaldoriaTSAScore			=	kMaxCaldoriaTSAScoreBefore +
														kMaxCaldoriaTSAScoreAfter;

//	Prehistoric

const GameScoreType kThrewBreakerScore				= 10;
const GameScoreType kExtendedBridgeScore			= 10;
const GameScoreType kGotHistoricalLogScore			= 5;
const GameScoreType kFinishedPrehistoricScore		= 10;

const GameScoreType kMaxPrehistoricScore			=	kThrewBreakerScore +
														kExtendedBridgeScore +
														kGotHistoricalLogScore +
														kFinishedPrehistoricScore;

//	Mars

const GameScoreType kThrownByRobotScore			= 3;
const GameScoreType kGotMarsCardScore				= 5;
const GameScoreType kSawMarsKioskScore				= 2;
const GameScoreType kSawTransportMapScore			= 2;
const GameScoreType kGotCrowBarScore				= 5;
const GameScoreType kTurnedOnTransportScore		= 5;
const GameScoreType kGotOxygenMaskScore			= 5;
const GameScoreType kAvoidedRobotScore				= 5;
const GameScoreType kActivatedPlatformScore		= 2;
const GameScoreType kUsedLiquidNitrogenScore		= 3;
const GameScoreType kUsedCrowBarScore				= 3;
const GameScoreType kFoundCardBombScore			= 4;
const GameScoreType kDisarmedCardBombScore			= 8;
const GameScoreType kGotCardBombScore				= 5;
const GameScoreType kThreadedMazeScore				= 5;
const GameScoreType kThreadedGearRoomScore			= 2;
const GameScoreType kEnteredShuttleScore			= 2;
const GameScoreType kEnteredLaunchTubeScore		= 4;
const GameScoreType kStoppedRobotsShuttleScore		= 10;
const GameScoreType kGotMarsOpMemChipScore			= 10;
const GameScoreType kFinishedMarsScore				= 10;

const GameScoreType kMaxMarsScore					=	kThrownByRobotScore +
														kGotMarsCardScore +
														kSawMarsKioskScore +
														kSawTransportMapScore +
														kGotCrowBarScore +
														kTurnedOnTransportScore +
														kGotOxygenMaskScore +
														kAvoidedRobotScore +
														kActivatedPlatformScore +
														kUsedLiquidNitrogenScore +
														kUsedCrowBarScore +
														kFoundCardBombScore +
														kDisarmedCardBombScore +
														kGotCardBombScore +
														kThreadedMazeScore +
														kThreadedGearRoomScore +
														kEnteredShuttleScore +
														kEnteredLaunchTubeScore +
														kStoppedRobotsShuttleScore +
														kGotMarsOpMemChipScore +
														kFinishedMarsScore;

//	Norad

const GameScoreType kSawSecurityMonitorScore		= 5;
const GameScoreType kFilledOxygenCanisterScore		= 5;
const GameScoreType kFilledArgonCanisterScore		= 5;
const GameScoreType kSawUnconsciousOperatorScore	= 5;
const GameScoreType kWentThroughPressureDoorScore	= 5;
const GameScoreType kPreppedSubScore				= 5;
const GameScoreType kEnteredSubScore				= 5;
const GameScoreType kExitedSubScore				= 10;
const GameScoreType kSawRobotAt54NorthScore		= 5;
const GameScoreType kPlayedWithClawScore			= 5;
const GameScoreType kUsedRetinalChipScore			= 5;
const GameScoreType kFinishedGlobeGameScore		= 10;
const GameScoreType kStoppedNoradRobotScore		= 10;
const GameScoreType kGotNoradOpMemChipScore		= 10;
const GameScoreType kFinishedNoradScore			= 10;

const GameScoreType kMaxNoradScore					=	kSawSecurityMonitorScore +
														kFilledOxygenCanisterScore +
														kFilledArgonCanisterScore +
														kSawUnconsciousOperatorScore +
														kWentThroughPressureDoorScore +
														kPreppedSubScore +
														kEnteredSubScore +
														kExitedSubScore +
														kSawRobotAt54NorthScore +
														kPlayedWithClawScore +
														kUsedRetinalChipScore +
														kFinishedGlobeGameScore +
														kStoppedNoradRobotScore +
														kGotNoradOpMemChipScore +
														kFinishedNoradScore;

//	WSC

const GameScoreType kRemovedDartScore				= 5;
const GameScoreType kAnalyzedDartScore				= 5;
const GameScoreType kBuiltAntidoteScore			= 5;
const GameScoreType kGotSinclairKeyScore			= 5;
const GameScoreType kGotArgonCanisterScore 		= 5;
const GameScoreType kGotNitrogenCanisterScore		= 5;
const GameScoreType kPlayedWithMessagesScore		= 2;
const GameScoreType kSawMorphExperimentScore		= 3;
const GameScoreType kEnteredSinclairOfficeScore	= 2;
const GameScoreType kSawBrochureScore				= 3;
const GameScoreType kSawSinclairEntry1Score		= 3;
const GameScoreType kSawSinclairEntry2Score		= 3;
const GameScoreType kSawSinclairEntry3Score		= 3;
const GameScoreType kSawWSCDirectoryScore			= 3;
const GameScoreType kUsedCrowBarInWSCScore			= 5;
const GameScoreType kFinishedPlasmaDodgeScore		= 10;
const GameScoreType kOpenedCatwalkScore			= 3;
const GameScoreType kStoppedWSCRobotScore			= 10;
const GameScoreType kGotWSCOpMemChipScore			= 10;
const GameScoreType kFinishedWSCScore				= 10;

const GameScoreType kMaxWSCScore					=	kRemovedDartScore +
														kAnalyzedDartScore +
														kBuiltAntidoteScore +
														kGotSinclairKeyScore +
														kGotArgonCanisterScore +
														kGotNitrogenCanisterScore +
														kPlayedWithMessagesScore +
														kSawMorphExperimentScore +
														kEnteredSinclairOfficeScore +
														kSawBrochureScore +
														kSawSinclairEntry1Score +
														kSawSinclairEntry2Score +
														kSawSinclairEntry3Score +
														kSawWSCDirectoryScore +
														kUsedCrowBarInWSCScore +
														kFinishedPlasmaDodgeScore +
														kOpenedCatwalkScore +
														kStoppedWSCRobotScore +
														kGotWSCOpMemChipScore +
														kFinishedWSCScore;

//	Gandhi

const GameScoreType kMarsGandhiScore				= 10;
const GameScoreType kNoradGandhiScore				= 10;
const GameScoreType kWSCGandhiScore				= 10;

const GameScoreType kMaxGandhiScore				=	kMarsGandhiScore +
														kNoradGandhiScore +
														kWSCGandhiScore;

const GameScoreType kMaxTotalScore					=	kMaxCaldoriaTSAScore +
														kMaxPrehistoricScore +
														kMaxMarsScore +
														kMaxNoradScore +
														kMaxWSCScore +
														kMaxGandhiScore;
} // End of namespace Pegasus

#endif