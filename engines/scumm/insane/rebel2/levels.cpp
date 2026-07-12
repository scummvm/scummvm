/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/events.h"
#include "common/system.h"
#include "common/util.h"

#include "graphics/cursorman.h"

#include "scumm/scumm_v7.h"

#include "scumm/smush/smush_player.h"

#include "scumm/insane/rebel2/rebel.h"

namespace Scumm {

static const int kRebel2GameplayAimCenterX = 160;
static const int kRebel2GameplayAimCenterY = 100;
static const uint32 kRebel2GameplayMouseSettleMs = 1000;

struct Rebel2LevelEndParams {
	int titleStartBeforeEnd;
	int titleEndBeforeEnd;
	int accLow;
	int accHigh;
	int errLow;
	int errHigh;
};

static const Rebel2LevelEndParams kRebel2LevelEndParams[16] = {
	{ 0,   0,  -1,  -1, -1, -1 },
	{ 120, 10,  96, 100, -1, -1 },
	{ 120, 10,  80,  94, -1, -1 },
	{  90,  5,  56,  72,  1,  3 },
	{ 120, 10,  42,  48,  2,  4 },
	{ 120, 10,  96, 100, -1, -1 },
	{ 545,435,  60,  76,  1,  4 },
	{ 115, 10,  -1,  -1,  1,  3 },
	{ 120, 10,  90,  96, -1, -1 },
	{ 100,  1,  36,  48,  2,  4 },
	{ 100, 10,  28,  36,  1,  3 },
	{ 100, 10,  70,  78, -1, -1 },
	{ 120, 10,  28,  36, -1, -1 },
	{ 150, 10,  86,  90,  3,  5 },
	{  75, 10,  96, 100, -1, -1 },
	{ 100, 10,  60,  68,  1,  3 }
};

static void purgeRebel2GameplayInputEvents(Common::EventManager *eventMan) {
	if (!eventMan)
		return;

	eventMan->getEventDispatcher()->clearEvents();
	eventMan->purgeMouseEvents();
	eventMan->purgeKeyboardEvents();
}

Common::String InsaneRebel2::getLevelDir(int levelId) {
	return Common::String::format("LEV%02d", levelId);
}

Common::String InsaneRebel2::getLevelPrefix(int levelId) {
	return Common::String::format("%02d", levelId);
}

void InsaneRebel2::runGame() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	if (_vm->_game.features & GF_DEMO) {
		splayer->play("OPEN/O_DEMO.SAN", 15);
		return;
	}

	playIntroSequence();

	while (!_vm->shouldQuit()) {
		int menuResult = runMainMenu();

		if (menuResult == kMenuQuit || _vm->shouldQuit())
			break;

		if (menuResult == kMenuResumeDemo) {
			playIntroSequence();
			if (!_vm->shouldQuit())
				showTopPilots();
			continue;
		}

		if (menuResult == kMenuNewGame) {
			int pilotResult = runLevelSelect();

			if (pilotResult == kLevelSelectQuit || _vm->shouldQuit())
				break;

			if (pilotResult == kLevelSelectBack)
				continue;

			int chapterResult = runChapterSelect();

			if (chapterResult == kChapterSelectQuit || _vm->shouldQuit())
				break;

			if (chapterResult == kChapterSelectPlay) {
				int selectedLevel = _selectedChapter + 1;
				debugC(DEBUG_INSANE, "InsaneRebel2: Starting chapter %d (level %d)", _selectedChapter + 1, selectedLevel);

				if (selectedLevel == 16) {
					playEndingSequence();
				}

				while (!_vm->shouldQuit() && selectedLevel >= 1 && selectedLevel <= 15) {
					int result = runLevel(selectedLevel);

					if (result == kLevelNextLevel) {
						updatePilotProgress(selectedLevel,
							_playerScore, _playerLives, 0, _playerRating);
						selectedLevel++;
						if (selectedLevel > 15) {
							playEndingSequence();
							break;
						}
					} else {
						break;
					}
				}

				if (_vm->shouldQuit())
					break;
			}
		}
	}
}

void InsaneRebel2::playIntroSequence() {
	debugC(DEBUG_INSANE, "Playing intro sequence");

	_gameState = kStateIntro;
	_menuInputActive = false;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	debugC(DEBUG_INSANE, "Playing main intro (O_OPEN_A.SAN)");
	splayer->setCurVideoFlags(0x28);
	splayer->play("OPEN/O_OPEN_A.SAN", 15);

	if (_vm->shouldQuit())
		return;

	debugC(DEBUG_INSANE, "Playing additional intro (O_OPEN_B.SAN)");
	splayer->setCurVideoFlags(0x28);
	splayer->play("OPEN/O_OPEN_B.SAN", 15);
}

