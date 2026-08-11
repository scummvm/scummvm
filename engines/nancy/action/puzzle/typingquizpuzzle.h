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

#ifndef NANCY_ACTION_TYPINGQUIZPUZZLE_H
#define NANCY_ACTION_TYPINGQUIZPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

#include "graphics/managed_surface.h"

namespace Nancy {
namespace Action {

// Real-time "typing quiz" arcade puzzle, new in Nancy 11 (Curse of Blackmoor Manor).
// Lettered balloons appear at fixed positions; the player pops a balloon by typing its
// character before it floats away. The puzzle runs for a fixed time limit; the score is
// a typing rate (characters per minute). If the final rate meets the target, the win
// scene fires; otherwise a default scene fires (with a different event flag depending on
// whether a partial threshold was reached). Unrelated to the text-entry QuizPuzzle (231).
class TypingQuizPuzzle : public RenderActionRecord {
public:
	TypingQuizPuzzle() : RenderActionRecord(7) {}
	virtual ~TypingQuizPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "TypingQuizPuzzle"; }

private:
	static const uint kMaxBalloons = 20;   // position/balloon slots
	static const uint kMaxImageRects = 20; // balloon image variations
	static const uint kNumDigits = 10;     // digit sprites for the score/timer displays
	static const uint kAllowedCharsLen = 30;

	enum BalloonState {
		kBalloonInactive = 0,
		kBalloonActive   = 1, // floating, can be popped
		kBalloonPopped   = 2  // popped, showing the burst sprite until _popEndTime
	};

	struct Balloon {
		BalloonState state = kBalloonInactive;
		int posIndex = -1;     // index into _positionRects
		Common::Rect imgRect;  // source rect in the puzzle image
		Common::Rect posRect;  // viewport-relative destination
		char ch = 0;
		uint32 spawnTime = 0;
		uint32 popEndTime = 0;
		uint16 lifetime = 0;   // seconds before the balloon floats away
	};

	// ---- Helpers ----
	void spawnInitialBalloons();
	void respawnBalloons(int maxToAdd);
	void spawnBalloon(uint slot);
	int pickFreePosition();      // index into _positionRects not currently occupied
	char pickRandomChar();
	bool isValidChar(byte c) const;
	void expireBalloons(uint32 now);
	void processTyping(uint32 now);
	void updateScore(uint32 now);
	void drawNumber(int value, const Common::Rect *digitRects, int x, int y);
	void redraw();
	void triggerSceneChange();

	// ---- File data ----
	Common::Path _imageName;                       // 0x000 puzzle sprite sheet
	uint16 _numImageRects = 0;                     // 0x021
	Common::Rect _balloonSrcRects[kMaxImageRects]; // 0x023
	Common::Rect _poppedSrcRect;                   // 0x163 burst sprite
	uint16 _numPositions = 0;                      // 0x173
	Common::Rect _positionRects[kMaxBalloons];     // 0x175 viewport-relative

	Common::Rect _scoreDigitRects[kNumDigits];     // 0x2b5
	Common::Rect _timerDigitRects[kNumDigits];     // 0x355
	Common::Point _scoreDest;                      // 0x405 viewport-relative
	Common::Point _timerDest;                      // 0x40d viewport-relative
	Common::Rect _passedMsgSrcRect;                // 0x41d result message sprite
	uint32 _passedMsgDuration = 0;                 // 0x42d ms

	bool _caseSensitive = false;                   // 0x431 (0 = case-insensitive)
	bool _allowDigits = false;                     // 0x432
	byte _keyboardMode = 0;                         // 0x433
	byte _allowedChars[kAllowedCharsLen] = {};     // 0x434

	uint16 _balloonLifeMin = 0;                    // 0x452 seconds
	uint16 _balloonLifeMax = 0;                    // 0x454
	uint16 _minBalloons = 0;                       // 0x456
	uint16 _maxBalloons = 0;                        // 0x458
	uint16 _timeLimit = 0;                         // 0x45a seconds
	uint16 _scoreThreshold = 0;                    // 0x45e partial-credit score
	uint16 _targetScore = 0;                       // 0x460 unscaled target rate
	uint16 _targetScorePercent = 0;                // 0x462 adaptive scaling percent
	uint16 _minTargetScore = 0;                    // 0x464 floor for the scaled target

	SoundDescription _popSound;                    // 0x466 balloon popped
	SoundDescription _wrongSound;                  // 0x497 wrong key
	SoundDescription _escapeSound;                 // 0x4c8 balloon floated away

	SceneChangeDescription _winScene;              // 0x4f9 (9999 = none)
	int16 _winFlag = -1;                           // 0x50f

	SoundDescription _winSound;                    // 0x511 played when the target is met

	SceneChangeDescription _defaultScene;          // 0x542 (did not reach the target)
	int16 _flagThreshold = -1;                     // 0x558 set if the partial threshold was reached
	int16 _flagFail = -1;                          // 0x55a set otherwise

	// ---- Runtime state ----
	Graphics::ManagedSurface _image;
	uint16 _fontID = 0;	// game default font for the balloon characters

	Balloon _balloons[kMaxBalloons];
	int _activeCount = 0;

	enum GameState {
		kPlaying  = 0, // typing against the clock
		kEvaluate = 1, // time is up: decide win/loss, play win sound
		kResult   = 2  // show the result message, then change scene
	};
	GameState _gameState = kPlaying;

	uint32 _startTime = 0;
	int _pops = 0;          // number of balloons popped
	int _score = 0;         // displayed typing rate (characters per minute)
	uint16 _effectiveTarget = 0;
	bool _reachedTarget = false;
	bool _reachedThreshold = false;
	uint32 _resultEndTime = 0;

	Common::String _pendingChars; // typed characters awaiting a match, filled by handleInput
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_TYPINGQUIZPUZZLE_H
