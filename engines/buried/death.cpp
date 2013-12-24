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

#include "buried/avi_frames.h"
#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/death.h"
#include "buried/frame_window.h"
#include "buried/graphics.h"
#include "buried/resources.h"
#include "buried/sound.h"

#include "common/error.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace Buried {

enum {
	BUTTON_QUIT = 1,
	BUTTON_RESTORE_GAME = 2,
	BUTTON_MAIN_MENU = 3
};

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

DeathWindow::DeathWindow(BuriedEngine *vm, Window *parent, int deathSceneIndex, GlobalFlags globalFlags) : Window(vm, parent), _deathSceneIndex(deathSceneIndex), _globalFlags(globalFlags) {
	_curButton = 0;
	_deathFrameIndex = -1;
	_lightOn = false;
	_walkthroughMode = false;

	_rect = Common::Rect(0, 0, 640, 480);
	_quit = Common::Rect(27, 422, 100, 460);
	_restoreGame = Common::Rect(112, 422, 185, 460);
	_mainMenu = Common::Rect(198, 422, 271, 460);

	_timer = setTimer(400);

	if (deathSceneIndex < 10) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_CASTLE_FILENAME));
	} else if (deathSceneIndex < 20) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_MAYAN_FILENAME));
	} else if (deathSceneIndex < 30) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_AGENTLAIR_FILENAME));
	} else if (deathSceneIndex < 40) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_DAVINCI_FILENAME));
	} else if (deathSceneIndex < 50) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_AILAB_FILENAME));
	} else if (deathSceneIndex < 60) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_ALIEN_FILENAME));
	} else if (deathSceneIndex < 70) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_FINALE_FILENAME));
	} else {
		error("Bad death scene index %d", deathSceneIndex);
	}

	// Set the frame index
	switch (deathSceneIndex) {
	case 15:
		_deathFrameIndex = 4;
		break;
	case 52:
	case 53:
		_deathFrameIndex = 1;
		break;
	case 54:
	case 55:
		_deathFrameIndex = 0;
		break;
	default:
		_deathFrameIndex = deathSceneIndex % 10;
		break;
	}

	_fontHeightA = (_vm->getLanguage() == Common::JA_JPN) ? 12 : 14;
	_textFontA = _vm->_gfx->createFont(_fontHeightA);

	_fontHeightB = 20;
	_textFontB = _vm->_gfx->createFont(_fontHeightB, true);

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
	if (_walkthroughMode) {
		if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0)) {
			// HACK HACK HACK: More horridness.
			Common::String stringResource = _vm->getString(IDS_DEATH_WALK_SCORE_DESC_TEMPL);
			_scoringTextDescriptions = Common::String::format(stringResource.c_str(), criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw);
			stringResource = _vm->getString(IDS_DEATH_WALK_SCORE_AMT_TEMPL);
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
	} else {
		totalScore -= hintsScore;

		if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0)) {
			// HACK HACK HACK: Did I mention this was terrible?
			Common::String stringResource = _vm->getString(IDS_DEATH_SCORE_DESC_TEMPL);
			_scoringTextDescriptions = Common::String::format(stringResource.c_str(), criticalEvidence, supportingEvidence, puzzlesSolved, researchBonusRaw, hints);
			stringResource = _vm->getString(IDS_DEATH_SCORE_AMT_TEMPL);
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
	}

	// This would be a hack, but since it's just printing one number, I'm not
	// loading that damned string too.
	_scoringTextFinalScore = Common::String::format("%d", totalScore);

	_vm->_sound->setAmbientSound();
}

#undef CHECK_PUZZLE_FLAG
#undef CHECK_RESEARCH_FLAG
#undef CHECK_CRITICAL_EVIDENCE
#undef CHECK_SUPPORTING_EVIDENCE

DeathWindow::~DeathWindow() {
	killTimer(_timer);

	delete _deathSceneFrames;

	delete _textFontA;
	delete _textFontB;
}

