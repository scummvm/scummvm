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

// Pachinko / pinball ball-drop puzzle, new in Nancy13 (AR 175.
//
// The player clicks a spring launcher (a fixed hotspot on the right of the board) to
// fire a ball leftward across a pin field. The ball falls under gravity and bounces off
// pins and bumper zones (restitution ~0.85) until it settles into one of two catch
// "machines": the Miner (a win) or the Yeti (a loss). Each machine then plays its own
// result animation (MUS_PachinkoWinANIM for the Miner) before the puzzle finishes.
//
// The chunk is a 167-byte header, a random "plink" sound block, two machine sub-objects
// (each: an ANIM sprite strip + a slide "mover" + three sound blocks + a 55-byte blob
// whose leading filename is the result movie), a pin-rect array, a polymorphic Nancy13
// ActionZone array (the bumpers/walls/overlays), and the give-up exit hotspot. The parse
// is byte-exact (verified to consume the whole chunk).
//
// The physics uses a polar-coordinate integrator: velocity as speed + heading, gravity
// added in cartesian, per-frame heading recomputed with atan2, sub-stepped rectangle
// collision with restitution. Per-ball deceleration is not in the chunk, so it is
// approximated (see kDrag).
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

	// One of the two "machine" sub-objects (Miner = win, Yeti = lose). Each is an
	// animated sprite whose frames slide from a start point to an end (catch) point.
	struct Machine {
		Common::Path imageName;					// the ANIM_OVL sprite strip
		int32 animRate = 0;						// frames per second
		Common::Array<Common::Rect> frames;		// sprite-strip source rects
		Common::Rect moverStart;				// slide path start rect (its centre is the anchor)
		Common::Rect moverEnd;					// slide path end / catch rect
		int32 moverSpeed = 0;
		RandomSoundBlock winchSound;			// [snd1] the winch-up cue
		RandomSoundBlock resultSound;			// [snd2] the win/lose voice cue (MinerWin*/PachinkoLose*)
		RandomSoundBlock fastSound;				// [snd3] the fast-winch cue
		Common::Path movieName;					// result animation (blob[0], "" == none)

		Graphics::ManagedSurface image;
		uint frame = 0;							// current animation frame
		uint32 nextFrameTime = 0;

		Common::Point catchPoint() const {
			return Common::Point((moverEnd.left + moverEnd.right) / 2,
				(moverEnd.top + moverEnd.bottom) / 2);
		}
	};

	// A single launched ball. Physics run in viewport space; the heading is stored as a
	// scalar speed plus a heading angle in radians.
	struct Ball {
		double x = 0.0;
		double y = 0.0;
		double speed = 0.0;			// px / second
		double angle = 0.0;			// radians; velocity = (cos a, -sin a) * speed
		uint frame = 0;
		bool active = false;
	};

	void readMachine(Common::SeekableReadStream &stream, Machine &m);
	void loadMachineImage(Machine &m);

	void redraw();
	void spawnBall();
	void stepBall(Ball &ball, double dt);
	bool collideBall(Ball &ball, double nx, double ny) const;
	bool ballSettled(const Ball &ball, const Machine &m) const;
	void advanceMachine(Machine &m, uint32 now);
	SoundDescription playSoundBlock(const RandomSoundBlock &block);
	void setDataCursor(uint16 cursorType) const;

	// -- File data (167-byte header, in stream order) --
	Common::Path _imageName;				// 0x00 - board overlay (MUS_PachinkoPUZ02_OVL)

	Common::Rect _ballSrc;					// ball sprite source in the overlay
	int32 _velMin = 0;						// launch-speed floor
	int32 _velMax = 0;						// launch-speed ceiling
	int32 _spawnYMin = 0;					// launch-heading (deg) range
	int32 _spawnYMax = 0;
	int32 _launchVecLen = 0;				// decorative launch nub length
	int16 _eventFlag = 0;					// "in progress" flag id
	Common::Path _ballImageName;			// the ball sprite sheet
	Common::Rect _launcherBallSrc;			// ball-in-launcher sprite src
	Common::Rect _launcherBallDest;			// ball-in-launcher dest
	Common::Rect _launcherHotspot;			// the clickable launcher
	int32 _spawnWindowMin = 0;
	int32 _spawnWindowMax = 0;				// spawn window (ms)

	RandomSoundBlock _plinkSounds;			// random ball-launch cues (LeverPull*)

	Machine _winMachine;					// the Miner
	Machine _loseMachine;					// the Yeti

	Common::Array<Common::Rect> _pins;		// static pin collision rects
	Common::Array<ActionZone> _zones;		// bumpers / walls / overlays (Nancy13 layout)

	// The give-up / exit hotspot (the base trailer's 23-byte record).
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
	Machine *_activeMachine = nullptr;		// the machine that caught the ball
	bool _solved = false;
	bool _exitRequested = false;
	uint32 _lastUpdate = 0;
	uint32 _resultTime = 0;
	SoundDescription _resultSoundDesc;

	Graphics::ManagedSurface _image;		// board overlay
	Graphics::ManagedSurface _ballImage;	// ball sprite sheet
	MoviePlayer _resultMovie;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PACHINKOPUZZLE_H
