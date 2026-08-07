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

#ifndef NANCY_ACTION_MINIGOLFPUZZLE_H
#define NANCY_ACTION_MINIGOLFPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/action/actionzone.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// Minigolf minigame introduced in Nancy12 (AR 161). The course is shown in a
// single static window: the player drops the ball on the tee square, then aims a
// golf club (angle + power) to putt it around a brick-walled course into the hole.
// Derives from the shared Nancy12 PuzzleBase layout (header + frame rects + two
// sound blocks + ActionZone array), see ActionZone.
class MinigolfPuzzle : public RenderActionRecord {
public:
	MinigolfPuzzle() : RenderActionRecord(7) {}
	virtual ~MinigolfPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "MinigolfPuzzle"; }

	void redraw();
	void drawBall();
	void drawGhostBall(const Common::Rect &src, const Common::Point &dest);
	void drawAimPreview();
	void playSoundBlock(const RandomSoundBlock &block);
	void launchBall(const Common::Point &maskCursor);
	void writeStrokeCount();			// mirror _strokes into the scorecard's TableData slot
	void aimToVelocity(double aimX, double aimY, double &vx, double &vy) const;
	void pipeExitVelocity(const ActionZone &zone, double inVx, double inVy, double &outVx, double &outVy) const;
	bool stepBall(double &x, double &y, double &vx, double &vy, bool playSounds);
	void updateBall();
	bool isWall(int px, int py) const;

	// The ball, zones and boundary mask live in a flat top-down "course" space (the
	// mask resolution). The screen is a 2:1 isometric projection of it: rotate 45
	// degrees about the mask centre and foreshorten Y by half, with the mask centre
	// at the viewport centre. Physics run in mask space; these convert for drawing
	// and for reading the cursor.
	Common::Point projectToScreen(double mx, double my) const;
	void unprojectToMask(int sx, int sy, double &mx, double &my) const;

	// File data
	Common::Path _ballImageName;		// GOL_Ball_OVL - the ball sprite sheet
	Common::Path _holeBoundaryName;		// GOL_Hole05B_BNDRY - course collision boundary OVL

	int32 _maxSpeed = 0;				// base+0x42, drag/speed scale
	double _decel = 0.0;				// base+0x46, per-frame linear deceleration (speed units/step)
	byte _launchMode = 0;				// base+0x4e, ==2 the ball starts pre-placed on the tee
	int16 _initialPower = 0;			// base+0x5f
	int16 _initialAngle = 0;			// base+0x61, degrees - the default aim direction
	int16 _strokeCountIndex = 0;			// base+0x63, TableData slot holding the stroke count for the scorecard
	byte _mirrorFlag = 0;				// base+0x69

	// A hole can have several cups, each with its own target scene/flag (e.g. hole
	// 4a's middle cup plays a cutscene). Set from the cup the ball actually drops in.
	SceneChangeDescription _winScene;
	bool _winHasFade = false;			// the cup's fade, played over the scene change
	byte _winFadeType = 0;
	uint16 _winFadeTotalTime = 0;
	uint16 _winFadeToBlackTime = 0;
	Common::Rect _winFadeRect;

	Common::Array<Common::Rect> _ballFrames;	// ball roll animation frames

	RandomSoundBlock _puttSound;		// played on launch
	RandomSoundBlock _wallSound;		// played on a wall bounce

	Common::Array<ActionZone> _zones;	// hole/sink/overlay zones
	Common::Array<bool> _inSlope;		// per-zone: ball currently inside a slope zone (for enter/leave kicks)
	Common::Array<uint> _sinkZones;		// derived: indices of the cup (sink) zones
	int _sunkZone = -1;					// zone index of the cup the ball dropped into
	RandomSoundBlock _reactionSound;	// derived: Nancy's voice line when the ball rolls over the cup

	// Runtime state
	enum State { kPlacing, kAiming, kMoving, kSunk };
	State _mgState = kPlacing;

	// Isometric projection anchors, computed in init().
	double _maskCenterX = 0.0;			// mask width / 2
	double _maskCenterY = 0.0;			// mask height / 2
	double _vpCenterX = 0.0;			// viewport width / 2
	double _vpCenterY = 0.0;			// viewport height / 2

	Common::Rect _teeRect;				// derived: the tee square (course space)
	uint32 _openColor = 0;				// boundary-mask colour that counts as open fairway

	double _ballX = 0.0;
	double _ballY = 0.0;
	double _velX = 0.0;
	double _velY = 0.0;
	byte _strokes = 0;					// strokes taken this hole; mirrored into _strokeCountIndex
	uint _ballFrame = 0;
	Common::Point _aimCursor;			// current cursor position while aiming
	uint32 _lastUpdate = 0;
	double _stepAccum = 0.0;			// leftover real time, integrated in fixed 30Hz steps
	uint32 _sunkTime = 0;
	bool _solved = false;
	bool _wasOverHole = false;			// ball was over the cup last step (to fire the reaction once per pass)

	// Pipe / teleport transit
	int _pipeZone = -1;					// zone holding the ball mid-transit, or -1
	uint32 _pipeReleaseTime = 0;		// when to emerge (for a timed hold)
	double _pipeInVx = 0.0;				// velocity on entry (for the "keep incoming" exit sentinels)
	double _pipeInVy = 0.0;
	bool _ballHidden = false;			// ball is inside a pipe (not drawn)

	Graphics::ManagedSurface _image;
	Graphics::ManagedSurface _boundaryMask;

	// Cosmetic overlay sprite (ActionZone type 0xd), e.g. hole 6a's broken wall.
	Graphics::ManagedSurface _overlayImage;
	Common::Rect _overlaySrc;
	Common::Rect _overlayDest;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MINIGOLFPUZZLE_H
