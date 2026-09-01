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
#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/action/actionzone.h"

namespace Nancy {
namespace Action {

// Pachinko / pinball ball-drop puzzle, new in Nancy13 (AR 175).
//
// The player clicks a spring launcher on the right of the board to fire a ball leftward
// across a pin field. The ball falls under gravity, bouncing off the pins and the bumper
// zones, until it drops into one of four holes. Each hole feeds one of two climbers racing
// up the mountain to the pot: the Miner (a win) or the Yeti (a loss). The first to reach
// the pot plays its result animation, then exits through its own scene and event flag.
class PachinkoPuzzle : public RenderActionRecord {
public:
	PachinkoPuzzle() : RenderActionRecord(7) {}
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
		double x = 0.0;
		double y = 0.0;
		double speed = 0.0;			// px / second
		double angle = 0.0;			// radians; velocity = (cos a, -sin a) * speed
		uint frame = 0;
		bool active = false;
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
	Common::Point climberAnchor(const Machine &m) const;

	void redraw();
	void spawnBall();
	void stepBall(Ball &ball, double dt);
	bool collidePins(Ball &ball) const;
	int catchInHole(const Ball &ball) const;	// hole index the ball fell into, or -1
	void advanceMachine(Machine &m, uint32 now);
	SoundDescription playSoundBlock(const RandomSoundBlock &block);
	// Zone cursors take the idle sprite of their type, hover/drag cursors the hotspot one.
	void setDataCursor(uint16 cursorType, bool hotspotVariant = true) const;

	// -- File data --
	Common::Path _imageName;				// board overlay

	Common::Rect _ballSrc;					// ball sprite source
	Common::Rect _ballEntry;				// top-right entry chute (where balls appear)
	int32 _velMin = 0;						// launch-speed floor
	int32 _velMax = 0;						// launch-speed ceiling
	int32 _spawnYMin = 0;					// launch-heading (deg) range
	int32 _spawnYMax = 0;
	int32 _launchVecLen = 0;				// decorative launch nub length
	Common::Rect _panelBounds;				// pin-panel bounds (side walls + floor)
	int16 _eventFlag = 0;					// "in progress" flag id
	Common::Path _ballImageName;			// the ball sprite sheet
	Common::Rect _launcherBallSrc;			// ball-in-launcher sprite src
	Common::Rect _launcherBallDest;			// ball-in-launcher dest
	Common::Rect _launcherHotspot;			// the clickable launcher
	int32 _spawnWindowMin = 0;
	int32 _spawnWindowMax = 0;				// spawn window (ms)

	RandomSoundBlock _plinkSounds;			// random ball-launch cues

	Machine _winMachine;					// the Miner
	Machine _loseMachine;					// the Yeti

	Common::Array<Common::Rect> _pins;		// static pin collision rects
	Common::Array<ActionZone> _zones;		// bumpers / walls / overlays
	Common::Array<Hole> _holes;				// the four catch holes (built from _zones)

	// The give-up / exit hotspot.
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;			// set on give-up

	// -- Runtime state --
	enum State {
		kRunning,		// launcher live, balls in flight
		kPlayResult,	// a ball was caught; start the machine's result movie
		kWaitResult,	// wait for the result movie / cue
		kComplete		// finish -> scene change
	};
	State _pzState = kRunning;

	Common::Array<Ball> _balls;
	bool _spawnPending = false;				// a launcher click awaiting a spawn
	uint32 _spawnClickTime = 0;
	Machine *_activeMachine = nullptr;		// the climber that reached the pot
	bool _exitRequested = false;
	uint32 _lastUpdate = 0;
	uint32 _resultTime = 0;
	SoundDescription _resultSoundDesc;

	Common::Path _litImageName;				// "lit" board overlay (hole highlights)
	Graphics::ManagedSurface _image;		// board overlay
	Graphics::ManagedSurface _ballImage;	// ball sprite sheet
	Graphics::ManagedSurface _litImage;		// lit-hole sprites
	MoviePlayer _resultMovie;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PACHINKOPUZZLE_H
