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

#if !defined(SCUMM_INSANE_REBEL_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_INSANE_REBEL_H

#include "scumm/nut_renderer.h"

#include "scumm/smush/smush_player.h"

#include "scumm/insane/insane.h"

#include "common/keyboard.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/events.h"
#include "common/queue.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Audio {
class QueuingAudioStream;
class SoundHandle;
}

namespace Scumm {

class InsaneRebel2 : public Insane, public Common::EventObserver {

public:
	InsaneRebel2(ScummEngine_v7 *scumm);
	~InsaneRebel2();

	// EventObserver interface - captures events before ScummEngine consumes them
	bool notifyEvent(const Common::Event &event) override;

	// Menu input event queue - events are captured by notifyEvent() and processed by processMenuInput()
	Common::Queue<Common::Event> _menuEventQueue;
	bool _menuInputActive;  // True when we're capturing menu input events

	// ======================= Menu System =======================
	// Main game states (emulates retail state machine from FUN_004142BD)
	enum GameState {
		kStateIntro = 0,      // Stage 0: Intro/Credits sequence
		kStateMainMenu = 1,   // Stage 1: Main menu (FUN_004147B2)
		kStateLevelSelect = 2,// Stage 2: Level selection (FUN_00414A41)
		kStateBriefing = 3,   // Stage 3: Mission briefing (FUN_00415CF8)
		kStateGameplay = 4,   // Stage 4: Gameplay (FUN_00416787)
		kStateCredits = 5,    // Credits sequence
		kStateQuit = 6        // Exit game
	};

	// Menu selection results (return values from FUN_004147B2)
	enum MenuResult {
		kMenuNewGame = 2,     // case 0: New Game
		kMenuContinue = 4,    // case 1: Continue
		kMenuOptions = 0,     // case 2: Options (stays in menu)
		kMenuExit = 0,        // case 3: Exit to title
		kMenuUnknown = 0,     // case 4: Unknown function
		kMenuCredits = 1,     // case 5: Show credits
		kMenuQuit = 0         // case 6: Quit game
	};

	GameState _gameState;           // Current game state
	int _menuSelection;             // Current menu item (0-6), mirrors DAT_00459988
	int _menuItemCount;             // Number of menu items (7 for main menu)
	int _menuInactivityTimer;       // Timeout counter (300 frames = ~10 sec)
	int _lastMenuVariant;           // Last random menu video shown (DAT_00482400)
	int _menuRepeatDelay;           // Delay for key repeat (DAT_00459ce0)
	bool _levelUnlocked[16];        // Which levels are available (progress flags)

	// Run the main menu loop - returns when game should start or quit
	// This is the main entry point called from ScummEngine::go()
	int runMainMenu();

	// Process menu input (keyboard/mouse) - returns selected item or -1
	int processMenuInput();

	// Draw menu overlay (selection highlight) on current frame
	void drawMenuOverlay(byte *renderBitmap, int pitch, int width, int height);

	// Get random menu video filename (emulates FUN_0041FDC8)
	Common::String getRandomMenuVideo();

	// Reset menu state for fresh start
	void resetMenu();

	// ================= Level Selection Menu ====================
	// Level selection results
	enum LevelSelectResult {
		kLevelSelectBack = 0,     // Return to main menu
		kLevelSelectPlay = 1,     // Play selected level
		kLevelSelectQuit = 2      // Quit game
	};

	int _levelSelection;          // Current level selection (0-based)
	int _levelItemCount;          // Number of level items (levels + options)
	int _selectedLevel;           // Final selected level ID (1-15)

	// Run level selection menu - returns LevelSelectResult
	int runLevelSelect();

	// Draw level selection overlay
	void drawLevelSelectOverlay(byte *renderBitmap, int pitch, int width, int height);

	// Process level select input - returns -1 (no action) or action code
	int processLevelSelectInput();

	// ======================= Level Loading System =======================
	// Emulates the level handler functions FUN_00417E53 through FUN_0041BBE8
	// Each level has: BEG (intro), gameplay SANs, END (completion), DIE variants,
	// RETRY, and OVER (game over) videos.

	// Level playback result (returned by runLevel)
	enum LevelResult {
		kLevelCompleted = 0,      // Level completed successfully
		kLevelNextLevel = 1,      // Proceed to next level
		kLevelGameOver = 2,       // No lives remaining
		kLevelQuit = 3,           // Player quit
		kLevelReturnToMenu = 4    // Return to main menu
	};

