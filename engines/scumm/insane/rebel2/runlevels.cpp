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

#include "scumm/scumm_v7.h"

#include "scumm/smush/smush_player.h"

#include "scumm/insane/rebel2/rebel.h"

namespace Scumm {

// ---------------------------------------------------------------------------
// Level 1 Handler - FUN_00417E53
// Single gameplay phase (01P01.SAN)
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel1() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play level beginning cinematic (01BEG.SAN)
	playLevelBegin(1);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// Main gameplay retry loop
	while (!_vm->shouldQuit()) {
		// Reset shield for this attempt
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;

		// Reset bit table before gameplay starts - FUN_00423880 calls FUN_00423a00(0)
		// This ensures all enemies are visible (not skipped by SKIP chunks)
		clearBit(0);

		// Play gameplay (01P01.SAN with 0x28 flags)
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV01/01P01.SAN", 12);

		// Store death frame for video selection
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			// Level completed!
			debug("Rebel2: Level 1 completed!");
			playLevelEnd(1);
			_levelUnlocked[1] = true;  // Unlock level 2
			return kLevelNextLevel;
		}

		// Player died - play death video with random A/B variant
		debug("Rebel2: Level 1 death at frame %d, lives=%d", _deathFrame, _playerLives - 1);
		playLevelDeathVariant(1, 1, _deathFrame);

		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(1);
			return kLevelGameOver;
		}

		// Play retry prompt and loop
		playLevelRetry(1);
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Wave State Management - FUN_00417b61
// Waits for video completion, accumulates kill state, redistributes kill credits.
// Used by all multi-wave levels (Level 2, 3, 6, etc.) as the core wave loop primitive.
// ---------------------------------------------------------------------------

uint16 InsaneRebel2::processWaveEnd(int16 mask, int16 *budget, int16 threshold, uint16 flags) {
	// FUN_00417b61: Core wave management function
	// Called after each wave video plays. Handles:
	// 1. Waiting for video to finish (with early exit on enemy completion)
	// 2. Copying wave state to accumulated phase state
	// 3. Redistributing kill credits from the budget
	//
	// Returns: kill bits credited this wave, or 0xFFFF on death/quit/completion

	uint16 result = 0;

	// Debug shortcut path: force-end current section when requested via Shift+S.
	// This returns the same sentinel (0xFFFF) used for section completion/death/quit.
	if (_skipSectionRequested) {
		_skipSectionRequested = false;
		_rebelPhaseState = mask;
		_rebelWaveState = mask;
		debug("Rebel2 processWaveEnd: Shift+S skip consumed (mask=0x%x)", (uint16)mask);
		return 0xFFFF;
	}

	// Step 1: Wait for video to finish (lines 21-32)
	// Original loop: while (damage < 0xff && frame < maxFrame-1 && !escPressed)
	// The SmushPlayer::play() call already blocks until video ends, so this step
	// is handled implicitly. The early-exit logic (threshold > 0: if frame > 50
	// AND all required enemy type bits are set, count up and break when > threshold)
	// would need per-frame callbacks to work precisely. For now, the primary effect
	// is covered by the video playing to completion and accumulating state.
	// TODO: Implement per-frame early exit callback for threshold-based wave termination.

	// Step 2: Copy wave state to phase state (line 33)
	// DAT_0047ab9c = DAT_0047ab98
	_rebelPhaseState = _rebelWaveState;
	debug("Rebel2 processWaveEnd: waveState=0x%x -> phaseState=0x%x mask=0x%x budget=%d threshold=%d flags=%d",
		_rebelWaveState, _rebelPhaseState, mask, budget ? *budget : -1, threshold, flags);

	// Step 3: Kill redistribution - add random unkilled types (lines 34-47)
	// Only when (flags & 2) != 0. Level 2 always passes flags=0, so inactive for Level 2.
	if ((flags & 2) != 0) {
		// Collect unkilled enemy type bits that are within the mask
		byte unkilled[8];
		int16 numUnkilled = 0;
		for (byte b = 0; (2 << (b & 0x1f)) < (int)(mask & 0x0e); b++) {
			if ((_rebelPhaseState & (2 << (b & 0x1f))) == 0) {
				unkilled[numUnkilled] = (byte)(2 << (b & 0x1f));
				numUnkilled++;
			}
		}
		if (numUnkilled > 0) {
			// Randomly add one unkilled type to phase state
			int idx = _vm->_rnd.getRandomNumber(numUnkilled - 1);
			_rebelPhaseState |= unkilled[idx];
			if (budget)
				(*budget)++;
		}
	}

	// Step 4: Kill credit transfer (lines 48-73)
	// Collect all SET enemy type bits from phase state
	byte killed[8];
	int16 numKilled = 0;
	for (byte b = 0; (2 << (b & 0x1f)) < (int)(mask & 0x0e); b++) {
		if ((_rebelPhaseState & (2 << (b & 0x1f))) != 0) {
			killed[numKilled] = (byte)(2 << (b & 0x1f));
			numKilled++;
		}
	}

	// Max credits: 8 normally, 2 if flag bit 0 set
	int16 maxCredits = ((flags & 1) == 0) ? 8 : 2;

	// Transfer kills from phase state to result, limited by budget
	int16 creditCount = 0;
	while (creditCount < maxCredits && numKilled > 0 && budget && *budget > 0) {
		int idx = _vm->_rnd.getRandomNumber(numKilled - 1);
		_rebelPhaseState -= killed[idx];   // Remove from accumulated state
		result |= killed[idx];              // Credit to return value
		(*budget)--;

		// Remove from array (shift remaining elements)
		for (int i = idx; i + 1 < numKilled; i++) {
			killed[i] = killed[i + 1];
		}
		numKilled--;
		creditCount++;
	}

	debug("Rebel2 processWaveEnd: result=0x%x phaseState=0x%x (after redistribution) budget=%d",
		result, _rebelPhaseState, budget ? *budget : -1);

	// Step 5: Return value (lines 74-78)
	// Return 0xFFFF if: dead, phase complete, or quit
	if (_playerDamage >= 255 || (int16)_rebelPhaseState >= mask || _vm->shouldQuit()) {
		return 0xFFFF;
	}
	return result;
}

