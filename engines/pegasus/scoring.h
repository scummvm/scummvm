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

const tCoordType kDeathScreenScoreLeft = 151;
const tCoordType kDeathScreenScoreTop = 212;
const tCoordType kDeathScreenScoreWidth = 124;
const tCoordType kDeathScreenScoreHeight = 12;
const tCoordType kDeathScreenScoreSkipVert = -16;

//	Caldoria & TSA

const tGameScoreType kSawINNScore					= 5;
const tGameScoreType kTookShowerScore				= 2;
const tGameScoreType kFixedHairScore				= 2;
const tGameScoreType kGotKeyCardScore				= 5;
const tGameScoreType kReadPaperScore				= 2;
const tGameScoreType kLookThroughTelescopeScore		= 2;
const tGameScoreType kSawCaldoriaKioskScore			= 2;
const tGameScoreType kGoToTSAScore					= 3;

const tGameScoreType kEnterTSAScore					= 2;
const tGameScoreType kSawBust1Score					= 2;
const tGameScoreType kSawBust2Score					= 2;
const tGameScoreType kSawBust3Score					= 2;
const tGameScoreType kSawBust4Score					= 2;
const tGameScoreType kSawBust5Score					= 2;
const tGameScoreType kSawBust6Score					= 2;
const tGameScoreType kSawTheoryScore				= 4;
const tGameScoreType kSawBackgroundScore			= 4;
const tGameScoreType kSawProcedureScore				= 4;
const tGameScoreType kGotJourneymanKeyScore			= 5;
const tGameScoreType kGotPegasusBiochipScore		= 5;
const tGameScoreType kGotBiosuitScore				= 5;
const tGameScoreType kGoToPrehistoricScore			= 5;

const tGameScoreType kPutLogInReaderScore			= 5;
const tGameScoreType kSawCaldoriaNormalScore		= 2;
const tGameScoreType kSawCaldoriaAlteredScore		= 2;
const tGameScoreType kSawNoradNormalScore			= 2;
const tGameScoreType kSawNoradAlteredScore			= 2;
const tGameScoreType kSawMarsNormalScore			= 2;
const tGameScoreType kSawMarsAlteredScore			= 2;
const tGameScoreType kSawWSCNormalScore				= 2;
const tGameScoreType kSawWSCAlteredScore			= 2;
const tGameScoreType kWentToReadyRoom2Score			= 5;
const tGameScoreType kWentAfterSinclairScore		= 5;
const tGameScoreType kUsedCardBombScore				= 10;
const tGameScoreType kShieldedCardBombScore			= 5;
const tGameScoreType kStunnedSinclairScore			= 10;
const tGameScoreType kDisarmedNukeScore				= 10;

const tGameScoreType kMaxCaldoriaTSAScoreBefore		=	kSawINNScore +
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

const tGameScoreType kMaxCaldoriaTSAScoreAfter		=	kWentAfterSinclairScore +
														kUsedCardBombScore +
														kShieldedCardBombScore +
														kStunnedSinclairScore +
														kDisarmedNukeScore;

const tGameScoreType kMaxCaldoriaTSAScore			=	kMaxCaldoriaTSAScoreBefore +
														kMaxCaldoriaTSAScoreAfter;

//	Prehistoric

const tGameScoreType kThrewBreakerScore				= 10;
const tGameScoreType kExtendedBridgeScore			= 10;
const tGameScoreType kGotHistoricalLogScore			= 5;
const tGameScoreType kFinishedPrehistoricScore		= 10;

const tGameScoreType kMaxPrehistoricScore			=	kThrewBreakerScore +
														kExtendedBridgeScore +
														kGotHistoricalLogScore +
														kFinishedPrehistoricScore;

//	Mars

