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
#include "common/events.h"
#include "common/endian.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "scumm/scumm_v7.h"
#include "scumm/scumm.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/rebel1/rebel.h"

namespace Scumm {

// Per-difficulty tuning tables from assault_data_3.bin (also loadable from C:\rebltune.txt)
// 21 sub-levels x 3 difficulties x 13 fields
// Fields: roll, lift, slide, drift, snap, miss, wham, shot, kill, time, levelPts, bonus, flags
const int16 kTuningTable[21][3][13] = {
	// Sub-level 0: "1A" (Flight Training part 1)
	{
		{ 100, 100,  60, 110,   0,   0,  15,   0,   0,   5,  500,  100, 2048 },  // Easy
		{ 100, 105,  60, 115,   0,   0,  25,   0,   0,   5, 1000,  200, 2048 },  // Normal
		{ 105, 110,  65, 120,   0,   0,  30,   0,   0,  10, 1500,  500, 2050 },  // Hard
	},
	// Sub-level 1: "1B" (Flight Training part 2)
	{
		{ 100,  16, 120,   0,   7,   0,  15,   0,  25,   5,  500,  100, 3072 },  // Easy
		{ 100,  18, 120,   0,   5,   0,  20,   0,  50,   5, 1000,  200, 3072 },  // Normal
		{ 100,  20, 150,   0,   1,   0,  25,   0,  75,  10, 1500,  500, 3074 },  // Hard
	},
	// Sub-level 2: "2" (Asteroid Field Training)
	{
		{   0,   0,   0,   0,   4,  15,  25,   0,  25,  10,  500,  100, 2048 },  // Easy
		{   0,   0,   0,   0,   2,  18,  30,   0,  50,  10, 1000,  200, 2048 },  // Normal
		{   0,   0,   0,   0,   0,  20,  35,   0,  75,  10, 1500,  500, 2050 },  // Hard
	},
	// Sub-level 3: "3" (Planet Kolaador)
	{
		{  70, 100, 150,  90,   0,   0,  20,   0,   0,   5, 1000,  100, 2048 },  // Easy
		{  72, 105, 155, 105,   0,   0,  25,   0,   0,   5, 2000,  200, 2048 },  // Normal
		{  75, 110, 160, 110,   0,   0,  28,   0,   0,  10, 3000,  500, 2050 },  // Hard
	},
	// Sub-level 4: "4A" (Star Destroyer Attack part 1)
	{
		{   0,   0,   0,   0,   2,  11,   0,   4,  25,   5,  500,  750, 2048 },  // Easy
		{   0,   0,   0,   0,   1,  25,   0,   6,  50,   5, 1000, 1500, 2048 },  // Normal
		{   0,   0,   0,   0,   1,  28,   0,   6,  75,  10, 1500, 2000, 2050 },  // Hard
	},
	// Sub-level 5: "4B" (Star Destroyer Attack part 2)
	{
		{   0,   0,   0,   0,   3,  20,   0,   2,  50,   5,  500,  750, 2064 },  // Easy
		{   0,   0,   0,   0,   1,  25,   0,   5, 100,   5, 1000, 1500, 2064 },  // Normal
		{   0,   0,   0,   0,   1,  28,   0,   6, 200,  10, 1500, 2000, 2064 },  // Hard
	},
	// Sub-level 6: "5A" (Tatooine Attack part 1)
	{
		{  70, 150,  50,  25,  10,   0,  20,   0,  25,   5,  500,   15, 3072 },  // Easy
		{  72, 165, 155,  30,   8,   0,  30,   0,  50,   5, 1000,   30, 3072 },  // Normal
		{ 110, 190,  55,  65,   3,   0,  33,   0,  75,  10, 1500,   75, 3074 },  // Hard
	},
	// Sub-level 7: "5B" (Tatooine Attack part 2)
	{
		{   0,   0,   0,   0,   5,   0,   0,   2,  25,   0,  500,   15, 2048 },  // Easy
		{   0,   0,   0,   0,   3,   0,   0,   5,  50,   5, 1000,   30, 2048 },  // Normal
		{   0,   0,   0,   0,   1,   0,   0,   6,  75,  10, 1500,   75, 2050 },  // Hard
	},
	// Sub-level 8: "6" (Asteroid Field Chase)
	{
		{   0,   0,   0,   0,   2,  20,  20,   0,  25,   5,  500,  100, 2048 },  // Easy
		{   0,   0,   0,   0,   1,  25,  30,   0,  50,   5, 1000,  200, 2048 },  // Normal
		{   0,   0,   0,   0,   0,  28,  33,   0,  75,  10, 1500,  500, 2050 },  // Hard
	},
	// Sub-level 9: "7" (Imperial Probe Droids)
	{
		{ 100, 150, 150,  25,   7,   0,  12,   2,  50,   5,  500,  100, 3072 },  // Easy
		{ 100, 160, 200,  35,   4,   0,  30,   4, 100,   5, 1000,  200, 3072 },  // Normal
		{ 100, 180, 250,  50,   3,   0,  33,   5, 100,  10, 1500,  500, 3074 },  // Hard
	},
	// Sub-level 10: "8" (Imperial Walkers)
	{
		{   0,   0,   0,   0,   0,   0,  30,   0,  25,   0, 1000,  100, 3074 },  // Easy
		{   0,   0,   0,   0,   0,   0,  36,   0,  50,   0, 2000,  200, 3074 },  // Normal
		{   0,   0,   0,   0,   0,   0,  39,   0,  75,   0, 3000,  500, 3074 },  // Hard
	},
	// Sub-level 11: "9A" (Stormtroopers part 1)
	{
		{   0,   0,   0,   0,   4,   0,   0,  15,  25,   0, 1000,  100, 3074 },  // Easy
		{   0,   0,   0,   0,   2,   0,   0,  25,  50,   0, 2000,  200, 3078 },  // Normal
		{   0,   0,   0,   0,   0,   0,   0,  30,  75,   0, 3000,  500, 3078 },  // Hard
	},
	// Sub-level 12: "9B" (Stormtroopers part 2)
	{
		{   0,   0,   0,   0,   0,   0,   0,  15,  25,   0, 1000,  100, 3098 },  // Easy
		{   0,   0,   0,   0,   0,   0,   0,  25,  50,   0, 2000,  200, 3098 },  // Normal
		{   0,   0,   0,   0,   0,   0,   0,  30,  75,   0, 3000,  500, 3098 },  // Hard
	},
	// Sub-level 13: "10" (Protect Rebel Transport)
	{
		{   0,   0,   0,   0,   3,  10,   0,   5,  25,   5,  500,  200, 2048 },  // Easy
		{   0,   0,   0,   0,   1,  16,   0,   5,  50,   5, 1000,  400, 2048 },  // Normal
		{   0,   0,   0,   0,   0,  18,   0,   7,  75,  10, 1500, 1000, 2050 },  // Hard
	},
	// Sub-level 14: "11" (Yavin Training)
	{
		{  70, 150, 150,  25,  12,   0,  30,   0,  50,   5,  500,  200, 3072 },  // Easy
		{  72, 165, 155,  30,   7,   0,  36,   0,  50,   5, 1000,  400, 3072 },  // Normal
		{  75, 170, 160,  33,   3,   0,  39,   0,  75,  10, 1500, 1000, 3074 },  // Hard
	},
	// Sub-level 15: "12" (TIE Attack)
	{
		{   0,   0,   0,   0,   4,  13,   0,   5,  25,   5,  500,  100, 2048 },  // Easy
		{   0,   0,   0,   0,   2,  20,   0,   5,  50,   5, 1000,  200, 2048 },  // Normal
		{   0,   0,   0,   0,   0,  23,   0,   5,  75,  10, 1500,  500, 2050 },  // Hard
	},
	// Sub-level 16: "13" (Death Star Surface)
	{
		{ 100,  16, 120,   0,  20,   0,  35,   8,  75,   5,  500,  100, 3072 },  // Easy
		{ 100,  18, 120,   0,  18,   0,  36,  10, 100,   5, 1000,  200, 3072 },  // Normal
		{ 100,  20, 150,   0,  15,   0,  39,  12, 200,  10, 1500,  500, 3074 },  // Hard
	},
	// Sub-level 17: "14A" (Surface Cannon part 1)
	{
		{   0,   0,   0,   0,   0,  20,  35,   8,  25,   0, 1000,  100, 2048 },  // Easy
		{   0,   0,   0,   0,   0,  27,  36,  12,  50,   0, 2000,  200, 2048 },  // Normal
		{   0,   0,   0,   0,   0,  28,  39,  12,  75,   0, 3000,  500, 2050 },  // Hard
	},
	// Sub-level 18: "14B" (Surface Cannon part 2)
	{
		{   0,   0,   0,   0,  10,  20,  35,   8,  25,   0, 1000,  100, 2048 },  // Easy
		{   0,   0,   0,   0,   5,  25,  36,  10,  50,   0, 2000,  200, 2048 },  // Normal
		{   0,   0,   0,   0,   4,  28,  39,  12,  75,   0, 3000,  500, 2050 },  // Hard
	},
	// Sub-level 19: "15A" (Death Star trench final part 1)
	{
		{   0,   0,   0,   0,   4,   0,  28,   3,  25,   5,  500,  100, 2048 },  // Easy
		{   0,   0,   0,   0,   3,   0,  36,   3,  50,   5, 1000,  200, 2048 },  // Normal
		{   0,   0,   0,   0,   3,   0,  39,   4,  75,  10, 1500,  500, 2050 },  // Hard
	},
	// Sub-level 20: "15B" (Death Star trench final part 2)
	{
		{   0,   0,   0,   0,   4,  10,  30,   3,  25,   5,  500,  100, 2048 },  // Easy
		{   0,   0,   0,   0,   3,  20,  34,   3,  50,   5, 1000,  200, 2048 },  // Normal
		{   0,   0,   0,   0,   2,  22,  35,   4,  75,  10, 1500,  500, 2050 },  // Hard
	},
};
const int kNumTunedLevels = 21;


void InsaneRebel1::loadTuningForLevel(int level) {
	int d = CLIP(_difficulty, 0, 2);
	int l = CLIP(level, 0, kNumTunedLevels - 1);
	_tuning.roll     = kTuningTable[l][d][0];
	_tuning.lift     = kTuningTable[l][d][1];
	_tuning.slide    = kTuningTable[l][d][2];
	_tuning.drift    = kTuningTable[l][d][3];
	_tuning.snap     = kTuningTable[l][d][4];
	_tuning.miss     = kTuningTable[l][d][5];
	_tuning.wham     = kTuningTable[l][d][6];
	_tuning.shot     = kTuningTable[l][d][7];
	_tuning.kill     = kTuningTable[l][d][8];
	_tuning.time     = kTuningTable[l][d][9];
	_tuning.levelPts = kTuningTable[l][d][10];
	_tuning.bonus    = kTuningTable[l][d][11];
	_tuning.flags    = kTuningTable[l][d][12];
	// initLevelFromTuning (0x13E7B) writes the 16-bit tuning flags word across
	// 0x75FE/0x75FF, so we must preserve both bytes.
	resetGameplayFlagsFromTuning();
	_protectedTargetA = 0;
	_protectedTargetB = 0;

	debugC(DEBUG_INSANE, "Loaded tuning level=%d diff=%d: roll=%d lift=%d slide=%d drift=%d snap=%d "
		"miss=%d wham=%d shot=%d kill=%d time=%d levelPts=%d bonus=%d flags=0x%x",
		level, d, _tuning.roll, _tuning.lift, _tuning.slide, _tuning.drift, _tuning.snap,
		_tuning.miss, _tuning.wham, _tuning.shot, _tuning.kill,
		_tuning.time, _tuning.levelPts, _tuning.bonus, _tuning.flags);
}

bool InsaneRebel1::isTouchscreenActive() const {
	return g_system->hasFeature(OSystem::kFeatureTouchscreen);
}

void InsaneRebel1::resetGameplayFlagsFromTuning() {
	const uint16 tuningFlags = (uint16)_tuning.flags;
	_gameplayFlags75fe = tuningFlags & 0x00FF;
	_gameplayFlags75ff = tuningFlags >> 8;
}

InsaneRebel1::InsaneRebel1(ScummEngine_v7 *scumm) : Insane(), _vm(scumm) {
	Insane::_vm = scumm;

	_screenWidth = 384;
	_screenHeight = 242;

	_shipPosX = kRA1CenterX;
	_shipPosY = kRA1CenterY;
	_flightAimX = kRA1CenterX;
	_flightAimY = kRA1CenterY;
	_shipDirIndex = 17;  // Center of 5x7 grid (2*7 + 3)

	_corridorLeftX = kRA1MinX;
	_corridorTopY = kRA1MinY;
	_corridorRightX = kRA1MaxX;
	_corridorBottomY = kRA1MaxY;

	_rollAccum = 0;
	_liftSmooth = 0;
	_posAccumX = 0;
	_posAccumY = 0;
	_turretFrameShipOffsetX = 0;
	_turretFrameShipOffsetY = 0;
	_turretFrameShipCenterX = kRA1CenterX;
	_turretFrameShipCenterY = kRA1CenterY;
	_turretFrameShipCenterValid = false;
	_driftParam = 0;

	// Start new games on the least punishing original tuning by default.
	_difficulty = 0;
	loadTuningForLevel(0);

	_perspectiveX = 0;
	_perspectiveY = 0;
	_projectionCurveExtent = 1;
	_onFootCharX = 0;
	_onFootCharY = 0;
	_onFootAnimCounter = 0;
	_onFootInitialized = false;
	memset(_projectionTable, 0, sizeof(_projectionTable));

	memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
	memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
	memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
	memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
	_inputAxisDeltaX = 0;
	_avgInputX = 0;
	_avgInputY = 0;
	_joystickAxisX = 0;
	_joystickAxisY = 0;
	_lastJoystickAxisEventTime = 0;
	_level2JoystickFilteredX = 0;
	_level2JoystickFilteredY = 0;
	_gamepadAimAxisX = 0;
	_gamepadAimAxisY = 0;
	_gamepadAimActive = false;
	_gameplayMouseSettleUntil = 0;
	_activeInputSource = kInputSourceMouse;

	_currentLevel = 0;
	_resumeLevel = 1;
	_activeSaveSlot = -1;
	_loadRequested = false;
	_flyControlMode = 0;
	_turretEmitterLeftX = 0;
	_turretEmitterLeftY = 0;
	_turretEmitterRightX = 0;
	_turretEmitterRightY = 0;
	_activeGameOpcode = 0;
	_frameGameOpcodeHintMask = 0;
	_frameGameOpcodeMask = 0;
	_frameHasGameChunk = false;
	_frameDispatchFlags = 0;
	_gameOp0BPhysicsUpdatedThisFrame = false;
	_gameOp0BOverlayRenderedThisFrame = false;

	_health = kMaxHealth;
	_lives = 3;
	_score = 0;
	_prevScore = 0;
	_damageFlags = 0;
	_prevDamageFlags = 0;
	_gameLatch5D = 0;
	_gameLatch5F = 0;
	_damageCooldown = 0;
	_deathTimer = 0;
	_screenFlash = 0;
	_frameCounter = 0;
	_currentSmushFrame = 0;
	_screenShakeEnabled = false;
	memset(_screenFlashBasePalette, 0, sizeof(_screenFlashBasePalette));
	_screenFlashBasePaletteValid = false;
	_deathCauseIndicator = 0;
	_hudRenderFlag = 0;
	_hudDirtyFlag = 0;
	_maxChapterUnlocked = 0;
	_unlockAllLevels = ConfMan.getBool("rebel1_unlock_all");
	_noDamage = ConfMan.getBool("rebel1_no_damage");
	_interactiveVideoActive = false;
	_preserveInteractiveRuntimeState = false;
	_interactiveVideoCheatSkipped = false;
	_restoreInteractiveVideoAudioState = false;
	memset(_savedInteractiveVideoTrackState, 0, sizeof(_savedInteractiveVideoTrackState));
	memset(_savedInteractiveVideoTrackGroupId, 0, sizeof(_savedInteractiveVideoTrackGroupId));
	_savedInteractiveVideoTrackCount = 0;
	_gameCounter = 0;
	_pathBranchEnabled = false;
	_rightPathSelected = false;
	_levelRouteIndex = -1;
	_pendingRouteIndex = -1;
	_pendingRouteStartFrame = 0;
	_pendingRouteCutoverFrame = -1;
	_pendingRouteVideoStartFrame = 0;
	_level7WarningFrames = 0;
	_level7WarningThreshold = 0;
	_levelGameplayPhase = 0;
	_level14Play2BSplicePending = false;
	_level14Play2BSpliced = false;
	_level14Play2BSpliceFrame = 0;
	_level5SuccessFramesRemaining = 0;
	_level14SuccessFrames = 0;
	_menuActive = false;
	_introTextActive = false;
	_introTextStartFrame = 0;
	_introTextEndFrame = 0;
	_introTextLevel = 0;
	memset(&_chapterSummary, 0, sizeof(_chapterSummary));
	_menuConfirmed = false;
	_menuSelection = 0;
	_menuFrameCounter = 0;
	_optionsActive = false;
	_optionsSel = 0;
	_levelSelectActive = false;
	_levelSelectSel = 0;
	_startLevel = 1;

	// Options: read initial state from the mixer.
	_optRookieOneFemale = false;
	_optMusicEnabled = !_vm->_mixer->isSoundTypeMuted(Audio::Mixer::kMusicSoundType);
	_optSfxEnabled = !_vm->_mixer->isSoundTypeMuted(Audio::Mixer::kSFXSoundType) &&
		!_vm->_mixer->isSoundTypeMuted(Audio::Mixer::kSpeechSoundType);
	// Initialize the dialogue-text (subtitles) toggle from the global setting so the
	// game and the in-game DIALOGUE TEXT menu label reflect it. The menu toggle writes the
	// same "subtitles" key, and ra1HandleText() gates rendering on it.
	_optTextEnabled = ConfMan.getBool("subtitles");
	_optControlsYFlip = false;
	_optRapidFire = true;
	_optVolume = _vm->_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) * 127 / Audio::Mixer::kMaxChannelVolume;

