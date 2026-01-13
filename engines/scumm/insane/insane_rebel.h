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

#include "common/list.h"
#include "common/rect.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Audio {
class QueuingAudioStream;
class SoundHandle;
}

namespace Scumm {

class InsaneRebel2 : public Insane {

public:
	InsaneRebel2(ScummEngine_v7 *scumm);
	~InsaneRebel2();

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
	void iactRebel2Opcode6(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);
	void iactRebel2Opcode8(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);
	void iactRebel2Opcode9(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);

	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
							   int32 setupsan13, int32 curFrame, int32 maxFrame) override;

	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
					  int16 par1, int16 par2, int16 par3, int16 par4) override;

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
	// Handler 8: Ground vehicle - uses crosshair sprite 0x2E (46)  
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
	
	EmbeddedSanFrame _rebelEmbeddedHud[5];  // Index 0 unused, 1-4 for userId slots
	
	// Check if a partial frame update should be skipped (overlaps with destroyed enemy)
	bool shouldSkipFrameUpdate(int left, int top, int width, int height) override;
	
	// Load and decode an embedded SAN animation from IACT chunk data
	// userId: HUD slot (1-4), animData: raw ANIM data, size: data size, renderBitmap: current frame buffer
	void loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) override;



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
