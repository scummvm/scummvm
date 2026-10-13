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

#ifndef NANCY_ACTION_PACHINKOPUZZLE_H
#define NANCY_ACTION_PACHINKOPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/action/puzzlerecord.h"
#include "engines/nancy/action/actionzone.h"
#include "engines/nancy/action/physicalobject.h"

namespace Nancy {
namespace Action {

// Pachinko / pinball ball-drop puzzle, new in Nancy13 (AR 175).
//
// The player pulls the lever on the right of the board and lets go of it to fire a ball
// leftward across a pin field; the longer the pull, the faster the ball. The ball falls
// under gravity, bouncing off the pins and the frame of the board, until it drops into
// one of four holes or reaches the bottom. Each hole feeds one of two climbers racing
// up the mountain to the pot: the Miner (a win) or the Yeti (a loss). The first to reach
// the pot plays its result animation, then exits through its own scene and event flag.
class PachinkoPuzzle : public PuzzleRecord {
public:
	PachinkoPuzzle() : PuzzleRecord(7) {}
	virtual ~PachinkoPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "PachinkoPuzzle"; }

	// One of the two mountain climbers (Miner = win, Yeti = lose). An animated sprite that
	// climbs from the bottom of the mountain up to the pot as balls fall into its holes.
	struct Machine {
		Common::Path imageName;					// the sprite strip
		int32 animRate = 0;						// frames per second
		Common::Array<Common::Rect> frames;		// sprite-strip source rects
		Common::Rect moverStart;				// climb-path bottom anchor
		Common::Rect moverEnd;					// climb-path top anchor (the pot)
		int32 moverSpeed = 0;					// climb steps gained per ball caught
		RandomSoundBlock winchSound;			// the winch-up cue
		RandomSoundBlock resultSound;			// the win/lose voice cue
		RandomSoundBlock fastSound;				// the fast-winch cue
		Common::Path movieName;					// result animation ("" == none)
		Common::Rect movieDest;					// where the result animation is drawn
		SceneChangeDescription resultScene;		// where the puzzle exits when this climber wins
		FlagDescription resultFlag;				// the event flag it sets on the way out

		Graphics::ManagedSurface image;
		uint frame = 0;							// current animation frame
		uint32 nextFrameTime = 0;
		int climbSteps = 0;						// accumulated climb (moverSpeed per catch)
	};

	// A single launched ball, moving in viewport space.
	struct Ball {
		PhysicalObject body;
	};

	// One of the four holes on the panel. A ball that drops in advances its climber and
	// briefly lights the hole.
	struct Hole {
		Common::Rect rect;
		Machine *climber = nullptr;
		RandomSoundBlock sound;			// the bell cue
		Common::Rect litSrc;			// lit-hole sprite source (in _litImage)
		Common::Rect litDest;			// where it is drawn
		uint32 litUntil = 0;			// keep it lit until this time
	};

	void readMachine(Common::SeekableReadStream &stream, Machine &m);
	void loadMachineImage(Machine &m);
	void buildHoles();
	void buildObstacles();
	Common::Point climberAnchor(const Machine &m) const;

	void redraw();
	void spawnBall();
	void updateLever();
	void releaseLever();
	int catchInHole(const Common::Point &pos) const;	// hole index the ball fell into, or -1
	bool isInDrain(const Common::Point &pos) const;
	void advanceMachine(Machine &m, uint32 now);

	// -- File data --

	Common::Rect _ballSrc;					// ball sprite source
	Common::Rect _ballEntry;				// top-right entry chute (where balls appear)
	int32 _velMin = 0;						// launch-speed floor
	int32 _velMax = 0;						// launch-speed ceiling
	int32 _spawnYMin = 0;					// launch-heading (deg) range
	int32 _spawnYMax = 0;
	int32 _gravity = 0;						// downward pull on the balls
	int32 _friction = 0;					// how fast the balls slow down
	Common::Rect _panelBounds;				// the frame of the pin panel, which the balls bounce off
	int16 _eventFlag = 0;					// "in progress" flag id
	Common::Path _ballImageName;			// the ball sprite sheet
	Common::Rect _leverSrc;					// the lever, pulled all the way out
	Common::Rect _leverDest;				// where the lever is when at rest
	Common::Rect _launcherHotspot;			// the clickable launcher
	int32 _pullTimeMin = 0;					// shortest pull that launches a ball (ms)
	int32 _pullTimeMax = 0;					// a pull this long launches at full speed (ms)

	RandomSoundBlock _plinkSounds;			// random ball-launch cues

	Machine _winMachine;					// the Miner
	Machine _loseMachine;					// the Yeti

	Common::Array<Common::Rect> _pins;		// static pin collision rects
	Common::Array<ActionZone> _zones;		// bumpers / walls / overlays
	Common::Array<Hole> _holes;				// the four catch holes (built from _zones)
	Common::Array<PhysicalObject::Obstacle> _obstacles;	// the pins and the cups under the holes

	// -- Runtime state --
	enum State {
		kRunning,		// launcher live, balls in flight
		kPlayResult,	// a ball was caught; start the machine's result movie
		kWaitResult,	// wait for the result movie / cue
		kComplete		// finish -> scene change
	};
	State _pzState = kRunning;

	Common::Array<Ball> _balls;
	bool _leverPulled = false;				// the mouse button is down on the lever
	uint32 _leverPullTime = 0;				// when the pull started
	Common::Rect _leverCurSrc;				// the part of the lever showing right now
	Machine *_activeMachine = nullptr;		// the climber that reached the pot
	bool _exitRequested = false;
	uint32 _lastUpdate = 0;
	uint32 _resultTime = 0;
	SoundDescription _resultSoundDesc;

	Common::Path _litImageName;				// "lit" board overlay (hole highlights)
	Graphics::ManagedSurface _ballImage;	// ball sprite sheet
	Graphics::ManagedSurface _litImage;		// lit-hole sprites
	MoviePlayer _resultMovie;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PACHINKOPUZZLE_H