	// Default high scores — from DS:0x1D0/0x298/0x2C0
	const struct { const char *name; int32 score; byte difficulty; } kDefaultScores[kHighScoreCount] = {
		{"Vince",   10000, 2}, {"Tamlynn",  9000, 2}, {"Chip",    8000, 2},
		{"Brett",    7000, 1}, {"Casey",    6000, 1}, {"Justin",  5000, 1},
		{"Bill",     4000, 0}, {"Aaron",    3000, 0}, {"Mary",    2000, 0},
		{"Ron",      1000, 0}
	};
	for (int i = 0; i < kHighScoreCount; i++) {
		Common::sprintf_s(_highScores[i].name, "<%s", kDefaultScores[i].name);
		_highScores[i].score = kDefaultScores[i].score;
		_highScores[i].difficulty = kDefaultScores[i].difficulty;
	}
	_highScoresActive = false;
	_textEntryActive = false;
	_textEntryPasscodeMode = false;
	_textEntryDone = false;
	_textEntryCanceled = false;
	_virtualKeyboardActive = false;
	_textEntryPickerIndex = 0;
	_textEntryPickerOffsetX = 0;
	_textEntryMaxChars = 0;
	_highScoreEntryIndex = -1;
	memset(_textEntryBuffer, 0, sizeof(_textEntryBuffer));

