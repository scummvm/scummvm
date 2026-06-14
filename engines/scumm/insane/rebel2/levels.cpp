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

#include "graphics/cursorman.h"

#include "scumm/scumm_v7.h"

#include "scumm/smush/smush_player.h"

#include "scumm/insane/rebel2/rebel.h"

namespace Scumm {

static const int kRebel2GameplayAimCenterX = 160;
static const int kRebel2GameplayAimCenterY = 100;
static const uint32 kRebel2GameplayMouseSettleMs = 1000;

static void purgeRebel2GameplayInputEvents(Common::EventManager *eventMan) {
	if (!eventMan)
		return;

	eventMan->getEventDispatcher()->clearEvents();
	eventMan->purgeMouseEvents();
	eventMan->purgeKeyboardEvents();
}

// ---------------------------------------------------------------------------
// Level Loading System
// ---------------------------------------------------------------------------
// Emulates the level handler functions from FUN_00417E53 through FUN_0041BBE8.
// Based on disassembly analysis of the Rebel Assault 2 executable.

Common::String InsaneRebel2::getLevelDir(int levelId) {
	return Common::String::format("LEV%02d", levelId);
}

Common::String InsaneRebel2::getLevelPrefix(int levelId) {
	return Common::String::format("%02d", levelId);
}

//
// runGame -- Main game entry point (FUN_004142BD)
//
// Full game loop: intro, main menu, pilot select, chapter select, level
// progression. Called from ScummEngine::go().
//
void InsaneRebel2::runGame() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Demo: just play the demo video and return
	if (_vm->_game.features & GF_DEMO) {
		splayer->play("OPEN/O_DEMO.SAN", 15);
		return;
	}

	// Case 0: Play intro sequence (Fox logo, LucasArts logo, O_OPEN_A, O_OPEN_B)
	playIntroSequence();

	// Cases 1-4: Main menu -> pilot select -> chapter select -> gameplay loop
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
				// _selectedChapter is 0-based, runLevel expects 1-based
				int selectedLevel = _selectedChapter + 1;
				debugC(DEBUG_INSANE, "InsaneRebel2: Starting chapter %d (level %d)", _selectedChapter + 1, selectedLevel);

				// Ending selected directly from chapter select (FUN_0041bbe8, case 0xf)
				if (selectedLevel == 16) {
					playEndingSequence();
				}

				// Level progression loop: on success, advance to next level
				while (!_vm->shouldQuit() && selectedLevel >= 1 && selectedLevel <= 15) {
					int result = runLevel(selectedLevel);

					if (result == kLevelNextLevel) {
						updatePilotProgress(selectedLevel - 1,
							_playerScore, _playerLives, _playerDamage);
						selectedLevel++;
						if (selectedLevel > 15) {
							playEndingSequence();
							break;
						}
					} else {
						if (_vm->shouldQuit() || result == kLevelQuit)
							break;
						break;
					}
				}

				if (_vm->shouldQuit())
					break;
			}
			// If kChapterSelectBack, loop back to main menu
		}
	}
}

//
// playIntroSequence -- Intro sequence (FUN_004142BD case 0)
//
// Original flow:
//   - If 'f','o','x' keys all held: play CREDITS/O_OPEN_C.SAN (Fox logo easter egg)
//   - If 'b','o','t' keys all held: play CREDITS/O_OPEN_D.SAN (LucasArts logo)
//   - Else: play OPEN/O_OPEN_A.SAN (main intro)
//   - If DAT_0047ab45 || DAT_0047ab47: play OPEN/O_OPEN_B.SAN (additional intro)
//
// We skip the easter eggs and play both O_OPEN_A + O_OPEN_B unconditionally.
//
void InsaneRebel2::playIntroSequence() {
	debugC(DEBUG_INSANE, "Playing intro sequence");

	_gameState = kStateIntro;
	_menuInputActive = false;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play main intro (OPEN/O_OPEN_A.SAN)
	// Original: FUN_0041f4d0("OPEN/O_OPEN_A.SAN", 0x28, 0xffff, 0xffff, 0)
	debugC(DEBUG_INSANE, "Playing main intro (O_OPEN_A.SAN)");
	splayer->setCurVideoFlags(0x28);
	splayer->play("OPEN/O_OPEN_A.SAN", 15);

	if (_vm->shouldQuit())
		return;

	// Play additional intro (OPEN/O_OPEN_B.SAN)
	// Original: conditional on DAT_0047ab45 || DAT_0047ab47
	// We play unconditionally (matches "Continue Intro" menu behavior)
	debugC(DEBUG_INSANE, "Playing additional intro (O_OPEN_B.SAN)");
	splayer->setCurVideoFlags(0x28);
	splayer->play("OPEN/O_OPEN_B.SAN", 15);
}