void InsaneRebel2::playMissionBriefing() {

	debugC(DEBUG_INSANE, "Playing mission briefing");

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x08);  // Briefing mode flag
	splayer->play("OPEN/O_LEVEL.SAN", 15);
}

void InsaneRebel2::playCinematic(const char *filename) {
	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename, 15);
}

void InsaneRebel2::playVideoWithText(const char *filename, int textID, int textX, int textY,
                                     int fadeInFrame, int fadeOutFrame) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;

	_textOverlayActive = true;
	_textOverlayID = textID;
	_textOverlayX = textX;
	_textOverlayY = textY;
	_textOverlayFadeIn = fadeInFrame;
	_textOverlayFadeOut = fadeOutFrame;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename, 15);

	_textOverlayActive = false;
}

void InsaneRebel2::playLevelBegin(int levelId) {

	struct TextOverlayParams {
		int textID;      // TRS string ID (-1 = no text overlay)
		int textX;
		int textY;
		int fadeInFrame;
		int fadeOutFrame;
	};

	// Text IDs are sequential: 0xAA (level 1) through 0xB8 (level 15)
	const TextOverlayParams levelTextParams[16] = {
		{ -1,   0,  0,   0,    0},    // Level 0 (unused)
		{0xAA, 0xA0, 10,   5, 0x4B},
		{0xAB, 0xA0, 10,   2, 0x46},
		{0xAC, 0xA0, 10,   2, 0x46},
		{0xAD, 0xA0, 10,   2,  100},
		{0xAE, 0xA0, 10,   5, 0x3C},
		{0xAF, 0xA0, 10,   5, 0x4B},
		{0xB0, 0xA0, 10,   5, 0x4B},
		{0xB1, 0xA0, 10,   5, 0x4B},
		{0xB2, 0xA0, 10, 200, 0x10E},
		{0xB3, 0xA0, 10,   2, 0x46},
		{0xB4, 0xA0, 10,   2, 0x46},
		{0xB5, 0xA0, 10,   5, 0x4B},
		{0xB6, 0xA0, 10,   2, 0x46},
		{0xB7, 0xA0, 10,   2, 0x46},
		{0xB8, 0xA0, 10,   2, 0x46},
	};

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sBEG.SAN", dir.c_str(), prefix.c_str());

	debugC(DEBUG_INSANE, "Playing level %d beginning: %s", levelId, filename.c_str());

	if (levelId >= 1 && levelId <= 15 && levelTextParams[levelId].textID >= 0) {
		const TextOverlayParams &p = levelTextParams[levelId];
		playVideoWithText(filename.c_str(), p.textID, p.textX, p.textY,
		                  p.fadeInFrame, p.fadeOutFrame);
	} else {
		playCinematic(filename.c_str());
	}
}

int InsaneRebel2::calculateLevelEndRating(int accuracy, int accLow, int accHigh,
		int flightErrors, int errLow, int errHigh, bool skillBonus) const {
	int accuracyGrade = -1;
	if (accuracy >= 0) {
		if (accuracy < accHigh) {
			if (accuracy < (accLow + accHigh) / 2)
				accuracyGrade = (accuracy < accLow) ? 0 : 1;
			else
				accuracyGrade = 2;
		} else {
			accuracyGrade = 3;
		}
	}

	int errorGrade = -1;
	if (flightErrors >= 0) {
		if (errHigh < flightErrors) {
			if ((errLow + errHigh) / 2 < flightErrors)
				errorGrade = (flightErrors <= errLow) ? 1 : 0;
			else
				errorGrade = 2;
		} else {
			errorGrade = 3;
		}
	}

	int rating = 0;
	if (accuracy >= 0 && flightErrors >= 0) {
		rating = (accuracyGrade + errorGrade + 1) / 2;
	} else if (accuracy >= 0) {
		rating = accuracyGrade;
	} else if (flightErrors >= 0) {
		rating = errorGrade;
	}

	if (skillBonus)
		rating++;

	return MAX(0, rating);
}

