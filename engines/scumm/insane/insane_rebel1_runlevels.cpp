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

#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

// ---------------------------------------------------------------------------
// Game flow (matching original at 0x15597)
// ---------------------------------------------------------------------------

// Play a passive cinematic (no game callback, skippable).
// Reuses RA2's pattern: reset handler, set cinematic flags, play video.
// startFrame > 0: fast-forward (decode without display/audio) to that frame.
void InsaneRebel1::playCinematic(const char *filename, int32 startFrame) {
	debug(1, "InsaneRebel1::playCinematic('%s', startFrame=%d)", filename, startFrame);
	SmushPlayer *splayer = _vm->_splayer;
	_player = splayer;
	_interactiveVideoActive = false;
	_vm->_smushVideoShouldFinish = false;
	splayer->setCurVideoFlags(0x28);  // Cinematic mode + buffer preserve
	if (startFrame > 0)
		splayer->setFastForwardToFrame(startFrame);
	splayer->play(filename, 12);
}

void InsaneRebel1::clearVideoBuffer() {
	if (_vm->_screenWidth <= 0 || _vm->_screenHeight <= 0)
		return;

	const int pixelCount = _vm->_screenWidth * _vm->_screenHeight;
	byte *clearBuffer = (byte *)calloc(pixelCount, 1);
	if (!clearBuffer)
		return;

	if (_vm->_macScreen) {
		_vm->mac_drawBufferToScreen(clearBuffer, _vm->_screenWidth, 0, 0, _vm->_screenWidth, _vm->_screenHeight);
	} else {
		_vm->_system->copyRectToScreen(clearBuffer, _vm->_screenWidth, 0, 0, _vm->_screenWidth, _vm->_screenHeight);
	}
	_vm->_system->updateScreen();

	free(clearBuffer);
}

// Intro sequence (0x155ef-0x158f8):
//   1. O1LOGO.ANM — LucasArts logo
//   2. O1OPEN.ANM — Star Wars opening crawl
void InsaneRebel1::playIntroSequence() {
	debug(1, "InsaneRebel1: Playing intro sequence");

	// LucasArts logo (original: PUSH 0x57cc, CALL FUN_1BA32 with flags 0x0420)
	playCinematic("OPEN/O1LOGO.ANM");
	if (_vm->shouldQuit())
		return;
	clearVideoBuffer();

	// Star Wars opening crawl (original: PUSH 0x5800, CALL FUN_1BA32)
	playCinematic("OPEN/O1OPEN.ANM");
}

// Main menu on O1OPTION.ANM background (0x15968).
// Original renders text overlay with 5 menu items via FUN_21F7A.
// For now, we play the menu video as a passive cinematic (non-interactive)
// and return "Start New Game" immediately.

// Level 1 flow (0x16100-0x167A2, from disassembly):
//   1. Load NUTs (L1BANK1, L1BANK2, L1EXPLD, L1BANG, L1LASER)
//   2. L1HANGAR.ANM — Full hangar departure cutscene (782 frames, flags 0x0420)
//   3. L1CU1.ANM — Pre-flight cutscene (flags 0x0400)
//   4. L1PLAY1L.ANM — Stage 1 flight, hard/left path (788 frames)
//      At frame 394, if player steers right → L1PLAY1R (easy path, 396 frames)
//   5. L1CU2.ANM — Mid-level cutscene
//   6. L1PLAY2.ANM — Stage 2 turret
//      If score < 5 (0x75D0): L1RETRY → retry Stage 2
//   7. L1END.ANM — Level complete
//   Death (health<0): L1CRASHA/B → lives check:
//     lives>0: L1NEW → jump back to Stage 1 (skip L1HANGAR/L1CU1)
//     lives==0: L1DEATH → return to menu