	// Shooting/targeting state
	_playerFired = false;
	_playerSecondaryHeld = false;
	_fireCooldown = 0;
	_rapidFirePhase = 0;
	_gameplayFlags75fe = 0;
	_gameplayFlags75ff = 0;
	memset(_shotSlots, 0, sizeof(_shotSlots));
	_shotAlternator = 0;
	_shotSideToggle = false;
	_targetProximity = 0;
	_prevTargetProx = 0;
	_targetAnimCounter = 0;
	_targetCount = 0;
	_prevTargetCount = 0;
	memset(_targetBoxX, 0, sizeof(_targetBoxX));
	memset(_targetBoxY, 0, sizeof(_targetBoxY));
	memset(_targetBoxVariant, 0, sizeof(_targetBoxVariant));
	memset(_gostSlots, 0, sizeof(_gostSlots));
	_gostSlotIdx = 0;
	_killCount = 0;
	_lastHitTarget = 0;
	_frameObjectHitRevealPending = false;
	resetEnemyShotSlots();
	_protectedTargetA = 0;
	_protectedTargetB = 0;
	_shieldGenHitsA = 0;
	_shieldGenHitsB = 0;
	_torpedoFired = false;
	_walkerHealth = 100;
	_walkerTimer = 0;
	_walkerBranchChoice = 0;
	_walkerRoundReplay = false;
	resetFrameObjectState();