// ---------------------------------------------------------------------------
// Level 2 Handler - FUN_00418063
// Multiple parts with P1/P2/P3 subdirectories
// Random animation variants for each part
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel2() {
	// FUN_00418063: Level 2 "Corellia Star" - Third-person on-foot shooting
	// Three phases, each with looping enemy waves until all enemy types killed.
	// Phase completion: (_rebelPhaseState & mask) == mask
	// Phase 1 mask: 0x06 (enemy types 1,2)
	// Phase 2 mask: 0x0e (enemy types 1,2,3)
	// Phase 3 mask: 0x0e (enemy types 1,2,3)
	//
	// Kill credit budget (from level data table DAT_0047e0e8):
	// Each phase gets a budget = tableBase + random(3). processWaveEnd() uses
	// this budget to randomly redistribute kill credits, creating non-deterministic
	// wave progression. Using calibrated defaults until exact table values extracted.
	static const int16 kLevel2BudgetBase[3] = { 3, 3, 3 };  // Phase 1, 2, 3

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	int bonusCount = 0;     // local_1c: tracks bonus events (DAT_0047ab9c & 0x10)
	int totalKills = 0;     // local_c: accumulated kill count across phases
	int totalMisses = 0;    // Accumulated misses (sVar1 + sVar2 from hit counters)
	int prevWaveState = 0;  // variantIdx: previous wave's state for Phase 3 randomization

	// Play cutscene (02CUT.SAN)
	playCinematic("LEV02/02CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	// Play level beginning cinematic (02BEG.SAN)
	// Original: FUN_004171c5("LEV02/02BEG.SAN", 0x20, 0xab, 0xa0, 10, 2, 0x46)
	// Includes text overlay from GAME.TRS — deferred until text rendering is ready.
	playLevelBegin(2);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_00401000 + FUN_00407d10 + FUN_0040c040: Reset game state (before retry loop)
	clearBit(0);

	// Main gameplay retry loop (restarts from beginning on death)
	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;
		bonusCount = 0;
		totalKills = 0;
		totalMisses = 0;

		// Reset Handler 25 cover state — player starts uncovered at level start
		// DAT_00457904 and DAT_0045790a are zero-initialized globals in the original
		_rebelAutopilot = 0;
		_rebelDamageLevel = 0;
		_rebelControlMode = 0;

		// FUN_0041c7d0: Reset per-attempt state
		_enemies.clear();
		for (int i = 0; i < 512; i++) {
			_rebelLinks[i][0] = 0;
			_rebelLinks[i][1] = 0;
			_rebelLinks[i][2] = 0;
		}

		// ----- PHASE 1: P1/02P01_X.SAN -----
		// FUN_0041c7d0: Reset per-phase counters
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;

		// Initialize kill budget from level data table + random(3)
		// Original: sVar4 = levelData[phase1Offset]; local_14[0] = sVar4 + random(3)
		int16 budget = kLevel2BudgetBase[0] + _vm->_rnd.getRandomNumber(2);

		// Play A.SAN (background loader) — flags 0x28 (preserve buffer, gameplay mode)
		debug("Rebel2: Level 2 Phase 1 - playing 02P01_A.SAN (background) budget=%d", budget);
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV02/P1/02P01_A.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		// processWaveEnd after A.SAN (threshold=0, no early exit for background loader)
		processWaveEnd(0x36, &budget, 0, 0);

		// Phase 1 wave loop: random B/C/D until all type 1,2 enemies killed
		// Original: while (uVar3 >= 0 && (DAT_0047ab9c & 6) != 6)
		while (_playerDamage < 255 && (_rebelPhaseState & 0x06) != 0x06) {
			if (_vm->shouldQuit())
				return kLevelQuit;

			// Random variant B, C, or D
			int variant = _vm->_rnd.getRandomNumber(2);  // 0-2
			const char *variants[] = {
				"LEV02/P1/02P01_B.SAN",
				"LEV02/P1/02P01_C.SAN",
				"LEV02/P1/02P01_D.SAN"
			};
			debug("Rebel2: Phase 1 wave - playing %s (state=0x%x budget=%d)", variants[variant], _rebelPhaseState, budget);
			// Wave videos use flags 0x428 (original: FUN_0041f4d0 param_2=0x428)
			splayer->setCurVideoFlags(0x428);
			splayer->play(variants[variant], 12);
			_deathFrame = splayer->_frame;

			// processWaveEnd with threshold=0x14 (20) — enables early exit when enemies killed
			processWaveEnd(0x36, &budget, 0x14, 0);
			debug("Rebel2: Phase 1 wave done - state=0x%x (need 0x06) budget=%d", _rebelPhaseState, budget);
		}

		// Check for bonus (bit 4 = 0x10)
		if ((_rebelPhaseState & 0x10) != 0)
			bonusCount++;

		if (_playerDamage >= 255)
			goto level2_death;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// Post segment 1 (02PST1.SAN)
		// Original: FUN_00417168("02PST1.SAN", 0x20) → flags = 0x20 | 0x08 = 0x28
		// FUN_00417168 adds | 8 to preserve the screen buffer between gameplay and transition
		// Reset handler to 0 so procPostRendering skips HUD/sprite drawing during cinematic
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV02/02PST1.SAN", 12);
		if (_vm->shouldQuit())
			return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		// ----- PHASE 2: P2/02P02_X.SAN -----
		_currentPhase = 2;
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;
		_enemies.clear();

		// Initialize Phase 2 budget
		budget = kLevel2BudgetBase[1] + _vm->_rnd.getRandomNumber(2);

		// Restore handler for gameplay — will be confirmed by IACT opcode 6
		_rebelHandler = 8;

		// Play A.SAN (background loader)
		debug("Rebel2: Level 2 Phase 2 - playing 02P02_A.SAN (background) budget=%d", budget);
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV02/P2/02P02_A.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		// Phase 2 wave loop: processWaveEnd at TOP of loop (matches assembly structure)
		// Original: local_10 = FUN_00417b61(0x3e, local_14, 0, 0); then switch(local_10)
		while (true) {
			uint16 waveSelect = processWaveEnd(0x3e, &budget, 0, 0);
			if (waveSelect == 0xFFFF || (_rebelPhaseState & 0x0e) == 0x0e)
				break;
			if (_vm->shouldQuit())
				return kLevelQuit;

			// If no specific pattern: randomize high bits (original lines 71-74)
			// When (local_10 & 0xc) == 0: add random 0x10/0x11/0x12
			if ((waveSelect & 0x0c) == 0) {
				waveSelect = _vm->_rnd.getRandomNumber(2) + 0x10;
			}

			// Variant selection matching original switch (FUN_418063 lines 75-96)
			const char *filename;
			switch (waveSelect) {
			case 4: case 6:
				filename = "LEV02/P2/02P02_B.SAN"; break;
			case 8: case 10:
				filename = "LEV02/P2/02P02_C.SAN"; break;
			case 0x0c: case 0x0e:
				filename = "LEV02/P2/02P02_A.SAN"; break;
			case 0x11:
				filename = "LEV02/P2/02P02_E.SAN"; break;
			case 0x12:
				filename = "LEV02/P2/02P02_F.SAN"; break;
			default:
				filename = "LEV02/P2/02P02_D.SAN"; break;
			}

			debug("Rebel2: Phase 2 wave - playing %s (state=0x%x sel=0x%x budget=%d)", filename, _rebelPhaseState, waveSelect, budget);
			splayer->setCurVideoFlags(0x428);
			splayer->play(filename, 12);
			_deathFrame = splayer->_frame;
		}

		if ((_rebelPhaseState & 0x10) != 0)
			bonusCount++;

		if (_playerDamage >= 255)
			goto level2_death;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// Post segment 2 (02PST2.SAN)
		// Original: FUN_00417168("02PST2.SAN", 0x20) → flags = 0x20 | 0x08 = 0x28
		// Reset handler to 0 so procPostRendering skips HUD/sprite drawing during cinematic
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV02/02PST2.SAN", 12);
		if (_vm->shouldQuit())
			return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		// ----- PHASE 3: P3/02P03_X.SAN -----
		_currentPhase = 3;
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;
		_enemies.clear();
		prevWaveState = 0;

		// Initialize Phase 3 budget
		budget = kLevel2BudgetBase[2] + _vm->_rnd.getRandomNumber(2);

		// Restore handler for gameplay — will be confirmed by IACT opcode 6
		_rebelHandler = 8;

		// Play A.SAN (background loader)
		debug("Rebel2: Level 2 Phase 3 - playing 02P03_A.SAN (background) budget=%d", budget);
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV02/P3/02P03_A.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		// Phase 3: processWaveEnd at BOTTOM (like Phase 1), waveSelect carried across iterations
		// Original: local_10 = FUN_00417b61(0x3e, local_14, 0, 0); while (loop) { ...; local_10 = FUN_00417b61(0x3e, local_14, 0x14, 0); }
		{
			uint16 waveSelect = processWaveEnd(0x3e, &budget, 0, 0);

			while (waveSelect != 0xFFFF && (_rebelPhaseState & 0x0e) != 0x0e) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				// Phase 3 randomization (original lines 113-115):
				// If previous wave state bit 0 was clear AND random(8)==0, set bit 0
				if (((prevWaveState & 1) == 0) && (_vm->_rnd.getRandomNumber(7) == 0)) {
					waveSelect |= 1;
				}
				prevWaveState = waveSelect;

				// Variant selection matching original switch (FUN_418063 lines 117-144)
				const char *filename;
				switch (waveSelect) {
				case 0:
					filename = "LEV02/P3/02P03_H.SAN"; break;
				case 2:
					filename = "LEV02/P3/02P03_G.SAN"; break;
				case 4:
					filename = "LEV02/P3/02P03_F.SAN"; break;
				case 6:
					filename = "LEV02/P3/02P03_E.SAN"; break;
				case 8:
					filename = "LEV02/P3/02P03_D.SAN"; break;
				case 10:
					filename = "LEV02/P3/02P03_C.SAN"; break;
				case 0x0c:
					filename = "LEV02/P3/02P03_B.SAN"; break;
				case 0x0e:
					filename = "LEV02/P3/02P03_A.SAN"; break;
				default:
					filename = "LEV02/P3/02P03_I.SAN"; break;
				}

				debug("Rebel2: Phase 3 wave - playing %s (state=0x%x sel=0x%x budget=%d)", filename, _rebelPhaseState, waveSelect, budget);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				// processWaveEnd at BOTTOM with threshold=0x14
				waveSelect = processWaveEnd(0x3e, &budget, 0x14, 0);
				debug("Rebel2: Phase 3 wave done - state=0x%x (need 0x0e) budget=%d", _rebelPhaseState, budget);
			}
		}

		if ((_rebelPhaseState & 0x10) != 0)
			bonusCount++;
		totalKills += _rebelKillCounter;

		if (_playerDamage >= 255)
			goto level2_death;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// Level completed! Calculate accuracy score.
		// Original: FUN_00417327 with score thresholds and medal ranks
		// Score presentation deferred until GAME.TRS text rendering is implemented.
		{
			totalMisses += _rebelHitCounter;
			int accuracy = 0;
			int totalShots = totalKills + totalMisses;
			if (totalKills > 0 && totalShots > 0) {
				accuracy = (totalKills * 100) / totalShots;
			}
			debug("Rebel2: Level 2 completed! kills=%d misses=%d accuracy=%d%% bonus=%d",
				totalKills, totalMisses, accuracy, bonusCount);
		}

		playLevelEnd(2);
		_levelUnlocked[2] = true;  // Unlock level 3
		return kLevelNextLevel;

	level2_death:
		// Player died — play death sequence and retry or game over
		// Original: FUN_00417168("LEV02/02DIE.SAN", 0x20)
		debug("Rebel2: Level 2 Phase %d death", _currentPhase);
		playCinematic("LEV02/02DIE.SAN");
		if (_vm->shouldQuit())
			return kLevelQuit;

		// Original: if (DAT_0047ab5c != 0) DAT_0047a7ee++ (bonus life award)
		// DAT_0047ab5c is set when player earns a bonus life (e.g., score threshold).
		// Currently not tracked — will be wired when bonus life system is implemented.
		_playerLives--;
		if (_playerLives <= 0) {
			// Original: FUN_00417ab2("LEV02/02OVER.SAN", 0x20, 2)
			playLevelGameOver(2);
			return kLevelGameOver;
		}
		playCinematic("LEV02/02RETRY.SAN");
		_playerDamage = 0;
		if (_vm->shouldQuit())
			return kLevelQuit;
		continue;  // Restart from beginning
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 3 Handler - FUN_0041885F
// Two phases with per-phase retry handling
// Phase 1: 03PLAY1.SAN, Phase 2: 03PLAY2.SAN
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel3() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	int phase1Score = 0;  // Score preserved across phase 2 retries

	// Play level beginning cinematic (03BEG.SAN)
	playLevelBegin(3);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// ----- PHASE 1 retry loop -----
	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;

		// Reset bit table before gameplay starts - FUN_00423880 calls FUN_00423a00(0)
		clearBit(0);

		// Play phase 1 gameplay (03PLAY1.SAN)
		debug("Rebel2: Level 3 Phase 1");
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV03/03PLAY1.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			// Phase 1 completed - save score and proceed to phase 2
			phase1Score = _playerScore;
			break;
		}

		// Died in phase 1 - frame-based death video
		debug("Rebel2: Level 3 Phase 1 death at frame %d", _deathFrame);
		playLevelDeathVariant(3, 1, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(3);
			return kLevelGameOver;
		}

		// Phase 1 retry (03RETRY.SAN)
		playLevelRetryVariant(3, 1);
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	if (_vm->shouldQuit())
		return kLevelQuit;

	// Post segment 1 (03POST1.SAN)
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	// Reset handler to 0 so procPostRendering skips HUD/sprite drawing during cinematic
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;
	splayer->setCurVideoFlags(0x28);
	splayer->play("LEV03/03POST1.SAN", 12);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// ----- PHASE 2 retry loop (preserves phase 1 score) -----
	_currentPhase = 2;

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;

		// Reset bit table before gameplay starts
		clearBit(0);

		// Play phase 2 gameplay (03PLAY2.SAN)
		debug("Rebel2: Level 3 Phase 2");
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV03/03PLAY2.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			// Level completed!
			debug("Rebel2: Level 3 completed!");
			playLevelEnd(3);
			_levelUnlocked[3] = true;  // Unlock level 4
			return kLevelNextLevel;
		}

		// Died in phase 2 - frame-based death video
		debug("Rebel2: Level 3 Phase 2 death at frame %d", _deathFrame);
		playLevelDeathVariant(3, 2, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			// Use phase 2 specific game over (03OVER.SAN, same file but at different point)
			playLevelGameOver(3);
			return kLevelGameOver;
		}

		// Phase 2 retry (03RETRYB.SAN)
		playLevelRetryVariant(3, 2);
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 4 Handler
// Cutscene + single gameplay phase
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel4() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play cutscene (04CUT.SAN)
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play("LEV04/04CUT.SAN", 12);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// Play level beginning cinematic (04BEG.SAN)
	playLevelBegin(4);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// Main gameplay retry loop
	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;

		// Reset bit table before gameplay starts
		clearBit(0);

		// Play gameplay (04PLAY.SAN)
		debug("Rebel2: Level 4 gameplay");
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV04/04PLAY.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			// Level completed!
			debug("Rebel2: Level 4 completed!");
			playLevelEnd(4);
			_levelUnlocked[4] = true;  // Unlock level 5
			return kLevelNextLevel;
		}

		// Died - play death video (04DIE.SAN, no variants)
		debug("Rebel2: Level 4 death");
		playLevelDeathVariant(4, 1, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(4);
			return kLevelGameOver;
		}

		playLevelRetry(4);
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 5 Handler - FUN_00418EC6
// Single gameplay phase (05PLAY.SAN)
// Random A/B death video like Level 1
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel5() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play level beginning cinematic (05BEG.SAN)
	playLevelBegin(5);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// Main gameplay retry loop
	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;

		// Reset bit table before gameplay starts
		clearBit(0);

		// Play gameplay (05PLAY.SAN)
		debug("Rebel2: Level 5 gameplay");
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV05/05PLAY.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			// Level completed!
			debug("Rebel2: Level 5 completed!");
			playLevelEnd(5);
			_levelUnlocked[5] = true;  // Unlock level 6
			return kLevelNextLevel;
		}

		// Died - play death video with random A/B variant
		debug("Rebel2: Level 5 death at frame %d", _deathFrame);
		playLevelDeathVariant(5, 1, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(5);
			return kLevelGameOver;
		}

		playLevelRetry(5);
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 6 Handler - FUN_00419317
// Two phases with per-phase retry (like Level 3)
// Phase 1: 06PLAY1.SAN, Phase 2: 06PLAY2.SAN
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel6() {
	// FUN_004190d6 — Mos Eisley: two-phase on-foot (Handler 8)
	// Phase 1 (levelId=5): 06PLAY1.SAN, mid-switch to 06PLAY1B.SAN at frame 0x2a8
	// Phase 2 (levelId=6): 06PLAY2.SAN, play until near-end
	// Original structure: outer do-while for phase 1 retries, inner while(true) for
	// phase 2 retries + death handling. Phase 1 death breaks inner → RETRY at outer bottom.
	// Phase 2 death → RETRYB → re-enters phase 2 within inner loop.
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	int totalScore = 0;

	// Play level beginning cinematic (06BEG.SAN)
	// Original: FUN_004171c5(s_LEV06_06BEG_SAN, 0x20, 0xaf, 0xa0, 10, 5, 0x4b)
	playLevelBegin(6);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_00401000 + FUN_0041c7d0 + FUN_0040c040 — handler init done by IACT opcode 6

	// Outer retry loop — restarts phase 1 on phase 1 death
	while (!_vm->shouldQuit()) {
		// FUN_00407d10 — reset shot/hit counters
		clearBit(0);

		// DAT_0047ab9c = 0xffffffff — init phase state
		_rebelPhaseState = 0xffffffff;

		// ----- PHASE 1 -----
		_rebelLevelType = 5;  // DAT_0047a7f8 = 5
		_currentPhase = 1;

		debug("Rebel2: Level 6 Phase 1");
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV06/06PLAY1.SAN", 12);
		// TODO: Mid-level switch at frame 0x2a8 to 06PLAY1B.SAN (flags 0x468)
		// + score checkpoint (FUN_00407f55) — needs per-frame callback
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield <= 0) {
			// Died in phase 1
			debug("Rebel2: Level 6 Phase 1 death at frame %d", _deathFrame);
			playLevelDeathVariant(6, 1, _deathFrame);
			if (_vm->shouldQuit())
				return kLevelQuit;

			_playerLives--;
			if (_playerLives <= 0) {
				playLevelGameOver(6);
				return kLevelGameOver;
			}

			// Phase 1 retry (06RETRY.SAN) → restart outer loop
			playLevelRetryVariant(6, 1);
			if (_vm->shouldQuit())
				return kLevelQuit;
			continue;
		}

		// Phase 1 survived — save score, play POST1
		totalScore = _playerScore;  // variantIdx = DAT_0047ab84

		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV06/06POST1.SAN", 12);
		if (_vm->shouldQuit())
			return kLevelQuit;

		// ----- PHASE 2 retry loop (inner while(true) in original) -----
		while (!_vm->shouldQuit()) {
			_rebelLevelType = 6;  // DAT_0047a7f8 = 6
			_currentPhase = 2;
			clearBit(0);  // FUN_00407d10

			debug("Rebel2: Level 6 Phase 2");
			splayer->setCurVideoFlags(0x28);
			splayer->play("LEV06/06PLAY2.SAN", 12);
			_deathFrame = splayer->_frame;

			if (_vm->shouldQuit())
				return kLevelQuit;

			// Accumulate score: variantIdx = DAT_0047ab84 + variantIdx
			totalScore += _playerScore;

			if (_playerShield > 0) {
				// Level completed!
				debug("Rebel2: Level 6 completed!");
				playLevelEnd(6);
				_levelUnlocked[6] = true;
				return kLevelNextLevel;
			}

			// Died in phase 2
			debug("Rebel2: Level 6 Phase 2 death at frame %d", _deathFrame);
			playLevelDeathVariant(6, 2, _deathFrame);
			if (_vm->shouldQuit())
				return kLevelQuit;

			_playerLives--;
			if (_playerLives <= 0) {
				playLevelGameOver(6);
				return kLevelGameOver;
			}

			// Phase 2 retry (06RETRYB.SAN) → re-enter phase 2
			playLevelRetryVariant(6, 2);
			if (_vm->shouldQuit())
				return kLevelQuit;
		}

		break;  // Should only reach here on shouldQuit
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 7 Handler - FUN_0041974C
// "TIE Training" - Canyon flight with fork at frame 1592
// Single gameplay phase (07PLAY.SAN), optional second segment (07PLAYB.SAN)
// Death: DAT_0047ab8c-based (fork reached → DIE_B, not reached → DIE_A)
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel7() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	bool reachedFork = false;  // DAT_0047ab8c equivalent — tracks if 07PLAYB was played

	// Play cutscene (07CUT.SAN)
	playCinematic("LEV07/07CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	// Play level beginning cinematic (07BEG.SAN)
	playLevelBegin(7);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_00401000 + FUN_0041c7d0 + FUN_00407d10
	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;
		reachedFork = false;

		clearBit(0);

		// Play gameplay (07PLAY.SAN)
		// Original: FUN_0041f4d0("07PLAY.SAN", 0x28, -1, -1, 0)
		// At frame 0x638 (1592), if DAT_0047ab8c != 0: play 07PLAYB.SAN (0x468)
		// TODO: Mid-level fork at frame 1592 requires per-frame callback.
		// For now, play the main video. The fork video (07PLAYB) would be triggered
		// by IACT callbacks setting a state flag during gameplay.
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV07/07PLAY.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			debug("Rebel2: Level 7 completed!");
			playLevelEnd(7);
			_levelUnlocked[7] = true;
			return kLevelNextLevel;
		}

		// Death video: DIE_B if fork reached, DIE_A if not
		// Original: s_LEV07_07DIE_B + ((DAT_0047ab8c != 0) - 1 & 0x14)
		debug("Rebel2: Level 7 death at frame %d, fork=%d", _deathFrame, reachedFork);
		if (reachedFork) {
			playCinematic("LEV07/07DIE_B.SAN");
		} else {
			playCinematic("LEV07/07DIE_A.SAN");
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(7);
			return kLevelGameOver;
		}

		playCinematic("LEV07/07RETRY.SAN");
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 8 Handler - FUN_00419976
// "Flight to Imdaar" - Y-Wing space battle (single phase)
// No cutscene (starts with BEG). flags=0x08 for gameplay.
// Death: random A or B
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel8() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// No cutscene — starts directly with BEG
	// Original: FUN_004171c5("08BEG.SAN", 0x20, 0xb1, 0xa0, 10, 5, 0x4b)
	playLevelBegin(8);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_00401000 + FUN_0041c7d0 + FUN_0040c040
	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;

		clearBit(0);

		// Play gameplay (08PLAY.SAN)
		// Original: FUN_0041f4d0("08PLAY.SAN", 8, -1, -1, 0) — note flags=0x08
		splayer->setCurVideoFlags(0x08);
		splayer->play("LEV08/08PLAY.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = 0;
			if (_rebelKillCounter > 0) {
				accuracy = (_rebelKillCounter * 100) / (_rebelHitCounter + _rebelKillCounter);
			}
			debug("Rebel2: Level 8 completed! accuracy=%d%%", accuracy);
			playLevelEnd(8);
			_levelUnlocked[8] = true;
			return kLevelNextLevel;
		}

		// Death: random A or B
		// Original: random(2) → A or B via string pointer arithmetic
		debug("Rebel2: Level 8 death at frame %d", _deathFrame);
		playLevelDeathVariant(8, 1, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(8);
			return kLevelGameOver;
		}

		playCinematic("LEV08/08RETRY.SAN");
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 9 Handler - FUN_00419B86
// "The Mine Field" - Navigate through force fields (single phase)
// No cutscene. Initial phaseState = 0xfffffffe (all bits set except bit 0).
// Mid-events at frames 0x19f (415) and 0x352 (850): FUN_00407f55 (score checkpoint)
// Death: DAT_0047ab94-based (0→A, 1→C, else→B)
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel9() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// No cutscene — starts directly with BEG
	// Original: FUN_004171c5("09BEG.SAN", 0x20, 0xb2, 0xa0, 10, 200, 0x10e)
	playLevelBegin(9);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_00401000 + FUN_0041c7d0 + FUN_0040c040
	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;

		clearBit(0);

		// Original: DAT_0047ab9c = 0xfffffffe (initial phase state — all bits except 0)
		_rebelPhaseState = 0xfffffffe;

		// Play gameplay (09PLAY.SAN)
		// Original: FUN_0041f4d0("09PLAY.SAN", 0x28, -1, -1, 0)
		// Mid-events at frames 415 and 850: FUN_00407f55 (score save)
		// These are handled implicitly — the IACT callbacks manage scoring.
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV09/09PLAY.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = 0;
			if (_rebelKillCounter > 0) {
				accuracy = (_rebelKillCounter * 100) / (_rebelHitCounter + _rebelKillCounter);
			}
			debug("Rebel2: Level 9 completed! accuracy=%d%%", accuracy);
			playLevelEnd(9);
			_levelUnlocked[9] = true;
			return kLevelNextLevel;
		}

		// Death: based on DAT_0047ab94 (death cause tracking)
		// Original: 0→DIE_A, 1→DIE_C, else→DIE_B
		debug("Rebel2: Level 9 death at frame %d", _deathFrame);
		playLevelDeathVariant(9, 1, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(9);
			return kLevelGameOver;
		}

		playCinematic("LEV09/09RETRY.SAN");
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 10 Handler - FUN_00419E0A
// "Speeder Bikes" - Forest speeder chase (single phase)
// Has cutscene. Single death video (10DIE.SAN, no variants).
// Original plays DIE then RETRY in sequence (no separate check).
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel10() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play cutscene (10CUT.SAN)
	playCinematic("LEV10/10CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	// Play level beginning cinematic (10BEG.SAN)
	// Original: FUN_004171c5("10BEG.SAN", 0x20, 0xb3, 0xa0, 10, 2, 0x46)
	playLevelBegin(10);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_00401000 + FUN_0041c7d0 + FUN_00407d10
	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;

		clearBit(0);

		// Play gameplay (10PLAY.SAN)
		// Original: FUN_0041f4d0("10PLAY.SAN", 0x28, -1, -1, 0)
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV10/10PLAY.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = 0;
			if (_rebelKillCounter > 0) {
				accuracy = (_rebelKillCounter * 100) / (_rebelHitCounter + _rebelKillCounter);
			}
			debug("Rebel2: Level 10 completed! accuracy=%d%%", accuracy);
			playLevelEnd(10);
			_levelUnlocked[10] = true;
			return kLevelNextLevel;
		}

		// Death + Retry: original plays both in sequence
		// Original: lives--, if 0 break to game over, else DIE+RETRY
		debug("Rebel2: Level 10 death at frame %d", _deathFrame);
		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(10);
			return kLevelGameOver;
		}

		playCinematic("LEV10/10DIE.SAN");
		if (_vm->shouldQuit())
			return kLevelQuit;
		playCinematic("LEV10/10RETRY.SAN");
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 11 Handler - FUN_0041A00C
// "Inside the Terror" - Three phases + bridge puzzle (Handler 8, on-foot)
//
// Phase 1: P1/11P01_X (A,B,C,D) - behind barrels, mask 0x0e
// Phase 2: P2/11P02_X (A,B,C,D) - walls on right, mask 0x0e, flags=3
// Phase 3 first half: P3/11P03_X (A-F) - bridge puzzle, mask 0x7e
//   Exit when (phaseState & 0x70) == 0x70
// POST3/POST3B/POST3C bridge cinematics
// Phase 3 second half: P3/11P03_X (G-L) - after bridge, mask 0x0e
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel11() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	int totalKills = 0;
	int totalMisses = 0;
	int prevPhaseState = 0;

	// Kill credit budget bases per phase (from level data table DAT_0047e0e8)
	static const int16 kLevel11BudgetBase[4] = { 3, 3, 3, 3 };

	// Play cutscene (11CUT.SAN)
	playCinematic("LEV11/11CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	// Play level beginning cinematic (11BEG.SAN)
	playLevelBegin(11);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_00401000 + FUN_00407d10 + FUN_0040c040: Reset game state
	clearBit(0);

	// Main gameplay retry loop (restarts from Phase 1 on death)
	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;
		totalKills = 0;
		totalMisses = 0;
		prevPhaseState = 0;

		// Reset Handler 8 cover state
		_rebelAutopilot = 0;
		_rebelDamageLevel = 0;
		_rebelControlMode = 0;

		// FUN_0041c7d0: Reset per-attempt state
		_enemies.clear();
		for (int i = 0; i < 512; i++) {
			_rebelLinks[i][0] = 0;
			_rebelLinks[i][1] = 0;
			_rebelLinks[i][2] = 0;
		}

		// ----- PHASE 1: P1/11P01_X.SAN -----
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;

		int16 budget = kLevel11BudgetBase[0] + _vm->_rnd.getRandomNumber(2);

		// Play A.SAN (background loader)
		debug("Rebel2: Level 11 Phase 1 - playing 11P01_A.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV11/P1/11P01_A.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		{
			uint16 waveSelect = processWaveEnd(0x0e, &budget, 0, 0);

			// Phase 1 wave loop: random(2) | (waveSelect & 8) → variants
			// 0→D, 1→C, 8→B, 9→A
			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				// Bonus sound check
				if ((_rebelPhaseState & 0x10) != 0 && (prevPhaseState & 0x10) == 0) {
					// FUN_00411931 bonus sound — not yet implemented
				}
				prevPhaseState = _rebelPhaseState;

				int sel = _vm->_rnd.getRandomNumber(1) | (waveSelect & 8);
				const char *filename;
				switch (sel) {
				case 1:  filename = "LEV11/P1/11P01_C.SAN"; break;
				case 8:  filename = "LEV11/P1/11P01_B.SAN"; break;
				case 9:  filename = "LEV11/P1/11P01_A.SAN"; break;
				default: filename = "LEV11/P1/11P01_D.SAN"; break;  // sel == 0
				}

				debug("Rebel2: Level 11 Phase 1 wave - %s (state=0x%x sel=%d)", filename, _rebelPhaseState, sel);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				waveSelect = processWaveEnd(0x0e, &budget, 0x14, 0);
			}
		}

		if (_playerDamage >= 255)
			goto level11_death_phase1;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// Post segment 1 (11POST1.SAN)
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV11/11POST1.SAN", 12);
		if (_vm->shouldQuit())
			return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		// ----- PHASE 2: P2/11P02_X.SAN -----
		_currentPhase = 2;
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;
		_enemies.clear();

		budget = kLevel11BudgetBase[1] + _vm->_rnd.getRandomNumber(2);
		_rebelHandler = 8;

		// Play A.SAN (background loader)
		debug("Rebel2: Level 11 Phase 2 - playing 11P02_A.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV11/P2/11P02_A.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		{
			// Phase 2: flags=3 (maxCredits=2, redistribution ON)
			uint16 waveSelect = processWaveEnd(0x0e, &budget, 0, 3);

			// Random(4) for variant selection: A, B, C, D
			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				int variant = _vm->_rnd.getRandomNumber(3);
				const char *filename;
				switch (variant) {
				case 0:  filename = "LEV11/P2/11P02_A.SAN"; break;
				case 1:  filename = "LEV11/P2/11P02_B.SAN"; break;
				case 2:  filename = "LEV11/P2/11P02_C.SAN"; break;
				default: filename = "LEV11/P2/11P02_D.SAN"; break;
				}

				debug("Rebel2: Level 11 Phase 2 wave - %s (state=0x%x)", filename, _rebelPhaseState);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				waveSelect = processWaveEnd(0x0e, &budget, 0x14, 3);
			}
		}

		if (_playerDamage >= 255)
			goto level11_death_phase2;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// Post segment 2 (11POST2.SAN)
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV11/11POST2.SAN", 12);
		if (_vm->shouldQuit())
			return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		// ----- PHASE 3 FIRST HALF: P3/11P03_X (A-F) -----
		// Bridge puzzle — exit when (phaseState & 0x70) == 0x70
		_currentPhase = 3;
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;
		_enemies.clear();
		prevPhaseState = 0;

		budget = kLevel11BudgetBase[2] + _vm->_rnd.getRandomNumber(2);
		_rebelHandler = 8;

		debug("Rebel2: Level 11 Phase 3 first half - playing 11P03_A.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV11/P3/11P03_A.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		{
			uint16 waveSelect = processWaveEnd(0x7e, &budget, 0, 0);
			int variantIdx = 0;  // Tracks variant for randomization threshold

			// Loop until (phaseState & 0x70) == 0x70 (bridge targets destroyed)
			while (waveSelect != 0xFFFF && (_rebelPhaseState & 0x70) != 0x70) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				// Bonus sound: (phaseState & 0xe) == 0xe and previous wasn't
				if ((_rebelPhaseState & 0x0e) == 0x0e && (prevPhaseState & 0x0e) != 0x0e) {
					// FUN_00411931 bonus sound — not yet implemented
				}
				prevPhaseState = _rebelPhaseState;

				// Randomization: wider range for first few waves
				if (variantIdx < 3) {
					variantIdx = _vm->_rnd.getRandomNumber(7);  // 0-7
				} else {
					variantIdx = _vm->_rnd.getRandomNumber(2);  // 0-2
				}

				const char *filename;
				switch (variantIdx) {
				case 0:  filename = "LEV11/P3/11P03_A.SAN"; break;
				case 1:  filename = "LEV11/P3/11P03_B.SAN"; break;
				case 2:  filename = "LEV11/P3/11P03_C.SAN"; break;
				case 3:  filename = "LEV11/P3/11P03_D.SAN"; break;
				case 4:  filename = "LEV11/P3/11P03_E.SAN"; break;
				case 5:  filename = "LEV11/P3/11P03_F.SAN"; break;
				case 6:  filename = "LEV11/P3/11P03_F.SAN"; break;  // duplicate F
				default: filename = "LEV11/P3/11P03_E.SAN"; break;  // duplicate E
				}

				debug("Rebel2: Level 11 Phase 3a wave - %s (state=0x%x variantIdx=%d)", filename, _rebelPhaseState, variantIdx);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				// Threshold only for higher variants (original: (2 < variantIdx) - 1 & 0x14)
				int16 threshold = (variantIdx > 2) ? 0x14 : 0;
				waveSelect = processWaveEnd(0x7e, &budget, threshold, 0);
			}
		}

		if (_playerDamage >= 255)
			goto level11_death_phase3;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// ----- PHASE 3 BRIDGE CINEMATICS -----
		{
			bool allBasicKilled = (_rebelPhaseState & 0x0e) >= 0x0e;
			if (!allBasicKilled) {
				// Normal bridge drop cinematic
				playCinematic("LEV11/11POST3.SAN");
				// Bonus checks (FUN_0042aa70) — deferred, play standard path
				// Original checks 0x77 and 0x62 for special POST3C cinematic
			} else {
				// All enemy types killed — bridge dropped successfully
				playCinematic("LEV11/11POST3B.SAN");
			}
		}

		if (_vm->shouldQuit())
			return kLevelQuit;

		// ----- PHASE 3 SECOND HALF: P3/11P03_X (G-L) -----
		// Reset shots/explosions (FUN_0041ca6a equivalent)
		for (int i = 0; i < 5; i++) {
			_explosions[i].active = false;
		}
		_enemies.clear();

		// Preserve only bits 1-3 of phase state (original: DAT_0047ab9c &= 0xe)
		_rebelPhaseState &= 0x0e;
		_rebelWaveState &= 0x0e;

		_rebelHandler = 8;

		budget = kLevel11BudgetBase[3] + _vm->_rnd.getRandomNumber(2);

		// Play G.SAN (background loader for second half)
		debug("Rebel2: Level 11 Phase 3 second half - playing 11P03_G.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV11/P3/11P03_G.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		// Only enter wave loop if not all basic types killed already
		if ((_rebelPhaseState & 0x0e) < 0x0e) {
			int variantIdx = 0;
			uint16 waveSelect = processWaveEnd(0x0e, &budget, 0, 0);

			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				// Wider randomization for first few waves
				if (variantIdx < 4) {
					variantIdx = _vm->_rnd.getRandomNumber(8);  // 0-8
				} else {
					variantIdx = _vm->_rnd.getRandomNumber(2);  // 0-2
				}

				const char *filename;
				switch (variantIdx) {
				case 0:  filename = "LEV11/P3/11P03_G.SAN"; break;
				case 1:  filename = "LEV11/P3/11P03_H.SAN"; break;
				case 2:  filename = "LEV11/P3/11P03_I.SAN"; break;
				case 3:  filename = "LEV11/P3/11P03_G.SAN"; break;  // G again
				case 4:  filename = "LEV11/P3/11P03_H.SAN"; break;  // H again
				case 5:  filename = "LEV11/P3/11P03_I.SAN"; break;  // I again
				case 6:  filename = "LEV11/P3/11P03_J.SAN"; break;
				case 7:  filename = "LEV11/P3/11P03_K.SAN"; break;
				default: filename = "LEV11/P3/11P03_L.SAN"; break;
				}

				debug("Rebel2: Level 11 Phase 3b wave - %s (state=0x%x variantIdx=%d)", filename, _rebelPhaseState, variantIdx);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				int16 threshold = (variantIdx > 2) ? 0x14 : 0;
				waveSelect = processWaveEnd(0x0e, &budget, threshold, 0);
			}
		}

		totalKills += _rebelKillCounter;

		if (_playerDamage >= 255)
			goto level11_death_phase3;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// ----- LEVEL COMPLETED -----
		{
			totalMisses += _rebelHitCounter;
			int accuracy = 0;
			int totalShots = totalKills + totalMisses;
			if (totalKills > 0 && totalShots > 0) {
				accuracy = (totalKills * 100) / totalShots;
			}
			debug("Rebel2: Level 11 completed! kills=%d misses=%d accuracy=%d%%",
				totalKills, totalMisses, accuracy);
		}

		playLevelEnd(11);
		_levelUnlocked[11] = true;  // Unlock level 12
		return kLevelNextLevel;

	level11_death_phase1:
		debug("Rebel2: Level 11 Phase 1 death");
		playCinematic("LEV11/11DIE_A.SAN");
		goto level11_retry;

	level11_death_phase2:
		debug("Rebel2: Level 11 Phase 2 death");
		playCinematic("LEV11/11DIE_B.SAN");
		goto level11_retry;

	level11_death_phase3:
		debug("Rebel2: Level 11 Phase 3 death");
		playCinematic("LEV11/11DIE_C.SAN");
		goto level11_retry;

	level11_retry:
		if (_vm->shouldQuit())
			return kLevelQuit;
		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(11);
			return kLevelGameOver;
		}
		playCinematic("LEV11/11RETRY.SAN");
		_playerDamage = 0;
		if (_vm->shouldQuit())
			return kLevelQuit;
		continue;  // Restart from Phase 1
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 12 Handler - FUN_0041AA14
// "Sewers" - Four phases FPS corridor shooting (Handler 25)
//
// Each phase: init video (P05/P06/P07/P08) → first wave → wave loop
// Phase 1: P1/12P01_X (A,B,C,D) mask=6
// Phase 2: P2/12P02_X (A,B,C,D,E,F) mask=6
// Phase 3: P3/12P03_X (A,B,C,D,F) mask=6
// Phase 4: P4/12P04_X (A,B,C,D,E,F) mask=0xe
// Closing: 12P09.SAN
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel12() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Kill credit budget bases per phase
	static const int16 kLevel12BudgetBase[4] = { 3, 4, 4, 4 };

	// Play cutscene (12CUT.SAN)
	playCinematic("LEV12/12CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	// Play level beginning cinematic (12BEG.SAN)
	playLevelBegin(12);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_0041c7d0 + FUN_00407d10 + FUN_0040c040: Reset game state
	clearBit(0);

	// Main gameplay retry loop (restarts from Phase 1 on death)
	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;

		// Reset state
		_rebelAutopilot = 0;
		_rebelDamageLevel = 0;
		_rebelControlMode = 0;

		_enemies.clear();
		for (int i = 0; i < 512; i++) {
			_rebelLinks[i][0] = 0;
			_rebelLinks[i][1] = 0;
			_rebelLinks[i][2] = 0;
		}

		// ----- PHASE 1: 12P05 → P1/12P01_X -----
		// FUN_00401000: Reset at top of each retry
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;

		int16 budget = kLevel12BudgetBase[0] + _vm->_rnd.getRandomNumber(2);

		// Initialization video (12P05.SAN)
		debug("Rebel2: Level 12 Phase 1 - init 12P05.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV12/12P05.SAN", 12);
		if (_vm->shouldQuit())
			return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		// First wave (P1/12P01_A.SAN)
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/P1/12P01_A.SAN", 12);
		_deathFrame = splayer->_frame;
		if (_vm->shouldQuit())
			return kLevelQuit;

		{
			uint16 waveSelect = processWaveEnd(6, &budget, 0x14, 0);

			// Wave loop: random(2) | (waveSelect & 2) → 0:C, 1:D, 2:A, 3:B
			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				int sel = _vm->_rnd.getRandomNumber(1) | (waveSelect & 2);
				const char *filename;
				switch (sel) {
				case 0:  filename = "LEV12/P1/12P01_C.SAN"; break;
				case 1:  filename = "LEV12/P1/12P01_D.SAN"; break;
				case 2:  filename = "LEV12/P1/12P01_A.SAN"; break;
				default: filename = "LEV12/P1/12P01_B.SAN"; break;
				}

				debug("Rebel2: Level 12 Phase 1 wave - %s (state=0x%x sel=%d)", filename, _rebelPhaseState, sel);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				waveSelect = processWaveEnd(6, &budget, 0x14, 0);
			}
		}

		if (_playerDamage >= 255)
			goto level12_death;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// ----- PHASE 2: 12P06 → P2/12P02_X -----
		_currentPhase = 2;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;

		budget = kLevel12BudgetBase[1] + _vm->_rnd.getRandomNumber(3);

		// Initialization video (12P06.SAN)
		debug("Rebel2: Level 12 Phase 2 - init 12P06.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/12P06.SAN", 12);
		if (_vm->shouldQuit())
			return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		// First wave (P2/12P02_A.SAN)
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/P2/12P02_A.SAN", 12);
		_deathFrame = splayer->_frame;
		if (_vm->shouldQuit())
			return kLevelQuit;

		{
			uint16 waveSelect = processWaveEnd(6, &budget, 0x14, 0);

			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				// Variant selection: (waveSelect & 2) controls which set
				int variantIdx;
				if ((waveSelect & 2) == 0) {
					variantIdx = _vm->_rnd.getRandomNumber(2) + 3;  // 3, 4, or 5
				} else {
					variantIdx = _vm->_rnd.getRandomNumber(2);      // 0, 1, or 2
				}

				const char *filename;
				switch (variantIdx) {
				case 0:  filename = "LEV12/P2/12P02_A.SAN"; break;
				case 1:  filename = "LEV12/P2/12P02_B.SAN"; break;
				case 2:  filename = "LEV12/P2/12P02_E.SAN"; break;
				case 3:  filename = "LEV12/P2/12P02_C.SAN"; break;
				case 4:  filename = "LEV12/P2/12P02_D.SAN"; break;
				default: filename = "LEV12/P2/12P02_F.SAN"; break;
				}

				debug("Rebel2: Level 12 Phase 2 wave - %s (state=0x%x variantIdx=%d)", filename, _rebelPhaseState, variantIdx);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				// Variants E(2) and F(5) reset threshold to 0
				int16 threshold = (variantIdx == 2 || variantIdx == 5) ? 0 : 0x14;
				waveSelect = processWaveEnd(6, &budget, threshold, 0);
			}
		}

		if (_playerDamage >= 255)
			goto level12_death;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// ----- PHASE 3: 12P07 → P3/12P03_X -----
		_currentPhase = 3;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;

		budget = kLevel12BudgetBase[2] + _vm->_rnd.getRandomNumber(3);

		// Initialization video (12P07.SAN)
		debug("Rebel2: Level 12 Phase 3 - init 12P07.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/12P07.SAN", 12);
		if (_vm->shouldQuit())
			return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		// First wave (P3/12P03_A.SAN)
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/P3/12P03_A.SAN", 12);
		_deathFrame = splayer->_frame;
		if (_vm->shouldQuit())
			return kLevelQuit;

		{
			int variantIdx = 0;
			uint16 waveSelect = processWaveEnd(6, &budget, 0x14, 0);

			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				// Wider randomization for first few waves
				if (variantIdx < 4) {
					variantIdx = _vm->_rnd.getRandomNumber(5);  // 0-5
				} else {
					variantIdx = _vm->_rnd.getRandomNumber(3);  // 0-3
				}

				const char *filename;
				switch (variantIdx) {
				case 0:  filename = "LEV12/P3/12P03_C.SAN"; break;
				case 1:  filename = "LEV12/P3/12P03_D.SAN"; break;
				case 2:  filename = "LEV12/P3/12P03_A.SAN"; break;
				case 3:  filename = "LEV12/P3/12P03_B.SAN"; break;
				case 4:  filename = "LEV12/P3/12P03_F.SAN"; break;
				default: filename = "LEV12/P3/12P03_F.SAN"; break;  // duplicate F
				}

				debug("Rebel2: Level 12 Phase 3 wave - %s (state=0x%x variantIdx=%d)", filename, _rebelPhaseState, variantIdx);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				waveSelect = processWaveEnd(6, &budget, 0x14, 0);
			}
		}

		if (_playerDamage >= 255)
			goto level12_death;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// ----- PHASE 4: 12P08 → P4/12P04_X -----
		_currentPhase = 4;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;

		budget = kLevel12BudgetBase[3] + _vm->_rnd.getRandomNumber(3);

		// Initialization video (12P08.SAN)
		debug("Rebel2: Level 12 Phase 4 - init 12P08.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/12P08.SAN", 12);
		if (_vm->shouldQuit())
			return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		// First wave (P4/12P04_A.SAN)
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/P4/12P04_A.SAN", 12);
		_deathFrame = splayer->_frame;
		if (_vm->shouldQuit())
			return kLevelQuit;

		{
			int variantIdx = 0;
			uint16 waveSelect = processWaveEnd(0x0e, &budget, 0x14, 0);

			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				if (variantIdx < 4) {
					variantIdx = _vm->_rnd.getRandomNumber(5);  // 0-5
				} else {
					variantIdx = _vm->_rnd.getRandomNumber(3);  // 0-3
				}

				const char *filename;
				switch (variantIdx) {
				case 0:  filename = "LEV12/P4/12P04_C.SAN"; break;
				case 1:  filename = "LEV12/P4/12P04_D.SAN"; break;
				case 2:  filename = "LEV12/P4/12P04_A.SAN"; break;
				case 3:  filename = "LEV12/P4/12P04_B.SAN"; break;
				case 4:  filename = "LEV12/P4/12P04_E.SAN"; break;
				default: filename = "LEV12/P4/12P04_F.SAN"; break;
				}

				debug("Rebel2: Level 12 Phase 4 wave - %s (state=0x%x variantIdx=%d)", filename, _rebelPhaseState, variantIdx);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				waveSelect = processWaveEnd(0x0e, &budget, 0x14, 0);
			}
		}

		if (_playerDamage >= 255)
			goto level12_death;
		if (_vm->shouldQuit())
			return kLevelQuit;

		// ----- CLOSING: 12P09.SAN -----
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/12P09.SAN", 12);
		if (_vm->shouldQuit())
			return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		// ----- LEVEL COMPLETED -----
		{
			int accuracy = 0;
			if (_rebelKillCounter > 0) {
				accuracy = (_rebelKillCounter * 100) / (_rebelHitCounter + _rebelKillCounter);
			}
			debug("Rebel2: Level 12 completed! kills=%d misses=%d accuracy=%d%%",
				_rebelKillCounter, _rebelHitCounter, accuracy);
		}

		// Bonus checks: FUN_0042aa70(0x61), FUN_0042aa70(99), FUN_0042aa70(0x74)
		// If all three bonuses found, play special ending (12END_Z.SAN)
		// Deferred until bonus tracking is implemented

		playLevelEnd(12);
		_levelUnlocked[12] = true;  // Unlock level 13
		return kLevelNextLevel;

	level12_death:
		// Single death video for all phases
		debug("Rebel2: Level 12 Phase %d death", _currentPhase);
		playCinematic("LEV12/12DIE.SAN");

		if (_vm->shouldQuit())
			return kLevelQuit;
		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(12);
			return kLevelGameOver;
		}
		playCinematic("LEV12/12RETRY.SAN");
		_playerDamage = 0;
		if (_vm->shouldQuit())
			return kLevelQuit;
		continue;  // Restart from Phase 1
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 13 Handler - FUN_0041B3E1
// "Escaping the Star Destroyer" - Two-phase flight/escape
// Phase A: 13PLAY_A.SAN (main flight), transitions to Phase B at maxFrame-10
// Phase B: 13PLAY_B.SAN (reactor loop, flags 0x468) — plays until
//   (DAT_0047ab90 == 0 && DAT_0047ab7c == 0) meaning all targets destroyed.
// Death: frame-based (A/B/C/B/D pattern)
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel13() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// No cutscene — starts directly with BEG
	// Original: FUN_004171c5("13BEG.SAN", 0x20, 0xb6, 0xa0, 10, 2, 0x46)
	playLevelBegin(13);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_00401000 + FUN_0041c7d0 + FUN_0040c040
	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;

		clearBit(0);

		// Phase A: Main escape flight (13PLAY_A.SAN)
		// Original: FUN_0041f4d0("13PLAY_A.SAN", 0x28, -1, -1, 0)
		// First inner loop runs until frame reaches maxFrame-10
		// Then Phase B (13PLAY_B.SAN, flags 0x468) plays at that exact frame
		// The 0x468 flags indicate seamless mid-video transition.
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV13/13PLAY_A.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		// If alive after Phase A, play Phase B (reactor destruction loop)
		// Original: at frame == maxFrame-10, play 13PLAY_B.SAN (0x468)
		// Then loop while (DAT_0047ab90 != 0 || DAT_0047ab7c != 0)
		// For now, play B as a sequential video. The IACT callbacks will manage
		// the reactor target state through opcode interactions.
		if (_playerShield > 0) {
			splayer->setCurVideoFlags(0x468);
			splayer->play("LEV13/13PLAY_B.SAN", 12);
			_deathFrame = splayer->_frame;
		}

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = 0;
			if (_rebelKillCounter > 0) {
				accuracy = (_rebelKillCounter * 100) / (_rebelHitCounter + _rebelKillCounter);
			}
			debug("Rebel2: Level 13 completed! accuracy=%d%%", accuracy);
			playLevelEnd(13);
			_levelUnlocked[13] = true;
			return kLevelNextLevel;
		}

		// Death: frame-based variant selection (FUN_0041B3E1 lines 47-61)
		debug("Rebel2: Level 13 death at frame %d", _deathFrame);
		playLevelDeathVariant(13, 1, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(13);
			return kLevelGameOver;
		}

		playCinematic("LEV13/13RETRY.SAN");
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 14 Handler - FUN_0041B6E8
// "TIE Attack" - Final space battle (single phase)
// No cutscene. Single death video (14DIE.SAN, no variants).
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel14() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// No cutscene — starts directly with BEG
	// Original: FUN_004171c5("14BEG.SAN", 0x20, 0xb7, 0xa0, 10, 2, 0x46)
	playLevelBegin(14);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_00401000 + FUN_0041c7d0 + FUN_0040c040
	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;

		clearBit(0);

		// Play gameplay (14PLAY.SAN)
		// Original: FUN_0041f4d0("14PLAY.SAN", 0x28, -1, -1, 0)
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV14/14PLAY.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = 0;
			if (_rebelKillCounter > 0) {
				accuracy = (_rebelKillCounter * 100) / (_rebelHitCounter + _rebelKillCounter);
			}
			debug("Rebel2: Level 14 completed! accuracy=%d%%", accuracy);
			playLevelEnd(14);
			_levelUnlocked[14] = true;
			return kLevelNextLevel;
		}

		// Death: single video (14DIE.SAN)
		debug("Rebel2: Level 14 death at frame %d", _deathFrame);
		playCinematic("LEV14/14DIE.SAN");
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(14);
			return kLevelGameOver;
		}

		playCinematic("LEV14/14RETRY.SAN");
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