void InsaneRebel2::prepareLevelEndStats(int levelId, int accuracy, int flightErrors, bool skillBonus) {
	if (levelId < 1 || levelId > 15) {
		_levelEndStats.active = false;
		return;
	}

	const Rebel2LevelEndParams &p = kRebel2LevelEndParams[levelId];
	const bool hasAccuracy = (accuracy >= 0 && p.accLow >= 0 && p.accHigh >= 0);
	const bool hasFlightErrors = (flightErrors >= 0 && p.errLow >= 0 && p.errHigh >= 0);
	const int ratingAward = calculateLevelEndRating(
		hasAccuracy ? accuracy : -1, p.accLow, p.accHigh,
		hasFlightErrors ? flightErrors : -1, p.errLow, p.errHigh,
		skillBonus);

	LevelDifficultyParams difficultyParams = getDifficultyParams();
	if (levelId == 15)
		difficultyParams = kDifficultyTable[CLIP(_difficulty, 0, 5)][16];

	const int bonus = (difficultyParams.specialPoints > 0) ?
		difficultyParams.specialPoints * ratingAward : 0;
	const int levelPoints = MAX(0, (int)difficultyParams.levelPoints);

	_levelEndStats.active = true;
	_levelEndStats.levelId = levelId;
	_levelEndStats.textX = 0xa0;
	_levelEndStats.textY = (levelId == 7) ? 0xb : 10;
	_levelEndStats.titleStartBeforeEnd = p.titleStartBeforeEnd;
	_levelEndStats.titleEndBeforeEnd = p.titleEndBeforeEnd;
	_levelEndStats.hasAccuracy = hasAccuracy;
	_levelEndStats.hasFlightErrors = hasFlightErrors;
	_levelEndStats.skillBonus = skillBonus;
	_levelEndStats.accuracy = hasAccuracy ? accuracy : -1;
	_levelEndStats.flightErrors = hasFlightErrors ? flightErrors : -1;
	_levelEndStats.bonus = bonus;
	_levelEndStats.oldRating = _playerRating;

	addScore(bonus);
	addScore(levelPoints);
	_playerRating += ratingAward;

	_levelEndStats.finalScore = _playerScore;
	_levelEndStats.newRating = _playerRating;

	if (_activePilot >= 0 && _activePilot < _numPilots)
		insertRanking(_pilots[_activePilot].name, _playerScore, _playerRating, _difficulty, levelId);
}

void InsaneRebel2::playLevelEnd(int levelId) {
	playLevelEnd(levelId, -1, -1, false);
}

void InsaneRebel2::playLevelEnd(int levelId, int accuracy, int flightErrors, bool skillBonus) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;
	prepareLevelEndStats(levelId, accuracy, flightErrors, skillBonus);

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sEND.SAN", dir.c_str(), prefix.c_str());

	debugC(DEBUG_INSANE, "Playing level %d end: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 15);

	_levelEndStats.active = false;
}

void InsaneRebel2::playLevelRetry(int levelId) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sRETRY.SAN", dir.c_str(), prefix.c_str());

	debugC(DEBUG_INSANE, "Playing level %d retry: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 15);
}

void InsaneRebel2::playLevelGameOver(int levelId) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sOVER.SAN", dir.c_str(), prefix.c_str());

	debugC(DEBUG_INSANE, "Playing level %d game over: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 15);
}

void InsaneRebel2::playEndingSequence() {

	debugC(DEBUG_INSANE, "Playing ending sequence (difficulty=%d)", _difficulty);

	_gameState = kStateGameplay;
	_menuInputActive = false;

	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
	memset(vs->getPixels(0, 0), 0, vs->pitch * vs->h);
	_vm->markRectAsDirty(kMainVirtScreen, 0, vs->w, 0, vs->h);

	if (_difficulty == 2) {
		playCinematic("FINAL/F_FIN_B.SAN");
	} else if (_difficulty == 3) {
		playCinematic("FINAL/F_FIN_C.SAN");
	} else {
		playCinematic("FINAL/F_FIN_A.SAN");
	}

	if (_vm->shouldQuit())
		return;

	playCinematic("FINAL/F_CREDIT.SAN");

	if (_vm->shouldQuit())
		return;

	playCinematic("FINAL/F_EPILOG.SAN");
}

void InsaneRebel2::playCreditsSequence() {

	debugC(DEBUG_INSANE, "Playing menu credits");
	resetVideoAudio();

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x20);
	splayer->play("OPEN/O_CREDIT.SAN", 15);
}

