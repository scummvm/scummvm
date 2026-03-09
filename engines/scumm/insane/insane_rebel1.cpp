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
#include "common/events.h"
#include "common/endian.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "scumm/scumm_v7.h"
#include "scumm/scumm.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

// Per-difficulty tuning tables from assault_data_3.bin
// Indexed: difficulty * 0x28B + level * 0x1F + offset
// Fields: roll, lift, slide, drift, snap, miss, wham, shot, kill
static const int16 kTuningTable[10][3][9] = {
	// Level 1 (Flight Training)
	{
		{ 100, 100,  60, 110,   0,   0,  15,   0,   0 },  // Easy
		{ 100, 105,  60, 115,   0,   0,  25,   0,   0 },  // Normal
		{ 105, 110,  65, 120,   0,   0,  30,   0,   0 },  // Hard
	},
	// Level 2 (Asteroid Field Training)
	{
		{ 100,  16, 120,   0,   7,   0,  15,   0,  25 },  // Easy
		{ 100,  18, 120,   0,   5,   0,  20,   0,  50 },  // Normal
		{ 100,  20, 150,   0,   1,   0,  25,   0,  75 },  // Hard
	},
	// Level 3 (Planet Kolaador)
	{
		{   0,   0,   0,   0,   4,  15,  25,   0,  25 },  // Easy
		{   0,   0,   0,   0,   2,  18,  30,   0,  50 },  // Normal
		{   0,   0,   0,   0,   0,  20,  35,   0,  75 },  // Hard
	},
	// Level 4 (Star Destroyer Attack)
	{
		{  70, 100, 150,  90,   0,   0,  20,   0,   0 },  // Easy
		{  72, 105, 155, 105,   0,   0,  25,   0,   0 },  // Normal
		{  75, 110, 160, 110,   0,   0,  28,   0,   0 },  // Hard
	},
	// Level 5 (Tatooine Attack)
	{
		{   0,   0,   0,   0,   2,  11,   0,   4,  25 },  // Easy
		{   0,   0,   0,   0,   1,  25,   0,   6,  50 },  // Normal
		{   0,   0,   0,   0,   1,  28,   0,   6,  75 },  // Hard
	},
	// Level 6 (Asteroid Field Chase)
	{
		{   0,   0,   0,   0,   3,  20,   0,   2,  50 },  // Easy
		{   0,   0,   0,   0,   1,  25,   0,   5, 100 },  // Normal
		{   0,   0,   0,   0,   1,  28,   0,   6, 200 },  // Hard
	},
	// Level 7 (Imperial Probe Droids)
	{
		{  70, 150,  50,  25,  10,   0,  20,   0,  25 },  // Easy
		{  72, 165, 155,  30,   8,   0,  30,   0,  50 },  // Normal
		{ 110, 190,  55,  65,   3,   0,  33,   0,  75 },  // Hard
	},
	// Level 8 (Imperial Walkers)
	{
		{   0,   0,   0,   0,   5,   0,   0,   2,  25 },  // Easy
		{   0,   0,   0,   0,   3,   0,   0,   5,  50 },  // Normal
		{   0,   0,   0,   0,   1,   0,   0,   6,  75 },  // Hard
	},
	// Level 9 (Stormtroopers)
	{
		{   0,   0,   0,   0,   2,  20,  20,   0,  25 },  // Easy
		{   0,   0,   0,   0,   1,  25,  30,   0,  50 },  // Normal
		{   0,   0,   0,   0,   0,  28,  33,   0,  75 },  // Hard
	},
	// Level 10 (Protect Rebel Transport)
	{
		{ 100, 150, 150,  25,   7,   0,  12,   2,  50 },  // Easy
		{ 100, 160, 200,  35,   4,   0,  30,   4, 100 },  // Normal
		{ 100, 180, 250,  50,   3,   0,  33,   5, 100 },  // Hard
	},
};
static const int kNumTunedLevels = 10;


void InsaneRebel1::loadTuningForLevel(int level) {
	int d = CLIP(_difficulty, 0, 2);
	int l = CLIP(level, 0, kNumTunedLevels - 1);
	_tuning.roll  = kTuningTable[l][d][0];
	_tuning.lift  = kTuningTable[l][d][1];
	_tuning.slide = kTuningTable[l][d][2];
	_tuning.drift = kTuningTable[l][d][3];
	_tuning.snap  = kTuningTable[l][d][4];
	_tuning.miss  = kTuningTable[l][d][5];
	_tuning.wham  = kTuningTable[l][d][6];
	_tuning.shot  = kTuningTable[l][d][7];
	_tuning.kill  = kTuningTable[l][d][8];
	debug(1, "RA1: Loaded tuning level=%d diff=%d: roll=%d lift=%d slide=%d snap=%d miss=%d wham=%d shot=%d kill=%d",
		level, d, _tuning.roll, _tuning.lift, _tuning.slide, _tuning.snap, _tuning.miss,
		_tuning.wham, _tuning.shot, _tuning.kill);
}

