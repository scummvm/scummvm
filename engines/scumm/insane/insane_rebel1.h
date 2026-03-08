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

namespace Scumm {

class ScummEngine_v7;
class SmushPlayer;
class SmushFont;

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

/**
 * Star Wars: Rebel Assault (RA1) game logic.
 * Adapts RA2 Handler 7 (ship flight) physics for RA1's 384x242 resolution.
 */
class InsaneRebel1 : public Insane, public Common::EventObserver {
public:
	InsaneRebel1(ScummEngine_v7 *scumm);
	~InsaneRebel1() override;

	bool notifyEvent(const Common::Event &event) override;

	void procPreRendering(byte *renderBitmap) override;
	void procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
		int32 setupsan13, int32 curFrame, int32 maxFrame) override;
	void procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
		int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
		int16 par1, int16 par2, int16 par3, int16 par4) override;
	void procSKIP(int32 subSize, Common::SeekableReadStream &b) override;

	void handleGameChunk(int32 subSize, Common::SeekableReadStream &b);
	bool isInteractiveVideoActive() const { return _interactiveVideoActive; }
	int getCurrentLevel() const { return _currentLevel; }
	uint16 getActiveGameOpcode() const { return _activeGameOpcode; }

	// Game flow (matching original at 0x15597)
	void runGame();

