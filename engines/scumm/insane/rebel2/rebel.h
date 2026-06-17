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

#if !defined(SCUMM_INSANE_REBEL2_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_INSANE_REBEL2_H

#include "scumm/nut_renderer.h"

#include "scumm/smush/smush_player.h"

#include "scumm/insane/insane.h"
#include "scumm/insane/rebel/rebel_audio.h"
#include "scumm/insane/rebel/rebel_gamepad.h"

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
	bool _virtualKeyboardActive;

	// ---------------------------------------------------------------------------
	// Menu System
	// ---------------------------------------------------------------------------
	// Main game states (emulates original state machine from FUN_004142BD)
	enum GameState {
		kStateIntro = 0,        // Stage 0: Intro/Credits sequence
		kStateMainMenu = 1,     // Stage 1: Main menu (FUN_004147B2)
		kStatePilotSelect = 2,  // Stage 2: Pilot selection (FUN_00414A41)
		kStateChapterSelect = 3,// Stage 3: Chapter selection (FUN_00415CF8)
		kStateGameplay = 4,     // Stage 4: Gameplay (FUN_00416787)
		kStateCredits = 5,      // Credits sequence
		kStateQuit = 6,         // Exit game
		kStateTopPilots = 8,    // Top Pilots score display (FUN_00420116)
		kStateDifficultySelect = 7, // Difficulty submenu within pilot select
		kStateOptions = 9       // Options menu (FUN_004167A6)
	};

	// Menu selection results (return values from FUN_004147B2)
	enum MenuResult {
		kMenuQuit = -1,       // case 6: Return to Launcher
		kMenuResumeDemo = 0,  // case 3 / inactivity: Continue Intro
		kMenuCredits = 1,     // case 5: Show credits
		kMenuNewGame = 2      // case 0: Start Game
	};

	GameState _gameState;           // Current game state
	int _menuSelection;             // Current menu item (0-6), mirrors DAT_00459988
	int _menuItemCount;             // Number of menu items (7 for main menu)
	int _menuInactivityTimer;       // Timeout counter (300 frames = ~10 sec)
	bool _menuInactivityTimedOut;   // Main menu should return to the intro/demo loop
	int _lastMenuVariant;           // Last random menu video shown (DAT_00482400)
	int _menuRepeatDelay;           // Delay for key repeat (DAT_00459ce0)
	bool _menuSelectionConfirmed;   // True only when user explicitly confirmed a selection
	bool _levelUnlocked[16];        // Which levels are available (progress flags)

	// Run the main menu loop - returns when game should start or quit
	// This is the main entry point called from ScummEngine::go()
	int runMainMenu();

	// Process menu input (keyboard/mouse) - returns selected item or -1
	int processMenuInput();

	// Format-code-aware string rendering (^fNN=font, ^cNNN=color)
	// parseFormatCode: advances str past ^fNN/^cNNN/^^/^l codes.
	// Returns: fontIdx (>=0) on font change, -2 on color/newline, -1 on no match.
	static int parseFormatCode(const char *&str, int &outColor);

	int getMenuStringWidth(const char *str) const;
	void drawMenuString(byte *renderBitmap, const char *str, int x, int y, int defaultColor = 1);
	void drawMenuStringCentered(byte *renderBitmap, const char *str, int cx, int y, int defaultColor = 1);
	void drawMenuStringRight(byte *renderBitmap, const char *str, int rx, int y, int defaultColor = 1);

	// Shared menu item renderer - emulates FUN_0041F5AE
	void drawMenuItems(byte *renderBitmap, int pitch, int width, int height,
	                   const char **items, int numItems, int selection,
	                   bool leftAligned = false);

	// Draw menu overlay (selection highlight) on current frame
	void drawMenuOverlay(byte *renderBitmap, int pitch, int width, int height);

	// Get random menu video filename (emulates FUN_0041FDC8)
	Common::String getRandomMenuVideo();

	// Reset menu state for fresh start
	void resetMenu();
	bool isMenuTextInputActive() const;
	void setVirtualKeyboardVisible(bool visible);
	void updateMenuVirtualKeyboard();

	// ---------------------------------------------------------------------------
	// Chapter Selection Screen (FUN_00415CF8)
	// ---------------------------------------------------------------------------
	// Actual level/chapter selection screen with preview thumbnail.
	// Distinct from pilot selection (FUN_00414A41).

	enum ChapterSelectResult {
		kChapterSelectBack = 2,   // Return to main menu (ESC or BACK selected)
		kChapterSelectPlay = 5,   // Play selected chapter
		kChapterSelectQuit = 0    // Quit game
	};

	int _chapterSelection;        // Current chapter selection (0-15, or 16 for BACK)
	int _chapterItemCount;        // Number of chapter items (17: 16 chapters + BACK)
	int _selectedChapter;         // Final selected chapter ID (0-15)
	Common::String _passwordInput; // Current password input string (max 8 chars)
	bool _chapterUnlocked[16];    // Which chapters are unlocked
	bool _debugUnlockAll;         // Debug flag to unlock all chapters for testing
	bool _noDamage;               // Game option: disable player damage

	// Unlock all chapters for testing (debug mode)
	// Call this to enable access to all chapters without passwords
	void unlockAllChapters();

	// Run chapter selection screen - emulates FUN_00415CF8
	int runChapterSelect();

	// Draw chapter selection overlay - called during O_LEVEL.SAN playback
	void drawChapterSelectOverlay(byte *renderBitmap, int pitch, int width, int height);

	// Process chapter select input - returns -1 (no action) or action code
	int processChapterSelectInput();

	// Draw the preview thumbnail box - emulates FUN_004292D0 calls in FUN_00415CF8
	void drawPreviewBox(byte *renderBitmap, int pitch, int width, int height);

	// View offset for chapter preview scrolling (DAT_0047abe2/DAT_0047abe4)
	int16 _previewOffsetX;   // X offset = -90 for chapter select
	int16 _previewOffsetY;   // Y offset = chapter * -50 + 75

	// Draw score/info display at bottom of chapter select - emulates FUN_00434cb0 calls
	void drawChapterInfoLine(byte *renderBitmap, int pitch, int width, int height);

	// Rating-to-medal string conversion (FUN_0042001f)
	Common::String getRankString(int rating);

	// Password table lookup (FUN_0041BCE0)
	Common::String getChapterPassword(int level, int difficulty);

	// ---------------------------------------------------------------------------
	// Top Pilots Screen (FUN_00420116)
	// ---------------------------------------------------------------------------
	// Ranked pilot scores with animated reveal, played over menu video.
	// Original: DAT_00443b58 ranking table, 0x4a-byte records, max 15 entries.

	static const int kMaxRankings = 15;

	struct RankingEntry {
		char name[40];       // +0x04: Pilot name (or "-----" for defaults)
		int32 score;         // +0x36: Total score
		int32 rating;        // +0x3a: Total rating (converted to rank medals)
		int16 difficulty;    // +0x3e: Difficulty tier (0-5), TRS index = difficulty + 155
		int16 chapter;       // +0x40: Highest chapter completed (1-15)
	};

	RankingEntry _rankings[kMaxRankings];
	int _numRankings;

	// Initialize ranking table with defaults (FUN_0040FF00)
	void initDefaultRankings();

	// Insert pilot score into sorted ranking table (FUN_00410271)
	void insertRanking(const char *name, int32 score, int32 rating, int16 difficulty, int16 chapter);

	// Run top pilots display - called from main menu "Show Top Pilots"
	void showTopPilots();

	// Draw top pilots overlay on current frame during video playback
	void drawTopPilotsOverlay(byte *renderBitmap, int pitch, int width, int height);

	int _topPilotsFrameCount;     // Animation frame counter (pilots revealed one per frame)
	int _topPilotsMaxFrames;      // Total frames to display (120 or 240)

	// ---------------------------------------------------------------------------
	// Options Menu (FUN_004167A6)
	// ---------------------------------------------------------------------------
	// TRS strings: 89 (title), 90-101 (toggle labels), 103 (volume), 107/109 (back)
	// Original settings array at DAT_00482e20[0..4]:
	//   [0]=text, [1]=music, [2]=voices, [3]=sound, [4]=hidden abort flag
	// Additional flags: DAT_0047a7fe (controls normal/flipped), DAT_0047a80a (rapid fire)
	// Volume: DAT_0047a804 (0-127), brightness/gamma: DAT_0047a802 (127-768)

	void showOptionsMenu();
	void drawOptionsOverlay(byte *renderBitmap, int pitch, int width, int height);
	int processOptionsInput();

	int _optionsSelection;
	int _optionsItemCount;
	bool _optionsExitRequested;
	bool _optMusicEnabled;
	bool _optSfxEnabled;
	bool _optVoicesEnabled;
	bool _optTextEnabled;
	bool _optControlsFlipped;
	bool _optRapidFire;
	int _optVolumeLevel;       // 0-127 (DAT_0047a804)

	// ---------------------------------------------------------------------------
	// Pilot Data System (FUN_00411B9A / FUN_00411980 / FUN_00411A5D)
	// ---------------------------------------------------------------------------
	// 10 pilot slots × 0x118 (280) bytes at DAT_004568A8.
	// Stored via SaveFileManager in a custom save file.

	static const int kMaxPilots = 10;
	static const int kMaxPilotNameLen = 15;
	static const int kNumLevels = 16;

	struct PilotData {
		char name[kMaxPilotNameLen + 1]; // +0x04: Pilot name (15 chars + null)
		int32 score[kNumLevels];         // +0x2C: Per-level score (0 = default, 0xFF = unplayed)
		int32 lives[kNumLevels];         // +0x6C: Per-level lives (4 = default, 0xFF = unplayed)
		int32 damage[kNumLevels];        // +0xAC: Per-level damage (0xFF = unplayed)
		int16 rating[kNumLevels];        // +0xEC: Per-level difficulty rating (0-50)
		int16 difficulty;                // +0x10C: Difficulty setting (0-5)

		void init() {
			memset(name, 0, sizeof(name));
			memset(rating, 0, sizeof(rating));
			difficulty = 2; // Default to 3rd option
			score[0] = 0;
			lives[0] = 4;
			damage[0] = 0;
			for (int i = 1; i < kNumLevels; i++) {
				score[i] = 0;
				lives[i] = 0xFF;
				damage[i] = 0xFF;
			}
		}
	};

	PilotData _pilots[kMaxPilots];       // DAT_004568A8 pilot array
	int _numPilots;                      // DAT_00480318 number of valid pilots
	int _activePilot;                    // DAT_0047a7ea selected pilot index

	// Pilot save/load via SaveFileManager
	bool loadPilots();
	bool savePilots();

	// Pilot management (FUN_00411B9A, FUN_00411D29)
	int createNewPilot();                // Returns index of new pilot, or -1 if full
	void deletePilot(int index);         // FUN_00411D29: shift remaining pilots down
	void copyPilot(int srcIndex);        // Copy pilot to new slot

	// Update pilot progress after level completion
	void updatePilotProgress(int levelIndex, int32 score, int32 lives, int32 damage);

	// ---------------------------------------------------------------------------
	// Pilot Selection Menu (FUN_00414A41)
	// ---------------------------------------------------------------------------
	// Pilot/save selection menu (separate from chapter selection).

	enum LevelSelectResult {
		kLevelSelectBack = 0,     // Return to main menu
		kLevelSelectPlay = 1,     // Play selected level
		kLevelSelectQuit = 2      // Quit game
	};

	// Pilot name input state (for NEW PILOT name entry)
	enum PilotMenuMode {
		kPilotModeSelect = 0,     // Normal pilot list selection
		kPilotModeNameInput = 1,  // Typing a new pilot name
		kPilotModeDifficulty = 2, // Difficulty submenu
		kPilotModeCopySelect = 3, // Selecting source pilot to copy
		kPilotModeDeleteSelect = 4 // Selecting pilot to delete
	};
	PilotMenuMode _pilotMenuMode;
	Common::String _pilotNameInput;      // Current name being typed
	int _pilotEditIndex;                 // Index of pilot being edited/created

	int _levelSelection;          // Current level selection (0-based)
	int _levelItemCount;          // Number of level items (levels + options)
	int _selectedLevel;           // Final selected level ID (1-15)
	int _difficultySelection;     // Current difficulty selection in submenu (0-based)

	// Run pilot selection menu - emulates FUN_00414A41
	int runLevelSelect();

	// Draw pilot selection overlay
	void drawLevelSelectOverlay(byte *renderBitmap, int pitch, int width, int height);

	// Process pilot select input - returns -1 (no action) or action code
	int processLevelSelectInput();

	// ---------------------------------------------------------------------------
	// Level Loading System
	// ---------------------------------------------------------------------------
	// Emulates the level handler functions FUN_00417E53 through FUN_0041BBE8.
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

	// Main game entry point — full game loop (intro, menu, pilot, chapter, levels)
	// Emulates the original game flow from FUN_004142BD
	void runGame();

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

	// Play level completion video (LEVXX/XXEND.SAN)
	void playLevelEnd(int levelId);

	// Play retry prompt video (LEVXX/XXRETRY.SAN)
	void playLevelRetry(int levelId);

	// Play game over video (LEVXX/XXOVER.SAN)
	void playLevelGameOver(int levelId);

	// Play the full ending sequence: finale + credits + epilogue (FUN_0041bbe8)
	void playEndingSequence();

	// Play main menu credits video (OPEN/O_CREDIT.SAN)
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
	int runLevel6();   // FUN_004190D6 - Two phases with mid-level video switch
	int runLevel7();   // FUN_0041974C - TIE Training: single + fork at frame 1592
	int runLevel8();   // FUN_00419976 - Flight to Imdaar: single phase space battle
	int runLevel9();   // FUN_00419B86 - Mine Field: single phase with mid-events
	int runLevel10();  // FUN_00419E0A - Speeder Bikes: single phase
	int runLevel11();  // FUN_0041A00C - Inside the Terror: 3 phases + bridge (Handler 8)
	int runLevel12();  // FUN_0041AA14 - Sewers: 4 phases FPS (Handler 25)
	int runLevel13();  // FUN_0041B3E1 - Escaping Star Destroyer: two-phase A→B
	int runLevel14();  // FUN_0041B6E8 - TIE Attack: single phase
	int runLevel15();  // FUN_0041B8D7 - Imdaar Alpha: single + level ID switch

	// Wave state management (FUN_00417b61)
	// Waits for current video to finish, accumulates kill state, redistributes
	// kill credits from the budget.
	// mask: required enemy bits (0x36 for Phase 1, 0x3e for Phases 2/3)
	// budget: kill credit budget counter (decremented per credit transfer)
	// threshold: early-exit frame threshold (0=disabled, 0x14=20 for wave loops)
	// flags: bit 1 = add random unkilled types, bit 0 = limit credits to 2 (else 8)
	struct WaveEndResult {
		WaveEndResult() : creditedBits(0), died(false), quit(false), completed(false), skipped(false) {}

		bool shouldStop() const { return died || quit || completed || skipped; }

		uint16 creditedBits;
		bool died;
		bool quit;
		bool completed;
		bool skipped;
	};

	WaveEndResult processWaveEnd(int16 mask, int16 *budget, int16 threshold, uint16 flags);

	// Play a raw SAN segment from a scripted level handler.
	// The original reaches these call sites through different wrappers/direct paths; this
	// only collapses the shared dispatch step. Callers still choose the original
	// flags and when to call processWaveEnd(). recordFrame preserves the original
	// split between gameplay/wave calls and transition/init-only segments.
	bool playLevelSegment(const char *filename, uint16 flags, bool recordFrame = true);
	void enableIOSGamepadController();
	void restoreIOSGamepadController();

	int calculateAccuracy(int kills, int misses) const;

	// Play DIE/OVER/RETRY tail. Returns true when the caller should restart its retry loop.
	bool handleLevelDeath(int levelId, int phase, const char *deathVideo, const char *retryVideo, int &levelResult);
	void resetLevelAttemptState(int initialPhase);
	void resetLevelPhaseState(bool clearEnemies);
	void clearEmbeddedHudFrames();
	void resetLevelWaveState();
	void resetExplosions();
	void resetHandler7FlightState();

	// Random number helper (emulates FUN_004233a0)
	int getRandomVariant(int max);

	// Select death video variant based on level, phase, and frame
	// Returns suffix like "A", "B", "C" for DIE_X.SAN
	Common::String selectDeathVideoVariant(int levelId, int phase, int frame);

	// Play cinematic video by filename
	void playCinematic(const char *filename);

	// Play cinematic with text overlay (emulates FUN_004171c5)
	// Text is progressively revealed during [fadeInFrame, fadeOutFrame)
	void playVideoWithText(const char *filename, int textID, int textX, int textY,
	                       int fadeInFrame, int fadeOutFrame);

	// Text overlay state (active during playVideoWithText cinematics)
	bool _textOverlayActive;      // True when text overlay should render
	int _textOverlayID;           // TRS string ID
	int _textOverlayX;            // X position for text rendering
	int _textOverlayY;            // Y position for text rendering
	int _textOverlayFadeIn;       // Frame to start progressive text reveal
	int _textOverlayFadeOut;      // Frame to stop text rendering

	// Render chapter title text overlay (emulates FUN_004341a0 in FUN_004171c5)
	void renderTextOverlay(byte *renderBitmap, int pitch, int width, int height, int curFrame);

	// Play death video with proper variant selection
	void playLevelDeathVariant(int levelId, int phase, int frame);

	// Play retry video (phase-specific for multi-phase levels)
	void playLevelRetryVariant(int levelId, int phase);

	// Reset gameplay aim to the original centered mouse/joystick baseline.
	void centerGameplayAim();
	// Tracks consecutive recorded gameplay SANs so wave-loop videos do not recenter aim.
	bool _gameplaySectionActive;
	RebelIOSGamepadControllerState _iosGamepadControllerState;

	// Level state tracking for multi-phase levels
	int _currentPhase;        // Current gameplay phase (1, 2, 3 for Level 2; 1, 2 for Level 3/6)
	int _deathFrame;          // Frame number where player died (for death video selection)
	bool _skipSectionRequested; // Debug shortcut (Shift+S): force current gameplay section to end

	// ---------------------------------------------------------------------------
	// Resources and Fonts
	// ---------------------------------------------------------------------------

	NutRenderer *_smush_cockpitNut;

	// Font used for opcode 9 text/subtitle rendering (DIHIFONT / TALKFONT)
	NutRenderer *_rebelMsgFont;

	// Menu system fonts (from info.md - FUN_403BD0 font loading)
	// Low resolution mode font list (stored in DAT_00485058 linked list):
	//   Font 0 (^f00): TALKFONT.NUT - Default menu font
	//   Font 1 (^f01): SMALFONT.NUT - Small font (for format code switching)
	//   Font 2 (^f02): TITLFONT.NUT - Title font
	//   Font 3 (^f03): POVFONT.NUT - POV font
	NutRenderer *_smush_talkfontNut;   // Font 0 - primary menu font (DAT_00485058)
	NutRenderer *_smush_smalfontNut;   // Font 1 - small font for ^f01 switching
	NutRenderer *_smush_titlefontNut;  // Font 2 - title font
	NutRenderer *_smush_povfontNut;    // Font 3 - POV font for Handler 8 overlay

	// Saved palette for pause overlay restoration (FUN_405A21)
	byte _savedPausePalette[768];
	bool _pauseOverlayActive;

	bool _introCursorPushed; // true when we've pushed an invisible cursor for intro


	int32 processMouse() override;
	Common::Point getGameplayAimPoint();
	Common::Point getRebelAutoPlayAimPoint();
	// Per-frame: pan the gameplay reticle incrementally from the held directional controls
	// (on-screen/physical gamepad dpad, keyboard arrows) instead of snapping it to a screen
	// edge. Call once per frame; getGameplayAimPoint() stays a pure getter.
	void updateGameplayAimFromGamepad();
	void warpGameplayMouseNow(int x, int y);

	// Analog stick state, ingested from EVENT_CUSTOM_BACKEND_ACTION_AXIS in notifyEvent()
	// and read (with a deadzone) by updateGameplayAimFromGamepad(). Signed; 0 = centered.
	int16 _joystickAxisX;
	int16 _joystickAxisY;

	// True once the gamepad has driven the gameplay reticle, until a genuine mouse/touch
	// motion takes over. While set, notifyEvent() drops stray pointer events so they can't
	// recenter the reticle (the cursor is locked ~center during gameplay, so a gamepad
	// "click" lands at center). See notifyEvent()/updateGameplayAimFromGamepad().
	bool _gamepadAimActive;
	// Suppress delayed lock/warp mousemove artifacts after centering Handler 7 gameplay.
	uint32 _gameplayMouseSettleUntil;
	uint32 _lastGameplayMenuCloseTime;
	uint32 _lastMenuGamepadNavigationTime;
	void openGameplayMainMenu(SmushPlayer *splayer);
	void openMenuMainMenu(SmushPlayer *splayer);
	bool isBitSet(int n) override;
	void setBit(int n) override;

	// Get current handler ID (8, 25, 38 etc.) for SMUSH player to query
	int getHandler() const { return _rebelHandler; }
	int getHandler25GrdSpriteMode() const { return _grdSpriteMode; }
	bool isHiRes() const;

	void iactRebel2Scene1(byte *renderBitmap, int32 codecparam, int32 setupsan12,
				  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
				  int16 par1, int16 par2, int16 par3, int16 par4);

	// Handle IACT opcode subcases
	void iactRebel2Opcode2(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);
	void iactRebel2Opcode3(Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);
	void iactRebel2Opcode6(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4);
	void updateOpcode6Handler(int16 par2);
	void handleOpcode6Handler8(Common::SeekableReadStream &b, int16 par4);
	void handleOpcode6Handler7(Common::SeekableReadStream &b, int16 par4);
	void handleOpcode6Handler25(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);
	void handleOpcode6Turret(Common::SeekableReadStream &b, int16 par4);
	void handleOpcode6GenericInit(int16 par4);
	void updateOpcode6GenericFlightState();
	void scanOpcode6EmbeddedAnim(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par4);
	void iactRebel2Opcode8(byte *renderBitmap, Common::SeekableReadStream &b, int32 chunkSize, int16 par2, int16 par3, int16 par4);
	bool loadOpcode8Handler7FlySprites(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);
	bool loadOpcode8Handler7ShotTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);
	bool loadOpcode8EdgeTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);
	bool loadOpcode8AuxSfx(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);
	bool loadOpcode8ShotOriginTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);
	void loadOpcode8EmbeddedAnim(byte *renderBitmap, Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par3, int16 par4);
	bool handleOpcode8EmbeddedAnim(byte *renderBitmap, byte *animData, int32 animDataSize, int16 par3, int16 par4);
	void iactRebel2Opcode9(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);

	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
							   int32 setupsan13, int32 curFrame, int32 maxFrame) override;

	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
					  int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
					  int16 par1, int16 par2, int16 par3, int16 par4) override;

	// ---------------------------------------------------------------------------
	// Rendering Helper Functions
	// ---------------------------------------------------------------------------
	// Extracted from procPostRendering for better readability.

	// Fill status bar background area (FUN_004288c0 equivalent)
	void renderStatusBarBackground(byte *renderBitmap, int pitch, int width, int height,
								   int videoWidth, int videoHeight, int statusBarY);

	void updatePostRenderScroll(int width, int height);
	void updatePostRenderDeath();
	void renderPostRenderMenuCursor(byte *renderBitmap, int pitch, int width, int height);
	bool handlePostRenderMenuModes(byte *renderBitmap, int pitch, int width, int height, bool introPlaying);
	bool handlePostRenderIntro(byte *renderBitmap, int pitch, int width, int height, int32 curFrame);
	void renderGameplayPostFrame(byte *renderBitmap, int pitch, int width, int height,
								 int videoWidth, int videoHeight, int statusBarY, int32 curFrame);
	void updateGameplayDamageEffects(byte *renderBitmap, int pitch, int width, int height);
	void updateGameplayDamageRecovery(int32 curFrame);
	void checkGameplayPostRenderCollisions(byte *renderBitmap, int pitch, int width, int height, int32 curFrame);

	// Draw NUT-based HUD overlays for Handler 0x26 turret modes
	void renderTurretHudOverlays(byte *renderBitmap, int pitch, int width, int height, int32 curFrame);

	// Draw embedded SAN HUD overlays from IACT chunks
	void renderEmbeddedHudOverlays(byte *renderBitmap, int pitch, int width, int height);

	// Draw DISPFONT.NUT status bar sprites (FUN_0041c012 equivalent)
	void renderStatusBarSprites(byte *renderBitmap, int pitch, int width, int height,
								int statusBarY, int32 curFrame);

	// Draw Handler 7 ship sprite (third-person ship - FLY sprites)
	void renderHandler7Ship(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler7FlySprite(byte *renderBitmap, int pitch, int width, int height,
		bool renderHiRes, int renderScale, int nativeViewX, int nativeViewY,
		int nativeX, int nativeY, NutRenderer *nut, int spriteIndex);

	// Draw Handler 8 ship sprite (third-person on foot - POV sprites)
	void renderHandler8Ship(byte *renderBitmap, int pitch, int width, int height);
	void renderVehicleShotImpacts(byte *renderBitmap, int pitch, int width, int height); // FUN_402DA8 (Handler 8)

	// Draw fallback ship using embedded HUD frame
	void renderFallbackShip(byte *renderBitmap, int pitch, int width, int height);

	// Draw per-enemy target indicators from the cockpit icon sheet.
	void renderEnemyOverlays(byte *renderBitmap, int pitch, int width, int height, int videoWidth);

	// Draw explosion animations from 5-slot system (dispatcher)
	void renderExplosions(byte *renderBitmap, int pitch, int width, int height);

	// Draw laser shot beams and impacts
	void renderLaserShots(byte *renderBitmap, int pitch, int width, int height);

	// Update target lock state and draw crosshair/reticle
	void renderCrosshair(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler8MonitorEffect(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler8PovOverlay(byte *renderBitmap, int pitch, int width, int height);
	Common::Point getHandler7ShipDrawPoint();
	Common::Point getHandler7ProjectedPoint();
	Common::Point getHandler7ShotTargetPoint();
	Common::Point getHandler8ShotTargetPoint();

	// Reset enemy active flags and collision zones at frame end
	void frameEndCleanup();

	// ---------------------------------------------------------------------------
	// Opcode 8 Helper Functions
	// ---------------------------------------------------------------------------
	// Resource loading extracted from iactRebel2Opcode8.

	// Load Handler 7 FLY NUT sprites from IACT data
	bool loadHandler7FlySprites(Common::SeekableReadStream &b, int64 remaining, int16 par4);
	bool loadHandler7ShotTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining, int16 par4);

	// Load turret HUD overlay NUT from ANIM data
	bool loadTurretHudOverlay(byte *animData, int32 size, int16 selector);

	// Load Handler 8 ship POV NUT sprites from ANIM data (par4 = sprite type: 1,3,6,7)
	bool loadHandler8ShipSprites(byte *animData, int32 size, int16 par4);

	// Load Handler 25 GRD NUT sprites from ANIM data (par4 = sprite type: 1,2)
	bool loadHandler25GrdSprites(byte *animData, int32 size, int16 par4);

	// Parse Handler 25 shot-origin table text from opcode 8 (par4 = 8).
	// FUN_0041CADB stores values into DAT_004578a6 / DAT_004578c6 (indices 5..19).
	bool loadHandler25ShotOriginTable(Common::SeekableReadStream &b, int64 startPos, int64 remaining);

	// Load Level 2 background from embedded ANIM
	bool loadLevel2Background(byte *animData, int32 size, byte *renderBitmap);

	// Override procSKIP to disable Full Throttle's conditional frame skip mechanism
	// RA2 uses a different system for conditional frames via IACT opcodes
	void procSKIP(int32 subSize, Common::SeekableReadStream &b) override;

	// Override procPreRendering to restore Level 2 background before FOBJ decoding
	// This is called at the start of each frame, before FOBJ sprites are decoded
	void procPreRendering(byte *renderBitmap) override;

	// mask231: when true, color 231 is treated as transparent (legacy sprites). For laser beams set false.
	void drawTexturedLine(byte *dst, int pitch, int width, int height, int x0, int y0, int x1, int y1, NutRenderer *nut, int spriteIdx, int v, bool mask231 = true);

	// ---------------------------------------------------------------------------
	// Laser Texture Buffer (DAT_0047fee4)
	// ---------------------------------------------------------------------------
	// Pre-rendered laser texture used by FUN_0040BBF6.
	// Initialized from CPITIMAG.NUT sprite 0 via initLaserTexture() (FUN_0040BAB0)
	struct LaserTexture {
		byte *pixels;      // Pixel data (rendered from NUT sprite)
		int16 width;       // Texture width
		int16 height;      // Texture height (clamped to max 15)
	};
	LaserTexture _laserTexture;  // DAT_0047fee4

	// ---------------------------------------------------------------------------
	// Edge Blend Table (DAT_0046a7d0)
	// ---------------------------------------------------------------------------
	// 256x256 lookup table used by drawEdgeHighlightLine() (FUN_410962) to compute
	// glow colors at beam edges. For each pixel on the beam edge, the table maps
	// [adjacent_pixel_above][adjacent_pixel_below] -> output color, producing a
	// color-blended highlight that gives beams their distinctive glow.
	//
	// Default table (FUN_410510 with NULL): table[a][b] = min(a,b) (identity/transparent).
	// Per-level tables loaded via IACT opcode 8 par4=1000 (FUN_405663 -> FUN_410510).
	// The per-level table tunes the glow to the current palette (red for some levels,
	// green for others).
	//
	// _rebelDetailMode (DAT_0047a7fc): Controls whether edge highlights are drawn.
	// Set from IACT opcode 6. When >= 0, edge highlights are enabled.
	byte _edgeTable[256 * 256];       // DAT_0046a7d0 - primary edge blend table
	int16 _rebelDetailMode;           // DAT_0047a7fc - edge highlight enable flag

	// Initialize edge blend table (FUN_410510)
	// data == nullptr: fill with default identity table (min(a,b))
	// data != nullptr: load 256x256 symmetric table from data (skips 8-byte header)
	void initEdgeTable(const byte *data);

	// Draw edge highlight line using the edge blend table (FUN_410962)
	// Each pixel is blended from its perpendicular neighbors via _edgeTable lookup.
	// For horizontal-dominant beams, reads pixels above/below the line.
	// For vertical-dominant beams, reads pixels left/right of the line.
	void drawEdgeHighlightLine(byte *dst, int pitch, int width, int height,
	                           int16 x0, int16 y0, int16 x1, int16 y1,
	                           int16 clipLeft, int16 clipTop, int16 clipRight, int16 clipBottom);

	// Initialize laser texture from NUT sprite (FUN_0040BAB0)
	void initLaserTexture(NutRenderer *nut, int spriteIdx);
	void freeLaserTexture();

	// Draw laser beam using pre-initialized texture (FUN_0040BBF6)
	// Two-layer rendering: textured scanlines (beam body) + edge highlights (glow).
	// Edge highlights are only drawn when _rebelDetailMode >= 0.
	void drawLaserBeam(byte *dst, int pitch, int width, int height,
	                   int16 gunX, int16 gunY, int16 targetX, int16 targetY,
	                   int16 animFrame, int16 maxFrames,
	                   int16 widthScale, int16 heightScale, int16 thickness);
	void renderNutSprite(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx);
	void renderNutSpriteMirrored(byte *dst, int pitch, int width, int height, int x, int y, NutRenderer *nut, int spriteIdx, bool mirror);

	struct enemy {
		int id;
		int type;                 // Enemy type/group from IACT opcode 4 par3 (determines DAT_0047ab98 bit)
		Common::Rect rect;
		bool active;
		bool destroyed;           // Set when enemy is shot - prevents re-activation
		int explosionFrame;       // Current explosion animation frame (0-32, -1 = done)
		bool explosionComplete;   // True when explosion animation has finished
		byte *savedBackground;    // Saved background pixels at moment of destruction
		int savedBgWidth;         // Width of saved background
		int savedBgHeight;        // Height of saved background
	};

	void initEnemyStruct(int id, int32 x, int32 y, int32 w, int32 h, bool active, bool destroyed, int32 explosionFrame, int type = 0);
	void enemyUpdate(byte *renderBitmap, Common::SeekableReadStream &b, int16 par2, int16 par3, int16 par4);

	Common::List<enemy> _enemies;

	// Current handler type for Rebel Assault 2 (determines crosshair sprite)
	// Handler 0: Background only
	// Handler 7: Third-Person Ship - uses crosshair sprite 0x2F (47)
	// Handler 8: Third-Person On Foot - uses crosshair sprite 0x2E (46)
	// Handler 0x19: FPS/Mixed view - uses crosshair sprite 0x2F (47)
	// Handler 0x26: Turret/Cockpit - crosshair varies by level type
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
	byte _rebelEmbeddedCodec45Palette[0x300];
	byte _rebelEmbeddedCodec45Lookup[0x8000];

	// Load and decode an embedded SAN animation from IACT chunk data
	// userId: HUD slot (1-4), animData: raw ANIM data, size: data size, renderBitmap: current frame buffer
	void loadEmbeddedSan(int userId, byte *animData, int32 size, byte *renderBitmap) override;

	// Render a decoded embedded frame to the video buffer
	// Handles transparency (color 0 and 231) and boundary checks
	void renderEmbeddedFrame(byte *renderBitmap, const EmbeddedSanFrame &frame, int userId);
	void drawHandler25CorridorOverlay(byte *renderBitmap);

	int16 _rebelLinks[512][3]; // Dependency links: Slot 0 (Disable on death), Slot 1/2 (Enable on death)
	void clearBit(int n);
	bool isShootingAllowed();  // FUN_0040d836/FUN_00401CCF: Check control mode before spawning shots

	struct Explosion {
		int x, y;
		int width, height;
		int counter;     // Duration counter (starts at 10)
		int scale;       // Determines sprite set (small/med/large)
		bool active;
	};

	enum ExplosionFrameAdvance {
		kExplosionAdvanceAfterDraw,
		kExplosionAdvanceBeforeDraw
	};

	Explosion _explosions[5];
	void spawnExplosion(int x, int y, int objectHalfWidth);

	// ---------------------------------------------------------------------------
	// Collision Zone System
	// ---------------------------------------------------------------------------
	// For Level 3 "pilot" ship obstacle avoidance (FUN_40E35E, FUN_40C3CC).
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
		int16 x1, y1;  // Vertex 1 (body[2], body[3])
		int16 x2, y2;  // Vertex 2 (body[4], body[5])
		int16 x3, y3;  // Vertex 3 (body[6], body[7])
		int16 x4, y4;  // Vertex 4 (body[8], body[9])
		int16 field1;   // body[0] - control field (frame check: field2 - 1 == field1)
		int16 field2;   // body[1] - control field
		int16 filterValue; // par4 from IACT header - used for < 1000 filter
		int16 subOpcode;   // 0x0D = primary, 0x0E = secondary
		bool active;
	};

	// Two zone tables matching DAT_0043fb00 (primary) and DAT_0043f9c8 (secondary)
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

	// Register a collision zone from IACT opcode 5 data
	void registerCollisionZone(Common::SeekableReadStream &b, int16 subOpcode, int16 par4);

	// Reset collision zone counters (called at end of frame)
	void resetCollisionZones();

	// Per-frame collision checking against registered zones (FUN_4092D9)
	// Tests aim/ship position against primary zone quadrilaterals
	// Applies collision damage from DAT_0047e0f6 when inside obstacle zone
	void checkCollisionZones(byte *renderBitmap, int pitch, int width, int height, int32 curFrame);

	// Handler 7 collision system (FUN_40E35E)
	// Mode 0/2: Obstacle collision using secondary zones — inside quad = hit
	// Mode 1/3: Wall/boundary collision using primary zones — per-edge push-back
	// Uses ship position (_flyShipScreenX/_flyShipScreenY) in raw buffer coords
	// and draws proximity shadow cues for nearby danger zones.
	void checkHandler7CollisionZones(byte *renderBitmap, int pitch, int width, int height, int32 curFrame);
	bool isHandler7ShipInsideObstacleZone(const CollisionZone &zone, int margin);
	void applyHandler7ObstacleHit(const CollisionZone &zone, int zoneIndex);
	void awardHandler7DodgeScore();
	void checkHandler7ObstacleZones(uint16 &warningMask);
	bool applyPlayerDamage(int damage);
	bool applyHandler7WallDamage(int wallDamage);
	void resetHandler7HorizontalVelocity(int16 velocity);
	void checkHandler7TopBoundary(const CollisionZone &zone, int16 vMargin, int wallDamage, uint16 &warningMask);
	void checkHandler7BottomBoundary(const CollisionZone &zone, int16 vMargin, int wallDamage, uint16 &warningMask);
	void checkHandler7LeftBoundary(const CollisionZone &zone, int16 hMargin, int wallDamage);
	void checkHandler7RightBoundary(const CollisionZone &zone, int16 hMargin, int wallDamage);
	void checkHandler7BoundaryZones(uint16 &warningMask);
	void renderHandler7WarningCues(byte *renderBitmap, int pitch, int width, int height, int32 curFrame, uint16 warningMask);

	int16 _playerDamage;  // Legacy damage counter (kept for compatibility/telemetry)
	int16 _playerShield;  // Shields: 0..255 where 255 = full
	int16 _playerLives;
	int32 _playerScore;

	int _viewX;
	int _viewY;
	int _hiResPresentationViewX;
	int _hiResPresentationViewY;
	int _gameplayPresentationClipBottom;

	// ---------------------------------------------------------------------------
	// Damage Visual Effect System
	// ---------------------------------------------------------------------------
	// Palette flash + screen shake on taking damage.
	// Original functions: FUN_420515, FUN_420562, FUN_420754, FUN_42073B
	//
	// FUN_42073B (triggerDamageEffect): Called on damage hit. Initiates palette
	//   flash via FUN_420515 and sets screen shake counter to 10.
	// FUN_420515 (initDamageFlash): Saves current palette, sets 5-frame flash.
	// FUN_420562 (updateDamageFlashPalette): Per-frame palette inversion.
	//   Normal hit: all RGB channels inverted toward white, fades over 5 frames.
	//   High damage (>=255): red channel pulsing on even frames.
	// FUN_420754 (updateDamageEffect): Per-frame screen shake (random scanline
	//   shifts) + calls FUN_420562. Called every frame from the render loop.

	void triggerDamageEffect();          // FUN_0042073B
	void initDamageFlash();              // FUN_00420515
	void updateDamageFlashPalette();     // FUN_00420562
	void updateDamageEffect(byte *renderBitmap, int pitch, int width, int height); // FUN_00420754
	void resetDamageFlash();             // FUN_00420501
	void restoreDamageFlashPalette();

	int16 _damageFlashCounter;           // DAT_00482404 - palette flash countdown (0..5)
	int16 _damageHighFlashCounter;       // DAT_00482408 - high-damage red flash (0..16)
	int16 _damageShakeCounter;           // DAT_0048240c - screen shake countdown (0..10)
	byte _damageSavedPalette[0x300];     // DAT_00459990 - palette snapshot before flash
	byte _damageRestorePalette[0x300];   // Boundary restore snapshot
	bool _damageRestorePaletteValid;

	// Rebel per-level counters / flags mapped from original globals
	bool _rebelOp6Initialized; // Guard: opcode 6 init block (clearBit/links/wave) runs once per video
	int _rebelHitCounter;    // DAT_0047ab80 - hit counter / state tracker
	int _rebelKillCounter;   // DAT_0047ab88 - enemies destroyed this phase
	bool _rebelYodaMode;     // DAT_0047ab5a > 2 - unlocks Yoda-mode shortcuts
	bool _rebelMovieMode;    // DAT_0047ab60 - Alt+M skips playable sections
	bool _rebelAutoPlay;     // DAT_0047ab64 - Alt+P computer-controlled play

	// Enemy wave/phase state tracking (FUN_004028c5 / FUN_00417b61)
	// DAT_0047ab98: Per-wave enemy kill state. Bits set when enemy types are destroyed.
	// DAT_0047ab9c: Per-phase accumulated state. Copied from _rebelWaveState between waves.
	// Phase completion: (_rebelPhaseState & mask) == mask (all required enemy types killed)
	int _rebelWaveState;     // DAT_0047ab98 - current wave enemy kill flags
	int _rebelPhaseState;    // DAT_0047ab9c - accumulated phase enemy kill flags

	// Opcode 6 state variables (from FUN_41CADB case 4)
	int _rebelAutopilot;     // DAT_00457904 - autopilot flag (0 or 1)
	int _rebelDamageLevel;   // DAT_0045790a - damage level (0-5)
	int _rebelFlightDir;     // DAT_00457902 - flight direction (0 or 1)
	int _rebelControlMode;   // DAT_0047a7e4 - control mode flags
	int _rebelInputThrottle; // DAT_00482278 - frame counter for input throttling (every 5 frames)

	// View offset variables (calculated from level type)
	int _rebelViewOffsetX;   // DAT_0045790c
	int _rebelViewOffsetY;   // DAT_0045790e
	int _rebelViewOffset2X;  // DAT_00457910
	int _rebelViewOffset2Y;  // DAT_00457912
	int _rebelViewMode1;     // DAT_00482270
	int _rebelViewMode2;     // DAT_00482274

	// Original counters mirrored from DAT_00443618 (values 100..109) and DAT_004436e0 (mask counters 1..9)
	short _rebelValueCounters[10]; // Index 0 -> value 100, ... Index 9 -> 109
	short _rebelMaskCounters[10];  // Index 1..9 used; index 0 unused
	int _rebelLastCounter;         // Mirrors DAT_0047ab90 (last updated counter)

	// Shield hit-point gauge: opcode-2 sets up a per-target counter; destroying a tracked
	// target decrements it, and the looping attack run ends when it reaches 0.
	int8 _rebelGaugeSlot[512];     // per target ID: -1 none; 0..9 value-counter; 10+slot mask-counter
	bool _rebelShieldGateActive;   // true while a shield-gated looping segment is playing
	bool _rebelShieldDestroyed;    // set when the shield/reactor is destroyed during the gate
	bool _rebelReactorMode;        // Level 13: finale ends when the last armed group is cleared
	bool _rebelGaugeArmed;         // at least one gauge group has been set up this attempt
	int _rebelLastArmedSlot;       // counter slot of the most recently armed group (-1 = none)
	bool _rebelGaugeCleared[10];   // value-counter slot fully destroyed (drives par3==2 surfaces)
	void resetShieldGauge();


	// ---------------------------------------------------------------------------
	// Handler 0x26 Turret Shot System
	// ---------------------------------------------------------------------------
	// Based on FUN_40AD63 disassembly - Turret laser rendering.
	// DAT_0044367a[2]: Shot duration counter (0=inactive)
	// DAT_0044367e[2]: Target X position
	// DAT_00443682[2]: Target Y position
	// DAT_0044368a[2]: Shot sequence number (for alternating gun pattern)
	// DAT_004436de: Level type (determines gun positions) - already have as _rebelLevelType

	struct TurretShot {
		int16 counter;     // DAT_0044367a[i] - duration counter, 0=inactive
		int16 targetX;     // DAT_0044367e[i] - target X position
		int16 targetY;     // DAT_00443682[i] - target Y position
		int16 seqNum;      // DAT_0044368a[i] - shot sequence (for alternating)
		int16 gunX;        // DAT_0045791c[i] - gun barrel X (Handler 25 base coords; render adds view offset)
		int16 gunY;        // DAT_00457920[i] - gun barrel Y (Handler 25 base coords; render adds view offset)
	};
	TurretShot _turretShots[2];
	int16 _turretShotSeqCounter;  // DAT_0047fe94 - global sequence counter

	// ---------------------------------------------------------------------------
	// Handler 8 Vehicle Shot System
	// ---------------------------------------------------------------------------
	// Based on FUN_402ED0 disassembly - Vehicle laser rendering.
	// DAT_0043e00a[2]: Shot duration counter
	// DAT_0043e00e[2]: Target X position
	// DAT_0043e012[2]: Target Y position
	// Gun position derived from ship position (_shipPosX, _shipPosY)

	struct VehicleShot {
		int16 counter;     // DAT_0043e00a[i] - duration counter, 0=inactive
		int16 targetX;     // DAT_0043e00e[i] - target X position
		int16 targetY;     // DAT_0043e012[i] - target Y position
	};
	VehicleShot _vehicleShots[2];

	// Handler 8 shot impact animation state (FUN_402DA8 / FUN_402ED0)
	// DAT_0043f81a[7]: impact duration counter
	// DAT_0043f828[7]: impact world X
	// DAT_0043f836[7]: impact world Y
	// DAT_0043f844[7]: impact sprite index from DAT_0047e030 background mask
	// DAT_0043f852: ring-buffer index
	struct VehicleShotImpact {
		int16 counter;
		int16 x;
		int16 y;
		int16 spriteIndex;
	};
	VehicleShotImpact _vehicleShotImpacts[7];
	int16 _vehicleShotImpactIndex;

	// ---------------------------------------------------------------------------
	// Handler 7 Third-Person Ship Shot System
	// ---------------------------------------------------------------------------
	// Based on FUN_40FADF disassembly - Third-Person Ship laser rendering.
	// DAT_00443750[2]: Shot duration counter
	// DAT_00443754[2]: Target X position
	// DAT_00443758[2]: Target Y position
	// DAT_0044375c[2]: Left gun X position
	// DAT_00443760[2]: Left gun Y position
	// DAT_00443764[2]: Right gun X position
	// DAT_00443768[2]: Right gun Y position
	// DAT_0044376c[2]: Shot variant

	struct SpaceShot {
		int16 counter;     // DAT_00443750[i] - duration counter, 0=inactive
		int16 targetX;     // DAT_00443754[i] - target X position
		int16 targetY;     // DAT_00443758[i] - target Y position
		int16 leftGunX;    // DAT_0044375c[i] - left gun X
		int16 leftGunY;    // DAT_00443760[i] - left gun Y
		int16 rightGunX;   // DAT_00443764[i] - right gun X
		int16 rightGunY;   // DAT_00443768[i] - right gun Y
		int16 variant;     // DAT_0044376c[i] - shot variant
	};
	SpaceShot _spaceShots[2];
	int16 _spaceShotDirection;  // DAT_0044374e - ship direction for gun lookup
	int16 _flyLeftGunX[35];     // DAT_004437c2 - handler 7 left muzzle X table
	int16 _flyLeftGunY[35];     // DAT_00443808 - handler 7 left muzzle Y table
	int16 _flyRightGunX[35];    // DAT_0044384e - handler 7 right muzzle X table
	int16 _flyRightGunY[35];    // DAT_00443894 - handler 7 right muzzle Y table
	bool _flyLeftGunTableLoaded;
	bool _flyRightGunTableLoaded;

	// Handler-specific shot spawning
	void spawnTurretShot(int x, int y);    // Handler 0x26
	void spawnVehicleShot(int x, int y);   // Handler 8
	void spawnSpaceShot(int x, int y);     // Handler 7
	void spawnHandler25Shot(int x, int y); // Handler 25 (speeder bike)
	void spawnShot(int x, int y);          // Dispatcher based on current handler

	// Handler-specific explosion rendering
	void renderExplosionFrame(byte *renderBitmap, int pitch, int width, int height,
	                          Explosion &explosion, int screenX, int screenY, ExplosionFrameAdvance advance,
	                          bool resolutionDependentScale);
	void renderTurretExplosions(byte *renderBitmap, int pitch, int width, int height);     // FUN_409FBC (Handler 0x26)
	void renderVehicleExplosions(byte *renderBitmap, int pitch, int width, int height);    // FUN_402696 (Handler 8)
	void renderSpaceExplosions(byte *renderBitmap, int pitch, int width, int height);      // FUN_40F1C5 (Handler 7)
	void renderHandler25Explosions(byte *renderBitmap, int pitch, int width, int height);  // FUN_41F29A (Handler 25)

	// Handler-specific laser rendering (FUN_40AD63, FUN_402ED0, FUN_40FADF, FUN_0041f004)
	void renderTurretLaserShots(byte *renderBitmap, int pitch, int width, int height);
	void renderVehicleLaserShots(byte *renderBitmap, int pitch, int width, int height);
	void renderSpaceLaserShots(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler25LaserShots(byte *renderBitmap, int pitch, int width, int height);

	// Get max shot duration from level table (DAT_0047e0f0 indexed by DAT_0047a7fa/DAT_0047a7f8)
	int16 getShotMaxDuration();

	// ---------------------------------------------------------------------------
	// Handler 8 Ship System
	// ---------------------------------------------------------------------------
	// For third-person on foot missions (Level 2, 11), the player controls Rookie One
	// that can turn in different directions. The ship sprite comes from
	// NUT files loaded via IACT opcode 8.
	//
	// Based on FUN_00401234 and FUN_00401ccf disassembly:
	// - DAT_0047e010: Primary ship sprite (POV001, subcase 1)
	// - DAT_0047e028: Secondary ship sprite (POV004, subcase 3)
	// - DAT_0047e020: Shot impact overlay (POV002, subcase 6)
	// - DAT_0047e018: Shot impact overlay (POV003, subcase 7)
	// - DAT_0043e006: Ship X position (raw, needs conversion for display)
	// - DAT_0043e008: Ship Y position (raw, needs conversion for display)
	// - DAT_0043e000: Level mode from opcode 6 par4

	NutRenderer *_shipSprite;        // DAT_0047e010 - Primary ship NUT
	NutRenderer *_shipSprite2;       // DAT_0047e028 - Secondary ship NUT
	NutRenderer *_shipOverlay1;      // DAT_0047e020 - Shot impact overlay
	NutRenderer *_shipOverlay2;      // DAT_0047e018 - Shot impact overlay

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
	// Set by opcode 6 par4, affects ship rendering behavior
	// Mode 0/1/3: "Shooting" - full movement range (127)
	// Mode 2: "Covered" - restricted movement (41) - behind cover
	// Mode 4: "Autopilot" - no shooting, scripted movement
	// Mode 5: "Cutscene" - ship not rendered
	int16 _shipLevelMode;            // DAT_0043e000
	char _handler8HudGlyph;          // DAT_0047e048
	int16 _handler8HudMessageTimer;  // DAT_0047e040
	int16 _handler8HudMessageIndex;  // DAT_0047e044

	// Movement range limiter for Handler 8 (Level 2 covered/shooting states)
	// Controls horizontal movement range: 127 for shooting, 41 for covered
	// Gradually transitions by ±10 per frame for smooth animation
	int16 _movementRangeLimit;       // DAT_0047e034

	// Control mode for Handler 7 (third-person ship) - DAT_004437c0
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

	// Previous mouse button state for edge detection (bit 0=left, bit 1=right, bit 2=middle)
	uint32 _prevMouseButtons;

	// Ship direction index for sprite selection (Handler 7)
	// Calculated from ship position: horizontal * 7 + vertical
	// horizontal: 0-4 (left to right), vertical: 0-6 (up to down)
	// Used to select which embedded HUD userId to render
	int16 _shipDirectionIndex;
	int16 _shipDirectionH;           // Horizontal direction (0-4, center=2)
	int16 _shipDirectionV;           // Vertical direction (0-6, center=3)

	// ---------------------------------------------------------------------------
	// Handler 7 FLY Ship System
	// ---------------------------------------------------------------------------
	// For third-person ship missions (Level 3, etc.), Handler 7 uses a 35-frame
	// direction-based ship sprite system. The ship visually banks and turns
	// based on player position using a 5x7 grid of sprites.
	//
	// Based on FUN_0040c3cc and FUN_0040d836 disassembly:
	// - DAT_0047fee8: Ship direction sprites (FLY001, par3=1, 35 frames)
	// - DAT_0047fef0: Ship effect sprites (FLY002, par3=3)
	// - DAT_0047fef8: Targeting overlay (FLY003, par3=2)
	// - DAT_0047ff00: High-res alternative (FLY004, par3=11)
	// - DAT_00443708: Ship X position, DAT_0044370a: Ship Y position
	// - DAT_0044370c: Smoothed horizontal velocity, DAT_0044370e: Vertical input

	NutRenderer *_flyShipSprite;     // DAT_0047fee8 - FLY001 (35 direction frames)
	NutRenderer *_flyLaserSprite;    // DAT_0047fef0 - FLY002 (danger/overlay effects)
	NutRenderer *_flyTargetSprite;   // DAT_0047fef8 - FLY003
	NutRenderer *_flyHiResSprite;    // DAT_0047ff00 - FLY004
	int16 _flyEffectAnimCounter;     // DAT_0047ff1c - animated FLY002 cue counter
	int16 _flyOverlayRepeatCount;    // DAT_00443b52 - repeats for ship overlay effect

	// Handler 7 ship state (FUN_40C3CC / FUN_0040d836)
	// Position in game coordinate space [20,404]x[20,240], center=(212,130)
	int16 _flyShipScreenX;           // DAT_00443708 - Ship X game position
	int16 _flyShipScreenY;           // DAT_0044370a - Ship Y game position

	// Physics state (velocity-based movement system from FUN_40C3CC case 4)
	int16 _smoothedVelocity;         // DAT_0044370c - Averaged horizontal velocity (from history)
	int16 _verticalInput;            // DAT_0044370e - Stored vertical input component
	int16 _velocityHistory[25];      // DAT_00443716 - Horizontal velocity ring buffer (25 entries)
	int16 _windHistoryX[15];         // DAT_00443b16 - Wind X history buffer
	int16 _windHistoryY[15];         // DAT_00443b34 - Wind Y history buffer
	int16 _windParamX;               // DAT_00443b12 - Wind X (from opcode 7 par4=0)
	int16 _windParamY;               // DAT_00443b14 - Wind Y (from opcode 7 par4=0)

	// Perspective view offsets (computed from ship position, used for rendering)
	int16 _perspectiveX;             // DAT_00443712 - Perspective shift X
	int16 _perspectiveY;             // DAT_00443714 - Perspective shift Y
	int16 _viewShift;                // DAT_00443710 - Clamped smoothed velocity for view transform
	bool _facingRight;               // DAT_0047ab8c - Ship facing right of center

	// ---------------------------------------------------------------------------
	// Handler 25 (0x19) GRD Ship System
	// ---------------------------------------------------------------------------
	// For mixed mode missions (Level 2 speeder bike, etc.), Handler 25 uses GRD NUT
	// sprites loaded via IACT opcode 8. The ship is rendered based on DAT_00457900 mode.
	//
	// Based on FUN_0041cadb case 6 and FUN_0041db5e disassembly:
	// - DAT_00482240: Primary ship sprite (GRD001, par4=1)
	// - DAT_00482238: Secondary ship sprite (GRD002, par4=2)
	// - DAT_00482258: Mode 3 overlay sprite (GRD005, par4=10)
	// - DAT_00457900: Sprite mode (1,2,3,4) controls which sprite to draw
	// - DAT_00457910: Ship X screen position
	// - DAT_00457912: Ship Y screen position
	// - DAT_00457902: Flight direction (affects GRD002 mirroring)
	// - DAT_0045790a: Damage level (affects rendering conditions)

	NutRenderer *_grd001Sprite;      // DAT_00482240 - GRD001 primary ship NUT
	NutRenderer *_grd002Sprite;      // DAT_00482238 - GRD002 secondary ship NUT
	NutRenderer *_grd005Sprite;      // DAT_00482258 - GRD005 mode 3 overlay NUT

	// Handler 25 shot-origin lookup tables from opcode 8/par4=8 text payload.
	// Indices 5..19 are filled by the "%hd %hd ..." parser in FUN_0041CADB case 6.
	// Uncovered Level 2 firing uses indices 5..14.
	int16 _grdShotOriginX[30];       // DAT_004578a6 equivalent
	int16 _grdShotOriginY[30];       // DAT_004578c6 equivalent
	bool _grdShotOriginTableLoaded;

	// Handler 25 sprite mode (DAT_00457900) - set by opcode 6 par3
	// Controls which sprite variant to draw:
	//   1: Draw _grd001Sprite normally
	//   2: Draw _grd001Sprite only when damaged (DAT_0045790a != 0)
	//   3: Draw _grd001Sprite and GRD005 (DAT_00482258) overlay
	//   4: Draw _grd001Sprite with buffer offset
	int16 _grdSpriteMode;            // DAT_00457900

	// Render Handler 25 ship sprites
	// renderHandler25ShipPre: Draw GRD001 BEFORE FOBJ (in procPreRendering)
	// renderHandler25Ship: Draw GRD002 and other overlays AFTER FOBJ (in procPostRendering)
	void renderHandler25ShipPre(byte *renderBitmap, int pitch, int width, int height);
	void renderHandler25Ship(byte *renderBitmap, int pitch, int width, int height);

	// ---------------------------------------------------------------------------
	// Handler 0x26 Turret HUD Overlays
	// ---------------------------------------------------------------------------
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

	/* Difficulty Level (0-5, from pilot menu; maps directly to table rows) */
	int _difficulty;

	// ---------------------------------------------------------------------------
	// Per-Level Difficulty Parameters
	// ---------------------------------------------------------------------------
	// Extracted from RA2WIN95.EXE at VA 0x47e0f0.
	// 2D table indexed by difficulty (0-5) × level type (0-16)
	// Original indexing: &DAT_0047e0f0 + chapter * 0x242 + levelType * 0x22
	// Level type (_rebelLevelType) is set by IACT opcode 6 par3
	// 17 entries: Lv1-5(0-4), Lv6A/6B(5-6), Lv7-14(7-14), Lv15A/15B(15-16)
	// -1 = not applicable for this level type

	struct LevelDifficultyParams {
		int16 laserDelay;      // +0x00: Laser fire delay (lower = faster)
		int16 snapDistance;    // +0x02: Crosshair snap distance to targets
		int16 missDamage;      // +0x04: Damage from enemy misses / grazing hits
		int16 dodgeDamage;     // +0x06: Damage from wall/obstacle collisions
		int16 shotDamage;      // +0x08: Damage from enemy projectile hits
		int16 specialDamage;   // +0x0A: Damage from special attacks
		int16 shotAccuracy;    // +0x0C: Enemy shot accuracy (0-100, -1=disabled)
		int16 hitPoints;       // +0x0E: Points awarded for destroying an enemy
		int16 dodgePoints;     // +0x10: Points awarded for dodging an obstacle
		int16 timePoints;      // +0x12: Time bonus points
		int16 levelPoints;     // +0x14: End-of-level bonus points
		int16 specialPoints;   // +0x16: Special action bonus points
		int16 flags;           // +0x18: Behavior flags bitfield
		int16 rollRate;        // +0x1A: Ship roll rate (flight controls)
		int16 liftRate;        // +0x1C: Ship lift rate (flight controls)
		int16 slideRate;       // +0x1E: Ship slide rate (flight controls)
		int16 driftRate;       // +0x20: Ship drift rate (flight controls)
	};

	// Table: 6 difficulty levels × 17 level types.
	// Menu labels in GAME.TRS are: Beginner, Novice, Standard, Expert, Custom1, Custom2.
	// Custom1 is identical to Standard; Custom2 matches Custom1 except Lv15B drift fields.
	static const LevelDifficultyParams kDifficultyTable[6][17];

	// Look up difficulty parameters for current _difficulty and _rebelLevelType
	LevelDifficultyParams getDifficultyParams() const;

	// Score system (FUN_0041bf8d equivalent)
	// Adds points to score and awards bonus life when crossing threshold
	void addScore(int points);

	// Score lookup uses LevelDifficultyParams fields:
	// hitPoints (DAT_0047e0fe), dodgePoints (DAT_0047e100), timePoints (DAT_0047e102)

	// Render score text to HUD (called from procPostRendering)
	void renderScoreHUD(byte *renderBitmap, int pitch, int width, int height, int statusBarY);

	// ---------------------------------------------------------------------------
	// Pause Overlay
	// ---------------------------------------------------------------------------
	// Show pause overlay with dimming effect and "PAUSED" text.
	// Emulates FUN_405A21 pause rendering (lines 242-305)
	void showPauseOverlay();

	// Target lock timer (DAT_00443676) - set to 7 when crosshair is over enemy
	int _targetLockTimer;

	// ---------------------------------------------------------------------------
	// Audio Handling
	// ---------------------------------------------------------------------------
	// RA2 doesn't use iMUSE -- audio is handled directly through the mixer.

	RebelAudio _audio;

	// Initialize audio system for RA2
	void initAudio(int sampleRate);

	// Terminate audio system
	void terminateAudio();

	// Reset streamed SAN audio at independent video boundaries.
	void resetVideoAudio();

	// Process audio dispatches - called from SmushPlayer when iMUSE is null
	// This replaces the iMUSE audio path for RA2
	void processAudioFrame(int16 feedSize);

	// Queue audio data for playback on a specific track
	void queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan);

	// ---------------------------------------------------------------------------
	// Sound Effects (SAD files)
	// ---------------------------------------------------------------------------
	// 8 standalone SAUD files in SYSTM/ loaded at init for one-shot SFX.
	// Slot mapping (from FUN_0042a3b0 init):
	//   0=BLAST.SAD   1=CRASH.SAD   2=EXPLODE.SAD  3=ALERT.SAD
	//   4=LOCKON.SAD  5=BONUS.SAD   6=HBLAST.SAD   7=TBLAST.SAD

	static const int kRA2NumSfx = 8;

	byte *_sfxData[kRA2NumSfx];         // Loaded PCM data for each SAD slot
	uint32 _sfxSize[kRA2NumSfx];        // PCM data size per slot
	Audio::SoundHandle _sfxHandles[kRA2NumSfx]; // Mixer handles for SFX playback

	// Load all SAD files from SYSTM/ directory
	void loadSfx();

	// Free all loaded SFX data
	void freeSfx();

	// Play a one-shot sound effect
	// slot: 0-7 (SAD file index), volume: 0-127, pan: -127..+127
	void playSfx(int slot, int volume, int pan);

	// ---------------------------------------------------------------------------
	// Auxiliary Sound Buffers
	// ---------------------------------------------------------------------------
	// 4 pre-allocated buffers (30000 bytes each) loaded from IACT stream data.
	// Original: DAT_00480308[0..3], loaded via FUN_004118df, played via FUN_00411931.
	// Used for embedded sound effects (e.g., soldier death sounds in handler 8 levels).
	static const int kRA2NumAuxSfx = 4;
	static const int kRA2AuxBufSize = 30000;

	byte *_auxSfxData[kRA2NumAuxSfx];     // Pre-allocated buffer pointers
	uint32 _auxSfxSize[kRA2NumAuxSfx];    // Current data size in each buffer
	Audio::SoundHandle _auxSfxHandles[kRA2NumAuxSfx]; // Mixer handles

	// Load sound data into auxiliary buffer (FUN_004118df equivalent)
	void loadAuxSfx(int buffer, const byte *data, uint32 size);

	// Play from auxiliary buffer (FUN_00411931 equivalent)
	void playAuxSfx(int buffer, int volume, int pan);

};

} // End of namespace Insane

#endif
