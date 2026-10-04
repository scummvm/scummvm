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

#ifndef NANCY_ACTION_WHACKITPUZZLE_H
#define NANCY_ACTION_WHACKITPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"
#include "engines/nancy/action/physicalobject.h"

namespace Nancy {
namespace Action {

// Nancy17 bees in the garden. The bees fly around, get angry after a while, and
// sting the player when they stay angry for too long. A puff of the spray can knocks
// down every bee it touches; a bee that is down gets back up after some time. With
// every bee down the puzzle sets its flag, and too many stings lose the game.
class WhackItPuzzle : public PuzzleRecord {
public:
	// The bees and the puffs go in front of the berries, which are overlays
	WhackItPuzzle() : PuzzleRecord(15) {}
	virtual ~WhackItPuzzle() {}

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "WhackItPuzzle"; }

	struct Animation {
		uint32 frameTime = 0;	// in milliseconds
		Common::Array<Common::Rect> frames;

		void readData(Common::SeekableReadStream &stream);
	};

	struct AnimationPlayer {
		const Animation *animation = nullptr;
		uint frame = 0;
		uint32 frameRemainder = 0;
		bool isDone = false;

		void start(const Animation *newAnimation);
		void advance(uint32 time, bool loops);
		// The current frame, centered on a position
		Common::Rect getDest(const Common::Point &pos) const;
		const Common::Rect &getSrc() const { return animation->frames[MIN<uint>(frame, animation->frames.size() - 1)]; }
	};

	enum BeeState {
		kDown,
		kReviving,
		kFlying,
		kAngry
	};

	struct Bee {
		BeeState state = kFlying;
		uint32 stateEndTime = 0;
		PhysicalObject body;
		AnimationPlayer animation;

		Common::Point getPos() const { return Common::Point((int)body.getX(), (int)body.getY()); }
	};

	// Drawn over the puzzle, so it can be see-through
	struct FlashOverlay : RenderObject {
		FlashOverlay() : RenderObject(16) {}

		bool isViewportRelative() const override { return true; }
	};

	struct Puff {
		Common::Point pos;
		AnimationPlayer animation;
	};

	// A time between the two given ones, counted from now
	uint32 getRandomTime(uint32 minTime, uint32 maxTime) const;
	void spawnBee(Bee &bee);
	void flyAgain(Bee &bee);
	void knockDown(Bee &bee);
	void updateBee(Bee &bee, uint32 numTicks);
	void sting();
	void updateSounds(uint numReviving, uint numFlying, uint numAngry);
	void redraw();

	// -- File data --
	Common::Rect _canSrc;
	uint16 _cursor = 0;
	Common::Point _canOffset;	// from the mouse to the can's top left corner
	Common::Point _startPos;	// a negative coordinate is random
	Common::Rect _bounds;
	int32 _minBees = 0;
	int32 _maxBees = 0;

	// How long the bees stay in each state
	uint32 _downTime[2] = { 0, 0 };
	uint32 _revivingTime[2] = { 0, 0 };
	uint32 _flyingTime[2] = { 0, 0 };
	uint32 _angryTime[2] = { 0, 0 };

	FlagDescription _allDownFlag;	// gets the opposite value while any bee flies
	bool _stopsWhenAllDown = false;
	int32 _hitFrame = 0;	// the frame of a bee that a puff has to touch

	Animation _puffAnimation;
	Animation _flyAnimation;
	Animation _downAnimation;

	RandomSoundBlock _puffSound;
	RandomSoundBlock _revivingSound;
	RandomSoundBlock _flyingSound;
	RandomSoundBlock _angrySound;
	RandomSoundBlock _stingSound;

	int32 _maxStings = 0;
	SceneChangeWithFlag _loseScene;
	RandomSoundBlock _loseSound;

	// -- Runtime --
	Common::Array<Bee> _bees;
	Common::Array<Puff> _puffs;
	bool _isActive = true;
	int32 _numStings = 0;
	uint32 _lastStingTime = 0;
	bool _hasStung = false;
	FlashOverlay _flash;
	uint32 _lastUpdateTime = 0;
	Common::Point _mousePos;
	bool _isMouseInside = false;
	bool _isLost = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_WHACKITPUZZLE_H