	if (loadRA1Nut("SYS/TALKFONT.NUT", _hudFontBank)) {
		debugC(DEBUG_INSANE, "InsaneRebel1::InsaneRebel1(): HUD/menu glyph font loaded from SYS/TALKFONT.NUT (%d chars)", _hudFontBank.numSprites);
	} else if (loadRA1Nut("SYS/TECHFONT.NUT", _hudFontBank)) {
		debugC(DEBUG_INSANE, "InsaneRebel1::InsaneRebel1(): HUD/menu glyph font loaded from SYS/TECHFONT.NUT (%d chars)", _hudFontBank.numSprites);
	} else {
		warning("InsaneRebel1::InsaneRebel1(): failed to load RA1 HUD font bank (TECHFONT/TALKFONT)");
	}

	if (loadRA1Nut("SYS/TITLFONT.NUT", _titleFontBank)) {
		debugC(DEBUG_INSANE, "InsaneRebel1::InsaneRebel1(): title glyph font loaded from SYS/TITLFONT.NUT (%d chars)", _titleFontBank.numSprites);
	} else if (loadRA1Nut("SYS/TALKFONT.NUT", _titleFontBank)) {
		debugC(DEBUG_INSANE, "InsaneRebel1::InsaneRebel1(): title glyph font fallback loaded from SYS/TALKFONT.NUT (%d chars)", _titleFontBank.numSprites);
	} else {
		warning("InsaneRebel1::InsaneRebel1(): failed to load title font bank (TITLFONT/TALKFONT)");
	}