const tGameScoreType kThrownByRobotScore			= 3;
const tGameScoreType kGotMarsCardScore				= 5;
const tGameScoreType kSawMarsKioskScore				= 2;
const tGameScoreType kSawTransportMapScore			= 2;
const tGameScoreType kGotCrowBarScore				= 5;
const tGameScoreType kTurnedOnTransportScore		= 5;
const tGameScoreType kGotOxygenMaskScore			= 5;
const tGameScoreType kAvoidedRobotScore				= 5;
const tGameScoreType kActivatedPlatformScore		= 2;
const tGameScoreType kUsedLiquidNitrogenScore		= 3;
const tGameScoreType kUsedCrowBarScore				= 3;
const tGameScoreType kFoundCardBombScore			= 4;
const tGameScoreType kDisarmedCardBombScore			= 8;
const tGameScoreType kGotCardBombScore				= 5;
const tGameScoreType kThreadedMazeScore				= 5;
const tGameScoreType kThreadedGearRoomScore			= 2;
const tGameScoreType kEnteredShuttleScore			= 2;
const tGameScoreType kEnteredLaunchTubeScore		= 4;
const tGameScoreType kStoppedRobotsShuttleScore		= 10;
const tGameScoreType kGotMarsOpMemChipScore			= 10;
const tGameScoreType kFinishedMarsScore				= 10;

const tGameScoreType kMaxMarsScore					=	kThrownByRobotScore +
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

const tGameScoreType kSawSecurityMonitorScore		= 5;
const tGameScoreType kFilledOxygenCanisterScore		= 5;
const tGameScoreType kFilledArgonCanisterScore		= 5;
const tGameScoreType kSawUnconsciousOperatorScore	= 5;
const tGameScoreType kWentThroughPressureDoorScore	= 5;
const tGameScoreType kPreppedSubScore				= 5;
const tGameScoreType kEnteredSubScore				= 5;
const tGameScoreType kExitedSubScore				= 10;
const tGameScoreType kSawRobotAt54NorthScore		= 5;
const tGameScoreType kPlayedWithClawScore			= 5;
const tGameScoreType kUsedRetinalChipScore			= 5;
const tGameScoreType kFinishedGlobeGameScore		= 10;
const tGameScoreType kStoppedNoradRobotScore		= 10;
const tGameScoreType kGotNoradOpMemChipScore		= 10;
const tGameScoreType kFinishedNoradScore			= 10;

const tGameScoreType kMaxNoradScore					=	kSawSecurityMonitorScore +
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

const tGameScoreType kRemovedDartScore				= 5;
const tGameScoreType kAnalyzedDartScore				= 5;
const tGameScoreType kBuiltAntidoteScore			= 5;
const tGameScoreType kGotSinclairKeyScore			= 5;
const tGameScoreType kGotArgonCanisterScore 		= 5;
const tGameScoreType kGotNitrogenCanisterScore		= 5;
const tGameScoreType kPlayedWithMessagesScore		= 2;
const tGameScoreType kSawMorphExperimentScore		= 3;
const tGameScoreType kEnteredSinclairOfficeScore	= 2;
const tGameScoreType kSawBrochureScore				= 3;
const tGameScoreType kSawSinclairEntry1Score		= 3;
const tGameScoreType kSawSinclairEntry2Score		= 3;
const tGameScoreType kSawSinclairEntry3Score		= 3;
const tGameScoreType kSawWSCDirectoryScore			= 3;
const tGameScoreType kUsedCrowBarInWSCScore			= 5;
const tGameScoreType kFinishedPlasmaDodgeScore		= 10;
const tGameScoreType kOpenedCatwalkScore			= 3;
const tGameScoreType kStoppedWSCRobotScore			= 10;
const tGameScoreType kGotWSCOpMemChipScore			= 10;
const tGameScoreType kFinishedWSCScore				= 10;

const tGameScoreType kMaxWSCScore					=	kRemovedDartScore +
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

const tGameScoreType kMarsGandhiScore				= 10;
const tGameScoreType kNoradGandhiScore				= 10;
const tGameScoreType kWSCGandhiScore				= 10;

const tGameScoreType kMaxGandhiScore				=	kMarsGandhiScore +
														kNoradGandhiScore +
														kWSCGandhiScore;

const tGameScoreType kMaxTotalScore					=	kMaxCaldoriaTSAScore +
														kMaxPrehistoricScore +
														kMaxMarsScore +
														kMaxNoradScore +
														kMaxWSCScore +
														kMaxGandhiScore;
} // End of namespace Pegasus

#endif