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

int InsaneRebel2::runLevel1() {
	playLevelBegin(1);
	if (_vm->shouldQuit())
		return kLevelQuit;

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;
		resetExplosions();

		clearBit(0);
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		if (!playLevelSegment("LEV01/01P01.SAN", 0x28))
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			debugC(DEBUG_INSANE, "Level 1 completed!");
			playLevelEnd(1, accuracy, -1, false);
			_levelUnlocked[1] = true;
			return kLevelNextLevel;
		}

		debugC(DEBUG_INSANE, "Level 1 death at frame %d, lives=%d", _deathFrame, _playerLives - 1);
		playLevelDeathVariant(1, 1, _deathFrame);

		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(1);
			return kLevelGameOver;
		}

		playLevelRetry(1);
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

InsaneRebel2::WaveEndResult InsaneRebel2::processWaveEnd(int16 mask, int16 *budget, int16 threshold, uint16 flags) {
	WaveEndResult result;

	if (_rebelMovieMode) {
		_skipSectionRequested = false;
		_rebelPhaseState = mask;
		_rebelWaveState = mask;
		debugC(DEBUG_INSANE, "processWaveEnd: movie mode completed gameplay wave (mask=0x%x)", (uint16)mask);
		result.completed = true;
		result.skipped = true;
		return result;
	}

	if (_skipSectionRequested) {
		_skipSectionRequested = false;
		_rebelPhaseState = mask;
		_rebelWaveState = mask;
		debugC(DEBUG_INSANE, "processWaveEnd: Shift+S skip consumed (mask=0x%x)", (uint16)mask);
		result.completed = true;
		result.skipped = true;
		return result;
	}

	_rebelPhaseState = _rebelWaveState;
	debugC(DEBUG_INSANE, "processWaveEnd: waveState=0x%x -> phaseState=0x%x mask=0x%x budget=%d threshold=%d flags=%d",
		_rebelWaveState, _rebelPhaseState, mask, budget ? *budget : -1, threshold, flags);

	if ((flags & 2) != 0) {
		byte unkilled[8];
		int16 numUnkilled = 0;
		for (byte b = 0; (2 << (b & 0x1f)) < (int)(mask & 0x0e); b++) {
			if ((_rebelPhaseState & (2 << (b & 0x1f))) == 0) {
				unkilled[numUnkilled] = (byte)(2 << (b & 0x1f));
				numUnkilled++;
			}
		}
		if (numUnkilled > 0) {
			int idx = _vm->_rnd.getRandomNumber(numUnkilled - 1);
			_rebelPhaseState |= unkilled[idx];
			if (budget)
				(*budget)++;
		}
	}

	byte killed[8];
	int16 numKilled = 0;
	for (byte b = 0; (2 << (b & 0x1f)) < (int)(mask & 0x0e); b++) {
		if ((_rebelPhaseState & (2 << (b & 0x1f))) != 0) {
			killed[numKilled] = (byte)(2 << (b & 0x1f));
			numKilled++;
		}
	}

	int16 maxCredits = ((flags & 1) == 0) ? 8 : 2;

	int16 creditCount = 0;
	while (creditCount < maxCredits && numKilled > 0 && budget && *budget > 0) {
		int idx = _vm->_rnd.getRandomNumber(numKilled - 1);
		_rebelPhaseState -= killed[idx];
		result.creditedBits |= killed[idx];
		(*budget)--;

		for (int i = idx; i + 1 < numKilled; i++) {
			killed[i] = killed[i + 1];
		}
		numKilled--;
		creditCount++;
	}

	debugC(DEBUG_INSANE, "processWaveEnd: result=0x%x phaseState=0x%x (after redistribution) budget=%d",
		result.creditedBits, _rebelPhaseState, budget ? *budget : -1);

	result.died = (_playerDamage >= 255);
	result.completed = ((int16)_rebelPhaseState >= mask);
	result.quit = _vm->shouldQuit();

	return result;
}

bool InsaneRebel2::playLevelSegment(const char *filename, uint16 flags, bool recordFrame) {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	const bool isRecordedGameplay = recordFrame && (flags & 0x08) != 0;
	if (isRecordedGameplay && _rebelMovieMode) {
		debugC(DEBUG_INSANE, "Movie mode skipping gameplay segment: %s", filename);
		_gameplaySectionActive = false;
		restoreIOSGamepadController();
		if (recordFrame)
			_deathFrame = 0;
		restoreDamageFlashPalette();
		return true;
	}

	if (isRecordedGameplay) {
		// Center only at the section boundary; looped wave videos are continuations.
		if (!_gameplaySectionActive && (flags & 0x40) == 0)
			centerGameplayAim();
		_gameplaySectionActive = true;
		enableIOSGamepadController();
	} else {
		if (_gameplaySectionActive)
			resetVideoAudio();
		_gameplaySectionActive = false;
		restoreIOSGamepadController();
	}

	splayer->setCurVideoFlags(flags);
	splayer->play(filename, 15);
	if (isRecordedGameplay)
		restoreIOSGamepadController();
	if (recordFrame)
		_deathFrame = splayer->_frame;
	restoreDamageFlashPalette();
	return !_vm->shouldQuit();
}

int InsaneRebel2::calculateAccuracy(int kills, int misses) const {
	const int totalShots = kills + misses;
	if (kills <= 0 || totalShots <= 0)
		return 0;

	return (kills * 100) / totalShots;
}

bool InsaneRebel2::handleLevelDeath(int levelId, int phase,
		const char *deathVideo, const char *retryVideo, int &levelResult) {
	debugC(DEBUG_INSANE, "Level %d Phase %d death", levelId, phase);
	playCinematic(deathVideo);
	if (_vm->shouldQuit()) {
		levelResult = kLevelQuit;
		return false;
	}

	_playerLives--;
	if (_playerLives <= 0) {
		playLevelGameOver(levelId);
		levelResult = kLevelGameOver;
		return false;
	}

	playCinematic(retryVideo);
	_playerDamage = 0;
	if (_vm->shouldQuit()) {
		levelResult = kLevelQuit;
		return false;
	}

	return true;
}