	// FUN_1CB22 uses "<<" layer markers that resolve to TECHFONT in the original.
	// Keep a dedicated TECH font bank for targeting markers/lock indicators.
	if (loadRA1Nut("SYS/TECHFONT.NUT", _techFontBank)) {
		debugC(DEBUG_INSANE, "InsaneRebel1::InsaneRebel1(): targeting glyph font loaded from SYS/TECHFONT.NUT (%d chars)", _techFontBank.numSprites);
	} else if (loadRA1Nut("SYS/TALKFONT.NUT", _techFontBank)) {
		debugC(DEBUG_INSANE, "InsaneRebel1::InsaneRebel1(): targeting glyph font fallback loaded from SYS/TALKFONT.NUT (%d chars)", _techFontBank.numSprites);
	} else {
		warning("InsaneRebel1::InsaneRebel1(): failed to load targeting font bank (TECHFONT/TALKFONT)");
	}

	// Audio
	initAudio(11025);
	memset(_sfxData, 0, sizeof(_sfxData));
	memset(_sfxSize, 0, sizeof(_sfxSize));
	loadSfx();

	// Null out Insane base class pointers that the default constructor doesn't initialize
	_smush_roadrashRip = nullptr;
	_smush_roadrsh2Rip = nullptr;
	_smush_roadrsh3Rip = nullptr;
	_smush_goglpaltRip = nullptr;
	_smush_tovista1Flu = nullptr;
	_smush_tovista2Flu = nullptr;
	_smush_toranchFlu = nullptr;
	_smush_minedrivFlu = nullptr;
	_smush_minefiteFlu = nullptr;
	_smush_bensgoggNut = nullptr;
	_smush_bencutNut = nullptr;
	_smush_iconsNut = nullptr;
	_smush_icons2Nut = nullptr;

	_vm->_system->getEventManager()->getEventDispatcher()->registerObserver(this, 1, false);
}

void InsaneRebel1::setCurrentSmushFrame(int32 frame) {
	_currentSmushFrame = frame;
}

void InsaneRebel1::warpGameplayMouseNow(int x, int y) {
	Common::EventManager *eventMan = _vm->_system->getEventManager();
	if (eventMan)
		eventMan->purgeMouseEvents();

	_vm->_mouse.x = x;
	_vm->_mouse.y = y;
	_vm->_system->warpMouse(_vm->_macScreen ? x * 2 : x,
		_vm->_macScreen ? y * 2 + 2 * _vm->_macScreenDrawOffset : y);

	if (eventMan)
		eventMan->purgeMouseEvents();
}

void InsaneRebel1::enableIOSGamepadController() {
	_iosGamepadControllerState.enable();
}

void InsaneRebel1::restoreIOSGamepadController() {
	_iosGamepadControllerState.restore();
}

InsaneRebel1::~InsaneRebel1() {
	restoreIOSGamepadController();
	_vm->_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	terminateAudio();
	freeSfx();
}

} // End of namespace Scumm
