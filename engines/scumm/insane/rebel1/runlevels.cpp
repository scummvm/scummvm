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

#include "common/config-manager.h"
#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "scumm/file.h"
#include "scumm/scumm_v7.h"
#include "scumm/smush/rebel/anim_ra1.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/rebel1/rebel.h"

namespace Scumm {

static const uint32 kRA1GameplayMouseSettleMs = 1000;

int32 findAnimFrameChunkOffset(ScummEngine_v7 *vm, const char *filename, int32 targetFrame) {
	if (targetFrame <= 0)
		return 0;

	ScummFile *file = vm->instantiateScummFile();
	if (!vm->openFile(*file, Common::Path(filename))) {
		delete file;
		return -1;
	}

	int32 result = -1;
	if (file->size() >= 8) {
		file->readUint32BE();
		const uint32 animSize = file->readUint32BE();
		const int64 animEnd = MIN<int64>((int64)file->pos() + animSize, file->size());

		int32 frameIndex = 0;
		RA1AnimStreamChunkIterator chunks(*file, animEnd);
		RA1AnimChunk chunk;
		while (chunks.next(chunk)) {
			if (chunk.tag == MKTAG('F', 'R', 'M', 'E')) {
				if (frameIndex == targetFrame) {
					result = (int32)chunk.offset;
					break;
				}
				frameIndex++;
			}

			chunks.skip(chunk);
		}
	}

	file->close();
	delete file;
	return result;
}

int32 findAnimFrameChunkOffsetByGameCounter(ScummEngine_v7 *vm, const char *filename, int32 targetCounter, int32 &localFrame) {
	localFrame = 0;
	if (targetCounter <= 0)
		return 0;

	ScummFile *file = vm->instantiateScummFile();
	if (!vm->openFile(*file, Common::Path(filename))) {
		delete file;
		return -1;
	}

	int32 result = -1;
	if (file->size() >= 8) {
		file->readUint32BE();
		const uint32 animSize = file->readUint32BE();
		const int64 animEnd = MIN<int64>((int64)file->pos() + animSize, file->size());

		int32 frameIndex = 0;
		RA1AnimStreamChunkIterator chunks(*file, animEnd);
		RA1AnimChunk chunk;
		while (chunks.next(chunk)) {
			if (chunk.tag == MKTAG('F', 'R', 'M', 'E')) {
				RA1AnimStreamChunkIterator subChunks(*file, chunk.endOffset);
				RA1AnimChunk subChunk;
				while (subChunks.next(subChunk)) {
					if (subChunk.tag == MKTAG('G', 'A', 'M', 'E') && subChunk.size >= 8) {
						const uint32 opcode = file->readUint32BE();
						const int32 counter = (int32)file->readUint32BE();
						if (opcode == 0x0B && counter >= targetCounter) {
							localFrame = frameIndex;
							result = (int32)chunk.offset;
							break;
						}
					}

					subChunks.skip(subChunk);
				}

				if (result >= 0)
					break;

				frameIndex++;
			}

			chunks.skip(chunk);
		}
	}

	file->close();
	delete file;
	return result;
}

static void formatTargetAccuracy(char *dst, size_t dstSize, int kills, int targetCount, bool perfectText) {
	if (perfectText && kills >= targetCount)
		Common::sprintf_s(dst, dstSize, "Target Accuracy: Perfect");
	else
		Common::sprintf_s(dst, dstSize, "Target Accuracy: %d percent", (kills * 100) / targetCount);
}

static int calculateThresholdBonus(int kills, int perfectThreshold, int perKillThreshold, int perKillBonus) {
	int bonus = 0;
	if (kills > perfectThreshold)
		bonus += 1000;
	if (kills > perKillThreshold)
		bonus += (kills - perKillThreshold) * perKillBonus;
	return bonus;
}


void InsaneRebel1::playCinematic(const char *filename, int32 startFrame) {
	debugC(DEBUG_INSANE, "InsaneRebel1::playCinematic('%s', startFrame=%d)", filename, startFrame);
	if (shouldAbortGameFlow())
		return;

	SmushPlayer *splayer = _vm->_splayer;
	_player = splayer;
	restoreScreenFlashPalette();
	_audio.reset();
	splayer->resetAudioTracks();
	applyAudioOptions();
	_interactiveVideoActive = false;
	_vm->_smushVideoShouldFinish = false;
	splayer->setCurVideoFlags(0x420);
	splayer->setFastForwardFromFrame(0);
	splayer->setFastForwardToFrame(startFrame > 0 ? startFrame : 0);
	splayer->play(filename, 15);

	_introTextActive = false;
}

void InsaneRebel1::playChapterCompleteCinematic(const char *filename, int16 unlockedChapter,
		int revealOffsetFromEnd, int stopOffsetFromEnd,
		const char *bonusLabel1, const char *detailText1, int bonusValue1,
		const char *bonusLabel2, const char *detailText2, int bonusValue2,
		int passwordIndex) {
	beginChapterSummaryOverlay(revealOffsetFromEnd, stopOffsetFromEnd,
		bonusLabel1, detailText1, bonusValue1,
		bonusLabel2, detailText2, bonusValue2,
		passwordIndex);
	playCinematic(filename);
	_chapterSummary.active = false;
	_score += _tuning.levelPts + bonusValue1 + bonusValue2;
	if (passwordIndex != 0)
		_maxChapterUnlocked = MAX<int16>(_maxChapterUnlocked, passwordIndex);
}

bool InsaneRebel1::playDeathOrRetry(const char *retryVideo, const char *gameOverVideo) {
	if (_lives > 0) {
		playCinematic(retryVideo);
		if (shouldAbortGameFlow())
			return false;
		_lives--;
		return true;
	}

	playCinematic(gameOverVideo);
	return false;
}

void InsaneRebel1::resetLevelDamageState() {
	_health = kMaxHealth;
	_damageFlags = 0;
	_prevDamageFlags = 0;
	_damageCooldown = 0;
	_deathTimer = 0;
	_screenFlash = 0;
	_screenShakeEnabled = false;
	_deathCauseIndicator = 0;
}

void InsaneRebel1::resetLevelFrameState() {
	_frameCounter = 0;
	_currentSmushFrame = 0;
	_gameCounter = 0;
	_activeGameOpcode = 0;
	_frameGameOpcodeHintMask = 0;
	_gameLatch5D = 0;
	_gameLatch5F = 0;
}

void InsaneRebel1::resetLevelTargetingState(bool resetKillCount) {
	if (resetKillCount)
		_killCount = 0;
	_targetCount = 0;
	_prevTargetCount = 0;
	_lastHitTarget = 0;
}

void InsaneRebel1::resetLevelFlightState(int16 shipDirIndex) {
	_shipPosX = kRA1CenterX;
	_shipPosY = kRA1CenterY;
	_shipDirIndex = shipDirIndex;
	_rollAccum = 0;
	_liftSmooth = 0;
	_posAccumX = 0;
	_posAccumY = 0;
	_perspectiveX = 0;
	_perspectiveY = 0;
}

void InsaneRebel1::resetLevelInputHistory(bool resetAxisDeltaX) {
	memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
	memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
	memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
	memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
	if (resetAxisDeltaX)
		_inputAxisDeltaX = 0;
	_avgInputX = 0;
	_avgInputY = 0;
	resetGamepadReticleAim();
}

void InsaneRebel1::resetLevelAttemptState(int16 flyControlMode, int16 gameplayPhase,
		int16 shipDirIndex, bool resetAxisDeltaX) {
	_flyControlMode = flyControlMode;
	resetLevelDamageState();
	resetLevelFrameState();
	resetGameplayFlagsFromTuning();
	resetLevelTargetingState();
	resetLevelFlightState(shipDirIndex);
	_levelGameplayPhase = gameplayPhase;
	resetLevelInputHistory(resetAxisDeltaX);
}

void InsaneRebel1::playLevelTransitionCutscene(int level) {
	switch (level) {
	case 4:
		// FALCON/BIGGS/WEDGE passcode group.
		playCinematic("CUT1/C1BLOCK.ANM");
		if (shouldAbortGameFlow())
			break;
		playCinematic("CUT1/C1DARTH1.ANM");
		if (shouldAbortGameFlow())
			break;
		playCinematic("CUT1/C1C3PO.ANM");
		if (shouldAbortGameFlow())
			break;
		playCinematic("CUT1/C1DARTH2.ANM");
		break;
	case 7:
		playCinematic("CUT2/C2CUT2.ANM");
		break;
	case 11:
		playCinematic("CUT3/C3BOOM.ANM");
		break;
	default:
		break;
	}
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

void InsaneRebel1::playIntroSequence() {
	playCinematic("OPEN/O1LOGO.ANM");
	if (shouldAbortGameFlow())
		return;

	playCinematic("OPEN/O1OPEN.ANM");
}

bool InsaneRebel1::runLevel1() {
	_currentLevel = 0;
	loadTuningForLevel(0);
	loadLevelSprites(1);

	beginLevelTitleOverlay(0);
	playCinematic("LVL1/L1HANGAR.ANM");
	if (shouldAbortGameFlow())
		return false;

	playCinematic("LVL1/L1CU1.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		bool stage2Started = false;

		resetLevelDamageState();
		resetLevelFrameState();
		resetLevelTargetingState();
		resetLevelFlightState();
		_pathBranchEnabled = true;
		_rightPathSelected = false;
		_flyControlMode = 1;
		loadTuningForLevel(0);

		playInteractiveVideo("LVL1/L1PLAY1L.ANM");
		if (shouldAbortGameFlow())
			return false;

		if (_rightPathSelected && _health >= 0) {
			_pathBranchEnabled = false;
			_flyControlMode = 1;
			playInteractiveVideo("LVL1/L1PLAY1R.ANM", 0x187);
			if (shouldAbortGameFlow())
				return false;
		}
		_pathBranchEnabled = false;

		if (_health >= 0) {
			playCinematic("LVL1/L1CU2.ANM");
			if (shouldAbortGameFlow())
				return false;

			while (!shouldAbortGameFlow()) {
				// Stage 2 uses the 1B tuning row.
				loadTuningForLevel(1);
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
				if (shouldAbortGameFlow())
					return false;

				if (_health < 0)
					break;

				if (_killCount > 4 || _interactiveVideoCheatSkipped) {
					const char *pathText = _rightPathSelected ? "Path Taken: Hard" : "Path Taken: Easy";
					const int pathBonus = _rightPathSelected ? _tuning.bonus * 3 : 0;
					char accuracyText[80];
					formatTargetAccuracy(accuracyText, sizeof(accuracyText), _killCount, 0x0E, true);
					const int targetBonus = calculateThresholdBonus(_killCount, 0x0D, 5, _tuning.bonus);
					playChapterCompleteCinematic("LVL1/L1END.ANM", 1, 0x78, 5,
						"Part I", pathText, pathBonus,
						"Part II", accuracyText, targetBonus);
					return !shouldAbortGameFlow();
				}

				playCinematic("LVL1/L1RETRY.ANM");
				if (shouldAbortGameFlow())
					return false;
			}
		}

		if (stage2Started)
			playCinematic("LVL1/L1CRASHB.ANM");
		else
			playCinematic("LVL1/L1CRASHA.ANM");
		if (shouldAbortGameFlow())
			return false;

		if (!playDeathOrRetry("LVL1/L1NEW.ANM", "LVL1/L1DEATH.ANM"))
			return false;
	}

	return false;
}

bool InsaneRebel1::runLevel2() {
	_currentLevel = 1;
	loadLevelSprites(2);
	loadTuningForLevel(2);

	beginLevelTitleOverlay(1);
	playCinematic("LVL2/L2INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		_flyControlMode = 0;
		resetLevelDamageState();
		resetLevelFrameState();
		resetLevelInputHistory();
		_killCount = 0;

		playInteractiveVideo("LVL2/L2PLAY.ANM");
		if (shouldAbortGameFlow())
			return false;

		if (_health >= 0) {
			char accuracyText[80];
			formatTargetAccuracy(accuracyText, sizeof(accuracyText), _killCount, 0x18, true);
			const int bonus = calculateThresholdBonus(_killCount, 0x17, 0x0C, _tuning.bonus);
			playChapterCompleteCinematic("LVL2/L2END.ANM", 2, 0x69, 10, " ", accuracyText, bonus);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL2/L2NEW.ANM", "LVL2/L2DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel3() {
	_currentLevel = 2;
	loadLevelSprites(3);
	loadTuningForLevel(3);

	beginLevelTitleOverlay(2);
	playCinematic("LVL3/L3INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		_flyControlMode = 1;
		resetLevelDamageState();
		resetLevelFrameState();
		resetGameplayFlagsFromTuning();
		_killCount = 0;
		_levelGameplayPhase = 0;
		resetLevelInputHistory();

		playInteractiveVideo("LVL3/L3PLAY.ANM");
		if (shouldAbortGameFlow())
			return false;

		if (_health >= 0) {
			playChapterCompleteCinematic("LVL3/L3END.ANM", 3, 0x69, 5,
				nullptr, nullptr, 0, nullptr, nullptr, 0, _difficulty + 1);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL3/L3NEW.ANM", "LVL3/L3DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel4() {
	_currentLevel = 3;
	loadLevelSprites(4);
	loadTuningForLevel(4);

	beginLevelTitleOverlay(3);
	playCinematic("LVL4/L4INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		loadTuningForLevel(4);
		_flyControlMode = 1;
		resetLevelDamageState();
		resetLevelFrameState();
		_pendingRouteStartFrame = 0;
		resetGameplayFlagsFromTuning();
		_killCount = 0;
		_levelGameplayPhase = 0;

		_protectedTargetA = 0x39;
		_protectedTargetB = 0x3A;
		_shieldGenHitsA = 0;
		_shieldGenHitsB = 0;
		_levelGameplayPhase = 1;
		bool shieldGeneratorsDestroyed = false;
		bool replayingShieldGeneratorPhase = false;
		while (!shouldAbortGameFlow()) {
			_preserveInteractiveRuntimeState = replayingShieldGeneratorPhase;
			playInteractiveVideo("LVL4/L4PLAY1.ANM");
			_preserveInteractiveRuntimeState = false;
			if (shouldAbortGameFlow())
				return false;

			if (_health < 0)
				break;

			shieldGeneratorsDestroyed = _interactiveVideoCheatSkipped ||
				(_protectedTargetA == 0 && _protectedTargetB == 0 &&
				_shieldGenHitsA > 0x30 + 0x3C && _shieldGenHitsB > 0x30);
			if (shieldGeneratorsDestroyed)
				break;

			debugC(DEBUG_INSANE, "L4 replaying shield-generator phase: hits=(%d,%d) protected=(%d,%d)",
				(int)_shieldGenHitsA, (int)_shieldGenHitsB,
				(int)_protectedTargetA, (int)_protectedTargetB);
			replayingShieldGeneratorPhase = true;
		}
		_protectedTargetA = 0;
		_protectedTargetB = 0;
		if (shouldAbortGameFlow())
			return false;

		if (_health >= 0 && shieldGeneratorsDestroyed) {
			_activeGameOpcode = 0;
			_gameLatch5D = 0;
			_gameLatch5F = 0;
			loadTuningForLevel(5);
			resetGameplayFlagsFromTuning();
			_killCount = 0;
			_levelGameplayPhase = 2;
			playInteractiveVideo("LVL4/L4PLAY2.ANM");
			if (shouldAbortGameFlow())
				return false;
		}

		if (_health >= 0 && shieldGeneratorsDestroyed) {
			const bool torpedoHit = (_killCount != 0);
			playChapterCompleteCinematic(torpedoHit ? "LVL4/L4END1.ANM" : "LVL4/L4END2.ANM",
				4, 0x69, 5, " ", torpedoHit ? "Torpedo Hit" : "Torpedo Missed",
				torpedoHit ? _tuning.bonus : 0);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL4/L4NEW.ANM", "LVL4/L4DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel5() {
	_currentLevel = 4;
	loadLevelSprites(5);
	loadTuningForLevel(6);

	beginLevelTitleOverlay(4);
	playCinematic("LVL5/L5INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		loadRA1Nut("LVL5/L5LASER.NUT", _laserBank);
		loadTuningForLevel(6);
		resetLevelAttemptState(1, 1);
		_level5SuccessFramesRemaining = 0x14;

		playInteractiveVideo("LVL5/L5PLAY.ANM");
		if (shouldAbortGameFlow())
			return false;

		if (_health < 0) {
			if (playDeathOrRetry("LVL5/L5NEW.ANM", "LVL5/L5DEATH.ANM"))
				continue;

			return false;
		}

		if (_killCount <= 2 && !_interactiveVideoCheatSkipped) {
			if (_lives > 0) {
				_lives--;
				playCinematic("LVL5/L5RETRY.ANM");
				if (shouldAbortGameFlow())
					return false;
				continue;
			}

			playCinematic("LVL5/L5DEATH2.ANM");
			return false;
		}

		playCinematic("LVL5/L5BINTRO.ANM");
		if (shouldAbortGameFlow())
			return false;

		loadRA1Nut("LVL5/L5LASER2.NUT", _laserBank);
		loadTuningForLevel(7);
		_activeGameOpcode = 0;
		_gameLatch5D = 0;
		_gameLatch5F = 0;
		_killCount = 0;
		_levelGameplayPhase = 2;
		_level5SuccessFramesRemaining = 0;
		playInteractiveVideo("LVL5/L5PLAY2.ANM");
		if (shouldAbortGameFlow())
			return false;

		if (_health >= 0) {
			char accuracyText[80];
			formatTargetAccuracy(accuracyText, sizeof(accuracyText), _killCount, 100, false);
			int bonus = 0;
			if (_killCount >= 0x1A)
				bonus += (_killCount - 0x19) * _tuning.bonus;
			if (_killCount >= 0x33)
				bonus += (_killCount - 0x32) * _tuning.bonus;
			if (_killCount >= 0x4C)
				bonus += (_killCount - 0x4B) * _tuning.bonus;
			playChapterCompleteCinematic("LVL5/L5END.ANM", 5, 0x69, 5, " ", accuracyText, bonus);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL5/L5NEW.ANM", "LVL5/L5DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel6() {
	_currentLevel = 5;
	loadLevelSprites(6);
	// Chapter 6 uses tuning slot 8, not the chapter-4B slot 5.
	loadTuningForLevel(8);

	beginLevelTitleOverlay(5);
	playCinematic("LVL6/L6INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		_flyControlMode = 1;
		resetLevelDamageState();
		resetLevelFrameState();
		resetGameplayFlagsFromTuning();
		_killCount = 0;
		_levelGameplayPhase = 0;
		resetLevelInputHistory();

		playInteractiveVideo("LVL6/L6PLAY.ANM");
		if (shouldAbortGameFlow())
			return false;

		if (_health >= 0) {
			char accuracyText[80];
			formatTargetAccuracy(accuracyText, sizeof(accuracyText), _killCount, 0x27, true);
			const int bonus = calculateThresholdBonus(_killCount, 0x26, 0x0C, _tuning.bonus);
			playChapterCompleteCinematic("LVL6/L6END.ANM", 6, 0x4B, 5,
				" ", accuracyText, bonus, nullptr, nullptr, 0, _difficulty + 4);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL6/L6NEW.ANM", "LVL6/L6DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel7() {
	const char *const kLevel7Segments[] = {
		"LVL7/L7PLAY1.ANM",
		"LVL7/L7PLAY2.ANM",
		"LVL7/L7PLAY3.ANM",
		"LVL7/L7PLAY4.ANM",
		"LVL7/L7PLAY5.ANM",
		"LVL7/L7PLAY6.ANM"
	};

	_currentLevel = 6;
	loadLevelSprites(7);
	loadRA1Nut("LVL7/L7LASER2.NUT", _enemyLaserBank);
	loadTuningForLevel(9);

	beginLevelTitleOverlay(6);
	playCinematic("LVL7/L7INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		resetLevelAttemptState(3, 0);
		_driftParam = 0x19;
		resetEnemyShotSlots();
		_level7WarningFrames = 0;
		_level7WarningThreshold = 0;

		int route = 0;
		int32 routeStartFrame = 0;
		int32 routeSourceFrame = 0;
		int32 routeVideoStartFrame = 0;
		while (!shouldAbortGameFlow()) {
			_levelRouteIndex = route;
			_pendingRouteIndex = -1;
			_pendingRouteStartFrame = routeSourceFrame;
			_pendingRouteCutoverFrame = -1;
			_pendingRouteVideoStartFrame = routeVideoStartFrame;
			playInteractiveVideo(kLevel7Segments[route], routeStartFrame);
			if (shouldAbortGameFlow())
				return false;

			if (_health < 0)
				break;

			if (_pendingRouteIndex < 0)
				break;

			route = _pendingRouteIndex;
			routeSourceFrame = _pendingRouteStartFrame;
			routeVideoStartFrame = _pendingRouteVideoStartFrame;
			// The ANM-local decision frame gates the route cutover.
			routeStartFrame = 0;
		}

		_levelRouteIndex = -1;
		_pendingRouteIndex = -1;
		_pendingRouteStartFrame = 0;
		_pendingRouteCutoverFrame = -1;
		_pendingRouteVideoStartFrame = 0;
		_level7WarningFrames = 0;
		_level7WarningThreshold = 0;

		if (_health >= 0) {
			char accuracyText[80];
			formatTargetAccuracy(accuracyText, sizeof(accuracyText), _killCount, 0x33, true);
			const int bonus = calculateThresholdBonus(_killCount, 0x32, 0x14, _tuning.bonus);
			playChapterCompleteCinematic("LVL7/L7END.ANM", 7, 0x69, 5, " ", accuracyText, bonus);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL7/L7NEW.ANM", "LVL7/L7DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel8() {
	const char *const kLevel8Routes[] = {
		"LVL8/L8PLAY.ANM",
		"LVL8/L8PLAY2.ANM",
		"LVL8/L8PLAY3.ANM"
	};

	_currentLevel = 7;
	loadLevelSprites(8);
	// The walker uses tuning slot 10.
	loadTuningForLevel(10);

	beginLevelTitleOverlay(7);
	playCinematic("LVL8/L8INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		resetLevelAttemptState(3, 0, 17, true);

		_walkerHealth = 100;
		_walkerTimer = 0;
		_walkerBranchChoice = 0;
		_walkerRoundReplay = false;

		int route = 0;
		int32 routeStartFrame = 0;
		bool replayingRound = false;
		while (!shouldAbortGameFlow()) {
			_levelRouteIndex = route;
			_pendingRouteIndex = -1;
			_pendingRouteStartFrame = routeStartFrame;
			_pendingRouteCutoverFrame = -1;
			_walkerRoundReplay = replayingRound;
			playInteractiveVideo(kLevel8Routes[route], routeStartFrame);
			_walkerRoundReplay = false;
			if (shouldAbortGameFlow())
				return false;

			if (_health < 0)
				break;

			if (_interactiveVideoCheatSkipped) {
				_walkerHealth = 0;
				break;
			}

			if (_walkerHealth <= 0)
				break;

			if (_pendingRouteIndex >= 0 && _pendingRouteIndex != route) {
				// Branch to the next walker route while preserving active state.
				routeStartFrame = _pendingRouteStartFrame;
				route = _pendingRouteIndex;
				replayingRound = false;
				continue;
			}

			// Replay the current route while preserving accumulated walker damage.
			debugC(DEBUG_INSANE, "L8 replaying route=%d walkerHealth=%d killCount=%d",
				route, (int)_walkerHealth, (int)_killCount);
			routeStartFrame = 0;
			_walkerTimer = 0;
			_walkerBranchChoice = 0;
			replayingRound = true;
		}

		_levelRouteIndex = -1;
		_pendingRouteIndex = -1;
		_pendingRouteStartFrame = 0;
		_pendingRouteCutoverFrame = -1;
		_walkerRoundReplay = false;

		if (shouldAbortGameFlow())
			return false;

		if (_health >= 0 && _walkerHealth <= 0) {
			playChapterCompleteCinematic("LVL8/L8END.ANM", 8, 0x5F, 5);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL8/L8NEW.ANM", "LVL8/L8DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel9() {
	uint8 originalRouteSeed = 0;
	auto getOriginalRouteBit = [&originalRouteSeed]() {
		originalRouteSeed = (uint8)(originalRouteSeed * 9 + 0x35);
		return (2 * originalRouteSeed) >> 8;
	};
	const int randPath1 = getOriginalRouteBit();
	const int randPath2 = getOriginalRouteBit();
	const int randPath3 = getOriginalRouteBit();
	auto playLevel9PathSelector = [&](const char *filename) {
		while (!shouldAbortGameFlow()) {
			_onFootCharX = 0;
			_onFootCharY = 0;
			_shipPosX = kRA1CenterX;
			_shipPosY = kRA1CenterY;
			_posAccumX = 0;
			_posAccumY = 0;
			_killCount = 0;
			_lastHitTarget = 0;

			playInteractiveVideo(filename);
			if (shouldAbortGameFlow() || _health < 0)
				return -1;
			if (_interactiveVideoCheatSkipped)
				return (_shipPosX < kRA1CenterX) ? 0 : 1;
			if (_killCount > 0)
				return (_shipPosX < kRA1CenterX) ? 0 : 1;

			debugC(DEBUG_INSANE, "L9 selector '%s' ended without target hit; replaying", filename);
		}
		return -1;
	};

	_currentLevel = 8;
	loadLevelSprites(9);
	// Playable routes alternate tuning selectors 0x0B and 0x0C.
	loadTuningForLevel(0x0B);

	beginLevelTitleOverlay(8);
	playCinematic("LVL9/L9INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		loadTuningForLevel(0x0B);
		resetLevelAttemptState(0, 0, 15);  // On-foot center direction
		_onFootCharX = 0;
		_onFootCharY = 0;
		_onFootAnimCounter = 0;
		_onFootInitialized = false;

		while (!shouldAbortGameFlow()) {
			loadTuningForLevel(0x0B);
			playInteractiveVideo("LVL9/L9PLAY1.ANM");
			if (shouldAbortGameFlow())
				return false;
			if (_health < 0)
				break;

			playCinematic("LVL9/L9CUT1.ANM");
			if (shouldAbortGameFlow())
				return false;

			loadTuningForLevel(0x0C);
			const int side1 = playLevel9PathSelector("LVL9/L9PLAY2.ANM");
			if (shouldAbortGameFlow())
				return false;
			if (_health < 0)
				break;
			if (side1 < 0)
				return false;

			_gameplayFlags75fe |= 4;
			playCinematic(side1 == 0 ? "LVL9/L9PLAY2A.ANM" : "LVL9/L9PLAY2B.ANM");
			if (shouldAbortGameFlow())
				return false;

			if (side1 == randPath1) {
				playCinematic("LVL9/L9CUT2A.ANM");
				if (shouldAbortGameFlow())
					return false;
				continue;
			}

			playCinematic("LVL9/L9CUT2B.ANM");
			if (shouldAbortGameFlow())
				return false;

			loadTuningForLevel(0x0B);
			playInteractiveVideo("LVL9/L9PLAY3A.ANM");
			if (shouldAbortGameFlow())
				return false;
			if (_health < 0)
				break;

			if (_killCount < 15 && !_interactiveVideoCheatSkipped) {
				playInteractiveVideo("LVL9/L9PLAY3B.ANM");
				if (shouldAbortGameFlow())
					return false;
				if (_health < 0)
					break;
			}

			playCinematic("LVL9/L9CUT3.ANM");
			if (shouldAbortGameFlow())
				return false;

			loadTuningForLevel(0x0C);
			const int side2 = playLevel9PathSelector("LVL9/L9PLAY4.ANM");
			if (shouldAbortGameFlow())
				return false;
			if (_health < 0)
				break;
			if (side2 < 0)
				return false;

			_gameplayFlags75fe |= 4;
			playCinematic(side2 == 0 ? "LVL9/L9PLAY4A.ANM" : "LVL9/L9PLAY4B.ANM");
			if (shouldAbortGameFlow())
				return false;

			if (side2 == randPath2) {
				playCinematic(side2 == 0 ? "LVL9/L9CUT4AX.ANM" : "LVL9/L9CUT4B.ANM");
				if (shouldAbortGameFlow())
					return false;

				loadTuningForLevel(0x0B);
				playInteractiveVideo("LVL9/L9PLAY5.ANM");
				if (shouldAbortGameFlow())
					return false;
				if (_health < 0)
					break;

				playCinematic("LVL9/L9CUT5.ANM");
				if (shouldAbortGameFlow())
					return false;

				loadTuningForLevel(0x0C);
				const int side3 = playLevel9PathSelector("LVL9/L9PLAY6.ANM");
				if (shouldAbortGameFlow())
					return false;
				if (_health < 0)
					break;
				if (side3 < 0)
					return false;

				_gameplayFlags75fe |= 4;
				if (side3 == randPath3) {
					playCinematic("LVL9/L9CUT6A.ANM");
					if (shouldAbortGameFlow())
						return false;
					continue;
				}

				playCinematic("LVL9/L9CUT6B.ANM");
				if (shouldAbortGameFlow())
					return false;
			} else {
				playCinematic(side2 == 0 ? "LVL9/L9CUT4A.ANM" : "LVL9/L9CUT4BX.ANM");
				if (shouldAbortGameFlow())
					return false;
			}

			loadTuningForLevel(0x0B);
			playInteractiveVideo("LVL9/L9PLAY7.ANM");
			if (shouldAbortGameFlow())
				return false;
			if (_health < 0)
				break;

			playChapterCompleteCinematic("LVL9/L9END.ANM", 9, 0x69, 5);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL9/L9NEW.ANM", "LVL9/L9DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel10() {
	_currentLevel = 9;
	loadLevelSprites(10);
	loadTuningForLevel(0x0D);

	beginLevelTitleOverlay(9);
	playCinematic("LVL10/L10INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		resetLevelAttemptState(1, 0);

		playInteractiveVideo("LVL10/L10PLAY.ANM");
		if (shouldAbortGameFlow())
			return false;

		if (_health >= 0) {
			char accuracyText[80];
			formatTargetAccuracy(accuracyText, sizeof(accuracyText), _killCount, 0x3E, true);
			const int bonus = calculateThresholdBonus(_killCount, 0x3D, 0x32, _tuning.bonus);
			playChapterCompleteCinematic("LVL10/L10END.ANM", 10, 0x4B, 5,
				" ", accuracyText, bonus, nullptr, nullptr, 0, _difficulty + 7);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL10/L10NEW.ANM", "LVL10/L10DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel11() {
	_currentLevel = 10;
	loadLevelSprites(11);
	// This tuning row has zero roll/slide, which prevents horizontal aiming.
	loadTuningForLevel(0x0E);

	beginLevelTitleOverlay(10);
	playCinematic("LVL11/L11INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		resetLevelAttemptState(1, 0);
		_turretEmitterLeftX = 25;
		_turretEmitterLeftY = 15;

		while (!shouldAbortGameFlow()) {
			playInteractiveVideo("LVL11/L11PLAY.ANM");
			if (shouldAbortGameFlow())
				return false;

			if (_health < 0)
				break;

			if (_killCount > 4 || _interactiveVideoCheatSkipped)
				break;

			playCinematic("LVL11/L11RETRY.ANM");
			if (shouldAbortGameFlow())
				return false;
		}

		if (_health >= 0) {
			char accuracyText[80];
			formatTargetAccuracy(accuracyText, sizeof(accuracyText), _killCount, 0x60, true);
			const int bonus = calculateThresholdBonus(_killCount, 0x5F, 0x0F, _tuning.bonus);
			playChapterCompleteCinematic("LVL11/L11END.ANM", 11, 0x69, 5, " ", accuracyText, bonus);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL11/L11NEW.ANM", "LVL11/L11DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel12() {
	_currentLevel = 11;
	loadLevelSprites(12);
	loadTuningForLevel(0x0F);

	beginLevelTitleOverlay(11);
	playCinematic("LVL12/L12INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		_flyControlMode = 1;
		resetLevelDamageState();
		_killCount = 0;

		while (!shouldAbortGameFlow()) {
			loadTuningForLevel(0x0F);
			resetLevelFrameState();
			_damageFlags = 0;
			resetGameplayFlagsFromTuning();
			resetLevelTargetingState(false);
			resetLevelFlightState();
			_levelGameplayPhase = 0;
			resetLevelInputHistory();

			playInteractiveVideo("LVL12/L12PLAY.ANM");
			if (shouldAbortGameFlow())
				return false;

			if (_levelGameplayPhase == 1 && !_interactiveVideoCheatSkipped) {
				playCinematic("LVL12/L12RETRY.ANM");
				if (shouldAbortGameFlow())
					return false;
				if (_health < 0)
					break;
				continue;
			}

			if (_health < 0)
				break;

			char accuracyText[80];
			formatTargetAccuracy(accuracyText, sizeof(accuracyText), _killCount, 0x58, true);
			const int bonus = calculateThresholdBonus(_killCount, 0x57, 0x46, _tuning.bonus);
			playChapterCompleteCinematic("LVL12/L12END.ANM", 12, 0x69, 5, " ", accuracyText, bonus);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL12/L12NEW.ANM", "LVL12/L12DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel13() {
	_currentLevel = 12;
	loadLevelSprites(13);
	loadRA1Nut("LVL13/L13LASR2.NUT", _enemyLaserBank);
	loadTuningForLevel(0x10);

	beginLevelTitleOverlay(12);
	playCinematic("LVL13/L13INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		resetLevelAttemptState(1, 0);
		resetEnemyShotSlots();

		playInteractiveVideo("LVL13/L13PLAY.ANM");
		if (shouldAbortGameFlow())
			return false;

		if (_health >= 0) {
			char accuracyText[80];
			formatTargetAccuracy(accuracyText, sizeof(accuracyText), _killCount, 0x146, true);
			const int bonus = calculateThresholdBonus(_killCount, 0x145, 0x14, _tuning.bonus);
			playChapterCompleteCinematic("LVL13/L13END.ANM", 13, 0x69, 5, " ", accuracyText, bonus);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL13/L13NEW.ANM", "LVL13/L13DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel14() {
	_currentLevel = 13;
	loadLevelSprites(14);
	loadTuningForLevel(0x11);

	beginLevelTitleOverlay(13);
	playCinematic("LVL14/L14INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		loadTuningForLevel(0x11);
		resetLevelAttemptState(1, 1);
		_level14SuccessFrames = 0;

		bool level14Phase1Complete = false;
		bool replayingLevel14Phase1 = false;
		while (!shouldAbortGameFlow()) {
			_preserveInteractiveRuntimeState = replayingLevel14Phase1;
			playInteractiveVideo("LVL14/L14PLAY.ANM");
			_preserveInteractiveRuntimeState = false;
			if (shouldAbortGameFlow())
				return false;

			if (_health < 0)
				break;

			level14Phase1Complete = _interactiveVideoCheatSkipped || _level14SuccessFrames >= 0x3C;
			if (level14Phase1Complete)
				break;

			debugC(DEBUG_INSANE, "L14 replaying phase 1: successFrames=%d targetsDestroyed=%d",
				(int)_level14SuccessFrames, areLevel14Phase1TargetsDestroyed() ? 1 : 0);
			replayingLevel14Phase1 = true;
		}
		if (shouldAbortGameFlow())
			return false;

		bool level14Phase2Complete = false;
		if (_health >= 0 && level14Phase1Complete) {
			_activeGameOpcode = 0;
			_gameLatch5D = 0;
			_gameLatch5F = 0;
			loadTuningForLevel(0x12);
			resetGameplayFlagsFromTuning();
			_killCount = 0;
			_levelGameplayPhase = 2;
			_level14SuccessFrames = 0;
			_level14Play2BSplicePending = false;
			_level14Play2BSpliced = false;
			_level14Play2BSpliceFrame = 0;

			const char *level14Phase2Video = "LVL14/L14PLAY2.ANM";
			bool replayingLevel14Phase2 = false;
			while (!shouldAbortGameFlow()) {
				_preserveInteractiveRuntimeState = replayingLevel14Phase2;
				playInteractiveVideo(level14Phase2Video);
				_preserveInteractiveRuntimeState = false;
				if (shouldAbortGameFlow())
					return false;

				if (_health < 0)
					break;

				if (_interactiveVideoCheatSkipped) {
					level14Phase2Complete = true;
					break;
				}

				if (_level14Play2BSplicePending) {
					const int32 spliceFrame = _level14Play2BSpliceFrame;
					_level14Play2BSplicePending = false;
					level14Phase2Video = "LVL14/L14PLY2B.ANM";

					playInteractiveVideo(level14Phase2Video, spliceFrame);
					if (shouldAbortGameFlow())
						return false;
					if (_health < 0)
						break;
				}

				level14Phase2Complete = _interactiveVideoCheatSkipped || _level14SuccessFrames >= 0x3C;
				if (level14Phase2Complete)
					break;

				debugC(DEBUG_INSANE, "L14 replaying phase 2: video=%s successFrames=%d targetsDestroyed=%d",
					level14Phase2Video, (int)_level14SuccessFrames,
					areLevel14Phase2TargetsDestroyed() ? 1 : 0);
				replayingLevel14Phase2 = true;
			}
			if (shouldAbortGameFlow())
				return false;
		}

		if (_health >= 0 && level14Phase2Complete) {
			playChapterCompleteCinematic("LVL14/L14END.ANM", 14, 0x69, 5,
				nullptr, nullptr, 0, nullptr, nullptr, 0, _difficulty + 10);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL14/L14NEW.ANM", "LVL14/L14DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

bool InsaneRebel1::runLevel15() {
	_currentLevel = 14;
	loadLevelSprites(15);
	loadTuningForLevel(0x13);

	beginLevelTitleOverlay(14);
	playCinematic("LVL15/L15INTRO.ANM");
	if (shouldAbortGameFlow())
		return false;

	while (!shouldAbortGameFlow()) {
		loadTuningForLevel(0x13);
		resetLevelAttemptState(1, 0);

		_levelGameplayPhase = 1;
		playInteractiveVideo("LVL15/L15PLAY1.ANM");
		if (shouldAbortGameFlow())
			return false;

		if (_health >= 0) {
			playCinematic("LVL15/L15INTR2.ANM");
			if (shouldAbortGameFlow())
				return false;

			_activeGameOpcode = 0;
			_gameLatch5D = 0;
			_gameLatch5F = 0;
			loadTuningForLevel(0x14);
			resetGameplayFlagsFromTuning();
			_killCount = 0;
			_torpedoFired = false;
			_levelGameplayPhase = 2;

			playInteractiveVideo("LVL15/L15PLAY2.ANM");
			if (shouldAbortGameFlow())
				return false;
		}

		if (_health >= 0 && !_torpedoFired && !_interactiveVideoCheatSkipped) {
			debugC(DEBUG_INSANE, "InsaneRebel1::runLevel15: Level 15 torpedo run ended without exhaust-port hit");
			return false;
		}

		if (_health >= 0) {
			int targetBonus = 0;
			if (_killCount > 0x57)
				targetBonus = 1000;
			if (_killCount > 0x14)
				targetBonus += (_killCount - 0x14) * _tuning.bonus;

			char accuracyText[80];
			formatTargetAccuracy(accuracyText, sizeof(accuracyText), _killCount, 0x58, false);
			playChapterCompleteCinematic("LVL15/L15END1.ANM", 15, 0x122, 0xA5,
				"Part I", accuracyText, targetBonus,
				"Part II", "Torpedo on mark", 10000, _difficulty + 13);
			return !shouldAbortGameFlow();
		}

		if (playDeathOrRetry("LVL15/L15NEW.ANM", "LVL15/L15DEATH.ANM"))
			continue;

		return false;
	}

	return false;
}

void InsaneRebel1::runGame() {
	typedef bool (InsaneRebel1::*RunLevelMethod)();
	const RunLevelMethod kLevelRunners[] = {
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
	const int numLevels = (int)(sizeof(kLevelRunners) / sizeof(kLevelRunners[0]));
	auto runLevelsFrom = [&](int startLevel, bool resetRunState) {
		int firstLevel = CLIP<int>(startLevel, 1, numLevels);

		while (!_vm->shouldQuit()) {
			_loadRequested = false;
			bool completed = true;
			int lastCompletedLevel = 0;

			_health = kMaxHealth;
			if (resetRunState) {
				_lives = 3;
				_score = 0;
				_prevScore = 0;
			}
			resetRunState = false;

			for (int level = firstLevel;
				 level <= numLevels && completed && !shouldAbortGameFlow();
				 ++level) {
				_resumeLevel = level;
				playLevelTransitionCutscene(level);
				if (shouldAbortGameFlow())
					break;

				completed = (this->*kLevelRunners[level - 1])();
				if (completed) {
					lastCompletedLevel = level;
					if (level < numLevels) {
						_startLevel = level + 1;
						_resumeLevel = _startLevel;
						autosaveProgress();
					}
				}
			}

			if (_loadRequested) {
				firstLevel = getCurrentSaveLevel();
				continue;
			}

			if (!shouldAbortGameFlow() && completed && lastCompletedLevel == numLevels)
				playCinematic("FIN/FNFINAL.ANM");
			if (!shouldAbortGameFlow())
				runHighScoreNameEntry();
			_currentLevel = 0;
			return;
		}
	};

	bool loadedStartupSave = false;
	if (ConfMan.hasKey("save_slot")) {
		const int saveSlot = ConfMan.getInt("save_slot");
		if (loadGameState(saveSlot, true).getCode() == Common::kNoError) {
			loadedStartupSave = true;
			runLevelsFrom(_resumeLevel, false);
		}
	}

	if (!loadedStartupSave) {
		playIntroSequence();
		if (shouldAbortGameFlow())
			return;
	}

	while (!_vm->shouldQuit()) {
		int menuResult = runMainMenu();
		if (_loadRequested) {
			runLevelsFrom(_resumeLevel, false);
			continue;
		}
		if (_vm->shouldQuit())
			return;

		switch (menuResult) {
		case 1: {
			runLevelsFrom(_startLevel, true);
			break;
		}
		case 2:
			runOptionsMenu();
			break;
		case 3: {
			const int passcodeLevel = runPasscodeEntryDialog();
			if (passcodeLevel >= 1 && passcodeLevel <= numLevels)
				runLevelsFrom(passcodeLevel, true);
			else if (passcodeLevel == numLevels + 1) {
				_health = kMaxHealth;
				_lives = 3;
				_score = 0;
				_prevScore = 0;
				playCinematic("FIN/FNFINAL.ANM");
				if (!shouldAbortGameFlow())
					runHighScoreNameEntry();
				_currentLevel = 0;
			}
			break;
		}
		case 4: {
			int selectedLevel = runLevelSelectMenu();
			if (selectedLevel >= 1 && selectedLevel <= numLevels)
				runLevelsFrom(selectedLevel, true);
			break;
		}
		case 5:
			showHighScores();
			if (!shouldAbortGameFlow())
				playCinematic("OPEN/O1OPEN.ANM");
			break;
		case 6:
			return;
		default:
			break;
		}
	}
}

void InsaneRebel1::resetInteractiveVideoAudio() {
	const int sampleRate = _audio.sampleRate();
	terminateAudio();
	initAudio(sampleRate);
}

void InsaneRebel1::preserveInteractiveVideoAudioState() {
	SmushPlayer *splayer = _vm->_splayer;

	_restoreInteractiveVideoAudioState = false;
	_savedInteractiveVideoTrackCount = 0;
	if (!splayer)
		return;

	_savedInteractiveVideoTrackCount = MIN<int>(splayer->_smushNumTracks, SMUSH_MAX_TRACKS);
	for (int i = 0; i < _savedInteractiveVideoTrackCount; i++) {
		_savedInteractiveVideoTrackState[i] = splayer->_smushTracks[i].state;
		_savedInteractiveVideoTrackGroupId[i] = splayer->_smushTracks[i].groupId;
	}

	_restoreInteractiveVideoAudioState = true;
}

void InsaneRebel1::restoreInteractiveVideoAudioState() {
	if (!_restoreInteractiveVideoAudioState)
		return;

	_restoreInteractiveVideoAudioState = false;
	if (_vm->shouldQuit() || _vm->_saveLoadFlag)
		return;

	SmushPlayer *splayer = _vm->_splayer;
	if (!splayer)
		return;

	const int trackCount = MIN<int>(_savedInteractiveVideoTrackCount, splayer->_smushNumTracks);
	for (int i = 0; i < trackCount; i++) {
		splayer->_smushTracks[i].state = _savedInteractiveVideoTrackState[i];
		splayer->_smushTracks[i].groupId = _savedInteractiveVideoTrackGroupId[i];
	}
}

void InsaneRebel1::setupInteractiveVideoState(int32 startFrame) {
	const bool level7RouteSplice = (_currentLevel == 6 && _levelRouteIndex > 0);
	const bool resumingRoute = startFrame > 0;
	const bool preserveRuntimeState = _preserveInteractiveRuntimeState || resumingRoute || level7RouteSplice;
	const bool preserveVideoState = !_preserveInteractiveRuntimeState && resumingRoute && !level7RouteSplice;

	SmushPlayer *splayer = _vm->_splayer;
	_player = splayer;
	applyAudioOptions();
	restoreScreenFlashPalette();
	if (!preserveRuntimeState)
		clearBit(0);
	_interactiveVideoActive = true;
	if (!preserveRuntimeState) {
		_onFootInitialized = false;
		resetFrameObjectState();
		resetGamepadReticleAim();
	}
	_vm->_smushVideoShouldFinish = false;
	splayer->setPreserveVideoStateOnNextPlay(preserveVideoState);
	splayer->setCurVideoFlags(0x28);
	splayer->setFastForwardFromFrame(0);
	splayer->setFastForwardToFrame(0);
}

void InsaneRebel1::resolveSeek(const char *filename, int32 startFrame, int32 &videoOffset, int32 &videoStartFrame) {
	const bool level7RouteSplice = (_currentLevel == 6 && _levelRouteIndex > 0);
	const bool resumingRoute = startFrame > 0;

	videoStartFrame = 0;
	videoOffset = 0;

	if (_currentLevel == 6 && level7RouteSplice) {
		// Advance the destination route by the previous clip tail already displayed.
		videoStartFrame = (_pendingRouteVideoStartFrame > 0) ?
			_pendingRouteVideoStartFrame : 1;
		videoOffset = findAnimFrameChunkOffset(_vm, filename, videoStartFrame);
		if (videoOffset < 0) {
			debugC(DEBUG_INSANE, "L7 route switch: route=%d destinationFrame=%d offset lookup failed",
				_levelRouteIndex, (int)videoStartFrame);
			videoStartFrame = 0;
			videoOffset = 0;
		} else {
			debugC(DEBUG_INSANE, "L7 route switch: route=%d decisionLocalFrame=%d opens destination at localFrame=%d offset=0x%x",
				_levelRouteIndex, (int)_pendingRouteStartFrame,
				(int)videoStartFrame, (unsigned)videoOffset);
		}
	} else if (_currentLevel == 7 && resumingRoute) {
		videoOffset = findAnimFrameChunkOffsetByGameCounter(_vm, filename, startFrame, videoStartFrame);
		if (videoOffset < 0) {
			debugC(DEBUG_INSANE, "L8 resume: route=%d timelineFrame=%d GAME counter lookup failed",
				_levelRouteIndex, (int)startFrame);
			videoStartFrame = startFrame;
			videoOffset = findAnimFrameChunkOffset(_vm, filename, videoStartFrame);
		}
		if (videoOffset < 0) {
			debugC(DEBUG_INSANE, "L8 resume: route=%d timelineFrame=%d localFrame=%d offset lookup failed",
				_levelRouteIndex, (int)startFrame, (int)videoStartFrame);
			videoStartFrame = 0;
			videoOffset = 0;
		} else {
			debugC(DEBUG_INSANE, "L8 resume: route=%d timelineFrame=%d -> localFrame=%d offset=0x%x",
				_levelRouteIndex, (int)startFrame, (int)videoStartFrame, (unsigned)videoOffset);
		}
	} else if (_currentLevel == 13 && resumingRoute) {
		// L14PLY2B is already the continuation clip. Preserve state, but do not seek.
		debugC(DEBUG_INSANE, "L14 splice: L14PLAY2 timelineFrame=%d -> L14PLY2B frame 0",
			(int)startFrame);
	}
}

void InsaneRebel1::captureInteractiveVideoInput() {
	const bool level7RouteSplice = (_currentLevel == 6 && _levelRouteIndex > 0);
	const bool preserveInputState = _preserveInteractiveRuntimeState || level7RouteSplice;

	enableIOSGamepadController();

	// Center mouse, hide system cursor, and lock mouse to window.
	if (!preserveInputState) {
		_gameplayMouseSettleUntil = 0;
		// Touch input uses absolute aiming, so avoid synthetic mouse motion there.
		if (!isTouchscreenActive()) {
			warpGameplayMouseNow(kRA1CenterX, kRA1CenterY);
			_gameplayMouseSettleUntil = _vm->_system->getMillis() + kRA1GameplayMouseSettleMs;
		}
	} else {
		_gameplayMouseSettleUntil = 0;
	}
	CursorMan.showMouse(false);
	if (!isTouchscreenActive())
		g_system->lockMouse(true);

	debugC(DEBUG_INSANE, "centerGameplayAim: mouse=(%d,%d) joystick=(%d,%d) gamepadAim=%d settleUntil=%u preserve=%d",
		_vm->_mouse.x, _vm->_mouse.y, _joystickAxisX, _joystickAxisY,
		_gamepadAimActive ? 1 : 0, _gameplayMouseSettleUntil,
		preserveInputState ? 1 : 0);
}

void InsaneRebel1::releaseInteractiveVideoInput() {
	_gameplayMouseSettleUntil = 0;
	g_system->lockMouse(false);
	restoreIOSGamepadController();
}

void InsaneRebel1::playInteractiveVideoFile(const char *filename, int32 videoOffset, int32 videoStartFrame) {
	_vm->_splayer->play(filename, 15, videoOffset, videoStartFrame);
	restoreScreenFlashPalette();
	restoreInteractiveVideoAudioState();
	_interactiveVideoActive = false;
}

void InsaneRebel1::playInteractiveVideo(const char *filename, int32 startFrame) {
	debugC(DEBUG_INSANE, "InsaneRebel1::playInteractiveVideo('%s', startFrame=%d)", filename, startFrame);
	if (shouldAbortGameFlow()) {
		_preserveInteractiveRuntimeState = false;
		_interactiveVideoCheatSkipped = false;
		return;
	}

	_interactiveVideoCheatSkipped = false;
	int32 videoStartFrame = 0;
	int32 videoOffset = 0;
	const bool preserveRuntimeState = _preserveInteractiveRuntimeState ||
		(startFrame > 0) || (_currentLevel == 6 && _levelRouteIndex > 0);

	if (!preserveRuntimeState)
		resetInteractiveVideoAudio();
	setupInteractiveVideoState(startFrame);
	resolveSeek(filename, startFrame, videoOffset, videoStartFrame);
	captureInteractiveVideoInput();
	playInteractiveVideoFile(filename, videoOffset, videoStartFrame);
	releaseInteractiveVideoInput();
	_preserveInteractiveRuntimeState = false;
}

} // End of namespace Scumm
