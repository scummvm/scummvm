/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/complete.h"
#include "buried/frame_window.h"
#include "buried/graphics.h"
#include "buried/sound.h"
#include "buried/video_window.h"

#include "graphics/font.h"
#include "graphics/surface.h"

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

CompletionWindow::CompletionWindow(BuriedEngine *vm, Window *parent, const GlobalFlags &globalFlags) : Window(vm, parent), _globalFlags(globalFlags) {
	_vm->_sound->setAmbientSound();

	_status = 0;
	_background = 0;
	_currentSoundEffectID = -1;
	_gageVideo = 0;

	_rect = Common::Rect(0, 0, 640, 480);

	_timer = setTimer(1000);

	_textFontA = _vm->_gfx->createFont(14);
	_textFontB = _vm->_gfx->createFont(20, true);

	_walkthroughMode = _globalFlags.generalWalkthroughMode != 0;

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
	CHECK_PUZZLE_FLAG(scoreGotWarGodPiece);
	CHECK_PUZZLE_FLAG(scoreCompletedDeathGod);
	CHECK_PUZZLE_FLAG(scoreEliminatedAgent3);
	CHECK_PUZZLE_FLAG(scoreTransportToKrynn);
	CHECK_PUZZLE_FLAG(scoreGotKrynnArtifacts);
	CHECK_PUZZLE_FLAG(scoreDefeatedIcarus);

	int researchBonusRaw = 0;
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
	int completionScore = 2000;
	int totalScore = finalCriticalEvidenceScore + finalSupportingEvidenceScore + finalPuzzleScore + finalResearchScore + completionScore;

	if (_walkthroughMode) {
		// TODO: This is a fucking mess between versions
	} else {
		totalScore -= hintsScore;

		// TODO: Another mess
	}

	// TODO: A third mess

	_vm->_sound->setAmbientSound();
}

#undef CHECK_PUZZLE_FLAG
#undef CHECK_RESEARCH_FLAG
#undef CHECK_CRITICAL_EVIDENCE
#undef CHECK_SUPPORTING_EVIDENCE

CompletionWindow::~CompletionWindow() {
	delete _gageVideo;
	killTimer(_timer);
	
	delete _textFontA;
	delete _textFontB;

	if (_background) {
		_background->free();
		delete _background;
	}
}

void CompletionWindow::onPaint() {
	if (_background)
		_vm->_gfx->blit(_background, 0, 0);

	if (_status == 3) {
		// Draw the first line
		uint32 textColor = _vm->_gfx->getColor(102, 204, 153);
		Common::String firstBlockText = _vm->getString(2100);
		Common::Rect firstBlockRect(10, 54, 283, 86);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, firstBlockText, firstBlockRect.left, firstBlockRect.top, firstBlockRect.width(), textColor, 14);

		// Draw the cause text
		Common::String secondBlockText = _vm->getString(2102);
		Common::Rect secondBlockRect(10, 120, 283, 215);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, secondBlockText, secondBlockRect.left, secondBlockRect.top, secondBlockRect.width(), textColor, 14);

		// Description text
		Common::Rect scoringTextRect(10, 248, 283, 378);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, _scoringTextDescriptions, scoringTextRect.left, scoringTextRect.top, scoringTextRect.width(), textColor, 14);

		// Scores
		textColor = _vm->_gfx->getColor(255, 255, 51);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, _scoringTextScores, scoringTextRect.left, scoringTextRect.top, scoringTextRect.width(), textColor, 14, true);

		// Total score
		Common::Rect finalScoreRect(122, 386, 283, 401);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontB, _scoringTextFinalScore, finalScoreRect.left, finalScoreRect.top, finalScoreRect.width(), textColor, 20, true);
	}
}

bool CompletionWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void CompletionWindow::onTimer(uint timer) {
	switch (_status) {
	case 0:
		_currentSoundEffectID = _vm->_sound->playSoundEffect("BITDATA/FUTAPT/FA_COURT.BTA");
		_status = 1;
		_background = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "BITDATA/FUTAPT/CONGRATS.BTS" : "BITDATA/FUTAPT/CONGRAT8.BTS");
		invalidateWindow(false);
		break;
	case 2:
		if (!_gageVideo || _gageVideo->getMode() == VideoWindow::kModeStopped) {
			delete _gageVideo;
			_gageVideo = 0;

			_status = 3;
			_background = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "BITDATA/FUTAPT/ENDING24.BTS" : "BITDATA/FUTAPT/ENDING8.BTS");
			invalidateWindow(false);
			_vm->_sound->setAmbientSound("BITDATA/FUTAPT/FA_FIN.BTA");
		}
		break;
	}
}

void CompletionWindow::onLButtonUp(const Common::Point &point, uint flags) {
	switch (_status) {
	case 1:
		_vm->_sound->stopSoundEffect(_currentSoundEffectID);
		_currentSoundEffectID = -1;
		_status = 2;

		if (_background) {
			_background->free();
			delete _background;
			_background = 0;
		}

		invalidateWindow(false);

		_gageVideo = new VideoWindow(_vm, this);

		if (!_gageVideo->openVideo("BITDATA/FUTAPT/FA_FIN.BTV"))
			error("Failed to load finale video");

		_gageVideo->setWindowPos(0, 104, 145, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder);
		_gageVideo->enableWindow(false);
		_gageVideo->showWindow(kWindowShow);
		_gageVideo->playVideo();
		break;
	case 2:
		if (!_gageVideo || _gageVideo->getMode() == VideoWindow::kModeStopped) {
			delete _gageVideo;
			_gageVideo = 0;

			_status = 4;
			_background = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "BITDATA/FUTAPT/ENDING24.BTS" : "BITDATA/FUTAPT/ENDING8.BTS");
			invalidateWindow(false);
		}
		break;
	case 3:
		((FrameWindow *)_parent)->showCredits();
		break;
	}
}

} // End of namespace Buried
