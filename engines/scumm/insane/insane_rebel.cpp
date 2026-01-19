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

	// Retail globals mapped: hit counter, cooldown, invulnerability flag
	_rebelHitCounter = 0;
	_rebelHitCooldown = 0;
	_rebelInvulnerable = false;

	// Opcode 6 state variables
	_rebelAutopilot = 0;
	_rebelDamageLevel = 0;
	_rebelFlightDir = 0;
	_rebelControlMode = 0;
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
	_corridorLeftX = 0;
	_corridorTopY = 0;
	_corridorRightX = 320;
	_corridorBottomY = 200;
	_hitCooldown = 0;

	for (i = 0; i < 2; i++) {
		_shots[i].active = false;
		_shots[i].counter = 0;
	}

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
	_flyControlMode = 0;   // DAT_004437c0 - Start in flight-only mode (no shooting)
	_shipFiring = false;
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

	// Initialize Handler 0x26 turret HUD overlay system
	_hudOverlayNut = nullptr;    // DAT_0047fe78 - Primary HUD overlay (GRD files, animated)
	_hudOverlay2Nut = nullptr;   // DAT_0047fe80 - Secondary HUD overlay

	// Initialize audio system for RA2 (since we don't use iMUSE)
	_audioSampleRate = 11025;  // RA2 audio is 11025 Hz, not 22050 Hz
	for (i = 0; i < kRA2MaxAudioTracks; i++) {
		_audioStreams[i] = nullptr;
		_audioTrackActive[i] = false;
	}

	// Initialize menu system
	_gameState = kStateMainMenu;  // Start at main menu
	_menuSelection = 0;           // First item selected
	// Main menu has 6 selectable items (0-5): START, OPTIONS, CONTINUE INTRO, TOP PILOTS, CREDITS, QUIT
	// Note: The coordinate formula uses param_3 = 7 (includes title) for Y position calculation
	// but _menuItemCount is the number of SELECTABLE items for bounds checking
	_menuItemCount = 6;
	_menuInactivityTimer = 0;
	_lastMenuVariant = -1;        // No previous menu video
	_menuRepeatDelay = 0;
	for (i = 0; i < 16; i++) {
		_levelUnlocked[i] = (i == 0);  // Only level 1 unlocked initially
	}

	// Initialize level selection system
	_levelSelection = 0;          // First level selected
	_levelItemCount = 2;          // Level 1 + MAIN MENU (will grow as more levels implemented)
	_selectedLevel = 1;           // Default selected level

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

	// Clean up Handler 0x26 turret HUD overlays
	delete _hudOverlayNut;
	delete _hudOverlay2Nut;

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
			// ESC skips videos
			if (splayer) {
				debug("Rebel2: ESC pressed - skipping video");
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
	bool wasPressed = false;
	bool isPressed = (_vm->_system->getEventManager()->getButtonState() & 1) != 0;
	
	// Edge detection: only trigger on button press (not hold)
	if (isPressed && !wasPressed) {
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
				debug("Rebel2: HIT enemy ID=%d at (%d,%d) - Rect: (%d,%d)-(%d,%d)", 
					it->id, mousePos.x, mousePos.y,
					it->rect.left, it->rect.top, it->rect.right, it->rect.bottom);

				// Spawn explosion using native system
				// Use width / 2 as the scale parameter
				spawnExplosion((it->rect.left + it->rect.right) / 2, 
							   (it->rect.top + it->rect.bottom) / 2, 
							   it->rect.width() / 2);

				// Disable self
				setBit(it->id);

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

				// Note: Background saving and masking is handled in procPostRendering
				// where we have access to the render bitmap
				// TODO: Play explosion sound

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
	wasPressed = isPressed;
	return buttons;
}

bool InsaneRebel2::shouldSkipFrameUpdate(int left, int top, int width, int height) {
	// Only check for Rebel2
	if (_vm->_game.id != GID_REBEL2) {
		return false;
	}
	
	Common::Rect updateRect(left, top, left + width, top + height);
	int updateArea = width * height;
	
	// Check if this update region significantly overlaps with any destroyed enemy
	Common::List<enemy>::iterator it;
	for (it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->destroyed) {
			// Calculate the intersection of the update rect and enemy rect
			Common::Rect enemyRect = it->rect;
			
			if (updateRect.intersects(enemyRect)) {
				// Calculate the intersection area
				int intLeft = MAX(updateRect.left, enemyRect.left);
				int intTop = MAX(updateRect.top, enemyRect.top);
				int intRight = MIN(updateRect.right, enemyRect.right);
				int intBottom = MIN(updateRect.bottom, enemyRect.bottom);
				int intArea = (intRight - intLeft) * (intBottom - intTop);
				
				// Require at least 70% overlap to skip the update
				// This prevents unrelated frame updates from being incorrectly skipped
				if (intArea * 100 >= updateArea * 70) {
					debug("Rebel2: Skipping frame update (%d,%d %dx%d) - %d%% overlap with destroyed enemy ID=%d",
						left, top, width, height, (intArea * 100) / updateArea, it->id);
					return true;
				}
			}
		}
	}
	
	return false;
}

bool InsaneRebel2::isBitSet(int n) {
	assert (n < 0x200);

	return (_iactBits[n] != 0);
}

void InsaneRebel2::setBit(int n) {
	assert (n < 0x200);

	_iactBits[n] = 1;
}

void InsaneRebel2::clearBit(int n) {
	assert (n < 0x200);

	_iactBits[n] = 0;
}

void InsaneRebel2::procSKIP(int32 subSize, Common::SeekableReadStream &b) {
	// Rebel Assault 2 does NOT use Full Throttle's conditional frame skip mechanism.
	// The base Insane::procSKIP() uses _iactBits to decide whether to skip frame objects,
	// but RA2 handles conditional content differently through IACT opcodes 2/3/4.
	//
	// By overriding this to do nothing, we prevent random frame objects from being
	// skipped due to uninitialized _iactBits state.
	//
	// If RA2 SKIP chunks need to be handled, implement RA2-specific logic here.
	// For now, just consume the data without setting _skipNext.
	(void)subSize;
	(void)b;
}

