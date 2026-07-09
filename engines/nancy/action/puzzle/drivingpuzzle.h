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

#ifndef NANCY_ACTION_DRIVINGPUZZLE_H
#define NANCY_ACTION_DRIVINGPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/navigationrecords.h"
#include "engines/nancy/action/actionzone.h"

namespace Nancy {
namespace Action {

// Real-time top-down driving minigames introduced in Nancy12. Two closely related
// action records share the same engine:
//   160 - kDriving  (drive Nancy's car around the Titusville town map, entering
//                    locations by driving into them)
//   167 - kChase    (kDriving plus a second, chaser car, a second zone array and
//                    two path-point arrays that steer the chaser)
//
// The map scrolls under a car-centered camera; the car is drawn as a rotation-atlas
// sprite whose frame is chosen from its heading. The map is populated with an
// ActionZone array: type 0x11 zones are location entrances (each carries the
// destination scene id and the transition effect), type 0x14 zones are boundaries,
// and the remaining subtypes are decorations and driving hazards.
//
// TODO (need runtime tuning):
//  - Fuel: burn the gas-gauge UI resource (index _frictionIndex) while driving; it
//    currently stays at its seeded value. The DT_RESOURCE dependency that reads it is
//    handled in ActionManager::processDependency.
//  - Steering: approximated with the arrow keys (the game uses click-to-steer).
//  - Collision: only the type 0x14 boundary rects block the car (no per-pixel mask).
//  - kChase: no second-path switch or "chaser left the viewport" loss branch.
//  - Overlay / hazard zone subtypes (0x0d/0x0e/0x0f/0x05/0x03/0x17) are ignored.
class DrivingPuzzle : public RenderActionRecord {
public:
	enum Variant { kDriving = 0, kChase };

	DrivingPuzzle(Variant variant) : RenderActionRecord(7), _variant(variant) {}
	virtual ~DrivingPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override {
		return _variant == kChase ? "ChasePuzzle" : "DrivingPuzzle";
	}

	// A destination the car can drive into: a location entrance (type 0x11) or the
	// chase's finish line (type 0x0c). Entering its map-space rect optionally sets an
	// event flag and transitions to the destination scene through a fade.
	struct DestinationZone {
		Common::Rect rect;
		SceneChangeDescription scene;
		bool hasFade = false;
		byte fadeType = 0;
		uint16 fadeTotalTime = 0;
		uint16 fadeToBlackTime = 0;
		Common::Rect fadeRect;
		int16 eventFlag = -1;
		byte eventFlagValue = 0;
		bool carInside = false;		// the car was inside this zone last frame
	};

	// A checkpoint (type 0x0b): driving over it sets an event flag once.
	struct Checkpoint {
		Common::Rect rect;
		int16 flagId = -1;
		byte flagValue = 0;
		bool triggered = false;
		bool carInside = false;		// the car was inside this zone last frame
	};

	// A recorded chaser-path waypoint (kChase): the pursuer plays these back in real
	// time, jumping to the entry whose timestamp the elapsed chase time has passed.
	struct Waypoint {
		uint32 timeMs = 0;
		int16 x = 0;
		int16 y = 0;
		double heading = 0.0;	// radians
	};

	// Reads an int16-prefixed array of Rects (a rotation-frame table).
	void readFrameRects(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &out);

	// Reads an int16-prefixed array of chaser-path waypoints (16 bytes each).
	void readWaypoints(Common::SeekableReadStream &stream, Common::Array<Waypoint> &out);

	// Reads the 130-byte PuzzleBase header blob: three filenames (map image, collision
	// map, car sprite atlas) and the car's physics parameters.
	void readBlob(Common::SeekableReadStream &stream);

	// Sorts an ActionZone array into its gameplay roles (destination, checkpoint and
	// boundary zones), decoding the destination scenes and their transition effects.
	void classifyZones(const Common::Array<ActionZone> &zones);

	// Plays one (randomly chosen) entry of a random-sound block.
	void playSoundBlock(const RandomSoundBlock &block);

	// Advances the car's heading/velocity/position for one frame from the current
	// movement input.
	void updatePhysics(const NancyInput &input);

	// Advances the chaser along its recorded path (kChase) and slows the player's
	// speed cap the closer the chaser gets.
	void updateChaser();

	// Chooses a rotation-atlas frame from a heading.
	uint frameIndexForHeading(double heading, uint frameCount) const;

	// Redraws the scrolling map (car-centered camera) and the car sprite(s) on top.
	void drawScene();

	Variant _variant;

	// Three filenames decoded from the header blob.
	Common::Path _imageName;			// visible town map ("MAP_Titusville")
	Common::Path _collisionName;		// collision mask ("MAP_TitusvilleCollision")
	Common::Path _carSpriteName;		// car rotation atlas ("MAP_Roadster_OVL")

	// Car physics parameters decoded from the header blob.
	int32 _startX = 0;			// blob+0x63: start position (map space)
	int32 _startY = 0;			// blob+0x67
	int32 _startAngle = 0;		// blob+0x6b: start heading, degrees
	int32 _forwardSpeed = 0;	// blob+0x6f: forward speed cap
	int32 _reverseSpeed = 0;	// blob+0x73
	int16 _frictionIndex = 0;	// blob+0x77: index into the shared friction table
	int32 _distanceDivisor = 0;	// blob+0x7b
	bool _retainState = false;	// blob+0x7f: resume from the saved position

	// Three random-sound blocks (tire blowout, horn, engine) and a rotation-frame
	// rect table precede the ActionZone array.
	RandomSoundBlock _soundBlocks[3];
	Common::Array<Common::Rect> _frameRects;
	Common::Array<ActionZone> _zones;

	// kChase (167) extras: five id/scene values, a second (chaser) car sprite
	// name, a second rotation-frame table, a second ActionZone array and two
	// recorded chaser paths (a main route and a shorter one).
	int16 _chaseParams[5] = {};
	Common::Path _chaseCarImageName;
	Common::Array<Common::Rect> _frameRects2;
	Common::Array<ActionZone> _zones2;
	Common::Array<Waypoint> _chaserPathA;
	Common::Array<Waypoint> _chaserPathB;

	// ActionZone gameplay roles.
	Common::Array<DestinationZone> _destinations;	// types 0x11 / 0x0c
	Common::Array<Checkpoint> _checkpoints;			// type 0x0b
	Common::Array<Common::Rect> _boundaries;		// type 0x14

	// Runtime state
	double _carX = 0.0;			// current car position (map space)
	double _carY = 0.0;
	double _carHeading = 0.0;	// radians
	double _carVelocity = 0.0;
	double _speedCap = 0.0;		// current forward speed cap (lowered as the chaser closes in)
	int _triggeredDest = -1;	// destination zone the car has entered (-1 == none)

	// Chaser (kChase) runtime state.
	bool _chaseStarted = false;
	uint32 _chaseStartTime = 0;
	uint _chaserWaypoint = 0;
	double _chaserX = 0.0;
	double _chaserY = 0.0;
	double _chaserHeading = 0.0;

	Graphics::ManagedSurface _image;			// the town map
	Graphics::ManagedSurface _carImage;			// the player car rotation atlas
	Graphics::ManagedSurface _chaseCarImage;	// the chaser car rotation atlas
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_DRIVINGPUZZLE_H