void InsaneRebel2::resetLevelAttemptState(int initialPhase) {
	_playerShield = 255;
	_playerDamage = 0;
	_currentPhase = initialPhase;
	resetDamageFlash();
	_damageHighFlashCounter = 0;
	_damageShakeCounter = 0;

	_rebelAutopilot = 0;
	_rebelDamageLevel = 0;
	_rebelControlMode = 0;
	resetExplosions();

	_enemies.clear();
	for (int i = 0; i < 512; i++) {
		_rebelLinks[i][0] = 0;
		_rebelLinks[i][1] = 0;
		_rebelLinks[i][2] = 0;
	}
}

void InsaneRebel2::resetLevelPhaseState(bool clearEnemies) {
	_rebelKillCounter = 0;
	_rebelHitCounter = 0;
	resetLevelWaveState();

	delete _grd001Sprite;
	_grd001Sprite = nullptr;
	delete _grd002Sprite;
	_grd002Sprite = nullptr;
	delete _grd005Sprite;
	_grd005Sprite = nullptr;
	_grdShotOriginTableLoaded = false;

	clearEmbeddedHudFrames();

	free(_level2Background);
	_level2Background = nullptr;
	_level2BackgroundLoaded = false;

	if (clearEnemies)
		_enemies.clear();

	// Every phase starts with the player fully in cover. This only has an
	// effect in the on-foot cover sections (handler 25, levels 2 and 11);
	// the other levels ignore these values.
	_rebelAutopilot = 1;
	_rebelDamageLevel = 5;
}

void InsaneRebel2::clearEmbeddedHudFrames() {
	for (uint i = 0; i < ARRAYSIZE(_rebelEmbeddedHud); ++i) {
		EmbeddedSanFrame &frame = _rebelEmbeddedHud[i];
		free(frame.pixels);
		frame.pixels = nullptr;
		frame.width = 0;
		frame.height = 0;
		frame.renderX = 0;
		frame.renderY = 0;
		frame.valid = false;
	}
}

void InsaneRebel2::resetLevelWaveState() {
	_rebelPhaseState = 0;
	_rebelWaveState = 0;
}

void InsaneRebel2::resetGaugeCounters() {
	for (int i = 0; i < 10; ++i) {
		_rebelValueCounters[i] = 0;
		_rebelGaugeBlink[i] = 0;
	}
	_rebelLastCounter = -1;        // non-zero sentinel: gauge not yet depleted
}

void InsaneRebel2::resetShieldGauge() {
	resetGaugeCounters();
	for (int i = 0; i < 15; ++i) {
		_turretShakeRingX[i] = 0;
		_turretShakeRingY[i] = 0;
	}
	_rebelShieldDestroyed = false;
	_rebelGaugeArmed = false;
	_rebelLastArmedSlot = -1;
}

void InsaneRebel2::decrementGaugeGroup(int slot, int targetId) {
	_rebelValueCounters[slot]--;
	if (_rebelValueCounters[slot] == 0) {
		_rebelGaugeBlink[slot] = 6;
		if (_rebelShieldGateActive && !_rebelReactorMode) {
			_rebelShieldDestroyed = true;
			debugC(DEBUG_INSANE, "Shield destroyed (gauge slot %d depleted by target %d)", slot, targetId);
		}
	}
	_rebelLastCounter = _rebelValueCounters[slot];
}

int16 InsaneRebel2::getWaveBudgetBase(int phase) const {
	// Per-phase wave budgets share the tail columns (rollRate..driftRate) of the
	// previous level-type row with the flight movement rates.
	const LevelDifficultyParams &p =
		kDifficultyTable[CLIP(_difficulty, 0, 5)][CLIP(_rebelLevelType - 1, 0, 16)];
	switch (phase) {
	case 1:
		return p.rollRate;
	case 2:
		return p.liftRate;
	case 3:
		return p.slideRate;
	default:
		return p.driftRate;
	}
}

void InsaneRebel2::resetExplosions() {
	for (uint i = 0; i < ARRAYSIZE(_explosions); ++i) {
		_explosions[i].active = false;
		_explosions[i].counter = 0;
		_explosions[i].x = 0;
		_explosions[i].y = 0;
		_explosions[i].width = 0;
		_explosions[i].height = 0;
		_explosions[i].scale = 0;
		_explosions[i].dx = 0;
		_explosions[i].dy = 0;
	}
}

void InsaneRebel2::resetHandler7FlightState() {
	_hitCooldown = 0;
	_flyShipScreenX = 0xd4;
	_flyShipScreenY = 0x82;
	_smoothedVelocity = 0;
	_verticalInput = 0;
	_flyOverlayRepeatCount = 0;
	_viewShift = 0;
	_perspectiveX = 0;
	_perspectiveY = 0;
	_windParamX = 0;
	_windParamY = 0;
	_corridorLeftX = 0;
	_corridorTopY = 0;
	_corridorRightX = 0x1a8;
	_corridorBottomY = 0x104;
	_facingRight = false;
	_spaceShotDirection = 0;
	memset(_flyLeftGunX, 0, sizeof(_flyLeftGunX));
	memset(_flyLeftGunY, 0, sizeof(_flyLeftGunY));
	memset(_flyRightGunX, 0, sizeof(_flyRightGunX));
	memset(_flyRightGunY, 0, sizeof(_flyRightGunY));
	_flyLeftGunTableLoaded = false;
	_flyRightGunTableLoaded = false;

	memset(_velocityHistory, 0, sizeof(_velocityHistory));
	memset(_windHistoryX, 0, sizeof(_windHistoryX));
	memset(_windHistoryY, 0, sizeof(_windHistoryY));

	for (int i = 0; i < 2; i++) {
		_spaceShots[i].counter = 0;
		_spaceShots[i].targetX = 0;
		_spaceShots[i].targetY = 0;
		_spaceShots[i].leftGunX = 0;
		_spaceShots[i].leftGunY = 0;
		_spaceShots[i].rightGunX = 0;
		_spaceShots[i].rightGunY = 0;
		_spaceShots[i].variant = 0;
	}
}