// ---------------------------------------------------------------------------
// Level 15 Handler - FUN_0041B8D7
// "Imdaar Alpha" - Final mission (single long phase with level ID switch)
// Has cutscene. Mid-level: DAT_0047a7f8 changes from 0xf to 0x10 at frame 0x21e.
// This represents a transition from the tunnel section to the core section.
// Death: frame-based (A/B/C/B/C/B/D pattern with 7 thresholds)
// On completion → FUN_0041BBE8 (credits/end game, not a playable level)
// ---------------------------------------------------------------------------

int InsaneRebel2::runLevel15() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play cutscene (15CUT.SAN)
	playCinematic("LEV15/15CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	// Play level beginning cinematic (15BEG.SAN)
	// Original: FUN_004171c5("15BEG.SAN", 0x20, 0xb8, 0xa0, 10, 2, 0x46)
	playLevelBegin(15);
	if (_vm->shouldQuit())
		return kLevelQuit;

	// FUN_00401000 + FUN_0041c7d0 + FUN_0040c040
	clearBit(0);

	// Original sets DAT_0047a7f8 = 0xf before the gameplay loop
	_rebelLevelType = 0xf;

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;

		clearBit(0);

		// Original: DAT_0047a7f8 = 0xf again at start of each retry
		// At frame 0x21e (542): switches to 0x10 (affects difficulty lookup mid-level)
		// The frame-based switch is handled by IACT opcode 6 in the video data.
		_rebelLevelType = 0xf;

		// Play gameplay (15PLAY.SAN)
		// Original: FUN_0041f4d0("15PLAY.SAN", 0x28, -1, -1, 0)
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV15/15PLAY.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit())
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = 0;
			if (_rebelKillCounter > 0) {
				accuracy = (_rebelKillCounter * 100) / (_rebelHitCounter + _rebelKillCounter);
			}
			debug("Rebel2: Level 15 completed! accuracy=%d%%", accuracy);
			playLevelEnd(15);
			_levelUnlocked[15] = true;
			// Level 15 completion leads to credits (FUN_0041BBE8)
			return kLevelNextLevel;
		}

		// Death: frame-based variant selection (FUN_0041B8D7 lines 46-65)
		debug("Rebel2: Level 15 death at frame %d", _deathFrame);
		playLevelDeathVariant(15, 1, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(15);
			return kLevelGameOver;
		}

		playCinematic("LEV15/15RETRY.SAN");
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

} // End of namespace Scumm
