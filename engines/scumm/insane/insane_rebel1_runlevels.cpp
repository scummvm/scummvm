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

	// Level-title text is only meant for the intro cinematic that armed it.
	// Clear it even when the movie ended through ESC, so it cannot leak into
	// the next cutscene or gameplay segment.
	_introTextActive = false;
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

	_currentLevel = 0;
	loadTuningForLevel(0);
	loadLevelSprites(1);

	beginLevelTitleOverlay(0);
	playCinematic("LVL1/L1HANGAR.ANM");
	if (_vm->shouldQuit())
		return false;

	playCinematic("LVL1/L1CU1.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		bool stage2Started = false;

		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_killCount = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_shipDirIndex = 17;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		_perspectiveX = 0;
		_perspectiveY = 0;
		_pathBranchEnabled = true;
		_rightPathSelected = false;
		_flyControlMode = 1;

		playInteractiveVideo("LVL1/L1PLAY1L.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_rightPathSelected && _health >= 0) {
			_pathBranchEnabled = false;
			_flyControlMode = 1;
			playInteractiveVideo("LVL1/L1PLAY1R.ANM", 0x187);
			if (_vm->shouldQuit())
				return false;
		}
		_pathBranchEnabled = false;

		if (_health >= 0) {
			playCinematic("LVL1/L1CU2.ANM");
			if (_vm->shouldQuit())
				return false;

			while (!_vm->shouldQuit()) {
				_flyControlMode = 2;
				_turretEmitterLeftX = 10;
				_turretEmitterLeftY = -5;
				_turretEmitterRightX = 10;
				_turretEmitterRightY = -5;
				_activeGameOpcode = 0;
				_gameLatch5D = 0;
				_gameLatch5F = 0;
				_killCount = 0;
				stage2Started = true;

				playInteractiveVideo("LVL1/L1PLAY2.ANM");
				if (_vm->shouldQuit())
					return false;

				if (_health < 0)
					break;

				if (_killCount > 4) {
					playCinematic("LVL1/L1END.ANM");
					_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)1);
					return !_vm->shouldQuit();
				}

				playCinematic("LVL1/L1RETRY.ANM");
				if (_vm->shouldQuit())
					return false;
			}
		}

		if (stage2Started)
			playCinematic("LVL1/L1CRASHB.ANM");
		else
			playCinematic("LVL1/L1CRASHA.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_lives <= 0) {
			playCinematic("LVL1/L1DEATH.ANM");
			return false;
		}

		playCinematic("LVL1/L1NEW.ANM");
		if (_vm->shouldQuit())
			return false;
		_lives--;
	}

	return false;
}