// playMissionBriefing -- Mission briefing screen (FUN_00415CF8).
void InsaneRebel2::playMissionBriefing() {

	debugC(DEBUG_INSANE, "Playing mission briefing");

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x08);  // Briefing mode flag
	splayer->play("OPEN/O_LEVEL.SAN", 15);
}

// playCinematic -- Play a cinematic/cutscene video.
// Resets handler to 0 (no HUD) and sets flags to 0x28 (cinematic + buffer preserve).
// All wrapper functions (FUN_00417168/4171c5/417ab2/417327) add | 8 before calling FUN_0041f4d0.
void InsaneRebel2::playCinematic(const char *filename) {
	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // No status bar during cinematics

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x28);  // Cinematic mode + buffer preserve (0x20 | 0x08)
	splayer->play(filename, 15);
}

// playVideoWithText -- Video with progressive text overlay (FUN_004171c5).
// displayLength = currentFrame + 10 - fadeInFrame, capped at 0xBE (190) chars.
// Text rendered at (textX, textY) via FUN_004341a0.
void InsaneRebel2::playVideoWithText(const char *filename, int textID, int textX, int textY,
                                     int fadeInFrame, int fadeOutFrame) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;

	// Set up text overlay state — procPostRendering reads these each frame
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

// playLevelBegin -- Level beginning cinematic (LEVXX/XXBEG.SAN).
// Uses per-level text overlay parameters from GAME.TRS via FUN_004171c5.
void InsaneRebel2::playLevelBegin(int levelId) {

	struct TextOverlayParams {
		int textID;      // TRS string ID (-1 = no text overlay)
		int textX;
		int textY;
		int fadeInFrame;
		int fadeOutFrame;
	};

	// Table of per-level text overlay parameters
	// All levels use FUN_004171c5 (verified against decompiled level handlers)
	// Text IDs are sequential: 0xAA (level 1) through 0xB8 (level 15)
	const TextOverlayParams levelTextParams[16] = {
		{ -1,   0,  0,   0,    0},    // Level 0 (unused)
		{0xAA, 0xA0, 10,   5, 0x4B},  // Level 1:  FUN_00417E53
		{0xAB, 0xA0, 10,   2, 0x46},  // Level 2:  FUN_00418063
		{0xAC, 0xA0, 10,   2, 0x46},  // Level 3:  FUN_0041885F
		{0xAD, 0xA0, 10,   2,  100},  // Level 4:  FUN_00418CC4
		{0xAE, 0xA0, 10,   5, 0x3C},  // Level 5:  FUN_00418EC6
		{0xAF, 0xA0, 10,   5, 0x4B},  // Level 6:  FUN_004190D6
		{0xB0, 0xA0, 10,   5, 0x4B},  // Level 7:  FUN_0041974C
		{0xB1, 0xA0, 10,   5, 0x4B},  // Level 8:  FUN_00419976
		{0xB2, 0xA0, 10, 200, 0x10E}, // Level 9:  FUN_00419B86
		{0xB3, 0xA0, 10,   2, 0x46},  // Level 10: FUN_00419E0A
		{0xB4, 0xA0, 10,   2, 0x46},  // Level 11: FUN_0041A00C
		{0xB5, 0xA0, 10,   5, 0x4B},  // Level 12: FUN_0041A3EB
		{0xB6, 0xA0, 10,   2, 0x46},  // Level 13: FUN_0041A806
		{0xB7, 0xA0, 10,   2, 0x46},  // Level 14: FUN_0041ABB2
		{0xB8, 0xA0, 10,   2, 0x46},  // Level 15: FUN_0041AEE8
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

// playLevelEnd -- Level completion video (FUN_00417327).
void InsaneRebel2::playLevelEnd(int levelId) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // No status bar during end cinematic

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sEND.SAN", dir.c_str(), prefix.c_str());

	debugC(DEBUG_INSANE, "Playing level %d end: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417327 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 15);
}

// playLevelRetry -- Retry prompt video (LEVXX/XXRETRY.SAN, FUN_00417168).
void InsaneRebel2::playLevelRetry(int levelId) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // Reset for retry - will be set by IACT opcode 6 if needed

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sRETRY.SAN", dir.c_str(), prefix.c_str());

	debugC(DEBUG_INSANE, "Playing level %d retry: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 15);
}

// playLevelGameOver -- Game over video (FUN_00417ab2).
void InsaneRebel2::playLevelGameOver(int levelId) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // No status bar during game over cinematic

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sOVER.SAN", dir.c_str(), prefix.c_str());

	debugC(DEBUG_INSANE, "Playing level %d game over: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417ab2 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 15);
}

// playEndingSequence -- Finale + credits + epilogue (FUN_0041bbe8).
//
// Original flow:
//   1. Play difficulty-dependent finale video:
//      - Difficulty 2: FINAL/F_FIN_B.SAN
//      - Difficulty 3: FINAL/F_FIN_C.SAN
//      - Default:      FINAL/F_FIN_A.SAN
//   2. Play credits: FINAL/F_CREDIT.SAN
//   3. Play epilogue: FINAL/F_EPILOG.SAN
//   4. Return to main menu
//
void InsaneRebel2::playEndingSequence() {

	debugC(DEBUG_INSANE, "Playing ending sequence (difficulty=%d)", _difficulty);

	// Switch to gameplay state to stop menu overlay rendering
	_gameState = kStateGameplay;
	_menuInputActive = false;

	// Clear the screen to remove any leftover menu pixels
	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
	memset(vs->getPixels(0, 0), 0, vs->pitch * vs->h);
	_vm->markRectAsDirty(kMainVirtScreen, 0, vs->w, 0, vs->h);

	// Difficulty-dependent finale video
	if (_difficulty == 2) {
		playCinematic("FINAL/F_FIN_B.SAN");
	} else if (_difficulty == 3) {
		playCinematic("FINAL/F_FIN_C.SAN");
	} else {
		playCinematic("FINAL/F_FIN_A.SAN");
	}

	if (_vm->shouldQuit())
		return;

	// Credits
	playCinematic("FINAL/F_CREDIT.SAN");

	if (_vm->shouldQuit())
		return;

	// Epilogue
	playCinematic("FINAL/F_EPILOG.SAN");
}

// playCreditsSequence -- Main menu credits (OPEN/O_CREDIT.SAN).
// This is the credits accessible from the main menu, NOT the ending credits.
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

// runLevel -- Main level dispatcher, calls per-level handlers.
int InsaneRebel2::runLevel(int levelId) {

	debugC(DEBUG_INSANE, "Starting level %d", levelId);

	// Validate level ID
	if (levelId < 1 || levelId > 15) {
		warning("Rebel2: Invalid level ID %d", levelId);
		return kLevelReturnToMenu;
	}

	// Switch to gameplay state to stop menu overlay rendering
	_gameState = kStateGameplay;
	_menuInputActive = false;

	// Clear the screen to remove any leftover menu pixels
	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
	memset(vs->getPixels(0, 0), 0, vs->pitch * vs->h);
	_vm->markRectAsDirty(kMainVirtScreen, 0, vs->w, 0, vs->h);

	// Set the current level
	_selectedLevel = levelId;

	// Set the level type for difficulty table lookup (DAT_0047a7f8).
	// Each original level function sets this before gameplay starts.
	// Levels 1-6 use types 0-5, but Level 6 also uses type 6 mid-level.
	// Levels 7-15 use types 7-15 (gap at type 6 which is Level 6 phase 2).
	// Level 15 also switches to type 16 mid-level at frame 0x21e.
	const int kLevelTypeMap[16] = {
		-1, 0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15
	};
	_rebelLevelType = kLevelTypeMap[levelId];

	// Lock the mouse to the game window during gameplay.
	// The original hides the cursor (ShowCursor(0)) and relies on Windows confining
	// the mouse to the game window. Without locking, the cursor can escape the
	// window making the ship uncontrollable.
	_gameplaySectionActive = false;
	CursorMan.showMouse(false);
	g_system->lockMouse(true);

	// Initialize common player state
	_playerLives = 3;
	_playerShield = 255;
	_playerScore = 0;
	_playerDamage = 0;
	resetDamageFlash();
	_damageHighFlashCounter = 0;
	_damageShakeCounter = 0;
	_currentPhase = 1;
	_skipSectionRequested = false;

	// Dispatch to per-level handler
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

	// Unlock the mouse when returning to menu
	restoreIOSGamepadController();
	g_system->lockMouse(false);
	CursorMan.showMouse(true);

	return result;
}

// ---------------------------------------------------------------------------
// Helper Functions
// ---------------------------------------------------------------------------

// Emulates FUN_004233a0.
int InsaneRebel2::getRandomVariant(int max) {
	return _vm->_rnd.getRandomNumber(max - 1);
}

//
// selectDeathVideoVariant -- Frame-based death video selection
//
// Returns variant suffix ("A", "B", "C", etc.) based on level, phase,
// and the frame where the player died. Emulates the per-level frame
// threshold tables in the original level handlers.
//
Common::String InsaneRebel2::selectDeathVideoVariant(int levelId, int phase, int frame) {

	switch (levelId) {
	case 1:
		// Level 1: Random between A and B
		// Original: random!=0 → A (offset 0), random==0 → B (offset 0x14)
		return (getRandomVariant(2) == 0) ? "B" : "A";

	case 2:
		// Level 2: Just "DIE" (no variants)
		return "";

	case 3:
		// Level 3: Based on death frame and phase
		if (phase == 1) {
			// Phase 1 death video selection (from FUN_0041885F lines 80-96)
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
			// Phase 2 death video selection (from FUN_0041885F lines 53-67)
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
		// Level 4: Single variant "A" (original plays 04DIE_A.SAN)
		return "A";

	case 5:
		// Level 5: Random between A and B
		// Original: random!=0 → A (offset 0), random==0 → B (offset 0x14)
		return (getRandomVariant(2) == 0) ? "B" : "A";

	case 6:
		// Level 6 (FUN_004190D6): Phase-based with detailed frame selection
		if (phase == 1) {
			// DAT_0047a7f8 == 5 (phase 1)
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
			// DAT_0047a7f8 == 6 (phase 2)
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
		// Level 7 (FUN_0041974C): Based on DAT_0047ab8c (fork state)
		// DAT_0047ab8c != 0 → DIE_B; == 0 → DIE_A
		// We use phase as a proxy (phase 2 = reached fork)
		return (phase >= 2) ? "B" : "A";

	case 8:
		// Level 8 (FUN_00419976): Random A or B
		// Original: random!=0 → A (offset 0), random==0 → B (offset 0x14)
		return (getRandomVariant(2) == 0) ? "B" : "A";

	case 9:
		// Level 9 (FUN_00419B86): Based on DAT_0047ab94 (death cause)
		// 0→A, 1→C, else→B. DAT_0047ab94 is not tracked yet.
		return "A";

	case 10:
		// Level 10 (FUN_00419E0A): Single death video (no variant suffix)
		return "";

	case 11:
		// Level 11 (FUN_0041A00C): Phase-based death videos
		// Phase 1 → DIE_A, Phase 2 → DIE_B, Phase 3 → DIE_C
		if (phase <= 1)
			return "A";
		if (phase == 2)
			return "B";
		return "C";

	case 12:
		// Level 12 (FUN_0041AA14): Single death video (no variants)
		return "";

	case 13:
		// Level 13 (FUN_0041B3E1): Frame-based
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
		// Level 14 (FUN_0041B6E8): Single death video (no variant suffix)
		return "";

	case 15:
		// Level 15 (FUN_0041B8D7): Frame-based with many thresholds
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

// playLevelDeathVariant -- Death video with variant selection.
void InsaneRebel2::playLevelDeathVariant(int levelId, int phase, int frame) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // No status bar during death cinematic

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String variant = selectDeathVideoVariant(levelId, phase, frame);
	Common::String filename;

	if (variant.empty()) {
		// No variant suffix.
		filename = Common::String::format("%s/%sDIE.SAN", dir.c_str(), prefix.c_str());
	} else {
		filename = Common::String::format("%s/%sDIE_%s.SAN", dir.c_str(), prefix.c_str(), variant.c_str());
	}

	debugC(DEBUG_INSANE, "Playing death video: %s (phase=%d, frame=%d)", filename.c_str(), phase, frame);

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 15);
}

// playLevelRetryVariant -- Phase-specific retry video.
void InsaneRebel2::playLevelRetryVariant(int levelId, int phase) {

	restoreDamageFlashPalette();
	resetVideoAudio();
	_gameplaySectionActive = false;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // Reset for retry - will be set by IACT opcode 6 if needed

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename;

	if ((levelId == 3 || levelId == 6) && phase == 2) {
		// Level 3/6 phase 2 has its own retry video: xxRETRYB.SAN
		filename = Common::String::format("%s/%sRETRYB.SAN", dir.c_str(), prefix.c_str());
	} else {
		// Standard retry video
		filename = Common::String::format("%s/%sRETRY.SAN", dir.c_str(), prefix.c_str());
	}

	debugC(DEBUG_INSANE, "Playing retry video: %s (phase=%d)", filename.c_str(), phase);

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 15);
}


} // End of namespace Scumm
