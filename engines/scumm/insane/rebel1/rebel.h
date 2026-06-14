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

#if !defined(SCUMM_INSANE_REBEL1_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_INSANE_REBEL1_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/events.h"
#include "scumm/insane/insane.h"
#include "scumm/insane/rebel/rebel_audio.h"
#include "scumm/insane/rebel/rebel_gamepad.h"
#include "scumm/smush/rebel/smush_player_ra1.h"

namespace Scumm {

class ScummEngine_v7;
class SmushFont;

enum RA1MenuCommand {
	kRA1MenuCommandNone = 0,
	kRA1MenuCommandUp,
	kRA1MenuCommandDown,
	kRA1MenuCommandLeft,
	kRA1MenuCommandRight,
	kRA1MenuCommandAccept,
	kRA1MenuCommandCancel,
	kRA1MenuCommandSelect1,
	kRA1MenuCommandSelect2,
	kRA1MenuCommandSelect3,
	kRA1MenuCommandSelect4,
	kRA1MenuCommandSelect5,
	kRA1MenuCommandSelect6
};

// Simple sprite bank for RA1 NUT files (ANIM v1 with odd-alignment padding).
// Separate from NutRenderer to avoid modifying shared NUT parsing code.
struct RA1Sprite {
	int16 xoffs;
	int16 yoffs;
	uint16 width;
	uint16 height;
	byte *data;  // Decoded pixel data (width * height bytes, 0 = transparent)
};

struct RA1SpriteBank {
	int numSprites;
	RA1Sprite *sprites;
	byte *decodedData;  // Single allocation for all decoded pixels
	uint32 decodedSize;

	RA1SpriteBank() : numSprites(0), sprites(nullptr), decodedData(nullptr), decodedSize(0) {}
	~RA1SpriteBank() { delete[] sprites; free(decodedData); }
};

// RA1 coordinate constants (scaled from RA2's 424x260 → 384x242)
static const int16 kRA1CenterX = 160;
static const int16 kRA1CenterY = 100;
static const int16 kRA1MinX = 20;
static const int16 kRA1MaxX = 300;
static const int16 kRA1MinY = 20;
static const int16 kRA1MaxY = 180;
static const int16 kRA1FocalX = 43;
static const int16 kRA1FocalY = 25;

int ra1ShotDirection(int16 x1, int16 y1, int16 x2, int16 y2);

/**
 * Star Wars: Rebel Assault (RA1) game logic.
 * Adapts RA2 Handler 7 (ship flight) physics for RA1's 384x242 resolution.
 */
class InsaneRebel1 : public Insane, public Common::EventObserver {
public:
	InsaneRebel1(ScummEngine_v7 *scumm);
	~InsaneRebel1() override;

	SmushPlayerRebel1 *ra1Player() const { return static_cast<SmushPlayerRebel1 *>(_player); }

	bool notifyEvent(const Common::Event &event) override;

	void procPreRendering(byte *renderBitmap) override;
	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
		int32 setupsan13, int32 curFrame, int32 maxFrame) override;
	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
		int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
		int16 par1, int16 par2, int16 par3, int16 par4) override;
	void procSKIP(int32 subSize, Common::SeekableReadStream &b) override;

	void handleGameChunk(int32 subSize, Common::SeekableReadStream &b,
		byte *renderBitmap = nullptr, int width = 0, int height = 0);
	bool isInteractiveVideoActive() const { return _interactiveVideoActive; }
	// True on touchscreen devices (e.g. Android). RA1 skips DOS-style cursor
	// warping/locking there; direct touch uses absolute aiming, while on-screen
	// gamepad/joystick input continues through the joystick path.
	bool isTouchscreenActive() const;
	void setFrameHasGameChunk(bool hasGameChunk) { _frameHasGameChunk = hasGameChunk; }
	void setCurrentSmushFrame(int32 frame);
	int getCurrentLevel() const { return _currentLevel; }
	int getLevelGameplayPhase() const { return _levelGameplayPhase; }
	uint16 getActiveGameOpcode() const { return _activeGameOpcode; }
	uint16 getEffectiveGameOpcode() const;
	uint16 getTargetHitGameOpcode() const;
	bool hasFrameGameOpcode(uint16 opcode) const {
		return opcode < 32 && (_frameGameOpcodeMask & (1u << opcode)) != 0;
	}
	void setFrameGameOpcodeHintMask(uint32 opcodeMask) { _frameGameOpcodeHintMask = opcodeMask; }
	void warpGameplayMouseNow(int x, int y);
	int16 getPerspectiveX() const { return _perspectiveX; }
	int16 getPerspectiveY() const { return _perspectiveY; }
	void projectGameplayPoint(int16 &x, int16 &y) const;
	void unprojectGameplayPoint(int16 &x, int16 &y) const;
	int16 getGameplayCursorX() const;
	int16 getGameplayCursorY() const;
	int16 getGameplayCursorX(uint16 opcode) const;
	int16 getGameplayCursorY(uint16 opcode) const;
	void setGameplayCursor(int16 x, int16 y);
	void setGameplayCursor(uint16 opcode, int16 x, int16 y);
	void updateFlightVariantCursor();
	bool handleFrameObjectTarget(int16 objectId, int16 left, int16 top, int16 width, int16 height,
		int codec, uint8 &ra1Param);
	void resetFrameObjectState();

	// Game flow (matching original at 0x15597)
	void runGame();
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false);
	Common::Error loadGameState(int slot, bool startupLoad = false);
	bool shouldAbortGameFlow() const { return _vm->shouldQuit() || _loadRequested; }