void InsaneRebel2::procPreRendering(byte *renderBitmap) {
	// Call base class implementation first (handles Full Throttle state machine)
	Insane::procPreRendering(renderBitmap);

	// For Level 2 gameplay (Handler 8 or 25), restore the background BEFORE FOBJ decoding.
	// The tiny FOBJ sprites (7x10, 9x38 pixels) only draw new sprite positions but don't
	// clear old ones. By restoring the full background each frame, we ensure old sprite
	// positions are erased before new ones are drawn.
	//
	// This is called at the start of handleFrame(), before any FOBJ chunks are processed.
	if ((_rebelHandler == 8 || _rebelHandler == 25) && _level2BackgroundLoaded && _level2Background && renderBitmap) {
		for (int y = 0; y < 200; y++) {
			memcpy(renderBitmap + y * 320, _level2Background + y * 320, 320);
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
			registerCollisionZone(b, par2);
		}

	} else if (par1 == 7) {
		// Opcode 7: Sprite/HUD control for Handler 7 (space flight levels like Level 3)
		// par2 = control type (41 = sprite selection?)
		// par3 = usually 0
		// par4 = sprite/slot ID (0 or 5 seen in Level 3)
		//
		// This opcode may control which ship direction sprite to display
		// or reference embedded graphics loaded elsewhere
		debug("Rebel2 IACT Opcode 7: par2=%d par3=%d par4=%d handler=%d",
			par2, par3, par4, _rebelHandler);

		// Read remaining IACT data to understand structure
		int64 startPos = b.pos();
		int64 remaining = b.size() - startPos;
		if (remaining > 0 && remaining <= 64) {
			byte payload[64];
			int bytesRead = b.read(payload, MIN((int64)64, remaining));
			debug("Rebel2 Opcode 7: payload (%d bytes): %02X %02X %02X %02X %02X %02X %02X %02X",
				bytesRead,
				bytesRead > 0 ? payload[0] : 0, bytesRead > 1 ? payload[1] : 0,
				bytesRead > 2 ? payload[2] : 0, bytesRead > 3 ? payload[3] : 0,
				bytesRead > 4 ? payload[4] : 0, bytesRead > 5 ? payload[5] : 0,
				bytesRead > 6 ? payload[6] : 0, bytesRead > 7 ? payload[7] : 0);
			b.seek(startPos);
		}

		// par2 == 41 (0x29) seems to be a common value
		// This might be a "show sprite" command referencing par4 as the slot
		if (par2 == 41) {
			// par4 could be a HUD slot or sprite index
			// For Handler 7, set which embedded HUD frame to display
			if (_rebelHandler == 7 && par4 >= 0 && par4 < 16) {
				// Mark this slot as the active one for direction-based rendering
				// This will be used in post-rendering to know which frame to show
				debug("Rebel2 Opcode 7: Activating HUD slot %d for Handler 7", par4);
			}
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
		
		if (parentId >= 0 && parentId < 512) {
			// Shift links
			_rebelLinks[parentId][2] = _rebelLinks[parentId][1];
			_rebelLinks[parentId][1] = _rebelLinks[parentId][0];
			_rebelLinks[parentId][0] = childId;
			
			// Apply initial state based on parent state
			if (!isBitSet(parentId)) {
				setBit(childId);
				debug("Rebel2: Linked ID=%d to Parent=%d (Slot 0). Parent Alive -> Child Disabled.", childId, parentId);
			} else {
				clearBit(childId);
				debug("Rebel2: Linked ID=%d to Parent=%d (Slot 0). Parent Dead -> Child Enabled.", childId, parentId);
			}
		}
		return;
	} else if (par3 == 1) { // Probabilistic / counter cases: par3 == 1
		int16 value = par4; // sVar6
		int16 targetId = b.readSint16LE(); // Offset +8 (sVar7)
		
		if (targetId < 0 || targetId >= 0x200) 
			return;
		
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
	// Handle IACT opcode 3 subcases (damage, counters, special 100 branch)
	// Mirrors retail FUN_0041CADB case 1 behavior where possible.

	// Very small cooldown counter decremented on each IACT to emulate DAT_0045790a behavior
	if (_rebelHitCooldown > 0) _rebelHitCooldown--;

	// Subcase: par3 == 5 -> damage logic, expects extra param at +10 (source enemy ID)
	if (par3 == 5) {
		b.skip(2); // Offset +8
		int16 srcId = b.readSint16LE(); // Offset +10 (Enemy ID)

		// Only proceed if source is active (bit clear)
		if (!isBitSet(srcId)) {
			if (_rebelHitCooldown < 2) {
				int limit = 20 + _difficulty * 20; // heuristic mapping for probability table
				if (limit < 5) limit = 5;
				if (limit > 90) limit = 90;
				if (_vm->_rnd.getRandomNumber(100) < limit) {
					// Apply damage unless invulnerable flag set (DAT_0047ab64)
					if (!_rebelInvulnerable) {
						int damageAmount = 5 + (_difficulty * 2);
						// Apply to shields first (do not end game on depletion during tests)
	
	
						// Update the retail-like damage accumulator (DAT_0047a7ec equivalent)
						_playerDamage += damageAmount;
						if (_playerDamage > 255) _playerDamage = 255;
						debug("Rebel2: Damage HIT by Enemy %d. Damage=%d (limit=%d)", srcId, _playerDamage, limit);
						// TODO: call UI update / flash screen / play sound to match retail (FUN_00420515 / FUN_0041189e)
					}
					// Impose short cooldown to prevent immediate repeated damage
					_rebelHitCooldown = 6;
				}
			}
		}
	}
	// Subcase: par3 == 1 -> increment hit counter when source active and par4 != 4
	else if (par3 == 1) {
		b.skip(2); // read extra param (source id)
		int16 srcId = b.readSint16LE();
		if (!isBitSet(srcId) && par4 != 4) {
			_rebelHitCounter++;
			debug("Rebel2: Incremented hit counter DAT_0047ab80 -> %d (source=%d)", _rebelHitCounter, srcId);
		}
	}
	// Special-case branch when par2 == 100 (retail: triggers damage/sound via different offsets)
	else if (par2 == 100) {
		b.skip(2);
		int16 srcId = b.readSint16LE();
		if (!isBitSet(srcId)) {
			int limit = 20 + _difficulty * 20;
			if (_vm->_rnd.getRandomNumber(100) < limit) {
				if (!_rebelInvulnerable) {
					int damageAmount = 5 + (_difficulty * 2);
					// Increment the retail-like damage accumulator (DAT_0047a7ec equivalent)
					_playerDamage += damageAmount;
					if (_playerDamage > 255) _playerDamage = 255;
					debug("Rebel2: Damage HIT (special) by Enemy %d. Damage=%d (limit=%d)", srcId, _playerDamage, limit);
				}
				_rebelHitCooldown = 6;
			}
		}
	}
	// other subcases not implemented yet
}

void InsaneRebel2::iactRebel2Opcode6(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4) {
	// Opcode 6: Level setup / mode switch
	// Based on FUN_41CADB case 4 (switch on *local_14 - 2 == 4, meaning opcode 6)
	//
	// For Handler 8 (third-person vehicle) - FUN_00401234 case 4:
	// - par3 sets ship level mode (DAT_0043e000)
	// - par4 == 1 triggers status bar display and state reset
	// - Updates ship position based on mouse input
	//
	// For Handler 0x26/0x19 (turret/space):
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

	// Handler 8 specific logic (third-person vehicle) - FUN_00401234 case 4
	if (_rebelHandler == 8) {
		// Set ship level mode (DAT_0043e000 = par3)
		_shipLevelMode = par3;

		// If par4 == 1, enable status bar and reset state
		if (par4 == 1) {
			_rebelStatusBarSprite = 5;  // Status bar sprite for Handler 8
			// Reset link tables
			for (int i = 0; i < 512; i++) {
				_rebelLinks[i][0] = 0;
				_rebelLinks[i][1] = 0;
				_rebelLinks[i][2] = 0;
			}
			debug("Rebel2 Opcode 6 (Handler 8): Status bar enabled, state reset");
		}

		// Skip position calculation for special modes 4 and 5
		if (_shipLevelMode != 4 && _shipLevelMode != 5) {
			// Calculate target position from mouse input
			// Mouse X maps to ship horizontal tilt, Mouse Y to vertical tilt
			// Based on FUN_00401234 lines 151-166:
			// local_18 = ((DAT_0047a7e0 * 5 + 0x27b) * 0x40) / 0xfe
			// local_1c = ((DAT_0047a7e2 * 5 + 0x27b) * 0x10) / 0xfe

			// Map mouse position (-127 to 127 range) to ship target
			// Mouse is 0-320, center is 160. Map to -127 to 127 range
			int16 mouseOffsetX = (int16)((_vm->_mouse.x - 160) * 127 / 160);
			int16 mouseOffsetY = (int16)((_vm->_mouse.y - 100) * 127 / 100);

			// Clamp to valid range
			if (mouseOffsetX > 127) mouseOffsetX = 127;
			if (mouseOffsetX < -127) mouseOffsetX = -127;
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
		_shipFiring = (_vm->VAR(_vm->VAR_LEFTBTN_HOLD) != 0);

		debug("Rebel2 Opcode 6 (Handler 8): mode=%d shipPos=(%d,%d) target=(%d,%d) firing=%d dir=(%d,%d,%d)",
			_shipLevelMode, _shipPosX, _shipPosY, _shipTargetX, _shipTargetY, _shipFiring,
			_shipDirectionH, _shipDirectionV, _shipDirectionIndex);

		// Handler 8 doesn't use the same view offset logic as other handlers
		// Skip the rest of the function for Handler 8
		return;
	}

	// Handler 7 specific logic (space flight) - FUN_0040d836 / FUN_0040c3cc
	// Used for Level 3 and similar space combat levels
	if (_rebelHandler == 7) {
		// Set control mode (DAT_004437c0 = par3 in FUN_40C3CC case 4)
		// This determines shooting capability:
		//   Mode 0: Flight/avoid mode - no shooting
		//   Mode 1: Alternate flight mode - no shooting
		//   Mode 2: Combat mode - shooting ENABLED
		_flyControlMode = par3;
		debug("Rebel2 Opcode 6 (Handler 7): Control mode set to %d (shooting %s)",
			par3, (par3 == 2) ? "ENABLED" : "DISABLED");

		// If par4 == 1, enable status bar
		if (par4 == 1) {
			_rebelStatusBarSprite = 5;  // Status bar sprite
			debug("Rebel2 Opcode 6 (Handler 7): Status bar enabled");
		}

		// Update ship screen position from mouse
		// Handler 7 uses DAT_0044370c (Y) and DAT_0044370e (X) for screen position
		// Get raw mouse position
		int16 rawMouseX = _vm->_mouse.x;
		int16 rawMouseY = _vm->_mouse.y;

		// Scale mouse to 320x200 logical space if video is larger
		int16 mouseX = rawMouseX;
		int16 mouseY = rawMouseY;
		if (_player && _player->_width > 320) {
			mouseX = (rawMouseX * 320) / _player->_width;
		}
		if (_player && _player->_height > 200) {
			mouseY = (rawMouseY * 200) / _player->_height;
		}

		// Clamp to screen bounds (matching FUN_0040c3cc bounds)
		if (mouseX < 0) mouseX = 0;
		if (mouseX > 319) mouseX = 319;
		if (mouseY < 0) mouseY = 0;
		if (mouseY > 199) mouseY = 199;

		// Update ship position with smooth interpolation
		// FUN_0040c3cc uses complex smoothing, we use simpler immediate response
		const int16 maxStep = 15;
		if (_shipPosX < mouseX) {
			_shipPosX = MIN((int16)(_shipPosX + maxStep), mouseX);
		} else if (_shipPosX > mouseX) {
			_shipPosX = MAX((int16)(_shipPosX - maxStep), mouseX);
		}
		if (_shipPosY < mouseY) {
			_shipPosY = MIN((int16)(_shipPosY + maxStep), mouseY);
		} else if (_shipPosY > mouseY) {
			_shipPosY = MAX((int16)(_shipPosY - maxStep), mouseY);
		}

		// Update Handler 7 screen position (DAT_0044370c/e)
		// These track the actual on-screen position for direction calculation
		_flyShipScreenX = _shipPosX;
		_flyShipScreenY = _shipPosY;

		// Calculate ship direction from position (FUN_0040d836 lines 88-106)
		// Formula from assembly:
		//   hDir = (0xa0 - posX) >> 6  (with signed rounding)
		//   vDir = (0x95 - posY) / 0x2b
		//   dirIndex = hDir * 7 + vDir
		//
		// Note: The assembly formula gives:
		//   hDir: 0-4 where 2 is center (0xa0=160, range is -96 to +96, >> 6 gives -1 to 1, but clamped to 0-4)
		//   vDir: 0-6 where 3 is center (0x95=149, 0x2b=43, so 149/43 ≈ 3.5)
		//
		// Simplified direction calculation based on mouse position relative to center

		// Horizontal direction (0-4, center=2)
		// Formula: (160 - posX) >> 6, clamped to 0-4
		int16 hDiff = 160 - _flyShipScreenX;
		int16 hDir = (hDiff + 64) >> 6;  // Add 64 to shift range, divide by 64
		if (hDir < 0) hDir = 0;
		if (hDir > 4) hDir = 4;

		// Vertical direction (0-6, center=3)
		// Formula: (149 - posY) / 43, clamped to 0-6
		int16 vDir = (149 - _flyShipScreenY) / 43;
		if (vDir < 0) vDir = 0;
		if (vDir > 6) vDir = 6;

		// Additional adjustment from assembly (lines 90-105):
		// If vDir==3 and abs(posY) > 10, adjust by +/-1
		// If hDir==2 and abs(posX) > 15, adjust by +/-1
		// This creates a "deadzone" at center to reduce flicker
		if (vDir == 3 && ABS(_flyShipScreenY - 100) > 10) {
			if (_flyShipScreenY < 100) vDir = 2;
			else vDir = 4;
		}
		if (hDir == 2 && ABS(_flyShipScreenX - 160) > 15) {
			if (_flyShipScreenX < 160) hDir = 3;
			else hDir = 1;
		}

		_shipDirectionH = hDir;
		_shipDirectionV = vDir;
		_shipDirectionIndex = hDir * 7 + vDir;

		// Clamp direction index to valid range (0-34)
		if (_shipDirectionIndex < 0) _shipDirectionIndex = 0;
		if (_shipDirectionIndex > 34) _shipDirectionIndex = 34;

		// Update firing state
		_shipFiring = (_vm->VAR(_vm->VAR_LEFTBTN_HOLD) != 0);

		debug("Rebel2 Handler7: rawMouse=(%d,%d) scaled=(%d,%d) shipPos=(%d,%d) screenPos=(%d,%d) dir=(%d,%d) idx=%d flySprite=%p",
			rawMouseX, rawMouseY, mouseX, mouseY, _shipPosX, _shipPosY,
			_flyShipScreenX, _flyShipScreenY, _shipDirectionH, _shipDirectionV, _shipDirectionIndex,
			(void*)_flyShipSprite);

		return;
	}

	// Step 1: If par4 == 1, initialize/reset state (lines 114-121)
	if (par4 == 1) {
		// Draw status bar sprite 5 (FUN_0040bb87 equivalent)
		_rebelStatusBarSprite = (_rebelLevelType == 5) ? 53 : 5;
		debug("Rebel2 Opcode 6: Status Bar ENABLED - sprite %d", _rebelStatusBarSprite);

		// Clear link tables (DAT_0045797c through DAT_0045917c)
		// These are 4 tables of 0x400 (1024) shorts each
		for (int i = 0; i < 512; i++) {
			_rebelLinks[i][0] = 0;
			_rebelLinks[i][1] = 0;
			_rebelLinks[i][2] = 0;
		}

		// DAT_0047ab98 = DAT_0047ab9c (reset state flags)
		// We don't have a direct equivalent, but we can reset relevant counters
		_rebelHitCounter = 0;
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

	// ===== Handler 7: FLY NUT Loading (Space Flight) =====
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
	// FUN_00401234 case 6: par3 for POV NUTs, par4=5 for background
	if (!handled && _rebelHandler == 8) {
		// Check for background loading first (par4=5)
		if (par4 == 5) {
			handled = loadLevel2Background(animData, animDataSize, renderBitmap);
		}
		// Check for POV NUT sprites
		else if (par3 == 1 || par3 == 3 || par3 == 6 || par3 == 7) {
			handled = loadHandler8ShipSprites(animData, animDataSize, par3);
		}
	}

	// ===== Handler 25 (0x19): Also supports Level 2 background =====
	if (!handled && _rebelHandler == 25 && par4 == 5) {
		handled = loadLevel2Background(animData, animDataSize, renderBitmap);
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

bool InsaneRebel2::loadHandler8ShipSprites(byte *animData, int32 size, int16 par3) {
	// Handler 8 ship POV NUT loading - FUN_00401234 case 6 (opcode 8)
	// par3 values:
	//   1: POV001 - Primary ship sprite (DAT_0047e010 / _shipSprite)
	//   3: POV004 - Secondary ship sprite (DAT_0047e028 / _shipSprite2)
	//   6: POV002 - Ship overlay 1 (DAT_0047e020 / _shipOverlay1)
	//   7: POV003 - Ship overlay 2 (DAT_0047e018 / _shipOverlay2)

	if (!animData || size <= 0) {
		return false;
	}

	// Only handle valid POV sprite slots
	if (par3 != 1 && par3 != 3 && par3 != 6 && par3 != 7) {
		return false;
	}

	NutRenderer *newNut = new NutRenderer(_vm, animData, size);
	if (!newNut || newNut->getNumChars() <= 0) {
		debug("Rebel2 loadHandler8ShipSprites: NUT load failed for par3=%d", par3);
		delete newNut;
		return false;
	}

	debug("Rebel2 loadHandler8ShipSprites: Loaded ship NUT par3=%d with %d sprites",
		par3, newNut->getNumChars());

	switch (par3) {
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
					if (codec == 3 && fobjW > 0 && fobjH > 0 && fobjW <= 320 && fobjH <= 200) {
						byte *rleData = fobjData + 14;  // Skip full 14-byte FOBJ header
						smushDecodeRLE(_level2Background, rleData, fobjX, fobjY, fobjW, fobjH, 320);

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
	bool found = false;
	Common::List<enemy>::iterator it;
	for (it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->id == enemyId) {
			it->rect = Common::Rect(x, y, x + w, y + h);
			// Only re-activate if not destroyed
			if (!it->destroyed) {
				it->active = true;
			}
			found = true;
			break;
		}
	}
	if (!found) {
		init_enemyStruct(enemyId, x, y, w, h, true, false, -1);
	}
}

void InsaneRebel2::init_enemyStruct(int id, int32 x, int32 y, int32 w, int32 h, bool active, bool destroyed, int32 explosionFrame) {
	enemy e;
	e.id = id;
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
	bool skipImmediateDraw = (_rebelHandler == 7 || _rebelHandler == 8 ||
	                          _rebelHandler == 0x26 || _rebelHandler == 0x19);

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

void InsaneRebel2::spawnExplosion(int x, int y, int objectHalfWidth) {
	// Find first free slot (FUN_40A2E0 logic)
	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active || _explosions[i].counter <= 0) {
			_explosions[i].active = true;
			_explosions[i].counter = 10;
			_explosions[i].x = x;
			_explosions[i].y = y;
			_explosions[i].scale = objectHalfWidth;
			// TODO: Play sound via FUN_0041189e equivalent
			break;
		}
	}
}

void InsaneRebel2::spawnShot(int x, int y) {
	// Find free shot slot (2 slots total)
	for (int i = 0; i < 2; i++) {
		if (!_shots[i].active) {
			_shots[i].active = true;
			_shots[i].counter = 4; // Lasts 4 frames
			_shots[i].x = x + _viewX;
			_shots[i].y = y + _viewY;
			// TODO: Play laser sound
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


void InsaneRebel2::drawLaserBeam(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, int progress, int maxProgress, int thickness, int param_9, NutRenderer *nut, int spriteIdx) {
	if (!nut || spriteIdx >= nut->getNumChars()) return;

	// Follow original FUN_0040BBF6 math precisely
	int texW = nut->getCharWidth(spriteIdx);
	int texH = nut->getCharHeight(spriteIdx);
	int param_11 = (_rebelLevelType <= 1) ? 12 : 25;

	if (maxProgress == 0) maxProgress = 1;
	int sVar7 = (param_11 * progress * 16) / maxProgress;

	int dx = x1 - x0;
	int dy = y1 - y0;
	int sVar6 = ((dx) * (param_11 + 1)) / param_11;
	int sVar1 = ((dy) * (param_11 + 1)) / param_11;

	int sVar4 = (sVar6 + x0) - (sVar6 * 16) / (sVar7 + 16);
	int sVar5 = (sVar1 + y0) - (sVar1 * 16) / (sVar7 + 16);
	int sVar6_end = (sVar6 + x0) - (sVar6 * 16) / (param_9 + sVar7 + 16);
	int sVar7_end = (sVar1 + y0) - (sVar1 * 16) / (param_9 + sVar7 + 16);

	const byte *srcBase = nut->getCharData(spriteIdx);
	if (!srcBase || texW <= 0 || texH <= 0) return;

	int iVar2 = abs(sVar5 - sVar7_end);
	int iVar3 = abs(sVar4 - sVar6_end);

	if (iVar2 < iVar3) {
		// Column major case (wide)
		iVar2 = abs(sVar4 - sVar6_end);
		long long temp = (long long)iVar2 * (long long)texH * (long long)thickness;
		// sVar1calc = (temp >> 3) / texW + 2
		int sVar1calc = (int)((temp >> 3) / texW) + 2;
		int local_24 = -sVar1calc;
		int sVar8 = sVar1calc >> 1;
		const byte *local_28 = srcBase;
		for (int local_2c = 0; local_2c < sVar1calc; local_2c++) {
			drawTexturedSegment(dst, pitch, width, height, sVar4, (sVar5 - sVar8) + local_2c,
						 sVar6_end, (sVar7_end - sVar8) + local_2c, texW, local_28);
			for (local_24 = texH + local_24; local_24 > 0; local_24 -= sVar1calc) {
				local_28 += texW;
			}
		}
	} else {
		// Row major case (tall)
		iVar2 = abs(sVar5 - sVar7_end);
		int local_30 = (int)(((long long)iVar2 * (long long)texH) / texW) + 2;
		if (texH < local_30) local_30 = texH;
		int local_24 = -local_30;
		const byte *local_28 = srcBase;
		int sVar1_half = local_30 >> 1;
		for (int local_2c = 0; local_2c < local_30; local_2c++) {
			drawTexturedSegment(dst, pitch, width, height, (sVar4 - sVar1_half) + local_2c, sVar5,
						 (sVar6_end - sVar1_half) + local_2c, sVar7_end, texW, local_28);
			for (local_24 = texH + local_24; local_24 > 0; local_24 -= local_30) {
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

void InsaneRebel2::registerCollisionZone(Common::SeekableReadStream &b, int16 subOpcode) {
	// IACT Opcode 5 data layout (from info.md):
	//   +0x00: opcode (5) - already read by caller
	//   +0x02: sub-opcode (0x0D or 0x0E) - passed as parameter
	//   +0x04: par3 (flags)
	//   +0x06: zoneType
	//   +0x08: frameStart
	//   +0x0A: frameEnd
	//   +0x0C-0x1A: X1,Y1,X2,Y2,X3,Y3,X4,Y4 vertex coordinates
	//
	// The stream position is currently at offset +0x04 (after opcode and sub-opcode)

	int16 par3 = b.readSint16LE();       // +0x04 (flags - unused for now)
	(void)par3;  // Suppress unused variable warning
	int16 zoneType = b.readSint16LE();   // +0x06
	int16 frameStart = b.readSint16LE(); // +0x08
	int16 frameEnd = b.readSint16LE();   // +0x0A
	int16 x1 = b.readSint16LE();         // +0x0C
	int16 y1 = b.readSint16LE();         // +0x0E
	int16 x2 = b.readSint16LE();         // +0x10
	int16 y2 = b.readSint16LE();         // +0x12
	int16 x3 = b.readSint16LE();         // +0x14
	int16 y3 = b.readSint16LE();         // +0x16
	int16 x4 = b.readSint16LE();         // +0x18
	int16 y4 = b.readSint16LE();         // +0x1A

	CollisionZone zone;
	zone.x1 = x1;
	zone.y1 = y1;
	zone.x2 = x2;
	zone.y2 = y2;
	zone.x3 = x3;
	zone.y3 = y3;
	zone.x4 = x4;
	zone.y4 = y4;
	zone.frameStart = frameStart;
	zone.frameEnd = frameEnd;
	zone.zoneType = zoneType;
	zone.subOpcode = subOpcode;
	zone.active = true;

	// Register zone into appropriate table based on sub-opcode
	if (subOpcode == 0x0D && _primaryZoneCount < kMaxCollisionZones) {
		// Primary collision zones (obstacles)
		_primaryZones[_primaryZoneCount++] = zone;
		debug("Rebel2: Registered PRIMARY collision zone %d: type=%d frames=[%d-%d] quad=(%d,%d)-(%d,%d)-(%d,%d)-(%d,%d)",
			_primaryZoneCount - 1, zoneType, frameStart, frameEnd,
			x1, y1, x2, y2, x3, y3, x4, y4);
	} else if (subOpcode == 0x0E && _secondaryZoneCount < kMaxCollisionZones) {
		// Secondary collision zones (boundaries)
		_secondaryZones[_secondaryZoneCount++] = zone;
		debug("Rebel2: Registered SECONDARY collision zone %d: type=%d frames=[%d-%d] quad=(%d,%d)-(%d,%d)-(%d,%d)-(%d,%d)",
			_secondaryZoneCount - 1, zoneType, frameStart, frameEnd,
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
	if (_corridorLeftX != 0 || _corridorRightX != 320) {
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

	// Draw loop
	for (int iy = 0; iy < drawH; iy++) {
		const byte *s = src + (srcOffsetY + iy) * w + srcOffsetX;
		byte *d = dst + (drawY + iy) * pitch + drawX;
		for (int ix = 0; ix < drawW; ix++) {
			byte px = s[ix];
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
	bool levelSelectMode = (introPlaying && _gameState == kStateLevelSelect);

	// Handle level selection input and rendering
	if (levelSelectMode) {
		// Show the standard Windows arrow cursor (same as menu)
		Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
		CursorMan.replaceCursor(cursor);
		delete cursor;
		CursorMan.showMouse(true);

		// Process level selection input
		int selection = processLevelSelectInput();

		// Draw level selection overlay
		drawLevelSelectOverlay(renderBitmap, pitch, width, height);

		// If a selection was confirmed, signal video to stop
		if (selection >= 0) {
			debug("Rebel2: Level selection confirmed: %d", selection);
			_vm->_smushVideoShouldFinish = true;
		}

		// Skip normal HUD rendering in level select mode
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

		// Update inactivity timer
		_menuInactivityTimer++;

		// Check for inactivity timeout (300 frames = ~10 sec at 30fps, or ~25 sec at 12fps)
		if (_menuInactivityTimer > 300) {
			debug("Rebel2: Menu inactivity timeout");
			// Reset timer but don't take action yet
			_menuInactivityTimer = 0;
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
	renderFallbackShip(renderBitmap, pitch, width, height);

	// Enemy indicators and destroyed enemy area erase
	renderEnemyOverlays(renderBitmap, pitch, width, height, videoWidth);

	// Explosion animations (FUN_409FBC)
	renderExplosions(renderBitmap, pitch, width, height);

	// Laser shot beams and impacts
	renderLaserShots(renderBitmap, pitch, width, height);

	// Collision zone visualization (debug - for Handler 7/8 pilot modes)
	if (_rebelHandler == 7 || _rebelHandler == 8) {
		drawCollisionZones(renderBitmap, pitch, width, height, 0);
	}

	// Crosshair/reticle (FUN_004089ab, FUN_0040d836)
	renderCrosshair(renderBitmap, pitch, width, height);

	// HUD score/lives rendering (FUN_0041c012)
	renderScoreHUD(renderBitmap, pitch, width, height, 0);

	// Frame end cleanup: reset enemy active flags and collision zones (FUN_403240)
	frameEndCleanup();
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
	// Handler 7 Ship Rendering (Space Flight - FLY sprites)
	// Uses _flyShipSprite (FLY001) with 35 direction frames (5x7 grid)

	if (_rebelHandler != 7 || !_flyShipSprite || _shipLevelMode == 5)
		return;

	// Base position at screen center with direction offset
	int baseX = 160;
	int baseY = 105;
	int16 posOffsetX = (_flyShipScreenX - 160) / 10;
	int16 posOffsetY = (_flyShipScreenY - 100) / 10;
	int shipScreenX = baseX + posOffsetX;
	int shipScreenY = baseY + posOffsetY;

	int numSprites = _flyShipSprite->getNumChars();
	int spriteIndex = _shipDirectionIndex;
	if (spriteIndex < 0) spriteIndex = 0;
	if (spriteIndex >= numSprites) spriteIndex = numSprites - 1;

	// Center sprite at position
	int spriteW = _flyShipSprite->getCharWidth(spriteIndex);
	int spriteH = _flyShipSprite->getCharHeight(spriteIndex);
	int drawX = shipScreenX - spriteW / 2 + _viewX;
	int drawY = shipScreenY - spriteH / 2 + _viewY;

	renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _flyShipSprite, spriteIndex);

	// Laser overlay if firing
	if (_shipFiring && _flyLaserSprite && _flyLaserSprite->getNumChars() > 0) {
		int laserIndex = spriteIndex % _flyLaserSprite->getNumChars();
		renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _flyLaserSprite, laserIndex);
	}

	// Targeting overlay
	if (_flyTargetSprite && _flyTargetSprite->getNumChars() > 0) {
		int targetW = _flyTargetSprite->getCharWidth(0);
		int targetH = _flyTargetSprite->getCharHeight(0);
		int targetX = shipScreenX - targetW / 2 + _viewX;
		int targetY = shipScreenY - targetH / 2 + _viewY;
		renderNutSprite(renderBitmap, pitch, width, height, targetX, targetY, _flyTargetSprite, 0);
	}

	debug("Rebel2 Handler7: Ship at (%d,%d) sprite=%d/%d dir=(%d,%d) idx=%d",
		drawX, drawY, spriteIndex, numSprites, _shipDirectionH, _shipDirectionV, _shipDirectionIndex);
}

void InsaneRebel2::renderHandler8Ship(byte *renderBitmap, int pitch, int width, int height) {
	// Handler 8 Ship Rendering (Third-Person Vehicle - POV sprites)
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

	// Secondary ship sprite
	if (_shipSprite2 && _shipSprite2->getNumChars() > spriteIndex) {
		renderNutSprite(renderBitmap, pitch, width, height, drawX, drawY, _shipSprite2, spriteIndex);
	}

	debug("Rebel2 Handler8: Ship at (%d,%d) raw(%d,%d) offset(%d,%d) sprite=%d/%d dir=(%d,%d)",
		drawX, drawY, _shipPosX, _shipPosY, displayOffsetX, displayOffsetY,
		spriteIndex, numSprites, _shipDirectionH, _shipDirectionV);
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
	// Draw enemy indicator brackets and erase destroyed enemy areas

	// Erase destroyed enemies' areas (fill with black)
	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->destroyed) {
			Common::Rect r = it->rect;
			if (r.left < 0) r.left = 0;
			if (r.top < 0) r.top = 0;
			if (r.right > width) r.right = width;
			if (r.bottom > height) r.bottom = height;

			for (int y = r.top; y < r.bottom; y++) {
				for (int x = r.left; x < r.right; x++) {
					renderBitmap[y * pitch + x] = 0;
				}
			}
		}
	}

	// Draw green brackets for active enemies (Easy/Medium difficulty only)
	if (_difficulty >= 2)
		return;

	Common::Rect viewRect(_viewX, _viewY, _viewX + videoWidth, _viewY + 200);

	for (Common::List<enemy>::iterator it = _enemies.begin(); it != _enemies.end(); ++it) {
		if (it->destroyed || !it->active || isBitSet(it->id))
			continue;

		Common::Rect r = it->rect;
		if (r.right <= viewRect.left || r.left >= viewRect.right ||
		    r.bottom <= viewRect.top || r.top >= viewRect.bottom)
			continue;

		const byte color = 5;  // Green
		drawCornerBrackets(renderBitmap, pitch, width, height, r.left, r.top, r.width(), r.height(), color);
	}
}

void InsaneRebel2::renderExplosions(byte *renderBitmap, int pitch, int width, int height) {
	// Draw explosion animations from 5-slot system

	if (!_smush_iconsNut)
		return;

	for (int i = 0; i < 5; i++) {
		if (!_explosions[i].active)
			continue;

		if (_explosions[i].counter <= 0) {
			_explosions[i].active = false;
			continue;
		}

		// Determine base sprite index based on scale (FUN_409FBC logic)
		int baseIndex;
		if (_explosions[i].scale < 11) {
			baseIndex = 9;   // Small/Medium
		} else if (_explosions[i].scale < 21) {
			baseIndex = 19;  // Medium/Large
		} else {
			baseIndex = 29;  // Large/XL
		}

		// Formula: Base + (12 - Counter)
		int spriteIndex = baseIndex + (12 - _explosions[i].counter);

		if (_smush_iconsNut->getNumChars() > spriteIndex) {
			int ew = _smush_iconsNut->getCharWidth(spriteIndex);
			int eh = _smush_iconsNut->getCharHeight(spriteIndex);
			int cx = _explosions[i].x - ew / 2;
			int cy = _explosions[i].y - eh / 2;
			renderNutSprite(renderBitmap, pitch, width, height, cx, cy, _smush_iconsNut, spriteIndex);
		}

		_explosions[i].counter--;
	}
}

void InsaneRebel2::renderLaserShots(byte *renderBitmap, int pitch, int width, int height) {
	// Draw laser shot beams and impacts

	if (!_smush_iconsNut || _smush_iconsNut->getNumChars() <= 0)
		return;

	// Gun positions (approximate for turret mode)
	const int GUN_LEFT_X = 10, GUN_LEFT_Y = 190;
	const int GUN_RIGHT_X = 310, GUN_RIGHT_Y = 190;

	for (int i = 0; i < 2; i++) {
		if (!_shots[i].active)
			continue;

		if (_shots[i].counter <= 0) {
			_shots[i].active = false;
			continue;
		}

		int maxProgress = 4;
		int progress = maxProgress - _shots[i].counter;

		// Draw beams based on level type
		if (_rebelLevelType <= 1) {
			// Type 1: 3 beams (Right, Middle, Left)
			drawLaserBeam(renderBitmap, pitch, width, height,
				310 + _viewX, 170 + _viewY, _shots[i].x, _shots[i].y,
				progress, maxProgress, 8, 12, _smush_iconsNut, 0);

			drawLaserBeam(renderBitmap, pitch, width, height,
				160 + _viewX, 380 + _viewY, _shots[i].x, _shots[i].y,
				progress, maxProgress, 5, 8, _smush_iconsNut, 0);

			drawLaserBeam(renderBitmap, pitch, width, height,
				10 + _viewX, 170 + _viewY, _shots[i].x, _shots[i].y,
				progress, maxProgress, 8, 12, _smush_iconsNut, 0);
		} else {
			// Other levels: 2 beams
			drawLaserBeam(renderBitmap, pitch, width, height,
				GUN_LEFT_X + _viewX, GUN_LEFT_Y + _viewY, _shots[i].x, _shots[i].y,
				progress, maxProgress, 8, 12, _smush_iconsNut, 0);

			drawLaserBeam(renderBitmap, pitch, width, height,
				GUN_RIGHT_X + _viewX, GUN_RIGHT_Y + _viewY, _shots[i].x, _shots[i].y,
				progress, maxProgress, 8, 12, _smush_iconsNut, 0);
		}

		// Impact flash
		renderNutSprite(renderBitmap, pitch, width, height,
			_shots[i].x - 7, _shots[i].y - 7, _smush_iconsNut, 0);

		_shots[i].counter--;
	}
}

void InsaneRebel2::renderCrosshair(byte *renderBitmap, int pitch, int width, int height) {
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
	case 7:    // Space flight
	case 0x19: // Mixed/turret
		reticleIndex = 47;
		break;
	case 0x26: { // Full turret - animated crosshair
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
	case 8:    // Ground vehicle
	default:
		reticleIndex = 46;
		break;
	}

	if (_smush_iconsNut->getNumChars() > reticleIndex) {
		int cw = _smush_iconsNut->getCharWidth(reticleIndex);
		int ch = _smush_iconsNut->getCharHeight(reticleIndex);
		renderNutSprite(renderBitmap, pitch, width, height,
			_vm->_mouse.x - cw / 2 + _viewX, _vm->_mouse.y - ch / 2 + _viewY,
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

// ======================= Menu System Implementation =======================
// Emulates retail menu system from FUN_004147B2 and FUN_0041FDC8

void InsaneRebel2::resetMenu() {
	_menuSelection = 0;
	_menuInactivityTimer = 0;
	_menuRepeatDelay = 0;
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
	// Returns: -1 = no action, 0-6 = menu item selected
	//
	// Events are captured by notifyEvent() (EventObserver) which runs before
	// ScummEngine::parseEvents() consumes them. This ensures we don't miss
	// any input events even though we only process them on video frames.

	int result = -1;

	// Menu item Y positions (low-res 320x200 mode):
	// From assembly: baseY = numItems * -5 + 0x68 = 7 * -5 + 104 = 69
	// Items at Y = 69, 79, 89, 99, 109, 119 with spacing of 10
	const int baseY = 69;
	const int itemHeight = 10;

	// Process events from the queue (populated by notifyEvent)
	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.pop();
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_menuInactivityTimer = 0;  // Reset inactivity timer on any input

			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				// Navigate up (wrap around)
				_menuSelection--;
				if (_menuSelection < 0) {
					_menuSelection = _menuItemCount - 1;
				}
				debug("Menu: Selection changed to %d (UP)", _menuSelection);
				break;

			case Common::KEYCODE_DOWN:
				// Navigate down (wrap around)
				_menuSelection++;
				if (_menuSelection >= _menuItemCount) {
					_menuSelection = 0;
				}
				debug("Menu: Selection changed to %d (DOWN)", _menuSelection);
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				// Confirm selection
				if (_menuSelection >= 0 && _menuSelection < _menuItemCount) {
					result = _menuSelection;
					debug("Menu: Item %d selected (ENTER)", _menuSelection);
				}
				break;

			case Common::KEYCODE_ESCAPE:
				// ESC - Exit/Quit (return special value)
				result = 5;  // Quit option (index 5)
				debug("Menu: ESC pressed - quit");
				break;

			default:
				break;
			}
			break;

		case Common::EVENT_LBUTTONDOWN:
			_menuInactivityTimer = 0;

			{
				// Get mouse position from the event
				int mouseX = event.mouse.x;
				int mouseY = event.mouse.y;

				debug("Menu: Click detected at (%d, %d)", mouseX, mouseY);

				// Check which item was clicked (larger hit area for better usability)
				for (int i = 0; i < _menuItemCount; i++) {
					int itemY = baseY + i * itemHeight;
					// Use a larger vertical hit area (full item height)
					if (mouseY >= itemY - 4 && mouseY < itemY + 6) {
						_menuSelection = i;
						result = i;
						debug("Menu: Item %d clicked at Y=%d (itemY=%d)", i, mouseY, itemY);
						break;
					}
				}
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			// Update mouse position for hover effects (optional)
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			// Handle quit request
			result = 5;  // Quit option
			break;

		default:
			break;
		}
	}

	return result;
}

void InsaneRebel2::drawMenuOverlay(byte *renderBitmap, int pitch, int width, int height) {
	// Emulates FUN_0041f5ae menu text overlay rendering
	//
	// IMPORTANT: The menu background comes from the O_MENU_X.SAN video file, NOT from MSTOVER.NUT.
	// The O_MENU_X.SAN files (A through O) each contain a full 320x200 FOBJ frame in Frame 0
	// which is decoded by SmushPlayer and stored in renderBitmap before this function is called.
	// MSTOVER.NUT is only used in cheat mode (when DAT_0047aba4 != 0).
	//
	// From FUN_4147B2: param_3 = (DAT_0047a806 == 0) + 6 = 7 for keyboard mode
	// Menu structure: 1 title + 6 selectable items = 7 total items
	static const char *menuItems[] = {
		"GAME MAIN MENU",   // Title (index 0)
		"START GAME",       // Item 0 (index 1)
		"OPTIONS",          // Item 1 (index 2)
		"CONTINUE INTRO",   // Item 2 (index 3)
		"SHOW TOP PILOTS",  // Item 3 (index 4)
		"SHOW CREDITS",     // Item 4 (index 5)
		"QUIT"              // Item 5 (index 6)
	};

	const int numItemsTotal = 7;  // Title + 6 menu options (used for Y calculations)
	const int numSelectableItems = 6;  // Selectable menu options (0-5)

	// The O_MENU_X.SAN video frame is already in renderBitmap from SmushPlayer.
	// We only draw text and selection highlights on top.

	// From assembly FUN_0041f5ae (low-res mode, DAT_0047a808 < 2):
	// Center X = 160 for 320px, scale for actual width
	// Title Y = 46, Item base Y = 69, spacing = 10
	const int centerX = width / 2;  // Center in actual buffer
	const int titleY = numItemsTotal * -5 + 0x51;  // 46
	const int itemBaseY = numItemsTotal * -5 + 0x68;  // 69
	const int itemSpacing = 10;

	debug(5, "drawMenuOverlay: buffer %dx%d, centerX=%d", width, height, centerX);

	// Use SMALFONT.NUT for menu text rendering
	NutRenderer *font = _smush_smalfontNut;
	if (!font) {
		debug(1, "drawMenuOverlay: font is NULL!");
		return;
	}

	// Get the number of characters in the font
	int numFontChars = font->getNumChars();
	debug(5, "drawMenuOverlay: font has %d chars", numFontChars);

	// Helper: calculate string width with bounds checking
	auto getStringWidth = [&](const char *str) -> int {
		int w = 0;
		while (*str) {
			byte c = (byte)*str++;
			if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
			if (c < numFontChars) {
				w += font->getCharWidth(c);
			}
		}
		return w;
	};

	// Debug: Check if renderBitmap has video content (non-zero pixels)
	static int debugFrameCount = 0;
	if (debugFrameCount < 5) {
		int nonZeroCount = 0;
		for (int i = 0; i < MIN(width * height, 1000); i++) {
			if (renderBitmap[i] != 0) nonZeroCount++;
		}
		debug(1, "drawMenuOverlay: frame %d, buffer sample has %d non-zero pixels in first 1000",
		      debugFrameCount, nonZeroCount);
		debugFrameCount++;
	}

	// Set up clipRect for the entire rendering area
	// Use screen dimensions (320x200) for the clip rect, not the video frame dimensions.
	// The menu text is designed for a 320x200 screen, and the underlying renderBitmap
	// (from SmushPlayer's virtual screen buffer) is always screen-sized. The width/height
	// parameters come from _player->_width/_height which may be smaller if the video
	// frame doesn't cover the full screen, but that would incorrectly clip the menu text.
	Common::Rect clipRect(0, 0, _vm->_screenWidth, _vm->_screenHeight);

	// Use screen width as the actual pitch for rendering. During SMUSH playback,
	// SmushPlayer sets vs->pitch = vs->w = _screenWidth (see smush_player.cpp init()).
	// The 'pitch' parameter comes from _player->_width which may differ from the actual
	// buffer pitch if the video frame has different dimensions. Using the wrong pitch
	// would cause character rows to be written at incorrect offsets in the buffer.
	int actualPitch = _vm->_screenWidth;

	auto drawString = [&](const char *str, int x, int y) {
		while (*str) {
			byte c = (byte)*str++;
			if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';

			// Skip characters outside font range
			if (c >= numFontChars) {
				debug(5, "drawMenuOverlay: char %d out of range (max %d)", c, numFontChars);
				continue;
			}

			int charW = font->getCharWidth(c);

			// Use NutRenderer's drawCharV7 which properly handles character rendering
			// col=-1 means use original font colors, hardcodedColors=true, smushColorMode=true
			if (x >= 0 && y >= 0 && charW > 0) {
				font->drawCharV7(renderBitmap, clipRect, x, y, actualPitch, -1,
				                 kStyleAlignLeft, c, true, true);
			}
			x += charW;
		}
	};

	// Draw title centered at Y = 46
	{
		int titleWidth = getStringWidth(menuItems[0]);
		int titleX = centerX - titleWidth / 2;
		drawString(menuItems[0], titleX, titleY);
	}

	// Draw menu items starting at Y = 69
	for (int i = 0; i < numSelectableItems; i++) {
		int itemY = itemBaseY + i * itemSpacing;
		const char *text = menuItems[i + 1];

		int textWidth = getStringWidth(text);
		int textX = centerX - textWidth / 2;
		drawString(text, textX, itemY);

		// Draw selection highlight box around selected item
		if (i == _menuSelection) {
			int bracketWidth = textWidth + 12;
			int bracketHeight = 10;
			byte highlightColor = 255;  // White/bright color for visibility

			int leftX = centerX - bracketWidth / 2;
			int rightX = centerX + bracketWidth / 2;
			int topY = itemY - 1;
			int bottomY = itemY + bracketHeight - 1;

			// Clamp to screen bounds (use screen dimensions, not video frame dimensions)
			int screenW = _vm->_screenWidth;
			int screenH = _vm->_screenHeight;
			if (leftX < 0) leftX = 0;
			if (rightX >= screenW) rightX = screenW - 1;
			if (topY < 0) topY = 0;
			if (bottomY >= screenH) bottomY = screenH - 1;

			// Draw selection rectangle using actualPitch (screen width)
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
	// Use hardcoded "PAUSED" string (TRS string 0x79 is "Quit Game" in RA2)
	const char *pauseText = "PAUSED";

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
		// Menu items (from FUN_004147B2 disassembly):
		// case 0: return 2 (New Game)
		// case 1: return 4 (Continue)
		// case 2: Options menu (stays in loop)
		// case 3: return 0 (Exit)
		// case 4: Unknown function
		// case 5: Credits (play O_CREDIT.SAN, then return 1)
		// case 6: Quit (stop video, exit)
		switch (_menuSelection) {
		case 0:  // Start Game -> Level Selection
			debug("Rebel2: Start Game selected - going to level selection");
			_gameState = kStateLevelSelect;
			_menuInputActive = false;
			return kMenuContinue;  // Go to level selection

		case 1:  // Continue (same as Start Game for now)
			debug("Rebel2: Continue selected - going to level selection");
			_gameState = kStateLevelSelect;
			_menuInputActive = false;
			return kMenuContinue;

		case 2:  // Options
			debug("Rebel2: Options selected");
			// TODO: Show options menu (FUN_00406ed2)
			// For now, just continue menu loop
			break;

		case 3:  // Exit (back to title/intro)
			debug("Rebel2: Exit selected");
			// Return to menu loop
			break;

		case 4:  // Unknown function (FUN_00420116)
			debug("Rebel2: Unknown menu item 4 selected");
			break;

		case 5:  // Credits
			debug("Rebel2: Credits selected");
			_gameState = kStateCredits;
			splayer->setCurVideoFlags(0x20);
			splayer->play("OPEN/O_CREDIT.SAN", 12);
			_gameState = kStateMainMenu;
			// After credits, return to menu
			break;

		case 6:  // Quit
			debug("Rebel2: Quit selected");
			_menuInputActive = false;
			return 0;

		default:
			debug("Rebel2: Unknown menu selection %d", _menuSelection);
			break;
		}
	}

	_menuInputActive = false;
	return 0;
}

// ==================== Level Selection Menu ====================
// Emulates FUN_00414A41 - Level selection menu
// For now, only Level 1 is available. This will be expanded later.

int InsaneRebel2::runLevelSelect() {
	// Level selection menu loop - emulates FUN_00414A41
	// Returns:
	//   kLevelSelectPlay (1) = Play selected level
	//   kLevelSelectBack (0) = Return to main menu
	//   kLevelSelectQuit (2) = Quit game

	debug("Rebel2: Entering level selection");

	// Enable menu input capture via EventObserver and clear any stale events
	_menuInputActive = true;
	while (!_menuEventQueue.empty()) _menuEventQueue.pop();

	// Initialize level selection state
	_levelSelection = 0;
	_levelItemCount = 7;  // Selectable items: 6 levels + MAIN MENU
	_selectedLevel = 1;   // Default to level 1
	_menuRepeatDelay = 0;
	_gameState = kStateLevelSelect;

	// Get the SmushPlayer
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;

	// Level selection loop - we'll reuse the menu video as background
	// In the original, this uses the same O_MENU_X.SAN videos
	while (!_vm->shouldQuit()) {
		_vm->_smushVideoShouldFinish = false;

		// Use a menu video as background for level selection
		Common::String menuVideo = getRandomMenuVideo();
		debug("Rebel2: Playing level select background: %s", menuVideo.c_str());

		// Set video flags for menu mode
		splayer->setCurVideoFlags(0x20);

		// Play the menu video - input is processed in procPostRendering
		splayer->play(menuVideo.c_str(), 12);

		if (_vm->shouldQuit()) {
			_menuInputActive = false;
			return kLevelSelectQuit;
		}

		// If video ended without selection, continue looping
		if (!_vm->_smushVideoShouldFinish) {
			continue;
		}

		_vm->_smushVideoShouldFinish = false;

		debug("Rebel2: Level selection made: %d", _levelSelection);

		// Process level selection
		// Menu items:
		// 0-5: Levels 1-6 (CHAPTER 1-6)
		// 6: MAIN MENU (back)
		if (_levelSelection >= 0 && _levelSelection <= 5) {
			// Level selected (0 = Level 1, 5 = Level 6)
			_selectedLevel = _levelSelection + 1;
			debug("Rebel2: Level %d selected", _selectedLevel);
			_menuInputActive = false;
			return kLevelSelectPlay;
		} else if (_levelSelection == 6) {
			// Main Menu (back)
			debug("Rebel2: Back to main menu selected");
			_menuInputActive = false;
			return kLevelSelectBack;
		}
	}

	_menuInputActive = false;
	return kLevelSelectQuit;
}

int InsaneRebel2::processLevelSelectInput() {
	// Process input for level selection menu
	// Similar to processMenuInput but for level selection
	// Returns: -1 = no action, 0+ = item selected
	//
	// Events are captured by notifyEvent() - see processMenuInput for details.

	int result = -1;

	// Level menu Y positions (similar to main menu)
	// Using same formula: base Y = numItemsTotal * -5 + 104
	// numItemsTotal = 3 (title + 2 selectable items)
	const int numItemsTotal = 3;
	const int baseY = numItemsTotal * -5 + 104;  // 89
	const int itemHeight = 10;

	// Process events from the queue (populated by notifyEvent)
	while (!_menuEventQueue.empty()) {
		Common::Event event = _menuEventQueue.pop();
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
				_levelSelection--;
				if (_levelSelection < 0) {
					_levelSelection = _levelItemCount - 1;
				}
				debug("LevelSelect: Selection changed to %d (UP)", _levelSelection);
				break;

			case Common::KEYCODE_DOWN:
				_levelSelection++;
				if (_levelSelection >= _levelItemCount) {
					_levelSelection = 0;
				}
				debug("LevelSelect: Selection changed to %d (DOWN)", _levelSelection);
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				if (_levelSelection >= 0 && _levelSelection < _levelItemCount) {
					result = _levelSelection;
					debug("LevelSelect: Item %d selected (ENTER)", _levelSelection);
				}
				break;

			case Common::KEYCODE_ESCAPE:
				// ESC - Back to main menu
				result = _levelItemCount - 1;  // Last item is "MAIN MENU"
				debug("LevelSelect: ESC pressed - back to menu");
				break;

			default:
				break;
			}
			break;

		case Common::EVENT_LBUTTONDOWN:
			{
				// Get mouse position from the event
				int mouseX = event.mouse.x;
				int mouseY = event.mouse.y;

				debug("LevelSelect: Click detected at (%d, %d)", mouseX, mouseY);

				for (int i = 0; i < _levelItemCount; i++) {
					int itemY = baseY + i * itemHeight;
					// Use a larger vertical hit area (full item height)
					if (mouseY >= itemY - 4 && mouseY < itemY + 6) {
						_levelSelection = i;
						result = i;
						debug("LevelSelect: Item %d clicked at Y=%d (itemY=%d)", i, mouseY, itemY);
						break;
					}
				}
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			// Update mouse position for hover effects
			_vm->_mouse.x = event.mouse.x;
			_vm->_mouse.y = event.mouse.y;
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			// Handle quit request - go back to main menu
			result = _levelItemCount - 1;
			break;

		default:
			break;
		}
	}

	return result;
}

void InsaneRebel2::drawLevelSelectOverlay(byte *renderBitmap, int pitch, int width, int height) {
	// Draw level selection menu overlay
	// Emulates FUN_0041f5ae for level selection mode
	//
	// From info.md - Low Resolution Coordinate Formulas:
	// Center X = 160, Title Y = numItems * -5 + 81, Item Base Y = numItems * -5 + 104
	// Item spacing = 10 pixels, Selection box height = 10 pixels

	// Level menu items - all 6 levels plus Main Menu
	// In the original game, this would show all unlocked levels plus options
	static const char *levelItems[] = {
		"SELECT CHAPTER",    // Title (index 0)
		"CHAPTER 1",         // Level 1 (index 1) - selectable
		"CHAPTER 2",         // Level 2 (index 2) - selectable
		"CHAPTER 3",         // Level 3 (index 3) - selectable
		"CHAPTER 4",         // Level 4 (index 4) - selectable
		"CHAPTER 5",         // Level 5 (index 5) - selectable
		"CHAPTER 6",         // Level 6 (index 6) - selectable
		"MAIN MENU"          // Back to menu (index 7) - selectable
	};

	const int numItemsTotal = 8;     // Title + 7 selectable items
	const int numSelectableItems = 7;

	// Calculate positions (low-res 320x200 mode)
	const int centerX = width / 2;
	const int titleY = numItemsTotal * -5 + 81;      // 81 - 15 = 66
	const int itemBaseY = numItemsTotal * -5 + 104;  // 104 - 15 = 89
	const int itemSpacing = 10;

	NutRenderer *font = _smush_smalfontNut;
	if (!font) {
		debug(1, "drawLevelSelectOverlay: font is NULL!");
		return;
	}

	int numFontChars = font->getNumChars();
	int actualPitch = pitch;
	Common::Rect clipRect(0, 0, width, height);

	// Helper function to draw centered text with optional highlight box
	// We'll use a simple loop instead of lambda for compatibility
	auto drawTextCentered = [&](const char *text, int y, bool highlight) {
		// Calculate text width first
		int textWidth = 0;
		for (const char *c = text; *c; c++) {
			int charIdx = (unsigned char)*c;
			if (charIdx < numFontChars) {
				textWidth += font->getCharWidth(charIdx);
			}
		}

		int curX = centerX - textWidth / 2;

		// Draw each character using drawCharV7
		for (const char *c = text; *c; c++) {
			int charIdx = (unsigned char)*c;
			if (charIdx < numFontChars) {
				int charWidth = font->getCharWidth(charIdx);
				if (curX >= 0 && curX + charWidth <= width && y >= 0) {
					// Use drawCharV7 with color -1 (original colors), hardcodedColors=true, smushColorMode=true
					font->drawCharV7(renderBitmap, clipRect, curX, y, actualPitch, -1,
					                 kStyleAlignLeft, charIdx, true, true);
				}
				curX += charWidth;
			}
		}

		// If highlighted, draw selection box around text
		if (highlight) {
			int boxWidth = textWidth + 12;
			int boxHeight = 10;
			int boxX = centerX - boxWidth / 2;
			int boxY = y - 1;

			// Highlight color (bright color for visibility)
			byte highlightColor = 0xF0;

			// Draw box border (top, bottom, left, right edges)
			if (boxY >= 0 && boxY < height && boxX >= 0 && boxX + boxWidth <= width) {
				// Top edge
				for (int px = boxX; px < boxX + boxWidth && px < width; px++) {
					if (px >= 0) renderBitmap[boxY * actualPitch + px] = highlightColor;
				}
				// Bottom edge
				int bottomY = boxY + boxHeight - 1;
				if (bottomY < height) {
					for (int px = boxX; px < boxX + boxWidth && px < width; px++) {
						if (px >= 0) renderBitmap[bottomY * actualPitch + px] = highlightColor;
					}
				}
				// Left edge
				for (int py = boxY; py < boxY + boxHeight && py < height; py++) {
					if (py >= 0 && boxX >= 0) renderBitmap[py * actualPitch + boxX] = highlightColor;
				}
				// Right edge
				int rightX = boxX + boxWidth - 1;
				if (rightX < width) {
					for (int py = boxY; py < boxY + boxHeight && py < height; py++) {
						if (py >= 0) renderBitmap[py * actualPitch + rightX] = highlightColor;
					}
				}
			}
		}
	};

	// Draw title (not selectable)
	drawTextCentered(levelItems[0], titleY, false);

	// Draw selectable items
	for (int i = 0; i < numSelectableItems; i++) {
		int itemY = itemBaseY + i * itemSpacing;
		bool isSelected = (i == _levelSelection);
		drawTextCentered(levelItems[i + 1], itemY, isSelected);
	}

	// Draw info text at bottom if a level is selected (not "MAIN MENU")
	if (_levelSelection < numSelectableItems - 1) {
		// Show difficulty or other info
		// From info.md: Difficulty at X=30, Y=180
		const char *difficultyText = "DIFFICULTY: EASY";
		int infoY = 180;
		int infoX = 30;

		// Draw left-aligned text using drawCharV7
		int curX = infoX;
		for (const char *c = difficultyText; *c; c++) {
			int charIdx = (unsigned char)*c;
			if (charIdx < numFontChars) {
				int charWidth = font->getCharWidth(charIdx);
				if (curX >= 0 && curX + charWidth <= width && infoY >= 0) {
					font->drawCharV7(renderBitmap, clipRect, curX, infoY, actualPitch, -1,
					                 kStyleAlignLeft, charIdx, true, true);
				}
				curX += charWidth;
			}
		}
	}
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
	// 2. Video flags are set to 0x20 (cinematic mode)
	//
	// Original: DAT_0047ee84 is only set by IACT opcode 6 during gameplay videos
	// Cinematics don't have opcode 6, so handler stays 0
	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // No status bar during cinematics

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x20);  // Cinematic mode
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
		// Level 2 uses Handler 8 (third-person vehicle mode) - set before gameplay
		_rebelHandler = 8;
		// First play the cutscene
		filename = Common::String::format("%s/%sCUT.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing cutscene %s", filename.c_str());
		splayer->setCurVideoFlags(0x20);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Part 1 (multiple variations - play A for now)
		splayer->setCurVideoFlags(0x28);
		filename = Common::String::format("%s/P1/%sP01_A.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Post segment 1
		filename = Common::String::format("%s/%sPST1.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->setCurVideoFlags(0x20);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Part 2
		splayer->setCurVideoFlags(0x28);
		filename = Common::String::format("%s/P2/%sP02_A.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Post segment 2
		filename = Common::String::format("%s/%sPST2.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->setCurVideoFlags(0x20);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Part 3
		splayer->setCurVideoFlags(0x28);
		filename = Common::String::format("%s/P3/%sP03_A.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);
		break;

	case 3:
		// Level 3: Two gameplay phases (space flight)
		// Level 3 uses Handler 7 (space flight mode) - FUN_0040d836/FUN_0040c3cc
		_rebelHandler = 7;
		filename = Common::String::format("%s/%sPLAY1.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Post segment
		filename = Common::String::format("%s/%sPOST1.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->setCurVideoFlags(0x20);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Phase 2
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
		splayer->setCurVideoFlags(0x20);
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
		filename = Common::String::format("%s/%sPOST1.SAN", dir.c_str(), prefix.c_str());
		debug("Rebel2: Playing %s", filename.c_str());
		splayer->setCurVideoFlags(0x20);
		splayer->play(filename.c_str(), 12);

		if (_vm->shouldQuit() || _playerShield == 0) return false;

		// Phase 2
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
	splayer->setCurVideoFlags(0x20);  // Cinematic mode
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
	if (levelId == 2 || levelId == 4) {
		filename = Common::String::format("%s/%sDIE.SAN", dir.c_str(), prefix.c_str());
	} else {
		filename = Common::String::format("%s/%sDIE_A.SAN", dir.c_str(), prefix.c_str());
	}

	debug("Rebel2: Playing level %d death: %s", levelId, filename.c_str());

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x20);
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
	splayer->setCurVideoFlags(0x20);
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
	splayer->setCurVideoFlags(0x20);
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
	default:
		// Levels 7-15: Use generic handler (similar to Level 1)
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
		// Level 6: Similar to Level 3 (two phases with frame-based selection)
		if (phase == 1) {
			if (frame < 300) return "A";
			if (frame < 600) return "B";
			return "C";
		} else {
			if (frame < 400) return "A";
			if (frame < 800) return "B";
			return "C";
		}

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
	splayer->setCurVideoFlags(0x20);
	splayer->play(filename.c_str(), 12);
}

void InsaneRebel2::playLevelRetryVariant(int levelId, int phase) {
	// Play retry video - phase-specific for multi-phase levels

	_rebelHandler = 0;
	_rebelStatusBarSprite = 0;  // Reset for retry - will be set by IACT opcode 6 if needed

	Common::String dir = getLevelDir(levelId);
	Common::String prefix = getLevelPrefix(levelId);
	Common::String filename;

	if (levelId == 3 && phase == 2) {
		// Level 3 phase 2 has its own retry video: 03RETRYB.SAN
		filename = Common::String::format("%s/%sRETRYB.SAN", dir.c_str(), prefix.c_str());
	} else {
		// Standard retry video
		filename = Common::String::format("%s/%sRETRY.SAN", dir.c_str(), prefix.c_str());
	}

	debug("Rebel2: Playing retry video: %s (phase=%d)", filename.c_str(), phase);

	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	splayer->setCurVideoFlags(0x20);
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
// Level 2 Handler - FUN_00418063
// Multiple parts with P1/P2/P3 subdirectories
// Random animation variants for each part
// =============================================================================

int InsaneRebel2::runLevel2() {
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	int bonusCount = 0;  // Tracks special events (local_1c in assembly)

	// Play cutscene (02CUT.SAN)
	playCinematic("LEV02/02CUT.SAN");
	if (_vm->shouldQuit()) return kLevelQuit;

	// Play level beginning cinematic (02BEG.SAN)
	playLevelBegin(2);
	if (_vm->shouldQuit()) return kLevelQuit;

	// Main gameplay retry loop (restarts from beginning on death)
	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;
		bonusCount = 0;

		// ===== PHASE 1: P1/02P01_X.SAN =====
		// First play variant A which contains the background IACT (opcode 8, par4=5)
		// The background is loaded during this video and persists for B/C/D variants
		{
			debug("Rebel2: Level 2 Phase 1 - playing 02P01_A.SAN (background loader)");
			splayer->setCurVideoFlags(0x28);
			splayer->play("LEV02/P1/02P01_A.SAN", 12);
			_deathFrame = splayer->_frame;
		}

		if (_vm->shouldQuit()) return kLevelQuit;

		// Now select random variant (B, C, or D) for actual gameplay
		if (_playerShield > 0) {
			int variant = getRandomVariant(3);
			const char *variants[] = {"B", "C", "D"};
			Common::String filename = Common::String::format("LEV02/P1/02P01_%s.SAN", variants[variant]);

			debug("Rebel2: Level 2 Phase 1 - playing %s (gameplay)", filename.c_str());
			splayer->setCurVideoFlags(0x28);
			splayer->play(filename.c_str(), 12);
			_deathFrame = splayer->_frame;
		}

		if (_vm->shouldQuit()) return kLevelQuit;

		if (_playerShield == 0) {
			// Died in phase 1
			debug("Rebel2: Level 2 Phase 1 death");
			playLevelDeathVariant(2, 1, _deathFrame);
			if (_vm->shouldQuit()) return kLevelQuit;

			_playerLives--;
			if (_playerLives <= 0) {
				playLevelGameOver(2);
				return kLevelGameOver;
			}
			playLevelRetry(2);
			if (_vm->shouldQuit()) return kLevelQuit;
			continue;  // Restart from beginning
		}

		// Post segment 1 (02PST1.SAN)
		splayer->setCurVideoFlags(0x20);
		splayer->play("LEV02/02PST1.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;

		_currentPhase = 2;

		// ===== PHASE 2: P2/02P02_X.SAN =====
		// First play variant A which contains the background IACT for this phase
		{
			debug("Rebel2: Level 2 Phase 2 - playing 02P02_A.SAN (background loader)");
			splayer->setCurVideoFlags(0x28);
			splayer->play("LEV02/P2/02P02_A.SAN", 12);
			_deathFrame = splayer->_frame;
		}

		if (_vm->shouldQuit()) return kLevelQuit;

		// Now select gameplay variant (B through F)
		if (_playerShield > 0) {
			int variant = getRandomVariant(5);
			const char *variants[] = {"B", "C", "D", "E", "F"};
			Common::String filename = Common::String::format("LEV02/P2/02P02_%s.SAN", variants[variant]);

			debug("Rebel2: Level 2 Phase 2 - playing %s (gameplay)", filename.c_str());
			splayer->setCurVideoFlags(0x28);
			splayer->play(filename.c_str(), 12);
			_deathFrame = splayer->_frame;
		}

		if (_vm->shouldQuit()) return kLevelQuit;

		if (_playerShield == 0) {
			// Died in phase 2
			debug("Rebel2: Level 2 Phase 2 death");
			playLevelDeathVariant(2, 2, _deathFrame);
			if (_vm->shouldQuit()) return kLevelQuit;

			_playerLives--;
			if (_playerLives <= 0) {
				playLevelGameOver(2);
				return kLevelGameOver;
			}
			playLevelRetry(2);
			if (_vm->shouldQuit()) return kLevelQuit;
			continue;  // Restart from beginning
		}

		// Post segment 2 (02PST2.SAN)
		splayer->setCurVideoFlags(0x20);
		splayer->play("LEV02/02PST2.SAN", 12);
		if (_vm->shouldQuit()) return kLevelQuit;

		_currentPhase = 3;

		// ===== PHASE 3: P3/02P03_X.SAN =====
		// First play variant A which contains the background IACT for this phase
		{
			debug("Rebel2: Level 2 Phase 3 - playing 02P03_A.SAN (background loader)");
			splayer->setCurVideoFlags(0x28);
			splayer->play("LEV02/P3/02P03_A.SAN", 12);
			_deathFrame = splayer->_frame;
		}

		if (_vm->shouldQuit()) return kLevelQuit;

		// Now select gameplay variant (B through I)
		if (_playerShield > 0) {
			int variant = getRandomVariant(8);
			const char *variants[] = {"B", "C", "D", "E", "F", "G", "H", "I"};
			Common::String filename = Common::String::format("LEV02/P3/02P03_%s.SAN", variants[variant]);

			debug("Rebel2: Level 2 Phase 3 - playing %s (gameplay)", filename.c_str());
			splayer->setCurVideoFlags(0x28);
			splayer->play(filename.c_str(), 12);
			_deathFrame = splayer->_frame;
		}

		if (_vm->shouldQuit()) return kLevelQuit;

		if (_playerShield == 0) {
			// Died in phase 3
			debug("Rebel2: Level 2 Phase 3 death");
			playLevelDeathVariant(2, 3, _deathFrame);
			if (_vm->shouldQuit()) return kLevelQuit;

			_playerLives--;
			if (_playerLives <= 0) {
				playLevelGameOver(2);
				return kLevelGameOver;
			}
			playLevelRetry(2);
			if (_vm->shouldQuit()) return kLevelQuit;
			continue;  // Restart from beginning
		}

		// Level completed!
		debug("Rebel2: Level 2 completed! bonusCount=%d", bonusCount);
		playLevelEnd(2);
		_levelUnlocked[2] = true;  // Unlock level 3
		return kLevelNextLevel;
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
	splayer->setCurVideoFlags(0x20);
	splayer->play("LEV03/03POST1.SAN", 12);
	if (_vm->shouldQuit()) return kLevelQuit;

	// ===== PHASE 2 retry loop (preserves phase 1 score) =====
	_currentPhase = 2;

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;

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
	splayer->setCurVideoFlags(0x20);
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
	SmushPlayer *splayer = ((ScummEngine_v7 *)_vm)->_splayer;
	int phase1Score = 0;

	// Play level beginning cinematic (06BEG.SAN)
	playLevelBegin(6);
	if (_vm->shouldQuit()) return kLevelQuit;

	// ===== PHASE 1 retry loop =====
	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;
		_currentPhase = 1;

		// Play phase 1 gameplay (06PLAY1.SAN)
		debug("Rebel2: Level 6 Phase 1");
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV06/06PLAY1.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit()) return kLevelQuit;

		if (_playerShield > 0) {
			phase1Score = _playerScore;
			break;
		}

		// Died in phase 1
		debug("Rebel2: Level 6 Phase 1 death at frame %d", _deathFrame);
		playLevelDeathVariant(6, 1, _deathFrame);
		if (_vm->shouldQuit()) return kLevelQuit;

		_playerLives--;
		if (_playerLives <= 0) {
			playLevelGameOver(6);
			return kLevelGameOver;
		}

		playLevelRetryVariant(6, 1);
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	if (_vm->shouldQuit()) return kLevelQuit;

	// Post segment 1 (06POST1.SAN)
	splayer->setCurVideoFlags(0x20);
	splayer->play("LEV06/06POST1.SAN", 12);
	if (_vm->shouldQuit()) return kLevelQuit;

	// ===== PHASE 2 retry loop =====
	_currentPhase = 2;

	while (!_vm->shouldQuit()) {
		_playerShield = 255;
		_playerDamage = 0;

		// Play phase 2 gameplay (06PLAY2.SAN)
		debug("Rebel2: Level 6 Phase 2");
		splayer->setCurVideoFlags(0x28);
		splayer->play("LEV06/06PLAY2.SAN", 12);
		_deathFrame = splayer->_frame;

		if (_vm->shouldQuit()) return kLevelQuit;

		if (_playerShield > 0) {
			// Level completed!
			debug("Rebel2: Level 6 completed!");
			playLevelEnd(6);
			_levelUnlocked[6] = true;  // Unlock level 7
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

		playLevelRetryVariant(6, 2);
		if (_vm->shouldQuit()) return kLevelQuit;
	}

	return kLevelQuit;
}

} // End of namespace Scumm
