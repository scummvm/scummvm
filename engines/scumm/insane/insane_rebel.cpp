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



#include "engines/engine.h"
#include "common/system.h"
#include "common/memstream.h"
#include "common/events.h"
#include "common/util.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "scumm/actor.h"
#include "scumm/file.h"
#include "scumm/resource.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"

#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse_engine.h"

#include "scumm/smush/smush_player.h"
#include "scumm/smush/smush_font.h"

#include "scumm/insane/insane_rebel.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Scumm {

// External codec functions from codec1.cpp
extern void smushDecodeRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
extern void smushDecodeRLEOpaque(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);
extern void smushDecodeUncompressed(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

InsaneRebel2::InsaneRebel2(ScummEngine_v7 *scumm) {
	_vm = scumm;

	// Initialize parent class pointers to nullptr to avoid crash in ~Insane()
	// because Insane() default constructor leaves them uninitialized.
	_smush_roadrashRip = nullptr;
	_smush_roadrsh2Rip = nullptr;
	_smush_roadrsh3Rip = nullptr;
	_smush_goglpaltRip = nullptr;
	_smush_tovista1Flu = nullptr;
	_smush_tovista2Flu = nullptr;
	_smush_toranchFlu = nullptr;
	_smush_minedrivFlu = nullptr;
	_smush_minefiteFlu = nullptr;
	_smush_bencutNut = nullptr;
	_smush_bensgoggNut = nullptr;

	// Rebel Assault 2 specific initialization can go here

	// Rebel Assault 2: Load cockpit sprites NUT which contains crosshairs, explosions, status bar
	// CPITIMAG.NUT = low-res (320x200), CPITIMHI.NUT = high-res (640x480)
	// For now, use CPITIMAG since the game runs at 320x200
	_smush_iconsNut = new NutRenderer(_vm, "SYSTM/CPITIMAG.NUT");
	_smush_icons2Nut = nullptr;  // Not used for Rebel2

	// Initialize laser texture buffer (DAT_0047fee4) from sprite 5 of CPITIMAG.NUT
	// This is done by FUN_0040BAB0/FUN_0040BB87 in the original with sprite index 5
	// Sprite 5 is 136x13 pixels - a wide, thin texture perfect for laser beams
	_laserTexture.pixels = nullptr;
	_laserTexture.width = 0;
	_laserTexture.height = 0;
	if (_smush_iconsNut && _smush_iconsNut->getNumChars() > 5) {
		initLaserTexture(_smush_iconsNut, 5);
	}
	_smush_cockpitNut = new NutRenderer(_vm, "SYSTM/DISPFONT.NUT");

	// Load SMALFONT.NUT for HUD score/lives rendering (DAT_00482200 equivalent)
	// This is used by FUN_0041c012 to render the score in the status bar
	_smush_dispfontNut = new NutRenderer(_vm, "SYSTM/SMALFONT.NUT");

	// Load DIHIFONT.NUT for in-video messages/subtitles (Opcode 9)
	_rebelMsgFont = new SmushFont(_vm, "SYSTM/DIHIFONT.NUT", true);

	// Load menu system fonts (from info.md - FUN_403BD0 lines 302-348)
	// In low resolution mode, fonts are loaded as a linked list:
	//   Font 0 (^f00): TALKFONT.NUT - Default menu font (DAT_00485058)
	//   Font 1 (^f01): SMALFONT.NUT - Small font for format code switching
	//   Font 2 (^f02): TITLFONT.NUT - Title font
	//   Font 3 (^f03): POVFONT.NUT - POV font (not loaded here)
	_smush_talkfontNut = new NutRenderer(_vm, "SYSTM/TALKFONT.NUT");
	_smush_smalfontNut = new NutRenderer(_vm, "SYSTM/SMALFONT.NUT");
	_smush_titlefontNut = new NutRenderer(_vm, "SYSTM/TITLFONT.NUT");

	// SmushFont for menu text rendering - uses SMALFONT with proper drawString support
	_menuFont = new SmushFont(_vm, "SYSTM/SMALFONT.NUT", true);

	// MSTOVER.NUT - Mouse Over background overlay (NOT a cursor!)
	// This is loaded into DAT_0047aba8 and used as a background overlay via FUN_004236e0
	// The original game uses the standard Windows arrow cursor (IDC_ARROW via LoadCursorA)
	_smush_mouseoverNut = new NutRenderer(_vm, "SYSTM/MSTOVER.NUT");

	_enemies.clear();
	_rebelHandler = 0;  // Not set yet - will be set by IACT opcode 6
	_rebelLevelType = 0;  // Level type from Opcode 6 par3, determines HUD sprite variant
	_rebelStatusBarSprite = 0;  // 0 = disabled, 5 or 53 = enabled (set by IACT opcode 6 par4==1)
	_introCursorPushed = false;  // Cursor state tracking for intro sequences

	_playerDamage = 0;
	_playerShield = 255; // Full shields by default (255)
	_playerLives = 3;
	_playerScore = 0;
	_viewX = 0;
	_viewY = 0;

	// Damage visual effect counters (FUN_420515/420562/420754/42073B)
	_damageFlashCounter = 0;
	_damageHighFlashCounter = 0;
	_damageShakeCounter = 0;
	memset(_damageSavedPalette, 0, sizeof(_damageSavedPalette));

	// Retail globals mapped: hit counter, cooldown, invulnerability flag
	_rebelHitCounter = 0;
	_rebelKillCounter = 0;
	_rebelInvulnerable = false;
	_rebelWaveState = 0;
	_rebelPhaseState = 0;

	// Opcode 6 state variables
	_rebelAutopilot = 0;
	_rebelDamageLevel = 0;
	_rebelFlightDir = 0;
	_rebelControlMode = 0;
	_rebelInputThrottle = 0;
	_rebelViewOffsetX = 0;
	_rebelViewOffsetY = 0;
	_rebelViewOffset2X = 0;
	_rebelViewOffset2Y = 0;
	_rebelViewMode1 = 0;
	_rebelViewMode2 = 0;

	// Initialize mirrored retail counters
	for (int i = 0; i < 10; ++i) {
		_rebelValueCounters[i] = 0;
		_rebelMaskCounters[i] = 0;
	}
	_rebelLastCounter = 0;

	_difficulty = 1; // Default to Medium (1). TODO: Read from game config
	_targetLockTimer = 0;  // DAT_00443676 equivalent

	_speed = 12;
	_insaneIsRunning = false;

	_numberArray = 0;
	_emulateInterrupt = 0;
	_flag1d = 0;
	_objArray1Idx = 0;
	_objArray1Idx2 = 0;
	_objArray2Idx = 0;
	_objArray2Idx2 = 0;
	_currSceneId = 1;
	_timer6Id = 0;
	_timerSpriteId = 0;
	_temp2SceneId = 0;
	_tempSceneId = 0;
	_currEnemy = -1;
	_currScenePropIdx = 0;
	_currScenePropSubIdx = 0;
	_currTrsMsg = 0;
	_sceneData2Loaded = 0;
	_sceneData1Loaded = 0;
	_keyboardDisable = 0;
	_needSceneSwitch = false;
	_idx2Exceeded = 0;
	_tiresRustle = false;
	_keybOldDx = 0;
	_keybOldDy = 0;
	_velocityX = 0;
	_velocityY = 0;
	_keybX = 0;
	_keybY = 0;
	_firstBattle = false;
	_battleScene = true;
	_kickBenProgress = false;
	_weaponBenJustSwitched = false;
	_kickEnemyProgress = false;
	_weaponEnemyJustSwitched = false;
	_beenCheated = 0;
	_posBrokenTruck = 0;
	_posBrokenCar = 0;
	_posFatherTorque = 0;
	_posCave = 0;
	_posVista = 0;
	_roadBranch = false;
	_roadStop = false;
	_carIsBroken = false;
	_benHasGoggles = false;
	_mineCaveIsNear = false;
	_objectDetected = false;
	_approachAnim = -1;
	_val54d = 0;
	_val57d = 0;
	_val115_ = false;
	_roadBumps = false;
	_val211d = 0;
	_val213d = 0;
	_metEnemiesListTail = 0;
	_smlayer_room = 0;
	_smlayer_room2 = 0;
	_isBenCut = 0;
	_continueFrame = 0;
	_continueFrame1 = 0;
	_counter1 = 0;
	_iactSceneId = 0;
	_iactSceneId2 = 0;

	int i, j;
	
	for (i = 0; i < 12; i++)
		_metEnemiesList[i] = 0;

	for (i = 0; i < 9; i++)
		for (j = 0; j < 9; j++)
			_enHdlVar[i][j] = 0;

	for (i = 0; i < 0x200; i++)
		_iactBits[i] = 0;

	for (i = 0; i < 512; i++) {
		_rebelLinks[i][0] = 0;
		_rebelLinks[i][1] = 0;
		_rebelLinks[i][2] = 0;
	}

	for (i = 0; i < 5; i++) {
		_explosions[i].active = false;
		_explosions[i].counter = 0;
	}

	// Initialize collision zone system (for Level 3 pilot ship obstacle avoidance)
	_primaryZoneCount = 0;
	_secondaryZoneCount = 0;
	for (i = 0; i < kMaxCollisionZones; i++) {
		_primaryZones[i].active = false;
		_secondaryZones[i].active = false;
	}
	// Corridor boundaries in game coordinate space (FUN_40C040 lines 21-24)
	// DAT_00443b0a=0, DAT_00443b0c=0, DAT_00443b0e=0x1a8(424), DAT_00443b10=0x104(260)
	_corridorLeftX = 0;
	_corridorTopY = 0;
	_corridorRightX = 0x1A8;   // 424 — full game buffer width
	_corridorBottomY = 0x104;  // 260 — full game buffer height
	_hitCooldown = 0;

	// Initialize legacy shot system (backwards compatibility)
	for (i = 0; i < 2; i++) {
		_shots[i].active = false;
		_shots[i].counter = 0;
	}

	// Initialize Handler 0x26 Turret shot system (FUN_40AD63)
	for (i = 0; i < 2; i++) {
		_turretShots[i].counter = 0;
		_turretShots[i].targetX = 0;
		_turretShots[i].targetY = 0;
		_turretShots[i].seqNum = 0;
		_turretShots[i].gunX = 0;
		_turretShots[i].gunY = 0;
	}
	_turretShotSeqCounter = 0;

	// Initialize Handler 8 Vehicle shot system (FUN_402ED0)
	for (i = 0; i < 2; i++) {
		_vehicleShots[i].counter = 0;
		_vehicleShots[i].targetX = 0;
		_vehicleShots[i].targetY = 0;
	}

	// Initialize Handler 7 Space shot system (FUN_40FADF)
	for (i = 0; i < 2; i++) {
		_spaceShots[i].counter = 0;
		_spaceShots[i].targetX = 0;
		_spaceShots[i].targetY = 0;
		_spaceShots[i].leftGunX = 0;
		_spaceShots[i].leftGunY = 0;
		_spaceShots[i].rightGunX = 0;
		_spaceShots[i].rightGunY = 0;
		_spaceShots[i].variant = 0;
	}
	_spaceShotDirection = 0;

	for (i = 0; i < 16; i++) {
		_rebelEmbeddedHud[i].pixels = nullptr;
		_rebelEmbeddedHud[i].width = 0;
		_rebelEmbeddedHud[i].height = 0;
		_rebelEmbeddedHud[i].renderX = 0;
		_rebelEmbeddedHud[i].renderY = 0;
		_rebelEmbeddedHud[i].valid = false;
	}

	// Initialize Handler 8 ship system
	_shipSprite = nullptr;
	_shipSprite2 = nullptr;
	_shipOverlay1 = nullptr;
	_shipOverlay2 = nullptr;
	_level2Background = nullptr;
	_level2BackgroundLoaded = false;
	_shipPosX = 0xa0;      // Start centered (160 in hex)
	_shipPosY = 0x28;      // Start at vertical center (40)
	_shipTargetX = 0xa0;
	_shipTargetY = 0x28;
	_shipLevelMode = 0;
	_movementRangeLimit = 127;  // DAT_0047e034 - Start at full range (shooting state)
	_flyControlMode = 0;   // DAT_004437c0 - Start in flight-only mode (no shooting)
	_shipFiring = false;
	_prevMouseButtons = 0; // For edge detection in mouse button handling
	_shipDirectionH = 2;   // Start centered horizontally (0-4 range)
	_shipDirectionV = 3;   // Start centered vertically (0-6 range)
	_shipDirectionIndex = 2 * 7 + 3;  // Center = 17

	// Initialize Handler 7 FLY ship system
	_flyShipSprite = nullptr;    // FLY001 - 35 direction frames
	_flyLaserSprite = nullptr;   // FLY002 - laser sprites
	_flyTargetSprite = nullptr;  // FLY003 - targeting overlay
	_flyHiResSprite = nullptr;   // FLY004 - high-res alternative
	_flyShipScreenX = 0xd4;      // Start at center (212) - matches DAT_00443708 default
	_flyShipScreenY = 0x82;      // Start at center (130) - matches DAT_0044370a default
	_smoothedVelocity = 0;       // DAT_0044370c
	_verticalInput = 0;          // DAT_0044370e
	memset(_velocityHistory, 0, sizeof(_velocityHistory));  // DAT_00443716
	memset(_windHistoryX, 0, sizeof(_windHistoryX));         // DAT_00443b16
	memset(_windHistoryY, 0, sizeof(_windHistoryY));         // DAT_00443b34
	_windParamX = 0;             // DAT_00443b12
	_windParamY = 0;             // DAT_00443b14
	_perspectiveX = 0;           // DAT_00443712
	_perspectiveY = 0;           // DAT_00443714
	_viewShift = 0;              // DAT_00443710
	_facingRight = false;        // DAT_0047ab8c

	// Initialize Handler 25 GRD ship system
	_grd001Sprite = nullptr;     // DAT_00482240 - GRD001 primary ship
	_grd002Sprite = nullptr;     // DAT_00482238 - GRD002 secondary ship
	_grdSpriteMode = 0;          // DAT_00457900 - sprite mode (1,2,3,4)

	// Initialize Handler 0x26 turret HUD overlay system
	_hudOverlayNut = nullptr;    // DAT_0047fe78 - Primary HUD overlay (GRD files, animated)
	_hudOverlay2Nut = nullptr;   // DAT_0047fe80 - Secondary HUD overlay

	// Initialize audio system for RA2 (since we don't use iMUSE)
	_audioSampleRate = 11025;  // RA2 audio is 11025 Hz, not 22050 Hz
	for (i = 0; i < kRA2MaxAudioTracks; i++) {
		_audioStreams[i] = nullptr;
		_audioTrackActive[i] = false;
	}

	// Initialize and load sound effects (SYSTM/*.SAD files)
	for (i = 0; i < kRA2NumSfx; i++) {
		_sfxData[i] = nullptr;
		_sfxSize[i] = 0;
	}
	loadSfx();

	// Initialize menu system
	_gameState = kStateMainMenu;  // Start at main menu
	_menuSelection = 0;           // First item selected
	// Main menu has 7 selectable items (0-6) matching GAME.TRS indices 11-17:
	//   0: Start Game, 1: Options, 2: Calibrate Joystick, 3: Continue Intro,
	//   4: Show Top Pilots, 5: Show Credits, 6: Return to Launcher
	// Note: The coordinate formula uses numItemsTotal = 8 (includes title) for Y position calculation
	// Formula from FUN_0041f5ae: (DAT_0047a806 == 0) + 6 = 7 items for keyboard mode
	_menuItemCount = 7;
	_menuInactivityTimer = 0;
	_lastMenuVariant = -1;        // No previous menu video
	_menuRepeatDelay = 0;
	for (i = 0; i < 16; i++) {
		_levelUnlocked[i] = (i == 0);  // Only level 1 unlocked initially
	}

	// Initialize chapter selection system (FUN_00415CF8)
	// 17 items: 16 chapters + BACK option
	_chapterSelection = 0;        // First chapter selected
	_chapterItemCount = 17;       // 16 chapters + BACK
	_selectedChapter = 0;         // Default selected chapter
	_passwordInput = "";          // No password input

	// Debug flag to unlock all chapters for testing
	// Based on original debug mode (DAT_0047ab34 == 'd') from FUN_00415CF8
	// Set to true to bypass normal unlock progression
	_debugUnlockAll = true;  // TODO: Set to false for release, or read from ScummVM config

	for (i = 0; i < 16; i++) {
		// If debug unlock is enabled, unlock all chapters
		// Otherwise only chapter 1 (index 0) is unlocked initially
		// Original: pilotData->scores[chapter] < 0xFF means unlocked
		_chapterUnlocked[i] = _debugUnlockAll || (i == 0);
	}

	// Initialize preview offset for chapter selection (FUN_00425170)
	// X offset: -90 (0xffa6), Y offset: selection * -50 + 75
	_previewOffsetX = -90;
	_previewOffsetY = 75;  // Chapter 0: 0 * -50 + 75 = 75

	// Initialize pilot selection system (FUN_00414A41)
	// Menu structure: [saved pilots] + 4 fixed options (NEW/DUPE/DELETE/MAIN MENU)
	_levelSelection = 0;          // First item selected
	_levelItemCount = 4;          // 0 saved pilots + 4 fixed options
	_selectedLevel = 1;           // Default selected level
	_difficultySelection = 2;     // Default to 3rd difficulty (matching original init param_3=2)

	// Initialize menu input capture system
	_menuInputActive = false;

	// Initialize level state tracking for multi-phase levels
	_currentPhase = 1;
	_deathFrame = 0;
	_phaseScore = 0;
	_phaseMisses = 0;

	// Register as EventObserver to capture input events before ScummEngine consumes them
	_vm->_system->getEventManager()->getEventDispatcher()->registerObserver(this, 1, false);
}


InsaneRebel2::~InsaneRebel2() {
	// Unregister EventObserver
	_vm->_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	terminateAudio();
	freeSfx();
	delete _rebelMsgFont;
	delete _menuFont;
	delete _smush_dispfontNut;
	delete _smush_talkfontNut;
	delete _smush_smalfontNut;
	delete _smush_titlefontNut;
	delete _smush_mouseoverNut;

	// Clean up Handler 8 ship sprites
	delete _shipSprite;
	delete _shipSprite2;
	delete _shipOverlay1;
	delete _shipOverlay2;
	free(_level2Background);
	_level2Background = nullptr;

	// Clean up Handler 7 FLY ship sprites
	delete _flyShipSprite;
	delete _flyLaserSprite;
	delete _flyTargetSprite;
	delete _flyHiResSprite;

	// Clean up Handler 25 GRD ship sprites
	delete _grd001Sprite;
	delete _grd002Sprite;

	// Clean up Handler 0x26 turret HUD overlays
	delete _hudOverlayNut;
	delete _hudOverlay2Nut;

	// Clean up laser texture buffer (DAT_0047fee4)
	freeLaserTexture();

	// Clean up embedded HUD overlays
	for (int i = 0; i < 16; i++) {
		free(_rebelEmbeddedHud[i].pixels);
		_rebelEmbeddedHud[i].pixels = nullptr;
	}
}

bool InsaneRebel2::notifyEvent(const Common::Event &event) {
	// Handle global key events (ESC to skip, SPACE to pause)
	// These work regardless of menu state
	if (event.type == Common::EVENT_KEYDOWN) {
		SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

		switch (event.kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
			// ESC handling depends on game state:
			// - In menus: Select quit option and confirm
			// - During gameplay/cutscenes: Skip video
			if (splayer) {
				if (_menuInputActive && (_gameState == kStateMainMenu ||
				                          _gameState == kStatePilotSelect ||
				                          _gameState == kStateDifficultySelect ||
				                          _gameState == kStateChapterSelect)) {
					// In menu mode: Select quit option and confirm selection
					// This emulates the assembly behavior from FUN_0041f5ae
					_menuSelection = _menuItemCount - 1;  // Select last item (quit/back)
					debug("Rebel2: ESC pressed in menu - selecting quit (item %d)", _menuSelection);
				} else {
					debug("Rebel2: ESC pressed - skipping video");
				}
				_vm->_smushVideoShouldFinish = true;
				return true;  // Consume the event
			}
			break;

		case Common::KEYCODE_SPACE:
			// SPACE toggles pause (emulates FUN_405A21 pause handling)
			// Only allow pausing during gameplay, not in menus
			if (splayer && _gameState == kStateGameplay) {
				if (splayer->_paused) {
					debug("Rebel2: SPACE pressed - unpausing");
					splayer->unpause();
				} else {
					debug("Rebel2: SPACE pressed - pausing");
					splayer->pause();
					// Show the pause overlay with dimming effect and "PAUSED" text
					showPauseOverlay();
				}
				return true;  // Consume the event
			}
			break;

		default:
			break;
		}
	}

	// Capture menu-related input events when menu input is active.
	// This is called before ScummEngine::parseEvents() consumes events,
	// so we can reliably capture keyboard/mouse input for menu navigation.
	if (!_menuInputActive)
		return false;  // Not capturing, let normal processing occur

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		// Queue these events for processing in processMenuInput()
		_menuEventQueue.push(event);
		break;
	default:
		break;
	}

	// Return false to allow ScummEngine to also process the event
	// (needed for quit handling, etc.)
	return false;
}

// Score lookup tables (from DAT_0047e0fe, DAT_0047e100, DAT_0047e102)
// These are indexed by: DAT_0047a7fa * 0x242 + DAT_0047a7f8 * 0x22
// For simplicity, we use fixed values based on difficulty level
// Values estimated from disassembly patterns (actual values would need extraction from game data)
const int16 InsaneRebel2::kScoreTableEnemyDestroy[16] = {
	100, 100, 100, 100,   // Easy (difficulty 0)
	150, 150, 150, 150,   // Medium (difficulty 1)
	200, 200, 200, 200,   // Hard (difficulty 2)
	250, 250, 250, 250    // Expert (difficulty 3)
};

const int16 InsaneRebel2::kScoreTableSpecial[16] = {
	50, 50, 50, 50,
	75, 75, 75, 75,
	100, 100, 100, 100,
	125, 125, 125, 125
};

const int16 InsaneRebel2::kScoreTableTimeBonus[16] = {
	1, 1, 1, 1,
	2, 2, 2, 2,
	3, 3, 3, 3,
	4, 4, 4, 4
};

// Score system implementation (FUN_0041bf8d equivalent)
// Adds points to score and awards bonus life when crossing threshold
void InsaneRebel2::addScore(int points) {
	// Calculate bonus life threshold based on difficulty (DAT_0047a7fa)
	// From FUN_0041bf8d:
	//   if (difficulty < 4) threshold = (difficulty * 5 + 5) * 1000
	//   else threshold = 15000
	int threshold;
	if (_difficulty < 4) {
		threshold = (_difficulty * 5 + 5) * 1000;  // 5000, 10000, 15000, 20000
	} else {
		threshold = 15000;
	}

	// Check if we're crossing a threshold (award bonus life)
	// Formula: score / threshold < (score + points) / threshold
	if (threshold > 0) {
		int oldMilestone = _playerScore / threshold;
		int newMilestone = (_playerScore + points) / threshold;
		if (oldMilestone < newMilestone) {
			// Award bonus life
			_playerLives++;
			// TODO: Play bonus life sound (FUN_0041189e(5, 0, 0x7f, 0, 0))
			debug("Rebel2: BONUS LIFE! Score crossed %d threshold. Lives=%d", threshold, _playerLives);
		}
	}

	// Add points to score
	_playerScore += points;
	debug("Rebel2: Score +%d = %d", points, _playerScore);
}

// Render score to HUD (part of FUN_0041c012)
// Score is drawn using FUN_00434cb0 with format string "%07ld"
// In retail, score is rendered to a status bar buffer, then blitted to screen at Y=180 (0xb4)
// The text within the status bar is at local Y=4, so screen Y = 180 + 4 = 184
void InsaneRebel2::renderScoreHUD(byte *renderBitmap, int pitch, int width, int height, int statusBarY) {
	// In retail, score is rendered by FUN_0041c012 which calls FUN_00434cb0
	// The status bar is blitted to screen at Y = DAT_0047ab2c + 0xb4 (typically 0 + 180 = 180)
	// Text position within status bar from FUN_0041c012 line 136-137:
	//   X = ((DAT_0047a808 < 2) - 1 & 0x101) + 0x101 = 0x101 (257) for low-res
	//   Y = ((DAT_0047a808 < 2) - 1 & 4) + 4 = 4 for low-res
	// So final screen position: X=257, Y=180+4=184
	// Format: 7-digit zero-padded decimal ("%07ld")

	(void)statusBarY; // Not used - we use fixed Y positions

	// Use SMALFONT.NUT (NutRenderer) for rendering digits
	// If not available, skip rendering
	if (!_smush_dispfontNut) {
		debug(1, "renderScoreHUD: _smush_dispfontNut is NULL!");
		return;
	}

	// The SMUSH buffer is 424x260, but the visible screen is 320x200
	// The view offset (_viewX, _viewY) determines where in the buffer the screen is showing
	// To render at fixed screen positions, we add the view offset

	// Convert score to 7-digit string
	char scoreStr[16];
	Common::sprintf_s(scoreStr, "%07d", _playerScore);

	// Status bar is at Y=180 (0xb4), text within it at Y=4, so total Y=184
	// Score X position is 257 (0x101)
	const int STATUS_BAR_Y = 180;  // 0xb4 from FUN_41C012 line 149/152
	const int SCORE_TEXT_Y = 4;    // Text position within status bar

	int scoreX = 257 + _viewX;
	int scoreY = STATUS_BAR_Y + SCORE_TEXT_Y + _viewY;

	debug(5, "renderScoreHUD: Drawing score=%d at buffer(%d,%d) viewOffset(%d,%d)",
		  _playerScore, scoreX, scoreY, _viewX, _viewY);

	// Draw each character manually using NutRenderer::drawCharV7
	Common::Rect clipRect(0, 0, width, height);
	int x = scoreX;
	for (int i = 0; scoreStr[i] != '\0'; i++) {
		byte ch = (byte)scoreStr[i];
		int charWidth = _smush_dispfontNut->getCharWidth(ch);
		if (charWidth > 0) {
			// Use drawCharV7 with color 255 (white) for visibility
			_smush_dispfontNut->drawCharV7(renderBitmap, clipRect, x, scoreY, pitch, 255, kStyleAlignLeft, ch, true, true);
			x += charWidth;
		}
	}

	// Also draw lives counter - in status bar at X=168 (0xa8), Y=7 within bar
	const int LIVES_TEXT_Y = 7;
	char livesStr[8];
	Common::sprintf_s(livesStr, "%d", _playerLives);
	int livesX = 168 + _viewX;
	int livesY = STATUS_BAR_Y + LIVES_TEXT_Y + _viewY;

	x = livesX;
	for (int i = 0; livesStr[i] != '\0'; i++) {
		byte ch = (byte)livesStr[i];
		int charWidth = _smush_dispfontNut->getCharWidth(ch);
		if (charWidth > 0) {
			_smush_dispfontNut->drawCharV7(renderBitmap, clipRect, x, livesY, pitch, 255, kStyleAlignLeft, ch, true, true);
			x += charWidth;
		}
	}
}

int32 InsaneRebel2::processMouse() {
	int32 buttons = 0;

	// Get button state directly from event manager (SCUMM VARs aren't updated during SMUSH)
	// Bit 0 = left button, Bit 1 = right button, Bit 2 = middle button
	uint32 currentButtons = _vm->_system->getEventManager()->getButtonState();

	// Edge detection for buttons
	bool leftPressed = (currentButtons & 1) != 0;
	bool leftWasPressed = (_prevMouseButtons & 1) != 0;
	bool rightPressed = (currentButtons & 2) != 0;
	bool rightWasPressed = (_prevMouseButtons & 2) != 0;

	// Store current state for next frame's edge detection
	_prevMouseButtons = currentButtons;

	// Update _rebelControlMode (DAT_0047a7e4) for Handler 25 covered/uncovered toggle:
	// Use "sticky" flags - set on button press, cleared by IACT handler after consumption.
	// This ensures button presses aren't missed due to timing.
	//
	// For Handler 25: use edge detection with sticky flags
	if (_rebelHandler == 25) {
		// Only SET flags on button press (edge), don't clear them here
		// The IACT handler will clear them after processing
		if (rightPressed && !rightWasPressed) {
			_rebelControlMode |= 2;  // Right button pressed - sticky
		}
		if (leftPressed && !leftWasPressed) {
			_rebelControlMode |= 1;  // Left button pressed - sticky
		}
	} else {
		// Other handlers: use simple hold state
		_rebelControlMode = 0;
		if (rightPressed) {
			_rebelControlMode |= 2;
		}
		if (leftPressed) {
			_rebelControlMode |= 1;
		}
	}

	// Left button: Trigger shot on button press (not hold)
	// From FUN_0040d836 (Handler 7) line 141: shots only spawn when DAT_004437c0 == 2
	// From FUN_00401CCF (Handler 8) line 82-84: mode 4 disables shooting
	if (leftPressed && !leftWasPressed && isShootingAllowed()) {
		Common::Point mousePos(_vm->_mouse.x, _vm->_mouse.y);
		debug("Rebel2 Click: Mouse=(%d,%d) Enemies=%d",
			mousePos.x, mousePos.y, _enemies.size());

		// Spawn visual shot immediately
		spawnShot(mousePos.x, mousePos.y);

		// Calculate world position for hit testing
		Common::Point worldMousePos(mousePos.x + _viewX, mousePos.y + _viewY);

		// Check for hit on any active enemy
		Common::List<enemy>::iterator it;
		for (it = _enemies.begin(); it != _enemies.end(); ++it) {
			debug("  Enemy ID=%d active=%d destroyed=%d rect=(%d,%d)-(%d,%d) contains=%d",
				it->id, it->active, it->destroyed,
				it->rect.left, it->rect.top, it->rect.right, it->rect.bottom,
				it->rect.contains(worldMousePos));
				
			if (it->active && it->rect.contains(worldMousePos)) {
				// Enemy hit!
				it->active = false;
				it->destroyed = true;  // Mark as destroyed so IACT won't re-activate
				debug("Rebel2: HIT enemy ID=%d type=%d at (%d,%d) - Rect: (%d,%d)-(%d,%d)",
					it->id, it->type, mousePos.x, mousePos.y,
					it->rect.left, it->rect.top, it->rect.right, it->rect.bottom);

				// Spawn visual explosion based on handler and enemy type.
				//
				// Each handler's explosion rendering (FUN_409FBC, FUN_402696,
				// FUN_40F1C5, FUN_41F29A) checks a per-level flags field:
				//   (*(ushort *)(&DAT_0047e108 + chapter*0x242 + level*0x22) & 1) == 0
				// When bit 0 is SET, explosion NUT sprites are NOT rendered even
				// though the counter ticks down. The flags come from GAME.TRS.
				//
				// Handler 8 (FUN_4028C5 line 94): Only type 0 sets the explosion
				// counter at all. Types 1-4 get BLAST sound, no visual explosion.
				//
				// Handler 25 (FUN_41E7C2 line 74): Types > 3 DO set the counter,
				// but rendering is suppressed by flags & 1 for on-foot levels.
				// The counter serves only as a timer (sound panning, tracking).
				// Handler 25 is specifically for on-foot corridor/FPS sections;
				// space combat uses handler 7 instead.
				//
				// Handlers 0x26, 7: All types get visual explosions.
				if (_rebelHandler != 8 && _rebelHandler != 25) {
					spawnExplosion((it->rect.left + it->rect.right) / 2,
								   (it->rect.top + it->rect.bottom) / 2,
								   it->rect.width() / 2);
				} else if (_rebelHandler == 8 && it->type == 0) {
					spawnExplosion((it->rect.left + it->rect.right) / 2,
								   (it->rect.top + it->rect.bottom) / 2,
								   it->rect.width() / 2);
				}

				// Disable self (prevents sprite from rendering via SKIP chunks)
				setBit(it->id);

				// Set enemy type bit in wave state (FUN_004028c5 line 74)
				// DAT_0047ab98 |= 1 << (enemyType & 0x1f)
				// This tracks which enemy GROUPS have been killed in this wave
				if (it->type > 0 && it->type < 32) {
					_rebelWaveState |= (1 << it->type);
					debug("Rebel2: Wave state updated: 0x%x (set bit %d)", _rebelWaveState, it->type);
				}

				// Increment kill counter (DAT_0047ab88)
				_rebelKillCounter++;

				// Handle dependencies
				int id = it->id;
				if (id >= 0 && id < 512) {
					// Slot 2: Enable (Explosion?)
					if (_rebelLinks[id][2] != 0) {
						clearBit(_rebelLinks[id][2]);
						debug("Rebel2: Enabled dependency Slot 2 (ID=%d) for Parent %d", _rebelLinks[id][2], id);
					}
					// Slot 1: Enable (Explosion?)
					if (_rebelLinks[id][1] != 0) {
						clearBit(_rebelLinks[id][1]);
						debug("Rebel2: Enabled dependency Slot 1 (ID=%d) for Parent %d", _rebelLinks[id][1], id);
					}
					// Slot 0: Disable (Shots?)
					if (_rebelLinks[id][0] != 0) {
						setBit(_rebelLinks[id][0]);
						debug("Rebel2: Disabled dependency Slot 0 (ID=%d) for Parent %d", _rebelLinks[id][0], id);
					}
				}

				// Play explosion sound (FUN_0041189e).
				// Pan based on enemy center X position: (screenX - 160) mapped to [-127,127]
				{
					int enemyCenterX = (it->rect.left + it->rect.right) / 2 - _viewX;
					int sfxPan = CLIP((enemyCenterX - 160) * 127 / 160, -127, 127);
					if (_rebelHandler == 8 && it->type >= 1 && it->type <= 4) {
						playSfx(0, 127, sfxPan);  // BLAST.SAD for handler 8 types 1-4
					} else {
						playSfx(2, 127, sfxPan);  // EXPLODE.SAD for all other enemies
					}
				}

				// Award score for destroying enemy (FUN_0041bf8d called from FUN_40A2E0)
				// Score value comes from lookup table DAT_0047e0fe indexed by difficulty
				int scoreIndex = _difficulty * 4;  // Simplified index
				if (scoreIndex >= 0 && scoreIndex < 16) {
					addScore(kScoreTableEnemyDestroy[scoreIndex]);
				}

				// Only hit one enemy per click
				break;
			}
		}
	}
	return buttons;
}

bool InsaneRebel2::isBitSet(int n) {
	// FUN_00423970: When param_1 < 1 (0 or negative), the bounds check fails and returns false.
	// This means ID 0 or negative IDs are always treated as "enabled" (not skipped).
	if (n < 1) {
		return false;
	}
	assert (n < 0x200);

	return (_iactBits[n] != 0);
}

void InsaneRebel2::setBit(int n) {
	// FUN_004239b0: When n < 1 (i.e., n == 0 or negative), set ALL bits to 1 (disable all objects)
	// This is used to disable all enemies/objects at once
	if (n < 1) {
		for (int i = 0; i < 0x200; i++)
			_iactBits[i] = 1;
		return;
	}
	assert (n < 0x200);
	_iactBits[n] = 1;
}

void InsaneRebel2::clearBit(int n) {
	// FUN_00423a00: When n < 1 (i.e., n == 0 or negative), clear ALL bits to 0 (enable all objects)
	// This is called by FUN_00423880 at the start of video playback to reset the bit table,
	// ensuring all enemies are visible when a new level/segment starts.
	if (n < 1) {
		for (int i = 0; i < 0x200; i++)
			_iactBits[i] = 0;
		return;
	}
	assert (n < 0x200);
	_iactBits[n] = 0;
}

// Check if shooting is allowed based on current handler and control mode
// From FUN_0040d836 (Handler 7): shooting only allowed when DAT_004437c0 == 2
// From FUN_00401CCF (Handler 8): mode 4/5 disable shooting
// From FUN_41DB5E (Handler 25): only shoot when fully uncovered (DAT_0045790a == 0)
bool InsaneRebel2::isShootingAllowed() {
	// Handler 7 (Third-Person Ship): Only mode 2 allows shooting
	// FUN_0040d836 line 141: if (DAT_004437c0 == 2) { /* spawn shots, draw crosshair */ }
	if (_rebelHandler == 7) {
		return (_flyControlMode == 2);
	}

	// Handler 8 (Third-Person On Foot): Modes 4/5 disable shooting
	// FUN_00401CCF line 82-84: if (DAT_0043e000 == 4) { param_5 = 0; }
	// Mode 5: Ship not rendered (cutscene)
	if (_rebelHandler == 8) {
		return (_shipLevelMode != 4 && _shipLevelMode != 5);
	}

	// Handler 25 (0x19): Only allow shooting when fully uncovered
	// FUN_41DB5E lines 170-171: if (((param_5 & 1) != 0) && (DAT_0045790a == 0))
	// _rebelDamageLevel = DAT_0045790a (cover transition counter, 0 = uncovered, 5 = covered)
	if (_rebelHandler == 25) {
		return (_rebelDamageLevel == 0);
	}

	// Handler 0x26 (Turret): Always allow shooting when active
	return (_rebelHandler != 0);
}

void InsaneRebel2::procSKIP(int32 subSize, Common::SeekableReadStream &b) {
	// Rebel Assault 2 uses SKIP chunks to conditionally skip the next FOBJ/PSAD chunk.
	// The SKIP chunk contains one or two object IDs. If the bit for the object is set
	// (i.e., the object is disabled/destroyed), skip the next chunk.
	//
	// This is the same mechanism as Full Throttle, but RA2 uses it for enemy objects:
	// - When an enemy is destroyed, setBit(enemy_id) is called
	// - SKIP chunks in the video contain the enemy ID
	// - If the bit is set, the next FOBJ (enemy sprite) is skipped
	// - This prevents destroyed enemy sprites from being rendered
	//
	// The original game's FUN_00423A50 chunk reader uses this mechanism.

	int16 par1, par2;
	_player->_skipNext = false;

	assert(subSize >= 4);
	par1 = b.readUint16LE();
	par2 = b.readUint16LE();

	if (!par2) {
		// Single ID mode: skip next chunk if this object's bit is set (disabled)
		bool bit1 = isBitSet(par1);
		if (bit1) {
			_player->_skipNext = true;
		}
		debug("Rebel2 SKIP: single ID=%d bit=%d skip=%d frame=%d", par1, bit1 ? 1 : 0, _player->_skipNext ? 1 : 0, _player->_frame);
	} else {
		// Dual ID mode: skip if bits are different (XOR logic)
		bool bit1 = isBitSet(par1);
		bool bit2 = isBitSet(par2);
		if (bit1 != bit2) {
			_player->_skipNext = true;
		}
		debug("Rebel2 SKIP: dual ID1=%d(bit=%d) ID2=%d(bit=%d) skip=%d frame=%d", par1, bit1 ? 1 : 0, par2, bit2 ? 1 : 0, _player->_skipNext ? 1 : 0, _player->_frame);
	}
}

void InsaneRebel2::procPreRendering(byte *renderBitmap) {
	// Call base class implementation first (handles Full Throttle state machine)
	Insane::procPreRendering(renderBitmap);

	// For Level 2 gameplay (Handler 8 only), restore the background BEFORE FOBJ decoding.
	// The tiny FOBJ sprites (7x10, 9x38 pixels) only draw new sprite positions but don't
	// clear old ones. By restoring the full background each frame, we ensure old sprite
	// positions are erased before new ones are drawn.
	//
	// This is called at the start of handleFrame(), before any FOBJ chunks are processed.
	if (_rebelHandler == 8 && _level2BackgroundLoaded && _level2Background && renderBitmap) {
		for (int y = 0; y < 200; y++) {
			memcpy(renderBitmap + y * 320, _level2Background + y * 320, 320);
		}
	}

	// For Handler 25 (Level 2 speeder bike), draw the corridor overlay BEFORE FOBJ decoding.
	// The corridor overlay (par3=4 -> _rebelEmbeddedHud[4]) is DAT_00482268, a 350x230 buffer.
	// From FUN_0041cadb line 216: FUN_00428a10(param_1,0,DAT_0045790c,DAT_0045790e,DAT_00482268)
	// It's drawn at (DAT_0045790c, DAT_0045790e) which are _rebelViewOffsetX/Y.
	//
	// For Mode 1: DAT_0045790c = damageLevel * -5 - 14, range -39 (covered) to -14 (uncovered)
	//
	// From FUN_00428a10: When position is negative, we skip source pixels and draw at 0.
	// Handler 25: Corridor overlay and FOBJ position offsets are set during
	// IACT opcode 6 processing (iactRebel2Opcode6), matching the original
	// FUN_41CADB architecture. No corridor drawing needed here.

	// Chapter selection: Set FOBJ offset to scroll preview thumbnails in O_LEVEL.SAN.
	// Original (FUN_00415CF8): offsets start at (0,0) for the first display update,
	// then FUN_00425170 sets them to (-90, chapter*-50+75) AFTER each frame.
	// Frame 0 must use (0,0) so the 80x800 preview strip at X=320 renders off-screen
	// and STOR captures it cleanly. Frames 1+ use the scroll offset so FTCH re-renders
	// the strip at the correct preview position.
	if (_gameState == kStateChapterSelect && _player) {
		if (_player->_frame > 0) {
			// Clear screen to black before FTCH re-renders the preview strip.
			// Our FTCH only re-draws the preview area (80px wide at X=230);
			// without clearing, old menu text and preview artifacts persist.
			if (renderBitmap) {
				memset(renderBitmap, 0, _vm->_screenWidth * _vm->_screenHeight);
			}
			_player->_fobjOffsetX = _previewOffsetX;
			_player->_fobjOffsetY = _previewOffsetY;
		}
	}
}

void InsaneRebel2::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
					  int16 par1, int16 par2, int16 par3, int16 par4) {
	// Debug: Log all IACT opcodes
	debug("Rebel2 IACT: opcode=%d par2=%d par3=%d par4=%d gameState=%d sceneId=%d",
		par1, par2, par3, par4, _gameState, _currSceneId);

	if (_keyboardDisable)
		return;

	// Handle menu IACT - menu videos have embedded ANIM data in IACT chunks
	// Menu IACTs have par1=8 (code), par2=46 (flags), par4>=1000 (userId)
	// The embedded ANIM contains the full menu frame
	if (_gameState == kStateMainMenu && par1 == 8 && par4 >= 1000) {
		debug("Rebel2 IACT: Menu mode - processing embedded ANIM (userId=%d)", par4);

		// Scan for embedded ANIM tag in the IACT data
		int64 startPos = b.pos();
		int64 totalSize = b.size();
		debug("Rebel2 IACT: stream pos=%d, size=%d, remaining=%d",
			(int)startPos, (int)totalSize, (int)(totalSize - startPos));

		if (totalSize > startPos) {
			int64 remaining = totalSize - startPos;
			int scanSize = (int)MIN<int64>(remaining, 65536);
			byte *scanBuf = (byte *)malloc(scanSize);
			if (scanBuf) {
				int bytesRead = b.read(scanBuf, scanSize);
				debug("Rebel2 IACT: Read %d bytes, first 16: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
					bytesRead, scanBuf[0], scanBuf[1], scanBuf[2], scanBuf[3],
					scanBuf[4], scanBuf[5], scanBuf[6], scanBuf[7],
					scanBuf[8], scanBuf[9], scanBuf[10], scanBuf[11],
					scanBuf[12], scanBuf[13], scanBuf[14], scanBuf[15]);

				// Look for ANIM tag (embedded SAN containing menu frame)
				for (int i = 0; i + 8 <= bytesRead; ++i) {
					if (READ_BE_UINT32(scanBuf + i) == MKTAG('A','N','I','M')) {
						int64 animStreamPos = startPos + i;
						uint32 animReportedSize = READ_BE_UINT32(scanBuf + i + 4);
						int32 toCopy = (int)MIN<int64>((int64)animReportedSize + 8, totalSize - animStreamPos);
						debug("Rebel2 IACT: Found embedded ANIM at offset %d, size %d", (int)i, (int)animReportedSize);
						if (toCopy > 0) {
							byte *animData = (byte *)malloc(toCopy);
							if (animData) {
								b.seek(animStreamPos);
								b.read(animData, toCopy);
								// Use userId as the HUD slot (1000 -> slot 0 for menu background)
								loadEmbeddedSan(0, animData, toCopy, renderBitmap);
								free(animData);
							}
						}
						b.seek(startPos);
						free(scanBuf);
						return;
					}
				}

				debug("Rebel2 IACT: No ANIM tag found in menu IACT data");
				b.seek(startPos);
				free(scanBuf);
			}
		}
		return;
	}

	if (_currSceneId == 1)
		iactRebel2Scene1(renderBitmap, codecparam, setupsan12, setupsan13, b, size, flags, par1, par2, par3, par4);
}


void InsaneRebel2::iactRebel2Scene1(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
				  int16 par1, int16 par2, int16 par3, int16 par4) {
	// par1 is the Opcode (word at offset +0)
	// par2 is word at offset +2
	// par3 is word at offset +4
	// par4 is word at offset +6
	//
	// Based on disassembly of FUN_4028C5 and FUN_4033CF:
	// 
	// For IACT opcode 4 (enemy position update), the structure is:
	//   Offset +0x06: Type/SubType (par3)
	//   Offset +0x08: Enemy ID
	//   Offset +0x0A: X position
	//   Offset +0x0C: Y position
	//   Offset +0x0E: Width
	//   Offset +0x10: Height
	//
	// The original game calculates bounding box center:
	//   centerX = X + (Width / 2)
	//   centerY = Y + (Height / 2)
	// Then subtracts scroll offsets:
	//   screenX = centerX - DAT_0043e006 (scrollX)
	//   screenY = centerY - DAT_0043e008 (scrollY)

	//   screenX = centerX - DAT_0043e006 (scrollX)
	//   screenY = centerY - DAT_0043e008 (scrollY)

	if (par1 == 4) {
		enemyUpdate(renderBitmap, b, par2, par3, par4);
	} else if (par1 == 2) {
		// Delegate handling to dedicated opcode 2 handler
		iactRebel2Opcode2(b, par2, par3, par4);
	} else if (par1 == 3) {
		iactRebel2Opcode3(b, par2, par3, par4);
	}
	else if (par1 == 5) {
		// Opcode 5: Collision Zone Registration (FUN_004033cf case 5)
		// Sub-opcode 0x0D (13) = Primary collision zones (obstacles)
		// Sub-opcode 0x0E (14) = Secondary collision zones (boundaries)
		// par2 is the sub-opcode that determines which zone table to use
		debug("Rebel2 IACT Opcode 5: par2=%d par3=%d par4=%d", par2, par3, par4);

		if (par2 == 0x0D || par2 == 0x0E) {
			// Register the collision zone from the remaining IACT data
			// par4 (userId from IACT header) is the filter value used by FUN_4092D9
			// for the < 1000 test (offset +6 in the original stored pointer)
			registerCollisionZone(b, par2, par4);
		}

	} else if (par1 == 7) {
		// Opcode 7: Handler 7 corridor/velocity control (FUN_40C3CC case 5)
		// IACT header: par1=7, par2=flags, par3=0, par4=sub-opcode
		// Body contains 2 int16 values (body[0], body[1])
		//
		// par4 sub-opcodes (from FUN_40C3CC case 5 switch on param_5[3]):
		//   0: Set velocity params (DAT_00443b12, DAT_00443b14)
		//   1: Set left X + top Y corridor boundaries (DAT_00443b0a, DAT_00443b0c)
		//   2: Set right X + bottom Y corridor boundaries (DAT_00443b0e, DAT_00443b10)
		//   5: Set flag (DAT_00443b52)

		int16 body0 = 0, body1 = 0;
		if (b.size() - b.pos() >= 4) {
			body0 = b.readSint16LE();
			body1 = b.readSint16LE();
		}

		switch (par4) {
		case 0:
			// Velocity/wind data — affects ship drift in FUN_40C3CC physics
			// DAT_00443b12 = horizontal wind, DAT_00443b14 = vertical wind
			_windParamX = body0;
			_windParamY = body1;
			debug("Rebel2 Opcode 7 par4=0: wind=(%d,%d)", body0, body1);
			break;
		case 1:
			// Set LEFT X boundary and TOP Y boundary
			_corridorLeftX = body0;
			_corridorTopY = body1;
			// Mode-dependent margin adjustment (FUN_40C3CC lines 341-351)
			if (_flyControlMode == 2) {
				_corridorLeftX += 15;
			} else if (_flyControlMode == 0) {
				_corridorLeftX += 20;
			}
			debug("Rebel2 Opcode 7 par4=1: corridor left=%d top=%d (adjusted left=%d)",
				body0, body1, _corridorLeftX);
			break;
		case 2:
			// Set RIGHT X boundary and BOTTOM Y boundary
			_corridorRightX = body0;
			_corridorBottomY = body1;
			// Mode-dependent margin adjustment (FUN_40C3CC lines 356-365)
			if (_flyControlMode == 2) {
				_corridorRightX -= 15;
			} else if (_flyControlMode == 0) {
				_corridorRightX -= 20;
			}
			debug("Rebel2 Opcode 7 par4=2: corridor right=%d bottom=%d (adjusted right=%d)",
				body0, body1, _corridorRightX);
			break;
		case 5:
			// Flag value
			debug("Rebel2 Opcode 7 par4=5: flag=%d", body0);
			break;
		default:
			debug("Rebel2 Opcode 7 par4=%d: body=(%d,%d) — unknown sub-opcode", par4, body0, body1);
			break;
		}

	} else if (par1 == 6) {
		// Opcode 6: Level setup / mode switch (FUN_41CADB case 4)
		iactRebel2Opcode6(renderBitmap, b, size, par2, par3, par4);
	} else if (par1 == 8) {
		// Opcode 8: HUD resource loading (FUN_41CADB case 6)
		iactRebel2Opcode8(renderBitmap, b, size, par2, par3, par4);
	} else if (par1 == 9) {
		// Opcode 9: Text/subtitle display
		iactRebel2Opcode9(renderBitmap, b, par2, par3, par4);
	} else if (par1 == 0 || par1 == 1) {
		// Low Opcodes seen in logs
		debug("Rebel2 IACT: Low Opcode %d (par2=%d par3=%d par4=%d)", par1, par2, par3, par4);
	} else {
		debug("Rebel2 IACT: Unknown Opcode %d (par2=%d par3=%d par4=%d)", par1, par2, par3, par4);
	}
}
void InsaneRebel2::iactRebel2Opcode2(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	// Handle IACT opcode 2 subcases based on par3 (type). Mirrors FUN_00407fcb behavior where relevant.
	// Keep existing linking behavior (par3 == 4) for compatibility.

	// Link case: par3 == 4
	if (par3 == 4) {
		int16 childId = b.readSint16LE(); // Offset +8
		int16 parentId = b.readSint16LE(); // Offset +10

		// Validate BOTH parentId AND childId to avoid triggering "set/clear ALL bits" behavior
		// when childId <= 0. The original game's setBit(0)/clearBit(0) affects ALL bits,
		// which would disable/enable all enemies at once - not the intended linking behavior.
		if (parentId >= 1 && parentId < 512 && childId >= 1 && childId < 512) {
			// Shift links (original: 4 link slots at DAT_0045797c/817c/897c/917c)
			_rebelLinks[parentId][2] = _rebelLinks[parentId][1];
			_rebelLinks[parentId][1] = _rebelLinks[parentId][0];
			_rebelLinks[parentId][0] = childId;

			// Mirror parent's bit state to child (INVERTED):
			// - Parent alive (bit clear) → setBit(child) → child hidden
			// - Parent dead (bit set) → clearBit(child) → child shown
			// From FUN_0041CADB case 0, par3==4:
			//   bVar3 = FUN_00423970(parentId);
			//   if (bVar3 == 0) setBit(childId); else clearBit(childId);
			// This ensures linked children (explosion/death sprites) are hidden
			// while the parent is alive, and revealed when the parent is destroyed.
			if (!isBitSet(parentId)) {
				setBit(childId);
				debug("Rebel2: Linked ID=%d to Parent=%d (Slot 0) - child DISABLED (parent alive)", childId, parentId);
			} else {
				clearBit(childId);
				debug("Rebel2: Linked ID=%d to Parent=%d (Slot 0) - child ENABLED (parent dead)", childId, parentId);
			}
		} else {
			debug("Rebel2: Skipping link with invalid IDs childId=%d parentId=%d", childId, parentId);
		}
		return;
	} else if (par3 == 1) { // Probabilistic / counter cases: par3 == 1
		int16 value = par4; // sVar6
		int16 targetId = b.readSint16LE(); // Offset +8 (sVar7)

		// Validate targetId >= 1 to avoid triggering "set/clear ALL bits" behavior
		// The original game's setBit(0)/clearBit(0) affects ALL bits, not intended here
		if (targetId < 1 || targetId >= 0x200)
			return;

		// Handler 8/25: FUN_401234 case 0 / FUN_0041CADB case 0 par3==1
		// From original FUN_0041CADB:
		//   if (par4 == 100) clearBit(body0);  // Force enable
		//   else { bitMask = 1 << (par4 & 0x1f); if (waveState & bitMask) setBit(body0); }
		if ((_rebelHandler == 8 || _rebelHandler == 25) && value != 0) {
			if (value == 100) {
				// par4==100: Force enable the target (original: FUN_00423a00)
				clearBit(targetId);
				debug("Rebel2 Opcode2 (H%d): Force ENABLE target=%d (par4=100)", _rebelHandler, targetId);
			} else {
				// Check wave state: if enemy type has been killed, disable target
				int bitMask = 1 << (value & 0x1f);
				if ((_rebelWaveState & bitMask) != 0) {
					setBit(targetId);
					debug("Rebel2 Opcode2 (H%d): Disable target=%d (type %d killed, wave=0x%x)", _rebelHandler, targetId, value, _rebelWaveState);
				}
			}
			return;
		}

		if (value > 1 && value < 10) { // 1 < value < 10: random disable
			if (_vm->_rnd.getRandomNumber(value) == 0) {
				setBit(targetId);
				debug("Rebel2 IACT Opcode2: Random DISABLE target=%d (value=%d)", targetId, value);
			}
		} else if (value > 10 && value < 20) { // 10 < value < 20: enable/disable with special value==11 = force enable
			if (value == 11) {
				clearBit(targetId);
				debug("Rebel2 IACT Opcode2: FORCE ENABLE target=%d (value=11)", targetId);
			} else {
				if (_vm->_rnd.getRandomNumber(value - 10) == 0) {
					clearBit(targetId);
					debug("Rebel2 IACT Opcode2: Random ENABLE target=%d (value=%d)", targetId, value);
				} else {
					setBit(targetId);
					debug("Rebel2 IACT Opcode2: Random DISABLE target=%d (value=%d)", targetId, value);
				}
			}
		} else if (value > 99 && value < 110) { // 99 < value < 110: increment value counter if target active
			if (!isBitSet(targetId)) {
				int idx = value - 100;
				if (idx >= 0 && idx < 10) {
					_rebelValueCounters[idx]++;
					_rebelLastCounter = _rebelValueCounters[idx];
					debug("Rebel2 IACT Opcode2: Increment VAL counter[%d] -> %d (target=%d)", value, _rebelValueCounters[idx], targetId);
				}
			}

		} else if (value > 0x3ff) { // Bitmask case: value > 0x3FF
 			for (int slot = 1; slot <= 9; ++slot) {
				if ((value & (1 << (slot - 1))) != 0) {
					if (!isBitSet(targetId)) {
						_rebelMaskCounters[slot]++;
						_rebelLastCounter = _rebelMaskCounters[slot];
						debug("Rebel2 IACT Opcode2: Increment MASK counter[%d] -> %d (target=%d)", slot, _rebelMaskCounters[slot], targetId);
					}
				}
			}
		}

		// Unknown sub-type: log and return
		debug("Rebel2 IACT Opcode2: Unhandled par3=%d par4=%d", par3, par4);
	}
}
void InsaneRebel2::iactRebel2Opcode3(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	// IACT opcode 3 — damage and hit counter processing.
	// Based on FUN_4092D9 (Handler 0x26), FUN_40E35E (Handler 7), FUN_401234 (Handler 8).
	//
	// The common dispatcher (FUN_4033CF) stores opcode 3 entries in the projectile impact
	// list (DAT_0043f9e0). For handlers 0x26/7 these are processed per-frame by the
	// per-handler collision function (FUN_4092D9/FUN_40E35E). For handlers 8/25 they're
	// processed immediately during IACT dispatch.
	//
	// FUN_403ba9() loop in FUN_4092D9 (lines 209-239):
	//   par3 == 1/2: Direct hit — increment hit counter, apply damage if conditions met
	//     - body[0] (offset +8): srcId for isBitSet check
	//     - par4 != 0: damage from DAT_0047e0f4 (direct hit damage table)
	//     - par3==1: par4 must be 1..9 for damage
	//     - par3==2: par4 must be > 99, with wave state bit check for par4 >= 101
	//
	//   par3 == 5: Probabilistic damage — probability check from DAT_0047e0fc
	//     - body[1] (offset +10): srcId for isBitSet check (different from par3=1/2!)
	//     - Damage from DAT_0047e0f8 (probabilistic damage table)
	//
	// Stream position on entry: at offset +8 (body[0], first word after 8-byte header)

	// Handler 25 has a different opcode 3 structure (FUN_41CADB case 1):
	//   par3==5: probabilistic damage WITH cover check (DAT_0045790a < 2)
	//   par3==1: increment hit counter ONLY (NO damage), requires par4 != 4
	//   par4==100: direct damage (separate check after par3 branches, NO cover check)
	// Other handlers (0x26/7/8) use FUN_4092D9/FUN_40E35E/FUN_401234 with different logic.
	if (_rebelHandler == 25) {
		// Handler 25 opcode 3 — FUN_41CADB case 1
		int16 srcIdBody0 = b.readSint16LE(); // body[0] (offset +8)
		int16 srcIdBody1 = b.readSint16LE(); // body[1] (offset +10)

		if (par3 == 5) {
			// Probabilistic damage with cover check (lines 81-92)
			debug("Rebel2 Opcode3: H25 par3=5 srcId=%d isBitSet=%d damageLevel=%d",
				srcIdBody1, isBitSet(srcIdBody1), _rebelDamageLevel);

			if (_rebelDamageLevel < 2 && !isBitSet(srcIdBody1)) {
				int probability = 20 + _difficulty * 20;
				if (probability < 5) probability = 5;
				if (probability > 90) probability = 90;
				int roll = _vm->_rnd.getRandomNumber(99);
				debug("Rebel2 Opcode3: probability=%d roll=%d (need roll < prob)", probability, roll);

				if (roll < probability) {
					if (!_rebelInvulnerable) {
						int damageAmount = 5 + (_difficulty * 2);
						_playerDamage += damageAmount;
						if (_playerDamage > 255) _playerDamage = 255;
						debug("Rebel2: H25 PROBABILISTIC damage from %d. Damage=%d total=%d",
							srcIdBody1, damageAmount, _playerDamage);
					}
					initDamageFlash();
				}
			} else {
				debug("Rebel2 Opcode3: H25 par3=5 BLOCKED (damageLevel=%d isBitSet=%d)",
					_rebelDamageLevel, isBitSet(srcIdBody1));
			}
		} else if (par3 == 1 && !isBitSet(srcIdBody0) && par4 != 4) {
			// Hit counter only — NO damage (lines 94-98)
			_rebelHitCounter++;
			debug("Rebel2: H25 hit counter++ -> %d (par3=1 par4=%d, no damage)",
				_rebelHitCounter, par4);
		} else {
			debug("Rebel2 Opcode3: H25 par3=%d par4=%d (no action)", par3, par4);
		}

		// Direct damage: par4==100, separate from par3 branches (lines 99-111)
		if (par4 == 100 && !isBitSet(srcIdBody0)) {
			if (!_rebelInvulnerable) {
				int directHitDamage = 8 + (_difficulty * 4);
				_playerDamage += directHitDamage;
				if (_playerDamage > 255) _playerDamage = 255;
				debug("Rebel2: H25 DIRECT HIT par4=100 damage=%d total=%d",
					directHitDamage, _playerDamage);
			}
			initDamageFlash();
		}
	} else if (par3 == 1 || par3 == 2) {
		// Non-Handler-25 direct hit path — FUN_4092D9 lines 209-227
		int16 srcId = b.readSint16LE(); // body[0] (offset +8): source enemy ID

		debug("Rebel2 Opcode3: par3=%d par4=%d srcId=%d isBitSet=%d",
			par3, par4, srcId, isBitSet(srcId));

		if (!isBitSet(srcId)) {
			_rebelHitCounter++;
			debug("Rebel2: Incremented hit counter -> %d", _rebelHitCounter);

			int directHitDamage = 8 + (_difficulty * 4);

			if (par4 != 0 && directHitDamage > 0) {
				bool shouldDamage = false;

				if (par3 == 1 && par4 < 10) {
					shouldDamage = true;
				} else if (par3 == 2 && par4 > 99) {
					if (par4 < 0x65 || (_rebelPhaseState & (1 << ((par4 + 0x9b) & 0x1f))) == 0) {
						shouldDamage = true;
					}
				}

				if (shouldDamage) {
					if (!_rebelInvulnerable) {
						_playerDamage += directHitDamage;
						if (_playerDamage > 255) _playerDamage = 255;
						debug("Rebel2: DIRECT HIT damage from enemy %d. par3=%d par4=%d damage=%d total=%d",
							srcId, par3, par4, directHitDamage, _playerDamage);
					}
					initDamageFlash();
				}
			}
		}
	} else if (par3 == 5) {
		// Non-Handler-25 probabilistic damage — FUN_4092D9 lines 228-239
		b.skip(2); // Skip body[0]
		int16 srcId = b.readSint16LE(); // body[1] (offset +10)

		debug("Rebel2 Opcode3: par3=5 srcId=%d isBitSet=%d", srcId, isBitSet(srcId));

		if (!isBitSet(srcId)) {
			int probability = 20 + _difficulty * 20;
			if (probability < 5) probability = 5;
			if (probability > 90) probability = 90;

			int roll = _vm->_rnd.getRandomNumber(99);
			debug("Rebel2 Opcode3: probability=%d roll=%d (need roll < prob)", probability, roll);

			if (roll < probability) {
				if (!_rebelInvulnerable) {
					int damageAmount = 5 + (_difficulty * 2);
					_playerDamage += damageAmount;
					if (_playerDamage > 255) _playerDamage = 255;
					debug("Rebel2: PROBABILISTIC damage from enemy %d. Damage=%d total=%d",
						srcId, damageAmount, _playerDamage);
				}
				if (_rebelHandler == 8) {
					triggerDamageEffect();
				} else {
					initDamageFlash();
				}
			}
		}
	} else {
		debug("Rebel2 Opcode3: UNHANDLED par3=%d par4=%d", par3, par4);
	}
}

void InsaneRebel2::iactRebel2Opcode6(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4) {
	// Opcode 6: Level setup / mode switch
	// Based on FUN_41CADB case 4 (switch on *local_14 - 2 == 4, meaning opcode 6)
	//
	// For Handler 8 (third-person on foot) - FUN_00401234 case 4:
	// - par3 sets ship level mode (DAT_0043e000)
	// - par4 == 1 triggers status bar display and state reset
	// - Updates ship position based on mouse input
	//
	// For Handler 0x26/0x19 (turret/FPS):
	// - Same par4 == 1 behavior
	// - Different view offset calculations

	debug("Rebel2 IACT Opcode 6: par2=%d par3=%d par4=%d", par2, par3, par4);

	// Update handler type if par2 is a known handler value (from FUN_4033CF case 6)
	if (par2 == 7 || par2 == 8 || par2 == 0x19 || par2 == 0x26) {
		// Reset Level 2 background flag when transitioning away from Handler 8
		if (_rebelHandler == 8 && par2 != 8) {
			_level2BackgroundLoaded = false;
		}
		_rebelHandler = par2;
		debug("Rebel2 Opcode 6: Setting handler=%d", par2);
	}

	// Handler 8 specific logic (third-person on foot) - FUN_00401234 case 4
	if (_rebelHandler == 8) {
		// Set ship level mode (DAT_0043e000 = par3)
		_shipLevelMode = par3;

		// If par4 == 1, enable status bar
		if (par4 == 1) {
			_rebelStatusBarSprite = 5;  // Status bar sprite for Handler 8
		}

		// Reset state when shipLevelMode != 0 && par4 == 1 (FUN_401234 lines 97-103)
		if (_shipLevelMode != 0 && par4 == 1) {
			// Clear ALL iactBits — matches FUN_00423880 calling FUN_00423a00(0)
			clearBit(0);
			// Clear link tables
			for (int i = 0; i < 512; i++) {
				_rebelLinks[i][0] = 0;
				_rebelLinks[i][1] = 0;
				_rebelLinks[i][2] = 0;
			}
			// DAT_0047ab98 = DAT_0047ab9c: Reset wave state to accumulated phase state
			_rebelWaveState = _rebelPhaseState;
			debug("Rebel2 Opcode 6 (Handler 8): State reset, wave=0x%x", _rebelWaveState);
		}

		// Skip position calculation for special modes 4 and 5
		if (_shipLevelMode != 4 && _shipLevelMode != 5) {
			// ===== Movement Range Transition (Covered vs Shooting) =====
			// Based on FUN_00401234 lines 85-120:
			// Mode 2 = "Covered" state - contract movement range to 41 (0x29)
			// Other modes = "Shooting" state - expand movement range to 127 (0x7f)
			// Transition happens gradually at ±10 per frame for smooth animation
			if (_shipLevelMode == 2) {
				// Covered state - contract movement range
				if (_movementRangeLimit > 41) {
					_movementRangeLimit -= 10;
				}
				if (_movementRangeLimit < 41) {
					_movementRangeLimit = 41;
				}
			} else {
				// Shooting state - expand movement range
				if (_movementRangeLimit < 127) {
					_movementRangeLimit += 10;
				}
				if (_movementRangeLimit > 127) {
					_movementRangeLimit = 127;
				}
			}

			// Calculate target position from mouse input
			// Mouse X maps to ship horizontal tilt, Mouse Y to vertical tilt
			// Based on FUN_00401234 lines 151-166:
			// local_18 = ((DAT_0047a7e0 * 5 + 0x27b) * 0x40) / 0xfe
			// local_1c = ((DAT_0047a7e2 * 5 + 0x27b) * 0x10) / 0xfe

			// Map mouse position (-127 to 127 range) to ship target
			// Mouse is 0-320, center is 160. Map to -127 to 127 range
			int16 mouseOffsetX = (int16)((_vm->_mouse.x - 160) * 127 / 160);
			int16 mouseOffsetY = (int16)((_vm->_mouse.y - 100) * 127 / 100);

			// Clamp X offset to movement range limit (covered/shooting state)
			// Based on FUN_00401234 lines 119-136
			if (mouseOffsetX > _movementRangeLimit) mouseOffsetX = _movementRangeLimit;
			if (mouseOffsetX < -_movementRangeLimit) mouseOffsetX = -_movementRangeLimit;
			// Y offset always uses full range (±127)
			if (mouseOffsetY > 127) mouseOffsetY = 127;
			if (mouseOffsetY < -127) mouseOffsetY = -127;

			// Calculate target positions using the original formula
			_shipTargetX = (int16)(((mouseOffsetX * 5 + 0x27b) * 0x40) / 0xfe);
			_shipTargetY = (int16)(-((mouseOffsetY * 5 + 0x27b) * 0x10) / 0xfe);

			// Smooth interpolation toward target (max 50 pixels per frame)
			const int16 maxStep = 50;  // 0x32 in hex
			if (_shipPosX < _shipTargetX) {
				int16 newX = _shipPosX + maxStep;
				_shipPosX = (newX > _shipTargetX) ? _shipTargetX : newX;
			} else if (_shipPosX > _shipTargetX) {
				int16 newX = _shipPosX - maxStep;
				_shipPosX = (newX < _shipTargetX) ? _shipTargetX : newX;
			}

			if (_shipPosY < _shipTargetY) {
				int16 newY = _shipPosY + maxStep;
				_shipPosY = (newY > _shipTargetY) ? _shipTargetY : newY;
			} else if (_shipPosY > _shipTargetY) {
				int16 newY = _shipPosY - maxStep;
				_shipPosY = (newY < _shipTargetY) ? _shipTargetY : newY;
			}

			// Calculate ship direction indices for sprite selection
			// Map mouse position to 5x7 direction grid (like Handler 7)
			int16 mouseX = _vm->_mouse.x;
			int16 mouseY = _vm->_mouse.y;

			// Scale mouse if video is larger than 320x200
			if (_player && _player->_width > 320) {
				mouseX = (mouseX * 320) / _player->_width;
			}
			if (_player && _player->_height > 200) {
				mouseY = (mouseY * 200) / _player->_height;
			}

			// Horizontal: 5 zones (0=far left, 2=center, 4=far right)
			if (mouseX < 64) _shipDirectionH = 0;
			else if (mouseX < 128) _shipDirectionH = 1;
			else if (mouseX < 192) _shipDirectionH = 2;
			else if (mouseX < 256) _shipDirectionH = 3;
			else _shipDirectionH = 4;

			// Vertical: 7 zones (0=far up, 3=center, 6=far down)
			if (mouseY < 28) _shipDirectionV = 0;
			else if (mouseY < 57) _shipDirectionV = 1;
			else if (mouseY < 86) _shipDirectionV = 2;
			else if (mouseY < 114) _shipDirectionV = 3;
			else if (mouseY < 143) _shipDirectionV = 4;
			else if (mouseY < 171) _shipDirectionV = 5;
			else _shipDirectionV = 6;

			_shipDirectionIndex = _shipDirectionH * 7 + _shipDirectionV;
		}

		// Update firing state from mouse button
		// Mode 4 (autopilot) disables shooting - FUN_00401CCF line 82-84
		if (_shipLevelMode == 4) {
			_shipFiring = false;
		} else {
			_shipFiring = (_vm->VAR(_vm->VAR_LEFTBTN_HOLD) != 0);
		}

		debug("Rebel2 Opcode 6 (Handler 8): mode=%d range=%d shipPos=(%d,%d) target=(%d,%d) firing=%d dir=(%d,%d,%d)",
			_shipLevelMode, _movementRangeLimit, _shipPosX, _shipPosY, _shipTargetX, _shipTargetY, _shipFiring,
			_shipDirectionH, _shipDirectionV, _shipDirectionIndex);

		// Handler 8 doesn't use the same view offset logic as other handlers
		// Skip the rest of the function for Handler 8
		return;
	}

	// Handler 7 specific logic (third-person ship) - FUN_0040d836 / FUN_0040c3cc
	// Used for Level 3 and similar space combat levels
	if (_rebelHandler == 7) {
		// Set control mode: DAT_004437c0 = param_5[3] = par4 in FUN_40C3CC case 4.
		// This determines collision mode and shooting capability:
		//   Mode 0: Obstacle avoidance — SECONDARY zones, corridor boundaries
		//   Mode 1: Tunnel flight — PRIMARY zones, per-edge push-back (hMargin=0x28)
		//   Mode 2: Combat mode — shooting ENABLED, SECONDARY zones
		//   Mode 3: Tunnel flight — PRIMARY zones, per-edge push-back (hMargin=0x0f)
		_flyControlMode = par4;
		debug("Rebel2 Opcode 6 (Handler 7): Control mode set to %d (shooting %s)",
			par4, (par4 == 2) ? "ENABLED" : "DISABLED");

		// Status bar: param_5[4] == 1 in original (first body word, 5th IACT word)
		// In our parsing, par3 maps to param_5[2] and the body follows par4.
		// FUN_40C3CC: if (param_5[4] == 1) FUN_0040bb87(DAT_0047a828,5);
		// par3 is param_5[2], which the original doesn't use here.
		// The body word for status bar is read separately below.
		int16 bodyStatusFlag = 0;
		if (b.size() - b.pos() >= 2) {
			bodyStatusFlag = b.readSint16LE();
		}
		if (bodyStatusFlag == 1) {
			_rebelStatusBarSprite = 5;  // Status bar sprite
			debug("Rebel2 Opcode 6 (Handler 7): Status bar enabled (body flag=%d)", bodyStatusFlag);
		}

		// ============================================================
		// Ship position update — FUN_40C3CC case 4, lines 49-327
		// ============================================================
		// Velocity-based physics with momentum/inertia:
		//   Mouse offset from center → scaled input [-127,127]
		//   → velocity history averaging → physics delta (clamped ±12/frame)
		//   → position clamping → corridor collision → perspective offsets
		//
		// Level data table (DAT_0047e0e8 + level*0x242 + difficulty*0x22):
		//   offset 0: smoothing param (>>4 +1 = window size)
		//   offset 2: Y speed          offset 4: X speed (levelSpeed)
		//   offset 6: wind multiplier  offset 14: corridor damage
		// We don't have the actual level data, so we use calibrated defaults.

		// --- Step 1: Mouse input as offset from screen center ---
		// DAT_0047a7e0 = mouseX - 160, DAT_0047a7e2 = mouseY - 100
		// _vm->_mouse.x/y are in virtual screen coords (0-319, 0-199)
		// consistent with handler 8 which uses _vm->_mouse.x directly.
		int16 inputX = (int16)(_vm->_mouse.x - 160);  // DAT_0047a7e0
		int16 inputY = (int16)(_vm->_mouse.y - 100);  // DAT_0047a7e2

		// Clamp: mouse mode uses [-160, 160] for X, [-127, 127] for Y (lines 55-70)
		if (inputX > 160) inputX = 160;
		if (inputX < -160) inputX = -160;
		if (inputY > 127) inputY = 127;
		if (inputY < -127) inputY = -127;

		// --- Step 2: Scale to [-127, 127] (lines 82-84) ---
		// Mouse mode: local_c = (DAT_0047a7e0 * 0x7f) / 0xa0
		int16 local_c = (int16)((inputX * 127) / 160);
		int16 local_14 = inputY;  // Y already in [-127, 127]

		// --- Step 3: Velocity history + smoothed average (lines 141-157) ---
		for (int i = 24; i > 0; i--) {
			_velocityHistory[i] = _velocityHistory[i - 1];
		}
		_velocityHistory[0] = local_c;

		// Window size = (levelData[0] >> 4) + 1. Calibrated default: 5.
		const int smoothWindow = 5;
		int velSum = 0;
		for (int i = 0; i < smoothWindow; i++) {
			velSum += _velocityHistory[i];
		}
		_smoothedVelocity = (int16)(velSum / smoothWindow);  // DAT_0044370c

		// --- Step 4: Wind history (lines 158-173) ---
		// Wind multiplier comes from level data[6]. Without data, use 0 (no wind).
		const int16 windMult = 0;
		int windSumX = 0, windSumY = 0;
		for (int i = 14; i > 0; i--) {
			_windHistoryX[i] = _windHistoryX[i - 1];
			windSumX += _windHistoryX[i];
		}
		_windHistoryX[0] = _windParamX;
		int16 windEffectX = (int16)((windMult * (windSumX + _windParamX)) / 15);

		for (int i = 14; i > 0; i--) {
			_windHistoryY[i] = _windHistoryY[i - 1];
			windSumY += _windHistoryY[i];
		}
		_windHistoryY[0] = _windParamY;
		int16 windEffectY = (int16)((windMult * (windSumY + _windParamY)) / 15);

		// --- Step 5: Position delta (lines 174-242) ---
		// levelSpeed (offset 4): calibrated so max velocity (127) → delta 12.
		//   8 = (speed * 127) >> 9 → speed ≈ 32
		// levelYSpeed (offset 2): calibrated so max input (127) → delta ~6.
		//   6 = (speed * 127) >> 10 → speed ≈ 48
		const int16 levelSpeed = 32;
		const int16 levelYSpeed = 48;
		int16 absSmoothVel = ABS(_smoothedVelocity);
		int16 positionDeltaX;

		if (_flyControlMode == 1) {
			// Mode 1: Full cross-axis coupling (lines 174-186)
			// Banking: vertical input deflects horizontal movement
			if (local_c < 1) {
				positionDeltaX = (int16)((levelSpeed * _smoothedVelocity - absSmoothVel * local_14 - windEffectX) >> 9);
			} else {
				positionDeltaX = (int16)((levelSpeed * _smoothedVelocity + absSmoothVel * local_14 - windEffectX) >> 9);
			}
		} else {
			// Mode 0/2/3: Reduced cross-axis coupling (lines 218-230)
			if (local_c < 1) {
				positionDeltaX = (int16)((levelSpeed * _smoothedVelocity - (absSmoothVel * local_14 >> 2) - windEffectX) >> 9);
			} else {
				positionDeltaX = (int16)((levelSpeed * _smoothedVelocity + (absSmoothVel * local_14 >> 2) - windEffectX) >> 9);
			}
		}

		// Clamp X delta to ±12 per frame (lines 187-192 / 231-236)
		if (positionDeltaX < -11) positionDeltaX = -12;
		if (positionDeltaX > 11) positionDeltaX = 12;

		// Apply X delta (line 193 / 237)
		_flyShipScreenX += positionDeltaX;

		// Y delta
		if (_flyControlMode == 1) {
			// Mode 1: clamped to ±12 with wind (lines 194-216)
			int yCalc = levelYSpeed * local_14 - (windEffectY >> 1);
			int yDelta = yCalc >> 10;
			if (yDelta < -12) yDelta = -12;
			if (yDelta > 12) yDelta = 12;
			_flyShipScreenY -= (int16)yDelta;
		} else {
			// Mode 0/2/3: unclamped (lines 238-241)
			_flyShipScreenY -= (int16)((levelYSpeed * local_14) >> 10);
		}

		// Store vertical input for direction sprite (line 243)
		_verticalInput = local_14;  // DAT_0044370e

		// Ship facing direction (line 244)
		_facingRight = (0xd4 < _smoothedVelocity + _flyShipScreenX);

		// --- Step 6: Position clamping (lines 245-256) ---
		if (_flyShipScreenX > 0x194) _flyShipScreenX = 0x194;  // 404
		if (_flyShipScreenY > 0xF0) _flyShipScreenY = 0xF0;    // 240
		if (_flyShipScreenX < 0x14) _flyShipScreenX = 0x14;    // 20
		if (_flyShipScreenY < 0x14) _flyShipScreenY = 0x14;    // 20

		// --- Step 7: Corridor collision — mode 0/2 only (lines 257-292) ---
		if (_flyControlMode == 0 || _flyControlMode == 2) {
			// Right boundary (lines 258-270)
			// Original: position is ALWAYS clamped; damage/bounce only when cooldown < 5
			if (_corridorRightX < _flyShipScreenX) {
				_flyShipScreenX = _corridorRightX;
				if (_hitCooldown < 5) {
					for (int i = 0; i < 25; i++) _velocityHistory[i] = -127;
					_hitCooldown = 10;
					_spaceShotDirection = 1;
					initDamageFlash();
					if (!_rebelInvulnerable) {
						int damage = 3 + (_difficulty * 2);
						_playerDamage += damage;
						if (_playerDamage > 255) _playerDamage = 255;
					}
					_rebelHitCounter++;
					playSfx(1, 127, 100);  // CRASH.SAD, right wall → pan right
				}
			}
			// Left boundary (lines 271-283)
			if (_flyShipScreenX < _corridorLeftX) {
				_flyShipScreenX = _corridorLeftX;
				if (_hitCooldown < 5) {
					for (int i = 0; i < 25; i++) _velocityHistory[i] = 127;
					_hitCooldown = 10;
					_spaceShotDirection = 0;
					initDamageFlash();
					if (!_rebelInvulnerable) {
						int damage = 3 + (_difficulty * 2);
						_playerDamage += damage;
						if (_playerDamage > 255) _playerDamage = 255;
					}
					_rebelHitCounter++;
					playSfx(1, 127, -100);  // CRASH.SAD, left wall → pan left
				}
			}
			// Y boundary clamping — no damage (lines 285-292)
			if (_corridorBottomY < _flyShipScreenY) {
				_flyShipScreenY = _corridorBottomY;
			}
			if (_flyShipScreenY < _corridorTopY) {
				_flyShipScreenY = _corridorTopY;
			}
		}

		// --- Step 8: Perspective offsets (lines 293-316) ---
		// f(x) = (focal * center * |offset|) / ((center - focal) * |offset| + focal * center)
		// Close view (DAT_0047a7fc < 1): focalX=0x34, focalY=0x2d
		// Far view (DAT_0047a7fc >= 1): focalX=0x2b, focalY=0x19
		{
			int absOffX = ABS(_flyShipScreenX - 0xd4);
			int16 focalX = 0x2b;  // Far view default for Level 3
			if (absOffX > 0) {
				_perspectiveX = (int16)((focalX * 0xd4 * absOffX) /
					((0xd4 - focalX) * absOffX + focalX * 0xd4));
			} else {
				_perspectiveX = 0;
			}
			if (_flyShipScreenX < 0xd5) _perspectiveX = -_perspectiveX;

			int absOffY = ABS(_flyShipScreenY - 0x82);
			int16 focalY = 0x19;  // Far view default for Level 3
			if (absOffY > 0) {
				_perspectiveY = (int16)((focalY * 0x82 * absOffY) /
					((0x82 - focalY) * absOffY + focalY * 0x82));
			} else {
				_perspectiveY = 0;
			}
			if (_flyShipScreenY < 0x83) _perspectiveY = -_perspectiveY;
		}

		// View shift = clamped smoothed velocity (FUN_0040d836 lines 68-74)
		_viewShift = _smoothedVelocity;
		if (_viewShift > 127) _viewShift = 127;
		if (_viewShift < -127) _viewShift = -127;

		// --- Step 9: Direction sprite (FUN_0040d836 lines 88-106) ---
		// 5x7 grid: vDir(0-4) * 7 + hDir(0-6) = sprite index (0-34)
		// vDir from vertical input: (0xa0 - verticalInput) >> 6
		int16 vDir = (int16)(((int)(0xa0 - _verticalInput) + ((0xa0 - _verticalInput) < 0 ? 63 : 0)) >> 6);
		if (vDir < 0) vDir = 0;
		if (vDir > 4) vDir = 4;

		// hDir from smoothed velocity: (0x95 - smoothedVelocity) / 0x2b
		int16 hDir = (int16)((0x95 - _smoothedVelocity) / 0x2b);
		if (hDir < 0) hDir = 0;
		if (hDir > 6) hDir = 6;

		// Hysteresis at center (lines 90-97, 98-105)
		if (hDir == 3 && ABS(_smoothedVelocity) > 10) {
			hDir = (_smoothedVelocity < 1) ? 4 : 2;
		}
		if (vDir == 2 && ABS(_verticalInput) > 15) {
			vDir = (_verticalInput < 1) ? 3 : 1;
		}

		_shipDirectionIndex = vDir * 7 + hDir;
		if (_shipDirectionIndex < 0) _shipDirectionIndex = 0;
		if (_shipDirectionIndex > 34) _shipDirectionIndex = 34;

		_shipFiring = (_flyControlMode == 2) && (_vm->VAR(_vm->VAR_LEFTBTN_HOLD) != 0);

		debug("Rebel2 H7: pos=(%d,%d) vel=%d vIn=%d dx=%d dir=%d mode=%d",
			_flyShipScreenX, _flyShipScreenY, _smoothedVelocity,
			_verticalInput, positionDeltaX, _shipDirectionIndex, _flyControlMode);

		return;
	}

	// Handler 25 (0x19) specific logic (mixed mode - speeder bike)
	// Based on FUN_0041cadb case 4 (opcode 6) lines 113-229
	if (_rebelHandler == 25) {
		// Read the reset flag from IACT data at offset 8-9 (local_14[4] in decompiled code)
		// The stream position should be at offset 8 after par4 was read
		// From FUN_0041cadb line 114: if (local_14[4] == 1) { ... reset ... }
		int16 par5 = 0;
		if (b.pos() + 2 <= b.size()) {
			int64 savedPos = b.pos();
			par5 = b.readSint16LE();
			b.seek(savedPos);  // Don't consume the stream
		}

		// If par5 == 1, enable status bar and reset state (lines 114-121)
		// Note: This is local_14[4] in the decompiled code, NOT local_14[3] (par4)
		if (par5 == 1) {
			_rebelStatusBarSprite = 5;
			// Clear ALL iactBits — matches FUN_00423880 calling FUN_00423a00(0)
			// at IACT callback registration time. Each new wave video starts with
			// a clean bit table so enemy IDs reused across videos work correctly.
			clearBit(0);
			// Reset link tables (DAT_0045797c through DAT_0045917c)
			for (int i = 0; i < 512; i++) {
				_rebelLinks[i][0] = 0;
				_rebelLinks[i][1] = 0;
				_rebelLinks[i][2] = 0;
			}
			// Reset wave state to accumulated phase state (same as Handler 8)
			// DAT_0047ab98 = DAT_0047ab9c: ensures new wave starts with correct state
			_rebelWaveState = _rebelPhaseState;
			debug("Rebel2 Opcode 6 (Handler 25): Status bar enabled, state reset, wave=0x%x autopilot=%d damageLevel=%d",
				_rebelWaveState, _rebelAutopilot, _rebelDamageLevel);
		}

		// Set sprite mode (DAT_00457900 = local_14[3]) - controls which GRD sprite to render
		// From FUN_0041cadb line 122: DAT_00457900 = local_14[3];
		// In ScummVM's IACT parsing: local_14[3] = offset 6-7 = par4
		// Mode 1: Uncovered, shooting position - sprite on left
		// Mode 2: Covered, vertical shift
		// Mode 3: Transition between covered/uncovered - sprite position depends on direction
		// Mode 4: Alternative uncovered position - sprite on right
		_grdSpriteMode = par4;  // local_14[3] maps to par4 (offset 6-7)

		debug("Rebel2 Handler25 Opcode6: par2=%d par3=%d par4=%d(mode) par5=%d(reset) autopilot=%d damageLevel=%d controlMode=%d",
			par2, par3, par4, par5, _rebelAutopilot, _rebelDamageLevel, _rebelControlMode);

		// Autopilot logic (lines 123-146)
		// From original FUN_0041cadb - NO damageLevel check, toggle happens immediately
		// The damage level counter provides the smooth visual transition
		if (!_rebelInvulnerable) {
			if (_rebelAutopilot == 0) {
				// Uncovered: RIGHT button enters cover
				if ((_rebelControlMode & 2) != 0) {
					_rebelAutopilot = 1;
					debug("Rebel2 Handler25: Entering cover (right click), controlMode=%d", _rebelControlMode);
				}
			} else {
				// Covered: ANY button exits cover
				if (_rebelControlMode != 0) {
					_rebelAutopilot = 0;
					debug("Rebel2 Handler25: Exiting cover (button click), controlMode=%d", _rebelControlMode);
				}
			}
			// Clear control mode after processing (sticky flags consumed)
			_rebelControlMode = 0;
		} else {
			// Invulnerable mode: random autopilot changes
			if (_rebelAutopilot == 0) {
				if (_vm->_rnd.getRandomNumber(100) == 0) {
					_rebelAutopilot = 1;
				}
			} else {
				if (_vm->_rnd.getRandomNumber(15) == 0) {
					_rebelAutopilot = 0;
					_rebelFlightDir = _vm->_rnd.getRandomNumber(2);
				}
			}
		}

		// Update damage level counter (lines 147-154)
		// This provides the smooth transition animation between covered/uncovered states
		int prevDamageLevel = _rebelDamageLevel;
		if (_rebelAutopilot == 0) {
			// Uncovered: decrement damage level towards 0
			if (_rebelDamageLevel > 0) {
				_rebelDamageLevel--;
			}
		} else {
			// Covered: increment damage level towards 5
			if (_rebelDamageLevel < 5) {
				_rebelDamageLevel++;
			}
		}
		if (_rebelDamageLevel != prevDamageLevel) {
			debug("Rebel2 Handler25: damageLevel transition %d -> %d (autopilot=%d)",
				prevDamageLevel, _rebelDamageLevel, _rebelAutopilot);
		}

		// Flight direction logic for mode 3 (lines 155-177)
		if (_grdSpriteMode == 3) {
			if (_rebelDamageLevel == 5) {
				// At max damage, check for direction change input
				// For now, use mouse X position to determine direction
				int16 mouseX = _vm->_mouse.x;
				if (_player && _player->_width > 320) {
					mouseX = (mouseX * 320) / _player->_width;
				}
				if (mouseX > 235) {  // 0x4b + 160 = 235
					_rebelFlightDir = 1;
				}
				if (mouseX < 85) {   // 160 - 0x4b = 85
					_rebelFlightDir = 0;
				}
			}
		} else {
			_rebelFlightDir = 0;
		}

		// Calculate sprite and view offset positions based on mode (lines 182-213)
		// DAT_0045790c = view offset X (for corridor overlay)
		// DAT_0045790e = view offset Y (for corridor overlay)
		// DAT_00457910 = sprite position X (relative to center)
		// DAT_00457912 = sprite position Y (relative to center)
		if (_grdSpriteMode == 1) {
			// Mode 1: Uncovered, shooting - sprite shifts left as damage increases
			_rebelViewMode1 = 0x0e;
			_rebelViewMode2 = 0;
			_rebelViewOffsetX = _rebelDamageLevel * -5 + -14;   // DAT_0045790c
			_rebelViewOffset2X = _rebelDamageLevel * -22;       // DAT_00457910
			_rebelViewOffsetY = 0;                              // DAT_0045790e
			_rebelViewOffset2Y = 0;                             // DAT_00457912
		} else if (_grdSpriteMode == 4) {
			// Mode 4: Alternative uncovered - sprite shifts right
			_rebelViewMode1 = 0x22;
			_rebelViewMode2 = 0;
			_rebelViewOffsetX = _rebelDamageLevel * 10 + -16;   // DAT_0045790c
			_rebelViewOffset2X = _rebelDamageLevel * 17 + -85;  // DAT_00457910 (0x11 = 17, -0x55 = -85)
			_rebelViewOffsetY = 0;
			_rebelViewOffset2Y = 0;
		} else if (_grdSpriteMode == 2) {
			// Mode 2: Covered - vertical shift
			_rebelViewMode1 = 0;
			_rebelViewMode2 = 0x0e;
			_rebelViewOffsetY = _rebelDamageLevel * -5 + -14;   // DAT_0045790e
			_rebelViewOffset2Y = (5 - _rebelDamageLevel) * 15 + -60;  // DAT_00457912 (0xf = 15, -0x3c = -60)
			_rebelViewOffsetX = 0;
			_rebelViewOffset2X = 0;
		} else if (_grdSpriteMode == 3) {
			// Mode 3: Transition - direction-dependent horizontal shift
			_rebelViewMode1 = 0x0f;
			_rebelViewMode2 = 0;
			// (-(DAT_00457902 == 0) & 6) - 3 = if dir==0: 6-3=3, else 0-3=-3
			int16 dirMultX = (_rebelFlightDir == 0) ? 3 : -3;
			// (-(DAT_00457902 == 0) & 0x28) - 0x14 = if dir==0: 40-20=20, else 0-20=-20
			int16 dirMultX2 = (_rebelFlightDir == 0) ? 20 : -20;
			_rebelViewOffsetX = dirMultX * (5 - _rebelDamageLevel) + -15;  // DAT_0045790c
			_rebelViewOffset2X = dirMultX2 * (5 - _rebelDamageLevel);      // DAT_00457910
			_rebelViewOffsetY = 0;
			_rebelViewOffset2Y = 0;
		} else {
			// Mode 0 or unknown: use Mode 1 defaults as fallback
			_rebelViewMode1 = 0x0e;
			_rebelViewMode2 = 0;
			_rebelViewOffsetX = _rebelDamageLevel * -5 + -14;
			_rebelViewOffset2X = _rebelDamageLevel * -22;
			_rebelViewOffsetY = 0;
			_rebelViewOffset2Y = 0;
			debug("Rebel2 Opcode 6 (Handler 25): Unknown mode %d, using Mode 1 fallback", _grdSpriteMode);
		}

		debug("Rebel2 Opcode 6 (Handler 25): mode=%d damage=%d dir=%d autopilot=%d viewOff=(%d,%d) spritePos=(%d,%d)",
			_grdSpriteMode, _rebelDamageLevel, _rebelFlightDir, _rebelAutopilot,
			_rebelViewOffsetX, _rebelViewOffsetY, _rebelViewOffset2X, _rebelViewOffset2Y);

		// Set FOBJ position offsets (FUN_00424510 in original, line 214)
		// All subsequent FOBJs in this frame will be shifted by these offsets
		if (_player) {
			_player->_fobjOffsetX = _rebelViewOffsetX;
			_player->_fobjOffsetY = _rebelViewOffsetY;
		}

		// Draw corridor overlay OPAQUELY (FUN_00428A10 in original, line 216)
		// This wipes previous frame content so codec 23 delta skip regions show clean corridor
		if (renderBitmap) {
			EmbeddedSanFrame &corridorOverlay = _rebelEmbeddedHud[4];
			if (corridorOverlay.valid && corridorOverlay.pixels) {
				int pitch = (_player && _player->_width > 0) ? _player->_width : 320;
				int bufHeight = (_player && _player->_height > 0) ? _player->_height : 200;

				int srcOffsetX = 0;
				int srcOffsetY = 0;
				int destX = _rebelViewOffsetX;
				int destY = _rebelViewOffsetY;
				int drawWidth = corridorOverlay.width;
				int drawHeight = corridorOverlay.height;

				if (destX < 0) { srcOffsetX = -destX; drawWidth -= srcOffsetX; destX = 0; }
				if (destY < 0) { srcOffsetY = -destY; drawHeight -= srcOffsetY; destY = 0; }
				if (destX + drawWidth > pitch) drawWidth = pitch - destX;
				if (destY + drawHeight > bufHeight) drawHeight = bufHeight - destY;
				if (drawWidth > corridorOverlay.width - srcOffsetX) drawWidth = corridorOverlay.width - srcOffsetX;
				if (drawHeight > corridorOverlay.height - srcOffsetY) drawHeight = corridorOverlay.height - srcOffsetY;

				if (drawWidth > 0 && drawHeight > 0) {
					for (int y = 0; y < drawHeight; y++) {
						memcpy(renderBitmap + (destY + y) * pitch + destX,
							   corridorOverlay.pixels + (srcOffsetY + y) * corridorOverlay.width + srcOffsetX,
							   drawWidth);
					}
				}
				debug("Rebel2 Opcode 6: Corridor overlay drawn at (%d,%d) size(%d,%d)",
					_rebelViewOffsetX, _rebelViewOffsetY, corridorOverlay.width, corridorOverlay.height);
			}
		}

		return;
	}

	// Step 1: If par4 == 1, initialize/reset state (lines 114-121)
	if (par4 == 1) {
		// Draw status bar sprite 5 (FUN_0040bb87 equivalent)
		_rebelStatusBarSprite = (_rebelLevelType == 5) ? 53 : 5;
		debug("Rebel2 Opcode 6: Status Bar ENABLED - sprite %d", _rebelStatusBarSprite);

		// Clear ALL iactBits — matches FUN_00423880 calling FUN_00423a00(0)
		clearBit(0);

		// Clear link tables (DAT_0045797c through DAT_0045917c)
		for (int i = 0; i < 512; i++) {
			_rebelLinks[i][0] = 0;
			_rebelLinks[i][1] = 0;
			_rebelLinks[i][2] = 0;
		}

		// DAT_0047ab98 = DAT_0047ab9c: At the start of each wave video,
		// reset wave state to accumulated phase state. Enemies killed in
		// previous waves stay killed; new kills add during this wave.
		_rebelWaveState = _rebelPhaseState;
		_rebelHitCounter = 0;
		debug("Rebel2 Opcode 6: Wave state reset to phase state 0x%x", _rebelWaveState);
	}

	// Step 2: Set level type (DAT_00457900 = par3)
	_rebelLevelType = par3;

	// Step 3: Autopilot/control mode logic (lines 123-146)
	// This determines whether the ship flies on autopilot or manual control
	if (!_rebelInvulnerable) {
		// Normal mode: check control mode flags
		if (_rebelAutopilot == 0) {
			if ((_rebelControlMode & 2) != 0) {
				_rebelAutopilot = 1;
			}
		} else {
			if (_rebelControlMode != 0) {
				_rebelAutopilot = 0;
			}
		}
	} else {
		// Invulnerable mode: random autopilot changes
		if (_rebelAutopilot == 0) {
			if (_vm->_rnd.getRandomNumber(100) == 0) {
				_rebelAutopilot = 1;
			}
		} else {
			if (_vm->_rnd.getRandomNumber(15) == 0) {
				_rebelAutopilot = 0;
				_rebelFlightDir = _vm->_rnd.getRandomNumber(2);
			}
		}
	}

	// Step 4: Update damage level counter (lines 147-154)
	if (_rebelAutopilot == 0) {
		if (_rebelDamageLevel > 0) {
			_rebelDamageLevel--;
		}
	} else {
		if (_rebelDamageLevel < 5) {
			_rebelDamageLevel++;
		}
	}

	// Handle level type 3 special direction logic (lines 155-181)
	if (_rebelLevelType == 3) {
		if (_rebelDamageLevel == 5) {
			// Check for joystick/key input to change direction
			// Simplified: use mouse position
			if (_vm->_mouse.x > 75) {
				_rebelFlightDir = 1;
			}
			if (_vm->_mouse.x < -75) {
				_rebelFlightDir = 0;
			}
		}
	} else {
		_rebelFlightDir = 0;
	}

	// Step 5: Calculate view offsets based on level type (lines 182-213)
	switch (_rebelLevelType) {
	case 1:
		// Type 1: Vertical movement
		_rebelViewMode1 = 0x0e;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * -5 - 0x0e;
		_rebelViewOffset2X = _rebelDamageLevel * -0x16;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	case 4:
		// Type 4: Different vertical movement
		_rebelViewMode1 = 0x22;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = _rebelDamageLevel * 10 - 0x10;
		_rebelViewOffset2X = _rebelDamageLevel * 0x11 - 0x55;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	case 2:
		// Type 2: Horizontal movement
		_rebelViewMode1 = 0;
		_rebelViewMode2 = 0x0e;
		_rebelViewOffsetY = _rebelDamageLevel * -5 - 0x0e;
		_rebelViewOffset2Y = (5 - _rebelDamageLevel) * 0x0f - 0x3c;
		_rebelViewOffsetX = 0;
		_rebelViewOffset2X = 0;
		break;

	case 3:
		// Type 3: Direction-based movement
		_rebelViewMode1 = 0x0f;
		_rebelViewMode2 = 0;
		{
			int dirFactor = (_rebelFlightDir == 0) ? 3 : -3;  // (-(ushort)(DAT_00457902 == 0) & 6) - 3
			int dirFactor2 = (_rebelFlightDir == 0) ? 0x14 : -0x14;  // (-(ushort)(DAT_00457902 == 0) & 0x28) - 0x14
			_rebelViewOffsetX = dirFactor * (5 - _rebelDamageLevel) - 0x0f;
			_rebelViewOffset2X = dirFactor2 * (5 - _rebelDamageLevel);
		}
		_rebelViewOffsetY = 0;
		_rebelViewOffset2Y = 0;
		break;

	default:
		// Default: No special offsets
		_rebelViewMode1 = 0;
		_rebelViewMode2 = 0;
		_rebelViewOffsetX = 0;
		_rebelViewOffsetY = 0;
		_rebelViewOffset2X = 0;
		_rebelViewOffset2Y = 0;
		break;
	}

	debug("Rebel2 Opcode 6: levelType=%d autopilot=%d damageLevel=%d viewOffset=(%d,%d)",
		_rebelLevelType, _rebelAutopilot, _rebelDamageLevel, _rebelViewOffsetX, _rebelViewOffsetY);

	// Detect and load embedded ANIM (SAN) within the remaining IACT payload
	// Note: chunkSize is the remaining IACT payload size after par1-par4 header
	{
		int64 startPos = b.pos();
		// Use chunkSize (remaining IACT payload) rather than b.size() (entire FRME stream)
		int64 remaining = chunkSize;
		if (remaining > 0) {
			int scanSize = (int)MIN<int64>(remaining, 65536);
			byte *scanBuf = (byte *)malloc(scanSize);
			if (scanBuf) {
				int bytesRead = b.read(scanBuf, scanSize);
				for (int i = 0; i + 8 <= bytesRead; ++i) {
					if (READ_BE_UINT32(scanBuf + i) == MKTAG('A','N','I','M')) {
						int64 animStreamPos = startPos + i;
						uint32 animReportedSize = READ_BE_UINT32(scanBuf + i + 4);
						// Limit to remaining IACT payload (chunkSize - offset into payload)
						int32 toCopy = (int)MIN<int64>((int64)animReportedSize + 8, chunkSize - i);
						if (toCopy > 0) {
							byte *animData = (byte *)malloc(toCopy);
							if (animData) {
								b.seek(animStreamPos);
								b.read(animData, toCopy);
								loadEmbeddedSan(par4, animData, toCopy, renderBitmap);
								free(animData);
							}
						}
						b.seek(startPos);
						free(scanBuf);
						return;
					}
				}
				b.seek(startPos);
				free(scanBuf);
			}
		}
	}
}

void InsaneRebel2::iactRebel2Opcode8(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4) {
	// Opcode 8: HUD/Ship resource loading
	// Dispatches to handler-specific loading functions based on current handler and parameters.
	//
	// Handler-specific routing (based on retail disassembly):
	//   Handler 7 (FUN_0040c3cc):  FLY NUT sprites via par4 (1, 2, 3, 11)
	//   Handler 8 (FUN_00401234):  POV NUT sprites via par3 (1, 3, 6, 7) or background via par4=5
	//   Handler 0x26 (FUN_00407fcb): Turret HUD NUT via par3 (1-4)
	//   Handler 0x19: Mixed turret mode, similar to 0x26
	//
	// Sound loading: par3 in range 21-47

	debug("Rebel2 IACT Opcode 8: handler=%d par2=%d par3=%d par4=%d (gameState=%d)",
		_rebelHandler, par2, par3, par4, _gameState);

	int64 startPos = b.pos();
	int64 remaining = (chunkSize > 0) ? chunkSize : (b.size() - startPos);

	// ===== Handler 7: FLY NUT Loading (Third-Person Ship) =====
	// FUN_0040c3cc case 6: par4 determines FLY sprite slot
	bool isHandler7FLY = (_rebelHandler == 7 && (par4 == 1 || par4 == 2 || par4 == 3 || par4 == 11));
	if (isHandler7FLY && remaining >= 14) {
		if (loadHandler7FlySprites(b, remaining, par4)) {
			b.seek(startPos);
			return;
		}
		b.seek(startPos);
	}

	// ===== Sound Loading (par3 21-47) =====
	if (par3 >= 21 && par3 <= 47) {
		debug("Rebel2 Opcode 8: Sound loading subcase %d (not implemented)", par3);
		// TODO: Implement sound loading via FUN_004118df equivalent
		return;
	}

	// ===== Scan for embedded ANIM data =====
	// Remaining handlers require finding ANIM tag in the stream
	debug("Rebel2 Opcode 8: Scanning for ANIM tag (startPos=%lld remaining=%lld)",
		(long long)startPos, (long long)remaining);

	if (remaining <= 0) {
		return;
	}

	int scanSize = (int)MIN<int64>(remaining, 65536);
	byte *scanBuf = (byte *)malloc(scanSize);
	if (!scanBuf) {
		return;
	}

	int bytesRead = b.read(scanBuf, scanSize);
	debug("Rebel2 Opcode 8: Read %d bytes for ANIM scan", bytesRead);

	// Find ANIM tag
	int animOffset = -1;
	for (int i = 0; i + 8 <= bytesRead; ++i) {
		if (READ_BE_UINT32(scanBuf + i) == MKTAG('A','N','I','M')) {
			animOffset = i;
			debug("Rebel2 Opcode 8: Found ANIM at offset %d", i);
			break;
		}
	}

	if (animOffset < 0) {
		debug("Rebel2 Opcode 8: No ANIM tag found");
		free(scanBuf);
		b.seek(startPos);
		return;
	}

	// Extract ANIM data
	uint32 animReportedSize = READ_BE_UINT32(scanBuf + animOffset + 4);
	int32 animDataSize = (int)MIN<int64>((int64)animReportedSize + 8, remaining - animOffset);
	if (animDataSize <= 0) {
		free(scanBuf);
		b.seek(startPos);
		return;
	}

	byte *animData = (byte *)malloc(animDataSize);
	if (!animData) {
		free(scanBuf);
		b.seek(startPos);
		return;
	}

	b.seek(startPos + animOffset);
	b.read(animData, animDataSize);

	bool handled = false;

	// ===== Handler 0x26/0x19: Turret HUD Overlays =====
	// FUN_00407fcb case 8: par3 1-4 for HUD NUT loading
	if (!handled && (_rebelHandler == 0x26 || _rebelHandler == 0x19)) {
		if (par3 >= 1 && par3 <= 4) {
			handled = loadTurretHudOverlay(animData, animDataSize, par3);
		}
	}

	// ===== Handler 8: POV Ship Sprites or Background =====
	// FUN_00401234 case 6: par4 selects POV NUT type (1,3,6,7) or background (5)
	// NOTE: par3 is always 0 for Handler 8; par4 contains the actual sprite type
	if (!handled && _rebelHandler == 8) {
		// Check for background loading first (par4=5)
		if (par4 == 5) {
			handled = loadLevel2Background(animData, animDataSize, renderBitmap);
		}
		// Check for POV NUT sprites (par4=1,3,6,7)
		else if (par4 == 1 || par4 == 3 || par4 == 6 || par4 == 7) {
			handled = loadHandler8ShipSprites(animData, animDataSize, par4);
		}
	}

	// ===== Handler 25 (0x19): Level 2 GRD Ship Sprites and Background =====
	// FUN_0041cadb case 6 (opcode 8): Uses PAR4 for switch selection
	//   par4=1: GRD001 - Primary ship sprite -> DAT_00482240 / _grd001Sprite
	//   par4=2: GRD002 - Secondary ship sprite -> DAT_00482238 / _grd002Sprite
	//   par4=4: 350x230 corridor overlay -> DAT_00482268, draws immediately
	//   par4=5: 320x200 background -> DAT_0048226c
	//   par4=6: Overlay -> DAT_00482250, draws immediately
	//   par4=7: Overlay -> DAT_00482248, draws immediately
	if (!handled && _rebelHandler == 25) {
		if (par4 == 1 || par4 == 2) {
			// GRD ship sprites - load into NutRenderer for per-frame rendering
			handled = loadHandler25GrdSprites(animData, animDataSize, par4);
		} else if (par4 == 5) {
			// Background (320x200) - stored for per-frame restoration
			handled = loadLevel2Background(animData, animDataSize, renderBitmap);
		} else if (par4 == 4 || par4 == 6 || par4 == 7) {
			// Overlays - draw immediately to renderBitmap
			// These complete the visual scene along with the background
			debug("Rebel2 Opcode 8: Handler 25 overlay par4=%d - drawing to screen", par4);
			loadEmbeddedSan(par4, animData, animDataSize, renderBitmap);
			handled = true;
		}
	}

	// ===== Fallback: Embedded SAN HUD overlays =====
	// For other cases, load as embedded SAN frame to HUD overlay slots
	if (!handled) {
		// Skip high-res data (par3 == 2, 4)
		if (par3 == 2 || par3 == 4) {
			debug("Rebel2 Opcode 8: Skipping high-res HUD par3=%d", par3);
			handled = true;
		} else {
			// Determine userId: Handler 0x19 uses par3, others use par4
			// Heuristic: if par3 is valid GRD range (1-13) and par4 is invalid, prefer par3
			int userId;
			bool usePar3 = (_rebelHandler == 0x19);
			if (!usePar3 && par3 >= 1 && par3 <= 13 && (par4 <= 0 || par4 >= 1000)) {
				usePar3 = true;
			}
			userId = usePar3 ? par3 : par4;

			// Skip audio tracks (userId >= 1000)
			if (userId > 0 && userId < 1000) {
				debug("Rebel2 Opcode 8: Loading embedded SAN HUD userId=%d (handler=%d par3=%d par4=%d)",
					userId, _rebelHandler, par3, par4);
				loadEmbeddedSan(userId, animData, animDataSize, renderBitmap);
				handled = true;
			}
		}
	}

	if (!handled) {
		debug("Rebel2 Opcode 8: Unhandled case - handler=%d par3=%d par4=%d", _rebelHandler, par3, par4);
	}

	free(animData);
	free(scanBuf);
	b.seek(startPos);
}

// ======================= Opcode 8 Helper Functions =======================
// These helper functions are extracted from the original monolithic iactRebel2Opcode8
// to improve code readability and match the retail FUN_* function structure.

bool InsaneRebel2::loadHandler7FlySprites(Common::SeekableReadStream &b, int64 remaining, int16 par4) {
	// Handler 7 FLY NUT loading - FUN_0040c3cc case 6 (opcode 8)
	// IACT structure after par1-par4 (we're at offset +8):
	//   +0-5 (6 bytes): additional header
	//   +6-9 (4 bytes): NUT data size (little-endian)
	//   +10+: NUT data
	//
	// par4 values (param_5[3] - 1 in assembly):
	//   1 -> case 0: FLY001 - Ship direction sprites (DAT_0047fee8)
	//   2 -> case 1: FLY003 - Targeting overlay (DAT_0047fef8)
	//   3 -> case 2: FLY002 - Laser fire sprites (DAT_0047fef0)
	//  11 -> case 10: FLY004 - High-res alternative (DAT_0047ff00)

	if (remaining < 14) {
		return false;
	}

	// Read additional header and size from fixed offset
	byte header[10];
	if (b.read(header, 10) != 10) {
		return false;
	}

	debug("Rebel2 loadHandler7FlySprites: header bytes: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		header[0], header[1], header[2], header[3], header[4],
		header[5], header[6], header[7], header[8], header[9]);

	// Size is at offset 14 from IACT start = bytes 6-9 of our header buffer
	uint32 nutSize = READ_LE_UINT32(header + 6);
	debug("Rebel2 loadHandler7FlySprites: par4=%d nutSize=%u remaining=%lld",
		par4, nutSize, (long long)remaining);

	if (nutSize == 0 || nutSize > (uint32)(remaining - 10)) {
		return false;
	}

	byte *nutData = (byte *)malloc(nutSize);
	if (!nutData) {
		return false;
	}

	int bytesRead = b.read(nutData, nutSize);
	debug("Rebel2 loadHandler7FlySprites: Read %d/%u bytes, first 16: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		bytesRead, nutSize,
		bytesRead > 0 ? nutData[0] : 0, bytesRead > 1 ? nutData[1] : 0,
		bytesRead > 2 ? nutData[2] : 0, bytesRead > 3 ? nutData[3] : 0,
		bytesRead > 4 ? nutData[4] : 0, bytesRead > 5 ? nutData[5] : 0,
		bytesRead > 6 ? nutData[6] : 0, bytesRead > 7 ? nutData[7] : 0,
		bytesRead > 8 ? nutData[8] : 0, bytesRead > 9 ? nutData[9] : 0,
		bytesRead > 10 ? nutData[10] : 0, bytesRead > 11 ? nutData[11] : 0,
		bytesRead > 12 ? nutData[12] : 0, bytesRead > 13 ? nutData[13] : 0,
		bytesRead > 14 ? nutData[14] : 0, bytesRead > 15 ? nutData[15] : 0);

	if (bytesRead != (int)nutSize) {
		warning("Rebel2 loadHandler7FlySprites: Short read! Got %d expected %u", bytesRead, nutSize);
		free(nutData);
		return false;
	}

	// Verify ANIM header
	if (bytesRead >= 8) {
		uint32 animTag = READ_BE_UINT32(nutData);
		if (animTag != MKTAG('A','N','I','M')) {
			warning("Rebel2 loadHandler7FlySprites: No ANIM tag! Data may be corrupted");
			free(nutData);
			return false;
		}
	}

	// Load as NUT
	NutRenderer *newNut = new NutRenderer(_vm, nutData, bytesRead);
	if (!newNut || newNut->getNumChars() <= 0) {
		debug("Rebel2 loadHandler7FlySprites: NUT load failed for par4=%d", par4);
		delete newNut;
		free(nutData);
		return false;
	}

	debug("Rebel2 loadHandler7FlySprites: Loaded FLY NUT par4=%d with %d sprites",
		par4, newNut->getNumChars());

	// Assign to appropriate slot based on par4 (matches FUN_0040c3cc case 6 switch)
	bool assigned = true;
	switch (par4) {
	case 1:  // FLY001 - Ship direction sprites (35 frames)
		delete _flyShipSprite;
		_flyShipSprite = newNut;
		debug("Rebel2: _flyShipSprite set with %d sprites", newNut->getNumChars());
		break;
	case 2:  // FLY003 - Targeting overlay
		delete _flyTargetSprite;
		_flyTargetSprite = newNut;
		break;
	case 3:  // FLY002 - Laser fire sprites
		delete _flyLaserSprite;
		_flyLaserSprite = newNut;
		break;
	case 11: // FLY004 - High-res alternative
		delete _flyHiResSprite;
		_flyHiResSprite = newNut;
		break;
	default:
		delete newNut;
		assigned = false;
		break;
	}

	free(nutData);
	return assigned;
}

bool InsaneRebel2::loadTurretHudOverlay(byte *animData, int32 size, int16 par3) {
	// Handler 0x26/0x19 turret HUD overlay loading - FUN_00407fcb case 8
	// Resolution-dependent loading:
	//   par3 == 1: Low-res primary HUD (DAT_0047fe78 / _hudOverlayNut)
	//   par3 == 2: High-res primary HUD (skip in 320x200 mode)
	//   par3 == 3: Low-res secondary HUD (DAT_0047fe80 / _hudOverlay2Nut)
	//   par3 == 4: High-res secondary HUD (skip in 320x200 mode)

	if (!animData || size <= 0) {
		return false;
	}

	// ScummVM runs at 320x200 (low-res), skip high-res data
	if (par3 == 2 || par3 == 4) {
		debug("Rebel2 loadTurretHudOverlay: Skipping high-res HUD par3=%d (running in low-res mode)", par3);
		return true;  // Successfully "handled" by skipping
	}

	if (par3 != 1 && par3 != 3) {
		return false;  // Not a turret HUD slot
	}

	NutRenderer *newNut = new NutRenderer(_vm, animData, size);
	if (!newNut || newNut->getNumChars() <= 0) {
		debug("Rebel2 loadTurretHudOverlay: NUT load failed for par3=%d", par3);
		delete newNut;
		return false;
	}

	debug("Rebel2 loadTurretHudOverlay: Loaded turret HUD NUT par3=%d with %d sprites",
		par3, newNut->getNumChars());

	if (par3 == 1) {
		// Low-res primary HUD overlay
		delete _hudOverlayNut;
		_hudOverlayNut = newNut;
	} else {  // par3 == 3
		// Low-res secondary HUD overlay
		delete _hudOverlay2Nut;
		_hudOverlay2Nut = newNut;
	}

	return true;
}

bool InsaneRebel2::loadHandler8ShipSprites(byte *animData, int32 size, int16 par4) {
	// Handler 8 ship POV NUT loading - FUN_00401234 case 6 (opcode 8)
	// par4 values (from IACT data offset +6, NOT par3 which is always 0):
	//   1: POV001 - Primary ship sprite (DAT_0047e010 / _shipSprite)
	//   3: POV004 - Secondary ship sprite (DAT_0047e028 / _shipSprite2)
	//   6: POV002 - Ship overlay 1 (DAT_0047e020 / _shipOverlay1)
	//   7: POV003 - Ship overlay 2 (DAT_0047e018 / _shipOverlay2)

	if (!animData || size <= 0) {
		return false;
	}

	// Only handle valid POV sprite slots
	if (par4 != 1 && par4 != 3 && par4 != 6 && par4 != 7) {
		return false;
	}

	NutRenderer *newNut = new NutRenderer(_vm, animData, size);
	if (!newNut || newNut->getNumChars() <= 0) {
		debug("Rebel2 loadHandler8ShipSprites: NUT load failed for par4=%d", par4);
		delete newNut;
		return false;
	}

	debug("Rebel2 loadHandler8ShipSprites: Loaded ship NUT par4=%d with %d sprites",
		par4, newNut->getNumChars());

	switch (par4) {
	case 1:  // POV001 - Primary ship sprite
		delete _shipSprite;
		_shipSprite = newNut;
		break;
	case 3:  // POV004 - Secondary ship sprite
		delete _shipSprite2;
		_shipSprite2 = newNut;
		break;
	case 6:  // POV002 - Ship overlay 1
		delete _shipOverlay1;
		_shipOverlay1 = newNut;
		break;
	case 7:  // POV003 - Ship overlay 2
		delete _shipOverlay2;
		_shipOverlay2 = newNut;
		break;
	default:
		delete newNut;
		return false;
	}

	return true;
}

bool InsaneRebel2::loadHandler25GrdSprites(byte *animData, int32 size, int16 par4) {
	// Handler 25 GRD ship NUT loading - FUN_0041cadb case 6 (opcode 8)
	// par4 values (from IACT data offset +6):
	//   1: GRD001 - Primary ship sprite (DAT_00482240 / _grd001Sprite)
	//   2: GRD002 - Secondary ship sprite (DAT_00482238 / _grd002Sprite)

	if (!animData || size <= 0) {
		return false;
	}

	// Only handle valid GRD sprite slots
	if (par4 != 1 && par4 != 2) {
		return false;
	}

	NutRenderer *newNut = new NutRenderer(_vm, animData, size);
	if (!newNut || newNut->getNumChars() <= 0) {
		debug("Rebel2 loadHandler25GrdSprites: NUT load failed for par4=%d", par4);
		delete newNut;
		return false;
	}

	debug("Rebel2 loadHandler25GrdSprites: Loaded GRD NUT par4=%d with %d sprites",
		par4, newNut->getNumChars());

	switch (par4) {
	case 1:  // GRD001 - Primary ship sprite
		delete _grd001Sprite;
		_grd001Sprite = newNut;
		debug("Rebel2: _grd001Sprite set with %d sprites", newNut->getNumChars());
		break;
	case 2:  // GRD002 - Secondary ship sprite
		delete _grd002Sprite;
		_grd002Sprite = newNut;
		debug("Rebel2: _grd002Sprite set with %d sprites", newNut->getNumChars());
		break;
	default:
		delete newNut;
		return false;
	}

	return true;
}

bool InsaneRebel2::loadLevel2Background(byte *animData, int32 size, byte *renderBitmap) {
	// Level 2 background loading from embedded ANIM - FUN_00401234 case 5
	// par4=5 contains the background image embedded as ANIM with FOBJ codec 3
	// Creates 320x200 buffer (DAT_0047e030 / _level2Background)

	if (!animData || size < 8) {
		return false;
	}

	debug("Rebel2 loadLevel2Background: Loading Level 2 background (animSize=%d)", size);

	// Allocate background buffer if needed (320x200 = 64000 bytes)
	if (_level2Background == nullptr) {
		_level2Background = (byte *)malloc(320 * 200);
		if (!_level2Background) {
			return false;
		}
		memset(_level2Background, 0, 320 * 200);
	}

	// Parse embedded ANIM to find FOBJ
	// Structure: ANIM tag at offset 0, AHDR, then FRME with FOBJ
	int animOffset = 0;
	if (READ_BE_UINT32(animData) == MKTAG('A','N','I','M')) {
		uint32 animSize = READ_BE_UINT32(animData + 4);
		debug("Rebel2 loadLevel2Background: Found ANIM tag, size=%u", animSize);

		// Skip ANIM header (8 bytes) + AHDR chunk
		if (size >= 16 && READ_BE_UINT32(animData + 8) == MKTAG('A','H','D','R')) {
			uint32 ahdrSize = READ_BE_UINT32(animData + 12);
			animOffset = 8 + 8 + ahdrSize;  // After ANIM tag + AHDR
			debug("Rebel2 loadLevel2Background: AHDR size=%u, FRME expected at offset %d", ahdrSize, animOffset);
		}
	}

	// Look for FRME containing FOBJ
	bool foundBackground = false;
	for (int scanPos = animOffset; scanPos + 16 < size && !foundBackground; scanPos++) {
		if (READ_BE_UINT32(animData + scanPos) == MKTAG('F','R','M','E')) {
			int frmeSize = READ_BE_UINT32(animData + scanPos + 4);
			debug("Rebel2 loadLevel2Background: Found FRME at %d, size=%d", scanPos, frmeSize);

			for (int fobjPos = scanPos + 8; fobjPos + 18 < scanPos + 8 + frmeSize && fobjPos + 18 < size; fobjPos++) {
				if (READ_BE_UINT32(animData + fobjPos) == MKTAG('F','O','B','J')) {
					byte *fobjData = animData + fobjPos + 8;

					// FOBJ header: codec(2), x(2), y(2), w(2), h(2)
					int16 codec = READ_LE_INT16(fobjData);
					int16 fobjX = READ_LE_INT16(fobjData + 2);
					int16 fobjY = READ_LE_INT16(fobjData + 4);
					int16 fobjW = READ_LE_INT16(fobjData + 6);
					int16 fobjH = READ_LE_INT16(fobjData + 8);

					debug("Rebel2 loadLevel2Background: Found FOBJ: codec=%d pos=(%d,%d) size=%dx%d",
						codec, fobjX, fobjY, fobjW, fobjH);

					// Decode codec 3 (RLE) into background buffer
					// Use smushDecodeRLEOpaque to write ALL colors including color 0 (black).
					// The standard smushDecodeRLE treats color 0 as transparent, which causes
					// the background to appear as a "sketch" with black pixels missing.
					if (codec == 3 && fobjW > 0 && fobjH > 0 && fobjW <= 320 && fobjH <= 200) {
						byte *rleData = fobjData + 14;  // Skip full 14-byte FOBJ header
						smushDecodeRLEOpaque(_level2Background, rleData, fobjX, fobjY, fobjW, fobjH, 320);

						debug("Rebel2 loadLevel2Background: Decoded Level 2 background (%dx%d at %d,%d)",
							fobjW, fobjH, fobjX, fobjY);
						_level2BackgroundLoaded = true;
						foundBackground = true;

						// Copy to render bitmap immediately if provided
						if (renderBitmap) {
							for (int by = 0; by < 200; by++) {
								memcpy(renderBitmap + by * 320, _level2Background + by * 320, 320);
							}
							debug("Rebel2 loadLevel2Background: Copied to renderBitmap");
						}
					}
					break;
				}
			}
			break;
		}
	}

	if (!foundBackground) {
		debug("Rebel2 loadLevel2Background: Failed to find/decode background FOBJ");
	}

	return foundBackground;
}

void InsaneRebel2::iactRebel2Opcode9(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	// Opcode 9: Text/Subtitle Display via IACT chunk
	// Note: Most RA2 subtitles use TRES chunks handled by SmushPlayer::handleTextResource()
	// This opcode handles inline text in IACT chunks (less common)
	//
	// IACT Chunk Layout (par1-par4 already read by handleIACT):
	// +0x00 (2): opcode = 9 (par1, already read)
	// +0x02 (2): par2 (already read)
	// +0x04 (2): par3 (already read)
	// +0x06 (2): par4 (already read)
	// +0x08 onwards: Text data structure
	//
	// Text Data Structure:
	// +0x00 (2): X position
	// +0x02 (2): Y position
	// +0x04 (2): flags (bit 0=center, bit 1=right, bit 2=wrap, bit 3=difficulty gated)
	// +0x06 (2): clipX (when flag & 4)
	// +0x08 (2): clipY
	// +0x0A (2): clipW
	// +0x0C (2): clipH
	// +0x10 onwards: NUL-terminated text string

	int64 startPos = b.pos();

	// Check for "TRES" tag (0x54524553) indicating string resource lookup
	uint32 tag = b.readUint32BE();

	const char *textStr = nullptr;
	char textBuffer[512];
	int16 posX = 160;  // Default center position
	int16 posY = 150;  // Default bottom-ish position
	int16 textFlags = 1;  // Default: center aligned
	int16 clipX = 16, clipY = 16, clipW = 288, clipH = 168;

	if (tag == MKTAG('T','R','E','S')) {
		// String resource lookup via TRES tag
		// The string index follows after the tag
		int32 stringIndex = b.readSint32LE();

		// Try to get string from SMUSH player's string resource
		if (_player && _player->getString(stringIndex)) {
			textStr = _player->getString(stringIndex);
			debug("Rebel2 Opcode 9: TRES string index=%d -> \"%s\"", stringIndex, textStr);
		} else {
			debug("Rebel2 Opcode 9: TRES string index=%d not found", stringIndex);
			return;
		}

		// After TRES + index, read positioning data
		// The remaining data contains X, Y, flags etc.
		if (b.size() - b.pos() >= 14) {
			posX = b.readSint16LE();
			posY = b.readSint16LE();
			textFlags = b.readSint16LE();
			clipX = b.readSint16LE();
			clipY = b.readSint16LE();
			clipW = b.readSint16LE();
			clipH = b.readSint16LE();
		}
	} else {
		// Inline text data - go back and read positioning structure
		b.seek(startPos);

		// Read text data structure
		posX = b.readSint16LE();      // +0x00
		posY = b.readSint16LE();      // +0x02
		textFlags = b.readSint16LE(); // +0x04
		clipX = b.readSint16LE();     // +0x06
		clipY = b.readSint16LE();     // +0x08
		clipW = b.readSint16LE();     // +0x0A
		clipH = b.readSint16LE();     // +0x0C
		b.skip(2);                    // +0x0E padding

		// Read inline text string (NUL-terminated)
		int textLen = 0;
		while (textLen < (int)sizeof(textBuffer) - 1) {
			byte ch = b.readByte();
			if (ch == 0 || b.eos()) break;
			textBuffer[textLen++] = ch;
		}
		textBuffer[textLen] = '\0';
		textStr = textBuffer;

		debug("Rebel2 Opcode 9: Inline text at (%d,%d) flags=0x%x -> \"%s\"", posX, posY, textFlags, textStr);
	}

	if (!textStr || textStr[0] == '\0') {
		debug("Rebel2 Opcode 9: Empty text string, skipping");
		return;
	}

	// Check difficulty gate (flag bit 3 = 0x08)
	// If set, only show text if difficulty check passes (we skip this check for simplicity)
	// In retail: FUN_00425d30(0) is called

	// Get render buffer dimensions
	int width = (_player && _player->_width > 0) ? _player->_width : 320;
	int height = (_player && _player->_height > 0) ? _player->_height : 200;

	// Apply coordinate clamping (from FUN_004033cf disassembly)
	// Low-res: X clamped to [16, 304], Y clamped to [16, 196]
	if (posX < 16) posX = 16;
	if (posX > 304) posX = 304;
	if (posY < 16) posY = 16;
	if (posY > 196) posY = 196;

	// Use the message font loaded during initialization (DIHIFONT.NUT)
	if (!_rebelMsgFont) {
		debug("Rebel2 Opcode 9: No message font loaded (_rebelMsgFont is null)");
		return;
	}

	// Calculate clipping rectangle
	if (!(textFlags & 0x04)) {
		// No clip rect specified, use default full-screen clip
		clipX = 0;
		clipY = 0;
		clipW = width;
		clipH = height;
	}

	Common::Rect clipRect(
		MAX<int>(0, clipX),
		MAX<int>(0, clipY),
		MIN<int>(clipX + clipW, width),
		MIN<int>(clipY + clipH, height)
	);

	// Determine text alignment flags
	TextStyleFlags styleFlags = kStyleAlignLeft;
	if (textFlags & 0x01) {
		styleFlags = kStyleAlignCenter;
	} else if (textFlags & 0x02) {
		styleFlags = kStyleAlignRight;
	}
	if (textFlags & 0x04) {
		styleFlags = (TextStyleFlags)(styleFlags | kStyleWordWrap);
	}

	// Use white color (index 255) for subtitle text
	// The original uses colors from the palette, commonly white or yellow for subtitles
	int16 textColor = 255;

	// RA2 fonts (like DIHIFONT.NUT) have only 58 characters starting at ASCII 32 (space).
	// We need to convert ASCII codes to font indices by subtracting 32.
	// Character mapping: font index = ASCII code - 32
	// So 'D' (68) becomes index 36, 'A' (65) becomes index 33, etc.
	// IMPORTANT: Skip format codes (^f00, ^c255, ^l) which TextRenderer parses as raw ASCII.
	char convertedText[512];
	int srcLen = strlen(textStr);
	int dstIdx = 0;
	int numChars = _rebelMsgFont->getNumChars();

	for (int i = 0; i < srcLen && dstIdx < (int)sizeof(convertedText) - 1; i++) {
		byte ch = (byte)textStr[i];

		// Check for format codes (^f, ^c, ^l) - keep them as raw ASCII
		if (ch == '^' && i + 1 < srcLen) {
			byte next = (byte)textStr[i + 1];
			if (next == 'f' && i + 3 < srcLen) {
				// ^fXX - font switch (4 chars total)
				convertedText[dstIdx++] = textStr[i++];  // ^
				convertedText[dstIdx++] = textStr[i++];  // f
				convertedText[dstIdx++] = textStr[i++];  // X
				convertedText[dstIdx++] = textStr[i];    // X
				continue;
			} else if (next == 'c' && i + 4 < srcLen) {
				// ^cXXX - color switch (5 chars total)
				convertedText[dstIdx++] = textStr[i++];  // ^
				convertedText[dstIdx++] = textStr[i++];  // c
				convertedText[dstIdx++] = textStr[i++];  // X
				convertedText[dstIdx++] = textStr[i++];  // X
				convertedText[dstIdx++] = textStr[i];    // X
				continue;
			} else if (next == 'l') {
				// ^l - line break marker (2 chars)
				convertedText[dstIdx++] = textStr[i++];  // ^
				convertedText[dstIdx++] = textStr[i];    // l
				continue;
			} else if (next == '^') {
				// ^^ - escaped caret (becomes single ^)
				i++;  // Skip first ^
				// Fall through to convert second ^ as normal char
				ch = '^';
			}
		}

		// Convert regular characters from ASCII to font index
		// First convert lowercase to uppercase (the font likely only has uppercase)
		if (ch >= 'a' && ch <= 'z') {
			ch = ch - 'a' + 'A';  // Convert to uppercase
		}

		if (ch >= 32 && ch < (byte)(32 + numChars)) {
			convertedText[dstIdx++] = ch - 32;  // Convert ASCII to font index
		} else if (ch == '\n' || ch == '\r') {
			convertedText[dstIdx++] = ch;  // Keep control characters as-is
		} else {
			convertedText[dstIdx++] = 0;  // Replace invalid characters with space (index 0)
		}
	}
	convertedText[dstIdx] = '\0';

	// Draw the text string (with converted character indices)
	if (textFlags & 0x04) {
		// Word-wrapped text
		_rebelMsgFont->drawStringWrap(convertedText, renderBitmap, clipRect, posX, posY, textColor, styleFlags);
	} else {
		// Single-line text
		_rebelMsgFont->drawString(convertedText, renderBitmap, clipRect, posX, posY, textColor, styleFlags);
	}

	debug("Rebel2 Opcode 9: Rendered subtitle at (%d,%d) flags=0x%x clip=(%d,%d,%d,%d)",
		posX, posY, textFlags, clipX, clipY, clipW, clipH);
}

void InsaneRebel2::enemyUpdate(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4) {
	// Opcode 4: Enemy position update
	// Read 5 shorts from the stream (offset +8 through +16)
	int16 enemyId = b.readSint16LE();  // Offset +8
	int16 x = b.readSint16LE();        // Offset +10 (0x0A)

	// If enemy is disabled in bit table, skip update
	bool disabled = isBitSet(enemyId);

	int16 y = b.readSint16LE();        // Offset +12 (0x0C)
	int16 w = b.readSint16LE();        // Offset +14 (0x0E) - Width
	int16 h = b.readSint16LE();        // Offset +16 (0x10) - Height

	// If disabled, stop processing this object
	if (disabled) {
		// debug("Rebel2: Skipping Opcode 4 for disabled enemy ID=%d", enemyId);
		return;
	}

	// The disassembly shows half-width/half-height are used for centering:
	//   halfW = w >> 1
	//   halfH = h >> 1
	//   centerX = x + halfW
	//   centerY = y + halfH
	// But for drawing the bounding box, we want the top-left corner (x, y) and full dimensions.

	// Update enemy list for hit detection
	// Enemy type comes from par4 (IACT offset +6), NOT par3 (offset +4).
	// In the original (FUN_004028C5/FUN_0041E7C2): sVar5/sVar2 = *(short *)(*local + 6)
	// This maps to par4 (userId field). Used for DAT_0047ab98 wave state bitmask:
	//   DAT_0047ab98 |= 1 << (type & 0x1f)
	debug(5, "Rebel2 Opcode4: handler=%d enemyId=%d par2=%d par3=%d par4/type=%d pos=(%d,%d) size=(%d,%d)",
		_rebelHandler, enemyId, par2, par3, par4, x, y, w, h);

	bool found = false;
	Common::List<enemy>::iterator it;
	for (it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->id == enemyId) {
			it->rect = Common::Rect(x, y, x + w, y + h);
			it->type = par4;  // Enemy type from IACT offset +6 (userId)
			// Only re-activate if not destroyed
			if (!it->destroyed) {
				it->active = true;
			}
			found = true;
			break;
		}
	}
	if (!found) {
		init_enemyStruct(enemyId, x, y, w, h, true, false, -1, par4);
	}
}

void InsaneRebel2::init_enemyStruct(int id, int32 x, int32 y, int32 w, int32 h, bool active, bool destroyed, int32 explosionFrame, int type) {
	enemy e;
	e.id = id;
	e.type = type;
	e.rect = Common::Rect(x, y, x + w, y + h);
	e.active = active;
	e.destroyed = destroyed;
	e.explosionFrame = explosionFrame;
	e.savedBackground = nullptr;
	e.savedBgWidth = 0;
	e.savedBgHeight = 0;
	_enemies.push_back(e);
}

// ======================= Embedded Frame Codec Decoders =======================
// These implement the retail codec functions FUN_0042BD60, FUN_0042BBF0, FUN_0042B5F0

void InsaneRebel2::decodeCodec21(byte *dst, const byte *src, int width, int height) {
	// Codec 21/44: Line Update codec (FUN_0042BD60)
	// Format: each line has 2-byte size header, then pairs of (skip, count+1, literal_bytes)
	for (int row = 0; row < height; row++) {
		int lineDataSize = READ_LE_UINT16(src);
		src += 2;
		const byte *lineEnd = src + lineDataSize;
		byte *lineDst = dst + row * width;
		int x = 0;

		while (src < lineEnd && x < width) {
			int skip = READ_LE_UINT16(src);
			src += 2;
			x += skip;
			if (src >= lineEnd) break;

			int count = READ_LE_UINT16(src) + 1;
			src += 2;
			while (count-- > 0 && x < width && src < lineEnd) {
				lineDst[x++] = *src++;
			}
		}
		src = lineEnd;
	}
}

void InsaneRebel2::decodeCodec23(byte *dst, const byte *src, int width, int height, int dataSize) {
	// Codec 23: Skip/Copy with embedded RLE (FUN_0042BBF0)
	// Format: each line has 2-byte size, then pairs of (skip, runSize, RLE_data)
	const byte *dataEnd = src + dataSize;

	for (int row = 0; row < height && src < dataEnd; row++) {
		int lineDataSize = READ_LE_UINT16(src);
		src += 2;
		const byte *lineEnd = src + lineDataSize;
		byte *lineDst = dst + row * width;
		int x = 0;

		while (src < lineEnd && x < width) {
			int skip = READ_LE_UINT16(src);
			src += 2;
			x += skip;
			if (src >= lineEnd || x >= width) break;

			int runSize = READ_LE_UINT16(src);
			src += 2;

			// Decode RLE within this run
			const byte *runEnd = src + runSize;
			while (src < runEnd && x < width) {
				byte code = *src++;
				int num = (code >> 1) + 1;
				if (num > width - x) num = width - x;

				if (code & 1) {
					// RLE run
					byte color = (src < runEnd) ? *src++ : 0;
					for (int i = 0; i < num && x < width; i++) {
						lineDst[x++] = color;
					}
				} else {
					// Literal run
					for (int i = 0; i < num && x < width && src < runEnd; i++) {
						lineDst[x++] = *src++;
					}
				}
			}
			src = runEnd;
		}
		src = lineEnd;
	}
}

void InsaneRebel2::decodeCodec45(byte *dst, const byte *src, int width, int height, int dataSize) {
	// Codec 45: RA2-specific BOMP RLE with variable header (FUN_0042B5F0)
	// May have a 6-byte sub-header starting with "01 FE"

	debug("Rebel2: Codec 45 first 20 bytes: %02X %02X %02X %02X %02X %02X | %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7],
		src[8], src[9], src[10], src[11], src[12], src[13], src[14], src[15],
		src[16], src[17], src[18], src[19]);

	// Probe for header offset
	int headerSkip = 0;
	bool foundValidOffset = false;

	// Check for known 6-byte header pattern: 01 FE XX XX XX XX
	if (dataSize > 6 && src[0] == 0x01 && src[1] == 0xFE) {
		headerSkip = 6;
		debug("Rebel2: Codec 45 found 01 FE header, skipping 6 bytes");
		foundValidOffset = true;
	}

	// If no known header found, probe offsets 0, 2, 4, 6 to find valid RLE start
	if (!foundValidOffset) {
		for (int testOffset = 0; testOffset <= 6 && testOffset + 2 <= dataSize; testOffset += 2) {
			int testLineSize = READ_LE_UINT16(src + testOffset);
			// A valid first line size should be: > 0, <= width*2
			if (testLineSize > 0 && testLineSize <= width * 2 && testLineSize < dataSize - testOffset) {
				// Validate by summing line sizes
				int sumTest = 0;
				int linesTest = 0;
				const byte *testPtr = src + testOffset;
				bool validSum = true;

				while (linesTest < height && testPtr + 2 <= src + dataSize) {
					int ls = READ_LE_UINT16(testPtr);
					if (ls <= 0 || ls > width * 2) {
						validSum = false;
						break;
					}
					sumTest += ls + 2;
					testPtr += ls + 2;
					linesTest++;
				}

				// Accept if we got close to expected number of lines
				if (validSum && linesTest >= height - 1) {
					headerSkip = testOffset;
					foundValidOffset = true;
					debug("Rebel2: Codec 45 found valid RLE at offset %d (tested %d lines)", testOffset, linesTest);
					break;
				}
			}
		}
	}

	if (!foundValidOffset) {
		debug("Rebel2: Codec 45 couldn't find valid RLE offset, using offset 0");
	}

	const byte *srcPtr = src + headerSkip;
	const byte *dataEnd = src + dataSize;

	// Check if this is per-line RLE or continuous RLE
	int firstVal = READ_LE_UINT16(srcPtr);
	bool perLineMode = (firstVal > 0 && firstVal <= width * 2);

	if (perLineMode) {
		debug("Rebel2: Codec 45 using per-line RLE (firstLineSize=%d)", firstVal);
		for (int row = 0; row < height && srcPtr < dataEnd; row++) {
			int lineSize = READ_LE_UINT16(srcPtr);
			srcPtr += 2;
			if (lineSize <= 0 || lineSize > (int)(dataEnd - srcPtr)) break;

			const byte *lineEnd = srcPtr + lineSize;
			byte *rowDst = dst + row * width;
			int x = 0;

			while (srcPtr < lineEnd && x < width) {
				byte ctrl = *srcPtr++;
				int count = (ctrl >> 1) + 1;
				if (ctrl & 1) {
					byte color = (srcPtr < lineEnd) ? *srcPtr++ : 0;
					for (int i = 0; i < count && x < width; i++) rowDst[x++] = color;
				} else {
					for (int i = 0; i < count && x < width && srcPtr < lineEnd; i++)
						rowDst[x++] = *srcPtr++;
				}
			}
			srcPtr = lineEnd;
		}
	} else {
		// Continuous BOMP RLE (no per-line headers)
		debug("Rebel2: Codec 45 using continuous BOMP RLE");
		for (int row = 0; row < height && srcPtr < dataEnd; row++) {
			byte *rowDst = dst + row * width;
			int x = 0;

			while (x < width && srcPtr < dataEnd) {
				byte ctrl = *srcPtr++;
				int count = (ctrl >> 1) + 1;

				if (ctrl & 1) {
					// RLE fill
					byte color = (srcPtr < dataEnd) ? *srcPtr++ : 0;
					for (int i = 0; i < count && x < width; i++) {
						rowDst[x++] = color;
					}
				} else {
					// Literal copy
					for (int i = 0; i < count && x < width && srcPtr < dataEnd; i++) {
						rowDst[x++] = *srcPtr++;
					}
				}
			}
		}
	}

	// Count non-zero pixels for debug
	int nonZero = 0;
	for (int i = 0; i < width * height; i++) {
		if (dst[i] != 0) nonZero++;
	}
	debug("Rebel2: Decoded codec 45: %dx%d, %d non-zero (%d%%)",
		width, height, nonZero, (nonZero * 100) / (width * height));
}

void InsaneRebel2::renderEmbeddedFrame(byte *renderBitmap, const EmbeddedSanFrame &frame, int userId) {
	// Render the decoded embedded frame to the video buffer
	// Skip immediate draw for handlers that render HUD during post-processing:
	// - Handler 7/8: Ship direction sprites selected based on direction
	// - Handler 0x26/0x19: Cockpit HUD positioned based on mouse/crosshair
	//
	// Exception: Handler 25 (0x19) background overlays (par4/userId=4, 6, 7) should draw immediately.
	// These complete the visual scene and are NOT positioned by mouse/crosshair.
	bool skipImmediateDraw = (_rebelHandler == 7 || _rebelHandler == 8 ||
	                          _rebelHandler == 0x26 || _rebelHandler == 0x19);

	// Handler 25 overlays:
	// - userId 4 (corridor overlay): Draw during procPostRendering at view offset, NOT immediately
	// - userId 6, 7 (static overlays): Draw immediately (they don't move)
	if (_rebelHandler == 0x19 && (userId == 6 || userId == 7)) {
		skipImmediateDraw = false;
		debug("Rebel2: Handler 25 static overlay userId=%d - forcing immediate draw", userId);
	}
	// userId 4 should NOT draw immediately - it will be drawn at view offset each frame

	if (!frame.valid || !renderBitmap || skipImmediateDraw) {
		if (skipImmediateDraw && frame.valid) {
			debug("Rebel2: Skipped immediate draw for Handler %d HUD %d (will render during post-processing)",
				_rebelHandler, userId);
		}
		return;
	}

	int pitch = (_player && _player->_width > 0) ? _player->_width : 320;
	int bufHeight = (_player && _player->_height > 0) ? _player->_height : 200;

	for (int y = 0; y < frame.height && (frame.renderY + y) < bufHeight; y++) {
		for (int x = 0; x < frame.width && (frame.renderX + x) < pitch; x++) {
			byte pixel = frame.pixels[y * frame.width + x];
			if (pixel != 0 && pixel != 231) {  // 0 and 231 = transparent
				int destX = frame.renderX + x;
				int destY = frame.renderY + y;
				if (destX >= 0 && destY >= 0) {
					renderBitmap[destY * pitch + destX] = pixel;
				}
			}
		}
	}
	debug("Rebel2: Rendered embedded HUD %d at (%d,%d)", userId, frame.renderX, frame.renderY);
}

void InsaneRebel2::loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) {
	// Validate userId - Level 3 uses slots 0-11, allow up to 15 for safety
	if (userId < 0 || userId > 15 || !animData || size < 32) {
		debug("Rebel2: Invalid embedded SAN: userId=%d, size=%d", userId, size);
		return;
	}
	
	Common::MemoryReadStream stream(animData, size);
	
	// Read ANIM header
	uint32 animTag = stream.readUint32BE();
	if (animTag != MKTAG('A','N','I','M')) {
		debug("Rebel2: Embedded SAN missing ANIM tag, got 0x%08X", animTag);
		return;
	}
	uint32 animSize = stream.readUint32BE();
	debug("Rebel2: Parsing embedded ANIM: userId=%d, reported size=%u, actual=%d", userId, animSize, size - 8);
	
	// Iterate through chunks to find FRME -> FOBJ
	while (!stream.eos() && stream.pos() < size) {
		uint32 tag = stream.readUint32BE();
		uint32 chunkSize = stream.readUint32BE();
		int32 nextChunkPos = stream.pos() + chunkSize;

		if (tag == MKTAG('F','R','M','E')) {
			// Iterate sub-chunks in FRME
			while (stream.pos() < nextChunkPos && !stream.eos()) {
				uint32 subTag = stream.readUint32BE();
				uint32 subSize = stream.readUint32BE();
				int32 nextSubPos = stream.pos() + subSize;

				if (subTag == MKTAG('F','O','B','J')) {
					// Found FOBJ - Embedded HUD Frame
					// Dump raw FOBJ bytes for analysis
					int32 fobjStart = stream.pos();
					byte rawHeader[20];
					int headerBytesToRead = MIN((int)subSize, 20);
					stream.read(rawHeader, headerBytesToRead);
					stream.seek(fobjStart);  // Reset to read normally

					debug("Rebel2: Raw FOBJ header (%d bytes): %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
						headerBytesToRead,
						rawHeader[0], rawHeader[1], rawHeader[2], rawHeader[3],
						rawHeader[4], rawHeader[5], rawHeader[6], rawHeader[7],
						rawHeader[8], rawHeader[9], rawHeader[10], rawHeader[11],
						rawHeader[12], rawHeader[13], rawHeader[14], rawHeader[15],
						rawHeader[16], rawHeader[17], rawHeader[18], rawHeader[19]);

					// Read FOBJ header
					int codec = stream.readUint16LE();
					int left = stream.readUint16LE();
					int top = stream.readUint16LE();
					int width = stream.readUint16LE();
					int height = stream.readUint16LE();
					stream.readUint16LE();  // unknown
					stream.readUint16LE();  // unknown

					debug("Rebel2: Embedded HUD frame: userId=%d, %dx%d at (%d,%d), codec=%d",
						userId, width, height, left, top, codec);

					// Skip high-resolution frames - ScummVM runs at 320x200
					// If frame dimensions exceed low-res screen size, it's high-res data
					if (width > 400 || height > 250) {
						debug("Rebel2: SKIPPING high-res embedded frame: userId=%d, %dx%d (exceeds 400x250)",
							userId, width, height);
						stream.seek(nextSubPos);
						continue;
					}

					// Allocate storage for the decoded frame
					EmbeddedSanFrame &frame = _rebelEmbeddedHud[userId];

					if (width > 0 && height > 0 && width <= 800 && height <= 480) {
						if (frame.width != width || frame.height != height || !frame.pixels) {
							free(frame.pixels);
							frame.pixels = (byte *)malloc(width * height);
							frame.width = width;
							frame.height = height;
						}
						// Clear buffer before decode (important for delta codecs)
						memset(frame.pixels, 0, width * height);

						// Update render position from FOBJ header
						frame.renderX = left;
						frame.renderY = top;
						
						// Read the raw FOBJ data
						int32 dataSize = subSize - 14;
						if (dataSize > 0) {
							byte *fobjData = (byte *)malloc(dataSize);
							stream.read(fobjData, dataSize);

							// Decode based on codec - use extracted helper functions (FUN_0042BD60, etc.)
							if (codec == 1 || codec == 3) {
								// Codec 1/3: RLE - use existing decoder (FUN_0042C590)
								smushDecodeRLE(frame.pixels, fobjData, 0, 0, width, height, width);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec %d/RLE): %dx%d", codec, width, height);
							} else if (codec == 20) {
								// Codec 20: Uncompressed (FUN_0042C400)
								smushDecodeUncompressed(frame.pixels, fobjData, 0, 0, width, height, width);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec 20/raw): %dx%d", width, height);
							} else if (codec == 21 || codec == 44) {
								// Codec 21/44: Line update (FUN_0042BD60)
								decodeCodec21(frame.pixels, fobjData, width, height);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec %d/line update): %dx%d", codec, width, height);
							} else if (codec == 45) {
								// Codec 45: RA2-specific BOMP RLE (FUN_0042B5F0)
								decodeCodec45(frame.pixels, fobjData, width, height, dataSize);
								frame.valid = true;
							} else if (codec == 23) {
								// Codec 23: Skip/copy with embedded RLE (FUN_0042BBF0)
								decodeCodec23(frame.pixels, fobjData, width, height, dataSize);
								frame.valid = true;
								debug("Rebel2: Decoded embedded HUD (codec 23/skip-RLE): %dx%d", width, height);
							} else {
								debug("Rebel2: TODO: Decode codec %d for embedded HUD", codec);
								frame.valid = false;
							}

							// Count non-zero pixels to verify frame has content
							if (frame.valid) {
								int nonZeroPixels = 0;
								for (int i = 0; i < width * height; i++) {
									if (frame.pixels[i] != 0) nonZeroPixels++;
								}
								debug("Rebel2: Frame userId=%d has %d non-zero pixels (%d%%)",
									userId, nonZeroPixels, (nonZeroPixels * 100) / (width * height));
							}

							// Render the decoded frame to the video buffer
							renderEmbeddedFrame(renderBitmap, frame, userId);

							free(fobjData);
						}
					}
					
					// Done with FOBJ - assume only one relevant frame per embedded SAN
					stream.seek(nextChunkPos);
					goto end_parsing;
				} else {
					// Skip other sub-chunks (AHDR inside FRME?) or padding
					stream.seek(nextSubPos);
					if (subSize & 1) stream.skip(1);
				}
			}
		} else {
			// Skip non-FRME chunks (AHDR, etc at top level)
			stream.seek(nextChunkPos);
			if (chunkSize & 1) stream.skip(1);
		}
	}
	
	debug("Rebel2: No FOBJ found in embedded SAN userId=%d", userId);

end_parsing:;
}

// Spawn explosion into the shared 5-slot system.
// In the original, each handler has its own spawn logic inside its enemy processing function:
//   Handler 0x26: FUN_40A2E0 (0x40A2E0) — spawns in slot arrays DAT_0044368e[]
//   Handler 8:    FUN_4028C5 (0x4028C5) — spawns in slot arrays DAT_0043f854[]
//   Handler 7:    FUN_40F628 (0x40F628) — spawns in slot arrays DAT_00443770[]
//   Handler 25:   FUN_41E7C2 (0x41E7C2) — spawns in slot arrays DAT_0045792c[]
// All share the same logic: find first free slot (counter==0), set counter=10,
// scale=objectHalfWidth, position=enemy center, velocity=0.
void InsaneRebel2::spawnExplosion(int x, int y, int objectHalfWidth) {
	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active || _explosions[i].counter <= 0) {
			_explosions[i].active = true;
			_explosions[i].counter = 10;
			_explosions[i].x = x;
			_explosions[i].y = y;
			_explosions[i].scale = objectHalfWidth;
			break;
		}
	}
}

// Get max shot duration from level table (DAT_0047e0f0 indexed by DAT_0047a7fa/DAT_0047a7f8)
// For now, use a reasonable default based on observed values
int16 InsaneRebel2::getShotMaxDuration() {
	// The original uses: DAT_0047e0f0 + DAT_0047a7fa * 0x242 + DAT_0047a7f8 * 0x22
	// Typical values observed: 2-5 frames
	return 4;
}

// Dispatcher - calls appropriate spawn function based on current handler
void InsaneRebel2::spawnShot(int x, int y) {
	switch (_rebelHandler) {
	case 0x26:  // Turret
		spawnTurretShot(x, y);
		break;
	case 8:     // Vehicle
		spawnVehicleShot(x, y);
		break;
	case 7:     // Space combat
		spawnSpaceShot(x, y);
		break;
	case 25:    // Speeder bike - uses turret shot array with different gun position
		spawnHandler25Shot(x, y);
		break;
	default:
		// Legacy fallback
		for (int i = 0; i < 2; i++) {
			if (!_shots[i].active) {
				_shots[i].active = true;
				_shots[i].counter = getShotMaxDuration();
				_shots[i].x = x + _viewX;
				_shots[i].y = y + _viewY;
				break;
			}
		}
		break;
	}
}

// Handler 0x26 Turret shot spawn (based on FUN_4089AB lines 127-140)
void InsaneRebel2::spawnTurretShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter == 0) {
			// FUN_0041189e(-(ushort)(DAT_004436de == 5) & 7, i + 1, 0x7f, 0, 0)
			// levelType 5: BLAST.SAD (slot 0), otherwise: TBLAST.SAD (slot 7)
			playSfx((_rebelLevelType == 5) ? 0 : 7, 127, 0);

			_turretShots[i].counter = getShotMaxDuration();
			_turretShots[i].seqNum = _turretShotSeqCounter;
			_turretShotSeqCounter++;
			_turretShots[i].targetX = x + _viewX;  // DAT_0044366e in original
			_turretShots[i].targetY = y + _viewY;  // DAT_00443670 in original
			break;
		}
	}
}

// Handler 8 Vehicle shot spawn (based on FUN_401CCF lines 65-69)
void InsaneRebel2::spawnVehicleShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_vehicleShots[i].counter == 0) {
			// FUN_0041189e(6, local_c + 1, 0x7f, 0, 0) — HBLAST.SAD
			playSfx(6, 127, 0);
			_vehicleShots[i].counter = getShotMaxDuration();
			_vehicleShots[i].targetX = x + _viewX;
			_vehicleShots[i].targetY = y + _viewY;
			break;
		}
	}
}

// Handler 25 on-foot shot spawn (based on FUN_0041db5e lines 170-190)
// Gun position computed from GRD002 character sprite.
// Original stores: DAT_0045791c[i] = gunOffsetTable[spriteIdx] + DAT_00457910 - DAT_0045790c
//                  DAT_00457920[i] = gunYTable[spriteIdx] + DAT_00457912 - DAT_0045790e
// Render adds view offset back, so screen gun = table[idx] + spriteOffset.
void InsaneRebel2::spawnHandler25Shot(int x, int y) {
	// Handler 25 can only shoot when uncovered (damage == 0)
	if (_rebelDamageLevel != 0) {
		return;  // Can't shoot while taking cover
	}

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter == 0) {
			// FUN_0041189e(6, local_1c + 1, 0x7f, 0, 0) — HBLAST.SAD
			playSfx(6, 127, 0);

			_turretShots[i].counter = getShotMaxDuration();
			_turretShots[i].seqNum = _turretShotSeqCounter;
			_turretShotSeqCounter++;

			// Target position is where player clicked (screen coords)
			_turretShots[i].targetX = x;
			_turretShots[i].targetY = y;

			// Compute gun position from GRD002 character sprite.
			// Original uses per-direction lookup tables DAT_004578a6/DAT_004578c6.
			// We approximate from the NUT sprite center + directional offset.
			if (_grd002Sprite && _grd002Sprite->getNumChars() > 0) {
				// Compute current sprite index (same logic as renderHandler25Ship)
				int spriteIdx;
				if (_rebelDamageLevel == 0) {
					// Uncovered: compute from crosshair position zones
					int16 areaLeft = (_corridorLeftX > 0) ? _corridorLeftX : 0;
					int16 areaRight = (_corridorRightX > 0) ? _corridorRightX : 320;
					int16 areaTop = (_corridorTopY > 0) ? _corridorTopY : 0;
					int16 areaBottom = (_corridorBottomY > 0) ? _corridorBottomY : 180;
					int areaWidth = areaRight - areaLeft;
					int areaHeight = areaBottom - areaTop;
					int zoneWidth = (areaWidth > 0) ? (areaWidth + 3) / 4 : 80;
					int zoneHeight = (areaHeight > 0) ? areaHeight / 2 : 90;
					int xZone = (zoneWidth > 0) ? ((zoneWidth / 2) + (x - areaLeft)) / zoneWidth : 2;
					int yZone = (zoneHeight > 0) ? ((zoneHeight / 2) + (y - areaTop)) / zoneHeight : 0;
					if (xZone < 0) xZone = 0;
					if (xZone > 4) xZone = 4;
					if (yZone < 0) yZone = 0;
					if (yZone > 1) yZone = 1;
					if (_rebelFlightDir == (yZone & 1)) {
						xZone = 4 - xZone;
					}
					spriteIdx = yZone * 5 + xZone + 5;
				} else {
					spriteIdx = (_rebelFlightDir == 0) ? (5 - _rebelDamageLevel) : (25 - _rebelDamageLevel);
				}
				int numSprites = _grd002Sprite->getNumChars();
				if (spriteIdx < 0) spriteIdx = 0;
				if (spriteIdx >= numSprites) spriteIdx = numSprites - 1;

				// Get sprite rendering position (same as in renderHandler25Ship)
				int16 spriteXOffset = _grd002Sprite->getCharXOffset(spriteIdx);
				int16 spriteYOffset = _grd002Sprite->getCharYOffset(spriteIdx);
				int spriteW = _grd002Sprite->getCharWidth(spriteIdx);
				int spriteH = _grd002Sprite->getCharHeight(spriteIdx);
				bool shouldMirror = (_rebelFlightDir != 0 && _rebelDamageLevel == 0);

				int drawX;
				if (shouldMirror) {
					drawX = _rebelViewOffset2X + (320 - spriteW - spriteXOffset);
				} else {
					drawX = _rebelViewOffset2X + spriteXOffset;
				}
				int drawY = spriteYOffset + _rebelViewOffset2Y;

				// Gun barrel is approximately at the character's hand level:
				// X: center of sprite ± directional offset toward the target
				// Y: about 60% down the sprite height (hand/arm level)
				_turretShots[i].gunX = drawX + spriteW / 2;
				_turretShots[i].gunY = drawY + (spriteH * 3) / 5;
			} else {
				// Fallback: approximate center-bottom of character area
				_turretShots[i].gunX = _rebelViewOffset2X + 160;
				_turretShots[i].gunY = _rebelViewOffset2Y + 140;
			}

			debug("Rebel2 Handler25: Spawned shot %d target (%d,%d) gun (%d,%d)",
				i, _turretShots[i].targetX, _turretShots[i].targetY,
				_turretShots[i].gunX, _turretShots[i].gunY);
			break;
		}
	}
}

// Handler 7 Space combat shot spawn (based on FUN_40D836 lines 146-166)
void InsaneRebel2::spawnSpaceShot(int x, int y) {
	for (int i = 0; i < 2; i++) {
		if (_spaceShots[i].counter == 0) {
			// FUN_0041189e(6, local_2c + 1, 0x7f, 0, 0) — HBLAST.SAD
			playSfx(6, 127, 0);

			_spaceShots[i].counter = getShotMaxDuration();
			_spaceShots[i].targetX = x;  // Screen coords
			_spaceShots[i].targetY = y;

			// Calculate gun positions from direction-based lookup tables
			// In the original, these come from tables indexed by _shipDirectionIndex
			// DAT_004437c2/DAT_00443808 for left gun, DAT_0044384e/DAT_00443894 for right gun
			// For now, use simplified positions relative to ship
			int shipScreenX = 160 + ((_shipPosX - 160) >> 3);
			int shipScreenY = 105 + ((_shipPosY - 40) >> 2);

			// Gun offsets (approximate from disassembly)
			_spaceShots[i].leftGunX = shipScreenX - 28;
			_spaceShots[i].leftGunY = shipScreenY + 10;
			_spaceShots[i].rightGunX = shipScreenX + 28;
			_spaceShots[i].rightGunY = shipScreenY + 10;
			_spaceShots[i].variant = _spaceShotDirection;
			break;
		}
	}
}

void InsaneRebel2::drawTexturedLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, NutRenderer *nut, int spriteIdx, int v, bool mask231) {
	if (!nut || spriteIdx >= nut->getNumChars()) return;

	const byte *srcData = nut->getCharData(spriteIdx);
	int texW = nut->getCharWidth(spriteIdx);
	int texH = nut->getCharHeight(spriteIdx);
	
	if (!srcData || texW <= 0 || texH <= 0) return;
	if (v < 0) v = 0;
	if (v >= texH) v = texH - 1;

	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;
	
	// Total length approximation for UV mapping
	int totalDist = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
	if (totalDist == 0) totalDist = 1;
	
	int currentDist = 0;

	for (;;) {
		if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
			// Map currentDist/totalDist to 0..texW (Run along texture width)
			int u = (currentDist * texW) / totalDist;
			if (u >= texW) u = texW - 1;
			
			byte color = srcData[v * texW + u];
			
			// Check for transparency (0 and optionally 231)
			if (color != 0 && (!mask231 || color != 231)) { 
				dst[y0 * pitch + x0] = color;
			}
		}
		
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
		
		currentDist++;
	}
}

// Helper: draw a textured segment between two points using the game's original routine (FUN_00429360 port)
void drawTexturedSegment(byte *dst, int pitch, int width, int height, int param_3, int param_4, int param_5, int param_6, int param_7, const byte *param_8) {
	// Ported from FUN_00429360 (decompiled). Only 0 in texture is transparent.
	int sVar4 = 0;                // left
	int sVar1 = 0;                // top
	int sVar7 = width - 1;        // right
	int sVar10 = height - 1;      // bottom

	int px0 = param_3;
	int py0 = param_4;
	int px1 = param_5;
	int py1 = param_6;

	// Clip against screen bounds (translation of original clipping logic)
	if (px0 == px1) {
		if (px0 < sVar4 || px0 > sVar7) return;
	} else {
		if (px0 < sVar4) {
			if (px1 < sVar4) return;
			py0 = py1 + ((py0 - py1) * (sVar4 - px1)) / (px0 - px1);
			px0 = sVar4;
		} else if (px0 > sVar7) {
			if (px1 > sVar7) return;
			py0 = py1 + ((py0 - py1) * (sVar7 - px1)) / (px0 - px1);
			px0 = sVar7;
		}
		if (px1 < sVar4) {
			py1 = py0 + ((py1 - py0) * (sVar4 - px0)) / (px1 - px0);
			px1 = sVar4;
		} else if (px1 > sVar7) {
			py1 = py0 + ((py1 - py0) * (sVar7 - px0)) / (px1 - px0);
			px1 = sVar7;
		}
	}

	if (py0 == py1) {
		if (py0 < sVar1 || py0 > sVar10) return;
	} else {
		if (py0 < sVar1) {
			if (py1 < sVar1) return;
			px0 = px1 + ((px0 - px1) * (sVar1 - py1)) / (py0 - py1);
			py0 = sVar1;
		} else if (py0 > sVar10) {
			if (py1 > sVar10) return;
			px0 = px1 + ((px0 - px1) * (sVar10 - py1)) / (py0 - py1);
			py0 = sVar10;
		}
		if (py1 < sVar1) {
			px1 = px0 + ((px1 - px0) * (sVar1 - py0)) / (py1 - py0);
			py1 = sVar1;
		} else if (py1 > sVar10) {
			px1 = px0 + ((px1 - px0) * (sVar10 - py0)) / (py1 - py0);
			py1 = sVar10;
		}
	}

	int dx = px1 - px0;
	int dy = py1 - py0;
	int absdx = dx < 0 ? -dx : dx;
	int absdy = dy < 0 ? -dy : dy;

	// pointer into destination and texture
	byte *baseDst = dst;
	const byte *texPtr = param_8;

	if (absdx == 0) {
		if (absdy == 0) {
			if (*texPtr != 0) baseDst[py0 * pitch + px0] = *texPtr;
			return;
		}
		// vertical-ish
		int step = absdy + 1;
		int curY = py0;
		int signY = dy > 0 ? 1 : -1;
		int iVar9 = step; // adv counter
		for (int i = 0; i < step; i++) {
			if (*texPtr != 0) baseDst[curY * pitch + px0] = *texPtr;
			curY += signY;
			iVar9 -= param_7;
			while (iVar9 < 0) { texPtr++; iVar9 += step; }
		}
		return;
	}

	if (absdy == 0) {
		// horizontal-ish
		int step = absdx + 1;
		int curX = px0;
		int signX = dx > 0 ? 1 : -1;
		int iVar11 = step;
		for (int i = 0; i < step; i++) {
			if (*texPtr != 0) baseDst[py0 * pitch + curX] = *texPtr;
			curX += signX;
			iVar11 -= param_7;
			while (iVar11 < 0) { texPtr++; iVar11 += step; }
		}
		return;
	}

	// general case
	int steps = (absdx > absdy) ? absdx + 1 : absdy + 1;
	int x = px0, y = py0;
	int sx = dx > 0 ? 1 : -1;
	int sy = dy > 0 ? 1 : -1;
	int err = absdx - absdy;
	int iVar12 = steps;

	for (int i = 0; i < steps; i++) {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			if (*texPtr != 0) baseDst[y * pitch + x] = *texPtr;
		}
		int e2 = 2 * err;
		if (e2 > -absdy) { err -= absdy; x += sx; }
		if (e2 < absdx) { err += absdx; y += sy; }
		iVar12 -= param_7;
		if (iVar12 < 0) { texPtr++; iVar12 += steps; }
	}
}


// Initialize laser texture buffer from NUT sprite (FUN_0040BAB0)
// This pre-renders a sprite into a buffer that drawLaserBeam uses
void InsaneRebel2::initLaserTexture(NutRenderer *nut, int spriteIdx) {
	if (!nut || spriteIdx >= nut->getNumChars())
		return;

	// Get sprite dimensions (FUN_0040BAB0 lines 13-14)
	int16 texWidth = nut->getCharWidth(spriteIdx);
	int16 texHeight = nut->getCharHeight(spriteIdx);

	// Clamp height to max 15 pixels (FUN_0040BAB0 lines 15-17)
	if (texHeight > 15) {
		texHeight = 15;
	}

	// Free existing texture if any (FUN_0040BAB0 lines 18-20)
	freeLaserTexture();

	// Allocate new buffer (FUN_0040BAB0 line 21)
	_laserTexture.width = texWidth;
	_laserTexture.height = texHeight;
	_laserTexture.pixels = (byte *)calloc(texWidth * texHeight, 1);

	if (!_laserTexture.pixels)
		return;

	// Render sprite into buffer (FUN_0040BAB0 lines 23-24)
	// We copy the sprite data directly since it's already in the right format
	const byte *srcData = nut->getCharData(spriteIdx);
	if (srcData) {
		int srcHeight = nut->getCharHeight(spriteIdx);
		int copyHeight = MIN(texHeight, (int16)srcHeight);
		memcpy(_laserTexture.pixels, srcData, texWidth * copyHeight);
	}

	debug("Rebel2: Initialized laser texture %dx%d from sprite %d", texWidth, texHeight, spriteIdx);
}

// Free laser texture buffer (FUN_0040BBD1)
void InsaneRebel2::freeLaserTexture() {
	free(_laserTexture.pixels);
	_laserTexture.pixels = nullptr;
	_laserTexture.width = 0;
	_laserTexture.height = 0;
}

// Draw laser beam using pre-initialized texture (FUN_0040BBF6)
// This is a direct port of the assembly function
//
// Parameters (matching FUN_0040bbf6):
//   dst, pitch, width, height: destination buffer info
//   gunX, gunY (param_3, param_4): gun/start position
//   targetX, targetY (param_5, param_6): target/end position
//   animFrame (param_7): current animation frame (shot counter)
//   maxFrames (param_8): max animation frames (shot duration)
//   widthScale (param_9): width scaling factor for perspective
//   heightScale (param_10): height/thickness multiplier
//   thickness (param_11): base line thickness
void InsaneRebel2::drawLaserBeam(byte *dst, int pitch, int width, int height,
                                  int16 gunX, int16 gunY, int16 targetX, int16 targetY,
                                  int16 animFrame, int16 maxFrames,
                                  int16 widthScale, int16 heightScale, int16 thickness) {
	// Check if laser texture is initialized
	if (!_laserTexture.pixels || _laserTexture.width <= 0 || _laserTexture.height <= 0)
		return;

	int16 texW = _laserTexture.width;
	int16 texH = _laserTexture.height;
	byte *texPixels = _laserTexture.pixels;

	// FUN_0040BBF6 line 23: sVar7 = (thickness * animFrame * 16) / maxFrames
	if (maxFrames == 0) maxFrames = 1;
	int16 sVar7 = (int16)(((int)thickness * (int)animFrame * 16) / (int)maxFrames);

	// FUN_0040BBF6 lines 24-25: Calculate delta with scaling
	int16 dx = targetX - gunX;
	int16 dy = targetY - gunY;
	int16 sVar6 = (int16)(((int)dx * (thickness + 1)) / (int)thickness);
	int16 sVar1 = (int16)(((int)dy * (thickness + 1)) / (int)thickness);

	// FUN_0040BBF6 lines 26-29: Calculate adjusted start and end points
	// Start point (closer to gun, adjusted by animation progress)
	int16 startX = (sVar6 + gunX) - (int16)(((int)sVar6 * 16) / (sVar7 + 16));
	int16 startY = (sVar1 + gunY) - (int16)(((int)sVar1 * 16) / (sVar7 + 16));
	// End point (closer to target)
	int16 endX = (sVar6 + gunX) - (int16)(((int)sVar6 * 16) / (widthScale + sVar7 + 16));
	int16 endY = (sVar1 + gunY) - (int16)(((int)sVar1 * 16) / (widthScale + sVar7 + 16));

	// FUN_0040BBF6 line 30: Get texture pixel pointer
	byte *local_28 = texPixels;

	// FUN_0040BBF6 lines 31-32: Calculate abs differences (FUN_004356e4 = abs)
	int iVar2 = abs(startY - endY);  // |dy| of beam
	int iVar3 = abs(startX - endX);  // |dx| of beam

	// FUN_0040BBF6 line 33: Choose rendering path based on beam orientation
	if (iVar2 < iVar3) {
		// Mostly horizontal beam - draw vertical scanlines
		// FUN_0040BBF6 lines 34-37
		iVar2 = abs(startX - endX);
		int temp = iVar2 * texH * heightScale;
		int16 numLines = (int16)((temp >> 3) / texW) + 2;
		int16 local_24 = -numLines;
		int16 halfLines = numLines >> 1;

		// FUN_0040BBF6 lines 39-46: Draw parallel lines
		for (int16 lineIdx = 0; lineIdx < numLines; lineIdx++) {
			// Draw one textured segment (vertical offset for this scanline)
			drawTexturedSegment(dst, pitch, width, height,
			                    startX, (startY - halfLines) + lineIdx,
			                    endX, (endY - halfLines) + lineIdx,
			                    texW, local_28);

			// Advance texture pointer (step through texture rows)
			for (local_24 = texH + local_24; local_24 > 0; local_24 -= numLines) {
				local_28 += texW;
			}
		}
	} else {
		// Mostly vertical beam - draw horizontal scanlines
		// FUN_0040BBF6 lines 54-56
		iVar2 = abs(startY - endY);
		int16 numLines = (int16)((iVar2 * texH) / texW) + 2;
		int16 local_24 = -numLines;

		// FUN_0040BBF6 lines 58-60: Clamp to texture height
		if (texH < numLines) {
			numLines = texH;
		}

		int16 halfLines = numLines >> 1;

		// FUN_0040BBF6 lines 61-68: Draw parallel lines
		for (int16 lineIdx = 0; lineIdx < numLines; lineIdx++) {
			// Draw one textured segment (horizontal offset for this scanline)
			drawTexturedSegment(dst, pitch, width, height,
			                    (startX - halfLines) + lineIdx, startY,
			                    (endX - halfLines) + lineIdx, endY,
			                    texW, local_28);

			// Advance texture pointer
			for (local_24 = texH + local_24; local_24 > 0; local_24 -= numLines) {
				local_28 += texW;
			}
		}
	}
}
void InsaneRebel2::drawLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, byte color) {
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;

	for (;;) {
		if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
			dst[y0 * pitch + x0] = color;
		}
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}

void InsaneRebel2::drawCornerBrackets(byte *dst, int pitch, int width, int height, int x, int y, int w, int h, byte color) {
	// Draw L-shaped brackets at corners of the rect (x,y,w,h)
	// Bracket size: approx 8 pixels
	int armLen = 2;
	if (armLen > w / 2) armLen = w / 2;
	if (armLen > h / 2) armLen = h / 2;

	int x2 = x + w - 1;
	int y2 = y + h - 1;

	// Top-Left Corner
	drawLine(dst, pitch, width, height, x, y, x + armLen, y, color);
	drawLine(dst, pitch, width, height, x, y, x, y + armLen, color);

	// Top-Right Corner
	drawLine(dst, pitch, width, height, x2 - armLen, y, x2, y, color);
	drawLine(dst, pitch, width, height, x2, y, x2, y + armLen, color);

	// Bottom-Left Corner
	drawLine(dst, pitch, width, height, x, y2, x + armLen, y2, color);
	drawLine(dst, pitch, width, height, x, y2 - armLen, x, y2, color);

	// Bottom-Right Corner
	drawLine(dst, pitch, width, height, x2 - armLen, y2, x2, y2, color);
	drawLine(dst, pitch, width, height, x2, y2 - armLen, x2, y2, color);
}

// ============================================================
// COLLISION ZONE SYSTEM (for Level 3 pilot ship obstacle avoidance)
// ============================================================
// Based on FUN_40E35E, FUN_40C3CC disassembly from info.md
// Zones are quadrilaterals registered via IACT opcode 5

void InsaneRebel2::registerCollisionZone(Common::SeekableReadStream &b, int16 subOpcode, int16 par4) {
	// IACT Opcode 5 data layout — corrected from FUN_4033CF / FUN_4092D9 analysis:
	//
	// Original game stores pointer to full IACT data (starting at opcode).
	// SmushPlayer reads the first 8 bytes as header (code/flags/unknown/userId),
	// so our stream starts at body[0] (IACT byte offset +8).
	//
	// FUN_4092D9 field mapping (byte offsets from stored pointer):
	//   +0x00: opcode (5) — already consumed by SmushPlayer
	//   +0x02: par2 (sub-opcode) — already consumed, passed as parameter
	//   +0x04: par3 — already consumed by SmushPlayer
	//   +0x06: par4 (userId) — filter value for < 1000 test, passed as parameter
	//   +0x08: body[0] (sVar1) — control field 1 (frame check: field2-1 == field1)
	//   +0x0A: body[1] (sVar2) — control field 2
	//   +0x0C: body[2] — vertex 1 X
	//   +0x0E: body[3] — vertex 1 Y
	//   +0x10: body[4] — vertex 2 X
	//   +0x12: body[5] — vertex 2 Y
	//   +0x14: body[6] — vertex 3 X
	//   +0x16: body[7] — vertex 3 Y
	//   +0x18: body[8] — vertex 4 X
	//   +0x1A: body[9] — vertex 4 Y

	int16 field1 = b.readSint16LE();     // body[0] — control field 1
	int16 field2 = b.readSint16LE();     // body[1] — control field 2
	int16 x1 = b.readSint16LE();         // body[2] — vertex 1 X
	int16 y1 = b.readSint16LE();         // body[3] — vertex 1 Y
	int16 x2 = b.readSint16LE();         // body[4] — vertex 2 X
	int16 y2 = b.readSint16LE();         // body[5] — vertex 2 Y
	int16 x3 = b.readSint16LE();         // body[6] — vertex 3 X
	int16 y3 = b.readSint16LE();         // body[7] — vertex 3 Y
	int16 x4 = b.readSint16LE();         // body[8] — vertex 4 X
	int16 y4 = b.readSint16LE();         // body[9] — vertex 4 Y

	CollisionZone zone;
	zone.x1 = x1;
	zone.y1 = y1;
	zone.x2 = x2;
	zone.y2 = y2;
	zone.x3 = x3;
	zone.y3 = y3;
	zone.x4 = x4;
	zone.y4 = y4;
	zone.field1 = field1;
	zone.field2 = field2;
	zone.filterValue = par4;
	zone.subOpcode = subOpcode;
	zone.active = true;

	// Register zone into appropriate table based on sub-opcode
	if (subOpcode == 0x0D && _primaryZoneCount < kMaxCollisionZones) {
		_primaryZones[_primaryZoneCount++] = zone;
		debug("Rebel2: Registered PRIMARY zone %d: filter=%d fields=[%d,%d] quad=(%d,%d)-(%d,%d)-(%d,%d)-(%d,%d)",
			_primaryZoneCount - 1, par4, field1, field2,
			x1, y1, x2, y2, x3, y3, x4, y4);
	} else if (subOpcode == 0x0E && _secondaryZoneCount < kMaxCollisionZones) {
		_secondaryZones[_secondaryZoneCount++] = zone;
		debug("Rebel2: Registered SECONDARY zone %d: filter=%d fields=[%d,%d] quad=(%d,%d)-(%d,%d)-(%d,%d)-(%d,%d)",
			_secondaryZoneCount - 1, par4, field1, field2,
			x1, y1, x2, y2, x3, y3, x4, y4);
	} else {
		debug("Rebel2: WARNING - Could not register zone (subOpcode=%d, primary=%d, secondary=%d)",
			subOpcode, _primaryZoneCount, _secondaryZoneCount);
	}
}

void InsaneRebel2::resetCollisionZones() {
	// Reset zone counters at end of frame (FUN_403240 equivalent)
	// This clears the zone tables so they can be rebuilt from the next frame's IACT chunks
	_primaryZoneCount = 0;
	_secondaryZoneCount = 0;
}

void InsaneRebel2::checkCollisionZones() {
	// Per-frame collision checking — FUN_4092D9 first loop (lines 39-202).
	// Tests aim/ship position against primary collision zone quadrilaterals.
	//
	// Original coordinate system:
	//   Zone vertices are in 424x260 buffer space, centered by subtracting (0xD4=212, 0x82=130).
	//   Aim position (DAT_00443668/DAT_0044366a) is in centered coords [-52..52, -45..45].
	//   In FUN_407FCB: DAT_00443668 is a smoothed mouse-derived position.
	//
	// For our implementation:
	//   Map mouse position to centered coords matching the original range.
	//   Mouse X 0..320 → centered X ≈ [-52..52] (with smoothing in original)
	//   Mouse Y 0..200 → centered Y ≈ [-45..45]

	if (_primaryZoneCount == 0) return;

	// Calculate aim position in centered coordinates.
	// Original: local_10 = mouseOffset + 0xa0, then smoothed and clamped to [-0x34..0x34]
	// Simplified mapping: mouse 0..320 → [-52..52], mouse 0..200 → [-45..45]
	int16 aimX = (int16)((_vm->_mouse.x - 160) * 52 / 160);
	int16 aimY = (int16)((100 - _vm->_mouse.y) * 45 / 100);

	// Clamp to original ranges (DAT_0047a7fc < 1 path)
	if (aimX > 0x34) aimX = 0x34;
	if (aimX < -0x34) aimX = -0x34;
	if (aimY > 0x2d) aimY = 0x2d;
	if (aimY < -0x2d) aimY = -0x2d;

	for (int i = 0; i < _primaryZoneCount; i++) {
		CollisionZone &zone = _primaryZones[i];
		if (!zone.active) continue;

		// Filter: only process zones with filterValue < 1000 (par4 from IACT header)
		// Original: *(short *)(*local_c + 6) < 1000
		if (zone.filterValue >= 1000) continue;

		// Frame check: field2 - 1 == field1
		// Original: sVar2 + -1 == (int)sVar1
		if (zone.field2 - 1 != zone.field1) continue;

		// Center zone vertices by subtracting buffer center (0xD4=212, 0x82=130)
		// Original: sVar4 = x1 - 0xD4, sVar8 = y1 - 0x82, etc.
		int cx1 = zone.x1 - 0xD4;
		int cy1 = zone.y1 - 0x82;
		int cx2 = zone.x2 - 0xD4;
		int cy2 = zone.y2 - 0x82;
		int cx3 = zone.x3 - 0xD4;
		int cy3 = zone.y3 - 0x82;
		int cx4 = zone.x4 - 0xD4;
		int cy4 = zone.y4 - 0x82;

		// Point-in-quadrilateral test — FUN_4092D9 lines 119-128
		// Tests if aim position is OUTSIDE the safe corridor (= collision with obstacle).
		// Original uses 4 edge interpolation tests connected by OR (any failure = collision).
		//
		// Edge 1: interpolate Y along top edge (v1→v2) at aim X position
		//   if aimY < interpolated Y → outside top edge → collision
		// Edge 2: interpolate Y along bottom edge (v4→v3) at aim X position
		//   if interpolated Y < aimY → outside bottom edge → collision
		// Edge 3: interpolate X along left edge (v1→v4) at aim Y position
		//   if aimX < interpolated X → outside left edge → collision
		// Edge 4: interpolate X along right edge (v2→v3) at aim Y position
		//   if interpolated X < aimX → outside right edge → collision
		bool collision = false;

		// Avoid division by zero for degenerate edges
		if (cx2 != cx1) {
			int interpY1 = ((aimX - cx1) * (cy2 - cy1)) / (cx2 - cx1) + cy1;
			if (aimY < interpY1) collision = true;
		}
		if (!collision && cx3 != cx4) {
			int interpY2 = ((aimX - cx4) * (cy3 - cy4)) / (cx3 - cx4) + cy4;
			if (interpY2 < aimY) collision = true;
		}
		if (!collision && cy4 != cy1) {
			int interpX1 = ((aimY - cy1) * (cx4 - cx1)) / (cy4 - cy1) + cx1;
			if (aimX < interpX1) collision = true;
		}
		if (!collision && cy3 != cy2) {
			int interpX2 = ((aimY - cy2) * (cx3 - cx2)) / (cy3 - cy2) + cx2;
			if (interpX2 < aimX) collision = true;
		}

		if (collision) {
			// Collision detected — apply damage from collision damage table
			// Original: DAT_0047a7ec += DAT_0047e0f6[levelIdx]
			// TODO: Read from per-level collision damage table DAT_0047e0f6
			int collisionDamage = 3 + (_difficulty * 2);

			if (!_rebelInvulnerable) {
				_playerDamage += collisionDamage;
				if (_playerDamage > 255) _playerDamage = 255;
				debug("Rebel2: COLLISION damage! zone=%d aim=(%d,%d) damage=%d total=%d",
					i, aimX, aimY, collisionDamage, _playerDamage);
			}
			// Visual effect — FUN_00420515 (palette flash)
			initDamageFlash();
			// TODO: FUN_0041189e sound based on collision direction
		} else {
			// Safely passed — award score bonus
			// Original: FUN_0041bf8d(DAT_0047e100[levelIdx])
			addScore(1);
		}
	}
}

void InsaneRebel2::checkHandler7CollisionZones() {
	// FUN_40E35E — Handler 7 per-frame collision system.
	// Uses ship position (_flyShipScreenX/_flyShipScreenY) in raw buffer coords.
	// Two modes depending on _flyControlMode:
	//   Mode 0/2: Obstacle collision using SECONDARY zones (inside quad = hit)
	//   Mode 1/3: Wall/boundary collision using PRIMARY zones (per-edge push-back)

	// Note: _hitCooldown is decremented in renderSpaceExplosions (FUN_40F1C5)
	// to match the original where the decrement happens during rendering.

	if (_flyControlMode == 0 || _flyControlMode == 2) {
		// ---- Mode 0/2: Obstacle collision using SECONDARY zones (FUN_403b5b) ----
		// Original lines 52-132: Point-in-quad test with 15px inward margin.
		// Inside the quad = collision with obstacle.
		const int margin = 15;  // local_14 = 0x0f, local_20 = 0x0f

		for (int i = 0; i < _secondaryZoneCount; i++) {
			CollisionZone &zone = _secondaryZones[i];
			if (!zone.active) continue;

			int x1 = zone.x1, y1 = zone.y1;
			int x2 = zone.x2, y2 = zone.y2;
			int x3 = zone.x3, y3 = zone.y3;
			int x4 = zone.x4, y4 = zone.y4;

			// Point-in-quad test (lines 75-89)
			// Start assuming inside, clear if outside any edge (with margin)
			bool inside = true;

			// Top edge: interpolate Y along v1→v2 at shipX, +15 margin
			if (x2 != x1) {
				int interpY = (_flyShipScreenX - x1) * (y2 - y1) / (x2 - x1) + margin + y1;
				if (_flyShipScreenY < interpY) inside = false;
			}
			// Bottom edge: interpolate Y along v4→v3 at shipX, -15 margin
			if (inside && x3 != x4) {
				int interpY = (_flyShipScreenX - x4) * (y3 - y4) / (x3 - x4) + y4 - margin;
				if (interpY < _flyShipScreenY) inside = false;
			}
			// Left edge: interpolate X along v1→v4 at shipY, +15 margin
			if (inside && y4 != y1) {
				int interpX = (_flyShipScreenY - y1) * (x4 - x1) / (y4 - y1) + margin + x1;
				if (_flyShipScreenX < interpX) inside = false;
			}
			// Right edge: interpolate X along v2→v3 at shipY, -15 margin
			if (inside && y3 != y2) {
				int interpX = (_flyShipScreenY - y2) * (x3 - x2) / (y3 - y2) + x2 - margin;
				if (interpX < _flyShipScreenX) inside = false;
			}

			// Frame match: field2 - 1 == field1 (line 90)
			if (zone.field2 - 1 == zone.field1) {
				if (inside) {
					// Collision with obstacle — apply damage and break
					_hitCooldown = 10;
					_spaceShotDirection = zone.filterValue + 2;

					int collisionDamage = 3 + (_difficulty * 2);
					if (!_rebelInvulnerable) {
						_playerDamage += collisionDamage;
						if (_playerDamage > 255) _playerDamage = 255;
					}
					_rebelHitCounter++;
					initDamageFlash();
					// Pan based on ship X position relative to screen center
					playSfx(1, 127, CLIP((_flyShipScreenX - 212) * 127 / 160, -127, 127));
					debug("Rebel2: Handler7 Mode0/2 OBSTACLE HIT zone=%d ship=(%d,%d) damage=%d",
						i, _flyShipScreenX, _flyShipScreenY, collisionDamage);
					break;  // Only one collision per frame (original breaks)
				} else {
					// Safely avoided obstacle — award score
					addScore(1);
				}
			}
		}

		// Corridor boundary proximity (lines 127-131)
		// These flags are used for directional indicators (not critical for damage)

	} else {
		// ---- Mode 1/3: Wall/boundary collision using PRIMARY zones (FUN_403b34) ----
		// Original lines 133-235: Per-edge interpolation with push-back.
		// Ship position is clamped to wall boundaries when hitting.
		int16 hMargin = (_flyControlMode == 1) ? 0x28 : 0x0f;  // local_14
		const int16 vMargin = 0x0f;  // local_20

		for (int i = 0; i < _primaryZoneCount; i++) {
			CollisionZone &zone = _primaryZones[i];
			if (!zone.active) continue;

			int x1 = zone.x1, y1 = zone.y1;
			int x2 = zone.x2, y2 = zone.y2;
			int x3 = zone.x3, y3 = zone.y3;
			int x4 = zone.x4, y4 = zone.y4;

			// Top edge: interpolate Y along v1→v2 at shipX (lines 152-166)
			if (x2 != x1) {
				int16 edgeY = (int16)((_flyShipScreenX - x1) * (y2 - y1) / (x2 - x1) + y1 + vMargin);
				if (_flyShipScreenY < edgeY) {
					// Ship above top wall — push down
					if (_hitCooldown < 5 && !_rebelInvulnerable) {
						int damage = 3 + (_difficulty * 2);
						_playerDamage += damage;
						if (_playerDamage > 255) _playerDamage = 255;
						_rebelHitCounter++;
						_hitCooldown = 10;
						playSfx(1, 127, 0);  // CRASH.SAD, top wall → center pan
						debug("Rebel2: Handler7 Mode1/3 TOP WALL ship=(%d,%d) edgeY=%d damage=%d",
							_flyShipScreenX, _flyShipScreenY, edgeY, damage);
					}
					_spaceShotDirection = 2;  // Direction: pushed down
					_flyShipScreenY = edgeY;  // Push-back
					initDamageFlash();
				}
			}

			// Bottom edge: interpolate Y along v4→v3 at shipX (lines 167-183)
			if (x3 != x4) {
				int16 edgeY = (int16)((_flyShipScreenX - x4) * (y3 - y4) / (x3 - x4) + y4 - vMargin);
				_corridorBottomY = vMargin + edgeY;  // DAT_00443b10 update
				if (edgeY < _flyShipScreenY) {
					// Ship below bottom wall — push up
					if (_hitCooldown < 5 && !_rebelInvulnerable) {
						int damage = 3 + (_difficulty * 2);
						_playerDamage += damage;
						if (_playerDamage > 255) _playerDamage = 255;
						_rebelHitCounter++;
						_hitCooldown = 10;
						playSfx(1, 127, 0);  // CRASH.SAD, bottom wall → center pan
						debug("Rebel2: Handler7 Mode1/3 BOTTOM WALL ship=(%d,%d) edgeY=%d damage=%d",
							_flyShipScreenX, _flyShipScreenY, edgeY, damage);
					}
					_spaceShotDirection = 3;  // Direction: pushed up
					_flyShipScreenY = edgeY;  // Push-back
					initDamageFlash();
				}
			}

			// Left edge: interpolate X along v1→v4 at shipY (lines 184-199)
			if (y4 != y1) {
				int16 edgeX = (int16)((_flyShipScreenY - y1) * (x4 - x1) / (y4 - y1) + x1 + hMargin);
				if (_flyShipScreenX < edgeX) {
					// Ship left of left wall — push right
					_flyShipScreenX = edgeX;  // Push-back
					if (_hitCooldown < 5 && !_rebelInvulnerable) {
						int damage = 3 + (_difficulty * 2);
						_playerDamage += damage;
						if (_playerDamage > 255) _playerDamage = 255;
						_rebelHitCounter++;
						_hitCooldown = 10;
						playSfx(1, 127, -100);  // CRASH.SAD, left wall → pan left
						debug("Rebel2: Handler7 Mode1/3 LEFT WALL ship=(%d,%d) edgeX=%d damage=%d",
							_flyShipScreenX, _flyShipScreenY, edgeX, damage);
					}
					_spaceShotDirection = 0;  // Direction: pushed right
					initDamageFlash();
				}
			}

			// Right edge: interpolate X along v2→v3 at shipY (lines 200-215)
			if (y3 != y2) {
				int16 edgeX = (int16)((_flyShipScreenY - y2) * (x3 - x2) / (y3 - y2) + x2 - hMargin);
				if (edgeX < _flyShipScreenX) {
					// Ship right of right wall — push left
					_flyShipScreenX = edgeX;  // Push-back
					if (_hitCooldown < 5 && !_rebelInvulnerable) {
						int damage = 3 + (_difficulty * 2);
						_playerDamage += damage;
						if (_playerDamage > 255) _playerDamage = 255;
						_rebelHitCounter++;
						_hitCooldown = 10;
						playSfx(1, 127, 100);  // CRASH.SAD, right wall → pan right
						debug("Rebel2: Handler7 Mode1/3 RIGHT WALL ship=(%d,%d) edgeX=%d damage=%d",
							_flyShipScreenX, _flyShipScreenY, edgeX, damage);
					}
					_spaceShotDirection = 1;  // Direction: pushed left
					initDamageFlash();
				}
			}
		}
	}
}

void InsaneRebel2::drawQuad(byte *dst, int pitch, int width, int height,
                            int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, byte color) {
	// Draw a quadrilateral by connecting its 4 vertices with lines
	// Vertex order: top-left (1), top-right (2), bottom-right (3), bottom-left (4)
	drawLine(dst, pitch, width, height, x1, y1, x2, y2, color);  // Top edge
	drawLine(dst, pitch, width, height, x2, y2, x3, y3, color);  // Right edge
	drawLine(dst, pitch, width, height, x3, y3, x4, y4, color);  // Bottom edge
	drawLine(dst, pitch, width, height, x4, y4, x1, y1, color);  // Left edge
}

void InsaneRebel2::drawCollisionZones(byte *dst, int pitch, int width, int height, byte color) {
	// Draw all active collision zones as wireframe quadrilaterals for debugging
	// Uses different colors for primary vs secondary zones

	const byte primaryColor = 44;    // Bright red for primary (obstacle) zones
	const byte secondaryColor = 47;  // Yellow for secondary (boundary) zones

	// Draw primary zones (sub-opcode 0x0D - obstacles)
	for (int i = 0; i < _primaryZoneCount; i++) {
		CollisionZone &zone = _primaryZones[i];
		if (!zone.active) continue;

		// Apply view offset to convert from video coords to screen coords
		int x1 = zone.x1 + _viewX;
		int y1 = zone.y1 + _viewY;
		int x2 = zone.x2 + _viewX;
		int y2 = zone.y2 + _viewY;
		int x3 = zone.x3 + _viewX;
		int y3 = zone.y3 + _viewY;
		int x4 = zone.x4 + _viewX;
		int y4 = zone.y4 + _viewY;

		drawQuad(dst, pitch, width, height, x1, y1, x2, y2, x3, y3, x4, y4, primaryColor);
	}

	// Draw secondary zones (sub-opcode 0x0E - boundaries)
	for (int i = 0; i < _secondaryZoneCount; i++) {
		CollisionZone &zone = _secondaryZones[i];
		if (!zone.active) continue;

		// Apply view offset
		int x1 = zone.x1 + _viewX;
		int y1 = zone.y1 + _viewY;
		int x2 = zone.x2 + _viewX;
		int y2 = zone.y2 + _viewY;
		int x3 = zone.x3 + _viewX;
		int y3 = zone.y3 + _viewY;
		int x4 = zone.x4 + _viewX;
		int y4 = zone.y4 + _viewY;

		drawQuad(dst, pitch, width, height, x1, y1, x2, y2, x3, y3, x4, y4, secondaryColor);
	}

	// Draw corridor boundaries as a rectangle (from IACT opcode 7)
	if (_corridorLeftX != 0 || _corridorRightX != 0x1A8) {
		const byte corridorColor = 45;  // Cyan for corridor boundaries
		// Draw vertical lines for left/right boundaries
		drawLine(dst, pitch, width, height,
			_corridorLeftX + _viewX, _corridorTopY + _viewY,
			_corridorLeftX + _viewX, _corridorBottomY + _viewY, corridorColor);
		drawLine(dst, pitch, width, height,
			_corridorRightX + _viewX, _corridorTopY + _viewY,
			_corridorRightX + _viewX, _corridorBottomY + _viewY, corridorColor);
		// Draw horizontal lines for top/bottom boundaries
		drawLine(dst, pitch, width, height,
			_corridorLeftX + _viewX, _corridorTopY + _viewY,
			_corridorRightX + _viewX, _corridorTopY + _viewY, corridorColor);
		drawLine(dst, pitch, width, height,
			_corridorLeftX + _viewX, _corridorBottomY + _viewY,
			_corridorRightX + _viewX, _corridorBottomY + _viewY, corridorColor);
	}
}

void InsaneRebel2::renderNutSprite(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx) {
	renderNutSpriteMirrored(dst, pitch, width, height, x, y, nut, spriteIdx, false);
}

// Render NUT sprite with optional horizontal mirroring
// Based on FUN_004236e0 disassembly - flags=0x2001 triggers horizontal flip
void InsaneRebel2::renderNutSpriteMirrored(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx, bool mirror) {
	if (!nut || spriteIdx < 0 || spriteIdx >= nut->getNumChars()) return;

	int w = nut->getCharWidth(spriteIdx);
	int h = nut->getCharHeight(spriteIdx);
	const byte *src = nut->getCharData(spriteIdx);

	// Clipping
	int drawX = x;
	int drawY = y;
	int drawW = w;
	int drawH = h;
	int srcOffsetX = 0;
	int srcOffsetY = 0;

	if (drawX < 0) {
		srcOffsetX = -drawX;
		drawW += drawX;
		drawX = 0;
	}
	if (drawY < 0) {
		srcOffsetY = -drawY;
		drawH += drawY;
		drawY = 0;
	}

	if (drawX + drawW > width) {
		drawW = width - drawX;
	}
	if (drawY + drawH > height) {
		drawH = height - drawY;
	}

	if (drawW <= 0 || drawH <= 0) return;

	// Draw loop - with optional horizontal mirroring
	for (int iy = 0; iy < drawH; iy++) {
		const byte *s = src + (srcOffsetY + iy) * w;
		byte *d = dst + (drawY + iy) * pitch + drawX;
		for (int ix = 0; ix < drawW; ix++) {
			int srcX;
			if (mirror) {
				// When mirrored, read from the opposite side of the sprite
				srcX = (w - 1) - (srcOffsetX + ix);
			} else {
				srcX = srcOffsetX + ix;
			}
			byte px = s[srcX];
			if (px != 231 && px != 0) { // Check both 0 and 231 (0xE7) for transparency
				d[ix] = px;
			}
		}
	}
}

void InsaneRebel2::procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
							   int32 setupsan13, int32 curFrame, int32 maxFrame) {

	// Determine correct pitch for the video buffer (usually 320 for Rebel2)
	int width = _player->_width;
	int height = _player->_height;
	if (width == 0) width = _vm->_screenWidth;
	if (height == 0) height = _vm->_screenHeight;
	int pitch = width;

	// Calculate View/Scroll Offsets
	// Rebel Assault 2 uses a buffer larger (424x260) than screen (320x200)
	// Map mouse X (0-320) to Scroll X (0-104)
	// Map mouse Y (0-200) to Scroll Y (0-60)
	int maxScrollX = width - _vm->_screenWidth;
	int maxScrollY = height - _vm->_screenHeight;
	
	if (maxScrollX < 0) maxScrollX = 0;
	if (maxScrollY < 0) maxScrollY = 0;
	
	// Simple linear mapping: Center of screen corresponds to center of buffer
	_viewX = (_vm->_mouse.x * maxScrollX) / _vm->_screenWidth;
	_viewY = (_vm->_mouse.y * maxScrollY) / _vm->_screenHeight;
	
	_player->setScrollOffset(_viewX, _viewY);

	// --- HUD Drawing Order (from FUN_004089ab assembly analysis) ---
	// Based on FUN_004089ab:
	// 1. Line 156: FUN_004288c0 fills status bar background at Y=0xb4 (180)
	// 2. Lines 171-226: Draw turret overlays, targeting reticle, crosshair
	// 3. Line 243: FUN_0041c012 draws status bar sprites LAST (on top)
	//
	// In FUN_0041c012:
	// - Sprites are drawn to buffer DAT_00482204 at position (0,0)
	// - Buffer is composited at Y=0xb4 (180) via FUN_0042f780
	// - DISPFONT.NUT (DAT_00482200) sprites 1-7 contain the status bar elements
	//
	// We draw directly to screen at Y=180
	
	// Use video content coordinates, NOT buffer coordinates
	const int videoWidth = 320;    // Native video width
	const int videoHeight = 200;   // Native video height  
	const int statusBarY = 180;    // 0xb4 - status bar starts at Y=180 in video coords

	// Hide HUD/status bar during intro videos (marked by SmushPlayer video flag 0x20)
	// The 0x20 flag indicates a non-interactive cutscene/intro sequence OR menu
	bool introPlaying = ((_player->_curVideoFlags & 0x20) != 0);

	// Check if we're in menu mode (menu state + intro flag)
	bool menuMode = (introPlaying && _gameState == kStateMainMenu);
	bool pilotSelectMode = (introPlaying && (_gameState == kStatePilotSelect || _gameState == kStateDifficultySelect));
	bool chapterSelectMode = (introPlaying && _gameState == kStateChapterSelect);

	// Handle pilot selection input and rendering (FUN_00414A41)
	// This is the pilot/save slot selection screen with centered menu
	if (pilotSelectMode) {
		// Show the standard Windows arrow cursor
		Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
		CursorMan.replaceCursor(cursor);
		delete cursor;
		CursorMan.showMouse(true);

		// Process pilot selection input - emulates FUN_00414A41 input handling
		int selection = processLevelSelectInput();

		// Draw pilot selection overlay - centered menu like main menu
		drawLevelSelectOverlay(renderBitmap, pitch, width, height);

		// If a selection was confirmed, signal video to stop
		if (selection >= 0) {
			debug("Rebel2: Pilot selection confirmed: %d", selection);
			_vm->_smushVideoShouldFinish = true;
		}

		// Skip normal HUD rendering in pilot select mode
		return;
	}

	// Handle chapter selection input and rendering (FUN_00415CF8)
	// This is the actual level/chapter selection screen with preview and password
	if (chapterSelectMode) {
		// Show the standard Windows arrow cursor (same as menu)
		Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
		CursorMan.replaceCursor(cursor);
		delete cursor;
		CursorMan.showMouse(true);

		// O_LEVEL.SAN provides the background with chapter preview thumbnails.
		// The FOBJ offset system (set in procPreRendering) scrolls the correct preview
		// into the preview box area. No black fill needed — video frame shows through.

		// Process chapter selection input - emulates FUN_00415CF8 input handling
		int selection = processChapterSelectInput();

		// Draw chapter selection overlay - emulates FUN_00415CF8 rendering
		drawChapterSelectOverlay(renderBitmap, pitch, width, height);

		// If a selection was confirmed, signal video to stop
		if (selection >= 0) {
			debug("Rebel2: Chapter selection confirmed: %d", selection);
			_vm->_smushVideoShouldFinish = true;
		}

		// Skip normal HUD rendering in chapter select mode
		return;
	}

	// Handle menu input and rendering if in menu mode
	if (menuMode) {
		// The original game uses the standard Windows arrow cursor (IDC_ARROW)
		// loaded via LoadCursorA(NULL, 0x7f00) in FUN_420C70.decompiled.txt
		// MSTOVER.NUT is a background overlay, NOT a cursor
		Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
		CursorMan.replaceCursor(cursor);
		delete cursor;
		CursorMan.showMouse(true);

		// Process menu input during each frame
		int selection = processMenuInput();

		// Update inactivity timer (only increments when no input is received)
		// Input resets timer in processMenuInput()
		_menuInactivityTimer++;

		// Check for inactivity timeout
		// From FUN_004147b2: 300 frames of inactivity returns 0 (exit to intro/attract mode)
		// At 12fps video rate, 300 frames = ~25 seconds of inactivity
		// The original checks: if (local_8 > 299) return 0;
		if (_menuInactivityTimer > 300) {
			debug("Rebel2: Menu inactivity timeout - ending video to loop");
			// Signal video to end so menu loop plays new video
			// This emulates the attract mode behavior where a new random
			// menu video is selected after inactivity
			_menuInactivityTimer = 0;
			// Don't set _smushVideoShouldFinish here - let video end naturally
			// This will cause runMainMenu to loop and play a new random video
		}

		// Draw menu selection overlay
		drawMenuOverlay(renderBitmap, pitch, width, height);

		// If a selection was confirmed, signal video to stop
		if (selection >= 0) {
			debug("Rebel2: Menu selection confirmed: %d", selection);
			_vm->_smushVideoShouldFinish = true;
		}

		// Skip normal HUD rendering in menu mode
		return;
	}

	// During intro/cinematic sequences:
	// - Hide the mouse cursor (original: ShowCursor(0) at startup in FUN_00420c70)
	// - Skip all HUD/status bar/crosshair rendering
	// - Skip mouse input processing (no shooting during intros)
	//
	// Original behavior from FUN_00403240:
	// - if (DAT_0047a814 == 0) { switch(DAT_0047ee84) { ... } }
	// - DAT_0047ee84 (handler) is only set by IACT opcode 6 during gameplay videos
	// - Cinematics/intros don't have opcode 6, so handler stays 0
	// - We use _rebelHandler == 0 as the primary indicator for intro/cinematic mode
	if (_rebelHandler == 0) {
		// Hide mouse cursor during intro - no crosshair, no clicking
		CursorMan.showMouse(false);

		// Track state transition for debugging
		if (!_introCursorPushed) {
			_introCursorPushed = true;
			debug("Rebel2: Intro/cinematic mode (handler=0, flags=0x%x, state=%d) - HUD disabled, mouse hidden",
				  _player->_curVideoFlags, _gameState);
		}
		// Skip all HUD rendering during intro - subtitles are rendered via opcode 9
		return;
	} else {
		// Gameplay mode - handler was set by IACT opcode 6
		if (_introCursorPushed) {
			_introCursorPushed = false;
			debug("Rebel2: Gameplay mode (handler=%d, flags=0x%x, state=%d) - HUD enabled",
				  _rebelHandler, _player->_curVideoFlags, _gameState);
		}
	}

	// From here on, we're in gameplay mode (_rebelHandler != 0)
	// Process mouse input for shooting
	// Original: FUN_00403240 only runs handlers when DAT_0047a814 == 0
	processMouse();

	// NOTE: Level 2 background is drawn ONCE during IACT opcode 8 par4=5 processing
	// (in procIACT when the background ANIM is first loaded). The 0x08 video flag
	// (preserve background) prevents the frame buffer from being cleared, so the
	// background persists. FOBJ sprites (enemies) are then decoded on top by SMUSH.
	// We do NOT redraw the background here as that would overwrite FOBJ content.

	// --- HUD Drawing Order (from FUN_004089ab assembly analysis) ---
	// 1. FUN_004288c0: Fill status bar background at Y=0xb4 (180)
	// 2. FUN_004089ab: Draw turret overlays, targeting reticle, crosshair
	// 3. FUN_0041c012: Draw status bar sprites LAST (on top)

	// STEP 0: Fill status bar background (FUN_004288c0)
	renderStatusBarBackground(renderBitmap, pitch, width, height, videoWidth, videoHeight, statusBarY);

	// STEP 1A: Draw NUT-based HUD overlays for Handler 0x26/0x19 (FUN_004089ab lines 195-226)
	renderTurretHudOverlays(renderBitmap, pitch, width, height, curFrame);

	// STEP 1B: Draw embedded SAN HUD overlays (from IACT chunks)
	renderEmbeddedHudOverlays(renderBitmap, pitch, width, height);

	// STEP 2: Draw DISPFONT.NUT status bar sprites (FUN_0041c012)
	renderStatusBarSprites(renderBitmap, pitch, width, height, statusBarY, curFrame);

	// Ship rendering (FUN_00401ccf for Handler 8, FUN_0040d836 for Handler 7)
	debug("Rebel2 Ship Check: handler=%d shipSprite=%p flyShipSprite=%p shipLevelMode=%d numSprites=%d/%d",
		_rebelHandler, (void*)_shipSprite, (void*)_flyShipSprite, _shipLevelMode,
		_shipSprite ? _shipSprite->getNumChars() : 0,
		_flyShipSprite ? _flyShipSprite->getNumChars() : 0);

	renderHandler7Ship(renderBitmap, pitch, width, height);
	renderHandler8Ship(renderBitmap, pitch, width, height);
	// GRD001 (wall/cockpit) drawn AFTER FOBJs per original FUN_0041DB5E lines 202-210
	renderHandler25ShipPre(renderBitmap, pitch, width, height);
	renderHandler25Ship(renderBitmap, pitch, width, height);
	renderFallbackShip(renderBitmap, pitch, width, height);

	// Enemy indicators and destroyed enemy area erase
	renderEnemyOverlays(renderBitmap, pitch, width, height, videoWidth);

	// Explosion animations (FUN_409FBC)
	renderExplosions(renderBitmap, pitch, width, height);

	// Laser shot beams and impacts
	renderLaserShots(renderBitmap, pitch, width, height);

	// Damage visual effects — handler-specific per original architecture:
	//   Handler 8:    FUN_401CCF line 119 → FUN_00420754 (palette flash + screen shake)
	//   Handler 0x19: FUN_41DB5E line 192 → FUN_00420562 (palette flash only, every frame)
	//   Handler 0x26: FUN_4092D9 lines 135/225/237 → FUN_00420515 trigger + palette flash
	//   Handler 7:    FUN_40E35E → FUN_00420515 trigger + palette flash
	if (_rebelHandler == 8) {
		// Full damage effect: palette flash + screen shake
		// Suppressed during autopilot (mode 4) and cutscene (mode 5)
		if (_shipLevelMode != 4 && _shipLevelMode != 5) {
			updateDamageEffect(renderBitmap, pitch, width, height);
		}
	} else if (_rebelHandler == 0x19 || _rebelHandler == 0x26 || _rebelHandler == 7) {
		// Palette flash only — no screen shake for turret/FPS/ship handlers
		updateDamageFlashPalette();
	}

	// Per-frame collision checking against registered zones.
	//
	// Handler 0x26 (turret): FUN_4092D9 — aim position vs primary zones (centered coords)
	//   Zones with filterValue < 1000 tested via point-in-quad against mouse/aim position.
	//
	// Handler 7 (ship): FUN_40E35E — ship position vs zones per control mode:
	//   Mode 0/2: SECONDARY zones (0x0E) — obstacle collision (inside quad = hit)
	//   Mode 1/3: PRIMARY zones (0x0D) — wall/boundary per-edge with push-back
	//   Uses ship position in raw buffer coords, hit cooldown, directional damage.
	if (_rebelHandler == 0x26) {
		checkCollisionZones();
	} else if (_rebelHandler == 7) {
		checkHandler7CollisionZones();
	}

	// Collision zone visualization (debug - for Handler 7/8 pilot modes)
	if (_rebelHandler == 7 || _rebelHandler == 8) {
		drawCollisionZones(renderBitmap, pitch, width, height, 0);
	}

	// Crosshair/reticle (FUN_004089ab, FUN_0040d836)
	renderCrosshair(renderBitmap, pitch, width, height);

	// HUD score/lives rendering (FUN_0041c012)
	renderScoreHUD(renderBitmap, pitch, width, height, 0);

	// Reset FOBJ position offsets (FUN_00424510(0,0) in original FUN_0041DB5E line 271)
	if (_player) {
		_player->_fobjOffsetX = 0;
		_player->_fobjOffsetY = 0;
	}

	// Frame end cleanup: reset enemy active flags and collision zones (FUN_403240)
	frameEndCleanup();
}

// ======================= Damage Visual Effect Functions =======================
// Palette flash + screen shake when the player takes damage.
// Original retail functions: FUN_420515, FUN_420562, FUN_420754, FUN_42073B, FUN_420501

// FUN_00420501 - Reset palette flash counter.
// Called at level start / scene transitions to clear any in-progress flash.
void InsaneRebel2::resetDamageFlash() {
	_damageFlashCounter = 0;
}

// FUN_00420515 - Save current palette and initiate a 5-frame flash.
// If a flash is already in progress, just resets the counter to 5
// (the palette was already saved on the first hit).
void InsaneRebel2::initDamageFlash() {
	if (_damageFlashCounter == 0) {
		// Save current SMUSH palette before modifying it
		memcpy(_damageSavedPalette, _player->_pal, 0x300);
	}
	_damageFlashCounter = 5;
}

// FUN_0042073B - Trigger both palette flash and screen shake.
// Called from the damage hit handler when the player takes damage.
void InsaneRebel2::triggerDamageEffect() {
	initDamageFlash();
	_damageShakeCounter = 10;
}

// FUN_00420562 - Per-frame palette modification.
//
// Two modes determined by _damageHighFlashCounter:
//
//   Normal hit flash (_damageHighFlashCounter == 0 or odd):
//     Decrements _damageFlashCounter. On even counter values, all 768 palette
//     bytes (RGB) are blended from inverted toward the saved original:
//       output[i] = 0xFF - ((0xFF - saved[i]) * (0x10 - counter)) >> 4
//     Counter 5→4(apply)→3(skip)→2(apply)→1(skip)→0(apply=original). The
//     alternating apply/skip creates a strobe-like flash effect.
//
//   High-damage red pulse (_playerDamage >= 0xFF, even counter):
//     Only the R channel (every 3rd byte) is modified using the same formula
//     with _damageHighFlashCounter. Creates a pulsing red tint overlay.
void InsaneRebel2::updateDamageFlashPalette() {
	// High-damage mode: persistent red pulsing when damage is maxed out
	if (_playerDamage < 0xFF) {
		_damageHighFlashCounter = 0;
	} else {
		if (_damageHighFlashCounter == 0) {
			// Save palette on first frame of high-damage mode
			memcpy(_damageSavedPalette, _player->_pal, 0x300);
		}
		if (_damageHighFlashCounter < 0x10) {
			_damageHighFlashCounter++;
		}
	}

	if (_damageHighFlashCounter == 0 || (_damageHighFlashCounter & 1) != 0) {
		// Normal hit flash path: decrement counter, apply on even values.
		// Original C: if ((counter != 0) && (counter--, (counter & 1) == 0))
		if (_damageFlashCounter != 0) {
			_damageFlashCounter--;
			if ((_damageFlashCounter & 1) == 0) {
				// Apply palette inversion on ALL RGB channels
				byte modPal[0x300];
				int blend = 0x10 - _damageFlashCounter;
				for (int i = 0; i < 0x300; i++) {
					modPal[i] = 0xFF - (((0xFF - _damageSavedPalette[i]) * blend) >> 4);
				}
				_player->setPalette(modPal);
			}
		}
	} else {
		// High-damage red-only flash (even _damageHighFlashCounter):
		// Modify only R channel (stride 3), G and B stay unchanged.
		byte modPal[0x300];
		memcpy(modPal, _player->_pal, 0x300);
		int blend = 0x10 - _damageHighFlashCounter;
		for (int i = 0; i < 0x300; i += 3) {
			modPal[i] = 0xFF - (((0xFF - _damageSavedPalette[i]) * blend) >> 4);
		}
		_player->setPalette(modPal);
	}
}

// FUN_00420754 - Per-frame screen shake + palette flash.
//
// Screen shake randomly shifts scanlines left or right for visual distortion.
// The number of affected scanlines decreases each frame (counter * 5),
// creating a diminishing shake effect over 10 frames.
//
// Called every frame from procPostRendering when not in cutscene modes
// (shipLevelMode != 4 and != 5, matching original: DAT_0043e000 != 4 && != 5).
void InsaneRebel2::updateDamageEffect(byte *renderBitmap, int pitch, int width, int height) {
	if (_damageShakeCounter != 0) {
		_damageShakeCounter--;
		int numLines = _damageShakeCounter * 5;

		// Temporary buffer for scanline rotation (case 1 in original)
		byte tempLine[640];

		for (int n = numLines; n > 0; n--) {
			// Pick a random scanline within the gameplay area (0..179, not status bar)
			int maxY = MIN(height, 180);
			int scanline = _vm->_rnd.getRandomNumber(maxY - 1);

			byte *linePtr = renderBitmap + pitch * scanline;
			int offset = _vm->_rnd.getRandomNumber(4) + 1;  // 1..5 pixel shift
			int direction = _vm->_rnd.getRandomNumber(4);    // 0..4

			int copyLen = pitch - offset;
			if (copyLen <= 0)
				continue;

			switch (direction) {
			case 0:
			case 3:
				// Shift left: copy line[offset..] -> line[0..]
				memmove(linePtr, linePtr + offset, copyLen);
				break;
			case 1:
				// Shift right with wrap: save, then copy
				memcpy(tempLine, linePtr, MIN(copyLen, (int)sizeof(tempLine)));
				memmove(linePtr + offset, tempLine, MIN(copyLen, (int)sizeof(tempLine)));
				break;
			case 2:
			case 4:
				// Shift right: copy line[0..] -> line[offset..]
				memmove(linePtr + offset, linePtr, copyLen);
				break;
			}
		}
	}

	// Palette flash runs every frame (even without shake)
	updateDamageFlashPalette();
}

// ======================= Rendering Helper Functions =======================
// These are extracted from procPostRendering for better readability

void InsaneRebel2::renderStatusBarBackground(byte *renderBitmap, int pitch, int width, int height,
											 int videoWidth, int videoHeight, int statusBarY) {
	// Fill status bar background (FUN_004288c0 equivalent)
	// Original assembly: FUN_004288c0(local_8, 0, 0, 0xb4, 0x140, 0x14, 4)
	// This fills width=320, height=20 starting at Y=180 with color index 4
	const byte statusBarBgColor = 4;

	for (int y = statusBarY; y < videoHeight; y++) {
		int destY = y + _viewY;
		if (destY >= height) continue;
		for (int x = 0; x < videoWidth; x++) {
			int destX = x + _viewX;
			if (destX >= pitch) continue;
			renderBitmap[destY * pitch + destX] = statusBarBgColor;
		}
	}
}

void InsaneRebel2::renderTurretHudOverlays(byte *renderBitmap, int pitch, int width, int height, int32 curFrame) {
	// Draw NUT-based HUD overlays for Handler 0x26/0x19 (turret modes)
	// From FUN_004089ab disassembly (lines 195-226):
	// - DAT_0047fe78 (_hudOverlayNut): Primary HUD overlay with 6 animation frames
	// - Position formula (low-res):
	//   X = 160 + (mouseOffsetX >> 4) - (width / 2) - spriteOffsetX
	//   Y = 182 - (mouseOffsetY >> 4) - height - spriteOffsetY
	// - Animation: spriteIndex = (frameCounter / 2) % 6

	if ((_rebelHandler != 0x26 && _rebelHandler != 0x19) || !_hudOverlayNut || _hudOverlayNut->getNumChars() <= 0)
		return;

	// Calculate mouse offset (clamped to -127..127)
	int mouseOffsetX = (_vm->_mouse.x - 160);
	int mouseOffsetY = (_vm->_mouse.y - 100);
	if (mouseOffsetX > 127) mouseOffsetX = 127;
	if (mouseOffsetX < -127) mouseOffsetX = -127;
	if (mouseOffsetY > 127) mouseOffsetY = 127;
	if (mouseOffsetY < -127) mouseOffsetY = -127;

	// Animation frame cycling: (frameCounter / 2) % 6
	int numSprites = _hudOverlayNut->getNumChars();
	int animFrameCount = MIN(numSprites, 6);
	int animFrame = 0;
	if (animFrameCount > 0) {
		animFrame = (curFrame / 2) % animFrameCount;
	}

	// Get sprite dimensions
	int spriteW = _hudOverlayNut->getCharWidth(animFrame);
	int spriteH = _hudOverlayNut->getCharHeight(animFrame);

	// Position calculation from assembly (low-res mode)
	int spriteOffsetX = 0;
	int spriteOffsetY = 0;
	int hudX = 160 + (mouseOffsetX >> 4) - (spriteW / 2) - spriteOffsetX;
	int hudY = 182 - (mouseOffsetY >> 4) - spriteH - spriteOffsetY;

	// Apply view offset for scrolling background
	hudX += _viewX;
	hudY += _viewY;

	// Draw base cockpit (sprite 0 always drawn first)
	renderNutSprite(renderBitmap, pitch, width, height, hudX, hudY, _hudOverlayNut, 0);

	// Draw animation overlay frame if not frame 0
	if (animFrame != 0 && animFrame < numSprites) {
		renderNutSprite(renderBitmap, pitch, width, height, hudX, hudY, _hudOverlayNut, animFrame);
	}

	debug(5, "Rebel2 HUD: Drawing NUT overlay frame %d/%d at (%d,%d) mouseOffset=(%d,%d)",
		  animFrame, numSprites, hudX, hudY, mouseOffsetX, mouseOffsetY);

	// Draw secondary HUD overlay if present (DAT_0047fe80)
	if (_hudOverlay2Nut && _hudOverlay2Nut->getNumChars() > 0) {
		int spr2W = _hudOverlay2Nut->getCharWidth(0);
		int spr2H = _hudOverlay2Nut->getCharHeight(0);
		int hud2X = 160 + (mouseOffsetX >> 4) - (spr2W / 2) + _viewX;
		int hud2Y = 182 - (mouseOffsetY >> 4) - spr2H + _viewY;
		renderNutSprite(renderBitmap, pitch, width, height, hud2X, hud2Y, _hudOverlay2Nut, 0);
	}
}

void InsaneRebel2::renderEmbeddedHudOverlays(byte *renderBitmap, int pitch, int width, int height) {
	// Draw embedded SAN HUD overlays (from IACT chunks)
	// For Handler 7 (Level 3): HUD elements are scattered across the screen
	// For turret handlers: slots 1-2 form a two-part cockpit overlay

	for (int hudSlot = 1; hudSlot < 16; hudSlot++) {
		EmbeddedSanFrame &frame = _rebelEmbeddedHud[hudSlot];
		if (!frame.valid || !frame.pixels || frame.width <= 0 || frame.height <= 0)
			continue;

		// Handler 25: Skip slot 4 (corridor overlay) in post-rendering.
		// The corridor is a full background image (no color 0 transparent center).
		// Drawing it here would cover enemies. It's already drawn in procPreRendering
		// with transparency to preserve frame persistence for codec 23 delta.
		if (_rebelHandler == 25 && hudSlot == 4) {
			continue;
		}

		// Skip small frames at (0,0) - likely animation patches
		if (frame.renderX == 0 && frame.renderY == 0 && frame.width < 50 && frame.height < 60) {
			debug(3, "Rebel2: Skipping small embedded frame at (0,0): slot=%d size=%dx%d",
				hudSlot, frame.width, frame.height);
			continue;
		}

		// For Handler 7: handle direction-based frame selection
		if (_rebelHandler == 7) {
			int groupMembers[16];
			int groupCount = 0;

			for (int id = 1; id < 16; id++) {
				EmbeddedSanFrame &g = _rebelEmbeddedHud[id];
				if (g.valid && g.renderX == frame.renderX && g.renderY == frame.renderY &&
					g.width == frame.width && g.height == frame.height) {
					groupMembers[groupCount++] = id;
				}
			}

			if (groupCount > 1) {
				int selectedOffset = _shipDirectionIndex % groupCount;
				int selectedId = groupMembers[selectedOffset];

				// Verify selected frame has pixels
				EmbeddedSanFrame &selectedFrame = _rebelEmbeddedHud[selectedId];
				int nonZero = 0;
				for (int i = 0; i < selectedFrame.width * selectedFrame.height; i++) {
					if (selectedFrame.pixels[i] != 0) nonZero++;
				}

				if (nonZero == 0) {
					for (int i = 0; i < groupCount; i++) {
						EmbeddedSanFrame &altFrame = _rebelEmbeddedHud[groupMembers[i]];
						int altNonZero = 0;
						for (int j = 0; j < altFrame.width * altFrame.height; j++) {
							if (altFrame.pixels[j] != 0) altNonZero++;
						}
						if (altNonZero > 0) {
							selectedId = groupMembers[i];
							break;
						}
					}
				}

				if (hudSlot != selectedId)
					continue;
			}
		}

		// Calculate destination position
		int destX = frame.renderX;
		int destY = frame.renderY;

		// Handler 0x26/0x19 turret positioning
		if ((_rebelHandler == 0x26 || _rebelHandler == 0x19) && (hudSlot == 1 || hudSlot == 2)) {
			destX = 160 - frame.width / 2 - frame.renderX;
			destY = 200 - frame.height - frame.renderY;
		}

		// Handler 7 large cockpit frame positioning
		if (_rebelHandler == 7 && (hudSlot == 1 || hudSlot == 2) && frame.width > 100) {
			destX = 160 - frame.width / 2 - frame.renderX;
			destY = 170 - frame.height - frame.renderY;
		} else if (_rebelHandler == 7 && destX > 100 && destY > 50) {
			int16 offsetX = (_shipPosX - 160) / 8;
			int16 offsetY = (_shipPosY - 100) / 8;
			destX += offsetX;
			destY += offsetY;
		}

		destX += _viewX;
		destY += _viewY;

		debug(3, "Rebel2: Rendering embedded HUD slot=%d size=%dx%d at (%d,%d)",
			hudSlot, frame.width, frame.height, destX, destY);

		// Draw frame with transparency (pixel 0 and 231 = transparent)
		for (int y = 0; y < frame.height && (destY + y) < height; y++) {
			for (int x = 0; x < frame.width && (destX + x) < pitch; x++) {
				byte pixel = frame.pixels[y * frame.width + x];
				if (pixel != 0 && pixel != 231) {
					int fx = destX + x;
					int fy = destY + y;
					if (fx >= 0 && fy >= 0) {
						renderBitmap[fy * pitch + fx] = pixel;
					}
				}
			}
		}
	}
}

void InsaneRebel2::renderStatusBarSprites(byte *renderBitmap, int pitch, int width, int height,
										  int statusBarY, int32 curFrame) {
	// Draw DISPFONT.NUT status bar sprites (FUN_0041c012 equivalent)
	// DISPFONT.NUT contains:
	//   Sprite 1: Status bar background frame
	//   Sprites 2-5: Difficulty stars (1-4)
	//   Sprite 6: Damage bar fill (with clip rect X=63, Y=9, W=64, H=6)
	//   Sprite 7: Damage alert (flashing red when critical)

	if (!_smush_cockpitNut)
		return;

	// Sprite 1: Status bar background
	if (_smush_cockpitNut->getNumChars() > 1) {
		renderNutSprite(renderBitmap, pitch, width, height, _viewX, statusBarY + _viewY, _smush_cockpitNut, 1);
	}

	// Difficulty indicator (sprites 2-5)
	int difficulty = 0;  // TODO: Read from game state
	if (difficulty > 3) difficulty = 3;
	int difficultySprite = difficulty + 2;
	if (_smush_cockpitNut->getNumChars() > difficultySprite) {
		renderNutSprite(renderBitmap, pitch, width, height, _viewX, statusBarY + _viewY, _smush_cockpitNut, difficultySprite);
	}

	// Damage bar (sprite 6) with clipped width
	if (_smush_cockpitNut->getNumChars() > 6) {
		int drawWidth = (64 * _playerDamage) / 255;
		if (drawWidth < 0) drawWidth = 0;
		if (drawWidth > 64) drawWidth = 64;

		const byte *src = _smush_cockpitNut->getCharData(6);
		int sw = _smush_cockpitNut->getCharWidth(6);
		int sh = _smush_cockpitNut->getCharHeight(6);

		// Clip rect inside sprite: X=63, Y=9, W=64, H=6
		const int clipX = 63, clipY = 9, clipW = 64, clipH = 6;

		if (src && sw > 0 && sh > 0) {
			int maxClipW = sw - clipX;
			if (maxClipW < 0) maxClipW = 0;
			int drawW = MIN(drawWidth, MIN(clipW, maxClipW));
			int drawH = MIN(clipH, sh - clipY);
			if (drawH < 0) drawH = 0;

			for (int y = 0; y < drawH; y++) {
				for (int x = 0; x < drawW; x++) {
					int destX = clipX + x + _viewX;
					int destY = statusBarY + clipY + y + _viewY;
					if (destX >= 0 && destX < pitch && destY >= 0 && destY < height) {
						byte pixel = src[(clipY + y) * sw + (clipX + x)];
						if (pixel != 0) {
							renderBitmap[destY * pitch + destX] = pixel;
						}
					}
				}
			}
		}
	}

	// Damage alert overlay (sprite 7) when damage > 170 and flashing
	if (_smush_cockpitNut->getNumChars() > 7) {
		if (_playerDamage > 170 && ((curFrame & 8) == 0)) {
			renderNutSprite(renderBitmap, pitch, width, height, 63 + _viewX, statusBarY + 9 + _viewY, _smush_cockpitNut, 7);
		}
	}
}

void InsaneRebel2::renderHandler7Ship(byte *renderBitmap, int pitch, int width, int height) {
	// Handler 7 Ship Rendering (Third-Person Ship - FLY sprites)
	// Based on FUN_0040d836 lines 173-185:
	//   FUN_004236e0(buf, frameInfo, screenX - 0xd4, screenY - 0x82, 0, sprite, frameIdx, 1, 0)
	// The ship sprite is drawn at the perspective-transformed position offset from center.
	// FUN_0041c720 transforms game coords (shipX, shipY) using perspective offsets.

	if (_rebelHandler != 7 || !_flyShipSprite || _shipLevelMode == 5)
		return;

	int numSprites = _flyShipSprite->getNumChars();
	int spriteIndex = _shipDirectionIndex;
	if (spriteIndex < 0) spriteIndex = 0;
	if (spriteIndex >= numSprites) spriteIndex = numSprites - 1;

	// Transform game coordinates to screen coordinates (FUN_0041c720 equivalent)
	// The perspective transform shifts the ship position based on perspective offsets.
	// Close view: FOBJ offset = (-52 - perspX, -45 - perspY), ship at screen center.
	// For now, use a simplified perspective: ship position = center + offset from center
	// scaled by perspective. In the original, FUN_00424510 shifts all FOBJ sprites.
	//
	// Screen position for sprite drawing (FUN_0040d836 line 174):
	//   drawX = transformedX - 0xd4, drawY = transformedY - 0x82
	// Where transformedX/Y come from FUN_0041c720(shipX, shipY, perspX, perspY, viewShift)
	//
	// Simplified: screenX = 160 + (shipX - 212) * perspFactor
	// With the perspective formula, objects near center barely move, objects at edges move more.
	int drawX = (_flyShipScreenX - 0xd4) + _perspectiveX;
	int drawY = (_flyShipScreenY - 0x82) + _perspectiveY;

	// Convert from game-center-relative to screen coordinates
	// The sprite system expects coordinates relative to the 320x200 frame
	// Center of frame = (160, 100), so offset = game position - game center
	drawX += 160 + _viewX;
	drawY += 100 + _viewY;

	// Center the sprite on the position
	int spriteW = _flyShipSprite->getCharWidth(spriteIndex);
	int spriteH = _flyShipSprite->getCharHeight(spriteIndex);
	drawX -= spriteW / 2;
	drawY -= spriteH / 2;

	renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _flyShipSprite, spriteIndex);

	// Laser overlay if firing (same position as ship)
	if (_shipFiring && _flyLaserSprite && _flyLaserSprite->getNumChars() > 0) {
		int laserIndex = spriteIndex % _flyLaserSprite->getNumChars();
		renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _flyLaserSprite, laserIndex);
	}

	debug("Rebel2 Handler7Ship: draw=(%d,%d) sprite=%d/%d shipPos=(%d,%d) persp=(%d,%d) smoothVel=%d vertIn=%d",
		drawX, drawY, spriteIndex, numSprites, _flyShipScreenX, _flyShipScreenY,
		_perspectiveX, _perspectiveY, _smoothedVelocity, _verticalInput);
}

void InsaneRebel2::renderHandler8Ship(byte *renderBitmap, int pitch, int width, int height) {
	// Handler 8 Ship Rendering (Third-Person On Foot - POV sprites)
	// Uses _shipSprite (POV001) with position-based offset

	if (_rebelHandler != 8 || !_shipSprite || _shipLevelMode == 5)
		return;

	// Calculate display offset from raw ship position (FUN_00401ccf lines 88-89)
	int16 displayOffsetX = (_shipPosX - 0xa0) >> 3;
	int16 displayOffsetY = (_shipPosY - 0x28) >> 2;

	// Base screen position (low-res: X=160, Y=105)
	int shipScreenX = 0xa0 + displayOffsetX;
	int shipScreenY = 0x69 + displayOffsetY;

	int numSprites = _shipSprite->getNumChars();
	int spriteIndex = 0;

	// Select sprite based on direction and sprite count
	if (numSprites >= 35) {
		spriteIndex = _shipDirectionH * 7 + _shipDirectionV;
		if (spriteIndex >= numSprites) spriteIndex = numSprites - 1;
	} else if (numSprites >= 25) {
		int vDir5 = (_shipDirectionV * 5) / 7;
		spriteIndex = _shipDirectionH * 5 + vDir5;
		if (spriteIndex >= numSprites) spriteIndex = numSprites - 1;
	} else if (numSprites >= 5) {
		spriteIndex = _shipDirectionH;
		if (spriteIndex >= numSprites) spriteIndex = numSprites - 1;
	} else if (numSprites == 2) {
		spriteIndex = _shipFiring ? 1 : 0;
	}

	// Center sprite at position
	int spriteW = _shipSprite->getCharWidth(spriteIndex);
	int spriteH = _shipSprite->getCharHeight(spriteIndex);
	int drawX = shipScreenX - spriteW / 2 + _viewX;
	int drawY = shipScreenY - spriteH / 2 + _viewY;

	renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _shipSprite, spriteIndex);

	// Shadow sprite (POV004 / DAT_0047e028): drawn at same position as primary ship.
	// Original FUN_401CCF lines 91-92 uses param_5 & 1 (firing flag) as sprite index
	// for both primary and shadow, NOT the direction-based spriteIndex.
	if (_shipSprite2) {
		int shadowIndex = _shipFiring ? 1 : 0;
		if (shadowIndex < _shipSprite2->getNumChars()) {
			renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _shipSprite2, shadowIndex);
		}
	}

	debug("Rebel2 Handler8: Ship at (%d,%d) raw(%d,%d) offset(%d,%d) sprite=%d/%d dir=(%d,%d)",
		drawX, drawY, _shipPosX, _shipPosY, displayOffsetX, displayOffsetY,
		spriteIndex, numSprites, _shipDirectionH, _shipDirectionV);
}

// Handler 25: Draw GRD001 (wall/cockpit overlay) in procPostRendering.
// Per original FUN_0041DB5E, GRD sprites are drawn AFTER FOBJ enemies, before GRD002.
//
// From FUN_0041db5e disassembly (lines 202-221):
// - Mode 1 with damage==0: Width halved (left half only, pixels 0-159)
// - Mode 4 with damage==0: Width halved AND buffer offset (right half only, pixels 160-319)
// - All other cases: Full width (320 pixels)
void InsaneRebel2::renderHandler25ShipPre(byte *renderBitmap, int pitch, int width, int height) {
	if (_rebelHandler != 25)
		return;

	if (!_grd001Sprite || _grd001Sprite->getNumChars() <= 0)
		return;

	// CRITICAL: Clip height to 180 (0xb4) to avoid drawing over status bar
	const int clipHeight = 180;
	int renderHeight = MIN(height, clipHeight);

	// Draw _grd001Sprite based on _grdSpriteMode (DAT_00457900)
	// Each mode has specific conditions from FUN_0041db5e:
	bool shouldDraw = false;
	bool useHalfWidth = false;
	bool useRightHalf = false;

	// Mode 1 (lines 202-210): Draw with width halving when uncovered
	if (_grdSpriteMode == 1) {
		shouldDraw = true;
		useHalfWidth = (_rebelDamageLevel == 0);  // Half width when uncovered
	}
	// Mode 2 (lines 222-224): Only draw when damaged (covered)
	else if (_grdSpriteMode == 2 && _rebelDamageLevel != 0) {
		shouldDraw = true;
	}
	// Mode 3 (lines 225-228): Always draw full width
	else if (_grdSpriteMode == 3) {
		shouldDraw = true;
	}
	// Mode 4 (lines 211-221): Draw to right half when uncovered
	else if (_grdSpriteMode == 4) {
		shouldDraw = true;
		useHalfWidth = (_rebelDamageLevel == 0);
		useRightHalf = (_rebelDamageLevel == 0);
	}

	if (shouldDraw) {
		int spriteW = _grd001Sprite->getCharWidth(0);
		int spriteH = _grd001Sprite->getCharHeight(0);
		int16 spriteXOffset = _grd001Sprite->getCharXOffset(0);
		int16 spriteYOffset = _grd001Sprite->getCharYOffset(0);

		int drawX = _rebelViewOffset2X + spriteXOffset;
		int drawY = _rebelViewOffset2Y + spriteYOffset;

		// Apply width-halving logic from original assembly:
		// When damage==0 (uncovered), the original halves DAT_00482234 (buffer width)
		// This clips the sprite to only half the screen.
		int renderWidth = width;
		byte *dstBitmap = renderBitmap;

		if (useHalfWidth) {
			renderWidth = width / 2;  // Clip to half width (160 pixels)

			if (useRightHalf) {
				// Mode 4: Draw to right half by offsetting the destination buffer
				// Original: DAT_00482230 += DAT_00482234 (adds 160 to buffer start)
				// This makes drawing appear on the right half (pixels 160-319)
				dstBitmap = renderBitmap + (width / 2);
			}
		}

		renderNutSprite(dstBitmap, pitch, renderWidth, renderHeight, drawX, drawY, _grd001Sprite, 0);

		debug("Rebel2 Handler25 PRE: GRD001 at (%d,%d) nutOff(%d,%d) viewOff(%d,%d) size(%d,%d) mode=%d dmg=%d halfW=%d rightHalf=%d renderW=%d",
			drawX, drawY, spriteXOffset, spriteYOffset, _rebelViewOffset2X, _rebelViewOffset2Y,
			spriteW, spriteH, _grdSpriteMode, _rebelDamageLevel, useHalfWidth ? 1 : 0, useRightHalf ? 1 : 0, renderWidth);
	}
}

void InsaneRebel2::renderHandler25Ship(byte *renderBitmap, int pitch, int width, int height) {
	// Handler 25 POST-rendering: Draw GRD002 (character sprite) on top of enemies.
	// GRD001 (wall/cockpit) is drawn before this via renderHandler25ShipPre().
	//
	// From FUN_0041db5e disassembly (lines 230-248):
	// GRD002 is drawn LAST (after enemies) so the character appears in front.

	if (_rebelHandler != 25)
		return;

	// CRITICAL: Clip height to 180 (0xb4) to avoid drawing over status bar
	const int clipHeight = 180;
	int renderHeight = MIN(height, clipHeight);

	// _grd002Sprite (GRD002) is always drawn if it exists (from FUN_41DB5E line 230)
	// The sprite index is calculated based on damage level and aiming position
	// From FUN_0041db5e lines 160-168:
	//   If damage == 0: index = yZone * 5 + xZone + 5 (aiming-based, 5-14)
	//   If damage != 0:
	//     If direction == 0: index = 5 - damage (0-5, covered left)
	//     If direction != 0: index = 25 - damage (20-25, covered right)
	if (_grd002Sprite && _grd002Sprite->getNumChars() > 0) {
		// Calculate sprite index based on damage level and direction
		int spriteIdx;
		int numSprites = _grd002Sprite->getNumChars();

		// Determine if we should mirror the sprite (from FUN_41DB5E lines 231-235)
		// Mirror when: direction != 0 AND damage == 0 (fully uncovered, facing right)
		bool shouldMirror = (_rebelFlightDir != 0) && (_rebelDamageLevel == 0);

		if (_rebelDamageLevel == 0) {
			// Uncovered state: use aiming-based sprite selection (5-14)
			// Calculate zones from crosshair position relative to playable area
			// From FUN_41DB5E lines 155-164
			//
			// The playable area bounds are defined by corridor boundaries.
			// xZone = 0-4 (left to right), yZone = 0-1 (top to bottom)
			// Default to center if bounds not set
			int16 areaLeft = (_corridorLeftX > 0) ? _corridorLeftX : 0;
			int16 areaRight = (_corridorRightX > 0) ? _corridorRightX : 320;
			int16 areaTop = (_corridorTopY > 0) ? _corridorTopY : 0;
			int16 areaBottom = (_corridorBottomY > 0) ? _corridorBottomY : 180;

			// Get crosshair position (using mouse position scaled to game coords)
			int16 crosshairX = _vm->_mouse.x;
			int16 crosshairY = _vm->_mouse.y;
			if (_player && _player->_width > 320) {
				crosshairX = (crosshairX * 320) / _player->_width;
				crosshairY = (crosshairY * 200) / _player->_height;
			}

			// Calculate zone widths
			int areaWidth = areaRight - areaLeft;
			int areaHeight = areaBottom - areaTop;
			int zoneWidth = (areaWidth > 0) ? (areaWidth + 3) / 4 : 80;  // Divide into ~4 zones
			int zoneHeight = (areaHeight > 0) ? areaHeight / 2 : 90;     // Divide into 2 zones

			// Calculate xZone (0-4) and yZone (0-1) from crosshair position
			int xZone = (zoneWidth > 0) ? ((zoneWidth / 2) + (crosshairX - areaLeft)) / zoneWidth : 2;
			int yZone = (zoneHeight > 0) ? ((zoneHeight / 2) + (crosshairY - areaTop)) / zoneHeight : 0;

			// Clamp to valid ranges
			if (xZone < 0) xZone = 0;
			if (xZone > 4) xZone = 4;
			if (yZone < 0) yZone = 0;
			if (yZone > 1) yZone = 1;

			// Direction-based sprite flip logic (line 161-162 in decompiled)
			// if (DAT_00457902 == (uVar7 & 1)) { local_58 = 4 - local_58; }
			if (_rebelFlightDir == (yZone & 1)) {
				xZone = 4 - xZone;
			}

			spriteIdx = yZone * 5 + xZone + 5;
		} else {
			// Transitioning/covered state: use direction-based sprite
			// From FUN_41DB5E lines 166-168:
			// sVar8 = ((-(ushort)(DAT_00457902 == 0) & 0xffec) + 0x19) - DAT_0045790a
			// direction == 0: 5 - damage
			// direction != 0: 25 - damage
			if (_rebelFlightDir == 0) {
				// Direction 0: sprites 0-5 (transition left)
				spriteIdx = 5 - _rebelDamageLevel;
			} else {
				// Direction 1: sprites 20-25 (transition right)
				spriteIdx = 25 - _rebelDamageLevel;
			}
		}

		// Clamp to valid range
		if (spriteIdx < 0) spriteIdx = 0;
		if (spriteIdx >= numSprites) spriteIdx = numSprites - 1;

		int spriteW = _grd002Sprite->getCharWidth(spriteIdx);
		int spriteH = _grd002Sprite->getCharHeight(spriteIdx);

		// Position calculation from FUN_41DB5E lines 237-247:
		// GRD002 explicitly adds sprite internal offsets from NUT header:
		//
		// Normal case (direction==0 OR damage!=0):
		//   local_60 = sprite_internal_x_offset (from NUT header +0x12)
		//   X = DAT_00457910 + local_60
		//   Y = sprite_internal_y_offset (from NUT header +0x14) + DAT_00457912
		//
		// Mirrored case (direction!=0 AND damage==0):
		//   local_60 = 320 - sprite_width - sprite_internal_x_offset
		//   X = DAT_00457910 + local_60
		//   Y = sprite_internal_y_offset + DAT_00457912
		//
		// Now using actual NUT sprite offsets from NutRenderer!
		int16 spriteXOffset = _grd002Sprite->getCharXOffset(spriteIdx);
		int16 spriteYOffset = _grd002Sprite->getCharYOffset(spriteIdx);

		int drawX, drawY;

		if (shouldMirror) {
			// Mirrored position: X = DAT_00457910 + (320 - sprite_width - sprite_x_offset)
			// From assembly lines 240-243
			drawX = _rebelViewOffset2X + (320 - spriteW - spriteXOffset);
		} else {
			// Normal position: X = DAT_00457910 + sprite_internal_x_offset
			// From assembly line 238
			drawX = _rebelViewOffset2X + spriteXOffset;
		}

		// Y = sprite_internal_y_offset + DAT_00457912
		// From assembly line 246
		drawY = spriteYOffset + _rebelViewOffset2Y;

		renderNutSpriteMirrored(renderBitmap, pitch, width, renderHeight, drawX, drawY, _grd002Sprite, spriteIdx, shouldMirror);

		debug("Rebel2 Handler25: GRD002 at (%d,%d) nutOffset(%d,%d) viewOffset(%d,%d) size(%d,%d) spriteIdx=%d damage=%d dir=%d mirror=%d",
			drawX, drawY, spriteXOffset, spriteYOffset, _rebelViewOffset2X, _rebelViewOffset2Y, spriteW, spriteH, spriteIdx, _rebelDamageLevel, _rebelFlightDir, shouldMirror ? 1 : 0);
	}
}

void InsaneRebel2::renderFallbackShip(byte *renderBitmap, int pitch, int width, int height) {
	// Fallback: Use embedded HUD frame as ship sprite (Level 3 style)
	// userId=11 contains the ship sprite strip

	if ((_rebelHandler != 7 && _rebelHandler != 8) || _shipLevelMode == 5)
		return;

	// Skip if we have proper sprites
	if (_rebelHandler == 7 && _flyShipSprite)
		return;
	if (_rebelHandler == 8 && _shipSprite)
		return;

	EmbeddedSanFrame &shipFrame = _rebelEmbeddedHud[11];
	if (!shipFrame.valid || !shipFrame.pixels || shipFrame.width <= 0 || shipFrame.height <= 0)
		return;

	// Calculate display offset
	int16 displayOffsetX = (_shipPosX - 0xa0) >> 3;
	int16 displayOffsetY = (_shipPosY - 0x28) >> 2;
	int shipScreenX = 0xa0 + displayOffsetX;
	int shipScreenY = 0x69 + displayOffsetY;

	// Detect sprite strip layout
	int spriteW = shipFrame.width;
	int spriteH = shipFrame.height;
	int srcX = 0, srcY = 0;
	int numHorizontal = 1, numVertical = 1;

	if (spriteW >= 200 && spriteW % 5 == 0) {
		numHorizontal = 5;
		spriteW = shipFrame.width / 5;
	}
	if (spriteH >= 350 && spriteH % 7 == 0) {
		numVertical = 7;
		spriteH = shipFrame.height / 7;
	}

	int hDir = MIN((int)_shipDirectionH, numHorizontal - 1);
	int vDir = MIN((int)_shipDirectionV, numVertical - 1);
	srcX = hDir * spriteW;
	srcY = vDir * spriteH;

	int drawX = shipScreenX - spriteW / 2 + _viewX;
	int drawY = shipScreenY - spriteH / 2 + _viewY;

	// Blit from embedded HUD
	for (int y = 0; y < spriteH && (drawY + y) < height; y++) {
		if (drawY + y < 0) continue;
		for (int x = 0; x < spriteW && (drawX + x) < width; x++) {
			if (drawX + x < 0) continue;
			int srcIdx = (srcY + y) * shipFrame.width + (srcX + x);
			byte pixel = shipFrame.pixels[srcIdx];
			if (pixel != 0 && pixel != 231) {
				int dstIdx = (drawY + y) * pitch + (drawX + x);
				renderBitmap[dstIdx] = pixel;
			}
		}
	}

	debug("Rebel2: Ship (fallback) at (%d,%d) strip=(%d,%d) of (%dx%d) dir=(%d,%d)",
		drawX, drawY, srcX, srcY, numHorizontal, numVertical, _shipDirectionH, _shipDirectionV);
}

void InsaneRebel2::renderEnemyOverlays(byte *renderBitmap, int pitch, int width, int height, int videoWidth) {
	// Draw enemy indicator brackets for active enemies
	//
	// NOTE: Do NOT fill destroyed enemy areas with black. The original game does not do this.
	// When an enemy is destroyed:
	// 1. setBit(enemy_id) disables the enemy in the bit table
	// 2. clearBit(dependency_id) enables dependent objects (explosion animations)
	// 3. SKIP chunks in the video cause enemy FOBJ sprites to be skipped (via procSKIP)
	// 4. renderExplosions() draws the explosion animation from the 5-slot system
	// 5. The background video shows through where the enemy was

	// Draw green brackets for active enemies (Easy/Medium difficulty only)
	if (_difficulty >= 2)
		return;

	// FOBJ sprites are rendered with _fobjOffsetX/Y applied (set from _rebelViewOffsetX/Y
	// for Handler 25). Brackets must use the same offset so they align with the sprites.
	int fobjOffX = _player ? _player->_fobjOffsetX : 0;
	int fobjOffY = _player ? _player->_fobjOffsetY : 0;

	Common::Rect viewRect(_viewX, _viewY, _viewX + videoWidth, _viewY + 200);

	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->destroyed || !it->active || isBitSet(it->id))
			continue;

		Common::Rect r(it->rect.left + fobjOffX, it->rect.top + fobjOffY,
		               it->rect.right + fobjOffX, it->rect.bottom + fobjOffY);
		if (r.right <= viewRect.left || r.left >= viewRect.right ||
		    r.bottom <= viewRect.top || r.top >= viewRect.bottom)
			continue;

		const byte color = 5;  // Green
		drawCornerBrackets(renderBitmap, pitch, width, height, r.left, r.top, r.width(), r.height(), color);
	}
}

// Dispatcher — calls per-handler explosion render function.
// Original code has separate functions per handler, each with its own
// position transformation, scale thresholds, and secondary NUT rendering.
void InsaneRebel2::renderExplosions(byte *renderBitmap, int pitch, int width, int height) {
	switch (_rebelHandler) {
	case 0x26:
		renderTurretExplosions(renderBitmap, pitch, width, height);
		break;
	case 8:
		renderVehicleExplosions(renderBitmap, pitch, width, height);
		break;
	case 7:
		renderSpaceExplosions(renderBitmap, pitch, width, height);
		break;
	case 25:
		renderHandler25Explosions(renderBitmap, pitch, width, height);
		break;
	default:
		break;
	}
}

// FUN_409FBC — Handler 0x26 (Turret/Cockpit) explosion rendering.
// Position: Uses FUN_0041c720 for 3D→2D projection. At low-res, world coords ≈ screen coords.
// Scale thresholds: Fixed (<11, <21).
// Secondary NUT: DAT_0047fe80 (rendered if DAT_0047a7fc >= 0).
// Hi-res: Coordinates doubled when DAT_0047a808 >= 2.
void InsaneRebel2::renderTurretExplosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		if (_explosions[i].counter <= 0) {
			_explosions[i].active = false;
			continue;
		}

		// FUN_409FBC: Fixed thresholds (0x0b=11, 0x15=21)
		int baseIndex;
		if (_explosions[i].scale < 11) {
			baseIndex = 9;   // Small (sprites 11-20)
		} else if (_explosions[i].scale < 21) {
			baseIndex = 19;  // Medium (sprites 21-30)
		} else {
			baseIndex = 29;  // Large (sprites 31-40)
		}

		int spriteIndex = baseIndex + (12 - _explosions[i].counter);

		// Position: world coords passed through FUN_0041c720 (3D→2D projection).
		// At 320x200 low-res turret view, projection is effectively identity.
		int screenX = _explosions[i].x;
		int screenY = _explosions[i].y;

		if (_smush_iconsNut->getNumChars() > spriteIndex) {
			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			renderNutSprite(renderBitmap, pitch, width, height,
				screenX - ew / 2, screenY - eh / 2, _smush_iconsNut, spriteIndex);
		}

		_explosions[i].counter--;
	}
}

// FUN_402696 — Handler 8 (Third-Person On-Foot) explosion rendering.
// Position: World coords minus camera offset (DAT_0043e006/DAT_0043e008 = _viewX/_viewY).
// Scale thresholds: Fixed (<11, <21) — same as handler 0x26.
// Secondary NUT: None (only DAT_0047a828).
void InsaneRebel2::renderVehicleExplosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		if (_explosions[i].counter <= 0) {
			_explosions[i].active = false;
			continue;
		}

		// FUN_402696: Fixed thresholds (0x0b=11, 0x15=21)
		int baseIndex;
		if (_explosions[i].scale < 11) {
			baseIndex = 9;
		} else if (_explosions[i].scale < 21) {
			baseIndex = 19;
		} else {
			baseIndex = 29;
		}

		int spriteIndex = baseIndex + (12 - _explosions[i].counter);

		// FUN_402696 line 22-23: screenX = worldX - DAT_0043e006, screenY = worldY - DAT_0043e008
		int screenX = _explosions[i].x - _viewX;
		int screenY = _explosions[i].y - _viewY;

		if (_smush_iconsNut->getNumChars() > spriteIndex) {
			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			renderNutSprite(renderBitmap, pitch, width, height,
				screenX - ew / 2, screenY - eh / 2, _smush_iconsNut, spriteIndex);
		}

		_explosions[i].counter--;
	}
}

// FUN_40F1C5 — Handler 7 (Third-Person Ship) explosion rendering.
// Position: Uses FUN_0041c720 for 3D→2D projection.
// Scale thresholds: Resolution-dependent (low-res: <11/<21, high-res: <21/<41).
// Secondary NUT: DAT_0047ff00 (FLY004, rendered if DAT_0047a7fc >= 0).
void InsaneRebel2::renderSpaceExplosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	// --- Part 1: Space shot explosions (FUN_40F1C5 lines 19-60) ---
	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		if (_explosions[i].counter <= 0) {
			_explosions[i].active = false;
			continue;
		}

		// FUN_40F1C5 lines 41-51: Resolution-dependent thresholds.
		// Low-res (DAT_0047a808 < 2): thresholds 20, 10
		// High-res: thresholds 40, 20
		// We run at low-res (320x200), so use 10/20 (same as fixed handlers).
		int baseIndex;
		if (_explosions[i].scale < 11) {
			baseIndex = 9;
		} else if (_explosions[i].scale < 21) {
			baseIndex = 19;
		} else {
			baseIndex = 29;
		}

		int spriteIndex = baseIndex + (12 - _explosions[i].counter);

		// Position: world coords through FUN_0041c720 (3D→2D projection).
		// At low-res, this is close to identity for the ship view.
		int screenX = _explosions[i].x;
		int screenY = _explosions[i].y;

		if (_smush_iconsNut->getNumChars() > spriteIndex) {
			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			renderNutSprite(renderBitmap, pitch, width, height,
				screenX - ew / 2, screenY - eh / 2, _smush_iconsNut, spriteIndex);
		}

		_explosions[i].counter--;
	}

	// --- Part 2: Corridor/zone hit explosion (FUN_40F1C5 lines 61-85) ---
	// Rendered when _hitCooldown > 0 (DAT_0044374c). Decrement happens HERE
	// (matching original where FUN_40F1C5 decrements DAT_0044374c during render).
	// _spaceShotDirection (DAT_0044374e) determines explosion side:
	//   0 = left side (hit left boundary), 1 = right side (hit right boundary)
	//   2 = bottom (zone push down), 3 = top (zone push up)
	// Sprite frames: 0x15 - cooldown = 21 - cooldown (frames 12→21 as cooldown 9→0)
	if (_hitCooldown != 0) {
		_hitCooldown--;

		int numChars = _smush_iconsNut->getNumChars();
		int spriteIndex = 0x15 - _hitCooldown;  // 21 - remaining cooldown

		if (spriteIndex >= 0 && spriteIndex < numChars) {
			// Compute ship screen position (simplified FUN_0041c720 transform)
			int shipDrawX = (_flyShipScreenX - 0xd4) + _perspectiveX + 160 + _viewX;
			int shipDrawY = (_flyShipScreenY - 0x82) + _perspectiveY + 100 + _viewY;

			// Per-direction offset from ship center.
			// Original uses lookup tables (DAT_004438da etc.) indexed by
			// _shipDirectionIndex (35 entries per direction). We approximate
			// with fixed offsets since we don't have the table data.
			int offsetX = 0, offsetY = 0;
			switch (_spaceShotDirection) {
			case 0:  // Left wall hit → explosion on left side of ship
				offsetX = -35;
				break;
			case 1:  // Right wall hit → explosion on right side of ship
				offsetX = 35;
				break;
			case 2:  // Zone push down → explosion on bottom
				offsetY = 20;
				break;
			case 3:  // Zone push up → explosion on top
				offsetY = -20;
				break;
			default:
				break;
			}

			int drawX = shipDrawX + offsetX;
			int drawY = shipDrawY + offsetY;

			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			renderNutSprite(renderBitmap, pitch, width, height,
				drawX - ew / 2, drawY - eh / 2, _smush_iconsNut, spriteIndex);

			debug("Rebel2 H7 corridor explosion: dir=%d frame=%d pos=(%d,%d) cooldown=%d",
				_spaceShotDirection, spriteIndex, drawX, drawY, _hitCooldown);
		}
	}
}

// FUN_41F29A — Handler 25 (FPS/Mixed) explosion rendering.
// Position: World coords + view offset (DAT_0045790c/DAT_0045790e = _rebelViewOffsetX/_rebelViewOffsetY).
// Scale thresholds: Resolution-dependent (same formula as Handler 7).
// Secondary NUT: DAT_00482260 (hi-res HUD alternative, rendered if DAT_0047a7fc >= 0).
// Note: No per-frame sound panning update (unlike handlers 0x26, 8, 7).
void InsaneRebel2::renderHandler25Explosions(byte *renderBitmap, int pitch, int width, int height) {
	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		if (_explosions[i].counter <= 0) {
			_explosions[i].active = false;
			continue;
		}

		// FUN_41F29A lines 27-37: Resolution-dependent thresholds (same as Handler 7).
		int baseIndex;
		if (_explosions[i].scale < 11) {
			baseIndex = 9;
		} else if (_explosions[i].scale < 21) {
			baseIndex = 19;
		} else {
			baseIndex = 29;
		}

		int spriteIndex = baseIndex + (12 - _explosions[i].counter);

		// FUN_41F29A line 22-23: screenX = worldX + DAT_0045790c, screenY = worldY + DAT_0045790e
		int screenX = _explosions[i].x + _rebelViewOffsetX;
		int screenY = _explosions[i].y + _rebelViewOffsetY;

		if (_smush_iconsNut->getNumChars() > spriteIndex) {
			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			renderNutSprite(renderBitmap, pitch, width, height,
				screenX - ew / 2, screenY - eh / 2, _smush_iconsNut, spriteIndex);
		}

		_explosions[i].counter--;
	}
}

// Dispatcher - calls appropriate render function based on current handler
void InsaneRebel2::renderLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	switch (_rebelHandler) {
	case 0x26:  // Turret - FUN_40AD63
		renderTurretLaserShots(renderBitmap, pitch, width, height);
		break;
	case 8:     // Vehicle - FUN_402ED0
		renderVehicleLaserShots(renderBitmap, pitch, width, height);
		break;
	case 7:     // Space combat - FUN_40FADF
		renderSpaceLaserShots(renderBitmap, pitch, width, height);
		break;
	case 25:    // Speeder bike - FUN_0041f004
		renderHandler25LaserShots(renderBitmap, pitch, width, height);
		break;
	default:
		// No laser rendering for other handlers
		break;
	}
}

// Handler 0x26 Turret laser rendering (FUN_40AD63)
// Gun positions depend on _rebelLevelType (DAT_004436de)
void InsaneRebel2::renderTurretLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// Uses pre-initialized _laserTexture from sprite 5 of CPITIMAG.NUT

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter <= 0)
			continue;

		// Calculate sound panning from target X position (FUN_004262f0 call)
		// sVar1 = ((2 - counter) * (targetX - 160)) / 2, clamped to [-127, 127]
		int16 pan = ((2 - _turretShots[i].counter) * (_turretShots[i].targetX - _viewX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning to sound channel i+1

		int16 targetX = _turretShots[i].targetX;
		int16 targetY = _turretShots[i].targetY;
		int16 progress = maxDuration - _turretShots[i].counter;

		// Gun positions based on level type (from FUN_40AD63 switch statement)
		// Parameters from assembly: widthScale=0xC(12), heightScale=8, thickness=0xC(12)
		switch (_rebelLevelType) {
		case 1:
			// Type 1: 3 guns (triple cannon configuration)
			// Gun 1: (0x136, 0xaa) = (310, 170)
			// Gun 2: (0xa0, 0x17c) = (160, 380)
			// Gun 3: (0x0a, 0xaa) = (10, 170)
			drawLaserBeam(renderBitmap, pitch, width, height,
				310 + _viewX, 170 + _viewY, targetX, targetY,
				progress, maxDuration, 12, 8, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				160 + _viewX, 380 + _viewY, targetX, targetY,
				progress, maxDuration, 12, 8, 12);

			drawLaserBeam(renderBitmap, pitch, width, height,
				10 + _viewX, 170 + _viewY, targetX, targetY,
				progress, maxDuration, 12, 8, 12);
			break;

		case 2:
		case 5:
			// Type 2/5: 2 guns (wing cannons)
			// Left: (0x6e, 0xe6) = (110, 230)
			// Right: (0xd2, 0xe6) = (210, 230)
			// Assembly uses widthScale=0x19(25) for these types
			drawLaserBeam(renderBitmap, pitch, width, height,
				110 + _viewX, 230 + _viewY, targetX, targetY,
				progress, maxDuration, 25, 8, 25);

			drawLaserBeam(renderBitmap, pitch, width, height,
				210 + _viewX, 230 + _viewY, targetX, targetY,
				progress, maxDuration, 25, 8, 25);
			break;

		case 6:
			// Type 6: 2 guns (offscreen - cinematic effect)
			// Gun 1: (-100, 0)
			// Gun 2: (0, 0)
			drawLaserBeam(renderBitmap, pitch, width, height,
				-100 + _viewX, 0 + _viewY, targetX, targetY,
				progress, maxDuration, 25, 8, 25);

			drawLaserBeam(renderBitmap, pitch, width, height,
				0 + _viewX, 0 + _viewY, targetX, targetY,
				progress, maxDuration, 25, 8, 25);
			break;

		default:
			// Default: 2 guns with alternating pattern based on shot sequence
			// When seqNum & 1 == 0: Left (10, 50), Right (310, 130)
			// When seqNum & 1 == 1: Left (310, 50), Right (10, 130)
			if ((_turretShots[i].seqNum & 1) == 0) {
				drawLaserBeam(renderBitmap, pitch, width, height,
					10 + _viewX, 50 + _viewY, targetX, targetY,
					progress, maxDuration, 25, 8, 25);

				drawLaserBeam(renderBitmap, pitch, width, height,
					310 + _viewX, 130 + _viewY, targetX, targetY,
					progress, maxDuration, 25, 8, 25);
			} else {
				drawLaserBeam(renderBitmap, pitch, width, height,
					310 + _viewX, 50 + _viewY, targetX, targetY,
					progress, maxDuration, 25, 8, 25);

				drawLaserBeam(renderBitmap, pitch, width, height,
					10 + _viewX, 130 + _viewY, targetX, targetY,
					progress, maxDuration, 25, 8, 25);
			}
			break;
		}

		_turretShots[i].counter--;
	}
}

// Handler 8 Vehicle laser rendering (FUN_402ED0)
// In the original, the laser is a short muzzle flash from gun barrel toward ship center,
// NOT a projectile traveling across the screen. The "hit" effect is handled separately.
void InsaneRebel2::renderVehicleLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// No NUT check needed - uses pre-initialized _laserTexture

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_vehicleShots[i].counter <= 0)
			continue;

		// Calculate sound panning from STORED target position (FUN_402ED0 lines 24-51)
		// pan = ((2 - counter) * (targetX - 160)) / 2, clamped to [-127, 127]
		int16 pan = ((2 - _vehicleShots[i].counter) * (_vehicleShots[i].targetX - _viewX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning

		// Calculate positions from CURRENT ship position (FUN_402ED0 lines 53-122)
		// The original game draws the laser from gun position toward ship center,
		// creating a short muzzle flash effect (7 pixels horizontal, 25 pixels vertical).
		//
		// Low-res formula (DAT_0047a808 < 2):
		// shipScreenY = ((shipPosY - 0x28) >> 2) + 0x69 = ((shipPosY - 40) >> 2) + 105
		// shipScreenX = ((shipPosX - 0xa0) >> 3) + 0xa0 = ((shipPosX - 160) >> 3) + 160
		// gunY = ((shipPosY - 0x28) >> 2) + 0x82 = shipScreenY + 25
		// gunX = ((shipPosX - 0xa0) >> 3) + 0xa7 = shipScreenX + 7
		int16 shipScreenX = ((_shipPosX - 160) >> 3) + 160;
		int16 shipScreenY = ((_shipPosY - 40) >> 2) + 105;
		int16 gunX = shipScreenX + 7;
		int16 gunY = shipScreenY + 25;

		int16 progress = maxDuration - _vehicleShots[i].counter;

		// Draw beam from gun toward ship center (muzzle flash effect)
		// From FUN_402ED0: widthScale=0x14(20), heightScale=8, thickness=4
		// Parameters: gunX, gunY -> shipScreenX, shipScreenY (NOT the stored target!)
		drawLaserBeam(renderBitmap, pitch, width, height,
			gunX + _viewX, gunY + _viewY,
			shipScreenX + _viewX, shipScreenY + _viewY,
			progress, maxDuration, 20, 8, 4);

		_vehicleShots[i].counter--;
	}
}

// Handler 7 Space combat laser rendering (FUN_40FADF)
// Dual beams from left and right gun positions
void InsaneRebel2::renderSpaceLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// No NUT check needed - uses pre-initialized _laserTexture

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_spaceShots[i].counter <= 0)
			continue;

		// Calculate sound panning
		int16 pan = ((_spaceShots[i].targetX - 160) * (2 - _spaceShots[i].counter)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning

		int16 targetX = _spaceShots[i].targetX;
		int16 targetY = _spaceShots[i].targetY;
		int16 leftGunX = _spaceShots[i].leftGunX;
		int16 leftGunY = _spaceShots[i].leftGunY;
		int16 rightGunX = _spaceShots[i].rightGunX;
		int16 rightGunY = _spaceShots[i].rightGunY;
		int16 progress = maxDuration - _spaceShots[i].counter;

		// Draw dual beams
		// From FUN_40FADF: widthScale=0xC(12), heightScale=4, thickness=6
		// Left gun beam
		drawLaserBeam(renderBitmap, pitch, width, height,
			leftGunX, leftGunY, targetX, targetY,
			progress, maxDuration, 12, 4, 6);

		// Right gun beam
		drawLaserBeam(renderBitmap, pitch, width, height,
			rightGunX, rightGunY, targetX, targetY,
			progress, maxDuration, 12, 4, 6);

		_spaceShots[i].counter--;
	}
}

// Handler 25 laser rendering (FUN_0041f004)
// Speeder bike laser shots - draws beam from gun position to target
void InsaneRebel2::renderHandler25LaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// FUN_0041f004 uses turret-style shot slots with view offset adjustment
	// Only render when player is uncovered (damage == 0)
	if (_rebelDamageLevel != 0) {
		return;  // Can't shoot while taking cover
	}

	int16 maxDuration = getShotMaxDuration();

	for (int i = 0; i < 2; i++) {
		if (_turretShots[i].counter <= 0)
			continue;

		// Calculate sound panning from target X position (FUN_004262f0)
		// sVar1 = ((2 - counter) * (targetX - 160)) / 2, clamped to [-127, 127]
		int16 pan = ((2 - _turretShots[i].counter) * (_turretShots[i].targetX - 160)) / 2;
		pan = CLIP<int16>(pan, -127, 127);
		// TODO: Apply panning to sound channel i+1

		// Target position (where player clicked)
		int16 targetX = _turretShots[i].targetX;
		int16 targetY = _turretShots[i].targetY;

		// Gun position computed at spawn time from GRD002 sprite data
		// Original: DAT_0045791c[i] + DAT_0045790c, DAT_00457920[i] + DAT_0045790e
		int16 gunX = _turretShots[i].gunX;
		int16 gunY = _turretShots[i].gunY;

		int16 progress = maxDuration - _turretShots[i].counter;

		// From FUN_0041f004 parameters for FUN_0040bbf6:
		// widthScale=0xC(12), heightScale=4, thickness=6
		drawLaserBeam(renderBitmap, pitch, width, height,
			gunX, gunY, targetX, targetY,
			progress, maxDuration, 12, 4, 6);

		_turretShots[i].counter--;

		debug("Rebel2 Handler25: Laser shot %d from (%d,%d) to (%d,%d) progress=%d/%d",
			i, gunX, gunY, targetX, targetY, progress, maxDuration);
	}
}

void InsaneRebel2::renderCrosshair(byte *renderBitmap, int pitch, int width, int height) {
	// From FUN_0040d836 (Handler 7) line 167-168: crosshair only drawn when DAT_004437c0 == 2
	// Don't draw crosshair when shooting is disabled (flight-only segments)
	if (!isShootingAllowed()) {
		return;
	}

	// Handler 25 (0x19): From FUN_41DB5E lines 195-197, crosshair only drawn when
	// DAT_0045790a == 0 (fully uncovered). Hide crosshair during cover transition.
	if (_rebelHandler == 25 && _rebelDamageLevel != 0) {
		return;
	}

	// Update target lock state and draw crosshair/reticle

	// Target lock detection (DAT_00443676 equivalent)
	Common::Point worldMousePos(_vm->_mouse.x + _viewX, _vm->_mouse.y + _viewY);
	bool targetLocked = false;

	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->active && !it->destroyed && it->rect.contains(worldMousePos)) {
			targetLocked = true;
			break;
		}
	}

	if (targetLocked) {
		_targetLockTimer = 7;
	} else if (_targetLockTimer > 0) {
		_targetLockTimer--;
	}

	// Draw crosshair
	if (!_smush_iconsNut)
		return;

	int reticleIndex;
	switch (_rebelHandler) {
	case 7:    // Third-Person Ship
	case 0x19: // FPS/Mixed (Handler 25)
		reticleIndex = 47;  // 0x2F
		break;
	case 0x26: { // Turret/Cockpit - animated crosshair
		static int turretAnimCounter = 0;
		turretAnimCounter++;

		int animOffset = (_targetLockTimer == 0) ? 0 : 3 - (turretAnimCounter & 3);

		if (_rebelLevelType == 5) {
			reticleIndex = 0x30 + animOffset;
		} else {
			reticleIndex = animOffset;
		}
		break;
	}
	case 8:    // Third-Person On Foot
	default:
		reticleIndex = 46;
		break;
	}

	if (_smush_iconsNut->getNumChars() > reticleIndex) {
		int cw = _smush_iconsNut->getCharWidth(reticleIndex);
		int ch = _smush_iconsNut->getCharHeight(reticleIndex);

		// Calculate crosshair position
		int crosshairX = _vm->_mouse.x - cw / 2 + _viewX;
		int crosshairY = _vm->_mouse.y - ch / 2 + _viewY;

		// Handler 25 (0x19): Add view offset to crosshair position
		// From FUN_41DB5E lines 198-199: X = DAT_00457914 + DAT_0045790c, Y = DAT_00457916 + DAT_0045790e
		if (_rebelHandler == 25) {
			crosshairX += _rebelViewOffsetX;
			crosshairY += _rebelViewOffsetY;
		}

		renderNutSprite(renderBitmap, pitch, width, height,
			crosshairX, crosshairY,
			_smush_iconsNut, reticleIndex);
	}
}

void InsaneRebel2::frameEndCleanup() {
	// Reset enemy active flags and collision zones at frame end
	// The original game rebuilds lists from scratch each frame

	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (!it->destroyed) {
			it->active = false;
		}
	}

	resetCollisionZones();
}

// ========== Audio Handling for Rebel Assault 2 ==========
// RA2 doesn't use iMUSE - we handle audio directly through the mixer

void InsaneRebel2::initAudio(int sampleRate) {
	_audioSampleRate = sampleRate;
	for (int i = 0; i < kRA2MaxAudioTracks; i++) {
		_audioStreams[i] = nullptr;
		_audioTrackActive[i] = false;
	}
}

void InsaneRebel2::terminateAudio() {
	for (int i = 0; i < kRA2MaxAudioTracks; i++) {
		if (_audioTrackActive[i]) {
			_vm->_mixer->stopHandle(_audioHandles[i]);
			_audioTrackActive[i] = false;
		}
		if (_audioStreams[i]) {
			_audioStreams[i]->finish();
			_audioStreams[i] = nullptr;
		}
	}
}

void InsaneRebel2::queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan) {
	if (trackIdx < 0 || trackIdx >= kRA2MaxAudioTracks || size <= 0 || !data) {
		debug(5, "InsaneRebel2::queueAudioData: Invalid params trackIdx=%d size=%d data=%p", trackIdx, size, (void*)data);
		return;
	}

	debug(5, "InsaneRebel2::queueAudioData: trackIdx=%d size=%d volume=%d pan=%d", trackIdx, size, volume, pan);

	// Create audio stream if not already active
	if (!_audioStreams[trackIdx]) {
		// RA2 audio is 8-bit unsigned mono at the track's sample rate
		debug("InsaneRebel2: Creating audio stream for track %d at %d Hz", trackIdx, _audioSampleRate);
		_audioStreams[trackIdx] = Audio::makeQueuingAudioStream(_audioSampleRate, false);
		_audioTrackActive[trackIdx] = true;
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_audioHandles[trackIdx],
								_audioStreams[trackIdx], -1, Audio::Mixer::kMaxChannelVolume, 0,
								DisposeAfterUse::NO);
	}

	debug(6, "InsaneRebel2: Queueing %d bytes to track %d (vol=%d)", size, trackIdx, volume);

	// Copy the audio data since queueBuffer may need to own it
	byte *audioCopy = (byte *)malloc(size);
	if (!audioCopy) {
		return;
	}
	memcpy(audioCopy, data, size);

	// Queue the audio data - RA2 SMUSH audio is 8-bit unsigned mono
	_audioStreams[trackIdx]->queueBuffer(audioCopy, size, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	// Apply volume and pan to the channel
	int scaledVolume = (volume * Audio::Mixer::kMaxChannelVolume) / 127;
	int scaledPan = (pan * 127) / 128;  // Convert -128..127 to -127..127
	_vm->_mixer->setChannelVolume(_audioHandles[trackIdx], scaledVolume);
	_vm->_mixer->setChannelBalance(_audioHandles[trackIdx], scaledPan);
}

void InsaneRebel2::processAudioFrame(int16 feedSize) {
	if (!_player) {
		return;
	}

	// Initialize dispatch data if needed (normally done in processDispatches for iMUSE games)
	if (_player->_smushTracksNeedInit) {
		_player->_smushTracksNeedInit = false;
		for (int i = 0; i < SMUSH_MAX_TRACKS; i++) {
			_player->_smushDispatch[i].fadeRemaining = 0;
			_player->_smushDispatch[i].fadeVolume = 0;
			_player->_smushDispatch[i].fadeSampleRate = 0;
			_player->_smushDispatch[i].elapsedAudio = 0;
			_player->_smushDispatch[i].audioLength = 0;
		}
	}

	// Access SmushPlayer's audio track data (InsaneRebel2 is a friend class)
	// Only iterate over actually allocated tracks (not SMUSH_MAX_TRACKS)
	for (int i = 0; i < _player->_smushNumTracks; i++) {
		SmushPlayer::SmushAudioTrack &track = _player->_smushTracks[i];
		SmushPlayer::SmushAudioDispatch &dispatch = _player->_smushDispatch[i];

		if (track.state == TRK_STATE_INACTIVE) {
			continue;
		}

		// Skip tracks that don't have valid buffer pointers yet
		// Note: dispatch.dataBuf is set when transitioning from FADING to PLAYING,
		// so tracks in FADING state won't have it set yet - that's OK, they'll be
		// transitioned below and then processed
		if (!track.blockPtr) {
			debug(5, "InsaneRebel2: Skipping track %d - blockPtr=%p state=%d",
				  i, (void*)track.blockPtr, track.state);
			continue;
		}

		// Check if this track type should be played
		bool isPlayableTrack = ((track.flags & TRK_TYPE_MASK) == IS_SPEECH && _player->isChanActive(CHN_SPEECH)) ||
							   ((track.flags & TRK_TYPE_MASK) == IS_BKG_MUSIC && _player->isChanActive(CHN_BKGMUS)) ||
							   ((track.flags & TRK_TYPE_MASK) == IS_SFX && _player->isChanActive(CHN_OTHER));

		if (!isPlayableTrack) {
			continue;
		}

		// Calculate base volume for this track type
		int baseVolume;
		switch (track.flags & TRK_TYPE_MASK) {
		case IS_SFX:
			baseVolume = (_player->_smushTrackVols[1] * track.volume) >> 7;
			break;
		case IS_BKG_MUSIC:
			baseVolume = (_player->_smushTrackVols[3] * track.volume) >> 7;
			break;
		case IS_SPEECH:
			baseVolume = (_player->_smushTrackVols[2] * track.volume) >> 7;
			break;
		default:
			baseVolume = track.volume;
			break;
		}
		int mixVolume = baseVolume * _player->_smushTrackVols[0] / 127;

		// Handle track state transitions: FADING -> PLAYING
		if (track.state == TRK_STATE_FADING) {
			dispatch.headerPtr = track.dataBuf;
			dispatch.dataBuf = track.subChunkPtr;
			dispatch.dataSize = track.dataSize;
			dispatch.currentOffset = 0;
			dispatch.audioLength = 0;
			track.state = TRK_STATE_PLAYING;
		}

		// Process audio for this track
		if (track.state != TRK_STATE_INACTIVE) {
			int32 tmpFeedSize = feedSize;

			while (tmpFeedSize > 0) {
				int32 mixInFrameCount = dispatch.currentOffset;

				// Use dispatch.dataBuf and dispatch.dataSize which are set consistently
				// when the track transitions from FADING to PLAYING, and audioRemaining
				// is calculated relative to these values by processAudioCodes
				if (mixInFrameCount > 0 && dispatch.dataBuf && dispatch.dataSize > 0) {
					// Ensure audioRemaining is non-negative for proper circular buffer access
					if (dispatch.audioRemaining < 0) {
						debug(5, "InsaneRebel2: Resetting negative audioRemaining=%d for track %d", dispatch.audioRemaining, i);
						dispatch.audioRemaining = 0;
					}
					int32 offset = dispatch.audioRemaining % dispatch.dataSize;

					// Limit to feed size proportional to sample rate
					if (dispatch.sampleRate > 0 && _player->_smushAudioSampleRate > 0) {
						int32 maxFrames = dispatch.sampleRate * tmpFeedSize / _player->_smushAudioSampleRate;
						if (mixInFrameCount > maxFrames) {
							mixInFrameCount = maxFrames;
						}
					}

					// Don't read past the buffer
					if (offset + mixInFrameCount > dispatch.dataSize) {
						mixInFrameCount = dispatch.dataSize - offset;
					}

					// Make sure we don't exceed available data
					if (dispatch.audioRemaining + mixInFrameCount > track.availableSize) {
						mixInFrameCount = track.availableSize - dispatch.audioRemaining;
						if (mixInFrameCount <= 0) {
							// Track is ending - no more data
							track.state = TRK_STATE_ENDING;
							break;
						}
					}

					if (mixInFrameCount > 0) {
						// Safety check: verify the pointer and offset are within bounds
						if (!dispatch.dataBuf || offset < 0 || offset + mixInFrameCount > dispatch.dataSize) {
							debug(1, "InsaneRebel2: Invalid audio buffer access track=%d dataBuf=%p offset=%d mixInFrameCount=%d dataSize=%d",
								  i, (void*)dispatch.dataBuf, offset, mixInFrameCount, dispatch.dataSize);
							break;
						}

						// Queue audio data directly to our audio streams
						queueAudioData(i, &dispatch.dataBuf[offset], mixInFrameCount, mixVolume, track.pan);

						// Update dispatch state
						dispatch.currentOffset -= mixInFrameCount;
						dispatch.audioRemaining += mixInFrameCount;

						// Calculate how much feed time was consumed
						if (dispatch.sampleRate > 0) {
							int32 consumedFeed = mixInFrameCount * _player->_smushAudioSampleRate / dispatch.sampleRate;
							tmpFeedSize -= consumedFeed;
						} else {
							tmpFeedSize -= mixInFrameCount;
						}
					}
				}

				// If currentOffset is depleted, process audio codes to get more
				if (dispatch.currentOffset <= 0) {
					// processAudioCodes returns true if there's more audio, false if done
					if (!_player->processAudioCodes(i, tmpFeedSize, mixVolume)) {
						break;
					}
					// If still no offset after processing codes, we're done
					if (dispatch.currentOffset <= 0) {
						break;
					}
				} else if (tmpFeedSize <= 0) {
					break;
				}
			}
		}

		track.audioRemaining = dispatch.audioRemaining;
		dispatch.state = track.state;
	}
}

// ========== Sound Effects (SAD files) ==========
// Loads standalone SAUD files from SYSTM/ for one-shot SFX playback.
// Original game loads these via FUN_0042a3b0 at init into DAT_00456888[0..7].

static const char *const kRA2SfxFiles[InsaneRebel2::kRA2NumSfx] = {
	"SYSTM/BLAST.SAD",    // 0 - Player laser fire
	"SYSTM/CRASH.SAD",    // 1 - Corridor/wall collision
	"SYSTM/EXPLODE.SAD",  // 2 - Enemy explosion
	"SYSTM/ALERT.SAD",    // 3 - Alert/warning
	"SYSTM/LOCKON.SAD",   // 4 - Target lock-on
	"SYSTM/BONUS.SAD",    // 5 - Bonus pickup
	"SYSTM/HBLAST.SAD",   // 6 - Heavy blast (player weapon)
	"SYSTM/TBLAST.SAD"    // 7 - TIE blast
};

void InsaneRebel2::loadSfx() {
	for (int i = 0; i < kRA2NumSfx; i++) {
		ScummFile *file = _vm->instantiateScummFile();
		_vm->openFile(*file, kRA2SfxFiles[i]);
		if (!file->isOpen()) {
			debug("InsaneRebel2::loadSfx: Could not open %s", kRA2SfxFiles[i]);
			delete file;
			continue;
		}

		// SAUD file structure: SAUD header (8) + STRK sub-chunk + SDAT sub-chunk
		// We scan for the SDAT tag to find the PCM data.
		uint32 fileSize = file->size();
		if (fileSize < 38) {  // Minimum: 8 (SAUD) + 22 (STRK) + 8 (SDAT header)
			debug("InsaneRebel2::loadSfx: %s too small (%d bytes)", kRA2SfxFiles[i], fileSize);
			file->close();
			delete file;
			continue;
		}

		// Verify SAUD tag
		uint32 tag = file->readUint32BE();
		if (tag != MKTAG('S', 'A', 'U', 'D')) {
			debug("InsaneRebel2::loadSfx: %s not a SAUD file (tag=0x%08x)", kRA2SfxFiles[i], tag);
			file->close();
			delete file;
			continue;
		}
		file->readUint32BE();  // Skip SAUD size

		// Scan for SDAT chunk (skip STRK and any other sub-chunks)
		bool foundSdat = false;
		while (file->pos() + 8 <= (int64)fileSize) {
			uint32 chunkTag = file->readUint32BE();
			uint32 chunkSize = file->readUint32BE();

			if (chunkTag == MKTAG('S', 'D', 'A', 'T')) {
				// Found PCM data
				uint32 pcmSize = MIN(chunkSize, fileSize - (uint32)file->pos());
				_sfxData[i] = (byte *)malloc(pcmSize);
				if (_sfxData[i]) {
					file->read(_sfxData[i], pcmSize);
					_sfxSize[i] = pcmSize;
					debug("InsaneRebel2::loadSfx: Loaded %s (%d bytes PCM)", kRA2SfxFiles[i], pcmSize);
				}
				foundSdat = true;
				break;
			} else {
				// Skip this sub-chunk
				file->seek(chunkSize, SEEK_CUR);
			}
		}

		if (!foundSdat) {
			debug("InsaneRebel2::loadSfx: No SDAT chunk in %s", kRA2SfxFiles[i]);
		}

		file->close();
		delete file;
	}
}

void InsaneRebel2::freeSfx() {
	for (int i = 0; i < kRA2NumSfx; i++) {
		// Stop any playing SFX on this slot
		_vm->_mixer->stopHandle(_sfxHandles[i]);
		free(_sfxData[i]);
		_sfxData[i] = nullptr;
		_sfxSize[i] = 0;
	}
}

void InsaneRebel2::playSfx(int slot, int volume, int pan) {
	if (slot < 0 || slot >= kRA2NumSfx || !_sfxData[slot] || _sfxSize[slot] == 0) {
		return;
	}

	// Stop any previous instance of this SFX slot
	_vm->_mixer->stopHandle(_sfxHandles[slot]);

	// Make a copy of the PCM data (makeRawStream with DisposeAfterUse::YES will free it)
	byte *pcmCopy = (byte *)malloc(_sfxSize[slot]);
	if (!pcmCopy) {
		return;
	}
	memcpy(pcmCopy, _sfxData[slot], _sfxSize[slot]);

	// Create a one-shot raw audio stream: 8-bit unsigned mono at 11025 Hz
	Audio::SeekableAudioStream *stream = Audio::makeRawStream(
		pcmCopy, _sfxSize[slot], 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);

	// Scale volume from 0-127 to ScummVM's 0-255 range
	int scaledVolume = (volume * Audio::Mixer::kMaxChannelVolume) / 127;

	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandles[slot],
		stream, -1, scaledVolume, pan);

	debug(5, "InsaneRebel2::playSfx: slot=%d vol=%d pan=%d size=%d", slot, volume, pan, _sfxSize[slot]);
}

// ======================= Menu System Implementation =======================
// Emulates retail menu system from FUN_004147B2 and FUN_0041FDC8

void InsaneRebel2::resetMenu() {
	_menuSelection = 0;
	_menuInactivityTimer = 0;
	_menuRepeatDelay = 0;
}

// Unlock all chapters for testing
// Emulates the debug mode from original FUN_00415CF8 (lines 60-71)
// where DAT_0047ab34 == 'd' enables level unlock via special codes
void InsaneRebel2::unlockAllChapters() {
	debug("Rebel2: Unlocking all chapters for testing");
	_debugUnlockAll = true;
	for (int i = 0; i < 16; i++) {
		_chapterUnlocked[i] = true;
		_levelUnlocked[i] = true;
	}
}

Common::String InsaneRebel2::getRandomMenuVideo() {
	// Emulates FUN_0041FDC8 - selects random menu video variant
	//
	// NOTE: The original game plays O_MENU.SAN when no progress flags are set,
	// but that file contains ONLY audio (no FOBJ video frames). The O_MENU_X.SAN
	// variants (A through O) contain actual 320x200 background images in Frame 0.
	//
	// We ALWAYS use a random variant to ensure a proper background is displayed.
	// The original behavior of showing O_MENU.SAN (audio-only) would result in
	// a black/undefined background which doesn't match the intended experience.

	// Select random variant (0-14 maps to A-O), ensuring different from last
	int variant;
	do {
		variant = _vm->_rnd.getRandomNumber(14);  // 0-14
	} while (variant == _lastMenuVariant && _lastMenuVariant >= 0);
	_lastMenuVariant = variant;

	// Map 0-14 to A-O (case 0/default = A, 1 = B, etc.)
	char letter = 'A' + variant;
	debug("Rebel2: Selected menu variant %c", letter);
	return Common::String::format("OPEN/O_MENU_%c.SAN", letter);
}

int InsaneRebel2::processMenuInput() {
	// Emulates FUN_0041f5ae menu input handling
	// Returns: -1 = no action, 0-4 = menu item selected
	//
	// Events are captured by notifyEvent() (EventObserver) which runs before
	// ScummEngine::parseEvents() consumes them. This ensures we don't miss
	// any input events even though we only process them on video frames.
	//
	// From FUN_0041f5ae disassembly:
	// - Keyboard: Up/Down arrows navigate, Enter confirms
	// - Mouse mode (DAT_0047a806 == 1): Y position maps to selection
	// - Key codes: Up=0x148, Down=0x150, Enter=0x0d, ESC=0x1b

	int result = -1;

	// Menu item Y positions (low-res 320x200 mode):
	// From FUN_0041f5ae: baseY = numItems * -5 + 0x68
	// With 8 total items (title + 7 options): 8 * -5 + 104 = 64
	// Items at Y = 64, 74, 84, 94, 104, 114, 124 with spacing of 10
	const int numItemsTotal = 8;  // Title + 7 selectable items (matching assembly)
	const int baseY = numItemsTotal * -5 + 0x68;  // = 64
	const int itemSpacing = 10;

	// Process events from the queue (populated by notifyEvent)
	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.pop();
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_menuInactivityTimer = 0;  // Reset inactivity timer on any input

			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				// Navigate up (wrap around) - emulates key code 0x148
				_menuSelection--;
				if (_menuSelection < 0) {
					_menuSelection = _menuItemCount - 1;
				}
				// Reset repeat delay counter (DAT_00459ce0)
				_menuRepeatDelay = 3;
				debug("Menu: Selection changed to %d (UP)", _menuSelection);
				break;

			case Common::KEYCODE_DOWN:
				// Navigate down (wrap around) - emulates key code 0x150
				_menuSelection++;
				if (_menuSelection >= _menuItemCount) {
					_menuSelection = 0;
				}
				_menuRepeatDelay = 3;
				debug("Menu: Selection changed to %d (DOWN)", _menuSelection);
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				// Confirm selection - emulates key code 0x0d
				if (_menuSelection >= 0 && _menuSelection < _menuItemCount) {
					result = _menuSelection;
					debug("Menu: Item %d selected (ENTER)", _menuSelection);
				}
				break;

			case Common::KEYCODE_ESCAPE:
				// ESC - Quit (index 4 = last item) - emulates key code 0x1b
				result = _menuItemCount - 1;  // Select quit option
				debug("Menu: ESC pressed - selecting quit (item %d)", result);
				break;

			default:
				break;
			}
			break;

		case Common::EVENT_LBUTTONDOWN:
			_menuInactivityTimer = 0;
			{
				// Get mouse position from the event
				int mouseY = event.mouse.y;

				debug("Menu: Left click at Y=%d", mouseY);

				// Check which item was clicked
				// From FUN_0041f5ae mouse mode: selection = (mouseY + 100 - baseY) / 10
				// But we use a simpler direct hit-test approach
				for (int i = 0; i < _menuItemCount; i++) {
					int itemY = baseY + i * itemSpacing;
					// Hit area: itemY - 2 to itemY + 8 (10 pixel height)
					if (mouseY >= itemY - 2 && mouseY < itemY + 8) {
						_menuSelection = i;
						result = i;
						debug("Menu: Item %d clicked (itemY=%d)", i, itemY);
						break;
					}
				}
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			// Update hover selection based on Y position
			// This emulates FUN_0041f5ae mouse mode behavior (DAT_0047a806 == 1)
			{
				int mouseY = event.mouse.y;
				// Calculate selection from mouse Y position
				// From assembly: DAT_00459988 = ((mouseY + 100) - (param_3 * -5 + 0x67)) / 10
				int newSelection = (mouseY + 100 - (numItemsTotal * -5 + 0x67)) / 10;

				// Clamp to valid range
				if (newSelection < 0) newSelection = 0;
				if (newSelection >= _menuItemCount) newSelection = _menuItemCount - 1;

				// Only update if within menu area (not too far above/below)
				int topY = baseY - 5;
				int bottomY = baseY + (_menuItemCount - 1) * itemSpacing + 10;
				if (mouseY >= topY && mouseY <= bottomY) {
					if (newSelection != _menuSelection) {
						_menuSelection = newSelection;
						debug(5, "Menu: Hover selection changed to %d (mouseY=%d)", _menuSelection, mouseY);
					}
				}
			}
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			// Handle quit request - select quit option
			result = _menuItemCount - 1;
			break;

		default:
			break;
		}
	}

	// Decrement repeat delay counter (for smooth keyboard navigation)
	if (_menuRepeatDelay > 0) {
		_menuRepeatDelay--;
	}

	return result;
}

void InsaneRebel2::drawMenuItems(byte *renderBitmap, int pitch, int width, int height,
                                  const char **items, int numItems, int selection,
                                  bool leftAligned) {
	// =====================================================================
	// Shared menu renderer - Emulates FUN_0041f5ae
	// Address: 0x41F5AE
	// =====================================================================
	//
	// items[0] = title string, items[1..numItems] = selectable items
	// numItems = number of selectable items (FUN_0041f5ae param_3)
	// selection = currently highlighted item (0-based, maps to DAT_00459988)
	// leftAligned = false: param_4==0 (centered), true: param_4==1 (left-aligned)
	//
	// Coordinate formulas from FUN_0041f5ae (low-res, DAT_0047a808 < 2):
	// Centered (param_4=0):
	//   Title X:     center - titleWidth/2  (centerX = 160)
	//   Title Y:     param_3 * -5 + 0x51
	//   Item X:      center - textWidth/2
	//   Box X:       center - bracketWidth/2
	// Left-aligned (param_4=1):
	//   Title X:     0x28 = 40
	//   Title Y:     param_3 * -5 + 0x56
	//   Item X:      0x17 = 23
	//   Box X:       0x14 = 20
	// Both modes:
	//   Item base Y: param_3 * -5 + 0x68
	//   Item Y:      param_3 * -5 + i * 10 + 0x68
	//   Box Y:       param_3 * -5 + i * 10 + 0x67  (1px above text)

	const int centerX = width / 2;
	const int titleY = numItems * -5 + (leftAligned ? 0x56 : 0x51);
	const int itemBaseY = numItems * -5 + 0x68;
	const int itemSpacing = 10;

	// =====================================================================
	// Font system - Emulates linked list from FUN_00403bd0
	// =====================================================================
	//   Font 0 (^f00): TALKFONT.NUT
	//   Font 1 (^f01): SMALFONT.NUT (menu items)
	//   Font 2 (^f02): TITLFONT.NUT (title)
	NutRenderer *fonts[3] = {
		_smush_talkfontNut,
		_smush_smalfontNut,
		_smush_titlefontNut
	};

	NutRenderer *defaultFont = fonts[0] ? fonts[0] : _smush_smalfontNut;
	if (!defaultFont) {
		debug(1, "drawMenuItems: no fonts available!");
		return;
	}

	Common::Rect clipRect(0, 0, _vm->_screenWidth, _vm->_screenHeight);
	int actualPitch = _vm->_screenWidth;

	// =====================================================================
	// Format code parser - Emulates FUN_00434d10 / FUN_00433da0
	// =====================================================================
	//   ^^ = literal ^, ^fNN = font switch, ^cNNN = color code, ^l = newline
	auto parseFormatCode = [&](const char *&str, int &outColor) -> int {
		if (*str != '^') return -1;

		const char *p = str + 1;
		if (*p == '^') {
			str = p;
			return -1;
		}
		if (*p == 'f') {
			p++;
			int fontIdx = 0;
			while (*p >= '0' && *p <= '9') {
				fontIdx = fontIdx * 10 + (*p - '0');
				p++;
			}
			str = p;
			return (fontIdx >= 0 && fontIdx < 3) ? fontIdx : 0;
		}
		if (*p == 'c') {
			p++;
			int color = 0;
			while (*p >= '0' && *p <= '9') {
				color = color * 10 + (*p - '0');
				p++;
			}
			str = p;
			outColor = color;
			return -2;
		}
		if (*p == 'l') {
			str = p + 1;
			return -2;
		}
		return -1;
	};

	// String width calculation - Emulates FUN_00433da0
	auto getStringWidth = [&](const char *str) -> int {
		int w = 0;
		NutRenderer *curFont = defaultFont;
		int curColor = -1;

		while (*str) {
			int fontChange = parseFormatCode(str, curColor);
			if (fontChange >= 0) {
				curFont = fonts[fontChange] ? fonts[fontChange] : defaultFont;
				continue;
			}
			if (fontChange == -2) continue;

			byte c = (byte)*str++;
			if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
			if (curFont && c < curFont->getNumChars()) {
				w += curFont->getCharWidth(c);
			}
		}
		return w;
	};

	// String rendering - Emulates FUN_00434d10
	// Codec 44 color substitution: font pixels with value 1 → ^cNNN color
	auto drawString = [&](const char *str, int x, int y) {
		NutRenderer *curFont = defaultFont;
		int curColor = 1;

		while (*str) {
			int fontChange = parseFormatCode(str, curColor);
			if (fontChange >= 0) {
				curFont = fonts[fontChange] ? fonts[fontChange] : defaultFont;
				continue;
			}
			if (fontChange == -2) continue;

			byte c = (byte)*str++;
			if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';

			if (!curFont) continue;
			int numChars = curFont->getNumChars();
			if (c >= numChars) continue;

			int charW = curFont->getCharWidth(c);

			if (x >= 0 && y >= 0 && charW > 0) {
				curFont->drawCharV7(renderBitmap, clipRect, x, y, actualPitch, curColor,
				                    kStyleAlignLeft, c, false, false);
			}
			x += charW;
		}
	};

	// =====================================================================
	// Draw title - items[0]
	// Centered: X = center - titleWidth/2
	// Left-aligned: X = 40 (0x28)
	// =====================================================================
	{
		int titleWidth = getStringWidth(items[0]);
		int titleX = leftAligned ? 40 : (centerX - titleWidth / 2);
		drawString(items[0], titleX, titleY);
	}

	// =====================================================================
	// Draw selectable items with selection highlight box
	// Centered: item X = center - textWidth/2, box X = center - bracketWidth/2
	// Left-aligned: item X = 23 (0x17), box X = 20 (0x14)
	// =====================================================================
	for (int i = 0; i < numItems; i++) {
		int itemY = itemBaseY + i * itemSpacing;
		const char *text = items[i + 1];

		int textWidth = getStringWidth(text);
		int textX = leftAligned ? 23 : (centerX - textWidth / 2);
		drawString(text, textX, itemY);

		// Selection highlight box - FUN_004292d0
		if (i == selection) {
			// Width: textWidth + ((DAT_0047a808 < 2) - 1 & 6) + 6 = textWidth + 6
			int bracketWidth = textWidth + 6;
			// Height: ((DAT_0047a808 < 2) - 1 & 10) + 10 = 10
			int bracketHeight = 10;

			// Flash color: (-((DAT_0047a7e4 & 1) == 0) & 8U) - 0x10
			// bit0==0: 8-16=248(0xF8), bit0==1: 0-16=240(0xF0)
			static int frameCounter = 0;
			frameCounter++;
			byte highlightColor = ((frameCounter / 8) & 1) ? 248 : 240;

			// Box position: Y = itemY - 1 (0x67 vs 0x68)
			int leftX = leftAligned ? 20 : (centerX - bracketWidth / 2);
			int rightX = leftX + bracketWidth;
			int topY = itemY - 1;
			int bottomY = itemY + bracketHeight - 1;

			int screenW = _vm->_screenWidth;
			int screenH = _vm->_screenHeight;
			if (leftX < 0) leftX = 0;
			if (rightX >= screenW) rightX = screenW - 1;
			if (topY < 0) topY = 0;
			if (bottomY >= screenH) bottomY = screenH - 1;

			// FUN_004292d0 - Draw rectangle border (4 lines)
			for (int x = leftX; x <= rightX && x < screenW; x++) {
				if (topY >= 0 && topY < screenH)
					renderBitmap[topY * actualPitch + x] = highlightColor;
				if (bottomY >= 0 && bottomY < screenH)
					renderBitmap[bottomY * actualPitch + x] = highlightColor;
			}
			for (int py = topY; py <= bottomY && py < screenH; py++) {
				if (leftX >= 0 && leftX < screenW)
					renderBitmap[py * actualPitch + leftX] = highlightColor;
				if (rightX >= 0 && rightX < screenW)
					renderBitmap[py * actualPitch + rightX] = highlightColor;
			}
		}
	}
}

void InsaneRebel2::drawMenuOverlay(byte *renderBitmap, int pitch, int width, int height) {
	// =====================================================================
	// Main menu renderer - calls shared drawMenuItems()
	// Emulates FUN_004147b2 -> FUN_0041f5ae with param_3=7, param_4=0
	// =====================================================================
	//
	// Menu strings loaded from GAME.TRS (keyboard mode indices 10-17):
	//   TRS index 10: "^f02Game Main Menu"           -> Title (uses TITLFONT)
	//   TRS index 11: "^f01^c005Start Game"          -> Item 0 (uses SMALFONT, color 5)
	//   TRS index 12: "^f01^c009Options"             -> Item 1
	//   TRS index 13: "^f01^c009Calibrate Joystick"  -> Item 2
	//   TRS index 14: "^f01^c009Continue Intro"      -> Item 3
	//   TRS index 15: "^f01^c009Show Top Pilots"     -> Item 4
	//   TRS index 16: "^f01^c009Show Credits"        -> Item 5
	//   TRS index 17: "^f01^c240Return to Launcher"  -> Item 6 (color 240)

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer) {
		debug(1, "drawMenuOverlay: SmushPlayer not available for TRS strings!");
		return;
	}

	// Load TRS strings 10-17 (title + 7 selectable items)
	const char *menuItems[8];
	for (int i = 0; i < 8; i++) {
		menuItems[i] = splayer->getString(10 + i);
		if (!menuItems[i] || !menuItems[i][0]) {
			debug(1, "drawMenuOverlay: TRS string %d not found!", 10 + i);
			menuItems[i] = "";
		}
	}

	// FUN_004147b2 line 25: param_3 = (DAT_0047a806 == 0) + 6 = 7 (keyboard mode)
	drawMenuItems(renderBitmap, pitch, width, height, menuItems, 7, _menuSelection);
}

// ======================= Pause Overlay =======================
// Emulates FUN_405A21 pause rendering (lines 242-305)
// Creates a dimmed overlay effect and displays "PAUSED" text
void InsaneRebel2::showPauseOverlay() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer) {
		debug("showPauseOverlay: No SmushPlayer active");
		return;
	}

	// Get frame buffer and palette from SmushPlayer
	// _dst points to the virtual screen pixels (the actual rendering destination)
	// _frameBuffer is only used for store/fetch operations, not general rendering
	byte *frameBuffer = splayer->_dst;
	byte *palette = splayer->_pal;
	int width = splayer->_width;
	int height = splayer->_height;

	if (!frameBuffer || !palette || width <= 0 || height <= 0) {
		debug("showPauseOverlay: No frame buffer (%p), palette (%p), or invalid dimensions (%dx%d)",
		      (void*)frameBuffer, (void*)palette, width, height);
		return;
	}

	debug("showPauseOverlay: Applying dimming effect to %dx%d buffer", width, height);

	// Apply dimming effect (emulates FUN_405A21 lines 242-251)
	// Original algorithm:
	//   For each pixel, take the green component of its palette entry
	//   and the green component of the previous pixel's palette entry,
	//   add them, divide by 8, add 16.
	// This creates a dark dimmed effect.
	int bufferSize = width * height;
	byte prevPixel = 0;

	for (int i = 0; i < bufferSize; i++) {
		byte curPixel = frameBuffer[i];

		// Get green components from palette (offset +1 in RGB triplets)
		int greenCur = palette[curPixel * 3 + 1];
		int greenPrev = palette[prevPixel * 3 + 1];

		// Apply dimming formula: (green1 + green2) >> 3 + 0x10
		byte dimmedValue = ((greenCur + greenPrev) >> 3) + 0x10;

		frameBuffer[i] = dimmedValue;
		prevPixel = curPixel;
	}

	// Draw border decorations (simplified version of FUN_405A21 lines 261-283)
	// Draw horizontal lines at top and bottom of a centered box
	int boxLeft = 12;
	int boxRight = width - 12;
	int boxTop = 23;   // 0x17
	int boxBottom = height - 23;  // ~175 for 200 height

	byte borderColor = 0x50;  // Gray border color

	// Top and bottom borders
	for (int x = boxLeft; x < boxRight; x++) {
		if (boxTop >= 0 && boxTop < height)
			frameBuffer[boxTop * width + x] = borderColor;
		if (boxBottom >= 0 && boxBottom < height)
			frameBuffer[boxBottom * width + x] = borderColor;
	}

	// Left and right borders
	for (int y = boxTop; y < boxBottom; y++) {
		if (boxLeft >= 0 && boxLeft < width)
			frameBuffer[y * width + boxLeft] = borderColor;
		if (boxRight >= 0 && boxRight < width)
			frameBuffer[y * width + boxRight] = borderColor;
	}

	// Draw corner decorations (simplified)
	byte cornerColor = 0x51;  // Slightly brighter for corners
	for (int i = 0; i < 5; i++) {
		// Top-left corner
		if (boxTop + i < height && boxLeft + 5 < width)
			frameBuffer[(boxTop + i) * width + boxLeft + 5] = cornerColor;
		if (boxTop + 5 < height && boxLeft + i < width)
			frameBuffer[(boxTop + 5) * width + boxLeft + i] = cornerColor;

		// Top-right corner
		if (boxTop + i < height && boxRight - 5 >= 0)
			frameBuffer[(boxTop + i) * width + boxRight - 5] = cornerColor;
		if (boxTop + 5 < height && boxRight - i >= 0)
			frameBuffer[(boxTop + 5) * width + boxRight - i] = cornerColor;

		// Bottom-left corner
		if (boxBottom - i >= 0 && boxLeft + 5 < width)
			frameBuffer[(boxBottom - i) * width + boxLeft + 5] = cornerColor;
		if (boxBottom - 5 >= 0 && boxLeft + i < width)
			frameBuffer[(boxBottom - 5) * width + boxLeft + i] = cornerColor;

		// Bottom-right corner
		if (boxBottom - i >= 0 && boxRight - 5 >= 0)
			frameBuffer[(boxBottom - i) * width + boxRight - 5] = cornerColor;
		if (boxBottom - 5 >= 0 && boxRight - i >= 0)
			frameBuffer[(boxBottom - 5) * width + boxRight - i] = cornerColor;
	}

	// Draw "PAUSED" text centered
	// Try to load from TRS - the exact index may vary by language version
	// TRS index 80 (0x50) is likely "PAUSED" or equivalent (from DAT_004573f8)
	// Note: splayer is already defined at the start of this function
	const char *pauseText = splayer ? splayer->getString(80) : nullptr;
	if (!pauseText || !pauseText[0]) {
		// Fallback only if TRS string not available
		pauseText = "PAUSED";
	}

	// Draw text using SmushFont if available
	if (_menuFont) {
		Common::Rect clipRect(0, 0, width, height);

		// Calculate centered position
		// Text should be centered horizontally and vertically in the box
		int textX = width / 2;  // SmushFont handles centering with kStyleAlignCenter
		int textY = height / 2 - 4;  // Slightly above center

		// Draw with color 4 and background 0x10 (matching original parameters)
		// FUN_00434cb0 params: x=10, y=10 or 20, color=4, bg=0x10
		_menuFont->drawString(pauseText, frameBuffer, clipRect, textX, textY, 0x10, kStyleAlignCenter);
	} else if (_smush_smalfontNut) {
		// Fallback: draw using NutRenderer directly
		NutRenderer *font = _smush_smalfontNut;
		int numFontChars = font->getNumChars();
		Common::Rect clipRect(0, 0, width, height);

		// Calculate text width
		int textWidth = 0;
		const char *p = pauseText;
		while (*p) {
			byte c = (byte)*p++;
			if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
			if (c < numFontChars) {
				textWidth += font->getCharWidth(c);
			}
		}

		// Draw centered
		int textX = (width - textWidth) / 2;
		int textY = height / 2 - 4;

		p = pauseText;
		while (*p) {
			byte c = (byte)*p++;
			if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
			if (c < numFontChars && textX >= 0 && textY >= 0) {
				font->drawCharV7(frameBuffer, clipRect, textX, textY, width, -1,
				                 kStyleAlignLeft, c, true, true);
				textX += font->getCharWidth(c);
			}
		}
	}

	// Update the screen to show the pause overlay
	// SmushPlayer uses copyRectToScreen to transfer the buffer to the display backend
	_vm->_system->copyRectToScreen(frameBuffer, width, 0, 0, width, height);
	_vm->_system->updateScreen();

	debug("showPauseOverlay: Overlay displayed");
}

int InsaneRebel2::runMainMenu() {
	// Main menu loop - emulates FUN_004147B2
	// Returns:
	//   kMenuNewGame (2) = Start new game
	//   kMenuContinue (4) = Continue game (level select)
	//   kMenuCredits (1) = Show credits then return to menu
	//   0 = Quit game

	debug("Rebel2: Entering main menu");

	resetMenu();
	_gameState = kStateMainMenu;

	// Enable menu input capture via EventObserver
	_menuInputActive = true;
	while (!_menuEventQueue.empty()) _menuEventQueue.pop();  // Clear any stale events

	// Get the SmushPlayer from ScummEngine_v7
	// Note: _player isn't set until SmushPlayer::initAudio() is called during playback
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Main menu loop
	while (!_vm->shouldQuit()) {
		// Reset video finish flag before playing menu
		_vm->_smushVideoShouldFinish = false;

		// Select and play a random menu video
		Common::String menuVideo = getRandomMenuVideo();
		debug("Rebel2: Playing menu video: %s", menuVideo.c_str());

		// Set video flags for menu (0x20 = intro/menu flag)
		// This tells procPostRendering we're in menu mode
		splayer->setCurVideoFlags(0x20);

		// Play the menu video
		// Input is processed in procPostRendering during playback
		// When user confirms selection, _vm->_smushVideoShouldFinish is set
		splayer->play(menuVideo.c_str(), 12);

		// Check for quit
		if (_vm->shouldQuit()) {
			_menuInputActive = false;
			return 0;
		}

		// If video ended naturally (not by selection), loop back
		if (!_vm->_smushVideoShouldFinish) {
			// Video ended without selection (reached end or ESC during video)
			// Continue looping menu videos
			continue;
		}

		// Clear the flag
		_vm->_smushVideoShouldFinish = false;

		// A selection was made - process it
		debug("Rebel2: Menu video ended with selection=%d", _menuSelection);

		// Process the menu result based on current selection
		// Menu items matching GAME.TRS indices 11-17 (FUN_004147B2):
		//   case 0 (TRS 11): Start Game -> pilot selection, returns 2
		//   case 1 (TRS 12): Options -> FUN_00416787 options screen
		//   case 2 (TRS 13): Calibrate Joystick -> FUN_00425820
		//   case 3 (TRS 14): Continue Intro -> replay O_OPEN videos
		//   case 4 (TRS 15): Show Top Pilots -> FUN_00420116(-1)
		//   case 5 (TRS 16): Show Credits -> play O_CREDIT.SAN, returns 1
		//   case 6 (TRS 17): Return to Launcher -> quit, returns 0
		switch (_menuSelection) {
		case 0:  // Start Game -> go to pilot selection
			debug("Rebel2: Start Game selected - going to pilot selection");
			_gameState = kStatePilotSelect;
			_menuInputActive = false;
			return kMenuNewGame;  // Return 2 (kMenuNewGame)

		case 1:  // Options -> show options menu
			debug("Rebel2: Options selected");
			// TODO: Implement options menu (FUN_004167a6)
			// Options: Music, Sound, Voices, Auto Control, Indicators,
			// Arrows, Difficulty (0-5), Music Volume, SFX Volume
			break;

		case 2:  // Calibrate Joystick
			debug("Rebel2: Calibrate Joystick selected");
			// TODO: Implement joystick calibration (FUN_00425820)
			// Plays O_CALIB.SAN with joystick calibration prompts
			break;

		case 3:  // Continue Intro -> replay intro videos
			debug("Rebel2: Continue Intro selected - replaying intro");
			// Temporarily switch to intro state to disable menu overlay
			// This emulates FUN_004142BD case 0 behavior
			_gameState = kStateIntro;
			_menuInputActive = false;
			// Play intro sequence again (O_OPEN_A/B)
			splayer->setCurVideoFlags(0x20);
			splayer->play("OPEN/O_OPEN_A.SAN", 12);
			if (!_vm->shouldQuit()) {
				splayer->play("OPEN/O_OPEN_B.SAN", 12);
			}
			// Restore menu state
			_gameState = kStateMainMenu;
			_menuInputActive = true;
			break;

		case 4:  // Show Top Pilots -> high score display
			debug("Rebel2: Show Top Pilots selected");
			// TODO: Implement high score display (FUN_00420116(-1))
			break;

		case 5:  // Show Credits -> play credits video
			debug("Rebel2: Show Credits selected - playing O_CREDIT.SAN");
			_gameState = kStateCredits;
			_menuInputActive = false;
			splayer->setCurVideoFlags(0x20);
			splayer->play("OPEN/O_CREDIT.SAN", 12);
			_gameState = kStateMainMenu;
			_menuInputActive = true;
			// Returns 1 in original -> stays at stage 1 (main menu)
			break;

		case 6:  // Return to Launcher -> quit game
			debug("Rebel2: Return to Launcher selected");
			_menuInputActive = false;
			return 0;  // Return 0 to exit

		default:
			debug("Rebel2: Unknown menu selection %d", _menuSelection);
			break;
		}
	}

	_menuInputActive = false;
	return 0;
}

// ==================== Chapter Selection Screen ====================
// Emulates FUN_00415CF8 - Chapter selection with preview and password input
// This is the actual level/chapter selection that players see after pilot select

int InsaneRebel2::runChapterSelect() {
	// Chapter selection screen loop - emulates FUN_00415CF8
	// Returns:
	//   kChapterSelectPlay (5) = Play selected chapter
	//   kChapterSelectBack (2) = Return to main menu (ESC or BACK)
	//   kChapterSelectQuit (0) = Quit game

	debug("Rebel2: Entering chapter selection (FUN_00415CF8)");

	// Enable menu input capture
	_menuInputActive = true;
	while (!_menuEventQueue.empty()) _menuEventQueue.pop();

	// Initialize chapter selection state
	// Original (lines 51-54): local_10 = 0xf; while (local_10 > 0 && locked) local_10--;
	// Finds highest unlocked chapter. With debug unlock all = 15 (FINALE).
	_chapterSelection = 15;
	while (_chapterSelection > 0 && !_chapterUnlocked[_chapterSelection]) {
		_chapterSelection--;
	}
	_chapterItemCount = 17;  // 16 chapters + RETURN TO PILOTS
	_selectedChapter = 0;
	_passwordInput = "";
	_menuRepeatDelay = 0;
	_gameState = kStateChapterSelect;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Initialize preview offset for initial selection
	_previewOffsetX = -90;
	_previewOffsetY = _chapterSelection * -50 + 75;

	// Set iactBits for chapter unlock state (FUN_00415CF8 lines 79-86)
	// Bits 16..1 correspond to chapters 0..15: set if unlocked, clear if locked.
	// These control SKIP chunks in O_LEVEL.SAN for locked/unlocked preview variants.
	for (int i = 0; i < 16; i++) {
		if (_chapterUnlocked[i])
			setBit(16 - i);
		else
			clearBit(16 - i);
	}

	// Chapter selection background - FUN_00415CF8 line 57:
	// FUN_0041f4d0(s_OPEN_O_LEVEL_SAN, 8, 0xffff, 0xffff, 0)
	// O_LEVEL.SAN contains chapter preview thumbnails at specific FOBJ positions.
	// The FOBJ offset system scrolls the correct preview into the preview box area.
	while (!_vm->shouldQuit()) {
		_vm->_smushVideoShouldFinish = false;

		debug("Rebel2: Playing chapter select background: OPEN/O_LEVEL.SAN");

		// Flags: 0x20 (overlay drawing). No 0x08 (preserve) — we want a black
		// background. O_LEVEL.SAN has no full-screen background FOBJ; the visible
		// screen area stays black, and preview thumbnails render at X=230 via offset.
		splayer->setCurVideoFlags(0x20);

		// Play O_LEVEL.SAN — preview thumbnails are rendered by FOBJ offset
		splayer->play("OPEN/O_LEVEL.SAN", 12);

		if (_vm->shouldQuit()) {
			_menuInputActive = false;
			return kChapterSelectQuit;
		}

		// If video ended without selection, continue looping
		if (!_vm->_smushVideoShouldFinish) {
			continue;
		}

		_vm->_smushVideoShouldFinish = false;

		debug("Rebel2: Chapter selection made: %d", _chapterSelection);

		// Process chapter selection (lines 134-236 of FUN_00415CF8)
		if (_chapterSelection == 16) {
			// BACK selected (index 16 = 17th item)
			debug("Rebel2: BACK to main menu selected");
			_menuInputActive = false;
			return kChapterSelectBack;
		}

		if (_chapterSelection >= 0 && _chapterSelection < 16) {
			// Chapter selected
			if (_chapterUnlocked[_chapterSelection]) {
				// Chapter is unlocked - start it
				_selectedChapter = _chapterSelection;
				debug("Rebel2: Chapter %d selected (unlocked)", _selectedChapter + 1);
				_menuInputActive = false;
				return kChapterSelectPlay;
			} else {
				// Chapter is locked - check password (lines 239-257 of FUN_00415CF8)
				// For now, just play error sound and continue
				debug("Rebel2: Chapter %d is locked", _chapterSelection + 1);
				continue;
			}
		}
	}

	_menuInputActive = false;
	return kChapterSelectQuit;
}

int InsaneRebel2::processChapterSelectInput() {
	// Process input for chapter selection screen
	// Emulates input handling in FUN_00415CF8 (lines 95-133)
	// Returns: -1 = no action, 0+ = item selected

	int result = -1;

	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.front();
		_menuEventQueue.pop();

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				// Move selection up, wrap to bottom
				_chapterSelection--;
				if (_chapterSelection < 0) {
					_chapterSelection = _chapterItemCount - 1;
				}
				// Update preview offset (FUN_00425170: Y = selected * -50 + 75)
				_previewOffsetY = _chapterSelection * -50 + 75;
				debug("ChapterSelect: Selection changed to %d (UP) offsetY=%d", _chapterSelection, _previewOffsetY);
				break;

			case Common::KEYCODE_DOWN:
				// Move selection down, wrap to top
				_chapterSelection++;
				if (_chapterSelection >= _chapterItemCount) {
					_chapterSelection = 0;
				}
				// Update preview offset (FUN_00425170: Y = selected * -50 + 75)
				_previewOffsetY = _chapterSelection * -50 + 75;
				debug("ChapterSelect: Selection changed to %d (DOWN) offsetY=%d", _chapterSelection, _previewOffsetY);
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				if (_chapterSelection >= 0 && _chapterSelection < _chapterItemCount) {
					result = _chapterSelection;
					debug("ChapterSelect: Item %d selected (ENTER)", _chapterSelection);
				}
				break;

			case Common::KEYCODE_ESCAPE:
				// ESC = Back to main menu (same as selecting BACK)
				result = 16;  // BACK index
				debug("ChapterSelect: ESC pressed - back to menu");
				break;

			case Common::KEYCODE_BACKSPACE:
				// Backspace for password input (line 107-112 of FUN_00415CF8)
				if (!_passwordInput.empty()) {
					_passwordInput.deleteLastChar();
					debug("ChapterSelect: Password backspace, now: %s", _passwordInput.c_str());
				}
				break;

			default:
				// Printable character for password input (lines 114-121 of FUN_00415CF8)
				if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7E) {
					if (_passwordInput.size() < 8) {
						_passwordInput += (char)event.kbd.ascii;
						debug("ChapterSelect: Password input: %s", _passwordInput.c_str());
					}
				}
				break;
			}
			break;

		case Common::EVENT_LBUTTONDOWN:
			// Click confirms the current selection (original: DAT_0047a7e4 & 1)
			if (_chapterSelection >= 0 && _chapterSelection < _chapterItemCount) {
				result = _chapterSelection;
				debug("ChapterSelect: Item %d confirmed (CLICK)", _chapterSelection);
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			{
				// Mouse hover changes highlight (original FUN_0041f5ae mouse mode).
				// Item Y = numItems * -5 + i * 10 + 0x68
				int baseY = _chapterItemCount * -5 + 0x68;
				int mouseY = event.mouse.y;

				for (int i = 0; i < _chapterItemCount; i++) {
					int itemY = baseY + i * 10;
					if (mouseY >= itemY - 4 && mouseY < itemY + 6) {
						if (i != _chapterSelection) {
							_chapterSelection = i;
							_previewOffsetY = _chapterSelection * -50 + 75;
							debug(5, "ChapterSelect: Hover changed to %d", _chapterSelection);
						}
						break;
					}
				}
			}
			break;

		default:
			break;
		}
	}

	return result;
}

// Draw preview box border - emulates FUN_004292D0 calls at lines 128-133 of FUN_00415CF8
void InsaneRebel2::drawPreviewBox(byte *renderBitmap, int pitch, int width, int height) {
	// Low-res (320x200) coordinates from FUN_00415CF8:
	// Outer box: X=0xe4 (228), Y=0x49 (73), W=0x54 (84), H=0x36 (54), color=0xF8
	// Inner box: X=0xe5 (229), Y=0x4a (74), W=0x52 (82), H=0x34 (52), color=4

	// Outer border (bright)
	int outerX = 228, outerY = 73, outerW = 84, outerH = 54;
	byte outerColor = 0xF8;

	// Draw outer box edges
	// Top edge
	for (int px = outerX; px < outerX + outerW && px < width; px++) {
		if (outerY >= 0 && outerY < height && px >= 0)
			renderBitmap[outerY * pitch + px] = outerColor;
	}
	// Bottom edge
	int bottomY = outerY + outerH - 1;
	if (bottomY < height) {
		for (int px = outerX; px < outerX + outerW && px < width; px++) {
			if (px >= 0)
				renderBitmap[bottomY * pitch + px] = outerColor;
		}
	}
	// Left edge
	for (int py = outerY; py < outerY + outerH && py < height; py++) {
		if (py >= 0 && outerX >= 0 && outerX < width)
			renderBitmap[py * pitch + outerX] = outerColor;
	}
	// Right edge
	int rightX = outerX + outerW - 1;
	if (rightX < width) {
		for (int py = outerY; py < outerY + outerH && py < height; py++) {
			if (py >= 0)
				renderBitmap[py * pitch + rightX] = outerColor;
		}
	}

	// Inner border (dark)
	int innerX = 229, innerY = 74, innerW = 82, innerH = 52;
	byte innerColor = 4;

	// Top edge
	for (int px = innerX; px < innerX + innerW && px < width; px++) {
		if (innerY >= 0 && innerY < height && px >= 0)
			renderBitmap[innerY * pitch + px] = innerColor;
	}
	// Bottom edge
	bottomY = innerY + innerH - 1;
	if (bottomY < height) {
		for (int px = innerX; px < innerX + innerW && px < width; px++) {
			if (px >= 0)
				renderBitmap[bottomY * pitch + px] = innerColor;
		}
	}
	// Left edge
	for (int py = innerY; py < innerY + innerH && py < height; py++) {
		if (py >= 0 && innerX >= 0 && innerX < width)
			renderBitmap[py * pitch + innerX] = innerColor;
	}
	// Right edge
	rightX = innerX + innerW - 1;
	if (rightX < width) {
		for (int py = innerY; py < innerY + innerH && py < height; py++) {
			if (py >= 0)
				renderBitmap[py * pitch + rightX] = innerColor;
		}
	}
}

// Draw preview thumbnail content - emulates FUN_00428a10 + FUN_00429b40
// Based on FUN_00415CF8 assembly analysis:
//
// The original uses O_LEVEL.SAN (640x400) with chapter previews stacked vertically.
// Video offset (FUN_00425170) shifts which preview is visible:
//   X offset = -90 (0xffa6)
//   Y offset = chapter * -50 + 75
//
// For 320x200 mode, O_MENU_X.SAN doesn't contain chapter-specific preview images.
// Those are only in O_LEVEL.SAN (640x400). We display a styled placeholder instead
// with the chapter number and visual styling to match the original UI appearance.
void InsaneRebel2::drawPreviewThumbnail(byte *renderBitmap, int pitch, int width, int height, int chapter) {
	// Preview destination area coordinates (inside the inner border)
	// From assembly: Inner box at X=230, Y=75, W=80, H=50
	const int destX = 230;
	const int destY = 75;
	const int thumbW = 80;  // 0x50
	const int thumbH = 50;  // 0x32

	// Fill preview area with a dark blue gradient background
	// This creates a styled placeholder since O_MENU_X.SAN doesn't have previews
	for (int py = 0; py < thumbH; py++) {
		int dy = destY + py;
		if (dy < 0 || dy >= height) continue;

		// Create vertical gradient: darker at top (0x10), lighter at bottom (0x18)
		byte bgColor = 0x10 + (py * 8 / thumbH);

		for (int px = 0; px < thumbW; px++) {
			int dx = destX + px;
			if (dx < 0 || dx >= width) continue;
			renderBitmap[dy * pitch + dx] = bgColor;
		}
	}

	// Draw chapter number overlay in the center of the preview
	NutRenderer *font = _smush_smalfontNut;
	if (!font) return;

	char chapterStr[16];
	if (chapter < 15) {
		snprintf(chapterStr, sizeof(chapterStr), "CH.%d", chapter + 1);
	} else {
		snprintf(chapterStr, sizeof(chapterStr), "FINALE");
	}

	// Calculate text width for centering
	int textWidth = 0;
	int numChars = font->getNumChars();
	for (const char *c = chapterStr; *c; c++) {
		int charIdx = (unsigned char)*c;
		if (charIdx < numChars) {
			textWidth += font->getCharWidth(charIdx);
		}
	}

	// Center the text in the preview area
	int textX = destX + (thumbW - textWidth) / 2;
	int textY = destY + thumbH / 2 - 4;

	Common::Rect clipRect(0, 0, width, height);

	// Draw text shadow (offset by 1,1)
	int curX = textX + 1;
	for (const char *c = chapterStr; *c; c++) {
		int charIdx = (unsigned char)*c;
		if (charIdx < numChars) {
			int charWidth = font->getCharWidth(charIdx);
			if (curX >= 0 && curX + charWidth <= width && textY + 1 >= 0 && textY + 1 < height) {
				font->drawCharV7(renderBitmap, clipRect, curX, textY + 1, pitch, 0,
				                 kStyleAlignLeft, charIdx, true, true);
			}
			curX += charWidth;
		}
	}

	// Draw main text (bright)
	curX = textX;
	for (const char *c = chapterStr; *c; c++) {
		int charIdx = (unsigned char)*c;
		if (charIdx < numChars) {
			int charWidth = font->getCharWidth(charIdx);
			if (curX >= 0 && curX + charWidth <= width && textY >= 0 && textY < height) {
				font->drawCharV7(renderBitmap, clipRect, curX, textY, pitch, -1,
				                 kStyleAlignLeft, charIdx, true, true);
			}
			curX += charWidth;
		}
	}

	// Draw lock icon for locked chapters
	if (!_chapterUnlocked[chapter]) {
		byte lockColor = 0xF8;
		int lockX = destX + thumbW - 15;
		int lockY = destY + 5;

		// Draw padlock shape
		for (int i = 2; i < 6; i++) {
			if (lockX + i < width && lockY < height && lockY >= 0)
				renderBitmap[lockY * pitch + lockX + i] = lockColor;
		}
		for (int i = 1; i < 4; i++) {
			if (lockX + 2 < width && lockY + i < height && lockY + i >= 0)
				renderBitmap[(lockY + i) * pitch + lockX + 2] = lockColor;
			if (lockX + 5 < width && lockY + i < height && lockY + i >= 0)
				renderBitmap[(lockY + i) * pitch + lockX + 5] = lockColor;
		}
		for (int y = 0; y < 4; y++) {
			for (int x = 1; x < 7; x++) {
				if (lockX + x < width && lockY + 4 + y < height && lockY + 4 + y >= 0)
					renderBitmap[(lockY + 4 + y) * pitch + lockX + x] = lockColor;
			}
		}
	}
}

// Draw score/info line at bottom of chapter select - emulates FUN_00434cb0 calls
// For unlocked chapters: score display using TRS 80 at (25, 190)
// For locked chapters: password prompt at (30, 190)
void InsaneRebel2::drawChapterInfoLine(byte *renderBitmap, int pitch, int width, int height) {
	if (_chapterSelection < 0 || _chapterSelection >= 16) return;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer) return;

	NutRenderer *font = _smush_smalfontNut;
	if (!font) return;

	Common::Rect clipRect(0, 0, _vm->_screenWidth, _vm->_screenHeight);
	int actualPitch = _vm->_screenWidth;

	if (_chapterUnlocked[_chapterSelection]) {
		// Unlocked: show score info using TRS 80 at X=25 (0x19), Y=190 (0xbe)
		const char *scoreStr = splayer->getString(80);
		if (!scoreStr || !scoreStr[0]) return;

		int curX = 25;
		int numChars = font->getNumChars();
		for (const char *c = scoreStr; *c; c++) {
			byte ch = (byte)*c;
			if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
			if (ch < numChars) {
				int charW = font->getCharWidth(ch);
				if (curX >= 0 && curX + charW <= width && 190 < height) {
					font->drawCharV7(renderBitmap, clipRect, curX, 190, actualPitch, 1,
					                 kStyleAlignLeft, ch, false, false);
				}
				curX += charW;
			}
		}
	}
}

// Draw chapter selection overlay - called during O_LEVEL.SAN playback
// FUN_00415CF8 - Chapter selection screen with preview thumbnail
void InsaneRebel2::drawChapterSelectOverlay(byte *renderBitmap, int pitch, int width, int height) {
	// Emulates FUN_00415CF8 rendering via shared drawMenuItems(leftAligned=true)
	//
	// GAME.TRS chapter selection strings:
	//   TRS 40     = "^f02Chapters" (title)
	//   TRS 41-56  = unlocked chapter names (e.g. "^f01^c244Chapter 1 - The Dreighton Triangle")
	//   TRS 57     = "^f01^c240RETURN TO PILOTS"
	//   TRS 60     = "^f02Chapters" (title, locked section duplicate)
	//   TRS 61-76  = locked chapter names (e.g. "^f01^c244Chapter 1 -")
	//   TRS 77     = "^f01^c240RETURN TO PILOTS" (locked section duplicate)
	//
	// Menu array: items[0]=title, items[1..16]=chapters, items[17]=RETURN TO PILOTS
	// FUN_0041f5ae(0, &DAT_004577a8, 0x11, 1): param_3=17, param_4=1 (left-aligned)

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer) return;

	// Build items array matching original DAT_004577a8 layout
	const char *items[18];

	// items[0] = title = TRS 40 ("^f02Chapters")
	items[0] = splayer->getString(40);
	if (!items[0] || !items[0][0]) items[0] = "^f02Chapters";

	// items[1..16] = chapters, using unlocked (TRS 41-56) or locked (TRS 61-76) strings
	for (int i = 1; i <= 16; i++) {
		bool unlocked = (i - 1 < 16) && _chapterUnlocked[i - 1];
		int trsIdx = unlocked ? (40 + i) : (60 + i);
		items[i] = splayer->getString(trsIdx);
		if (!items[i] || !items[i][0]) items[i] = "";
	}

	// items[17] = "RETURN TO PILOTS" = TRS 57 ("^f01^c240RETURN TO PILOTS")
	items[17] = splayer->getString(57);
	if (!items[17] || !items[17][0]) items[17] = "^f01^c240RETURN TO PILOTS";

	// Render menu using shared renderer with left-aligned mode
	drawMenuItems(renderBitmap, pitch, width, height, items, 17, _chapterSelection, true);

	// Draw preview box border on the right side (FUN_004292d0 calls at lines 128-133)
	// The actual preview image is rendered by O_LEVEL.SAN FOBJ via the offset system
	drawPreviewBox(renderBitmap, pitch, width, height);

	// Draw score/info line at bottom
	drawChapterInfoLine(renderBitmap, pitch, width, height);
}

// ==================== Pilot Selection Menu (FUN_00414A41) ====================
// Emulates FUN_00414A41 - Pilot/save selection menu
// This appears before chapter selection. All options go to chapter selection except MAIN MENU.

int InsaneRebel2::runLevelSelect() {
	// Pilot selection menu loop - emulates FUN_00414A41
	// Returns:
	//   kLevelSelectPlay (1) = Go to chapter selection (pilot selected or NEW+difficulty chosen)
	//   kLevelSelectBack (0) = Return to main menu (MAIN MENU or ESC)
	//   kLevelSelectQuit (2) = Quit game
	//
	// Original action dispatch (FUN_00414A41):
	//   sel < N        → saved pilot selected → return 3 (start game)
	//   sel == N       → ADD NEW PILOT → difficulty submenu → loop back
	//   sel == N+1     → COPY PILOT → source select (no-op if N==0)
	//   sel == N+2     → DELETE PILOT → confirm select (no-op if N==0)
	//   sel == N+3     → RETURN TO MAIN MENU → return 1
	//   ESC            → return 1

	debug("Rebel2: Entering pilot selection (FUN_00414A41)");

	_menuInputActive = true;
	while (!_menuEventQueue.empty()) _menuEventQueue.pop();

	// Number of saved pilots (TODO: implement save system)
	int numPilots = 0;

	_levelSelection = 0;
	_levelItemCount = numPilots + 4;  // N pilots + NEW/COPY/DELETE/MAIN MENU
	_selectedLevel = 1;
	_menuRepeatDelay = 0;
	_gameState = kStatePilotSelect;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Pilot selection uses menu video as background (320x200 mode)
	while (!_vm->shouldQuit()) {
		_vm->_smushVideoShouldFinish = false;

		Common::String menuVideo = getRandomMenuVideo();
		debug("Rebel2: Playing pilot select background: %s", menuVideo.c_str());

		splayer->setCurVideoFlags(0x20);
		splayer->play(menuVideo.c_str(), 12);

		if (_vm->shouldQuit()) {
			_menuInputActive = false;
			return kLevelSelectQuit;
		}

		if (!_vm->_smushVideoShouldFinish) {
			continue;
		}

		_vm->_smushVideoShouldFinish = false;

		// Dispatch based on current game state
		if (_gameState == kStateDifficultySelect) {
			// Difficulty submenu — selection made
			debug("Rebel2: Difficulty %d selected", _difficultySelection);
			// Original stores difficulty in pilot record and loops back.
			// Since we have no save system, proceed to chapter select.
			_gameState = kStatePilotSelect;
			_menuInputActive = false;
			return kLevelSelectPlay;
		}

		// Pilot menu — process selection
		debug("Rebel2: Pilot selection made: %d (numPilots=%d)", _levelSelection, numPilots);

		if (_levelSelection < numPilots) {
			// Saved pilot selected — go to chapter selection
			_selectedLevel = _levelSelection + 1;
			debug("Rebel2: Pilot %d selected - going to chapter selection", _selectedLevel);
			_menuInputActive = false;
			return kLevelSelectPlay;
		} else if (_levelSelection == numPilots) {
			// ADD NEW PILOT — show difficulty submenu
			debug("Rebel2: ADD NEW PILOT - showing difficulty submenu");
			_gameState = kStateDifficultySelect;
			_difficultySelection = 2;  // Default to 3rd option (matching original init)
			// Continue loop — next video frame will render difficulty menu
			continue;
		} else if (_levelSelection == numPilots + 1) {
			// COPY PILOT — no-op when numPilots==0 (original checks local_10 != 0)
			if (numPilots > 0) {
				debug("Rebel2: COPY PILOT selected");
				// TODO: implement copy pilot sub-flow
			} else {
				debug("Rebel2: COPY PILOT - no pilots to copy");
			}
			continue;
		} else if (_levelSelection == numPilots + 2) {
			// DELETE PILOT — no-op when numPilots==0 (original checks local_10 != 0)
			if (numPilots > 0) {
				debug("Rebel2: DELETE PILOT selected");
				// TODO: implement delete pilot sub-flow
			} else {
				debug("Rebel2: DELETE PILOT - no pilots to delete");
			}
			continue;
		} else if (_levelSelection == numPilots + 3) {
			// RETURN TO MAIN MENU
			debug("Rebel2: Back to main menu selected");
			_menuInputActive = false;
			return kLevelSelectBack;
		}
	}

	_menuInputActive = false;
	return kLevelSelectQuit;
}

int InsaneRebel2::processLevelSelectInput() {
	// Process input for pilot selection and difficulty submenu
	// Handles both kStatePilotSelect and kStateDifficultySelect modes
	// Returns: -1 = no action, 0+ = item selected

	int result = -1;

	// Determine which menu mode we're in
	bool isDifficultyMode = (_gameState == kStateDifficultySelect);
	int &selection = isDifficultyMode ? _difficultySelection : _levelSelection;
	int itemCount = isDifficultyMode ? 6 : _levelItemCount;

	// Mouse hit Y positions — must match drawMenuItems() formula
	const int baseY = itemCount * -5 + 0x68;
	const int itemHeight = 10;

	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.pop();
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				selection--;
				if (selection < 0) {
					selection = itemCount - 1;
				}
				debug("LevelSelect: Selection changed to %d (UP, %s)",
				      selection, isDifficultyMode ? "difficulty" : "pilot");
				break;

			case Common::KEYCODE_DOWN:
				selection++;
				if (selection >= itemCount) {
					selection = 0;
				}
				debug("LevelSelect: Selection changed to %d (DOWN, %s)",
				      selection, isDifficultyMode ? "difficulty" : "pilot");
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				if (selection >= 0 && selection < itemCount) {
					result = selection;
					debug("LevelSelect: Item %d selected (ENTER, %s)",
					      selection, isDifficultyMode ? "difficulty" : "pilot");
				}
				break;

			case Common::KEYCODE_ESCAPE:
				if (isDifficultyMode) {
					// ESC in difficulty submenu — return to pilot menu
					// Note: original has no ESC in difficulty submenu, but we add it as a
					// graceful fallback
					_gameState = kStatePilotSelect;
					debug("LevelSelect: ESC in difficulty - back to pilot menu");
				} else {
					// ESC in pilot menu — back to main menu (return 1 in original)
					result = _levelItemCount - 1;  // Last item = MAIN MENU
					debug("LevelSelect: ESC pressed - back to main menu");
				}
				break;

			default:
				break;
			}
			break;

		case Common::EVENT_LBUTTONDOWN:
			{
				int mouseY = event.mouse.y;
				debug("LevelSelect: Click at Y=%d (%s)", mouseY,
				      isDifficultyMode ? "difficulty" : "pilot");

				for (int i = 0; i < itemCount; i++) {
					int itemY = baseY + i * itemHeight;
					if (mouseY >= itemY - 4 && mouseY < itemY + 6) {
						selection = i;
						result = i;
						debug("LevelSelect: Item %d clicked at Y=%d (itemY=%d)", i, mouseY, itemY);
						break;
					}
				}
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			if (isDifficultyMode) {
				_gameState = kStatePilotSelect;
			} else {
				result = _levelItemCount - 1;
			}
			break;

		default:
			break;
		}
	}

	return result;
}

void InsaneRebel2::drawLevelSelectOverlay(byte *renderBitmap, int pitch, int width, int height) {
	// =====================================================================
	// Pilot selection / difficulty submenu renderer
	// Emulates FUN_00414A41 → FUN_0041f5ae
	// =====================================================================

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	if (!splayer) {
		debug(1, "drawLevelSelectOverlay: SmushPlayer not available for TRS strings!");
		return;
	}

	if (_gameState == kStateDifficultySelect) {
		// =====================================================================
		// Difficulty submenu - LAB_00414ff6
		// FUN_0041f5ae(0, &DAT_00457458, 6, 0)
		// DAT_00457458 = 7 entries loaded from TRS 110-116 (FUN_00414073 lines 47-50)
		// param_3 = 6 → items[0]=title(TRS 110), items[1..6]=selectable(TRS 111-116)
		// =====================================================================
		const char *diffItems[7];
		for (int i = 0; i < 7; i++) {
			diffItems[i] = splayer->getString(110 + i);
			if (!diffItems[i] || !diffItems[i][0]) {
				diffItems[i] = "";
			}
		}
		drawMenuItems(renderBitmap, pitch, width, height, diffItems, 6, _difficultySelection);
		return;
	}

	// =====================================================================
	// Pilot menu - FUN_0041f5ae(0, &DAT_00457768, N+4, 0)
	// =====================================================================
	// items[0]    = title (TRS 20)
	// items[1..N] = saved pilots (formatted with ^f01^c005)
	// items[N+1]  = TRS 21 (ADD NEW PILOT)
	// items[N+2]  = TRS 22 (COPY PILOT)
	// items[N+3]  = TRS 23 (DELETE PILOT)
	// items[N+4]  = TRS 24 (RETURN TO MAIN MENU)

	int numPilots = 0;  // TODO: implement save system

	const char *pilotItems[11];
	int idx = 0;

	// Title: TRS 20
	pilotItems[idx++] = splayer->getString(20);

	// Saved pilot slots would be inserted here (items[1..numPilots])
	// Each formatted as "^f01^c005<name>^f00" by the original

	// Fixed options: TRS 21-24
	for (int i = 0; i < 4; i++) {
		pilotItems[idx++] = splayer->getString(21 + i);
	}

	for (int i = 0; i < idx; i++) {
		if (!pilotItems[i] || !pilotItems[i][0]) {
			pilotItems[i] = "";
		}
	}

	drawMenuItems(renderBitmap, pitch, width, height, pilotItems, numPilots + 4, _levelSelection);

	// Pilot info display at fixed coordinates when saved pilot selected
	// FUN_00414A41 lines 78-86: FUN_00434cb0 at Y=0xb4(180) and Y=0xbe(190)
	// TODO: Implement when save system is added
}

// ======================= Level Loading System =======================
// Emulates the level handler functions from FUN_00417E53 through FUN_0041BBE8
// Based on disassembly analysis of the retail Rebel Assault 2 executable.

Common::String InsaneRebel2::getLevelDir(int levelId) {
	// Returns directory name like "LEV01" for level 1
	return Common::String::format("LEV%02d", levelId);
}

Common::String InsaneRebel2::getLevelPrefix(int levelId) {
	// Returns file prefix like "01" for level 1
	return Common::String::format("%02d", levelId);
}

void InsaneRebel2::playIntroSequence() {
	// Emulates case 0 in FUN_004142BD
	// Plays the game intro sequence:
	// 1. CREDITS/O_OPEN_C.SAN - Fox logo (if certain conditions)
	// 2. CREDITS/O_OPEN_D.SAN - LucasArts logo (if certain conditions)
	// 3. OPEN/O_OPEN_A.SAN - Main intro
	// 4. OPEN/O_OPEN_B.SAN - Additional intro (if conditions)

	debug("Rebel2: Playing intro sequence");

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Set intro flags (non-interactive)
	splayer->setCurVideoFlags(0x20);

	// Play Fox logo (CREDITS/O_OPEN_C.SAN)
	// In retail, this checks if 'f', 'o', 'x' keys are held (easter egg)
	// We'll play it unconditionally for now
	debug("Rebel2: Playing Fox logo");
	splayer->play("CREDITS/O_OPEN_C.SAN", 12);

	if (_vm->shouldQuit()) return;

	// Play LucasArts logo (CREDITS/O_OPEN_D.SAN)
	// In retail, this checks if 'b', 'o', 't' keys are held
	debug("Rebel2: Playing LucasArts logo");
	splayer->play("CREDITS/O_OPEN_D.SAN", 12);

	if (_vm->shouldQuit()) return;

	// Play main intro (OPEN/O_OPEN_A.SAN)
	debug("Rebel2: Playing main intro");
	splayer->play("OPEN/O_OPEN_A.SAN", 12);

	if (_vm->shouldQuit()) return;

	// Play additional intro (OPEN/O_OPEN_B.SAN)
	// In retail, this plays if DAT_0047ab45 or DAT_0047ab47 != 0
	debug("Rebel2: Playing additional intro");
	splayer->play("OPEN/O_OPEN_B.SAN", 12);
}

void InsaneRebel2::playMissionBriefing() {
	// Emulates FUN_00415CF8 (partial - just the video)
	// Plays OPEN/O_LEVEL.SAN which shows the mission briefing screen

	debug("Rebel2: Playing mission briefing");

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x08);  // Briefing mode flag
	splayer->play("OPEN/O_LEVEL.SAN", 12);
}

void InsaneRebel2::playCinematic(const char *filename) {
	// Play a cinematic/cutscene video with proper intro mode setup
	// This helper ensures:
	// 1. Handler is reset to 0 (no HUD, no shooting)
	// 2. Video flags are set to 0x28 (cinematic with buffer preserve)
	//
	// Original: All video wrapper functions (FUN_00417168, FUN_004171c5,
	// FUN_00417ab2, FUN_00417327) add | 8 to the base flags before calling
	// FUN_0041f4d0, so the 0x08 bit (preserve buffer) is always set.
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // No status bar during cinematics

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x28);  // Cinematic mode + buffer preserve (0x20 | 0x08)
	splayer->play(filename, 12);
}

void InsaneRebel2::playLevelBegin(int levelId) {
	// Play the level beginning cinematic (LEVXX/XXBEG.SAN)
	// Emulates FUN_004171c5 call in each level handler

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sBEG.SAN", dir.c_str(), prefix.c_str());

	debug("Rebel2: Playing level %d beginning: %s", levelId, filename.c_str());
	playCinematic(filename.c_str());
}

bool InsaneRebel2::playLevelGameplay(int levelId) {
	// Play the main gameplay video(s) for a level
	// Returns true if level completed (damage < 0xff), false if died
	//
	// Different levels have different gameplay structures:
	// - Level 1, 4, 5: Single gameplay SAN (XXPXX.SAN or XXPLAY.SAN)
	// - Level 2: Multiple parts with subdirectories (P1/, P2/, P3/)
	// - Level 3, 6: Two gameplay phases (XXPLAY1.SAN, XXPLAY2.SAN)

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename;

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Set gameplay flags (interactive with HUD)
	splayer->setCurVideoFlags(0x28);

	// Reset damage/shield for this level
	_playerShield = 255;
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // Will be set by IACT opcode 6 if par4==1

	debug("Rebel2: Starting gameplay for level %d", levelId);

	switch (levelId) {
	case 1:
		// Level 1: Single gameplay file (01P01.SAN)
		// Level 1 uses Handler 0x26 (turret mode) - set before gameplay
		_rebelHandler = 0x26;
		filename = Common::String::format("%s/%sP01.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);
		break;

	case 2:
		// Level 2: Has cutscene first, then multiple parts
		// Level 2 uses Handler 8 (third-person on foot mode) - set before gameplay
		_rebelHandler = 8;
		// First play the cutscene
		filename = Common::String::format("%s/%sCUT.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing cutscene %s", filename.c_str());
		splayer->setCurVideoFlags(0x28);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Part 1 (multiple variations - play A for now)
		splayer->setCurVideoFlags(0x28);
		filename = Common::String::format("%s/P1/%sP01_A.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Post segment 1
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		filename = Common::String::format("%s/%sPST1.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->setCurVideoFlags(0x28);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Part 2
		_rebelHandler = 8;
		splayer->setCurVideoFlags(0x28);
		filename = Common::String::format("%s/P2/%sP02_A.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Post segment 2
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		filename = Common::String::format("%s/%sPST2.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->setCurVideoFlags(0x28);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Part 3
		_rebelHandler = 8;
		splayer->setCurVideoFlags(0x28);
		filename = Common::String::format("%s/P3/%sP03_A.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);
		break;

	case 3:
		// Level 3: Two gameplay phases (third-person ship)
		// Level 3 uses Handler 7 (third-person ship mode) - FUN_0040d836/FUN_0040c3cc
		_rebelHandler = 7;
		filename = Common::String::format("%s/%sPLAY1.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Post segment
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		filename = Common::String::format("%s/%sPOST1.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->setCurVideoFlags(0x28);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Phase 2 — handler will be re-set by IACT opcode 6
		splayer->setCurVideoFlags(0x28);
		filename = Common::String::format("%s/%sPLAY2.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);
		break;

	case 4:
		_rebelHandler = 0x26;
		// Level 4: Has cutscene, then single gameplay
		filename = Common::String::format("%s/%sCUT.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing cutscene %s", filename.c_str());
		splayer->setCurVideoFlags(0x28);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit()) return false;

		splayer->setCurVideoFlags(0x28);
		filename = Common::String::format("%s/%sPLAY.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);
		break;

	case 5:
		// Level 5: Single gameplay file
		filename = Common::String::format("%s/%sPLAY.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);
		break;

	case 6:
		// Level 6: Two gameplay phases
		filename = Common::String::format("%s/%sPLAY1.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Post segment
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		filename = Common::String::format("%s/%sPOST1.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->setCurVideoFlags(0x28);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Phase 2 — handler will be re-set by IACT opcode 6
		splayer->setCurVideoFlags(0x28);
		filename = Common::String::format("%s/%sPLAY2.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);
		break;

	default:
		// For levels 7-15 (not in demo), try common patterns
		// First try XXPLAY.SAN
		filename = Common::String::format("%s/%sPLAY.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Trying %s", filename.c_str());
		splayer->play(filename.c_str(), 12);
		break;
	}

	// Return true if player survived (shield > 0), false if died
	return (_playerShield > 0);
}

void InsaneRebel2::playLevelEnd(int levelId) {
	// Play level completion video (LEVXX/XXEND.SAN)
	// Emulates FUN_00417327 call

	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // No status bar during end cinematic

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sEND.SAN", dir.c_str(), prefix.c_str());

	debug("Rebel2: Playing level %d end: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417327 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 12);
}

void InsaneRebel2::playLevelDeath(int levelId) {
	// Play death video (LEVXX/XXDIE_X.SAN)
	// The variant depends on the frame where player died
	// For simplicity, we'll play the A variant

	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // No status bar during death cinematic

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);

	// Most levels have DIE_A, some have just DIE
	Common::String filename;
	if (levelId == 2 || levelId == 4 || levelId == 10 || levelId == 12 || levelId == 14) {
		filename = Common::String::format("%s/%sDIE.SAN", dir.c_str(), prefix.c_str());
	} else {
		filename = Common::String::format("%s/%sDIE_A.SAN", dir.c_str(), prefix.c_str());
	}

	debug("Rebel2: Playing level %d death: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 12);
}

void InsaneRebel2::playLevelRetry(int levelId) {
	// Play retry prompt video (LEVXX/XXRETRY.SAN)
	// Reset handler state for the retry cinematic

	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // Reset for retry - will be set by IACT opcode 6 if needed

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sRETRY.SAN", dir.c_str(), prefix.c_str());

	debug("Rebel2: Playing level %d retry: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 12);
}

void InsaneRebel2::playLevelGameOver(int levelId) {
	// Play game over video (LEVXX/XXOVER.SAN)
	// Emulates FUN_00417ab2 call

	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // No status bar during game over cinematic

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename = Common::String::format("%s/%sOVER.SAN", dir.c_str(), prefix.c_str());

	debug("Rebel2: Playing level %d game over: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417ab2 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 12);
}

void InsaneRebel2::playCreditsSequence() {
	// Play the end credits (OPEN/O_CREDIT.SAN)
	// Individual credits are in CREDITS/CRED_XX.SAN

	debug("Rebel2: Playing credits");

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x20);
	splayer->play("OPEN/O_CREDIT.SAN", 12);
}

int InsaneRebel2::runLevel(int levelId) {
	// Main level dispatcher - calls per-level handlers
	// Each level handler emulates its retail counterpart (FUN_00417E53 etc.)

	debug("Rebel2: Starting level %d", levelId);

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

	// Initialize common player state
	_playerLives = 3;
	_playerShield = 255;
	_playerScore = 0;
	_playerDamage = 0;
	resetDamageFlash();
	_damageHighFlashCounter = 0;
	_damageShakeCounter = 0;
	_currentPhase = 1;
	_phaseScore = 0;
	_phaseMisses = 0;

	// Dispatch to per-level handler
	switch (levelId) {
	case 1:
		return runLevel1();
	case 2:
		return runLevel2();
	case 3:
		return runLevel3();
	case 4:
		return runLevel4();
	case 5:
		return runLevel5();
	case 6:
		return runLevel6();
	case 7:
		return runLevel7();
	case 8:
		return runLevel8();
	case 9:
		return runLevel9();
	case 10:
		return runLevel10();
	case 11:
		return runLevel11();
	case 12:
		return runLevel12();
	case 13:
		return runLevel13();
	case 14:
		return runLevel14();
	case 15:
		return runLevel15();
	default:
		return runLevel1();
	}
}

// =============================================================================
// Helper functions
// =============================================================================

int InsaneRebel2::getRandomVariant(int max) {
	// Emulates FUN_004233a0 - returns random number 0 to max-1
	return _vm->_rnd.getRandomNumber(max - 1);
}

Common::String InsaneRebel2::selectDeathVideoVariant(int levelId, int phase, int frame) {
	// Select death video variant based on level, phase, and death frame
	// Emulates the frame-based death video selection in retail level handlers
	//
	// Returns variant suffix: "A", "B", "C", etc.

	switch (levelId) {
	case 1:
		// Level 1: Random between A and B
		return (getRandomVariant(2) == 0) ? "A" : "B";

	case 2:
		// Level 2: Just "DIE" (no variants)
		return "";

	case 3:
		// Level 3: Based on death frame and phase
		if (phase == 1) {
			// Phase 1 death video selection (from FUN_0041885F lines 80-96)
			if (frame < 0x10c) return "B";       // < 268
			if (frame < 0x1a9) return "A";       // < 425
			if (frame < 0x247) return "C";       // < 583
			if (frame < 700) return "A";
			if (frame < 900) return "B";
			return "A";
		} else {
			// Phase 2 death video selection (from FUN_0041885F lines 53-67)
			if (frame < 0x2f1) return "A";       // < 753
			if (frame < 0x347) return "B";       // < 839
			if (frame < 0x3b1) return "C";       // < 945
			if (frame < 0x405) return "A";       // < 1029
			return "C";
		}

	case 4:
		// Level 4: Just "DIE" (no variants)
		return "";

	case 5:
		// Level 5: Random between A and B (like Level 1)
		return (getRandomVariant(2) == 0) ? "A" : "B";

	case 6:
		// Level 6 (FUN_004190D6): Phase-based with detailed frame selection
		if (phase == 1) {
			// DAT_0047a7f8 == 5 (phase 1)
			if (frame < 0x4e) return "D";
			if (frame < 0xe0) return "A";
			if (frame < 0x122) return "D";
			if (frame < 0x1b4) return "B";
			if (frame < 499) return "D";
			if (frame < 0x286) return "C";
			return "D";
		} else {
			// DAT_0047a7f8 == 6 (phase 2)
			if (frame < 0xcc) return "E";
			if (frame < 0xfe) return "G";
			if (frame < 0x122) return "E";
			if (frame < 0x149) return "G";
			if (frame < 0x166) return "F";
			if (frame < 0x174) return "E";
			if (frame < 0x19f) return "F";
			if (frame < 0x1b2) return "G";
			if (frame < 0x1c8) return "F";
			if (frame < 0x207) return "E";
			if (frame < 0x217) return "F";
			if (frame < 0x23b) return "G";
			if (frame < 0x25b) return "F";
			if (frame < 0x285) return "E";
			return "G";
		}

	case 7:
		// Level 7 (FUN_0041974C): Based on DAT_0047ab8c (fork state)
		// DAT_0047ab8c != 0 → DIE_B; == 0 → DIE_A
		// We use phase as a proxy (phase 2 = reached fork)
		return (phase >= 2) ? "B" : "A";

	case 8:
		// Level 8 (FUN_00419976): Random A or B
		return (getRandomVariant(2) == 0) ? "A" : "B";

	case 9:
		// Level 9 (FUN_00419B86): Based on DAT_0047ab94 (death cause)
		// 0→A, 1→C, else→B. Use phase as proxy.
		return "A";  // Default; exact tracking of DAT_0047ab94 deferred

	case 10:
		// Level 10 (FUN_00419E0A): Single death video (no variant suffix)
		return "";

	case 11:
		// Level 11 (FUN_0041A00C): Phase-based death videos
		// Phase 1 → DIE_A, Phase 2 → DIE_B, Phase 3 → DIE_C
		if (phase <= 1) return "A";
		if (phase == 2) return "B";
		return "C";

	case 12:
		// Level 12 (FUN_0041AA14): Single death video (no variants)
		return "";

	case 13:
		// Level 13 (FUN_0041B3E1): Frame-based
		if (frame < 0x1c2) return "A";
		if (frame < 0x302) return "B";
		if (frame < 0x4ec) return "C";
		if (frame < 0x5b4) return "B";
		return "D";

	case 14:
		// Level 14 (FUN_0041B6E8): Single death video (no variant suffix)
		return "";

	case 15:
		// Level 15 (FUN_0041B8D7): Frame-based with many thresholds
		if (frame < 0x21e) return "A";
		if (frame < 0x2f9) return "B";
		if (frame < 0x3e5) return "C";
		if (frame < 0x4a0) return "B";
		if (frame < 0x588) return "C";
		if (frame < 0x65e) return "B";
		return "D";

	default:
		return "A";
	}
}

void InsaneRebel2::playLevelDeathVariant(int levelId, int phase, int frame) {
	// Play death video with proper variant selection

	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // No status bar during death cinematic

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String variant = selectDeathVideoVariant(levelId, phase, frame);
	Common::String filename;

	if (variant.empty()) {
		// No variant suffix (Level 2, 4)
		filename = Common::String::format("%s/%sDIE.SAN", dir.c_str(), prefix.c_str());
	} else {
		filename = Common::String::format("%s/%sDIE_%s.SAN", dir.c_str(), prefix.c_str(), variant.c_str());
	}

	debug("Rebel2: Playing death video: %s (phase=%d, frame=%d)", filename.c_str(), phase, frame);

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 12);
}

void InsaneRebel2::playLevelRetryVariant(int levelId, int phase) {
	// Play retry video - phase-specific for multi-phase levels

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

	debug("Rebel2: Playing retry video: %s (phase=%d)", filename.c_str(), phase);

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play(filename.c_str(), 12);
}

// =============================================================================
// Level 1 Handler - FUN_00417E53
// Single gameplay phase (01P01.SAN)
// =============================================================================

int InsaneRebel2::runLevel1() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play level beginning cinematic (01BEG.SAN)
	playLevelBegin(1);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(1);
			return kLevelGameOver;
		}

		// Play retry prompt and loop
		playLevelRetry(1);
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

// =============================================================================
// Wave State Management - FUN_00417b61
// Waits for video completion, accumulates kill state, redistributes kill credits.
// Used by all multi-wave levels (Level 2, 3, 6, etc.) as the core wave loop primitive.
// =============================================================================

uint16 InsaneRebel2::processWaveEnd(int16 mask, int16 *budget, int16 threshold, uint16 flags) {
	// FUN_00417b61: Core wave management function
	// Called after each wave video plays. Handles:
	// 1. Waiting for video to finish (with early exit on enemy completion)
	// 2. Copying wave state to accumulated phase state
	// 3. Redistributing kill credits from the budget
	//
	// Returns: kill bits credited this wave, or 0xFFFF on death/quit/completion

	uint16 result = 0;

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
			if (budget) (*budget)++;
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

// =============================================================================
// Level 2 Handler - FUN_00418063
// Multiple parts with P1/P2/P3 subdirectories
// Random animation variants for each part
// =============================================================================

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
	int prevWaveState = 0;  // local_8: previous wave's state for Phase 3 randomization

	// Play cutscene (02CUT.SAN)
	playCinematic("LEV02/02CUT.SAN");
	if (_vm->shouldQuit()) return kLevelQuit;

	// Play level beginning cinematic (02BEG.SAN)
	// Original: FUN_004171c5("LEV02/02BEG.SAN", 0x20, 0xab, 0xa0, 10, 2, 0x46)
	// Includes text overlay from GAME.TRS — deferred until text rendering is ready.
	playLevelBegin(2);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		// ===== PHASE 1: P1/02P01_X.SAN =====
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

		if (_vm->shouldQuit()) return kLevelQuit;

		// processWaveEnd after A.SAN (threshold=0, no early exit for background loader)
		processWaveEnd(0x36, &budget, 0, 0);

		// Phase 1 wave loop: random B/C/D until all type 1,2 enemies killed
		// Original: while (uVar3 >= 0 && (DAT_0047ab9c & 6) != 6)
		while (_playerDamage < 255 && (_rebelPhaseState & 0x06) != 0x06) {
			if (_vm->shouldQuit()) return kLevelQuit;

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
		if ((_rebelPhaseState & 0x10) != 0) bonusCount++;

		if (_playerDamage >= 255) goto level2_death;
		if (_vm->shouldQuit()) return kLevelQuit;

		// Post segment 1 (02PST1.SAN)
		// Original: FUN_00417168("02PST1.SAN", 0x20) → flags = 0x20 | 0x08 = 0x28
		// FUN_00417168 adds | 8 to preserve the screen buffer between gameplay and transition
		// Reset handler to 0 so procPostRendering skips HUD/sprite drawing during cinematic
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV02/02PST1.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		// ===== PHASE 2: P2/02P02_X.SAN =====
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

		if (_vm->shouldQuit()) return kLevelQuit;

		// Phase 2 wave loop: processWaveEnd at TOP of loop (matches assembly structure)
		// Original: local_10 = FUN_00417b61(0x3e, local_14, 0, 0); then switch(local_10)
		while (true) {
			uint16 waveSelect = processWaveEnd(0x3e, &budget, 0, 0);
			if (waveSelect == 0xFFFF || (_rebelPhaseState & 0x0e) == 0x0e) break;
			if (_vm->shouldQuit()) return kLevelQuit;

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

		if ((_rebelPhaseState & 0x10) != 0) bonusCount++;

		if (_playerDamage >= 255) goto level2_death;
		if (_vm->shouldQuit()) return kLevelQuit;

		// Post segment 2 (02PST2.SAN)
		// Original: FUN_00417168("02PST2.SAN", 0x20) → flags = 0x20 | 0x08 = 0x28
		// Reset handler to 0 so procPostRendering skips HUD/sprite drawing during cinematic
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV02/02PST2.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		// ===== PHASE 3: P3/02P03_X.SAN =====
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

		if (_vm->shouldQuit()) return kLevelQuit;

		// Phase 3: processWaveEnd at BOTTOM (like Phase 1), waveSelect carried across iterations
		// Original: local_10 = FUN_00417b61(0x3e, local_14, 0, 0); while (loop) { ...; local_10 = FUN_00417b61(0x3e, local_14, 0x14, 0); }
		{
			uint16 waveSelect = processWaveEnd(0x3e, &budget, 0, 0);

			while (waveSelect != 0xFFFF && (_rebelPhaseState & 0x0e) != 0x0e) {
				if (_vm->shouldQuit()) return kLevelQuit;

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

		if ((_rebelPhaseState & 0x10) != 0) bonusCount++;
		totalKills += _rebelKillCounter;

		if (_playerDamage >= 255) goto level2_death;
		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;
		continue;  // Restart from beginning
	}

	return kLevelQuit;
}

// =============================================================================
// Level 3 Handler - FUN_0041885F
// Two phases with per-phase retry handling
// Phase 1: 03PLAY1.SAN, Phase 2: 03PLAY2.SAN
// =============================================================================

int InsaneRebel2::runLevel3() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	int phase1Score = 0;  // Score preserved across phase 2 retries

	// Play level beginning cinematic (03BEG.SAN)
	playLevelBegin(3);
	if (_vm->shouldQuit()) return kLevelQuit;

	// ===== PHASE 1 retry loop =====
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

		if (_vm->shouldQuit()) return kLevelQuit;

		if (_playerShield > 0) {
			// Phase 1 completed - save score and proceed to phase 2
			phase1Score = _playerScore;
			break;
		}

		// Died in phase 1 - frame-based death video
		debug("Rebel2: Level 3 Phase 1 death at frame %d", _deathFrame);
		playLevelDeathVariant(3, 1, _deathFrame);
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(3);
			return kLevelGameOver;
		}

		// Phase 1 retry (03RETRY.SAN)
		playLevelRetryVariant(3, 1);
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	if (_vm->shouldQuit()) return kLevelQuit;

	// Post segment 1 (03POST1.SAN)
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	// Reset handler to 0 so procPostRendering skips HUD/sprite drawing during cinematic
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;
	splayer->setCurVideoFlags(0x28);
	splayer->play("LEV03/03POST1.SAN", 12);
	if (_vm->shouldQuit()) return kLevelQuit;

	// ===== PHASE 2 retry loop (preserves phase 1 score) =====
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

		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			// Use phase 2 specific game over (03OVER.SAN, same file but at different point)
			playLevelGameOver(3);
			return kLevelGameOver;
		}

		// Phase 2 retry (03RETRYB.SAN)
		playLevelRetryVariant(3, 2);
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

// =============================================================================
// Level 4 Handler
// Cutscene + single gameplay phase
// =============================================================================

int InsaneRebel2::runLevel4() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play cutscene (04CUT.SAN)
	// Original: FUN_00417168 adds | 8, so flags = 0x20 | 0x08 = 0x28
	splayer->setCurVideoFlags(0x28);
	splayer->play("LEV04/04CUT.SAN", 12);
	if (_vm->shouldQuit()) return kLevelQuit;

	// Play level beginning cinematic (04BEG.SAN)
	playLevelBegin(4);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(4);
			return kLevelGameOver;
		}

		playLevelRetry(4);
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

// =============================================================================
// Level 5 Handler - FUN_00418EC6
// Single gameplay phase (05PLAY.SAN)
// Random A/B death video like Level 1
// =============================================================================

int InsaneRebel2::runLevel5() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play level beginning cinematic (05BEG.SAN)
	playLevelBegin(5);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(5);
			return kLevelGameOver;
		}

		playLevelRetry(5);
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

// =============================================================================
// Level 6 Handler - FUN_00419317
// Two phases with per-phase retry (like Level 3)
// Phase 1: 06PLAY1.SAN, Phase 2: 06PLAY2.SAN
// =============================================================================

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
	if (_vm->shouldQuit()) return kLevelQuit;

	// FUN_00401000 + FUN_0041c7d0 + FUN_0040c040 — handler init done by IACT opcode 6

	// Outer retry loop — restarts phase 1 on phase 1 death
	while (!_vm->shouldQuit()) {
		// FUN_00407d10 — reset shot/hit counters
		clearBit(0);

		// DAT_0047ab9c = 0xffffffff — init phase state
		_rebelPhaseState = 0xffffffff;

		// ===== PHASE 1 =====
		_rebelLevelType = 5;  // DAT_0047a7f8 = 5
		_currentPhase = 1;

		debug("Rebel2: Level 6 Phase 1");
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV06/06PLAY1.SAN", 12);
		// TODO: Mid-level switch at frame 0x2a8 to 06PLAY1B.SAN (flags 0x468)
		// + score checkpoint (FUN_00407f55) — needs per-frame callback
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit()) return kLevelQuit;

		if (_playerShield <= 0) {
			// Died in phase 1
			debug("Rebel2: Level 6 Phase 1 death at frame %d", _deathFrame);
			playLevelDeathVariant(6, 1, _deathFrame);
			if (_vm->shouldQuit()) return kLevelQuit;

			_playerLives--;
			if (_playerLives <= 0) {
				playLevelGameOver(6);
				return kLevelGameOver;
			}

			// Phase 1 retry (06RETRY.SAN) → restart outer loop
			playLevelRetryVariant(6, 1);
			if (_vm->shouldQuit()) return kLevelQuit;
			continue;
		}

		// Phase 1 survived — save score, play POST1
		totalScore = _playerScore;  // local_8 = DAT_0047ab84

		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV06/06POST1.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;

		// ===== PHASE 2 retry loop (inner while(true) in original) =====
		while (!_vm->shouldQuit()) {
			_rebelLevelType = 6;  // DAT_0047a7f8 = 6
			_currentPhase = 2;
			clearBit(0);  // FUN_00407d10

			debug("Rebel2: Level 6 Phase 2");
			splayer->setCurVideoFlags(0x28);
			splayer->play("LEV06/06PLAY2.SAN", 12);
			_deathFrame = splayer->_frame;

			if (_vm->shouldQuit()) return kLevelQuit;

			// Accumulate score: local_8 = DAT_0047ab84 + local_8
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
			if (_vm->shouldQuit()) return kLevelQuit;

			_playerLives--;
			if (_playerLives <= 0) {
				playLevelGameOver(6);
				return kLevelGameOver;
			}

			// Phase 2 retry (06RETRYB.SAN) → re-enter phase 2
			playLevelRetryVariant(6, 2);
			if (_vm->shouldQuit()) return kLevelQuit;
		}

		break;  // Should only reach here on shouldQuit
	}

	return kLevelQuit;
}

// =============================================================================
// Level 7 Handler - FUN_0041974C
// "TIE Training" - Canyon flight with fork at frame 1592
// Single gameplay phase (07PLAY.SAN), optional second segment (07PLAYB.SAN)
// Death: DAT_0047ab8c-based (fork reached → DIE_B, not reached → DIE_A)
// =============================================================================

int InsaneRebel2::runLevel7() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	bool reachedFork = false;  // DAT_0047ab8c equivalent — tracks if 07PLAYB was played

	// Play cutscene (07CUT.SAN)
	playCinematic("LEV07/07CUT.SAN");
	if (_vm->shouldQuit()) return kLevelQuit;

	// Play level beginning cinematic (07BEG.SAN)
	playLevelBegin(7);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(7);
			return kLevelGameOver;
		}

		playCinematic("LEV07/07RETRY.SAN");
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

// =============================================================================
// Level 8 Handler - FUN_00419976
// "Flight to Imdaar" - Y-Wing space battle (single phase)
// No cutscene (starts with BEG). flags=0x08 for gameplay.
// Death: random A or B
// =============================================================================

int InsaneRebel2::runLevel8() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// No cutscene — starts directly with BEG
	// Original: FUN_004171c5("08BEG.SAN", 0x20, 0xb1, 0xa0, 10, 5, 0x4b)
	playLevelBegin(8);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(8);
			return kLevelGameOver;
		}

		playCinematic("LEV08/08RETRY.SAN");
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

// =============================================================================
// Level 9 Handler - FUN_00419B86
// "The Mine Field" - Navigate through force fields (single phase)
// No cutscene. Initial phaseState = 0xfffffffe (all bits set except bit 0).
// Mid-events at frames 0x19f (415) and 0x352 (850): FUN_00407f55 (score checkpoint)
// Death: DAT_0047ab94-based (0→A, 1→C, else→B)
// =============================================================================

int InsaneRebel2::runLevel9() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// No cutscene — starts directly with BEG
	// Original: FUN_004171c5("09BEG.SAN", 0x20, 0xb2, 0xa0, 10, 200, 0x10e)
	playLevelBegin(9);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(9);
			return kLevelGameOver;
		}

		playCinematic("LEV09/09RETRY.SAN");
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

// =============================================================================
// Level 10 Handler - FUN_00419E0A
// "Speeder Bikes" - Forest speeder chase (single phase)
// Has cutscene. Single death video (10DIE.SAN, no variants).
// Original plays DIE then RETRY in sequence (no separate check).
// =============================================================================

int InsaneRebel2::runLevel10() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play cutscene (10CUT.SAN)
	playCinematic("LEV10/10CUT.SAN");
	if (_vm->shouldQuit()) return kLevelQuit;

	// Play level beginning cinematic (10BEG.SAN)
	// Original: FUN_004171c5("10BEG.SAN", 0x20, 0xb3, 0xa0, 10, 2, 0x46)
	playLevelBegin(10);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;
		playCinematic("LEV10/10RETRY.SAN");
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

// =============================================================================
// Level 11 Handler - FUN_0041A00C
// "Inside the Terror" - Three phases + bridge puzzle (Handler 8, on-foot)
//
// Phase 1: P1/11P01_X (A,B,C,D) - behind barrels, mask 0x0e
// Phase 2: P2/11P02_X (A,B,C,D) - walls on right, mask 0x0e, flags=3
// Phase 3 first half: P3/11P03_X (A-F) - bridge puzzle, mask 0x7e
//   Exit when (phaseState & 0x70) == 0x70
// POST3/POST3B/POST3C bridge cinematics
// Phase 3 second half: P3/11P03_X (G-L) - after bridge, mask 0x0e
// =============================================================================

int InsaneRebel2::runLevel11() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	int totalKills = 0;
	int totalMisses = 0;
	int prevPhaseState = 0;

	// Kill credit budget bases per phase (from level data table DAT_0047e0e8)
	static const int16 kLevel11BudgetBase[4] = { 3, 3, 3, 3 };

	// Play cutscene (11CUT.SAN)
	playCinematic("LEV11/11CUT.SAN");
	if (_vm->shouldQuit()) return kLevelQuit;

	// Play level beginning cinematic (11BEG.SAN)
	playLevelBegin(11);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		// ===== PHASE 1: P1/11P01_X.SAN =====
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

		if (_vm->shouldQuit()) return kLevelQuit;

		{
			uint16 waveSelect = processWaveEnd(0x0e, &budget, 0, 0);

			// Phase 1 wave loop: random(2) | (waveSelect & 8) → variants
			// 0→D, 1→C, 8→B, 9→A
			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_playerDamage >= 255) goto level11_death_phase1;
		if (_vm->shouldQuit()) return kLevelQuit;

		// Post segment 1 (11POST1.SAN)
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV11/11POST1.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		// ===== PHASE 2: P2/11P02_X.SAN =====
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

		if (_vm->shouldQuit()) return kLevelQuit;

		{
			// Phase 2: flags=3 (maxCredits=2, redistribution ON)
			uint16 waveSelect = processWaveEnd(0x0e, &budget, 0, 3);

			// Random(4) for variant selection: A, B, C, D
			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_playerDamage >= 255) goto level11_death_phase2;
		if (_vm->shouldQuit()) return kLevelQuit;

		// Post segment 2 (11POST2.SAN)
		_rebelHandler = 0;
		_rebelStatusBarSprite = 0;
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV11/11POST2.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;

		totalKills += _rebelKillCounter;
		totalMisses += _rebelHitCounter;

		// ===== PHASE 3 FIRST HALF: P3/11P03_X (A-F) =====
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

		if (_vm->shouldQuit()) return kLevelQuit;

		{
			uint16 waveSelect = processWaveEnd(0x7e, &budget, 0, 0);
			int local_8 = 0;  // Tracks variant for randomization threshold

			// Loop until (phaseState & 0x70) == 0x70 (bridge targets destroyed)
			while (waveSelect != 0xFFFF && (_rebelPhaseState & 0x70) != 0x70) {
				if (_vm->shouldQuit()) return kLevelQuit;

				// Bonus sound: (phaseState & 0xe) == 0xe and previous wasn't
				if ((_rebelPhaseState & 0x0e) == 0x0e && (prevPhaseState & 0x0e) != 0x0e) {
					// FUN_00411931 bonus sound — not yet implemented
				}
				prevPhaseState = _rebelPhaseState;

				// Randomization: wider range for first few waves
				if (local_8 < 3) {
					local_8 = _vm->_rnd.getRandomNumber(7);  // 0-7
				} else {
					local_8 = _vm->_rnd.getRandomNumber(2);  // 0-2
				}

				const char *filename;
				switch (local_8) {
				case 0:  filename = "LEV11/P3/11P03_A.SAN"; break;
				case 1:  filename = "LEV11/P3/11P03_B.SAN"; break;
				case 2:  filename = "LEV11/P3/11P03_C.SAN"; break;
				case 3:  filename = "LEV11/P3/11P03_D.SAN"; break;
				case 4:  filename = "LEV11/P3/11P03_E.SAN"; break;
				case 5:  filename = "LEV11/P3/11P03_F.SAN"; break;
				case 6:  filename = "LEV11/P3/11P03_F.SAN"; break;  // duplicate F
				default: filename = "LEV11/P3/11P03_E.SAN"; break;  // duplicate E
				}

				debug("Rebel2: Level 11 Phase 3a wave - %s (state=0x%x local_8=%d)", filename, _rebelPhaseState, local_8);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				// Threshold only for higher variants (original: (2 < local_8) - 1 & 0x14)
				int16 threshold = (local_8 > 2) ? 0x14 : 0;
				waveSelect = processWaveEnd(0x7e, &budget, threshold, 0);
			}
		}

		if (_playerDamage >= 255) goto level11_death_phase3;
		if (_vm->shouldQuit()) return kLevelQuit;

		// ===== PHASE 3 BRIDGE CINEMATICS =====
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

		if (_vm->shouldQuit()) return kLevelQuit;

		// ===== PHASE 3 SECOND HALF: P3/11P03_X (G-L) =====
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

		if (_vm->shouldQuit()) return kLevelQuit;

		// Only enter wave loop if not all basic types killed already
		if ((_rebelPhaseState & 0x0e) < 0x0e) {
			int local_8 = 0;
			uint16 waveSelect = processWaveEnd(0x0e, &budget, 0, 0);

			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit()) return kLevelQuit;

				// Wider randomization for first few waves
				if (local_8 < 4) {
					local_8 = _vm->_rnd.getRandomNumber(8);  // 0-8
				} else {
					local_8 = _vm->_rnd.getRandomNumber(2);  // 0-2
				}

				const char *filename;
				switch (local_8) {
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

				debug("Rebel2: Level 11 Phase 3b wave - %s (state=0x%x local_8=%d)", filename, _rebelPhaseState, local_8);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				int16 threshold = (local_8 > 2) ? 0x14 : 0;
				waveSelect = processWaveEnd(0x0e, &budget, threshold, 0);
			}
		}

		totalKills += _rebelKillCounter;

		if (_playerDamage >= 255) goto level11_death_phase3;
		if (_vm->shouldQuit()) return kLevelQuit;

		// ===== LEVEL COMPLETED =====
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
		if (_vm->shouldQuit()) return kLevelQuit;
		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(11);
			return kLevelGameOver;
		}
		playCinematic("LEV11/11RETRY.SAN");
		_playerDamage = 0;
		if (_vm->shouldQuit()) return kLevelQuit;
		continue;  // Restart from Phase 1
	}

	return kLevelQuit;
}

// =============================================================================
// Level 12 Handler - FUN_0041AA14
// "Sewers" - Four phases FPS corridor shooting (Handler 25)
//
// Each phase: init video (P05/P06/P07/P08) → first wave → wave loop
// Phase 1: P1/12P01_X (A,B,C,D) mask=6
// Phase 2: P2/12P02_X (A,B,C,D,E,F) mask=6
// Phase 3: P3/12P03_X (A,B,C,D,F) mask=6
// Phase 4: P4/12P04_X (A,B,C,D,E,F) mask=0xe
// Closing: 12P09.SAN
// =============================================================================

int InsaneRebel2::runLevel12() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Kill credit budget bases per phase
	static const int16 kLevel12BudgetBase[4] = { 3, 4, 4, 4 };

	// Play cutscene (12CUT.SAN)
	playCinematic("LEV12/12CUT.SAN");
	if (_vm->shouldQuit()) return kLevelQuit;

	// Play level beginning cinematic (12BEG.SAN)
	playLevelBegin(12);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		// ===== PHASE 1: 12P05 → P1/12P01_X =====
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
		if (_vm->shouldQuit()) return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		// First wave (P1/12P01_A.SAN)
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/P1/12P01_A.SAN", 12);
		_deathFrame = splayer->_frame;
		if (_vm->shouldQuit()) return kLevelQuit;

		{
			uint16 waveSelect = processWaveEnd(6, &budget, 0x14, 0);

			// Wave loop: random(2) | (waveSelect & 2) → 0:C, 1:D, 2:A, 3:B
			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_playerDamage >= 255) goto level12_death;
		if (_vm->shouldQuit()) return kLevelQuit;

		// ===== PHASE 2: 12P06 → P2/12P02_X =====
		_currentPhase = 2;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;

		budget = kLevel12BudgetBase[1] + _vm->_rnd.getRandomNumber(3);

		// Initialization video (12P06.SAN)
		debug("Rebel2: Level 12 Phase 2 - init 12P06.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/12P06.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		// First wave (P2/12P02_A.SAN)
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/P2/12P02_A.SAN", 12);
		_deathFrame = splayer->_frame;
		if (_vm->shouldQuit()) return kLevelQuit;

		{
			uint16 waveSelect = processWaveEnd(6, &budget, 0x14, 0);

			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit()) return kLevelQuit;

				// Variant selection: (waveSelect & 2) controls which set
				int local_8;
				if ((waveSelect & 2) == 0) {
					local_8 = _vm->_rnd.getRandomNumber(2) + 3;  // 3, 4, or 5
				} else {
					local_8 = _vm->_rnd.getRandomNumber(2);      // 0, 1, or 2
				}

				const char *filename;
				switch (local_8) {
				case 0:  filename = "LEV12/P2/12P02_A.SAN"; break;
				case 1:  filename = "LEV12/P2/12P02_B.SAN"; break;
				case 2:  filename = "LEV12/P2/12P02_E.SAN"; break;
				case 3:  filename = "LEV12/P2/12P02_C.SAN"; break;
				case 4:  filename = "LEV12/P2/12P02_D.SAN"; break;
				default: filename = "LEV12/P2/12P02_F.SAN"; break;
				}

				debug("Rebel2: Level 12 Phase 2 wave - %s (state=0x%x local_8=%d)", filename, _rebelPhaseState, local_8);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				// Variants E(2) and F(5) reset threshold to 0
				int16 threshold = (local_8 == 2 || local_8 == 5) ? 0 : 0x14;
				waveSelect = processWaveEnd(6, &budget, threshold, 0);
			}
		}

		if (_playerDamage >= 255) goto level12_death;
		if (_vm->shouldQuit()) return kLevelQuit;

		// ===== PHASE 3: 12P07 → P3/12P03_X =====
		_currentPhase = 3;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;

		budget = kLevel12BudgetBase[2] + _vm->_rnd.getRandomNumber(3);

		// Initialization video (12P07.SAN)
		debug("Rebel2: Level 12 Phase 3 - init 12P07.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/12P07.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		// First wave (P3/12P03_A.SAN)
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/P3/12P03_A.SAN", 12);
		_deathFrame = splayer->_frame;
		if (_vm->shouldQuit()) return kLevelQuit;

		{
			int local_8 = 0;
			uint16 waveSelect = processWaveEnd(6, &budget, 0x14, 0);

			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit()) return kLevelQuit;

				// Wider randomization for first few waves
				if (local_8 < 4) {
					local_8 = _vm->_rnd.getRandomNumber(5);  // 0-5
				} else {
					local_8 = _vm->_rnd.getRandomNumber(3);  // 0-3
				}

				const char *filename;
				switch (local_8) {
				case 0:  filename = "LEV12/P3/12P03_C.SAN"; break;
				case 1:  filename = "LEV12/P3/12P03_D.SAN"; break;
				case 2:  filename = "LEV12/P3/12P03_A.SAN"; break;
				case 3:  filename = "LEV12/P3/12P03_B.SAN"; break;
				case 4:  filename = "LEV12/P3/12P03_F.SAN"; break;
				default: filename = "LEV12/P3/12P03_F.SAN"; break;  // duplicate F
				}

				debug("Rebel2: Level 12 Phase 3 wave - %s (state=0x%x local_8=%d)", filename, _rebelPhaseState, local_8);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				waveSelect = processWaveEnd(6, &budget, 0x14, 0);
			}
		}

		if (_playerDamage >= 255) goto level12_death;
		if (_vm->shouldQuit()) return kLevelQuit;

		// ===== PHASE 4: 12P08 → P4/12P04_X =====
		_currentPhase = 4;
		_rebelPhaseState = 0;
		_rebelWaveState = 0;

		budget = kLevel12BudgetBase[3] + _vm->_rnd.getRandomNumber(3);

		// Initialization video (12P08.SAN)
		debug("Rebel2: Level 12 Phase 4 - init 12P08.SAN budget=%d", budget);
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/12P08.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		// First wave (P4/12P04_A.SAN)
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/P4/12P04_A.SAN", 12);
		_deathFrame = splayer->_frame;
		if (_vm->shouldQuit()) return kLevelQuit;

		{
			int local_8 = 0;
			uint16 waveSelect = processWaveEnd(0x0e, &budget, 0x14, 0);

			while (waveSelect != 0xFFFF) {
				if (_vm->shouldQuit()) return kLevelQuit;

				if (local_8 < 4) {
					local_8 = _vm->_rnd.getRandomNumber(5);  // 0-5
				} else {
					local_8 = _vm->_rnd.getRandomNumber(3);  // 0-3
				}

				const char *filename;
				switch (local_8) {
				case 0:  filename = "LEV12/P4/12P04_C.SAN"; break;
				case 1:  filename = "LEV12/P4/12P04_D.SAN"; break;
				case 2:  filename = "LEV12/P4/12P04_A.SAN"; break;
				case 3:  filename = "LEV12/P4/12P04_B.SAN"; break;
				case 4:  filename = "LEV12/P4/12P04_E.SAN"; break;
				default: filename = "LEV12/P4/12P04_F.SAN"; break;
				}

				debug("Rebel2: Level 12 Phase 4 wave - %s (state=0x%x local_8=%d)", filename, _rebelPhaseState, local_8);
				splayer->setCurVideoFlags(0x428);
				splayer->play(filename, 12);
				_deathFrame = splayer->_frame;

				waveSelect = processWaveEnd(0x0e, &budget, 0x14, 0);
			}
		}

		if (_playerDamage >= 255) goto level12_death;
		if (_vm->shouldQuit()) return kLevelQuit;

		// ===== CLOSING: 12P09.SAN =====
		splayer->setCurVideoFlags(0x428);
		splayer->play("LEV12/12P09.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;
		processWaveEnd(1, &budget, 0, 0);

		// ===== LEVEL COMPLETED =====
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

		if (_vm->shouldQuit()) return kLevelQuit;
		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(12);
			return kLevelGameOver;
		}
		playCinematic("LEV12/12RETRY.SAN");
		_playerDamage = 0;
		if (_vm->shouldQuit()) return kLevelQuit;
		continue;  // Restart from Phase 1
	}

	return kLevelQuit;
}

// =============================================================================
// Level 13 Handler - FUN_0041B3E1
// "Escaping the Star Destroyer" - Two-phase flight/escape
// Phase A: 13PLAY_A.SAN (main flight), transitions to Phase B at maxFrame-10
// Phase B: 13PLAY_B.SAN (reactor loop, flags 0x468) — plays until
//   (DAT_0047ab90 == 0 && DAT_0047ab7c == 0) meaning all targets destroyed.
// Death: frame-based (A/B/C/B/D pattern)
// =============================================================================

int InsaneRebel2::runLevel13() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// No cutscene — starts directly with BEG
	// Original: FUN_004171c5("13BEG.SAN", 0x20, 0xb6, 0xa0, 10, 2, 0x46)
	playLevelBegin(13);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(13);
			return kLevelGameOver;
		}

		playCinematic("LEV13/13RETRY.SAN");
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

// =============================================================================
// Level 14 Handler - FUN_0041B6E8
// "TIE Attack" - Final space battle (single phase)
// No cutscene. Single death video (14DIE.SAN, no variants).
// =============================================================================

int InsaneRebel2::runLevel14() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// No cutscene — starts directly with BEG
	// Original: FUN_004171c5("14BEG.SAN", 0x20, 0xb7, 0xa0, 10, 2, 0x46)
	playLevelBegin(14);
	if (_vm->shouldQuit()) return kLevelQuit;

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

		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(14);
			return kLevelGameOver;
		}

		playCinematic("LEV14/14RETRY.SAN");
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

// =============================================================================
// Level 15 Handler - FUN_0041B8D7
// "Imdaar Alpha" - Final mission (single long phase with level ID switch)
// Has cutscene. Mid-level: DAT_0047a7f8 changes from 0xf to 0x10 at frame 0x21e.
// This represents a transition from the tunnel section to the core section.
// Death: frame-based (A/B/C/B/C/B/D pattern with 7 thresholds)
// On completion → FUN_0041BBE8 (credits/end game, not a playable level)
// =============================================================================

int InsaneRebel2::runLevel15() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Play cutscene (15CUT.SAN)
	playCinematic("LEV15/15CUT.SAN");
	if (_vm->shouldQuit()) return kLevelQuit;

	// Play level beginning cinematic (15BEG.SAN)
	// Original: FUN_004171c5("15BEG.SAN", 0x20, 0xb8, 0xa0, 10, 2, 0x46)
	playLevelBegin(15);
	if (_vm->shouldQuit()) return kLevelQuit;

	// FUN_00401000 + FUN_0041c7d0 + FUN_0040c040
	clearBit(0);

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_deathFrame = 0;

		clearBit(0);

		// Original: DAT_0047a7f8 = 0xf (level 15) before gameplay
		// At frame 0x21e (542): DAT_0047a7f8 = 0x10 (switches to "level 16" internally)
		// After gameplay: DAT_0047a7f8 = 0x10 (stays at 16)
		// This level ID switch affects which difficulty data is used mid-level.
		// The IACT callbacks handle gameplay regardless of this ID.

		// Play gameplay (15PLAY.SAN)
		// Original: FUN_0041f4d0("15PLAY.SAN", 0x28, -1, -1, 0)
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV15/15PLAY.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit()) return kLevelQuit;

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
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(15);
			return kLevelGameOver;
		}

		playCinematic("LEV15/15RETRY.SAN");
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

} // End of namespace Scumm