int InsaneRebel2::runLevel2() {
	int bonusCount = 0;
	int totalKills = 0;
	int totalMisses = 0;
	int prevWaveState = 0;

	playCinematic("LEV02/02CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	playLevelBegin(2);
	if (_vm->shouldQuit())
		return kLevelQuit;

	clearBit(0);

	while (!_vm->shouldQuit()) {
		resetLevelAttemptState(1);
		bonusCount = 0;
		totalKills = 0;
		totalMisses = 0;

		resetLevelPhaseState(false);

		int16 budget = getWaveBudgetBase(1) + _vm->_rnd.getRandomNumber(2);

		debugC(DEBUG_INSANE, "Level 2 Phase 1 - playing 02P01_A.SAN (background) budget=%d", budget);
		if (!playLevelSegment("LEV02/P1/02P01_A.SAN", 0x28))
			return kLevelQuit;

		processWaveEnd(0x36, &budget, 0, 0);

		while (_playerDamage < 255 && (_rebelPhaseState & 0x06) != 0x06) {
			if (_vm->shouldQuit())
				return kLevelQuit;

			int variant = _vm->_rnd.getRandomNumber(2);  // 0-2
			const char *variants[] = {
				"LEV02/P1/02P01_B.SAN",
				"LEV02/P1/02P01_C.SAN",
				"LEV02/P1/02P01_D.SAN"
			};
			debugC(DEBUG_INSANE, "Phase 1 wave - playing %s (state=0x%x budget=%d)", variants[variant], _rebelPhaseState, budget);
			if (!playLevelSegment(variants[variant], 0x428))
				return kLevelQuit;

			processWaveEnd(0x36, &budget, 0x14, 0);
			debugC(DEBUG_INSANE, "Phase 1 wave done - state=0x%x (need 0x06) budget=%d", _rebelPhaseState, budget);
		}

		if ((_rebelPhaseState & 0x10) != 0)
			bonusCount++;

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(2, _currentPhase, "LEV02/02DIE.SAN", "LEV02/02RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		if (!playLevelSegment("LEV02/02PST1.SAN", 0x28, false))
			return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		_currentPhase = 2;
		resetLevelPhaseState(true);

		budget = getWaveBudgetBase(2) + _vm->_rnd.getRandomNumber(2);

		_rebelHandler = 8;

		debugC(DEBUG_INSANE, "Level 2 Phase 2 - playing 02P02_A.SAN (background) budget=%d", budget);
		if (!playLevelSegment("LEV02/P2/02P02_A.SAN", 0x28))
			return kLevelQuit;

		while (true) {
			WaveEndResult waveEnd = processWaveEnd(0x3e, &budget, 0, 0);
			uint16 waveSelect = waveEnd.creditedBits;
			if (waveEnd.shouldStop() || (_rebelPhaseState & 0x0e) == 0x0e)
				break;
			if (_vm->shouldQuit())
				return kLevelQuit;


			if ((waveSelect & 0x0c) == 0) {
				waveSelect = _vm->_rnd.getRandomNumber(2) + 0x10;
			}

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

			debugC(DEBUG_INSANE, "Phase 2 wave - playing %s (state=0x%x sel=0x%x budget=%d)", filename, _rebelPhaseState, waveSelect, budget);
			if (!playLevelSegment(filename, 0x428))
				return kLevelQuit;
		}

		if ((_rebelPhaseState & 0x10) != 0)
			bonusCount++;

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(2, _currentPhase, "LEV02/02DIE.SAN", "LEV02/02RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		if (!playLevelSegment("LEV02/02PST2.SAN", 0x28, false))
			return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		_currentPhase = 3;
		resetLevelPhaseState(true);
		prevWaveState = 0;

		budget = getWaveBudgetBase(3) + _vm->_rnd.getRandomNumber(2);

		_rebelHandler = 8;

		debugC(DEBUG_INSANE, "Level 2 Phase 3 - playing 02P03_A.SAN (background) budget=%d", budget);
		if (!playLevelSegment("LEV02/P3/02P03_A.SAN", 0x28))
			return kLevelQuit;

		{
			WaveEndResult waveEnd = processWaveEnd(0x3e, &budget, 0, 0);

			while (!waveEnd.shouldStop() && (_rebelPhaseState & 0x0e) != 0x0e) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				uint16 waveSelect = waveEnd.creditedBits;

				if (((prevWaveState & 1) == 0) && (_vm->_rnd.getRandomNumber(7) == 0)) {
					waveSelect |= 1;
				}
				prevWaveState = waveSelect;

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

				debugC(DEBUG_INSANE, "Phase 3 wave - playing %s (state=0x%x sel=0x%x budget=%d)", filename, _rebelPhaseState, waveSelect, budget);
				if (!playLevelSegment(filename, 0x428))
					return kLevelQuit;

				waveEnd = processWaveEnd(0x3e, &budget, 0x14, 0);
				debugC(DEBUG_INSANE, "Phase 3 wave done - state=0x%x (need 0x0e) budget=%d", _rebelPhaseState, budget);
			}
		}

		if ((_rebelPhaseState & 0x10) != 0)
			bonusCount++;
		totalKills += _rebelKillCounter;

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(2, _currentPhase, "LEV02/02DIE.SAN", "LEV02/02RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		{
			totalMisses += _rebelHitCounter;
			int accuracy = calculateAccuracy(totalKills, totalMisses);
			debugC(DEBUG_INSANE, "Level 2 completed! kills=%d misses=%d accuracy=%d%% bonus=%d",
				totalKills, totalMisses, accuracy, bonusCount);
			playLevelEnd(2, accuracy, -1, bonusCount > 2);
		}

		_levelUnlocked[2] = true;
		return kLevelNextLevel;
	}

	return kLevelQuit;
}

int InsaneRebel2::runLevel3() {
	int phase1Score = 0;
	int phase1FlightErrors = 0;

	playLevelBegin(3);
	if (_vm->shouldQuit())
		return kLevelQuit;

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;
		resetExplosions();

		clearEmbeddedHudFrames();

		clearBit(0);
		resetHandler7FlightState();
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		debugC(DEBUG_INSANE, "Level 3 Phase 1");
		if (!playLevelSegment("LEV03/03PLAY1.SAN", 0x28))
			return kLevelQuit;

		if (_playerShield > 0) {
			phase1Score = _playerScore;
			phase1FlightErrors = _rebelHitCounter;
			break;
		}

		debugC(DEBUG_INSANE, "Level 3 Phase 1 death at frame %d", _deathFrame);
		playLevelDeathVariant(3, 1, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(3);
			return kLevelGameOver;
		}

		playLevelRetryVariant(3, 1);
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	if (_vm->shouldQuit())
		return kLevelQuit;

	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;
	if (!playLevelSegment("LEV03/03POST1.SAN", 0x28, false))
		return kLevelQuit;

	_currentPhase = 2;

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_playerScore = phase1Score;
		resetExplosions();

		clearEmbeddedHudFrames();

		clearBit(0);
		resetHandler7FlightState();
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		debugC(DEBUG_INSANE, "Level 3 Phase 2");
		if (!playLevelSegment("LEV03/03PLAY2.SAN", 0x28))
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			int flightErrors = phase1FlightErrors + _rebelHitCounter;
			debugC(DEBUG_INSANE, "Level 3 completed!");
			playLevelEnd(3, accuracy, flightErrors, false);
			_levelUnlocked[3] = true;
			return kLevelNextLevel;
		}

		debugC(DEBUG_INSANE, "Level 3 Phase 2 death at frame %d", _deathFrame);
		playLevelDeathVariant(3, 2, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(3);
			return kLevelGameOver;
		}

		playLevelRetryVariant(3, 2);
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

int InsaneRebel2::runLevel4() {
	if (!playLevelSegment("LEV04/04CUT.SAN", 0x28, false))
		return kLevelQuit;

	playLevelBegin(4);
	if (_vm->shouldQuit())
		return kLevelQuit;

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;
		resetExplosions();

		clearBit(0);
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		debugC(DEBUG_INSANE, "Level 4 gameplay");
		if (!playLevelSegment("LEV04/04PLAY.SAN", 0x28))
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			debugC(DEBUG_INSANE, "Level 4 completed!");
			playLevelEnd(4, accuracy, _rebelHitCounter, false);
			_levelUnlocked[4] = true;  // Unlock level 5
			return kLevelNextLevel;
		}

		debugC(DEBUG_INSANE, "Level 4 death");
		// The death video only plays on the retry path; the last life goes
		// straight to the game-over cinematic.
		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(4);
			return kLevelGameOver;
		}

		playLevelDeathVariant(4, 1, _deathFrame);
		if (_vm->shouldQuit())
			return kLevelQuit;

		playLevelRetry(4);
		if (_vm->shouldQuit())
			return kLevelQuit;
	}

	return kLevelQuit;
}

