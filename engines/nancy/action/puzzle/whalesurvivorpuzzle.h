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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_WHALESURVIVORPUZZLE_H
#define NANCY_ACTION_WHALESURVIVORPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

#include "graphics/managed_surface.h"

namespace Nancy {
namespace Action {

// "Never a Dall Moment" - arcade game in Nancy 9 (Danger on Deception Island).
// The player controls Dolly the dall's porpoise, eating fish while avoiding hazards
// (sharks, octopuses, orcas, pollution) and periodically surfacing to breathe.
// Win by reaching the score target. Two difficulty levels (easy/hard).
class WhaleSurvivorPuzzle : public RenderActionRecord {
public:
	WhaleSurvivorPuzzle() : RenderActionRecord(7) {}
	virtual ~WhaleSurvivorPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "WhaleSurvivorPuzzle"; }

private:
	// ---- Constants ----
	static const int kNumEntityTypes  = 6;
	static const int kNumSubSprites   = 4;   // sub-sprite entries per entity type
	static const int kMaxLives        = 3;
	static const int kOxygenStages    = 8;
	static const int kMaxEntities     = 32;
	static const int kNumScoreDigits  = 10;  // digit sprite count (0-9)
	static const int kNumFloorSprites = 7;   // ocean floor background sprites

	// ---- File data ----

	Common::Path _imageNameMain;     // game overlay (sprites + UI)
	Common::Path _imageNameStart;    // start screen overlay
	Common::Path _imageNameTryAgain; // try-again overlay
	Common::Path _imageNameWin;      // win overlay

	Common::Rect _porpoiseSwimmingRect;
	Common::Rect _porpoiseBellyUpRect;
	Common::Rect _livesSrcRect;

	// Per-entity-type sprite data (6 fish types, 6 hazard types)
	struct SubSprite {
		bool active = false;
		Common::Rect src; // source rect in main overlay
	};
	struct EntityType {
		Common::Rect mainSrc;           // overall bounding src rect
		SubSprite subSprites[kNumSubSprites]; // individual component sprites
	};
	EntityType _fishTypes[kNumEntityTypes];
	EntityType _hazardTypes[kNumEntityTypes];

	// Start button source rects - unpressed/pressed state (2 × 16 bytes)
	Common::Rect _startButtonSrcRects[2];

	// Quit button source rects - unpressed/pressed state (2 × 16 bytes)
	Common::Rect _quitButtonSrcRects[2];

	// Score digit source rects in main overlay (digits 0-9)
	Common::Rect _scoreDigitSrcRects[kNumScoreDigits];

	// Ocean floor background sprites (source rects and screen dest rects)
	Common::Rect _floorSrcRects[kNumFloorSprites];   // 0x614..0x683 (source in overlay)
	Common::Rect _floorDestRects[kNumFloorSprites];  // 0x82c..0x89b (dest on screen)

	// Lives indicator dest rects on screen (viewport-relative)
	Common::Rect _livesDestRects[kMaxLives];

	// Oxygen bar: 8 stage source rects (full->empty) in the overlay
	Common::Rect _oxygenSrcRects[kOxygenStages];  // 0x684..0x703

	// Bubble/breath animation: 8 frame source rects (one per breathFrame step)
	Common::Rect _bubbleSrcRects[kOxygenStages];  // 0x704..0x783

	// Screen positions for UI buttons (viewport-relative)
	Common::Rect _startButtonDestRect;	// 0x7ec
	Common::Rect _quitButtonDestRect;	// 0x7fc

	// Oxygen bar screen position (viewport-relative top-left)
	int _oxygenBarX = 0;  // 0x7ac
	int _oxygenBarY = 0;  // 0x7b0

	// Playfield bounds (viewport-relative), surface Y and max Y
	Common::Rect _playfieldRect;  // 0x80c..0x81b
	Common::Rect _overlayRect;    // 0x81c..0x82b — region where overlay images are drawn
	int _surfaceY  = 0;           // 0x784 (min Y for breathing)
	int _maxY      = 0;           // 0x788 (max Y the porpoise can reach)
	int _spawnYMin = 0;           // 0x78c (entity spawn Y range)
	int _spawnYMax = 0;           // 0x790
	int _scoreX    = 0;           // 0x79c (score display X, vp-relative)
	int _scoreY    = 0;           // 0x7a0 (score display Y, vp-relative)

	// Movement speeds: original stores speed/denominator pairs -> px/ms
	float _speedX      = 0.0f; // 0x8c0 / 0x8b4
	float _speedY      = 0.0f; // 0x8c4 / 0x8b8
	float _breathSpeed = 0.0f; // 0x8c8 / 0x8bc  (natural upward drift)

	// Fish speed
	uint32 _fishAnimFPS   = 40;
	float  _fishSpeedRange = 1.0f; // 0x8d0
	float  _fishSpeedBase  = 5.0f; // 0x8d4