	// Play the intro sequence (CREDITS/O_OPEN_C, O_OPEN_D, OPEN/O_OPEN_A, O_OPEN_B)
	// Emulates case 0 in FUN_004142BD
	void playIntroSequence();

	// Play the mission briefing video (OPEN/O_LEVEL.SAN)
	// Emulates FUN_00415CF8
	void playMissionBriefing();

	// Main level runner - plays a complete level by ID (1-15)
	// Handles all videos: BEG, gameplay, END/DIE/RETRY/OVER
	// Returns LevelResult
	int runLevel(int levelId);

	// Play level beginning cinematic (LEVXX/XXBEG.SAN)
	void playLevelBegin(int levelId);

	// Play main gameplay video(s) for a level
	// Returns true if level completed, false if player died
	bool playLevelGameplay(int levelId);

	// Play level completion video (LEVXX/XXEND.SAN)
	void playLevelEnd(int levelId);

	// Play death video (LEVXX/XXDIE_X.SAN) - variant based on frame/location
	void playLevelDeath(int levelId);

	// Play retry prompt video (LEVXX/XXRETRY.SAN)
	void playLevelRetry(int levelId);

	// Play game over video (LEVXX/XXOVER.SAN)
	void playLevelGameOver(int levelId);

	// Play the ending/credits sequence
	void playCreditsSequence();

	// Get the directory name for a level (e.g., "LEV01" for level 1)
	Common::String getLevelDir(int levelId);

	// Get the file prefix for a level (e.g., "01" for level 1)
	Common::String getLevelPrefix(int levelId);

	// Per-level handlers (emulate FUN_00417E53 through FUN_0041BBE8)
	// These implement the complete level logic including retry handling
	int runLevel1();   // FUN_00417E53 - Single gameplay phase
	int runLevel2();   // FUN_00418063 - Multiple parts with P1/P2/P3 subdirs
	int runLevel3();   // FUN_0041885F - Two phases with per-phase retry
	int runLevel4();   // Cutscene + single gameplay
	int runLevel5();   // FUN_00418EC6 - Single gameplay phase
	int runLevel6();   // FUN_00419317 - Two phases with per-phase retry

	// Random number helper (emulates FUN_004233a0)
	int getRandomVariant(int max);

	// Select death video variant based on level, phase, and frame
	// Returns suffix like "A", "B", "C" for DIE_X.SAN
	Common::String selectDeathVideoVariant(int levelId, int phase, int frame);

	// Play cinematic video by filename
	void playCinematic(const char *filename);

	// Play death video with proper variant selection
	void playLevelDeathVariant(int levelId, int phase, int frame);

	// Play retry video (phase-specific for multi-phase levels)
	void playLevelRetryVariant(int levelId, int phase);

	// Level state tracking for multi-phase levels
	int _currentPhase;        // Current gameplay phase (1, 2, 3 for Level 2; 1, 2 for Level 3/6)
	int _deathFrame;          // Frame number where player died (for death video selection)
	int _phaseScore;          // Accumulated score from previous phases (preserved on phase retry)
	int _phaseMisses;         // Accumulated misses from previous phases

	// =============================================================

	NutRenderer *_smush_cockpitNut;

	// Font used for HUD score/lives/damage display (SMALFONT.NUT)
	// DAT_00482200 equivalent - used by FUN_0041c012 for status bar rendering
	NutRenderer *_smush_dispfontNut;

	// Font used for opcode 9 text/subtitle rendering (DIHIFONT / TALKFONT)
	SmushFont *_rebelMsgFont;

	// Menu system fonts (from info.md - FUN_403BD0 font loading)
	// Low resolution mode font list (stored in DAT_00485058 linked list):
	//   Font 0 (^f00): TALKFONT.NUT - Default menu font
	//   Font 1 (^f01): SMALFONT.NUT - Small font (for format code switching)
	//   Font 2 (^f02): TITLFONT.NUT - Title font
	//   Font 3 (^f03): POVFONT.NUT - POV font
	NutRenderer *_smush_talkfontNut;   // Font 0 - primary menu font (DAT_00485058)
	NutRenderer *_smush_smalfontNut;   // Font 1 - small font for ^f01 switching
	NutRenderer *_smush_titlefontNut;  // Font 2 - title font

	// SmushFont for menu text rendering (uses SMALFONT.NUT with proper string drawing)
	SmushFont *_menuFont;