void InsaneRebel2::centerGameplayAim() {
	Common::EventManager *eventMan = _vm->_system->getEventManager();
	purgeRebel2GameplayInputEvents(eventMan);

	_joystickAxisX = 0;
	_joystickAxisY = 0;
	_gamepadAimActive = false;

	warpGameplayMouseNow(kRebel2GameplayAimCenterX, kRebel2GameplayAimCenterY);
	purgeRebel2GameplayInputEvents(eventMan);
	_gameplayMouseSettleUntil = _vm->_system->getMillis() + kRebel2GameplayMouseSettleMs;

	debugC(DEBUG_INSANE, "centerGameplayAim: mouse=(%d,%d) joystick=(%d,%d) gamepadAim=%d settleUntil=%u",
		_vm->_mouse.x, _vm->_mouse.y,
		_joystickAxisX, _joystickAxisY, _gamepadAimActive ? 1 : 0,
		_gameplayMouseSettleUntil);
}

void InsaneRebel2::warpGameplayMouseNow(int x, int y) {
	Common::EventManager *eventMan = _vm->_system->getEventManager();
	if (eventMan)
		eventMan->purgeMouseEvents();

	const int scale = isHiRes() ? 2 : 1;
	const int physicalX = x * scale;
	const int physicalY = y * scale;

	_vm->_mouse.x = physicalX;
	_vm->_mouse.y = physicalY;
	_vm->_system->warpMouse(_vm->_macScreen ? physicalX * 2 : physicalX,
		_vm->_macScreen ? physicalY * 2 + 2 * _vm->_macScreenDrawOffset : physicalY);

	if (eventMan)
		eventMan->purgeMouseEvents();
}

int InsaneRebel2::runLevel(int levelId) {

	debugC(DEBUG_INSANE, "Starting level %d", levelId);

	if (levelId < 1 || levelId > 15) {
		warning("Rebel2: Invalid level ID %d", levelId);
		return kLevelReturnToMenu;
	}

	_gameState = kStateGameplay;
	_menuInputActive = false;

	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
	memset(vs->getPixels(0, 0), 0, vs->pitch * vs->h);
	_vm->markRectAsDirty(kMainVirtScreen, 0, vs->w, 0, vs->h);

	_selectedLevel = levelId;

	// Levels 1-6 use types 0-5, but Level 6 also uses type 6 mid-level.
	// Levels 7-15 use types 7-15 (gap at type 6 which is Level 6 phase 2).
	// Level 15 also switches to type 16 mid-level at frame 0x21e.
	const int kLevelTypeMap[16] = {
		-1, 0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15
	};
	_rebelLevelType = kLevelTypeMap[levelId];

	_gameplaySectionActive = false;
	CursorMan.showMouse(false);
	g_system->lockMouse(true);

	if (_activePilot >= 0 && _activePilot < _numPilots && _pilots[_activePilot].damage[levelId - 1] < 0xFF) {
		_playerLives = _pilots[_activePilot].lives[levelId - 1];
		_playerScore = _pilots[_activePilot].score[levelId - 1];
		_playerRating = _pilots[_activePilot].rating[levelId - 1];
	} else {
		_playerLives = 3;
		_playerScore = 0;
		_playerRating = 0;
	}
	_playerShield = 255;
	_playerDamage = 0;
	resetDamageFlash();
	_damageHighFlashCounter = 0;
	_damageShakeCounter = 0;
	_currentPhase = 1;
	_skipSectionRequested = false;

	int result;
	switch (levelId) {
	case 1:
		result = runLevel1();
		break;
	case 2:
		result = runLevel2();
		break;
	case 3:
		result = runLevel3();
		break;
	case 4:
		result = runLevel4();
		break;
	case 5:
		result = runLevel5();
		break;
	case 6:
		result = runLevel6();
		break;
	case 7:
		result = runLevel7();
		break;
	case 8:
		result = runLevel8();
		break;
	case 9:
		result = runLevel9();
		break;
	case 10:
		result = runLevel10();
		break;
	case 11:
		result = runLevel11();
		break;
	case 12:
		result = runLevel12();
		break;
	case 13:
		result = runLevel13();
		break;
	case 14:
		result = runLevel14();
		break;
	case 15:
		result = runLevel15();
		break;
	default:
		result = runLevel1();
		break;
	}

	restoreIOSGamepadController();
	g_system->lockMouse(false);
	CursorMan.showMouse(true);

	return result;
}