bool InsaneRebel1::runLevel1() {
	debug(1, "InsaneRebel1: Running level 1");

	// Load level sprites (original: pushes L1BANK1..L1BANG NUT filenames)
	loadLevelSprites(1);

	// L1HANGAR.ANM — Hangar departure (original: 0x5918, flags 0x0420)
	// Plays once at level start, never replayed on retry.
	playCinematic("LVL1/L1HANGAR.ANM");
	if (_vm->shouldQuit())
		return false;

	// L1CU1.ANM — Pre-flight cutscene (original: 0x5944, flags 0x0400)
	// Plays once at level start, never replayed on retry.
	playCinematic("LVL1/L1CU1.ANM");
	if (_vm->shouldQuit())
		return false;

	// Retry loop — on death with lives, L1NEW plays then jumps back here
	while (!_vm->shouldQuit()) {
		// Reset health for this attempt (original: MOV WORD [0x7560], 98 at 0x16214)
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_pathBranchEnabled = true;
		_rightPathSelected = false;

		// Stage 1 flight — L1PLAY1L (hard/left path)
		// The first 394 frames are the common section. At counter 394, if
		// ship is right of center, we switch to L1PLAY1R (easy path).
		playInteractiveVideo("LVL1/L1PLAY1L.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_rightPathSelected && _health >= 0) {
			debug(1, "InsaneRebel1: Switching to right path (L1PLAY1R)");
			_pathBranchEnabled = false;
			playInteractiveVideo("LVL1/L1PLAY1R.ANM");
			if (_vm->shouldQuit())
				return false;
		}
		_pathBranchEnabled = false;

		if (_health >= 0) {
			// L1CU2.ANM — Mid-level cutscene (original: 0x5977)
			playCinematic("LVL1/L1CU2.ANM");
			if (_vm->shouldQuit())
				return false;

			// L1PLAY2.ANM — Stage 2 turret (original: 0x5986)
			playInteractiveVideo("LVL1/L1PLAY2.ANM");
			if (_vm->shouldQuit())
				return false;

			// TODO: Check score threshold (original: CMP WORD [0x75D0], 5)
			// If score < 5: L1RETRY → retry Stage 2

			// L1END.ANM — Level complete! (original: 0x59a3)
			playCinematic("LVL1/L1END.ANM");
			return true;
		}

		// Death sequence (original: 0x165dd-0x166bb)
		// Random crash variant A or B
		if (_vm->_rnd.getRandomNumber(1) == 0)
			playCinematic("LVL1/L1CRASHA.ANM");
		else
			playCinematic("LVL1/L1CRASHB.ANM");
		if (_vm->shouldQuit())
			return false;

		// Check lives (original: CMP WORD [0x7562], 0 at 0x1666B)
		_lives--;
		if (_lives <= 0) {
			// Game over — L1DEATH then return (original: 0x166C0)
			playCinematic("LVL1/L1DEATH.ANM");
			debug(1, "InsaneRebel1: Game over (no lives left)");
			return false;
		}

		// Lives remaining — L1NEW briefing then retry (original: 0x16675)
		playCinematic("LVL1/L1NEW.ANM");
		if (_vm->shouldQuit())
			return false;

		// Loop back to gameplay (original: JMP 0x16214 — health reset + Stage 1)
	}

	return false;
}

// Level 2: Asteroid Field Training
// Flow: L2NEW → L2INTRO → L2PLAY (interactive) → L2END/L2DEATH
bool InsaneRebel1::runLevel2() {
	debug(1, "InsaneRebel1: Running level 2");

	_currentLevel = 1;
	loadLevelSprites(2);
	loadTuningForLevel(1);

	// L2INTRO.ANM — intro cutscene (481 frames)
	playCinematic("LVL2/L2INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	// Retry loop
	while (!_vm->shouldQuit()) {
		// Reset state for this attempt
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;
		_killCount = 0;

		// L2PLAY.ANM — asteroid dodge (800 frames, interactive)
		playInteractiveVideo("LVL2/L2PLAY.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			// Level complete!
			playCinematic("LVL2/L2END.ANM");
			return true;
		}

		// Death
		playCinematic("LVL2/L2DEATH.ANM");
		if (_vm->shouldQuit())
			return false;

		_lives--;
		if (_lives <= 0) {
			debug(1, "InsaneRebel1: Game over (no lives left)");
			return false;
		}

		// Retry briefing
		playCinematic("LVL2/L2NEW.ANM");
		if (_vm->shouldQuit())
			return false;
	}

	return false;
}

// Main game entry point — called from ScummEngine::go().
// Matches original flow at 0x15597: intro → menu → level.
void InsaneRebel1::runGame() {
	// Play intro sequence (logo + opening)
	playIntroSequence();
	if (_vm->shouldQuit())
		return;

	// Main menu → gameplay loop
	while (!_vm->shouldQuit()) {
		int menuResult = runMainMenu();
		if (_vm->shouldQuit())
			return;

		switch (menuResult) {
		case 1: {
#if 0 // Skip level 1 for testing — jump straight to level 2
			// Start New Game — play L1NEW briefing then level 1
			playCinematic("LVL1/L1NEW.ANM");
			if (_vm->shouldQuit())
				return;

			bool completed = runLevel1();
#else
			bool completed = true;
#endif
			if (completed && !_vm->shouldQuit()) {
				completed = runLevel2();
				if (completed) {
					debug(1, "InsaneRebel1: Level 2 completed!");
					// TODO: Continue to level 3
				}
			}
			_currentLevel = 0;
			// Return to menu after level ends
			break;
		}
		case 2:
			// Game Options
			runOptionsMenu();
			break;
		case 5:
			// Exit
			return;
		default:
			// Passcode, Demo — not yet implemented, return to menu
			break;
		}
	}
}

// Play interactive gameplay video (with ship physics + HUD).
void InsaneRebel1::playInteractiveVideo(const char *filename) {
	debug(1, "InsaneRebel1::playInteractiveVideo('%s')", filename);

	// Stop any leftover audio from previous video
	terminateAudio();
	initAudio(_audioSampleRate);

	SmushPlayer *splayer = _vm->_splayer;
	_player = splayer;
	clearBit(0);
	_interactiveVideoActive = true;
	_vm->_smushVideoShouldFinish = false;
	splayer->setCurVideoFlags(0x28);

	// Center mouse, hide system cursor (we draw our own), lock mouse to window
	smush_warpMouse(160, 100, -1);
	CursorMan.showMouse(false);
	g_system->lockMouse(true);

	splayer->play(filename, 12);
	_interactiveVideoActive = false;

	g_system->lockMouse(false);
}

} // End of namespace Scumm