void DeathWindow::onPaint() {
	Graphics::Surface *topBitmap = _vm->_gfx->getBitmap(IDB_DEATH_UI_TOP);
	_vm->_gfx->blit(topBitmap, 301, 0);
	topBitmap->free();
	delete topBitmap;

	Graphics::Surface *leftBitmap = _vm->_gfx->getBitmap(IDB_DEATH_UI_LEFT);
	_vm->_gfx->blit(leftBitmap, 0, 0);
	leftBitmap->free();
	delete leftBitmap;

	if (_walkthroughMode) {
		Graphics::Surface *lowerLeftBitmap = _vm->_gfx->getBitmap(IDB_DEATH_WT_LOWER_LEFT);
		_vm->_gfx->blit(lowerLeftBitmap, 0, 416);
		lowerLeftBitmap->free();
		delete lowerLeftBitmap;
	}

	Graphics::Surface *rightBitmap = _vm->_gfx->getBitmap(IDB_DEATH_UI_RIGHT);
	_vm->_gfx->blit(rightBitmap, 624, 0);
	rightBitmap->free();
	delete rightBitmap;

	Graphics::Surface *bottomBitmap = _vm->_gfx->getBitmap(IDB_DEATH_UI_BOTTOM);
	_vm->_gfx->blit(bottomBitmap, 301, 468);
	bottomBitmap->free();
	delete bottomBitmap;

	const Graphics::Surface *deathSceneHeader = _deathSceneFrames->getFrame(_deathFrameIndex);
	_vm->_gfx->blit(deathSceneHeader, 301, 10);

	uint32 textColor = _vm->_gfx->getColor(153, 102, 204);
	Common::String firstBlock = _vm->getString(IDS_DEATH_SCENE_MESSAGE_TEXT_BASE + _deathSceneIndex * 5);
	Common::Rect firstBlockRect(10, 54, 283, 86);
	_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, firstBlock, firstBlockRect.left, firstBlockRect.top, firstBlockRect.width(), firstBlockRect.height(), textColor, _fontHeightA);

	Common::String secondBlock = _vm->getString(IDS_DEATH_SCENE_MESSAGE_TEXT_BASE + _deathSceneIndex * 5 + 1);
	Common::Rect secondBlockRect(10, 120, 283, 215);
	_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, secondBlock, secondBlockRect.left, secondBlockRect.top, secondBlockRect.width(), secondBlockRect.height(), textColor, _fontHeightA);

	Common::Rect scoringDescRect(10, 248, 283, 378);
	_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, _scoringTextDescriptions, scoringDescRect.left, scoringDescRect.top, scoringDescRect.width(), scoringDescRect.height(), textColor, _fontHeightA);

	textColor = _vm->_gfx->getColor(212, 109, 0);
	_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, _scoringTextScores, scoringDescRect.left, scoringDescRect.top, scoringDescRect.width(), scoringDescRect.height(), textColor, _fontHeightA, kTextAlignRight);

	// CHECKME: This does center vertical alignment, so check the y coordinates
	Common::Rect finalTextScoreRect(122, 386, 283, 401);
	_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontB, _scoringTextFinalScore, finalTextScoreRect.left, finalTextScoreRect.top, finalTextScoreRect.width(), finalTextScoreRect.height(), textColor, _fontHeightB, kTextAlignRight);
}

bool DeathWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void DeathWindow::onTimer(uint timer) {
	_vm->_sound->timerCallback(); // clone2727 says: Don't think this is necessary

	// Flip the state of the light
	_lightOn = !_lightOn;

	Common::Rect destRect(137, 51);
	destRect.moveTo(164, 0);

	if (_lightOn) {
		// Light is now on
		Graphics::Surface *lightBitmap = _vm->_gfx->getBitmap(IDB_DEATH_ELIGHT_ON);
		_vm->_gfx->blit(lightBitmap, 164, 0);
		lightBitmap->free();
		delete lightBitmap;
	} else {
		// Light is now off
		Graphics::Surface *leftBitmap = _vm->_gfx->getBitmap(IDB_DEATH_UI_LEFT);
		_vm->_gfx->blit(leftBitmap, Common::Rect(164, 0, 164 + 137, 51), destRect);
		leftBitmap->free();
		delete leftBitmap;
	}

	// Force just that section of the screen to update
	invalidateRect(destRect, false);
	_vm->_gfx->updateScreen(false);
}

void DeathWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (_quit.contains(point)) {
		Graphics::Surface *buttons = _vm->_gfx->getBitmap(_walkthroughMode ? IDB_DEATH_WT_BUTTONS_DEPRESSED : IDB_DEATH_BUTTONS_DEPRESSED);
		Common::Rect destRect(29, 424, 29 + 76, 424 + 36);
		_vm->_gfx->blit(buttons, Common::Rect(76, 36), destRect);
		buttons->free();
		delete buttons;
		invalidateRect(destRect, false);
		_vm->_gfx->updateScreen(false);
		_curButton = BUTTON_QUIT;
	} else if (_restoreGame.contains(point)) {
		Graphics::Surface *buttons = _vm->_gfx->getBitmap(_walkthroughMode ? IDB_DEATH_WT_BUTTONS_DEPRESSED : IDB_DEATH_BUTTONS_DEPRESSED);
		Common::Rect destRect(109, 424, 109 + 80, 424 + 36);
		_vm->_gfx->blit(buttons, Common::Rect(80, 0, 80 + 80, 36), destRect);
		buttons->free();
		delete buttons;
		invalidateRect(destRect, false);
		_vm->_gfx->updateScreen(false);
		_curButton = BUTTON_RESTORE_GAME;
	} else if (_mainMenu.contains(point)) {
		Graphics::Surface *buttons = _vm->_gfx->getBitmap(_walkthroughMode ? IDB_DEATH_WT_BUTTONS_DEPRESSED : IDB_DEATH_BUTTONS_DEPRESSED);
		Common::Rect destRect(195, 424, 195 + 76, 424 + 36);
		_vm->_gfx->blit(buttons, Common::Rect(166, 0, 166 + 76, 36), destRect);
		buttons->free();
		delete buttons;
		invalidateRect(destRect, false);
		_vm->_gfx->updateScreen(false);
		_curButton = BUTTON_MAIN_MENU;
	}
}

void DeathWindow::onLButtonUp(const Common::Point &point, uint flags) {
	switch (_curButton) {
	case BUTTON_QUIT:
		if (_quit.contains(point)) {
			_vm->quitGame();
			return;
		}
		break;
	case BUTTON_RESTORE_GAME:
		if (_restoreGame.contains(point)) {
			if (_walkthroughMode) {
				// TODO: Do fake continue
			} else {
				// Show restore game window
				FrameWindow *frameWindow = (FrameWindow *)_parent;
				Common::Error result = _vm->runLoadDialog();

				if (result.getCode() == Common::kUnknownError) {
					// Try to get us back to the main menu at this point
					frameWindow->showMainMenu();
					return;
				} else if (result.getCode() == Common::kNoError) {
					// Loaded successfully
					return;
				}
			}
		}
		break;
	case BUTTON_MAIN_MENU:
		if (_mainMenu.contains(point)) {
			((FrameWindow *)_parent)->showMainMenu();
			return;
		}
		break;
	default:
		return;
	}

	_curButton = 0;
	invalidateWindow(false);
}

void DeathWindow::onMouseMove(const Common::Point &point, uint flags) {
	switch (_curButton) {
	case BUTTON_QUIT:
		if (!_quit.contains(point)) {
			_curButton = 0;
			invalidateRect(_quit, false);
		}
		break;
	case BUTTON_RESTORE_GAME:
		if (!_restoreGame.contains(point)) {
			_curButton = 0;
			invalidateRect(_restoreGame, false);
		}
		break;
	case BUTTON_MAIN_MENU:
		if (!_mainMenu.contains(point)) {
			_curButton = 0;
			invalidateRect(_mainMenu, false);
		}
		break;
	}
}

} // End of namespace Buried