int InsaneRebel2::runLevel5() {
	playLevelBegin(5);
	if (_vm->shouldQuit())
		return kLevelQuit;

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;
		resetExplosions();

		clearBit(0);
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		debugC(DEBUG_INSANE, "Level 5 gameplay");
		if (!playLevelSegment("LEV05/05PLAY.SAN", 0x08))
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			debugC(DEBUG_INSANE, "Level 5 completed!");
			playLevelEnd(5, accuracy, -1, false);
			_levelUnlocked[5] = true;  // Unlock level 6
			return kLevelNextLevel;
		}

		debugC(DEBUG_INSANE, "Level 5 death at frame %d", _deathFrame);
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

int InsaneRebel2::runLevel6() {
	int phase1Score = 0;
	int phase1FlightErrors = 0;

	playLevelBegin(6);
	if (_vm->shouldQuit())
		return kLevelQuit;

	while (!_vm->shouldQuit()) {
		clearBit(0);
		resetExplosions();

		_rebelPhaseState = 0xffffffff;

		_rebelLevelType = 5;
		_currentPhase = 1;
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		debugC(DEBUG_INSANE, "Level 6 Phase 1 (shield attack run)");
		resetShieldGauge();
		_rebelShieldGateActive = true;

		if (!playLevelSegment("LEV06/06PLAY1.SAN", 0x28)) {
			_rebelShieldGateActive = false;
			return kLevelQuit;
		}

		while (_playerShield > 0 && !_rebelShieldDestroyed && !_vm->shouldQuit()) {
			resetShieldGauge();
			if (!playLevelSegment("LEV06/06PLAY1B.SAN", 0x468)) {
				_rebelShieldGateActive = false;
				return kLevelQuit;
			}
		}
		_rebelShieldGateActive = false;

		if (_playerShield <= 0) {
			debugC(DEBUG_INSANE, "Level 6 Phase 1 death at frame %d", _deathFrame);
			playLevelDeathVariant(6, 1, _deathFrame);
			if (_vm->shouldQuit())
				return kLevelQuit;

			_playerLives--;
			if (_playerLives <= 0) {
				playLevelGameOver(6);
				return kLevelGameOver;
			}

			playLevelRetryVariant(6, 1);
			if (_vm->shouldQuit())
				return kLevelQuit;
			_playerShield = 255;  // full shield on retry (original resets damage before replaying)
			_playerDamage = 0;
			continue;
		}

		phase1Score = _playerScore;
		phase1FlightErrors = _rebelHitCounter;

		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		if (!playLevelSegment("LEV06/06POST1.SAN", 0x28, false))
			return kLevelQuit;

		while (!_vm->shouldQuit()) {
			_rebelLevelType = 6;
			_currentPhase = 2;
			_playerScore = phase1Score;
			clearBit(0);
			resetExplosions();
			_rebelKillCounter = 0;
			_rebelHitCounter = 0;

			debugC(DEBUG_INSANE, "Level 6 Phase 2");
			if (!playLevelSegment("LEV06/06PLAY2.SAN", 0x28))
				return kLevelQuit;

			if (_playerShield > 0) {
				int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
				int flightErrors = phase1FlightErrors + _rebelHitCounter;
				debugC(DEBUG_INSANE, "Level 6 completed!");
				playLevelEnd(6, accuracy, flightErrors, false);
				_levelUnlocked[6] = true;
				return kLevelNextLevel;
			}

			debugC(DEBUG_INSANE, "Level 6 Phase 2 death at frame %d", _deathFrame);
			playLevelDeathVariant(6, 2, _deathFrame);
			if (_vm->shouldQuit())
				return kLevelQuit;

			_playerLives--;
			if (_playerLives <= 0) {
				playLevelGameOver(6);
				return kLevelGameOver;
			}

			playLevelRetryVariant(6, 2);
			if (_vm->shouldQuit())
				return kLevelQuit;
			_playerShield = 255;  // full shield on retry, as in phase 1
			_playerDamage = 0;
		}

		break;  // Should only reach here on shouldQuit
	}

	return kLevelQuit;
}