	uint32 _hazardAnimFPS   = 50;
	float  _hazardSpeedRange = 1.0f; // 0x8dc
	float  _hazardSpeedBase  = 3.5f; // 0x8e0

	uint32 _bubbleIntervalMs = 50;   // 0x8ac
	uint32 _tryAgainDelayMs  = 2000; // 0x8b0
	uint32 _oxygenTickMs     = 2000; // 0x8e8
	uint32 _initialFish      = 4;    // 0x8f0
	uint32 _initialHazards   = 2;    // 0x8f4
	bool   _isHardMode       = false;// 0x8fc
	bool   _randomSpawn      = true; // 0x8fd
	uint32 _scoreDivisor     = 500;  // 0x8fe
	float  _oxygenDeplSpeed  = 2.0f; // 0x91a
	uint32 _maxScore         = 20000;// 0x91e
	uint32 _breathFreq       = 1;    // 0x922

	SoundDescription _eatSound;      // 0x926 - fish eaten
	SoundDescription _hurtSound;     // 0x957 - hit by hazard
	SoundDescription _breatheSound;  // 0x988 - surface/breathe
	SoundDescription _sound4;        // 0x9b9 - unused (silence)
	SoundDescription _sound5;        // 0x9ea - unused (silence)

	SceneChangeWithFlag _lossScene;  // 0xa1b
	SoundDescription    _winSound;   // 0xa35
	SceneChangeWithFlag _winScene;   // 0xa66

	SoundDescription _gameOverSound; // 0xa80
	SoundDescription _tryAgainSound; // 0xab1

	// ---- Runtime state ----

	Graphics::ManagedSurface _imageMain;
	Graphics::ManagedSurface _imageStart;
	Graphics::ManagedSurface _imageTryAgain;
	Graphics::ManagedSurface _imageWin;

	enum EntityKind {
		kEntityFish   = 0,
		kEntityHazard = 1
	};

	// Entity (fish or hazard) currently on screen
	struct Entity {
		bool       active   = false;
		EntityKind kind     = kEntityFish;
		int   typeIdx   = 0;   // 0..5, which sprite type
		float floatX    = 0.0f; // left edge (decreases over time)
		int   x         = 0;    // integer left (= (int)floatX)
		int   y         = 0;    // top (fixed at spawn)
		int   width     = 0;    // sprite width
		int   height    = 0;    // sprite height
		float speedX    = 0.0f; // px/ms
		uint32 animFPS  = 40;   // ms per animation step (for speed calc)
		bool  removed   = false;
	};

	Entity _entities[kMaxEntities];

	enum GameState {
		kStartScreen  = 0, // show start overlay, wait for click
		kPlaying      = 1, // active gameplay
		kRoundWon     = 2, // all fish eaten -> start win countdown
		kHitAnimation = 3, // porpoise hit, show try-again animation
		kWinScreen    = 4, // won entirely -> show win overlay and trigger win scene
		kTryAgain     = 5, // reset round after hit
		kCountdown    = 6  // brief countdown before executing scene change
	};
	GameState _gameState = kStartScreen;

	// Porpoise position and state
	float _porpX = 0.0f, _porpY = 0.0f; // float positions
	int   _porpLeft = 0, _porpTop = 0;  // integer pixel positions (viewport-relative)
	int   _porpWidth = 0, _porpHeight = 0;

	enum PorpoiseAnim {
		kPorpoiseSwim    = 0,
		kPorpoiseSurface = 1,
		kPorpoiseEat     = 2,
		kPorpoiseHit     = 3
	};
	PorpoiseAnim _porpoiseAnim = kPorpoiseSwim;

	uint32 _inputFlags          = 0;     // cached NancyInput::input flags from handleInput
	uint32 _nextBubbleMs        = 0;     // next bubble animation tick
	int    _breathFrame         = 0;     // current breath animation frame (0..7)
	bool   _firstHitFrame       = false; // true on the first frame after taking a hit
	bool   _startButtonHeld     = false; // true while left mouse held over start button
	bool   _quitButtonHeld      = false; // true while left mouse held over quit button

	// Oxygen/survival state
	bool   _oxygenDepleting = false;
	int    _oxygenStage     = 0;     // 0=full, 8=dead
	uint32 _oxygenNextTickMs = 0;

	// Score and lives
	int    _score = 0;
	int    _lives = kMaxLives;
	int    _prevScore = -1; // track when score changes for redraw

	// Spawn counters (maintain target number of entities on screen)
	int _fishCount    = 0;
	int _hazardCount  = 0;

	// Timers
	uint32 _lastMs         = 0;
	uint32 _countdownEndMs = 0;
	bool   _executeWin     = false; // whether to execute win or loss scene at countdown end

	// ---- Private helpers ----
	void initRound();
	void spawnEntity(EntityKind kind);
	void updateGame(uint32 nowMs);
	void checkCollisions();
	void updateOxygen(uint32 nowMs);
	void loseLife(uint32 nowMs);
	void redraw();
	int  findFreeEntity() const;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_WHALESURVIVORPUZZLE_H