InsaneRebel1::InsaneRebel1(ScummEngine_v7 *scumm) : Insane(), _vm(scumm) {
	_screenWidth = 384;
	_screenHeight = 242;

	_shipPosX = kRA1CenterX;
	_shipPosY = kRA1CenterY;
	_shipDirIndex = 17;  // Center of 5x7 grid (2*7 + 3)

	_corridorLeftX = kRA1MinX;
	_corridorTopY = kRA1MinY;
	_corridorRightX = kRA1MaxX;
	_corridorBottomY = kRA1MaxY;

	_rollAccum = 0;
	_liftSmooth = 0;
	_posAccumX = 0;
	_posAccumY = 0;
	_driftParam = 0;

	_difficulty = 0;  // Easy by default
	loadTuningForLevel(0);

	_perspectiveX = 0;
	_perspectiveY = 0;
	_projectionCurveExtent = 1;
	memset(_projectionTable, 0, sizeof(_projectionTable));

	memset(_inputHistoryX, 0, sizeof(_inputHistoryX));
	memset(_inputHistoryY, 0, sizeof(_inputHistoryY));
	memset(_viewHistoryX, 0, sizeof(_viewHistoryX));
	memset(_viewHistoryY, 0, sizeof(_viewHistoryY));
	_avgInputX = 0;
	_avgInputY = 0;
	_mouseOffsetX = 0;
	_mouseOffsetY = 0;
	_mouseBiasX = 0;
	_mouseBiasY = 0;
	_mousePrevBiasX = 0;
	_mousePrevBiasY = 0;
	_mouseBiasLatch = false;
	_mouseRecentering = false;

	_currentLevel = 0;
	_flyControlMode = 0;
	_turretEmitterLeftX = 0;
	_turretEmitterLeftY = 0;
	_turretEmitterRightX = 0;
	_turretEmitterRightY = 0;
	_activeGameOpcode = 0;
	_frameGameOpcodeMask = 0;
	_frameDispatchFlags = 0;

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
	_interactiveVideoActive = false;
	_gameCounter = 0;
	_pathBranchEnabled = false;
	_rightPathSelected = false;
	_levelRouteIndex = -1;
	_pendingRouteIndex = -1;
	_menuActive = false;
	_menuConfirmed = false;
	_menuSelection = 0;
	_menuFrameCounter = 0;
	_optionsActive = false;
	_optionsSel = 0;
	_levelSelectActive = false;
	_levelSelectSel = 0;
	_startLevel = 1;
	_turbulenceEnabled = false;

	// Shooting/targeting state
	_playerFired = false;
	_fireCooldown = 0;
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
	resetFrameObjectState();

	if (loadRA1Nut("SYS/TALKFONT.NUT", _hudFontBank)) {
		debug(1, "InsaneRebel1: HUD/menu glyph font loaded from SYS/TALKFONT.NUT (%d chars)", _hudFontBank.numSprites);
	} else if (loadRA1Nut("SYS/TECHFONT.NUT", _hudFontBank)) {
		debug(1, "InsaneRebel1: HUD/menu glyph font loaded from SYS/TECHFONT.NUT (%d chars)", _hudFontBank.numSprites);
	} else {
		warning("InsaneRebel1: failed to load RA1 HUD font bank (TECHFONT/TALKFONT)");
	}

	if (loadRA1Nut("SYS/TITLFONT.NUT", _titleFontBank)) {
		debug(1, "InsaneRebel1: title glyph font loaded from SYS/TITLFONT.NUT (%d chars)", _titleFontBank.numSprites);
	} else if (loadRA1Nut("SYS/TALKFONT.NUT", _titleFontBank)) {
		debug(1, "InsaneRebel1: title glyph font fallback loaded from SYS/TALKFONT.NUT (%d chars)", _titleFontBank.numSprites);
	} else {
		warning("InsaneRebel1: failed to load title font bank (TITLFONT/TALKFONT)");
	}

	// FUN_1CB22 uses "<<" layer markers that resolve to TECHFONT in the original.
	// Keep a dedicated TECH font bank for targeting markers/lock indicators.
	if (loadRA1Nut("SYS/TECHFONT.NUT", _techFontBank)) {
		debug(1, "InsaneRebel1: targeting glyph font loaded from SYS/TECHFONT.NUT (%d chars)", _techFontBank.numSprites);
	} else if (loadRA1Nut("SYS/TALKFONT.NUT", _techFontBank)) {
		debug(1, "InsaneRebel1: targeting glyph font fallback loaded from SYS/TALKFONT.NUT (%d chars)", _techFontBank.numSprites);
	} else {
		warning("InsaneRebel1: failed to load targeting font bank (TECHFONT/TALKFONT)");
	}

	// Audio
	initAudio(11025);

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

InsaneRebel1::~InsaneRebel1() {
	_vm->_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	terminateAudio();
}

} // End of namespace Scumm