int InsaneRebel2::runLevel7() {
	playCinematic("LEV07/07CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	playLevelBegin(7);
	if (_vm->shouldQuit())
		return kLevelQuit;

	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;
		_level7TookRightFork = false;
		resetExplosions();

		clearBit(0);
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		// The corridor forks at frame 0x638. updateLevel7Fork() is armed only
		// during 07PLAY: if the player is on the right half there, it stops this
		// segment (so 07PLAYB can splice in) and sets _level7TookRightFork; the
		// left half plays 07PLAY through to its end.
		_level7ForkActive = true;
		const bool playOk = playLevelSegment("LEV07/07PLAY.SAN", 0x28);
		_level7ForkActive = false;
		if (!playOk)
			return kLevelQuit;

		// Right fork: continue into the alternate corridor segment (0x40 =
		// continuation, so the ship position carries over instead of recentering).
		if (_level7TookRightFork && _playerShield > 0) {
			if (!playLevelSegment("LEV07/07PLAYB.SAN", 0x468))
				return kLevelQuit;
		}

		if (_playerShield > 0) {
			debugC(DEBUG_INSANE, "Level 7 completed! fork=%d", _level7TookRightFork);
			playLevelEnd(7, -1, _rebelHitCounter, !_level7TookRightFork);
			_levelUnlocked[7] = true;
			return kLevelNextLevel;
		}

		// Death cinematic depends on which corridor side the player died on
		// (the original keys 07DIE_B/07DIE_A on DAT_0047ab8c), independent of
		// the fork frame; taking the right fork implies the right side.
		const bool diedOnRight = _level7TookRightFork || (_flyShipScreenX + _smoothedVelocity > 0xd4);
		debugC(DEBUG_INSANE, "Level 7 death at frame %d, right=%d", _deathFrame, diedOnRight);
		if (diedOnRight) {
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

int InsaneRebel2::runLevel8() {
	playLevelBegin(8);
	if (_vm->shouldQuit())
		return kLevelQuit;

	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;
		resetExplosions();

		clearBit(0);
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		if (!playLevelSegment("LEV08/08PLAY.SAN", 0x08))
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			debugC(DEBUG_INSANE, "Level 8 completed! accuracy=%d%%", accuracy);
			playLevelEnd(8, accuracy, -1, false);
			_levelUnlocked[8] = true;
			return kLevelNextLevel;
		}

		debugC(DEBUG_INSANE, "Level 8 death at frame %d", _deathFrame);
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

int InsaneRebel2::runLevel9() {
	playLevelBegin(9);
	if (_vm->shouldQuit())
		return kLevelQuit;

	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;
		resetExplosions();

		clearBit(0);
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;
		resetShieldGauge();

		_rebelPhaseState = 0xfffffffe;

		if (!playLevelSegment("LEV09/09PLAY.SAN", 0x28))
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			debugC(DEBUG_INSANE, "Level 9 completed! accuracy=%d%%", accuracy);
			playLevelEnd(9, accuracy, _rebelHitCounter, false);
			_levelUnlocked[9] = true;
			return kLevelNextLevel;
		}

		debugC(DEBUG_INSANE, "Level 9 death at frame %d", _deathFrame);
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

int InsaneRebel2::runLevel10() {
	playCinematic("LEV10/10CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	playLevelBegin(10);
	if (_vm->shouldQuit())
		return kLevelQuit;

	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;
		resetExplosions();

		clearBit(0);
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		if (!playLevelSegment("LEV10/10PLAY.SAN", 0x28))
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			debugC(DEBUG_INSANE, "Level 10 completed! accuracy=%d%%", accuracy);
			playLevelEnd(10, accuracy, _rebelHitCounter, false);
			_levelUnlocked[10] = true;
			return kLevelNextLevel;
		}

		debugC(DEBUG_INSANE, "Level 10 death at frame %d", _deathFrame);
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

int InsaneRebel2::runLevel11() {
	int totalKills = 0;
	int totalMisses = 0;
	int prevPhaseState = 0;

	playCinematic("LEV11/11CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	playLevelBegin(11);
	if (_vm->shouldQuit())
		return kLevelQuit;

	clearBit(0);

	while (!_vm->shouldQuit()) {
		resetLevelAttemptState(1);
		totalKills = 0;
		totalMisses = 0;
		prevPhaseState = 0;

		resetLevelPhaseState(false);

		int16 budget = getWaveBudgetBase(1) + _vm->_rnd.getRandomNumber(2);

		debugC(DEBUG_INSANE, "Level 11 Phase 1 - playing 11P01_A.SAN budget=%d", budget);
		if (!playLevelSegment("LEV11/P1/11P01_A.SAN", 0x28))
			return kLevelQuit;

		{
			WaveEndResult waveEnd = processWaveEnd(0x0e, &budget, 0, 0);

			while (!waveEnd.shouldStop()) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				uint16 waveSelect = waveEnd.creditedBits;

				if ((_rebelPhaseState & 0x10) != 0 && (prevPhaseState & 0x10) == 0) {
					playAuxSfx(3, 127, 0);
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

				debugC(DEBUG_INSANE, "Level 11 Phase 1 wave - %s (state=0x%x sel=%d)", filename, _rebelPhaseState, sel);
				if (!playLevelSegment(filename, 0x428))
					return kLevelQuit;

				waveEnd = processWaveEnd(0x0e, &budget, 0x14, 0);
			}
		}

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(11, 1, "LEV11/11DIE_A.SAN", "LEV11/11RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		if (!playLevelSegment("LEV11/11POST1.SAN", 0x28, false))
			return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		_currentPhase = 2;
		resetLevelPhaseState(true);

		budget = getWaveBudgetBase(2) + _vm->_rnd.getRandomNumber(2);
		_rebelHandler = 8;

		debugC(DEBUG_INSANE, "Level 11 Phase 2 - playing 11P02_A.SAN budget=%d", budget);
		if (!playLevelSegment("LEV11/P2/11P02_A.SAN", 0x28))
			return kLevelQuit;

		{
			WaveEndResult waveEnd = processWaveEnd(0x0e, &budget, 0, 3);

			while (!waveEnd.shouldStop()) {
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

				debugC(DEBUG_INSANE, "Level 11 Phase 2 wave - %s (state=0x%x)", filename, _rebelPhaseState);
				if (!playLevelSegment(filename, 0x428))
					return kLevelQuit;

				waveEnd = processWaveEnd(0x0e, &budget, 0x14, 3);
			}
		}

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(11, 2, "LEV11/11DIE_B.SAN", "LEV11/11RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		if (!playLevelSegment("LEV11/11POST2.SAN", 0x28, false))
			return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		_currentPhase = 3;
		resetLevelPhaseState(true);
		prevPhaseState = 0;

		budget = getWaveBudgetBase(3) + _vm->_rnd.getRandomNumber(2);
		_rebelHandler = 8;

		debugC(DEBUG_INSANE, "Level 11 Phase 3 first half - playing 11P03_A.SAN budget=%d", budget);
		if (!playLevelSegment("LEV11/P3/11P03_A.SAN", 0x28))
			return kLevelQuit;

		{
			WaveEndResult waveEnd = processWaveEnd(0x7e, &budget, 0, 0);
			int variantIdx = 0;  // Tracks variant for randomization threshold

			while (!waveEnd.shouldStop() && (_rebelPhaseState & 0x70) != 0x70) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				if ((_rebelPhaseState & 0x0e) == 0x0e && (prevPhaseState & 0x0e) != 0x0e) {
					playAuxSfx(3, 127, 0);
				}
				prevPhaseState = _rebelPhaseState;

				if (variantIdx < 3) {
					variantIdx = _vm->_rnd.getRandomNumber(7);
				} else {
					variantIdx = _vm->_rnd.getRandomNumber(2);
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
				default: filename = "LEV11/P3/11P03_E.SAN"; break;
				}

				debugC(DEBUG_INSANE, "Level 11 Phase 3a wave - %s (state=0x%x variantIdx=%d)", filename, _rebelPhaseState, variantIdx);
				if (!playLevelSegment(filename, 0x428))
					return kLevelQuit;

				int16 threshold = (variantIdx > 2) ? 0x14 : 0;
				waveEnd = processWaveEnd(0x7e, &budget, threshold, 0);
			}
		}

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(11, 3, "LEV11/11DIE_C.SAN", "LEV11/11RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		{
			bool allBasicKilled = (_rebelPhaseState & 0x0e) >= 0x0e;
			if (!allBasicKilled) {
				playCinematic("LEV11/11POST3.SAN");
			} else {
				playCinematic("LEV11/11POST3B.SAN");
			}
		}

		if (_vm->shouldQuit())
			return kLevelQuit;

		resetExplosions();
		_enemies.clear();

		_rebelPhaseState &= 0x0e;
		_rebelWaveState &= 0x0e;

		_rebelHandler = 8;

		// The phase-3 budget carries into the second half; no new roll.

		debugC(DEBUG_INSANE, "Level 11 Phase 3 second half - playing 11P03_G.SAN budget=%d", budget);
		if (!playLevelSegment("LEV11/P3/11P03_G.SAN", 0x28))
			return kLevelQuit;

		if ((_rebelPhaseState & 0x0e) < 0x0e) {
			int variantIdx = 0;
			WaveEndResult waveEnd = processWaveEnd(0x0e, &budget, 0, 0);

			while (!waveEnd.shouldStop()) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				if (variantIdx < 4) {
					variantIdx = _vm->_rnd.getRandomNumber(8);
				} else {
					variantIdx = _vm->_rnd.getRandomNumber(2);
				}

				const char *filename;
				switch (variantIdx) {
				case 0:  filename = "LEV11/P3/11P03_G.SAN"; break;
				case 1:  filename = "LEV11/P3/11P03_H.SAN"; break;
				case 2:  filename = "LEV11/P3/11P03_I.SAN"; break;
				case 3:  filename = "LEV11/P3/11P03_G.SAN"; break;
				case 4:  filename = "LEV11/P3/11P03_H.SAN"; break;
				case 5:  filename = "LEV11/P3/11P03_I.SAN"; break;
				case 6:  filename = "LEV11/P3/11P03_J.SAN"; break;
				case 7:  filename = "LEV11/P3/11P03_K.SAN"; break;
				default: filename = "LEV11/P3/11P03_L.SAN"; break;
				}

				debugC(DEBUG_INSANE, "Level 11 Phase 3b wave - %s (state=0x%x variantIdx=%d)", filename, _rebelPhaseState, variantIdx);
				if (!playLevelSegment(filename, 0x428))
					return kLevelQuit;

				int16 threshold = (variantIdx > 2) ? 0x14 : 0;
				waveEnd = processWaveEnd(0x0e, &budget, threshold, 0);
			}
		}

		totalKills += _rebelKillCounter;

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(11, 3, "LEV11/11DIE_C.SAN", "LEV11/11RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		{
			totalMisses += _rebelHitCounter;
			int accuracy = calculateAccuracy(totalKills, totalMisses);
			debugC(DEBUG_INSANE, "Level 11 completed! kills=%d misses=%d accuracy=%d%%",
				totalKills, totalMisses, accuracy);
			playLevelEnd(11, accuracy, -1, false);
		}

		_levelUnlocked[11] = true;
		return kLevelNextLevel;
	}

	return kLevelQuit;
}

int InsaneRebel2::runLevel12() {
	playCinematic("LEV12/12CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	playLevelBegin(12);
	if (_vm->shouldQuit())
		return kLevelQuit;

	clearBit(0);

	while (!_vm->shouldQuit()) {
		resetLevelAttemptState(1);

		resetLevelPhaseState(false);

		int16 budget = getWaveBudgetBase(1) + _vm->_rnd.getRandomNumber(2);

		debugC(DEBUG_INSANE, "Level 12 Phase 1 - init 12P05.SAN budget=%d", budget);
		if (!playLevelSegment("LEV12/12P05.SAN", 0x28, false))
			return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		if (!playLevelSegment("LEV12/P1/12P01_A.SAN", 0x428))
			return kLevelQuit;

		{
			WaveEndResult waveEnd = processWaveEnd(6, &budget, 0x14, 0);

			while (!waveEnd.shouldStop()) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				uint16 waveSelect = waveEnd.creditedBits;
				int sel = _vm->_rnd.getRandomNumber(1) | (waveSelect & 2);
				const char *filename;
				switch (sel) {
				case 0:  filename = "LEV12/P1/12P01_C.SAN"; break;
				case 1:  filename = "LEV12/P1/12P01_D.SAN"; break;
				case 2:  filename = "LEV12/P1/12P01_A.SAN"; break;
				default: filename = "LEV12/P1/12P01_B.SAN"; break;
				}

				debugC(DEBUG_INSANE, "Level 12 Phase 1 wave - %s (state=0x%x sel=%d)", filename, _rebelPhaseState, sel);
				if (!playLevelSegment(filename, 0x428))
					return kLevelQuit;

				waveEnd = processWaveEnd(6, &budget, 0x14, 0);
			}
		}

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(12, _currentPhase, "LEV12/12DIE.SAN", "LEV12/12RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		_currentPhase = 2;
		resetLevelWaveState();

		budget = getWaveBudgetBase(2) + _vm->_rnd.getRandomNumber(3);

		debugC(DEBUG_INSANE, "Level 12 Phase 2 - init 12P06.SAN budget=%d", budget);
		if (!playLevelSegment("LEV12/12P06.SAN", 0x428, false))
			return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		if (!playLevelSegment("LEV12/P2/12P02_A.SAN", 0x428))
			return kLevelQuit;

		{
			WaveEndResult waveEnd = processWaveEnd(6, &budget, 0x14, 0);

			while (!waveEnd.shouldStop()) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				uint16 waveSelect = waveEnd.creditedBits;

				int variantIdx;
				if ((waveSelect & 2) == 0) {
					variantIdx = _vm->_rnd.getRandomNumber(2) + 3;
				} else {
					variantIdx = _vm->_rnd.getRandomNumber(2);
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

				debugC(DEBUG_INSANE, "Level 12 Phase 2 wave - %s (state=0x%x variantIdx=%d)", filename, _rebelPhaseState, variantIdx);
				if (!playLevelSegment(filename, 0x428))
					return kLevelQuit;

				int16 threshold = (variantIdx == 2 || variantIdx == 5) ? 0 : 0x14;
				waveEnd = processWaveEnd(6, &budget, threshold, 0);
			}
		}

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(12, _currentPhase, "LEV12/12DIE.SAN", "LEV12/12RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		_currentPhase = 3;
		resetLevelWaveState();

		budget = getWaveBudgetBase(3) + _vm->_rnd.getRandomNumber(3);

		debugC(DEBUG_INSANE, "Level 12 Phase 3 - init 12P07.SAN budget=%d", budget);
		if (!playLevelSegment("LEV12/12P07.SAN", 0x428, false))
			return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		if (!playLevelSegment("LEV12/P3/12P03_A.SAN", 0x428))
			return kLevelQuit;

		{
			int variantIdx = 0;
			WaveEndResult waveEnd = processWaveEnd(6, &budget, 0x14, 0);

			while (!waveEnd.shouldStop()) {
				if (_vm->shouldQuit())
					return kLevelQuit;

				if (variantIdx < 4) {
					variantIdx = _vm->_rnd.getRandomNumber(5);
				} else {
					variantIdx = _vm->_rnd.getRandomNumber(3);
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

				debugC(DEBUG_INSANE, "Level 12 Phase 3 wave - %s (state=0x%x variantIdx=%d)", filename, _rebelPhaseState, variantIdx);
				if (!playLevelSegment(filename, 0x428))
					return kLevelQuit;

				waveEnd = processWaveEnd(6, &budget, 0x14, 0);
			}
		}

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(12, _currentPhase, "LEV12/12DIE.SAN", "LEV12/12RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		_currentPhase = 4;
		resetLevelWaveState();

		budget = getWaveBudgetBase(4) + _vm->_rnd.getRandomNumber(3);

		debugC(DEBUG_INSANE, "Level 12 Phase 4 - init 12P08.SAN budget=%d", budget);
		if (!playLevelSegment("LEV12/12P08.SAN", 0x428, false))
			return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		if (!playLevelSegment("LEV12/P4/12P04_A.SAN", 0x428))
			return kLevelQuit;

		{
			int variantIdx = 0;
			WaveEndResult waveEnd = processWaveEnd(0x0e, &budget, 0x14, 0);

			while (!waveEnd.shouldStop()) {
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

				debugC(DEBUG_INSANE, "Level 12 Phase 4 wave - %s (state=0x%x variantIdx=%d)", filename, _rebelPhaseState, variantIdx);
				if (!playLevelSegment(filename, 0x428))
					return kLevelQuit;

				waveEnd = processWaveEnd(0x0e, &budget, 0x14, 0);
			}
		}

		if (_playerDamage >= 255) {
			int levelResult;
			if (handleLevelDeath(12, _currentPhase, "LEV12/12DIE.SAN", "LEV12/12RETRY.SAN", levelResult))
				continue;
			return levelResult;
		}
		if (_vm->shouldQuit())
			return kLevelQuit;

		if (!playLevelSegment("LEV12/12P09.SAN", 0x428, false))
			return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		{
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			debugC(DEBUG_INSANE, "Level 12 completed! kills=%d misses=%d accuracy=%d%%",
				_rebelKillCounter, _rebelHitCounter, accuracy);
			playLevelEnd(12, accuracy, -1, false);
		}

		_levelUnlocked[12] = true;
		return kLevelNextLevel;
	}

	return kLevelQuit;
}

int InsaneRebel2::runLevel13() {
	playLevelBegin(13);
	if (_vm->shouldQuit())
		return kLevelQuit;

	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;
		resetExplosions();

		clearBit(0);
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		resetShieldGauge();
		_rebelShieldGateActive = true;
		_rebelReactorMode = true;
		if (!playLevelSegment("LEV13/13PLAY_A.SAN", 0x28)) {
			_rebelShieldGateActive = false;
			_rebelReactorMode = false;
			return kLevelQuit;
		}

		// Reactor finale: loop 13PLAY_B until the reactor is destroyed or the player dies.
		// The gauge persists from A (no reset) so hits carry over; finaleGuard caps the loop.
		int finaleGuard = 0;
		while (_playerShield > 0 && !_rebelShieldDestroyed && !_vm->shouldQuit() && finaleGuard < 60) {
			++finaleGuard;
			if (!playLevelSegment("LEV13/13PLAY_B.SAN", 0x468)) {
				_rebelShieldGateActive = false;
				_rebelReactorMode = false;
				return kLevelQuit;
			}
		}
		_rebelShieldGateActive = false;
		_rebelReactorMode = false;

		if (_playerShield > 0 && _rebelShieldDestroyed) {
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			debugC(DEBUG_INSANE, "Level 13 completed! accuracy=%d%%", accuracy);
			playLevelEnd(13, accuracy, _rebelHitCounter, false);
			_levelUnlocked[13] = true;
			return kLevelNextLevel;
		}

		debugC(DEBUG_INSANE, "Level 13 death at frame %d", _deathFrame);
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

int InsaneRebel2::runLevel14() {
	playLevelBegin(14);
	if (_vm->shouldQuit())
		return kLevelQuit;

	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;
		resetExplosions();

		clearBit(0);
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		if (!playLevelSegment("LEV14/14PLAY.SAN", 0x28))
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			debugC(DEBUG_INSANE, "Level 14 completed! accuracy=%d%%", accuracy);
			playLevelEnd(14, accuracy, -1, false);
			_levelUnlocked[14] = true;
			return kLevelNextLevel;
		}

		debugC(DEBUG_INSANE, "Level 14 death at frame %d", _deathFrame);
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

int InsaneRebel2::runLevel15() {
	playCinematic("LEV15/15CUT.SAN");
	if (_vm->shouldQuit())
		return kLevelQuit;

	playLevelBegin(15);
	if (_vm->shouldQuit())
		return kLevelQuit;

	clearBit(0);

	_rebelLevelType = 0xf;

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;
		resetExplosions();

		clearBit(0);
		_rebelKillCounter = 0;
		_rebelHitCounter = 0;

		_rebelLevelType = 0xf;
		_level15SecondHalf = false;

		if (!playLevelSegment("LEV15/15PLAY.SAN", 0x28))
			return kLevelQuit;

		if (_playerShield > 0) {
			int accuracy = calculateAccuracy(_rebelKillCounter, _rebelHitCounter);
			debugC(DEBUG_INSANE, "Level 15 completed! accuracy=%d%%", accuracy);
			_currentPhase = 2;
			_rebelLevelType = 0x10;
			playLevelEnd(15, accuracy, _rebelHitCounter, false);
			_levelUnlocked[15] = true;
			return kLevelNextLevel;
		}

		debugC(DEBUG_INSANE, "Level 15 death at frame %d", _deathFrame);
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