private:
	// Intro sequence: O1LOGO → O1OPEN (0x155ef-0x158f8)
	void playIntroSequence();
	void clearVideoBuffer();

	// Main menu loop on O1OPTION.ANM background (0x15968)
	// Returns: 1=Start New Game, 2=Game Options, 3=Enter Passcode,
	// 4=Level Select, 5=Continue Demo, 6=Exit
	int runMainMenu();
	int runLevelSelectMenu();

	// Level 1 flow (0x16100): hangar → CU1 → gameplay → CU2 → turret → end
	// Returns true if level completed, false if player quit
	bool runLevel1();

	// Level 2 flow: NEW → INTRO → PLAY (asteroid dodge) → END/DEATH
	bool runLevel2();
	bool runLevel3();
	bool runLevel4();
	bool runLevel5();
	bool runLevel6();
	bool runLevel7();
	bool runLevel8();
	bool runLevel9();
	bool runLevel10();
	bool runLevel11();
	bool runLevel12();
	bool runLevel13();
	bool runLevel14();
	bool runLevel15();

	// Play a passive cinematic (no game callback, skippable)
	// startFrame > 0: fast-forward (decode without display) to that frame
	void playCinematic(const char *filename, int32 startFrame = 0);
	void playLevelTransitionCutscene(int level);
	void playChapterCompleteCinematic(const char *filename, int16 unlockedChapter,
		int revealOffsetFromEnd, int stopOffsetFromEnd,
		const char *bonusLabel1 = nullptr, const char *detailText1 = nullptr, int bonusValue1 = 0,
		const char *bonusLabel2 = nullptr, const char *detailText2 = nullptr, int bonusValue2 = 0,
		int passwordIndex = 0);
	bool playDeathOrRetry(const char *retryVideo, const char *gameOverVideo);
	void autosaveProgress();
	int getAutosaveTargetSlot() const;
	int getCurrentSaveLevel() const;
	struct SaveState {
		int resumeLevel;
		int lives;
		int score;
		int prevScore;
		int difficulty;
		int maxChapterUnlocked;
	};
	Common::Error writeSaveState(int slot, const Common::String &desc, const SaveState &state) const;
	bool readSaveState(int slot, SaveState &state, Common::String *desc = nullptr) const;
	void resetLevelDamageState();
	void resetLevelFrameState();
	void resetLevelTargetingState(bool resetKillCount = true);
	void resetLevelFlightState(int16 shipDirIndex = 17);
	void resetLevelInputHistory(bool resetAxisDeltaX = false);
	void resetLevelAttemptState(int16 flyControlMode, int16 gameplayPhase,
		int16 shipDirIndex = 17, bool resetAxisDeltaX = false);

	// Play interactive gameplay video (with ship physics + HUD)
	void playInteractiveVideo(const char *filename, int32 startFrame = 0);
	void resetInteractiveVideoAudio();
	void preserveInteractiveVideoAudioState();
	void restoreInteractiveVideoAudioState();
	void setupInteractiveVideoState(int32 startFrame);
	void resolveSeek(const char *filename, int32 startFrame, int32 &videoOffset, int32 &videoStartFrame);
	void captureInteractiveVideoInput();
	void releaseInteractiveVideoInput();
	void playInteractiveVideoFile(const char *filename, int32 videoOffset, int32 videoStartFrame);
	void enableIOSGamepadController();
	void restoreIOSGamepadController();
	void openGameplayMainMenu();
	bool loadRA1Nut(const char *filename, RA1SpriteBank &bank, bool warnIfMissing = true);
	void loadLevelSprites(int level);
	void handleGameOpcode5EReset(uint32 param1);
	void handleGameOpcode5DLinkLatch(uint32 param1);
	void handleGameOpcode5FRandomHitLatch(uint32 param1);
	void handleGameOpcode07ShipFlight(int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void handleGameOpcode0DCorridor(int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void handleGameOpcode0EZone(int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void handleGameOpcode0BFirstPerson(int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void handleGameOpcode5ATarget(int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void handleGameCounterOpcode(uint32 opcode, int32 subSize, Common::SeekableReadStream &b, uint32 param1);
	void updateShipPhysics();
	void updateTurretPhysics();
	void updateTurretShipDirection(int16 offsetY);
	void getCollisionShipCenter(int16 &x, int16 &y) const;
	bool isOp0BReticleControlLevel() const;
	bool shouldInvertTouchYSettingForCurrentLevel() const;
	bool usesGamepadReticleAimForCurrentFrame() const;
	void resetGamepadReticleAim();
	bool updateGamepadReticleAim(int16 &inputX, int16 &inputY, bool *usedJoystick);
	void preprocessMouseAxes(int16 &inputX, int16 &inputY, bool *usedJoystick = nullptr);
	void rebuildProjectionTable(int16 curveStep, int16 curveExtent);
	void resetProjectionTable();
	void checkDynamicLevelBranch(int32 curFrame = -1);
	void renderShip(byte *dst, int pitch, int width, int height);
	void renderHUD(byte *dst, int pitch, int width, int height);
	void renderMainMenuOverlay(byte *dst, int pitch, int width, int height);
	void renderTextEntryOverlay(byte *dst, int pitch, int width, int height);
	void renderExplosions(byte *dst, int pitch, int width, int height);
	void renderTargetBoxes(byte *dst, int pitch, int width, int height);
	void renderTargeting(byte *dst, int pitch, int width, int height);
	void renderGostSlots(byte *dst, int pitch, int width, int height);
	void renderGostScorePopup(byte *dst, int pitch, int width, int height,
							  int16 centerX, int16 centerY, int16 frame);
	void renderLaserShots(byte *dst, int pitch, int width, int height);
	void renderShotOverlayPipeline(byte *dst, int pitch, int width, int height,
		bool drawTargetBoxes, bool drawTargeting = true);
	void renderGameOp0BOverlayDuringChunk(byte *dst, int pitch, int width, int height);
	void handleLevel14Play2BSplice(int32 curFrame, int32 maxFrame);
	void renderLevel7RouteOverlays(byte *dst, int pitch, int width, int height);
	void renderLevel5Part2Overlay(byte *dst, int pitch, int width, int height, int32 curFrame);
	void renderLevelHitsOverlay(byte *dst, int pitch, int width, int height, int y,
		bool screenSpace);
	void resetEnemyShotSlots();
	void renderLevel13EnemyShots(byte *dst, int pitch, int width, int height);
	void getTurretShipCenter(int16 &x, int16 &y) const;
	void renderLevel8Overlay(byte *dst, int pitch, int width, int height,
		int viewportX, int viewportY);
	void updateLevel8WalkerState();
	void syncViewportOffset(bool usePerspectiveViewport);
	void renderSprite(byte *dst, int pitch, int width, int height,
					  int x, int y, const RA1Sprite &sprite);
	void updateGostSlotPosition(int16 targetIdx, int16 left, int16 top, int16 right, int16 bottom);
	void applyFrameObjectHitState(int16 targetIdx);
	bool isFrameObjectPrimarySet(int16 objectId) const;
	bool areFrameObjectPrimaryBitsSet(int byteIndex, byte mask) const;
	void clearFrameObjectPrimaryBits(int byteIndex, byte mask);
	bool areLevel14Phase1TargetsDestroyed() const;
	bool areLevel14Phase2TargetsDestroyed() const;

	// Shooting pipeline — FUN_1CCA0 (0x1CCA0) shot spawner,
	// FUN_1C0EF (0x1C0EF) target detection, FUN_1C940 (0x1C940) shot processing
	bool isTorpedoModeActive() const;
	void processShot();
	void checkTargetHit(int16 targetIdx, int16 left, int16 top, int16 right, int16 bottom);

	// Audio
	void initAudio(int sampleRate);
	void terminateAudio();
	void loadSfx();
	void freeSfx();
	void playSfx(int slot, int volume, int pan);
	void stopSfx(int slot);
	void applyAudioOptions();
	void queueAudioData(int trackIdx, uint8 *data, int32 size, int volume, int pan);
public:
	void drawFontBankString(byte *dst, int pitch, int width, int height, int x, int y, const char *text);
	int getFontBankStringWidth(const char *text);
	int getFontBankLineAdvance(const char *text);
	void processAudioFrame(int16 feedSize);
private:

	ScummEngine_v7 *_vm;

	RA1SpriteBank _shipBank;
	RA1SpriteBank _shipBankAlt; // Secondary ship bank (e.g. L1BANK2 mode-2 sprites)
	RA1SpriteBank _displayBank;   // SYS/DISPLAY.NUT — bottom status bar
	RA1SpriteBank _titleFontBank; // SYS/TITLFONT.NUT — default subtitle/title layer
	RA1SpriteBank _hudFontBank;   // RA1 HUD text glyphs (TECHFONT/TALKFONT via RA1 loader)
	RA1SpriteBank _techFontBank;  // SYS/TECHFONT.NUT — targeting glyph layer ("<<" markers)
	RA1SpriteBank _bangBank;      // LxBANG.NUT — impact/explosion sprites (10 frames)
	RA1SpriteBank _laserBank;     // LxLASER.NUT — laser/shot effect sprites
	RA1SpriteBank _enemyLaserBank; // LxLASR2.NUT — incoming projectile sprites
	SmushFont *_menuFont;         // Use engine text renderer for correct TALKFONT character mapping

	// RA1 screen dimensions (384x242)
	int _screenWidth;
	int _screenHeight;

	// Ship screen position = kCenter + (accumulator >> 8)
	// Original: _DAT_74B6/_74B8 (base=160,100) + _DAT_74BA/_74BC (offset)
	int16 _shipPosX;
	int16 _shipPosY;
	// GAME opcode 0x09 uses a separate aim cursor; other handlers target via _shipPos.
	int16 _flightAimX;
	int16 _flightAimY;

	// Direction sprite index (5x7 grid = 35 sprites, vDir*7 + hDir)
	int16 _shipDirIndex;

	// Corridor boundaries (set by GAME opcode 0x0D, computed as left+width, top+height)
	int16 _corridorLeftX;
	int16 _corridorTopY;
	int16 _corridorRightX;
	int16 _corridorBottomY;

	// Physics state (accumulator-based, matching FUN_1DEB5)
	// _74CA: horizontal roll accumulator, driven by input * roll_tuning
	int32 _rollAccum;
	// _74CE: vertical smoothing, exponential decay toward -inputY
	int32 _liftSmooth;
	// _74C2/_74C6: position accumulators (32-bit), pixel offset = accum >> 8
	int32 _posAccumX;
	int32 _posAccumY;
	int16 _turretFrameShipOffsetX;
	int16 _turretFrameShipOffsetY;
	int16 _turretFrameShipCenterX;
	int16 _turretFrameShipCenterY;
	bool _turretFrameShipCenterValid;

	// Per-frame drift bias from GAME 0x07 field3
	int16 _driftParam;

	// Perspective view offsets (0x74B6/0x74B8: viewport scroll base)
	int16 _perspectiveX;
	int16 _perspectiveY;
	static const int kProjectionTableSize = 80;
	int16 _projectionCurveExtent;
	int16 _projectionTable[kProjectionTableSize];

	// Input history buffers for 0x0B handler (FUN_1CDA7) — 10-frame averaging
	static const int kInputHistorySize = 10;
	int16 _inputHistoryX[kInputHistorySize];  // 0x7580: horizontal input history
	int16 _inputHistoryY[kInputHistorySize];  // 0x7594: vertical input history
	int16 _viewHistoryX[kInputHistorySize];   // 0x75A8: viewport horizontal history
	int16 _viewHistoryY[kInputHistorySize];   // 0x75BC: viewport vertical history
	int16 _inputAxisDeltaX; // Current 0x0B horizontal input sample, before history averaging
	int16 _avgInputX;    // smoothed horizontal input (clamped to [-0xA0, 0xA0])
	int16 _avgInputY;    // smoothed vertical input (clamped to [-0x46, 0x41])
	int16 _joystickAxisX;   // Rebel-specific left-stick X captured from keymapper axis events
	int16 _joystickAxisY;   // Rebel-specific left-stick Y captured from keymapper axis events
	uint32 _lastJoystickAxisEventTime;
	int16 _level2JoystickFilteredX; // Smoothed Level 2 analog X input
	int16 _level2JoystickFilteredY; // Smoothed Level 2 analog Y input
	int16 _gamepadAimAxisX; // Relative gamepad 0x0B aim in preprocessed input space
	int16 _gamepadAimAxisY;
	bool _gamepadAimActive;
	// Suppress delayed lock/warp mousemove artifacts after centering interactive gameplay.
	uint32 _gameplayMouseSettleUntil;
	enum InputSource {
		kInputSourceMouse,
		kInputSourceJoystickAnalog,
		kInputSourceJoystickDigital,
		kInputSourceJoystickReticle
	};
	InputSource _activeInputSource;

	// GAME opcode 0x0B physics update (scrolling cockpit/surface levels)
	void updateGameOp0BPhysics();
	void updateScreenFlashPalette();
	void restoreScreenFlashPalette();

	// 0x19/0x1A on-foot handler (Level 9 Stormtroopers)
	void initOnFootSequence();
	void preprocessOnFootAim(int16 &inputX, int16 &inputY, bool *usedJoystick = nullptr);
	void updateOnFootSequence();
	void updateOnFootAimVariant();
	void finishOnFootFrame();
	int16 _onFootCharX;      // Character draw X (g_shipOffsetX in original)
	int16 _onFootCharY;      // Character draw Y (g_shipOffsetY in original)
	int16 _onFootAnimCounter; // DAT_0000828a: fire animation counter
	bool _onFootInitialized;  // First-frame init flag for 0x19

	// Current level index (0-based: 0=LVL1, 1=LVL2, etc.)
	int _currentLevel;
	int _resumeLevel;             // 1-based level used when saving/restoring RA1 progress
	int _activeSaveSlot;          // Last manually saved or loaded slot; -1 means use autosave slot
	bool _loadRequested;          // Runtime load requested while unwinding the current video flow

	// Intro title overlay (RunTwoLineTextSplash from original)
	bool _introTextActive;
	int32 _introTextStartFrame;  // revealBaseY: frame at which text begins appearing
	int32 _introTextEndFrame;    // stopY: frame after which text stops
	int _introTextLevel;         // index into kLevelTitles
	void beginLevelTitleOverlay(int level);
	void drawLevelTitleOverlay(byte *dst, int pitch, int width, int height, int32 curFrame, int32 maxFrame);

	static const int kChapterSummaryTextSize = 80;
	struct ChapterSummaryState {
		bool active;
		int revealOffsetFromEnd;
		int stopOffsetFromEnd;
		bool hasBonus1;
		bool hasBonus2;
		char bonusLabel1[kChapterSummaryTextSize];
		char detailText1[kChapterSummaryTextSize];
		int bonusValue1;
		char bonusLabel2[kChapterSummaryTextSize];
		char detailText2[kChapterSummaryTextSize];
		int bonusValue2;
		int passwordIndex;
	};
	ChapterSummaryState _chapterSummary;
	void beginChapterSummaryOverlay(int revealOffsetFromEnd, int stopOffsetFromEnd,
		const char *bonusLabel1, const char *detailText1, int bonusValue1,
		const char *bonusLabel2, const char *detailText2, int bonusValue2,
		int passwordIndex);
	void drawChapterSummaryOverlay(byte *dst, int pitch, int width, int height, int32 curFrame, int32 maxFrame);

	// Control mode (from GAME opcode 0x5E)
	int16 _flyControlMode;
	// Mode-2 emitter offsets used by FUN_1D79C when _DAT_75E4 == 2.
	int16 _turretEmitterLeftX;
	int16 _turretEmitterLeftY;
	int16 _turretEmitterRightX;
	int16 _turretEmitterRightY;
	// Last per-frame GAME opcode observed in the current playback stream.
	// Kept for legacy call sites; frame-accurate dispatch uses _frameGameOpcodeMask.
	uint16 _activeGameOpcode;
	// GAME opcodes pre-scanned from the current frame before FOBJ dispatch.
	uint32 _frameGameOpcodeHintMask;
	uint32 _frameGameOpcodeMask;
	bool _frameHasGameChunk;
	uint16 _frameDispatchFlags;
	bool _gameOp0BPhysicsUpdatedThisFrame;
	bool _gameOp0BOverlayRenderedThisFrame;

	// Difficulty (0=easy, 1=normal, 2=hard) — matches original DAT_22BC
	int _difficulty;

	// Per-difficulty tuning (from assault_data_3.bin, indexed: difficulty * 0x28B + level * 0x1F)
	// Original game loads from C:\rebltune.txt; use a hardcoded table.
	// 21 sub-levels (1A,1B,2,3,4A,4B,5A,5B,6,7,8,9A,9B,10,11,12,13,14A,14B,15A,15B)
	struct TuningParams {
		int16 roll;      // +0x05: horizontal speed/sensitivity
		int16 lift;      // +0x07: vertical speed/sensitivity
		int16 slide;     // +0x09: cross-axis coupling
		int16 drift;     // +0x0B: drift/turbulence multiplier
		int16 snap;      // +0x0D: hit radius for shooting targets
		int16 miss;      // +0x0F: light/scripted damage; FUN_1CDA7 uses it for bit 0x40
		int16 wham;      // +0x11: medium/contact damage; FUN_1CDA7 uses it for bit 0x20
		int16 shot;      // +0x13: heavy/projectile damage; FUN_1CDA7 uses it for bit 0x80
		int16 kill;      // +0x15: score per target kill
		int16 time;      // +0x17: survival bonus (added every 32 frames)
		int16 levelPts;  // +0x19: chapter completion bonus (RunChapterCompleteSummaryScreen)
		int16 bonus;     // +0x1B: per-level bonus multiplier for kills/accuracy
		int16 flags;     // +0x1D: level behavior flags (loaded into DAT_75FE at level init)
	};
	TuningParams _tuning;

	void loadTuningForLevel(int level);
	void resetGameplayFlagsFromTuning();

	// Damage system (from Ghidra decompilation of FUN_1DEB5)
	int16 _health;               // 0x7560: current health (init=98, negative=dead, max=98)
	int16 _lives;                // 0x7562: remaining extra lives
	int _score;                  // 0x7564: current score
	int _prevScore;              // 0x8288: previous score (for extra life bonus at 10k intervals)
	byte _damageFlags;           // 0x74D4: per-frame collision bitmask (cleared each frame)
	byte _prevDamageFlags;       // 0x74D6: previous frame's damage flags (for explosion direction)
	uint16 _gameLatch5D;         // 0x75D2: GAME 0x5D latch (scene/obstacle/event trigger)
	uint16 _gameLatch5F;         // 0x75D4: GAME 0x5F latch (probabilistic hit trigger)
	int16 _damageCooldown;       // 0x74D8: invulnerability timer (10 frames after hit)
	int16 _deathTimer;           // 0x756A: death animation countdown (30 on death)
	int16 _screenFlash;          // 0x7736: screen flash timer on hit
	uint32 _frameCounter;        // Gameplay handler frame accumulator
	int32 _currentSmushFrame;    // Current frontend/movie frame (original DAT_7740)
	bool _screenShakeEnabled;    // 0x41AC: when true, SetCameraOffset adds ±2 random jitter
	byte _screenFlashBasePalette[0x300];
	bool _screenFlashBasePaletteValid;
	byte _deathCauseIndicator;   // 0x772E: non-zero = player died; selects death animation variant
	byte _hudRenderFlag;         // 0x7600: 0xFF when HUD should render (set by combat mode handlers)
	byte _hudDirtyFlag;          // 0x7601: 0xFF after HUD redraw (set by renderHUD)
	int16 _maxChapterUnlocked;   // 0x7730: highest unlocked passcode slot (0=none)
	bool _unlockAllLevels;       // Option: expose level select without passcodes
	bool _noDamage;              // Option: suppress player damage

	static const int16 kMaxHealth = 98;
	static const int16 kDeathTimerInit = 30;
	static const int16 kDamageCooldownInit = 10;
	enum { kNumLevels = 15 };

	// Streamed SMUSH audio
	RebelAudio _audio;
	bool _restoreInteractiveVideoAudioState;
	int16 _savedInteractiveVideoTrackState[SMUSH_MAX_TRACKS];
	int _savedInteractiveVideoTrackGroupId[SMUSH_MAX_TRACKS];
	int _savedInteractiveVideoTrackCount;
	static const int kNumSfx = 8;
	enum SfxSlot {
		kSfxLaserShot = 0,
		kSfxExplode = 1,
		kSfxBoom = 2,
		kSfxKlaxon = 3,
		kSfxLockOn = 4,
		kSfxAlert = 5,
		kSfxBonus = 6,
		kSfxBlast = 7
	};
	byte *_sfxData[kNumSfx];
	uint32 _sfxSize[kNumSfx];
	Audio::SoundHandle _sfxHandles[kNumSfx];

	// True only while an interactive gameplay SMUSH is running.
	bool _interactiveVideoActive;
	bool _preserveInteractiveRuntimeState;
	bool _interactiveVideoCheatSkipped;
	RebelIOSGamepadControllerState _iosGamepadControllerState;

	// Path branching for levels with left/right alternative videos.
	// Original sets nextSceneA/nextSceneB when GAME 0x07 counter == 394 (0x18A).
	// We check ship position at that counter value to decide left vs right path.
	static const int32 kPathBranchCounter = 394;  // GAME 0x07 field1 value
	int32 _gameCounter;          // GAME chunk field1/logical counter
	bool _pathBranchEnabled;     // True when branching is active for this video
	bool _rightPathSelected;     // True if player branched into L1PLAY1R
	int _levelRouteIndex;        // Current mid-level route/segment for branching levels
	int _pendingRouteIndex;      // Next route requested by original frame-branch logic
	int32 _pendingRouteStartFrame; // Resume/frame-gate target for branch-driven route switches
	int32 _pendingRouteCutoverFrame; // Delayed inline route splice frame (branchFrame + 7)
	int32 _pendingRouteVideoStartFrame; // L7 destination ANM-local frame after gate adjustment
	int16 _level7WarningFrames;  // RunLevel7Flow local_38: 30-frame branch warning countdown
	int16 _level7WarningThreshold; // Collapses RunLevel7Flow local_40 into its DAT_2361 threshold
	int _levelGameplayPhase;     // Level-local interactive phase (e.g. LVL4 PLAY1 vs PLAY2)
	// RunLevel14Flow queues L14PLY2B at L14PLAY2 maxFrame-0x0F.
	bool _level14Play2BSplicePending;
	bool _level14Play2BSpliced;
	int32 _level14Play2BSpliceFrame;

	// Main menu / options state
	void runOptionsMenu();
	int runPasscodeEntryDialog();
	bool runHighScoreNameEntry();
	bool handleMenuCommand(RA1MenuCommand command);
	bool handleControllerMenuAction(ScummAction action);
	bool handleControllerMenuAxis(int16 oldAxisX, int16 oldAxisY);
	// Extra feature: navigate/click the menus with the mouse.
	bool handleMenuMouse(const Common::Event &event);
	bool handleTextEntryAction(ScummAction action);
	bool handleTextEntryKey(const Common::Event &event);
	int getMainMenuItemCount() const;
	int getMainMenuResultForSelection(int selection) const;
	void playMenuBackground();
	bool runTextEntryMenuLoop();
	void setVirtualKeyboardVisible(bool visible);
	void beginTextEntry(bool passcodeMode);
	void finishTextEntry(bool canceled);
	void selectTextEntryChar();
	const char *getChapterCompletePassword(int passwordIndex) const;
	bool _menuActive;
	bool _menuConfirmed;
	int _menuSelection; // Visible main-menu row; mapped to return values by getMainMenuResultForSelection().
	int _menuFrameCounter;

	// Options submenu state — RunGameOptionsMenu (0x14B42)
	static const int kOptionsItemCount = 9;
	bool _optionsActive;     // True when showing options instead of main menu
	int _optionsSel;         // 0..8 selected option row
	bool _levelSelectActive; // True when showing level-select submenu
	int _levelSelectSel;     // 0=Level1 ... N-1=Back
	int _startLevel;         // 1-based start level for "Start New Game"

	// Per-option state (matching original RunGameOptionsMenu globals)
	bool _optRookieOneFemale; // DAT_22c3: Rookie One gender
	bool _optMusicEnabled;    // DAT_22b7: music on/off
	bool _optSfxEnabled;      // DAT_22b8: sfx+voice on/off
	bool _optTextEnabled;     // DAT_22b9: dialogue text on/off
	bool _optControlsYFlip;   // DAT_22be: Y-axis inversion
	bool _optRapidFire;       // Option: held fire keeps shooting
	int  _optVolume;          // DAT_22c1: master volume 0..127

	// High scores / TOP PILOTS display — data at DS:0x1D0
	static const int kHighScoreCount = 10;
	struct HighScoreEntry {
		char name[20];   // 0x14 bytes per entry (includes '<' prefix)
		int32 score;
		byte difficulty;  // 0/1/2 → tech font glyph '{','|','}' (easy/normal/hard)
	};
	HighScoreEntry _highScores[kHighScoreCount];
	bool _highScoresActive;  // True when showing TOP PILOTS overlay
	void showHighScores();
	int getMenuTalkTextWidth(const char *text);
	void drawMenuTalkText(byte *dst, int pitch, int width, int height, int x, int y, const char *text);
	void drawMenuTitleText(byte *dst, int pitch, int width, int height, int x, int y, const char *text);
	void renderHighScoresOverlay(byte *dst, int pitch, int width, int height);
	void renderOptionsOverlay(byte *dst, int pitch, int width, int height);
	void renderLevelSelectOverlay(byte *dst, int pitch, int width, int height);
	void renderMainMenuItems(byte *dst, int pitch, int width, int height);

	// Character picker shared by RunPasscodeEntryDialog and RunHighScoreNameEntry.
	static const int kTextEntryBufferSize = 20;
	bool _textEntryActive;
	bool _textEntryPasscodeMode;
	bool _textEntryDone;
	bool _textEntryCanceled;
	bool _virtualKeyboardActive;
	int _textEntryPickerIndex;
	int _textEntryPickerOffsetX;
	int _textEntryMaxChars;
	int _highScoreEntryIndex;
	char _textEntryBuffer[kTextEntryBufferSize];

	// Shooting state — FUN_1CCA0 (0x1CCA0)
	bool _playerFired;       // 0x7570: current fire-button state
	// 3DO ControlB/second held button used by L9 on-foot controls.
	bool _playerSecondaryHeld;
	int16 _fireCooldown;     // 0x757C: previous-frame fire-button state (edge gate when rapid fire is off)
	int16 _rapidFirePhase;   // 3DO FUN_0000c3a4: held-fire modulo-3 shot gate
	uint16 _gameplayFlags75fe; // 0x75FE: gameplay mode flags
	uint16 _gameplayFlags75ff; // 0x75FF: targeting / shot-style flags

	// Explosion shot slots (2 slots) — FUN_1CCA0 (0x1CCA0)
	static const int kMaxShotSlots = 2;
	struct ShotSlot {
		int16 timer;     // 0x75E6: countdown (5 or 2, 0=inactive)
		int16 posX;      // 0x75F2: cursor X at time of shot
		int16 posY;      // 0x75F6: cursor Y at time of shot
		int16 centerX;   // 0x75EA: perspective-adjusted X
		int16 centerY;   // 0x75EE: perspective-adjusted Y
		int16 variant;   // 0x75FA: emitter table selector (DAT_241F snapshot)
	};
	ShotSlot _shotSlots[kMaxShotSlots];
	int16 _shotAlternator;   // 0x241F: alternates between 0/1
	bool _shotSideToggle;    // 0x2423: 0x0B side-toggle for mode-1 beam emitters

	// Targeting state — FUN_1C0EF (0x1C0EF)
	int16 _targetProximity;  // 0x7558: 0=none, 1=near, 2=on-target
	int16 _prevTargetProx;   // 0x755A: previous frame's proximity
	int16 _targetAnimCounter; // 0x755C: lock-marker animation counter
	int16 _targetCount;      // 0x7552: active targets this frame
	int16 _prevTargetCount;  // 0x7554: previous frame target count
	static const int kMaxTargetBoxes = 20;
	int16 _targetBoxX[kMaxTargetBoxes];       // 0x74DA: per-target overlay X
	int16 _targetBoxY[kMaxTargetBoxes];       // 0x7502: per-target overlay Y
	int16 _targetBoxVariant[kMaxTargetBoxes]; // 0x752A: size/near bucket ('i' + bucket)

	// GOST hit animation slots (10 slots) — FUN_1C9CD (0x1C9CD)
	static const int kMaxGostSlots = 10;
	struct GostSlot {
		int16 targetId;  // 0x23C3: target identifier (0=empty)
		int16 frame;     // 0x23D7: animation frame (0-9, >=10 = done)
		int16 posX;      // 0x239B: screen X
		int16 posY;      // 0x23AF: screen Y
	};
	GostSlot _gostSlots[kMaxGostSlots];
	int16 _gostSlotIdx;      // 0x23EB: next slot to write (circular 0-9)

	int16 _killCount;        // 0x75D0: targets destroyed this stage
	int16 _lastHitTarget;    // 0x75D6: recent-kill latch, allows at most one hit per frame
	bool _frameObjectHitRevealPending; // DispatchSmushFrameChunks local_14 high-id reveal latch

	// Incoming enemy projectile slots used by Level 13 RunLevel13Flow.
	static const int kMaxEnemyShotSlots = 5;
	struct EnemyShotSlot {
		int16 timer;
		int16 startX;
		int16 startY;
		int16 targetX;
		int16 targetY;
		int16 direction;
		uint16 flags;
	};
	EnemyShotSlot _enemyShotSlots[kMaxEnemyShotSlots];

	// Protected target IDs — 0x7732/0x7734 in original
	// Targets listed here can be hit repeatedly (no event mask toggle).
	// Used by Level 4 (shield generators) and Level 15 (torpedo targets).
	int16 _protectedTargetA; // 0x7732: protected objectId+1 (0=none)
	int16 _protectedTargetB; // 0x7734: protected objectId+1 (0=none)

	// Per-target hit counters for shield generator tracking (Level 4)
	int16 _shieldGenHitsA;   // Hits on _protectedTargetA
	int16 _shieldGenHitsB;   // Hits on _protectedTargetB
	int16 _level5SuccessFramesRemaining; // DOS RunLevel5Flow: 20-frame hold after the third kill
	int16 _level14SuccessFrames; // RunLevel14Flow: 60-frame hold after required targets are destroyed

	// Level 15 torpedo success latch. The original derives this from
	// g_gameplayPhaseFlags bit 1, which is _frameObjectState primary byte 0 bit 0x02.
	bool _torpedoFired;

	// Level 8 walker-specific state — RunLevel8Flow (0x18546)
	int16 _walkerHealth;     // Walker health percentage (0-100), init=100
	int16 _walkerTimer;      // Attack window countdown (100→0)
	int16 _walkerBranchChoice; // Directional choice: 0=none, 1=left, 2=right
	bool _walkerRoundReplay; // Preserve walker damage across a replayed L8 route

	// Attack window frame numbers per route (3 routes × 3 windows)
	// Route 0: 2588/1709/262, Route 1: 2323/1444/-2, Route 2: 877/-2/-2
	// -2 = disabled (no window at that slot for this route)
	static const int16 kWalkerAttackWindow1[3];
	static const int16 kWalkerAttackWindow2[3];
	static const int16 kWalkerAttackWindow3[3];

	static const int kFrameObjectStateBytes = 300;
	byte _frameObjectState[kFrameObjectStateBytes];
};

} // End of namespace Scumm

#endif