int InsaneRebel2::getRandomVariant(int max) {
	return _vm->_rnd.getRandomNumber(max - 1);
}

Common::String InsaneRebel2::selectDeathVideoVariant(int levelId, int phase, int frame) {

	switch (levelId) {
	case 1:
		return (getRandomVariant(2) == 0) ? "B" : "A";

	case 2:
		return "";

	case 3:
		if (phase == 1) {
			if (frame < 0x10c)
				return "B";       // < 268
			if (frame < 0x1a9)
				return "A";       // < 425
			if (frame < 0x247)
				return "C";       // < 583
			if (frame < 700)
				return "A";
			if (frame < 900)
				return "B";
			return "A";
		} else {
			if (frame < 0x2f1)
				return "A";       // < 753
			if (frame < 0x347)
				return "B";       // < 839
			if (frame < 0x3b1)
				return "C";       // < 945
			if (frame < 0x405)
				return "A";       // < 1029
			return "C";
		}

	case 4:
		return "A";

	case 5:
		return (getRandomVariant(2) == 0) ? "B" : "A";

	case 6:
		if (phase == 1) {
			if (frame < 0x4e)
				return "D";
			if (frame < 0xe0)
				return "A";
			if (frame < 0x122)
				return "D";
			if (frame < 0x1b4)
				return "B";
			if (frame < 499)
				return "D";
			if (frame < 0x286)
				return "C";
			return "D";
		} else {
			if (frame < 0xcc)
				return "E";
			if (frame < 0xfe)
				return "G";
			if (frame < 0x122)
				return "E";
			if (frame < 0x149)
				return "G";
			if (frame < 0x166)
				return "F";
			if (frame < 0x174)
				return "E";
			if (frame < 0x19f)
				return "F";
			if (frame < 0x1b2)
				return "G";
			if (frame < 0x1c8)
				return "F";
			if (frame < 0x207)
				return "E";
			if (frame < 0x217)
				return "F";
			if (frame < 0x23b)
				return "G";
			if (frame < 0x25b)
				return "F";
			if (frame < 0x285)
				return "E";
			return "G";
		}

	case 7:
		return (phase >= 2) ? "B" : "A";

	case 8:
		return (getRandomVariant(2) == 0) ? "B" : "A";

	case 9:
		// Picked by what killed the player: shot -> A, direct hit -> C, collision -> B.
		if (_rebelDeathCause == 0)
			return "A";
		return (_rebelDeathCause == 1) ? "C" : "B";

	case 10:
		return "";

	case 11:
		if (phase <= 1)
			return "A";
		if (phase == 2)
			return "B";
		return "C";

	case 12:
		return "";

	case 13:
		if (frame < 0x1c2)
			return "A";
		if (frame < 0x302)
			return "B";
		if (frame < 0x4ec)
			return "C";
		if (frame < 0x5b4)
			return "B";
		return "D";

	case 14:
		return "";

	case 15:
		if (frame < 0x21e)
			return "A";
		if (frame < 0x2f9)
			return "B";
		if (frame < 0x3e5)
			return "C";
		if (frame < 0x4a0)
			return "B";
		if (frame < 0x588)
			return "C";
		if (frame < 0x65e)
			return "B";
		return "D";

	default:
		return "A";
	}
}

void InsaneRebel2::playLevelDeathVariant(int levelId, int phase, int frame) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String variant = selectDeathVideoVariant(levelId, phase, frame);
	Common::String filename;

	if (variant.empty()) {
		filename = Common::String::format("%s/%sDIE.SAN", dir.c_str(), prefix.c_str());
	} else {
		filename = Common::String::format("%s/%sDIE_%s.SAN", dir.c_str(), prefix.c_str(), variant.c_str());
	}

	debugC(DEBUG_INSANE, "Playing death video: %s (phase=%d, frame=%d)", filename.c_str(), phase, frame);

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 15);
}

void InsaneRebel2::playLevelRetryVariant(int levelId, int phase) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename;

	if ((levelId == 3 || levelId == 6) && phase == 2) {
		filename = Common::String::format("%s/%sRETRYB.SAN", dir.c_str(), prefix.c_str());
	} else {
		filename = Common::String::format("%s/%sRETRY.SAN", dir.c_str(), prefix.c_str());
	}

	debugC(DEBUG_INSANE, "Playing retry video: %s (phase=%d)", filename.c_str(), phase);

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 15);
}


} // End of namespace Scumm