	// MSTOVER.NUT - Mouse Over background overlay (NOT a cursor!)
	// Loaded into DAT_0047aba8 and rendered via FUN_004236e0 as background
	NutRenderer *_smush_mouseoverNut;

	bool _introCursorPushed; // true when we've pushed an invisible cursor for intro
	

	int32 processMouse() override;
	bool isBitSet(int n) override;
	void setBit(int n) override;

	void iactRebel2Scene1(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
				  int16 par1, int16 par2, int16 par3, int16 par4);

	// Handle IACT opcode subcases
	void iactRebel2Opcode2(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);
	void iactRebel2Opcode3(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);
	void iactRebel2Opcode6(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4);
	void iactRebel2Opcode8(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4);
	void iactRebel2Opcode9(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);

	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
							   int32 setupsan13, int32 curFrame, int32 maxFrame) override;

	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
					  int16 par1, int16 par2, int16 par3, int16 par4) override;

	// ======================= Rendering Helper Functions =======================
	// These are extracted from procPostRendering for better readability

	// Fill status bar background area (FUN_004288c0 equivalent)
	void renderStatusBarBackground(byte *renderBitmap, int pitch, int width, int height,
								   int videoWidth, int videoHeight, int statusBarY);

	// Draw NUT-based HUD overlays for Handler 0x26/0x19 turret modes
	void renderTurretHudOverlays(byte *renderBitmap, int pitch, int width, int height, int32 curFrame);

	// Draw embedded SAN HUD overlays from IACT chunks
	void renderEmbeddedHudOverlays(byte *renderBitmap, int pitch, int width, int height);

	// Draw DISPFONT.NUT status bar sprites (FUN_0041c012 equivalent)
	void renderStatusBarSprites(byte *renderBitmap, int pitch, int width, int height,
								int statusBarY, int32 curFrame);

	// Draw Handler 7 ship sprite (space flight - FLY sprites)
	void renderHandler7Ship(byte *renderBitmap, int pitch, int width, int height);

	// Draw Handler 8 ship sprite (third-person vehicle - POV sprites)
	void renderHandler8Ship(byte *renderBitmap, int pitch, int width, int height);

	// Draw fallback ship using embedded HUD frame
	void renderFallbackShip(byte *renderBitmap, int pitch, int width, int height);

	// Draw enemy indicator brackets and erase destroyed enemy areas
	void renderEnemyOverlays(byte *renderBitmap, int pitch, int width, int height, int videoWidth);

	// Draw explosion animations from 5-slot system
	void renderExplosions(byte *renderBitmap, int pitch, int width, int height);

	// Draw laser shot beams and impacts
	void renderLaserShots(byte *renderBitmap, int pitch, int width, int height);

	// Update target lock state and draw crosshair/reticle
	void renderCrosshair(byte *renderBitmap, int pitch, int width, int height);

	// Reset enemy active flags and collision zones at frame end
	void frameEndCleanup();

	// ======================= Opcode 6 Helper Functions =======================
	// Handler-specific setup extracted from iactRebel2Opcode6

	// Handler 8 (third-person vehicle) setup - FUN_00401234 case 4
	void opcode6Handler8Setup(int16 par3, int16 par4);

	// Handler 7 (space flight) setup - FUN_0040c3cc case 4
	void opcode6Handler7Setup(int16 par3, int16 par4);

	// Calculate view offsets based on level type (lines 182-213)
	void opcode6CalcViewOffsets();

	// ======================= Opcode 8 Helper Functions =======================
	// Resource loading extracted from iactRebel2Opcode8

	// Load Handler 7 FLY NUT sprites from IACT data
	bool loadHandler7FlySprites(Common::SeekableReadStream &b, int64 remaining, int16 par4);

	// Load turret HUD overlay NUT from ANIM data
	bool loadTurretHudOverlay(byte *animData, int32 size, int16 par3);

	// Load Handler 8 ship POV NUT sprites from ANIM data
	bool loadHandler8ShipSprites(byte *animData, int32 size, int16 par3);

	// Load Level 2 background from embedded ANIM
	bool loadLevel2Background(byte *animData, int32 size, byte *renderBitmap);

	// Override procSKIP to disable Full Throttle's conditional frame skip mechanism
	// RA2 uses a different system for conditional frames via IACT opcodes
	void procSKIP(int32 subSize, Common::SeekableReadStream &b) override;

	// Override procPreRendering to restore Level 2 background before FOBJ decoding
	// This is called at the start of each frame, before FOBJ sprites are decoded
	void procPreRendering(byte *renderBitmap) override;

	void drawLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, byte color);
	// mask231: when true, color 231 is treated as transparent (legacy sprites). For laser beams set false.
	void drawTexturedLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, NutRenderer *nut, int spriteIdx, int v, bool mask231 = true);

	void drawLaserBeam(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, int progress, int maxProgress, int thickness, int param_9, NutRenderer *nut, int spriteIdx);
	void renderNutSprite(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx);

	struct enemy {
		int id;
		Common::Rect rect;
		bool active;
		bool destroyed;           // Set when enemy is shot - prevents re-activation
		int explosionFrame;       // Current explosion animation frame (0-32, -1 = done)
		bool explosionComplete;   // True when explosion animation has finished
		byte *savedBackground;    // Saved background pixels at moment of destruction
		int savedBgWidth;         // Width of saved background
		int savedBgHeight;        // Height of saved background
	};

	void init_enemyStruct(int id, int32 x, int32 y, int32 w, int32 h, bool active, bool destroyed, int32 explosionFrame);
	void enemyUpdate(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);

	Common::List<enemy> _enemies;
	
	// Current handler type for Rebel Assault 2 (determines crosshair sprite)
	// Handler 0: Background only
	// Handler 7: Space flight - uses crosshair sprite 0x2F (47)
	// Handler 8: Third-person vehicle - uses crosshair sprite 0x2E (46)  
	// Handler 0x19: Mixed/turret view - uses crosshair sprite 0x2F (47)
	// Handler 0x26: Full turret - crosshair varies by level type
	int _rebelHandler;
	
	// Level type from IACT opcode 6 par3 (corresponds to DAT_004436de)
	// Determines crosshair variant for turret mode:
	// - levelType == 5: Use sprites 0x30+ (48+) for crosshair
	// - levelType != 5: Use sprites 0-3 for crosshair (with animation)
	int _rebelLevelType;

	// Status bar sprite index (5 or 53) triggered by Opcode 6 par4
	// 0 = disabled
	int _rebelStatusBarSprite;

	// Embedded SAN HUD overlays (extracted from IACT chunks)
	// These are decoded frame buffers from embedded ANIM data
	// Slots 1-4 correspond to userId in the IACT wrapper
	struct EmbeddedSanFrame {
		byte *pixels;      // Decoded frame pixels (8-bit indexed)
		int width;         // Frame width
		int height;        // Frame height
		int renderX;       // X position to render (0 = centered based on slot)
		int renderY;       // Y position to render
		bool valid;        // True if this slot has valid data
	};
	
	EmbeddedSanFrame _rebelEmbeddedHud[16];  // HUD overlay slots (userId 0-15)
	
	// Check if a partial frame update should be skipped (overlaps with destroyed enemy)
	bool shouldSkipFrameUpdate(int left, int top, int width, int height) override;
	
	// Load and decode an embedded SAN animation from IACT chunk data
	// userId: HUD slot (1-4), animData: raw ANIM data, size: data size, renderBitmap: current frame buffer
	void loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) override;

	// ======================= Embedded Frame Codec Decoders =======================
	// These functions decode different codec formats used in embedded ANIM/FOBJ data
	// Based on retail FUN_0042C590 (codec 1), FUN_0042BD60 (codec 21), etc.

	// Decode codec 21/44 (Line Update) - skip/copy pairs per line
	// Used for fonts and some HUD frames (FUN_0042BD60)
	void decodeCodec21(byte *dst, const byte *src, int width, int height);

	// Decode codec 23 (Skip/Copy with embedded RLE) - hybrid format
	// Used for embedded HUD frames with transparency (FUN_0042BBF0)
	void decodeCodec23(byte *dst, const byte *src, int width, int height, int dataSize);

	// Decode codec 45 (RA2-specific BOMP RLE) - variable header format
	// Used for small animation elements and HUD pieces (FUN_0042B5F0)
	void decodeCodec45(byte *dst, const byte *src, int width, int height, int dataSize);

	// Render a decoded embedded frame to the video buffer
	// Handles transparency (color 0 and 231) and boundary checks
	void renderEmbeddedFrame(byte *renderBitmap, const EmbeddedSanFrame &frame, int userId);



	int16 _rebelLinks[512][3]; // Dependency links: Slot 0 (Disable on death), Slot 1/2 (Enable on death)
	void clearBit(int n);

	struct Explosion {
		int x, y;
		int width, height;
		int counter;     // Duration counter (starts at 10)
		int scale;       // Determines sprite set (small/med/large)
		bool active;
	};
	
	Explosion _explosions[5];
	void spawnExplosion(int x, int y, int objectHalfWidth);

	// ======================= Collision Zone System =======================
	// For Level 3 "pilot" ship obstacle avoidance (FUN_40E35E, FUN_40C3CC)
	// Collision zones are quadrilaterals defined by IACT Opcode 5
	// The player's ship position is tested against these zones each frame
	//
	// Zone Data Layout from IACT chunk:
	//   +0x00: opcode (5)
	//   +0x02: sub-opcode (0x0D = primary, 0x0E = secondary)
	//   +0x04: par3 (flags)
	//   +0x06: zoneType (e.g., 5 for damage zones)
	//   +0x08: frameStart
	//   +0x0A: frameEnd
	//   +0x0C-0x1A: X1,Y1,X2,Y2,X3,Y3,X4,Y4 vertex coordinates

	struct CollisionZone {
		int16 x1, y1;  // Top-left
		int16 x2, y2;  // Top-right
		int16 x3, y3;  // Bottom-right
		int16 x4, y4;  // Bottom-left
		int16 frameStart;
		int16 frameEnd;
		int16 zoneType;
		int16 subOpcode;  // 0x0D = primary, 0x0E = secondary
		bool active;
	};

	// Two zone tables matching retail DAT_0043fb00 (primary) and DAT_0043f9c8 (secondary)
	static const int kMaxCollisionZones = 5;
	CollisionZone _primaryZones[kMaxCollisionZones];    // Sub-opcode 0x0D zones
	CollisionZone _secondaryZones[kMaxCollisionZones];  // Sub-opcode 0x0E zones
	int _primaryZoneCount;
	int _secondaryZoneCount;

	// Corridor boundaries from IACT opcode 7 sub-opcodes 1 and 2
	int16 _corridorLeftX;    // DAT_00443b0a - Left X boundary
	int16 _corridorTopY;     // DAT_00443b0c - Top Y boundary
	int16 _corridorRightX;   // DAT_00443b0e - Right X boundary
	int16 _corridorBottomY;  // DAT_00443b10 - Bottom Y boundary

	// Hit cooldown timer (DAT_0044374c) - prevents rapid damage stacking
	int16 _hitCooldown;

	// Draw collision zone quadrilaterals for visualization/debugging
	// Called from procPostRendering when collision zones should be visible
	void drawCollisionZones(byte *dst, int pitch, int width, int height, byte color);

	// Draw a single quadrilateral (4 edges)
	void drawQuad(byte *dst, int pitch, int width, int height, 
	              int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, byte color);

	// Register a collision zone from IACT opcode 5 data
	void registerCollisionZone(Common::SeekableReadStream &b, int16 subOpcode);

	// Reset collision zone counters (called at end of frame)
	void resetCollisionZones();

	int16 _playerDamage;  // Legacy damage counter (kept for compatibility/telemetry)
	int16 _playerShield;  // Shields: 0..255 where 255 = full
	int16 _playerLives;
	int32 _playerScore;

	int _viewX;
	int _viewY;

	// Rebel per-level counters / flags mapped from retail globals
	int _rebelHitCounter;    // DAT_0047ab80 - hit counter / state tracker
	int _rebelHitCooldown;   // DAT_0045790a - cooldown / timing for damage checks
	bool _rebelInvulnerable; // DAT_0047ab64 - toggles invulnerability / state

	// Opcode 6 state variables (from FUN_41CADB case 4)
	int _rebelAutopilot;     // DAT_00457904 - autopilot flag (0 or 1)
	int _rebelDamageLevel;   // DAT_0045790a - damage level (0-5)
	int _rebelFlightDir;     // DAT_00457902 - flight direction (0 or 1)
	int _rebelControlMode;   // DAT_0047a7e4 - control mode flags

	// View offset variables (calculated from level type)
	int _rebelViewOffsetX;   // DAT_0045790c
	int _rebelViewOffsetY;   // DAT_0045790e
	int _rebelViewOffset2X;  // DAT_00457910
	int _rebelViewOffset2Y;  // DAT_00457912
	int _rebelViewMode1;     // DAT_00482270
	int _rebelViewMode2;     // DAT_00482274

	// Retail counters mirrored from DAT_00443618 (values 100..109) and DAT_004436e0 (mask counters 1..9)
	short _rebelValueCounters[10]; // Index 0 -> value 100, ... Index 9 -> 109
	short _rebelMaskCounters[10];  // Index 1..9 used; index 0 unused
	int _rebelLastCounter;         // Mirrors DAT_0047ab90 (last updated counter)


	struct Shot {
		bool active;
		int counter;
		int x, y;       // Target position
	};
	Shot _shots[2];
	void spawnShot(int x, int y);

	// ======================= Handler 8 Ship System =======================
	// For third-person vehicle missions (Levels 2, 3), the player controls a ship
	// that can turn in different directions. The ship sprite comes from
	// NUT files loaded via IACT opcode 8.
	//
	// Based on FUN_00401234 and FUN_00401ccf disassembly:
	// - DAT_0047e010: Primary ship sprite (POV001, subcase 1)
	// - DAT_0047e028: Secondary ship sprite (POV004, subcase 3)
	// - DAT_0047e020: Additional overlay (POV002, subcase 6)
	// - DAT_0047e018: Additional overlay (POV003, subcase 7)
	// - DAT_0043e006: Ship X position (raw, needs conversion for display)
	// - DAT_0043e008: Ship Y position (raw, needs conversion for display)
	// - DAT_0043e000: Level mode from opcode 6 par3

	NutRenderer *_shipSprite;        // DAT_0047e010 - Primary ship NUT
	NutRenderer *_shipSprite2;       // DAT_0047e028 - Secondary ship NUT
	NutRenderer *_shipOverlay1;      // DAT_0047e020 - Additional overlay
	NutRenderer *_shipOverlay2;      // DAT_0047e018 - Additional overlay

	// Level 2 background buffer (DAT_0047e030)
	// Loaded from IACT opcode 8, par4=5 - contains 320x200 background image
	// decoded from embedded ANIM in gameplay video frame 0
	byte *_level2Background;
	bool _level2BackgroundLoaded;

	// Ship position tracking (matches DAT_0043e006/008)
	// These are "raw" positions that get converted for display
	int16 _shipPosX;                 // DAT_0043e006
	int16 _shipPosY;                 // DAT_0043e008

	// Ship target positions (where ship is trying to move to)
	// Set from mouse/joystick input in opcode 6 processing
	int16 _shipTargetX;              // DAT_0043e002 - Target X
	int16 _shipTargetY;              // DAT_0043e004 - Target Y

	// Level mode for handler 8 (different from _rebelLevelType)
	// Set by opcode 6 par3, affects ship rendering behavior
	int16 _shipLevelMode;            // DAT_0043e000

	// Control mode for Handler 7 (space flight) - DAT_004437c0
	// Set by IACT opcode 6 par3 when handler is 7
	// Determines shooting capability and collision zone type:
	//   Mode 0: Flight/avoid mode - no shooting, uses secondary zones (sub-opcode 0x0E)
	//   Mode 1: Alternate flight mode - no shooting, uses primary zones (sub-opcode 0x0D)
	//   Mode 2: Combat mode - shooting ENABLED, crosshair shown, uses secondary zones
	// In Level 3's first sequence, par3=0 (no shooting - pure obstacle avoidance)
	// In combat sequences, par3=2 (shooting enabled)
	int16 _flyControlMode;           // DAT_004437c0

	// Ship firing state (from mouse button)
	bool _shipFiring;

	// Ship direction index for sprite selection (Handler 7)
	// Calculated from ship position: horizontal * 7 + vertical
	// horizontal: 0-4 (left to right), vertical: 0-6 (up to down)
	// Used to select which embedded HUD userId to render
	int16 _shipDirectionIndex;
	int16 _shipDirectionH;           // Horizontal direction (0-4, center=2)
	int16 _shipDirectionV;           // Vertical direction (0-6, center=3)

	// Helper to load a NUT file from IACT chunk data
	NutRenderer *loadNutFromIact(Common::SeekableReadStream &b, int dataSize);

	// ======================= Handler 7 FLY Ship System =======================
	// For space flight missions (Level 3, etc.), Handler 7 uses a 35-frame
	// direction-based ship sprite system. The ship visually banks and turns
	// based on player position using a 5x7 grid of sprites.
	//
	// Based on FUN_0040c3cc and FUN_0040d836 disassembly:
	// - DAT_0047fee8: Ship direction sprites (FLY001, par3=1, 35 frames)
	// - DAT_0047fef0: Laser fire sprites (FLY002, par3=3)
	// - DAT_0047fef8: Targeting overlay (FLY003, par3=2)
	// - DAT_0047ff00: High-res alternative (FLY004, par3=11)
	// - DAT_0044370c: Ship Y screen position
	// - DAT_0044370e: Ship X screen position

	NutRenderer *_flyShipSprite;     // DAT_0047fee8 - FLY001 (35 direction frames)
	NutRenderer *_flyLaserSprite;    // DAT_0047fef0 - FLY002
	NutRenderer *_flyTargetSprite;   // DAT_0047fef8 - FLY003
	NutRenderer *_flyHiResSprite;    // DAT_0047ff00 - FLY004

	// Handler 7 screen position (different from Handler 8's raw positions)
	int16 _flyShipScreenX;           // DAT_0044370e - Ship X screen position
	int16 _flyShipScreenY;           // DAT_0044370c - Ship Y screen position

	// ======================= Handler 0x26 Turret HUD Overlays =======================
	// For turret missions (Level 1, etc.), Handler 0x26 uses NUT-based HUD overlays
	// loaded via IACT opcode 8. These contain animated cockpit panel elements.
	//
	// Based on FUN_00407fcb and FUN_004089ab disassembly:
	// - DAT_0047fe78: Primary HUD overlay (GRD001/002, par3=1 or 2, 6 animation frames)
	// - DAT_0047fe80: Secondary HUD overlay (GRD010, par3=3 or 4, static or animated)
	//
	// Animation: The HUD overlay cycles through 6 sprite frames for blinking lights
	// Formula: spriteIndex = (frameCounter / 2) % 6
	//
	// Position formula (from FUN_004089ab lines 203-222):
	// X = 160 + (mouseOffsetX >> 4) - (width / 2) - spriteOffsetX
	// Y = 182 - (mouseOffsetY >> 4) - height - spriteOffsetY

	NutRenderer *_hudOverlayNut;     // DAT_0047fe78 - Primary HUD overlay (animated)
	NutRenderer *_hudOverlay2Nut;    // DAT_0047fe80 - Secondary HUD overlay

	/* Difficulty Level (0, 1, 2 = Easy, Med, Hard) */
	int _difficulty;
	void drawCornerBrackets(byte *dst, int pitch, int width, int height, int x, int y, int w, int h, byte color);

	// Score system (FUN_0041bf8d equivalent)
	// Adds points to score and awards bonus life when crossing threshold
	void addScore(int points);

	// Score lookup tables (indices into per-level point values)
	// DAT_0047e0fe: Points for destroying enemies
	// DAT_0047e100: Points for certain special events
	// DAT_0047e102: Points awarded per frame (time bonus)
	static const int16 kScoreTableEnemyDestroy[16];  // Per difficulty/level
	static const int16 kScoreTableSpecial[16];
	static const int16 kScoreTableTimeBonus[16];

	// Render score text to HUD (called from procPostRendering)
	void renderScoreHUD(byte *renderBitmap, int pitch, int width, int height, int statusBarY);

	// ======================= Pause Overlay =======================
	// Show pause overlay with dimming effect and "PAUSED" text
	// Emulates FUN_405A21 pause rendering (lines 242-305)
	void showPauseOverlay();

	// Target lock timer (DAT_00443676) - set to 7 when crosshair is over enemy
	int _targetLockTimer;

	// ========== Audio Handling ==========
	// Rebel Assault 2 doesn't use iMUSE - audio is handled directly here

	static const int kRA2MaxAudioTracks = 4;

	Audio::QueuingAudioStream *_audioStreams[kRA2MaxAudioTracks];
	Audio::SoundHandle _audioHandles[kRA2MaxAudioTracks];
	bool _audioTrackActive[kRA2MaxAudioTracks];
	int _audioSampleRate;

	// Initialize audio system for RA2
	void initAudio(int sampleRate);

	// Terminate audio system
	void terminateAudio();

	// Process audio dispatches - called from SmushPlayer when iMUSE is null
	// This replaces the iMUSE audio path for RA2
	void processAudioFrame(int16 feedSize);

	// Queue audio data for playback on a specific track
	void queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan);

};

} // End of namespace Insane

#endif