bool InsaneRebel1::runLevel2() {
	debug(1, "InsaneRebel1: Running level 2");

	_currentLevel = 1;
	loadLevelSprites(2);
	// DOS RunLevel2Flow launches L2PLAY.ANM with gameplay selector 2.
	loadTuningForLevel(2);

	beginLevelTitleOverlay(1);
	playCinematic("LVL2/L2INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 0;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;
		_killCount = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;

		playInteractiveVideo("LVL2/L2PLAY.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			playCinematic("LVL2/L2END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)2);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL2/L2NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL2/L2DEATH.ANM");
		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel3() {
	debug(1, "InsaneRebel1: Running level 3");

	_currentLevel = 2;
	loadLevelSprites(3);
	// DOS RunLevel3Flow launches L3PLAY.ANM with gameplay selector 3.
	loadTuningForLevel(3);

	beginLevelTitleOverlay(2);
	playCinematic("LVL3/L3INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 1;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		playInteractiveVideo("LVL3/L3PLAY.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			playCinematic("LVL3/L3END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)3);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL3/L3NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL3/L3DEATH.ANM");
		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel4() {
	debug(1, "InsaneRebel1: Running level 4");

	_currentLevel = 3;
	loadLevelSprites(4);
	// DOS RunLevel4Flow launches L4PLAY1/2.ANM with gameplay selector 4.
	loadTuningForLevel(4);

	beginLevelTitleOverlay(3);
	playCinematic("LVL4/L4INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 1;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_levelGameplayPhase = 0;

		// Phase 1: Destroy two shield generators.
		// Original sets DAT_00007732=0x39, DAT_00007734=0x3A — protected target IDs
		// that can be hit repeatedly without event mask toggle.
		_protectedTargetA = 0x39;
		_protectedTargetB = 0x3A;
		_shieldGenHitsA = 0;
		_shieldGenHitsB = 0;
		_levelGameplayPhase = 1;
		playInteractiveVideo("LVL4/L4PLAY1.ANM");
		_protectedTargetA = 0;
		_protectedTargetB = 0;
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			// Phase 2: torpedo run. The DOS loop enables torpedo mode at frame
			// 0x3E and exits early as soon as killCount becomes nonzero.
			_activeGameOpcode = 0;
			_gameLatch5D = 0;
			_gameLatch5F = 0;
			_gameplayFlags75ff = 0;
			_killCount = 0;
			_levelGameplayPhase = 2;
			playInteractiveVideo("LVL4/L4PLAY2.ANM");
			if (_vm->shouldQuit())
				return false;
		}

		if (_health >= 0) {
			// L4END1 = torpedo hit, L4END2 = torpedo missed
			playCinematic((_killCount != 0) ? "LVL4/L4END1.ANM" : "LVL4/L4END2.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)4);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL4/L4NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL4/L4DEATH.ANM");
		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel5() {
	debug(1, "InsaneRebel1: Running level 5");

	_currentLevel = 4;
	loadLevelSprites(5);
	loadTuningForLevel(4);

	beginLevelTitleOverlay(4);
	playCinematic("LVL5/L5INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		loadRA1Nut("LVL5/L5LASER.NUT", _laserBank);
		_flyControlMode = 1;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		playInteractiveVideo("LVL5/L5PLAY.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health < 0) {
			if (_lives > 0) {
				playCinematic("LVL5/L5NEW.ANM");
				if (_vm->shouldQuit())
					return false;
				_lives--;
				continue;
			}

			playCinematic("LVL5/L5DEATH.ANM");
			return false;
		}

		if (_killCount <= 2) {
			if (_lives > 0) {
				_lives--;
				playCinematic("LVL5/L5RETRY.ANM");
				if (_vm->shouldQuit())
					return false;
				continue;
			}

			playCinematic("LVL5/L5DEATH2.ANM");
			return false;
		}

		playCinematic("LVL5/L5BINTRO.ANM");
		if (_vm->shouldQuit())
			return false;

		loadRA1Nut("LVL5/L5LASER2.NUT", _laserBank);
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_killCount = 0;
		playInteractiveVideo("LVL5/L5PLAY2.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			playCinematic("LVL5/L5END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)5);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL5/L5NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL5/L5DEATH.ANM");
		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel6() {
	debug(1, "InsaneRebel1: Running level 6");

	_currentLevel = 5;
	loadLevelSprites(6);
	loadTuningForLevel(5);

	beginLevelTitleOverlay(5);
	playCinematic("LVL6/L6INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 1;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		playInteractiveVideo("LVL6/L6PLAY.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			playCinematic("LVL6/L6END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)6);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL6/L6NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL6/L6DEATH.ANM");
		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel7() {
	debug(1, "InsaneRebel1: Running level 7");

	static const char *const kLevel7Segments[] = {
		"LVL7/L7PLAY1.ANM",
		"LVL7/L7PLAY2.ANM",
		"LVL7/L7PLAY3.ANM",
		"LVL7/L7PLAY4.ANM",
		"LVL7/L7PLAY5.ANM",
		"LVL7/L7PLAY6.ANM"
	};

	_currentLevel = 6;
	loadLevelSprites(7);
	loadTuningForLevel(6);

	beginLevelTitleOverlay(6);
	playCinematic("LVL7/L7INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 3;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_shipDirIndex = 17;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		_perspectiveX = 0;
		_perspectiveY = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		int route = 0;
		while (!_vm->shouldQuit()) {
			_levelRouteIndex = route;
			_pendingRouteIndex = -1;
			playInteractiveVideo(kLevel7Segments[route]);
			if (_vm->shouldQuit())
				return false;

			if (_health < 0)
				break;

			if (_pendingRouteIndex < 0 || _pendingRouteIndex == route)
				break;

			route = _pendingRouteIndex;
		}

		_levelRouteIndex = -1;
		_pendingRouteIndex = -1;

		if (_health >= 0) {
			playCinematic("LVL7/L7END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)7);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL7/L7NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL7/L7DEATH.ANM");
		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel8() {
	debug(1, "InsaneRebel1: Running level 8");

	static const char *const kLevel8Routes[] = {
		"LVL8/L8PLAY.ANM",
		"LVL8/L8PLAY2.ANM",
		"LVL8/L8PLAY3.ANM"
	};

	_currentLevel = 7;
	loadLevelSprites(8);
	loadTuningForLevel(7);

	beginLevelTitleOverlay(7);
	playCinematic("LVL8/L8INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 3;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_shipDirIndex = 17;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		_perspectiveX = 0;
		_perspectiveY = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		// Walker-specific state — RunLevel8Flow (0x18546)
		_walkerHealth = 100;
		_walkerTimer = 0;
		_walkerBranchChoice = 0;

		// g_level8HitboxBuffer (0x7698) = _frameObjectState[150..299] filled with 0xFF.
		// This enables all frame object event masks in the secondary half of the array,
		// which the IACT 0x5A handler uses to gate walker-related frame objects.
		memset(_frameObjectState + 150, 0xFF, 150);

		int route = 0;
		while (!_vm->shouldQuit()) {
			_levelRouteIndex = route;
			_pendingRouteIndex = -1;
			playInteractiveVideo(kLevel8Routes[route]);
			if (_vm->shouldQuit())
				return false;

			if (_health < 0)
				break;

			if (_pendingRouteIndex < 0 || _pendingRouteIndex == route)
				break;

			route = _pendingRouteIndex;
		}

		_levelRouteIndex = -1;
		_pendingRouteIndex = -1;

		if (_health >= 0) {
			playCinematic("LVL8/L8END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)8);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL8/L8NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL8/L8DEATH.ANM");
		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel9() {
	debug(1, "InsaneRebel1: Running level 9");

	const int randPath1 = _vm->_rnd.getRandomNumber(1);
	const int randPath2 = _vm->_rnd.getRandomNumber(1);
	const int randPath3 = _vm->_rnd.getRandomNumber(1);

	_currentLevel = 8;
	loadLevelSprites(9);
	loadTuningForLevel(8);

	beginLevelTitleOverlay(8);
	playCinematic("LVL9/L9INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 0;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_shipDirIndex = 15;  // On-foot center direction
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		_perspectiveX = 0;
		_perspectiveY = 0;
		_levelGameplayPhase = 0;
		_onFootCharX = 0;
		_onFootCharY = 0;
		_onFootAnimCounter = 0;
		_onFootInitialized = false;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		while (!_vm->shouldQuit()) {
			playInteractiveVideo("LVL9/L9PLAY1.ANM");
			if (_vm->shouldQuit())
				return false;
			if (_health < 0)
				break;

			playCinematic("LVL9/L9CUT1.ANM");
			if (_vm->shouldQuit())
				return false;

			_shipPosX = kRA1CenterX;
			_posAccumX = 0;
			playInteractiveVideo("LVL9/L9PLAY2.ANM");
			if (_vm->shouldQuit())
				return false;
			if (_health < 0)
				break;

			_gameplayFlags75fe |= 4;
			const int side1 = (_shipPosX < kRA1CenterX) ? 0 : 1;
			playCinematic(side1 == 0 ? "LVL9/L9PLAY2A.ANM" : "LVL9/L9PLAY2B.ANM");
			if (_vm->shouldQuit())
				return false;

			if (side1 == randPath1) {
				playCinematic("LVL9/L9CUT2A.ANM");
				if (_vm->shouldQuit())
					return false;
				continue;
			}

			playCinematic("LVL9/L9CUT2B.ANM");
			if (_vm->shouldQuit())
				return false;

			playInteractiveVideo("LVL9/L9PLAY3A.ANM");
			if (_vm->shouldQuit())
				return false;
			if (_health < 0)
				break;

			if (_killCount < 15) {
				playInteractiveVideo("LVL9/L9PLAY3B.ANM");
				if (_vm->shouldQuit())
					return false;
				if (_health < 0)
					break;
			}

			playCinematic("LVL9/L9CUT3.ANM");
			if (_vm->shouldQuit())
				return false;

			_shipPosX = kRA1CenterX;
			_posAccumX = 0;
			playInteractiveVideo("LVL9/L9PLAY4.ANM");
			if (_vm->shouldQuit())
				return false;
			if (_health < 0)
				break;

			_gameplayFlags75fe |= 4;
			const int side2 = (_shipPosX < kRA1CenterX) ? 0 : 1;
			playCinematic(side2 == 0 ? "LVL9/L9PLAY4A.ANM" : "LVL9/L9PLAY4B.ANM");
			if (_vm->shouldQuit())
				return false;

			if (side2 == randPath2) {
				playCinematic(side2 == 0 ? "LVL9/L9CUT4AX.ANM" : "LVL9/L9CUT4B.ANM");
				if (_vm->shouldQuit())
					return false;

				playInteractiveVideo("LVL9/L9PLAY5.ANM");
				if (_vm->shouldQuit())
					return false;
				if (_health < 0)
					break;

				playCinematic("LVL9/L9CUT5.ANM");
				if (_vm->shouldQuit())
					return false;

				_shipPosX = kRA1CenterX;
				_posAccumX = 0;
				playInteractiveVideo("LVL9/L9PLAY6.ANM");
				if (_vm->shouldQuit())
					return false;
				if (_health < 0)
					break;

				_gameplayFlags75fe |= 4;
				const int side3 = (_shipPosX < kRA1CenterX) ? 0 : 1;
				if (side3 == randPath3) {
					playCinematic("LVL9/L9CUT6A.ANM");
					if (_vm->shouldQuit())
						return false;
					continue;
				}

				playCinematic("LVL9/L9CUT6B.ANM");
				if (_vm->shouldQuit())
					return false;
			} else {
				playCinematic(side2 == 0 ? "LVL9/L9CUT4A.ANM" : "LVL9/L9CUT4BX.ANM");
				if (_vm->shouldQuit())
					return false;
			}

			playInteractiveVideo("LVL9/L9PLAY7.ANM");
			if (_vm->shouldQuit())
				return false;
			if (_health < 0)
				break;

			playCinematic("LVL9/L9END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)9);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL9/L9NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL9/L9DEATH.ANM");
		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel10() {
	debug(1, "InsaneRebel1: Running level 10");

	_currentLevel = 9;
	loadLevelSprites(10);
	loadTuningForLevel(9);

	beginLevelTitleOverlay(9);
	playCinematic("LVL10/L10INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 1;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_shipDirIndex = 17;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		_perspectiveX = 0;
		_perspectiveY = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		playInteractiveVideo("LVL10/L10PLAY.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			playCinematic("LVL10/L10END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)10);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL10/L10NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL10/L10DEATH.ANM");
		return false;
	}

	return false;
}

// Level 11 flow (RunLevel11Flow, 0x19F67): Yavin Training
// Turret-style level. Single interactive phase with kill-count retry.
// Original: L11INTRO → L11PLAY (turret, killCount>4 to pass) → L11RETRY → retry/L11END
bool InsaneRebel1::runLevel11() {
	debug(1, "InsaneRebel1: Running level 11");

	_currentLevel = 10;
	loadLevelSprites(11);
	loadTuningForLevel(10);

	beginLevelTitleOverlay(10);
	playCinematic("LVL11/L11INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 1;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_shipDirIndex = 17;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		_perspectiveX = 0;
		_perspectiveY = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;
		_turretEmitterLeftX = 25;
		_turretEmitterLeftY = 15;

		while (!_vm->shouldQuit()) {
			playInteractiveVideo("LVL11/L11PLAY.ANM");
			if (_vm->shouldQuit())
				return false;

			if (_health < 0)
				break;

			// Original: killCount > 4 means pass
			if (_killCount > 4)
				break;

			// Not enough kills — retry
			playCinematic("LVL11/L11RETRY.ANM");
			if (_vm->shouldQuit())
				return false;
		}

		if (_health >= 0) {
			playCinematic("LVL11/L11END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)11);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL11/L11NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL11/L11DEATH.ANM");
		return false;
	}

	return false;
}

// Level 12 flow (RunLevel12Flow, 0x1A2DD): TIE Attack
// Single interactive phase with mid-level retry mechanism.
// Original: L12INTRO → L12PLAY → (retry at specific frame) → L12END
bool InsaneRebel1::runLevel12() {
	debug(1, "InsaneRebel1: Running level 12");

	_currentLevel = 11;
	loadLevelSprites(12);
	loadTuningForLevel(11);

	beginLevelTitleOverlay(11);
	playCinematic("LVL12/L12INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 1;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_shipDirIndex = 17;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		_perspectiveX = 0;
		_perspectiveY = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		playInteractiveVideo("LVL12/L12PLAY.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			playCinematic("LVL12/L12END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)12);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL12/L12NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL12/L12DEATH.ANM");
		return false;
	}

	return false;
}

// Level 13 flow (RunLevel13Flow, 0x1A6E3): Death Star Surface
// Flight level with enemy projectile system (original has 5-slot projectile tracking).
// Original: L13INTRO → L13PLAY → L13END/L13NEW/L13DEATH
bool InsaneRebel1::runLevel13() {
	debug(1, "InsaneRebel1: Running level 13");

	_currentLevel = 12;
	loadLevelSprites(13);
	loadTuningForLevel(12);

	beginLevelTitleOverlay(12);
	playCinematic("LVL13/L13INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 1;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_shipDirIndex = 17;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		_perspectiveX = 0;
		_perspectiveY = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		playInteractiveVideo("LVL13/L13PLAY.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			playCinematic("LVL13/L13END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)13);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL13/L13NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL13/L13DEATH.ANM");
		return false;
	}

	return false;
}

// Level 14 flow (RunLevel14Flow, 0x1ACB0): Surface Cannon
// Two interactive phases: L14PLAY (targeting cannons) + L14PLAY2 (exhaust port approach).
// Original: L14INTRO → L14PLAY → L14PLAY2 → L14END/L14NEW/L14DEATH
bool InsaneRebel1::runLevel14() {
	debug(1, "InsaneRebel1: Running level 14");

	_currentLevel = 13;
	loadLevelSprites(14);
	loadTuningForLevel(13);

	beginLevelTitleOverlay(13);
	playCinematic("LVL14/L14INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 1;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_shipDirIndex = 17;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		_perspectiveX = 0;
		_perspectiveY = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		// Phase 1: targeting surface cannons
		playInteractiveVideo("LVL14/L14PLAY.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			// Phase 2: exhaust port approach
			_activeGameOpcode = 0;
			_gameLatch5D = 0;
			_gameLatch5F = 0;
			_gameplayFlags75ff = 0;
			_killCount = 0;

			playInteractiveVideo("LVL14/L14PLAY2.ANM");
			if (_vm->shouldQuit())
				return false;
		}

		if (_health >= 0) {
			playCinematic("LVL14/L14END.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)14);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL14/L14NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL14/L14DEATH.ANM");
		return false;
	}

	return false;
}

// Level 15 flow (RunLevel1GameLoop, 0x1B283): Death Star Trench
// Two interactive phases with mid-level cutscene.
// Original: L15INTRO → L15PLAY1 (trench run) → L15INTR2 (torpedo lock cutscene)
//   → L15PLAY2 (final approach + torpedo) → L15END1/L15NEW/L15DEATH
bool InsaneRebel1::runLevel15() {
	debug(1, "InsaneRebel1: Running level 15");

	_currentLevel = 14;
	loadLevelSprites(15);
	loadTuningForLevel(14);

	beginLevelTitleOverlay(14);
	playCinematic("LVL15/L15INTRO.ANM");
	if (_vm->shouldQuit())
		return false;

	while (!_vm->shouldQuit()) {
		_flyControlMode = 1;
		_health = kMaxHealth;
		_damageFlags = 0;
		_prevDamageFlags = 0;
		_damageCooldown = 0;
		_deathTimer = 0;
		_screenFlash = 0;
		_screenShakeEnabled = false;
		_deathCauseIndicator = 0;
		_frameCounter = 0;
		_gameCounter = 0;
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_gameplayFlags75ff = 0;
		_killCount = 0;
		_targetCount = 0;
		_prevTargetCount = 0;
		_lastHitTarget = 0;
		_shipPosX = kRA1CenterX;
		_shipPosY = kRA1CenterY;
		_shipDirIndex = 17;
		_rollAccum = 0;
		_liftSmooth = 0;
		_posAccumX = 0;
		_posAccumY = 0;
		_perspectiveX = 0;
		_perspectiveY = 0;
		_levelGameplayPhase = 0;
		memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
		memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
		memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
		memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
		_avgInputX = 0;
		_avgInputY = 0;

		// Phase 1: trench run
		_levelGameplayPhase = 1;
		playInteractiveVideo("LVL15/L15PLAY1.ANM");
		if (_vm->shouldQuit())
			return false;

		if (_health >= 0) {
			// Torpedo lock cutscene
			playCinematic("LVL15/L15INTR2.ANM");
			if (_vm->shouldQuit())
				return false;

			// Phase 2: final approach and torpedo shot. The DOS flow enables
			// torpedo mode at frame 0x18A and completes only after object-state
			// bit 0x7602 & 2 is set by the exhaust-port hit.
			_activeGameOpcode = 0;
			_gameLatch5D = 0;
			_gameLatch5F = 0;
			_gameplayFlags75ff = 0;
			_killCount = 0;
			_torpedoFired = false;
			_levelGameplayPhase = 2;

			playInteractiveVideo("LVL15/L15PLAY2.ANM");
			if (_vm->shouldQuit())
				return false;
		}

		if (_health >= 0 && !_torpedoFired) {
			debug(1, "InsaneRebel1: Level 15 torpedo run ended without exhaust-port hit");
			return false;
		}

		if (_health >= 0) {
			playCinematic("LVL15/L15END1.ANM");
			_maxChapterUnlocked = MAX(_maxChapterUnlocked, (int16)15);
			return !_vm->shouldQuit();
		}

		if (_lives > 0) {
			playCinematic("LVL15/L15NEW.ANM");
			if (_vm->shouldQuit())
				return false;
			_lives--;
			continue;
		}

		playCinematic("LVL15/L15DEATH.ANM");
		return false;
	}

	return false;
}

// Main game entry point — called from ScummEngine::go().
// Matches original flow at 0x15597: intro → menu → level.
void InsaneRebel1::runGame() {
	typedef bool (InsaneRebel1::*RunLevelMethod)();
	static const RunLevelMethod kLevelRunners[] = {
		&InsaneRebel1::runLevel1,
		&InsaneRebel1::runLevel2,
		&InsaneRebel1::runLevel3,
		&InsaneRebel1::runLevel4,
		&InsaneRebel1::runLevel5,
		&InsaneRebel1::runLevel6,
		&InsaneRebel1::runLevel7,
		&InsaneRebel1::runLevel8,
		&InsaneRebel1::runLevel9,
		&InsaneRebel1::runLevel10,
		&InsaneRebel1::runLevel11,
		&InsaneRebel1::runLevel12,
		&InsaneRebel1::runLevel13,
		&InsaneRebel1::runLevel14,
		&InsaneRebel1::runLevel15
	};

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
			// START NEW GAME — sequential play from _startLevel
			const int numLevels = (int)(sizeof(kLevelRunners) / sizeof(kLevelRunners[0]));
			const int startLevel = CLIP<int>(_startLevel, 1, numLevels);
			bool completed = true;

			for (int level = startLevel;
				 level <= numLevels && completed && !_vm->shouldQuit();
				 ++level) {
				completed = (this->*kLevelRunners[level - 1])();
				if (completed && level < numLevels)
					_startLevel = level + 1;
			}
			_currentLevel = 0;
			break;
		}
		case 2:
			// Game Options
			runOptionsMenu();
			break;
		case 3: {
			// Level Select — launch directly
			int selectedLevel = runLevelSelectMenu();
			if (selectedLevel >= 1 && selectedLevel <= (int)(sizeof(kLevelRunners) / sizeof(kLevelRunners[0]))) {
				_startLevel = selectedLevel;
				(this->*kLevelRunners[selectedLevel - 1])();
				_currentLevel = 0;
			}
			break;
		}
		case 4:
			// CONTINUE DEMO — attract mode.
			// Original shows TOP PILOTS (O1SCORE.ANM) then loops O1OPEN.ANM.
			showHighScores();
			if (!_vm->shouldQuit())
				playCinematic("OPEN/O1OPEN.ANM");
			break;
		case 5:
			// Exit
			return;
		default:
			break;
		}
	}
}

// Play interactive gameplay video (with ship physics + HUD).
void InsaneRebel1::playInteractiveVideo(const char *filename, int32 startFrame) {
	debug(1, "InsaneRebel1::playInteractiveVideo('%s', startFrame=%d)", filename, startFrame);

	// Stop any leftover audio from previous video
	terminateAudio();
	initAudio(_audioSampleRate);

	SmushPlayer *splayer = _vm->_splayer;
	_player = splayer;
	clearBit(0);
	_interactiveVideoActive = true;
	_levelRouteChoice = 0;
	_onFootInitialized = false;  // Reset so each segment triggers counter==0 init
	resetFrameObjectState();
	_vm->_smushVideoShouldFinish = false;
	splayer->setCurVideoFlags(0x28);
	if (startFrame > 0)
		splayer->setFastForwardToFrame(startFrame);

	// Center mouse, hide system cursor (we draw our own), lock mouse to window
	smush_warpMouse(160, 100, -1);
	CursorMan.showMouse(false);
	g_system->lockMouse(true);

	splayer->play(filename, 12);
	_interactiveVideoActive = false;

	g_system->lockMouse(false);
}

} // End of namespace Scumm
