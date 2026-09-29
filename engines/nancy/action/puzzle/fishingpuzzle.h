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

#ifndef NANCY_ACTION_FISHINGPUZZLE_H
#define NANCY_ACTION_FISHINGPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy16 ice fishing puzzle (AR 193). Fish and junk swim across the lanes of the lake
// below the ice hole, and clicking moves the hook to the clicked depth. Whatever touches
// the hook first is caught, and each catch leads to its own scene. Large catches also
// get snagged on the line itself. Pulling the hook out of the water ends the puzzle.
class FishingPuzzle : public PuzzleRecord {
public:
	// Drawn over the scene's movies, which include a water animation covering the viewport
	FishingPuzzle() : PuzzleRecord(15) {}
	virtual ~FishingPuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "FishingPuzzle"; }

	struct Vec2 {
		float x = 0;
		float y = 0;

		Vec2() {}
		Vec2(float x_, float y_) : x(x_), y(y_) {}

		Vec2 operator+(const Vec2 &v) const { return Vec2(x + v.x, y + v.y); }
		Vec2 operator-(const Vec2 &v) const { return Vec2(x - v.x, y - v.y); }
		Vec2 operator*(float f) const { return Vec2(x * f, y * f); }
		float length() const;
	};

	struct Ending {
		SceneChangeWithFlag scene;
		RandomSoundBlock sound;
	};

	struct Catchable {
		Common::String name;
		bool snagsLine = false;			// caught by touching any part of the line, not just the hook
		float minSpeed = 0;				// pixels per second
		float maxSpeed = 0;
		Common::Path imageName;
		uint imageID = 0;
		Common::Array<Common::Rect> frames;
		Common::Rect hitbox;			// relative to the frame origin
		Ending ending;
	};

	struct LaneEntry {
		uint catchable = 0;
		int32 percent = 0;
	};

	struct Lane {
		Common::Rect bounds;
		float minInterval = 0;			// seconds between catchables
		float maxInterval = 0;
		Common::Array<LaneEntry> entries;

		float ticks = 0;
		float interval = 0;
	};

	struct Waypoint {
		Vec2 pos;
		Vec2 tangent;
		float ticks = 0;				// time taken to get here from the previous waypoint
	};

	// A catchable swimming across a lane
	struct Swimmer {
		uint catchable = 0;
		bool isMirrored = false;		// swims right to left
		Common::Array<Waypoint> path;
		float segmentTicks = 0;
		Vec2 pos;
		float animPhase = 0;			// 0 to 1, over all frames
		float animFPS = 0;
	};

	static void readEnding(Common::SeekableReadStream &stream, Ending &ending);
	static Vec2 hermite(const Vec2 &p1, const Vec2 &t1, const Vec2 &p2, const Vec2 &t2, float s);
	static bool segmentsIntersect(const Vec2 &a1, const Vec2 &a2, const Vec2 &b1, const Vec2 &b2);

	void pickLaneInterval(Lane &lane);
	void spawn(const Lane &lane, uint catchable);
	bool moveSwimmer(Swimmer &swimmer, float ticks);
	const Common::Rect &getFrame(const Swimmer &swimmer) const;
	bool isCaught(const Swimmer &swimmer, const Vec2 &hook, const Vec2 &lineTop, const Vec2 &lineBottom) const;

	void moveHookTo(const Common::Point &pos);
	bool moveLine(int32 ticks);
	void update(int32 lineTicks, float ticks);
	void startEnding(const Ending &ending);

	void redraw();

	// -- File data --
	Common::Array<Catchable> _catchables;
	Common::Array<Lane> _lanes;
	uint16 _cursor = 0;

	// The line image is _image
	float _lineAcceleration = 0;		// pixels per tick squared
	Common::Rect _lineSrc;
	Common::Point _hookPoint;			// all relative to the line's top left
	Common::Point _lineTop;
	Common::Point _lineBottom;
	Common::Rect _lineBounds;
	Common::Point _lineStart;			// left and bottom
	Common::Point _clickOffset;
	RandomSoundBlock _reelSound;
	int32 _exitHookY = 0;				// pulling the hook above this ends the puzzle
	Ending _exitEnding;

	// -- Runtime state --
	Common::Array<Graphics::ManagedSurface *> _catchableImages;
	Common::Array<uint> _drawOrder;
	Common::Array<Swimmer> _swimmers;

	Vec2 _linePos;
	Vec2 _lineVelocity;
	Vec2 _lineAccel;
	Common::Point _lineTarget;

	uint32 _lastTickTime = 0;			// the line moves in whole ticks
	uint32 _lastFrameTime = 0;			// everything else moves smoothly
	bool _isEnding = false;
	bool _exitRequested = false;
	SceneChangeWithFlag _endScene;
	SoundDescription _endSound;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_FISHINGPUZZLE_H
