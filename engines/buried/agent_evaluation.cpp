/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/agent_evaluation.h"
#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/resources.h"

namespace Buried {

#define CHECK_PUZZLE_FLAG(flag) \
	if (_globalFlags.flag != 0) \
		puzzlesSolved++

#define CHECK_RESEARCH_FLAG(flag) \
	if (_globalFlags.flag != 0) \
		researchBonusRaw++

#define CHECK_CRITICAL_EVIDENCE(flag) \
	if (_globalFlags.evcapBaseID[i] == flag) \
		criticalEvidence++

#define CHECK_SUPPORTING_EVIDENCE(flag) \
	if (_globalFlags.evcapBaseID[i] == flag) \
		supportingEvidence++

AgentEvaluation::AgentEvaluation(BuriedEngine *vm, GlobalFlags &globalFlags, int deathSceneIndex) :
	_globalFlags(globalFlags) {

	int puzzlesSolved = 0;
	CHECK_PUZZLE_FLAG(scoreGotTranslateBioChip);
	CHECK_PUZZLE_FLAG(scoreEnteredSpaceStation);
	CHECK_PUZZLE_FLAG(scoreDownloadedArthur);
	CHECK_PUZZLE_FLAG(scoreFoundSculptureDiagram);
	CHECK_PUZZLE_FLAG(scoreEnteredKeep);
	CHECK_PUZZLE_FLAG(scoreGotKeyFromSmithy);
	CHECK_PUZZLE_FLAG(scoreEnteredTreasureRoom);
	CHECK_PUZZLE_FLAG(scoreFoundSwordDiamond);
	CHECK_PUZZLE_FLAG(scoreMadeSiegeCycle);
	CHECK_PUZZLE_FLAG(scoreEnteredCodexTower);
	CHECK_PUZZLE_FLAG(scoreLoggedCodexEvidence);
	CHECK_PUZZLE_FLAG(scoreEnteredMainCavern);
	CHECK_PUZZLE_FLAG(scoreGotWealthGodPiece);
	CHECK_PUZZLE_FLAG(scoreGotRainGodPiece);
	CHECK_PUZZLE_FLAG(scoreGotWarGodPiece);
	CHECK_PUZZLE_FLAG(scoreCompletedDeathGod);
	CHECK_PUZZLE_FLAG(scoreEliminatedAgent3);
	CHECK_PUZZLE_FLAG(scoreTransportToKrynn);
	CHECK_PUZZLE_FLAG(scoreGotKrynnArtifacts);
	CHECK_PUZZLE_FLAG(scoreDefeatedIcarus);

	int researchBonusRaw = 0;
	CHECK_RESEARCH_FLAG(scoreResearchINNLouvreReport); // > v1.01
	CHECK_RESEARCH_FLAG(scoreResearchINNHighBidder);
	CHECK_RESEARCH_FLAG(scoreResearchINNAppeal);
	CHECK_RESEARCH_FLAG(scoreResearchINNUpdate);
	CHECK_RESEARCH_FLAG(scoreResearchINNJumpsuit);
	CHECK_RESEARCH_FLAG(scoreResearchBCJumpsuit);
	CHECK_RESEARCH_FLAG(scoreResearchMichelle);
	CHECK_RESEARCH_FLAG(scoreResearchMichelleBkg);
	CHECK_RESEARCH_FLAG(scoreResearchLensFilter);
	CHECK_RESEARCH_FLAG(scoreResearchCastleFootprint);
	CHECK_RESEARCH_FLAG(scoreResearchDaVinciFootprint);
	CHECK_RESEARCH_FLAG(scoreResearchMorphSculpture);
	CHECK_RESEARCH_FLAG(scoreResearchEnvironCart);
	CHECK_RESEARCH_FLAG(scoreResearchAgent3Note);
	CHECK_RESEARCH_FLAG(scoreResearchAgent3DaVinci);

	int criticalEvidence = 0;
	int supportingEvidence = 0;
	for (int i = 0; i < _globalFlags.evcapNumCaptured; i++) {
		CHECK_CRITICAL_EVIDENCE(CASTLE_EVIDENCE_SWORD);
		CHECK_CRITICAL_EVIDENCE(MAYAN_EVIDENCE_ENVIRON_CART);
		CHECK_CRITICAL_EVIDENCE(DAVINCI_EVIDENCE_CODEX);
		CHECK_CRITICAL_EVIDENCE(AI_EVIDENCE_SCULPTURE);

		CHECK_SUPPORTING_EVIDENCE(CASTLE_EVIDENCE_FOOTPRINT);
		CHECK_SUPPORTING_EVIDENCE(MAYAN_EVIDENCE_BROKEN_GLASS_PYRAMID);
		CHECK_SUPPORTING_EVIDENCE(MAYAN_EVIDENCE_PHONY_BLOOD);
		CHECK_SUPPORTING_EVIDENCE(CASTLE_EVIDENCE_AGENT3);
		CHECK_SUPPORTING_EVIDENCE(DAVINCI_EVIDENCE_FOOTPRINT);
		CHECK_SUPPORTING_EVIDENCE(DAVINCI_EVIDENCE_AGENT3);
		CHECK_SUPPORTING_EVIDENCE(DAVINCI_EVIDENCE_LENS_FILTER);
	}

	int hints = _globalFlags.scoreHintsTotal;
	int finalCriticalEvidenceScore = criticalEvidence * 1000;
	int finalSupportingEvidenceScore = supportingEvidence * 500;
	int finalPuzzleScore = puzzlesSolved * 200;
	int finalResearchScore = researchBonusRaw * 100;
	int hintsScore = hints * 50;
	int completionScore = (deathSceneIndex == 60) ? 2000 : 0;
	int totalScore = finalCriticalEvidenceScore + finalSupportingEvidenceScore + finalPuzzleScore + finalResearchScore + completionScore;

	// Build the string buffers
	// Newer versions include these strings as resources in the main executable.
	// For earlier versions, we hardcode them here.
	if (_globalFlags.generalWalkthroughMode != 0) {
		if (vm->getVersion() >= MAKEVERSION(1, 0, 4, 0)) {
			Common::String stringResource = vm->getString(IDS_DEATH_WALK_SCORE_DESC_TEMPL);
			_scoringTextDescriptions = Common::String::format(stringResource.c_str(), criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw);
			stringResource = vm->getString(IDS_DEATH_WALK_SCORE_AMT_TEMPL);
			_scoringTextScores = Common::String::format(stringResource.c_str(), finalCriticalEvidenceScore, finalSupportingEvidenceScore, finalPuzzleScore, finalResearchScore, completionScore);
		} else {
			if (deathSceneIndex == 60) {
				_scoringTextDescriptions = Common::String::format("Critical Evidence: %d / 4 x 1000\nSupporting Evidence: %d / 7 x 500\nPuzzles Solved: %d / 19 x 200\nResearch Bonus: %d / 15 x 100\nCompletion Bonus:",
																  criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw);
				_scoringTextScores = Common::String::format("%d\n%d\n%d\n%d\n%d", finalCriticalEvidenceScore, finalSupportingEvidenceScore, finalPuzzleScore, finalResearchScore, completionScore);
			} else {
				_scoringTextDescriptions = Common::String::format("Critical Evidence: %d / 4 x 1000\nSupporting Evidence: %d / 7 x 500\nPuzzles Solved: %d / 19 x 200\nResearch Bonus: %d / 15 x 100",
																  criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw);
				_scoringTextScores = Common::String::format("%d\n%d\n%d\n%d", finalCriticalEvidenceScore, finalSupportingEvidenceScore, finalPuzzleScore, finalResearchScore);
			}
		}

		_scoringTextDescriptionsWithScores = Common::String::format("Current Points:\n\nCritical Evidence: %d / 4 x 1000: %d\nSupporting Evidence: %d / 7 x 500: %d\nPuzzles Solved: %d / 19 x 200: %d\nResearch Bonus: %d / 15 x 100: %d\nCompletion Bonus: %d\n\nTotal Score: %d\n",
																	criticalEvidence, finalCriticalEvidenceScore,
																	supportingEvidence, finalSupportingEvidenceScore,
																	puzzlesSolved, finalPuzzleScore,
																	researchBonusRaw, finalResearchScore,
																	completionScore, totalScore);
	} else {
		totalScore -= hintsScore;

		if (vm->getVersion() >= MAKEVERSION(1, 0, 4, 0)) {
			Common::String stringResource = vm->getString(IDS_DEATH_SCORE_DESC_TEMPL);
			_scoringTextDescriptions = Common::String::format(stringResource.c_str(), criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw, hints);
			stringResource = vm->getString(IDS_DEATH_SCORE_AMT_TEMPL);
			_scoringTextScores = Common::String::format(stringResource.c_str(), finalCriticalEvidenceScore, finalSupportingEvidenceScore, finalPuzzleScore, finalResearchScore, completionScore, -hintsScore);
		} else {
			if (deathSceneIndex == 60) {
				_scoringTextDescriptions = Common::String::format("Critical Evidence: %d / 4 x 1000\nSupporting Evidence: %d / 7 x 500\nPuzzles Solved: %d / 20 x 200\nResearch Bonus: %d / 15 x 100\nCompletion Bonus:\n\nHints: %d @ -50 ea.",
																  criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw, hints);
				_scoringTextScores = Common::String::format("%d\n%d\n%d\n%d\n%d\n\n%d", finalCriticalEvidenceScore, finalSupportingEvidenceScore, finalPuzzleScore, finalResearchScore, completionScore, -hintsScore);
			} else {
				_scoringTextDescriptions = Common::String::format("Critical Evidence: %d / 4 x 1000\nSupporting Evidence: %d / 7 x 500\nPuzzles Solved: %d / 20 x 200\nResearch Bonus: %d / 15 x 100\n\n\nHints: %d @ -50 ea.",
																  criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw, hints);
				_scoringTextScores = Common::String::format("%d\n%d\n%d\n%d\n\n\n%d", finalCriticalEvidenceScore, finalSupportingEvidenceScore, finalPuzzleScore, finalResearchScore, -hintsScore);
			}
		}

		_scoringTextDescriptionsWithScores = Common::String::format("Current Points:\n\nCritical Evidence: %d / 4 x 1000: %d\nSupporting Evidence: %d / 7 x 500: %d\nPuzzles Solved: %d / 20 x 200: %d\nResearch Bonus: %d / 15 x 100: %d\nCompletion Bonus: %d\n\nHints: %d @ -50 ea.\n\nTotal Score: %d\n",
																	criticalEvidence, finalCriticalEvidenceScore,
																	supportingEvidence, finalSupportingEvidenceScore,
																	puzzlesSolved, finalPuzzleScore,
																	researchBonusRaw, finalResearchScore,
																	completionScore, -hintsScore, totalScore);
	}

	_scoringTextFinalScore = Common::String::format("%d", totalScore);
}

#undef CHECK_PUZZLE_FLAG
#undef CHECK_RESEARCH_FLAG
#undef CHECK_CRITICAL_EVIDENCE
#undef CHECK_SUPPORTING_EVIDENCE

} // End of namespace Buried
