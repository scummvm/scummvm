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
//   167 - kChase    (kDriving plus a chaser car - Jane - that plays back a recorded path
//                    in real time; Nancy has to keep her in view. An event-flag-gated
//                    state machine drives the outcome: the win is the chaser completing
//                    its second path once Jane is caught; letting her drive off-view, or
//                    driving into a trigger zone, ends it otherwise)
//
// The map scrolls under a car-centered camera; the car is drawn as a rotation-atlas
// sprite whose frame is chosen from its heading. The map is populated with an
// ActionZone array: type 0x11 zones are location entrances (each carries the
// destination scene id and the transition effect), type 0x0d zones are cosmetic
// decorations (buildings, parked cars, potholes and animated cows/flags/fountains),
// and the rest are the driving hazards.
//
// Controls: the car steers to face the cursor; the left mouse button drives forward
// (the further from the car the cursor is, the faster) and the right button reverses.
// Fuel is a UI resource (index _frictionIndex) drained with the distance driven; potholes
// damage the tires; at 100 damage a tire blows and the car leaves for the flat-tire scene
// (blob+0x80). Mud slows the car; a location is entered by parking in its zone and pressing
// space. The car position, heading and accumulated tire damage persist across visits
// (DrivingData), like the original's retainState. The dashboard gas/tire gauges are not
// part of this record: they are the scene's own OverlayStaticTerse records gated by
// DT_RESOURCE dependencies on the fuel and tire UI resources.
//
// Collision uses the "...Collision" mask, whose white streets are drivable and dark
// areas are off-road; the type 0x14 boundary rects are only a fallback if it fails to load.
//
// TODO:
//  - kChase: the "caught Jane" transition (state 1 -> 2, the win) is gated on an event
//    flag the chase scene is expected to set (nothing in this record sets it); confirm
//    what triggers it so a missed catch can't still win.
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

	// A destination the car can drive into: a location entrance (type 0x11) or a drive-in
	// scene trigger (type 0x0c, used in the chase). Entering its map-space rect optionally
	// sets an event flag and transitions to the scene through a fade. Location entrances
	// need a spacebar press to enter (you park first); the drive-in trigger (autoTrigger)
	// fires the moment the car drives into it.
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
		bool autoTrigger = false;	// true for the chase finish (drive-in), false for parking
		bool carInside = false;		// the car was inside this zone last frame
	};

	// Index into _chaseParams (167). The five values are the chase's outcome scenes and
	// the event flags that gate its state machine.
	enum ChaseParam {
		kChaseGate01Flag = 0,		// clears to advance from kPursuit to kShortcut
		kChaseOffViewScene = 1,		// scene entered when the chaser leaves the viewport
		kChaseOffViewFlag = 2,		// flag set (to 1) on the off-viewport outcome
		kChaseGate12Flag = 3,		// sets to advance from kShortcut to kCaught (second path)
		kChasePathEndScene = 4		// scene entered when the chaser finishes its route
	};

	// The chase runs through three phases, driven by checkpoint flags:
	enum ChaseState {
		kPursuit = 0,	// Nancy must keep Jane in sight; losing her off-screen is a loss
		kShortcut = 1,	// Nancy lets Jane go and races her to the state line by another road
		kCaught = 2		// Jane is caught and plays her crash sequence on the second path
	};

	// A checkpoint (type 0x0b): driving into it sets an event flag, but only while its own
	// base condition holds. The chase sequences its phases this way - one checkpoint clears
	// the pursuit gate (starting the shortcut), a later one (gated on that) sets the caught
	// flag - so the condition must be honored, not just the rect.
	struct Checkpoint {
		Common::Rect rect;
		int16 flagId = -1;			// tail: the flag driven over sets
		byte flagValue = 0;			// tail: the value it sets
		int16 condFlag = -1;		// base zone val49: flag gating whether it can fire
		byte condValue = 0;			// base zone val4b: the value that arms it
		bool wasActive = false;		// (condition held AND car inside) last frame
	};

	// A mud puddle (type 0x03): slows the car (adds to its velocity decay) while inside.
	struct MudZone {
		Common::Rect rect;
		double decel = 0.0;
	};

	// A pothole (type 0x17): driving into it damages the tires by a random amount in
	// [minDamage, maxDamage].
	struct Pothole {
		Common::Rect rect;
		int32 minDamage = 0;
		int32 maxDamage = 0;
		bool carInside = false;		// the car was inside this zone last frame
	};

	// A removable road obstacle (type 0x14): a cow (or similar) that blocks the car while
	// its event-flag condition holds. Each has a matching 0x0d cow overlay gated on the same
	// flag, so the sprite and the collision appear and vanish together with the story state.
	struct Obstacle {
		Common::Rect rect;
		int16 condFlag = -1;			// base zone val49: event flag gating the block
		byte condValue = 0;				// base zone val4b: the flag value that activates it
	};

	// A cosmetic map decoration (type 0x0d): a sprite drawn onto the map at destRect.
	// A single source rect is static; several are animation frames cycled over time.
	// It is only visible while its event-flag condition holds (condFlag == -1 = always).
	struct Overlay {
		int imageIndex = -1;
		Common::Array<Common::Rect> srcRects;
		Common::Rect destRect;			// map space
		int16 condFlag = -1;			// base zone val49: event flag gating visibility
		byte condValue = 0;				// base zone val4b: the flag value that shows it
		bool aboveCar = false;			// layer 1 draws over the car (tall props), 0 under
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

	// Arms a pending exit (applied in kActionTrigger): from a destination zone (keeping
	// its fade), or from a raw scene id plus an optional event flag to set.
	void armExit(const DestinationZone &dest);
	void armExitScene(uint16 sceneID, int16 flag, byte flagValue);

	// Advances the car's velocity/position for one frame. Throttle is +1 forward, -1
	// reverse, 0 coast; cursorDist (how far the cursor is from the car) sets the forward
	// speed. The heading is set separately (steering toward the cursor).
	void updatePhysics(int throttle, double cursorDist);

	// Top-left of the car-centered camera window into the map, clamped to its bounds.
	Common::Point cameraOffset() const;

	// Persists the car's position/heading and tire state so it survives leaving the map
	// (and saving). Only does anything when the header's retainState flag is set.
	void saveState() const;

	// Refills the gas tank to the full amount from the UIRC boot chunk (the infinite-fuel cheat).
	void refillFuel();

	// Clears the accumulated pothole wear and restores the spare tire to good (the fix-tire cheat).
	void repairTire();

	// Whether a map-space point is off the road: off the map, or a non-white (dark)
	// pixel in the collision mask (its white marks the drivable streets).
	bool isWall(int px, int py) const;

	// Whether the car may not drive at this map-space point (off the road).
	bool isBlocked(const Common::Point &p) const;

	// Advances the chaser along its recorded path (kChase) and slows the player's
	// speed cap the closer the chaser gets.
	void updateChaser();

	// Chooses a rotation-atlas frame from a heading.
	uint frameIndexForHeading(double heading, uint frameCount) const;

	// Loads (and caches) a decoration sprite by name, returning its index into
	// _overlayImages, or -1 on failure.
	int overlayImageIndex(const Common::String &name);

	// Draws the map's cosmetic decorations (animated frames cycled over time), offset by
	// the camera and clipped to the visible window.
	void drawOverlays(const Common::Point &cam, bool aboveCar);

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
	int16 _frictionIndex = 0;	// blob+0x77: UIRC resource index for the fuel gauge
	static const uint kTireResourceIndex = 2;	// UIRC resource index for the tire gauge (1 = good)
	int32 _distanceDivisor = 0;	// blob+0x7b
	bool _retainState = false;	// blob+0x7f: resume from the saved position
	uint16 _finishScene = kNoScene;	// blob+0x80: the scene entered when a tire goes flat

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
	Common::Array<DestinationZone> _destinations;	// types 0x11 / 0x0c (parking spaces)
	Common::Array<Checkpoint> _checkpoints;			// type 0x0b
	Common::Array<MudZone> _mudZones;				// type 0x03
	Common::Array<Pothole> _potholes;				// type 0x17
	Common::Array<Overlay> _overlays;				// type 0x0d (map decorations)
	Common::Array<Obstacle> _obstacles;				// type 0x14 (flag-gated road obstacles)
	Common::Array<Graphics::ManagedSurface> _overlayImages;
	Common::Array<Common::String> _overlayImageNames;

	// Runtime state
	double _carX = 0.0;			// current car position (map space)
	double _carY = 0.0;
	double _carHeading = 0.0;	// radians
	double _carVelocity = 0.0;	// pixels per second
	double _speedCap = 0.0;		// current forward speed cap (lowered as the chaser closes in)
	uint32 _lastPhysicsMs = 0;	// real time of the last physics step (frame-rate independence)
	int _parkedDest = -1;		// destination zone the car is currently parked in (-1 == none)

	// A pending exit to another scene (a location, the chase finish, or a chase outcome).
	// Armed via armExit()/armExitScene(); applied and finished in the kActionTrigger state.
	SceneChangeDescription _exitScene;
	bool _exitHasFade = false;
	byte _exitFadeType = 0;
	uint16 _exitFadeTotalTime = 0;
	uint16 _exitFadeToBlackTime = 0;
	Common::Rect _exitFadeRect;
	int16 _exitFlag = -1;
	byte _exitFlagValue = 0;

	// Chase (167) state machine: 0 = following the first path, 1 = waiting to switch,
	// 2 = following the second path.
	ChaseState _chaseState = kPursuit;
	bool _chaserOnPathB = false;

	// Fuel + tire hazards. Fuel is the gas-gauge UI resource (index _frictionIndex),
	// drained as the car drives; the fractional part is accumulated here. Tire damage
	// builds up from potholes; once it blows a tire the car leaves for the flat-tire
	// scene while _flatTirePending waits for the blowout sound to finish.
	double _fuelBurnAccum = 0.0;
	int _tireDamage = 0;
	bool _flatTirePending = false;
	bool _infiniteFuel = false;	// cheat: Ctrl+Shift+G tops the tank and stops it draining

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
	Graphics::ManagedSurface _collisionMask;	// road/off-road mask ("MAP_TitusvilleCollision")
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_DRIVINGPUZZLE_H
