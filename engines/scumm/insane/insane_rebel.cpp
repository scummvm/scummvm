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
#include "common/events.h"
#include "common/savefile.h"
#include "common/util.h"

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

#include "common/config-manager.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Scumm {


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

	// Initialize edge blend table with default identity table (FUN_404BCE -> FUN_410510(NULL))
	// Per-level tables are loaded later via IACT opcode 8 par4=1000
	initEdgeTable(nullptr);
	// DAT_0047a7fc: Controls edge highlight rendering and widescreen features.
	// Set from param_10 of FUN_403BD0 (main game init). Values:
	//   < 0: Edge highlights disabled (low-detail mode)
	//   >= 0: Edge highlights enabled, >= 1: high-detail (secondary NUTs, widescreen)
	// Always use high detail in ScummVM.
	_rebelDetailMode = 1;
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

	// Text overlay state (FUN_004171c5 chapter title rendering)
	_textOverlayActive = false;
	_textOverlayID = 0;
	_textOverlayX = 0;
	_textOverlayY = 0;
	_textOverlayFadeIn = 0;
	_textOverlayFadeOut = 0;

	// Retail globals mapped: hit counter, cooldown, invulnerability flag
	_rebelOp6Initialized = false;
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

	for (i = 0; i < 0x401; i++)
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

	// Initialize auxiliary sound buffers (4 × 30000 bytes, loaded from IACT stream)
	for (i = 0; i < kRA2NumAuxSfx; i++) {
		_auxSfxData[i] = (byte *)calloc(kRA2AuxBufSize, 1);
		_auxSfxSize[i] = 0;
	}

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
	_menuSelectionConfirmed = false;
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
	_debugUnlockAll = ConfMan.getBool("rebel2_unlock_all");

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

	// Initialize pilot data system
	_numPilots = 0;
	_activePilot = 0;
	for (i = 0; i < kMaxPilots; i++) {
		_pilots[i].init();
	}
	loadPilots(); // Load saved pilots from disk

	// Initialize pilot selection system (FUN_00414A41)
	// Menu structure: [saved pilots] + 4 fixed options (NEW/DUPE/DELETE/MAIN MENU)
	_levelSelection = 0;          // First item selected
	_levelItemCount = _numPilots + 4; // N saved pilots + 4 fixed options
	_selectedLevel = 1;           // Default selected level
	_difficultySelection = 2;     // Default to 3rd difficulty (matching original init param_3=2)
	_pilotMenuMode = kPilotModeSelect;
	_pilotNameInput = "";
	_pilotEditIndex = -1;

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
			// - During gameplay: Pause and open ScummVM menu
			// - During cutscenes/intros: Skip video
			if (splayer) {
				if (_menuInputActive && (_gameState == kStateMainMenu ||
				                          _gameState == kStatePilotSelect ||
				                          _gameState == kStateDifficultySelect ||
				                          _gameState == kStateChapterSelect)) {
					// In menu mode: Select quit option and confirm selection
					// This emulates the assembly behavior from FUN_0041f5ae
					_menuSelection = _menuItemCount - 1;  // Select last item (quit/back)
					_menuSelectionConfirmed = true;
					debug("Rebel2: ESC pressed in menu - selecting quit (item %d)", _menuSelection);
					_vm->_smushVideoShouldFinish = true;
				} else if (_gameState == kStateGameplay && _rebelHandler != 0) {
					// During active gameplay (handler != 0): pause and open ScummVM menu.
					// _rebelHandler is non-zero (7, 8, 0x19, 0x26) only during interactive
					// gameplay sections, and 0 during intro/cutscene/post videos within a level.
					debug("Rebel2: ESC pressed during gameplay - opening ScummVM menu");
					bool wasPaused = splayer->_paused;
					if (!wasPaused)
						splayer->pause();
					_vm->openMainMenuDialog();
					if (!wasPaused)
						splayer->unpause();
				} else {
					// During cutscenes/intros/mission briefings: skip video
					debug("Rebel2: ESC pressed - skipping video");
					_vm->_smushVideoShouldFinish = true;
				}
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

// Per-level difficulty parameters extracted from RA2WIN95.EXE at VA 0x47e0f0
// Original: 2D table indexed by DAT_0047a7fa (difficulty) * 0x242 + DAT_0047a7f8 (levelType) * 0x22
// 17 fields per entry (34 bytes), 17 entries per difficulty, 5 difficulties
// Field names from official Difficulty Editor: {laserDelay, snapDistance, missDamage, dodgeDamage,
//   shotDamage, specialDamage, shotAccuracy, hitPoints, dodgePoints, timePoints,
//   levelPoints, specialPoints, flags, rollRate, liftRate, slideRate, driftRate}
// -1 = not applicable for this level type
const InsaneRebel2::LevelDifficultyParams InsaneRebel2::kDifficultyTable[5][17] = {
	// Difficulty 0 (Beginner) - 17 level types
	{
		{   5,    3,   15,   -1,    2,   -1,   75,   25,   -1,    2,  500,  250,    8,    5,    5,    6,   -1}, // Lv1
		{   4,    3,   -1,   -1,    2,   -1,   40,   25,   -1,    0,  500,  250,    8,   90,   90,  120,   25}, // Lv2
		{   6,    5,   15,   30,    3,   12,   75,   25,   50,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    3,   18,   17,    2,   20,   75,   25,   50,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv4
		{   5,    4,   18,   -1,    3,   -1,   75,   25,   -1,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv5
		{   5,    5,   -1,   19,    2,   15,   75,   25,   50,    2,  500,   -1,    8,   -1,   -1,   -1,   -1}, // Lv6A
		{   5,    5,  180,   27,    3,   -1,   75,   25,   50,    2,  500,  250,    8,  120,  120,  120,   75}, // Lv6B
		{  -1,   -1,   -1,   40,   -1,   -1,   -1,   -1,   50,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    5,   21,   -1,    3,   -1,   50,   25,   -1,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    6,   15,   30,    4,   -1,   60,   25,   50,    2,  500,  250,    8,   90,   90,   90,  135}, // Lv9
		{   5,   15,   -1,   30,   70,   -1,   75,   25,   50,    2,  500,  250,    8,   10,    6,    7,   -1}, // Lv10
		{   4,    4,   10,   -1,    5,   -1,   65,   25,   -1,    0,  500,  250,    8,    5,    6,    7,    8}, // Lv11
		{   4,    2,   -1,   -1,    6,   -1,   65,   25,   -1,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    6,   20,   30,    1,   20,   85,   25,   50,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    3,   24,   -1,    2,   -1,   75,   25,   -1,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    8,   -1,   -1,    3,   -1,   75,   25,   -1,    2,  500,   -1,    8,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    6,  255,   30,    4,   10,   75,   25,   50,    2,  500,  250,    8,   -1,   -1,   -1,   -1}, // Lv15B
	},
	// Difficulty 1 (Easy) - 17 level types
	{
		{   6,    1,   25,   -1,    3,   -1,   75,   50,   -1,    4, 1000,  500,   16,    6,    6,    7,   -1}, // Lv1
		{   4,    2,   -1,   -1,    4,   -1,   40,   50,   -1,    0, 1000,  500,   16,  100,  100,  135,   30}, // Lv2
		{   6,    4,   17,   35,    5,   12,   75,   50,  100,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    2,   60,   30,    4,   20,   75,   50,  100,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv4
		{   5,    1,   28,   -1,    3,   -1,   75,   50,   -1,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv5
		{   5,    2,   -1,   25,    4,   15,   75,   50,  100,    4, 1000,   -1,   16,   -1,   -1,   -1,   -1}, // Lv6A
		{   5,    2,  190,   35,    4,   -1,   75,   50,  100,    4, 1000,  500,   16,  140,  140,  140,   90}, // Lv6B
		{  -1,   -1,   -1,   65,   -1,   -1,   -1,   -1,  100,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    3,   24,   -1,    3,   -1,   50,   50,   -1,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    4,   17,   45,    5,   -1,   75,   50,  100,    4, 1000,  500,   16,  100,  100,  100,  140}, // Lv9
		{   5,   12,   -1,   35,   75,   -1,   75,   50,  100,    4, 1000,  500,   16,   10,    6,    7,   -1}, // Lv10
		{   4,    2,   15,   -1,    5,   -1,   75,   50,   -1,    0, 1000,  500,   16,    5,    6,    7,    8}, // Lv11
		{   4,    1,   -1,   -1,    8,   -1,   75,   50,   -1,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    5,   30,   35,    1,   20,   85,   50,  100,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    2,   28,   -1,    2,   -1,   75,   50,   -1,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    7,   -1,   -1,    4,   -1,   75,   50,   -1,    4, 1000,   -1,   16,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    6,  255,   35,    4,   10,   75,   50,  100,    4, 1000,  500,   16,   -1,   -1,   -1,   -1}, // Lv15B
	},
	// Difficulty 2 (Medium) - 17 level types
	{
		{   7,    0,   35,   -1,    5,   -1,   75,   75,   -1,    6, 1500,  750,    0,    7,    7,    8,   -1}, // Lv1
		{   4,    1,   -1,   -1,    6,   -1,   40,   75,   -1,    0, 1500,  750,    0,  110,  110,  150,   35}, // Lv2
		{   6,    1,   20,   38,    7,   12,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    1,  100,   35,    6,   20,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv4
		{   6,    1,   30,   -1,    4,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv5
		{   6,    1,   -1,   30,    6,   15,   75,   75,  150,    6, 1500,   -1,    0,   -1,   -1,   -1,   -1}, // Lv6A
		{   6,    1,  200,   50,   12,   -1,   75,   75,  150,    6, 1500,  750,    0,  160,  160,  160,  105}, // Lv6B
		{  -1,   -1,   -1,   80,   -1,   -1,   -1,   -1,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    1,   27,   -1,    3,   -1,   60,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    3,   19,   60,    7,   -1,   75,   75,  150,    6, 1500,  750,    0,  110,  110,  110,  150}, // Lv9
		{   5,    9,   -1,   40,  100,   -1,   85,   75,  150,    6, 1500,  750,    0,   11,    7,    8,   -1}, // Lv10
		{   4,    1,   20,   -1,    6,   -1,   75,   75,   -1,    0, 1500,  750,    0,    6,    7,    8,    9}, // Lv11
		{   4,    0,   -1,   -1,   11,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    3,   40,   40,    3,   15,   76,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    0,   38,   -1,    4,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    4,   -1,   -1,    7,   -1,   75,   75,   -1,    6, 1500,   -1,    0,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    5,  255,   40,    7,   10,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv15B
	},
	// Difficulty 3 (Hard) - 17 level types
	{
		{   8,    0,   77,   -1,    7,   -1,   80,  100,   -1,    8, 2000, 1000,    4,    8,    8,    9,   -1}, // Lv1
		{   4,    0,   -1,   -1,    7,   -1,   40,  100,   -1,    0, 2000, 1000,    4,  120,  120,  165,   40}, // Lv2
		{   6,    0,   23,   42,   12,   10,   75,  100,  200,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    0,  120,   50,   16,   10,   75,  100,  200,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv4
		{   5,    0,   55,   -1,    4,   -1,   75,  100,   -1,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv5
		{   6,    0,    0,   50,   15,   15,   79,  100,  200,    8, 2000,   -1,    4,   -1,   -1,   -1,   -1}, // Lv6A
		{   6,    0,  220,   90,   15,   -1,   90,  100,  200,    8, 2000, 1000,    4,  180,  180,  180,  140}, // Lv6B
		{  -1,   -1,   -1,   90,   -1,   -1,   -1,   -1,  200,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    0,   35,   -1,    3,   -1,   68,  100,   -1,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    2,   30,   75,   20,   -1,   80,  100,  200,    8, 2000, 1000,    4,  120,  120,  120,  200}, // Lv9
		{   5,    8,   -1,   50,  110,   -1,   90,  100,  200,    8, 2000, 1000,    4,   12,    8,    9,   -1}, // Lv10
		{   4,    0,   30,   -1,    7,   -1,   75,  100,   -1,    0, 2000, 1000,    4,    7,    8,    9,   10}, // Lv11
		{   4,    0,   -1,   -1,   13,   -1,   75,  100,   -1,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    3,   55,   55,    5,   12,   77,  100,  200,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    0,   49,   -1,    4,   -1,   75,  100,   -1,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    4,   -1,   -1,   10,   -1,   79,  100,   -1,    8, 2000,   -1,    4,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    4,  255,   45,    8,    5,   80,  100,  200,    8, 2000, 1000,    4,   -1,   -1,   -1,   -1}, // Lv15B
	},
	// Difficulty 4 (Jedi) — identical to difficulty 2 (Medium) in original data
	{
		{   7,    0,   35,   -1,    5,   -1,   75,   75,   -1,    6, 1500,  750,    0,    7,    7,    8,   -1}, // Lv1
		{   4,    1,   -1,   -1,    6,   -1,   40,   75,   -1,    0, 1500,  750,    0,  110,  110,  150,   35}, // Lv2
		{   6,    1,   20,   38,    7,   12,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv3
		{   5,    1,  100,   35,    6,   20,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv4
		{   6,    1,   30,   -1,    4,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv5
		{   6,    1,   -1,   30,    6,   15,   75,   75,  150,    6, 1500,   -1,    0,   -1,   -1,   -1,   -1}, // Lv6A
		{   6,    1,  200,   50,   12,   -1,   75,   75,  150,    6, 1500,  750,    0,  160,  160,  160,  105}, // Lv6B
		{  -1,   -1,   -1,   80,   -1,   -1,   -1,   -1,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv7
		{   5,    1,   27,   -1,    3,   -1,   60,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv8
		{   5,    3,   19,   60,    7,   -1,   75,   75,  150,    6, 1500,  750,    0,  110,  110,  110,  150}, // Lv9
		{   5,    9,   -1,   40,  100,   -1,   85,   75,  150,    6, 1500,  750,    0,   11,    7,    8,   -1}, // Lv10
		{   4,    1,   20,   -1,    6,   -1,   75,   75,   -1,    0, 1500,  750,    0,    6,    7,    8,    9}, // Lv11
		{   4,    0,   -1,   -1,   11,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv12
		{   5,    3,   40,   40,    3,   15,   76,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv13
		{   5,    0,   38,   -1,    4,   -1,   75,   75,   -1,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv14
		{   5,    4,   -1,   -1,    7,   -1,   75,   75,   -1,    6, 1500,   -1,    0,   -1,   -1,   -1,   -1}, // Lv15A
		{   5,    5,  255,   40,    7,   10,   75,   75,  150,    6, 1500,  750,    0,   -1,   -1,   -1,   -1}, // Lv15B
	},
};

InsaneRebel2::LevelDifficultyParams InsaneRebel2::getDifficultyParams() const {
	int diff = CLIP(_difficulty, 0, 4);
	int lvIdx = CLIP((int)_rebelLevelType, 0, 16);
	return kDifficultyTable[diff][lvIdx];
}

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

// Render score text to HUD (part of FUN_0041c012)
// FUN_0041c012 lines 133-137: calls FUN_00434cb0 with format "%07ld"
// Position (low-res): X = 0x101 (257), Y = 4 within status bar → screen Y = 184
void InsaneRebel2::renderScoreHUD(byte *renderBitmap, int pitch, int width, int height, int statusBarY) {
	(void)statusBarY;

	if (!_smush_dispfontNut)
		return;

	char scoreStr[16];
	Common::sprintf_s(scoreStr, "%07d", _playerScore);

	// Score position from FUN_0041c012 assembly (low-res mode):
	//   X = ((DAT_0047a808 < 2) - 1 & 0x101) + 0x101 = 0x101 = 257
	//   Y = ((DAT_0047a808 < 2) - 1 & 4) + 4 = 4 (within status bar at Y=180)
	int scoreX = 257 + _viewX;
	int scoreY = 180 + 4 + _viewY;

	// Render each digit as a NUT sprite (direct pixel blit with color 0 transparency).
	// This matches the original's FUN_00434cb0 → FUN_004341a0 text rendering which
	// uses the NUT font's embedded palette colors (1=white, 3=gray, 4=black outline).
	// Render each character applying xoffs/yoffs from NUT frame headers,
	// matching FUN_0042cba0 lines 13-14:
	//   param_3 = *(short *)(param_6 + 2) + param_3;  // X += xoffs
	//   param_4 = *(short *)(param_6 + 4) + param_4;  // Y += yoffs
	int x = scoreX;
	for (int i = 0; scoreStr[i] != '\0'; i++) {
		byte ch = (byte)scoreStr[i];
		if (ch < _smush_dispfontNut->getNumChars()) {
			int charX = x + _smush_dispfontNut->getCharXOffset(ch);
			int charY = scoreY + _smush_dispfontNut->getCharYOffset(ch);
			renderNutSprite(renderBitmap, pitch, width, height, charX, charY, _smush_dispfontNut, ch);
			x += _smush_dispfontNut->getCharWidth(ch);
		}
	}
}

// ======================= Pilot Data System =======================
// Save/load pilot profiles using ScummVM's save file system (one pilot per slot).
// Follows the Hypno/Wetlands pattern: each pilot = one ScummVM save slot.
// Uses ScummEngine::makeSavegameName() for standard ScummVM file naming.
// Original: FUN_00411980 (load) / FUN_00411A5D (save)

static const uint32 kPilotSaveMagic = MKTAG('R', 'A', '2', 'P');
static const uint16 kPilotSaveVersion = 2;

bool InsaneRebel2::loadPilots() {
	_numPilots = 0;

	for (int i = 0; i < kMaxPilots; i++) {
		Common::String filename = _vm->makeSavegameName(i, false);
		Common::InSaveFile *sf = _vm->_saveFileMan->openForLoading(filename);
		if (!sf)
			break; // Slots are contiguous

		uint32 magic = sf->readUint32BE();
		if (magic != kPilotSaveMagic) {
			delete sf;
			break;
		}

		uint16 version = sf->readUint16LE();

		PilotData &p = _pilots[i];
		sf->read(p.name, kMaxPilotNameLen + 1);
		p.name[kMaxPilotNameLen] = '\0';
		for (int j = 0; j < kNumLevels; j++)
			p.score[j] = sf->readSint32LE();
		for (int j = 0; j < kNumLevels; j++)
			p.lives[j] = sf->readSint32LE();
		for (int j = 0; j < kNumLevels; j++)
			p.damage[j] = sf->readSint32LE();
		p.difficulty = sf->readSint16LE();
		if (version >= 2) {
			for (int j = 0; j < kNumLevels; j++)
				p.rating[j] = sf->readSint16LE();
		}
		delete sf;

		_numPilots = i + 1;
	}

	debug("Rebel2: Loaded %d pilot(s)", _numPilots);
	return _numPilots > 0;
}

bool InsaneRebel2::savePilots() {
	bool ok = true;

	for (int i = 0; i < _numPilots; i++) {
		Common::String filename = _vm->makeSavegameName(i, false);
		Common::OutSaveFile *sf = _vm->_saveFileMan->openForSaving(filename, false);
		if (!sf) {
			warning("Rebel2: Failed to save pilot %d", i);
			ok = false;
			continue;
		}

		sf->writeUint32BE(kPilotSaveMagic);
		sf->writeUint16LE(kPilotSaveVersion);

		const PilotData &p = _pilots[i];
		sf->write(p.name, kMaxPilotNameLen + 1);
		for (int j = 0; j < kNumLevels; j++)
			sf->writeSint32LE(p.score[j]);
		for (int j = 0; j < kNumLevels; j++)
			sf->writeSint32LE(p.lives[j]);
		for (int j = 0; j < kNumLevels; j++)
			sf->writeSint32LE(p.damage[j]);
		sf->writeSint16LE(p.difficulty);
		for (int j = 0; j < kNumLevels; j++)
			sf->writeSint16LE(p.rating[j]);

		sf->finalize();
		delete sf;
	}

	// Remove leftover files beyond current count
	for (int i = _numPilots; i < kMaxPilots; i++) {
		Common::String filename = _vm->makeSavegameName(i, false);
		_vm->_saveFileMan->removeSavefile(filename);
	}

	debug("Rebel2: Saved %d pilot(s)", _numPilots);
	return ok;
}

int InsaneRebel2::createNewPilot() {
	// FUN_00411B9A: Create new pilot slot
	if (_numPilots >= kMaxPilots)
		return -1;

	int idx = _numPilots;
	_pilots[idx].init();
	_numPilots++;
	return idx;
}

void InsaneRebel2::deletePilot(int index) {
	// FUN_00411D29: Delete pilot and shift remaining down
	if (index < 0 || index >= _numPilots)
		return;

	for (int i = index; i < _numPilots - 1; i++) {
		_pilots[i] = _pilots[i + 1];
	}
	_numPilots--;

	// Clear the now-unused last slot
	_pilots[_numPilots].init();
}

void InsaneRebel2::copyPilot(int srcIndex) {
	// Copy pilot from srcIndex to a new slot
	if (srcIndex < 0 || srcIndex >= _numPilots || _numPilots >= kMaxPilots)
		return;

	int newIdx = _numPilots;
	_pilots[newIdx] = _pilots[srcIndex];

	// Append " COPY" or truncate name to fit
	Common::String name(_pilots[newIdx].name);
	if (name.size() + 5 <= kMaxPilotNameLen) {
		name += " COPY";
	} else if (name.size() > 0) {
		// Truncate and add marker
		name = name.substr(0, kMaxPilotNameLen - 2) + " C";
	}
	Common::strlcpy(_pilots[newIdx].name, name.c_str(), sizeof(_pilots[newIdx].name));

	_numPilots++;
}

void InsaneRebel2::updatePilotProgress(int levelIndex, int32 score, int32 lives, int32 damage) {
	if (_activePilot < 0 || _activePilot >= _numPilots)
		return;
	if (levelIndex < 0 || levelIndex >= kNumLevels)
		return;

	PilotData &pilot = _pilots[_activePilot];
	pilot.score[levelIndex] = score;
	pilot.lives[levelIndex] = lives;
	pilot.damage[levelIndex] = damage;

	// Unlock next level if this one completed with damage < 0xFF
	if (damage < 0xFF && levelIndex + 1 < kNumLevels) {
		if (pilot.damage[levelIndex + 1] == 0xFF) {
			// Initialize next level as playable
			pilot.score[levelIndex + 1] = 0;
			pilot.lives[levelIndex + 1] = 4;
			pilot.damage[levelIndex + 1] = 0;
		}
	}

	savePilots();
}

int InsaneRebel2::getPilotHighestLevel() const {
	if (_activePilot < 0 || _activePilot >= _numPilots)
		return 0;

	const PilotData &pilot = _pilots[_activePilot];
	int highest = 0;
	for (int i = kNumLevels - 1; i >= 0; i--) {
		if (pilot.damage[i] < 0xFF) {
			highest = i;
			break;
		}
	}
	return highest;
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

				// Spawn visual explosion based on handler, enemy type, and flags.
				//
				// Rendering functions (FUN_409FBC, FUN_402696, FUN_40F1C5,
				// FUN_41F29A) check DAT_0047e108 flags & 1 — when set,
				// explosion NUT sprites are suppressed. This is checked
				// during rendering in renderExplosions().
				//
				// Handler 8 (FUN_4028C5): Only type 0 spawns explosion.
				// Handler 25 (FUN_41E7C2): Types > 3 set counter but
				// rendering suppressed by flags bit 0.
				// Handlers 0x26, 7: All types get visual explosions.
				if (_rebelHandler != 8 && _rebelHandler != 25) {
					spawnExplosion((it->rect.left + it->rect.right) / 2,
								   (it->rect.top + it->rect.bottom) / 2,
								   it->rect.width() / 2);
				} else if (_rebelHandler == 8 && it->type == 0) {
					spawnExplosion((it->rect.left + it->rect.right) / 2,
								   (it->rect.top + it->rect.bottom) / 2,
								   it->rect.width() / 2);
				} else if (_rebelHandler == 25 && it->type > 3) {
					// Counter is set for timing/sound, but rendering
					// may be suppressed by flags bit 0
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

				// Play enemy death sound.
				// Pan based on enemy center X position: (screenX - 160) mapped to [-127,127]
				{
					int enemyCenterX = (it->rect.left + it->rect.right) / 2 - _viewX;
					int sfxPan = CLIP((enemyCenterX - 160) * 127 / 160, -127, 127);
					if (_rebelHandler == 8 && it->type >= 1 && it->type <= 4) {
						// Handler 8 soldier types 1-4: play from auxiliary buffer 0
						// Original: FUN_00411931(0, slot+3, 0x7f, pan, 0)
						playAuxSfx(0, 127, sfxPan);
					} else {
						// All other enemies: EXPLODE.SAD
						// Original: FUN_0041189e(2, slot+3, 0x7f, pan, 0)
						playSfx(2, 127, sfxPan);
					}
				}

				// Award score for destroying enemy (FUN_0041bf8d called from FUN_40A2E0)
				// Score value comes from DAT_0047e0fe indexed by difficulty×level
				{
					LevelDifficultyParams dparams = getDifficultyParams();
					if (dparams.hitPoints > 0) {
						addScore(dparams.hitPoints);
					}
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
	assert (n < 0x401);

	return (_iactBits[n] != 0);
}

void InsaneRebel2::setBit(int n) {
	// FUN_004239b0: When n < 1 (i.e., n == 0 or negative), set ALL bits to 1 (disable all objects)
	// This is used to disable all enemies/objects at once
	if (n < 1) {
		for (int i = 0; i < 0x401; i++)
			_iactBits[i] = 1;
		return;
	}
	assert (n < 0x401);
	_iactBits[n] = 1;
}

void InsaneRebel2::clearBit(int n) {
	// FUN_00423a00: When n < 1 (i.e., n == 0 or negative), clear ALL bits to 0 (enable all objects)
	// This is called by FUN_00423880 at the start of video playback to reset the bit table,
	// ensuring all enemies are visible when a new level/segment starts.
	if (n < 1) {
		for (int i = 0; i < 0x401; i++)
			_iactBits[i] = 0;
		return;
	}
	assert (n < 0x401);
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


} // End of namespace Scumm