private:
	// Intro sequence: O1LOGO → O1OPEN (0x155ef-0x158f8)
	void playIntroSequence();
	void clearVideoBuffer();

	// Main menu loop on O1OPTION.ANM background (0x15968)
	// Returns: 1=Start New Game, 2=Game Options, 3=Level Select, 4=Continue Demo, 5=Exit
	int runMainMenu();
	void runLevelSelectMenu();

	// Level 1 flow (0x16100): hangar → CU1 → gameplay → CU2 → turret → end
	// Returns true if level completed, false if player quit
	bool runLevel1();

	// Level 2 flow: NEW → INTRO → PLAY (asteroid dodge) → END/DEATH
	bool runLevel2();

	// Play a passive cinematic (no game callback, skippable)
	// startFrame > 0: fast-forward (decode without display) to that frame
	void playCinematic(const char *filename, int32 startFrame = 0);

	// Play interactive gameplay video (with ship physics + HUD)
	void playInteractiveVideo(const char *filename);
	bool loadRA1Nut(const char *filename, RA1SpriteBank &bank);
	void loadLevelSprites(int level);
	void updateShipPhysics();
	void updateTurretPhysics();
	void renderShip(byte *dst, int pitch, int width, int height);
	void renderHUD(byte *dst, int pitch, int width, int height);
	void renderMainMenuOverlay(byte *dst, int pitch, int width, int height);
	void renderExplosions(byte *dst, int pitch, int width, int height);
	void renderTargeting(byte *dst, int pitch, int width, int height);
	void renderGostSlots(byte *dst, int pitch, int width, int height);
	void renderLaserShots(byte *dst, int pitch, int width, int height);
	void renderSprite(byte *dst, int pitch, int width, int height,
					  int x, int y, const RA1Sprite &sprite);

	// Shooting pipeline — FUN_1CCA0 (0x1CCA0) shot spawner,
	// FUN_1C0EF (0x1C0EF) target detection, FUN_1C940 (0x1C940) shot processing
	void processShot();
	void checkTargetHit(int16 targetIdx, int16 left, int16 top, int16 right, int16 bottom);

	// Audio
	void initAudio(int sampleRate);
	void terminateAudio();
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
	SmushFont *_menuFont;         // Use engine text renderer for correct TALKFONT character mapping

	// RA1 screen dimensions (384x242)
	int _screenWidth;
	int _screenHeight;

	// Ship screen position = kCenter + (accumulator >> 8)
	// Original: _DAT_74B6/_74B8 (base=160,100) + _DAT_74BA/_74BC (offset)
	int16 _shipPosX;
	int16 _shipPosY;

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

	// Per-frame drift bias from GAME 0x07 field3
	int16 _driftParam;

	// Perspective view offsets (0x74B6/0x74B8: viewport scroll base)
	int16 _perspectiveX;
	int16 _perspectiveY;

	// Input history buffers for 0x0B handler (FUN_1CDA7) — 10-frame averaging
	static const int kInputHistorySize = 10;
	int16 _inputHistoryX[kInputHistorySize];  // 0x7580: horizontal input history
	int16 _inputHistoryY[kInputHistorySize];  // 0x7594: vertical input history
	int16 _viewHistoryX[kInputHistorySize];   // 0x75A8: viewport horizontal history
	int16 _viewHistoryY[kInputHistorySize];   // 0x75BC: viewport vertical history
	int16 _avgInputX;    // smoothed horizontal input (clamped to [-0xA0, 0xA0])
	int16 _avgInputY;    // smoothed vertical input (clamped to [-0x46, 0x41])

	// 0x0B handler physics update (asteroid/surface levels)
	void updateAsteroidPhysics();

	// Current level index (0-based: 0=LVL1, 1=LVL2, etc.)
	int _currentLevel;

	// Control mode (from GAME opcode 0x5E)
	int16 _flyControlMode;
	// Mode-2 emitter offsets used by FUN_1D79C when _DAT_75E4 == 2.
	int16 _turretEmitterLeftX;
	int16 _turretEmitterLeftY;
	int16 _turretEmitterRightX;
	int16 _turretEmitterRightY;
	// Last per-frame GAME movement handler opcode (0x07/0x08/0x09/0x0A/0x0B/0x1A).
	// Used to mirror assembly handler-specific overlay pipeline behavior.
	uint16 _activeGameOpcode;

	// Difficulty (0=easy, 1=normal, 2=hard) — matches original DAT_22BC
	int _difficulty;

	// Per-difficulty tuning (from assault_data_3.bin, indexed: difficulty * 0x28B + level * 0x1F)
	struct TuningParams {
		int16 roll;    // 0x1B1B: horizontal speed/sensitivity
		int16 lift;    // 0x1B1D: vertical speed/sensitivity
		int16 slide;   // 0x1B1F: cross-axis coupling
		int16 drift;   // 0x1B21: drift/turbulence multiplier
		int16 snap;    // 0x1B23: hit radius for shooting targets
		int16 miss;    // 0x1B25: obstacle collision damage (0x0B bit 0x40)
		int16 wham;    // 0x1B27: light/wall damage
		int16 shot;    // 0x1B29: heavy/projectile damage
		int16 kill;    // 0x1B2B: score per target kill
	};
	TuningParams _tuning;

	void loadTuningForLevel(int level);

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
	uint32 _frameCounter;        // 0x7740: global frame counter

	static const int16 kMaxHealth = 98;
	static const int16 kDeathTimerInit = 30;
	static const int16 kDamageCooldownInit = 10;

	// Audio state (same structure as RA2)
	static const int kMaxAudioTracks = 4;
	Audio::QueuingAudioStream *_audioStreams[kMaxAudioTracks];
	Audio::SoundHandle _audioHandles[kMaxAudioTracks];
	bool _audioTrackActive[kMaxAudioTracks];
	int _audioSampleRate;

	// True only while an interactive gameplay SMUSH is running.
	bool _interactiveVideoActive;

	// Path branching for levels with left/right alternative videos.
	// Original sets nextSceneA/nextSceneB when GAME 0x07 counter == 394 (0x18A).
	// We check ship position at that counter value to decide left vs right path.
	static const int32 kPathBranchCounter = 394;  // GAME 0x07 field1 value
	int32 _gameCounter;          // GAME 0x07 field1 — the original's _DAT_7740
	bool _pathBranchEnabled;     // True when branching is active for this video
	bool _rightPathSelected;     // True if player chose the right/easy path

	// Main menu / options state
	void runOptionsMenu();
	bool _menuActive;
	bool _menuConfirmed;
	int _menuSelection; // 0..4 maps to return values 1..5
	int _menuFrameCounter;

	// Options submenu state
	bool _optionsActive;     // True when showing options instead of main menu
	int _optionsSel;         // 0=difficulty, 1=turbulence, 2=back
	bool _levelSelectActive; // True when showing level-select submenu
	int _levelSelectSel;     // 0=Level1, 1=Level2, 2=Back
	int _startLevel;         // 1-based start level for "Start New Game"

	bool _turbulenceEnabled;  // Random per-frame jitter in deltaX (original has it on)

	// Shooting state — FUN_1CCA0 (0x1CCA0)
	bool _playerFired;       // 0x7570: fire button pressed this frame
	int16 _fireCooldown;     // 0x757C: button-edge gate in original input pipeline

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

	// Targeting state — FUN_1C0EF (0x1C0EF)
	int16 _targetProximity;  // 0x7558: 0=none, 1=near, 2=on-target
	int16 _prevTargetProx;   // 0x755A: previous frame's proximity
	int16 _targetCount;      // 0x7552: active targets this frame
	int16 _prevTargetCount;  // 0x7554: previous frame target count

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
	int16 _lastHitTarget;    // 0x75D6: prevents double-hit on same target
};

} // End of namespace Scumm

#endif
