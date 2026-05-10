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

#ifndef NANCY_ACTION_CUTTINGPUZZLE_H
#define NANCY_ACTION_CUTTINGPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Action {

// Art Studio Lathe Puzzle in Nancy 8.
//
// The player uses a lathe to cut grooves into a wooden dowel. The UI has:
//   - A blade that can be moved to different horizontal positions (marker rects)
//   - A lever that sets the depth of the cut (0=none, 1=shallow, 2=medium, 3=deep)
//   - A start/stop switch that runs the lathe
//
// Running the lathe plays an animation over 14 macro-cycles. When the animation
// completes, the groove depth at the current blade position is recorded as the
// maximum of the previous depth and the current lever setting.
//
// The puzzle is solved when grooveDepths[i] == correctGrooves[i] for all i.
//
class CuttingPuzzle : public RenderActionRecord {
public:
	CuttingPuzzle() : RenderActionRecord(7) {}
	virtual ~CuttingPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	enum SubState {
		kIdle           = 0, // waiting for player input, or checking solution
		kLatheFinished  = 1, // one lathe run ended; determine outcome
		kWaitTimer      = 2, // waiting for frame-advance delay
		kWaitDoneSound  = 3  // waiting for the completion sound to finish
	};

	Common::String getRecordTypeName() const override { return "CuttingPuzzle"; }

	void redrawSurface();
	int grooveTypeForIndex(int i) const;

	// ---- data-file fields ----

	Common::Path _imageName1;   // data+0x000 sprite sheet (background/restore image)
	Common::Path _imageName2; // data+0x021 sprite sheet (interactive overlays)

	uint16 _numGrooves  = 0;      // data+0x042  how many grooves to cut
	// data+0x044 skipped (2 bytes)
	// data+0x046 skipped (2 bytes)

	// Destination rects (viewport-local screen positions to draw to)
	Common::Rect _noAnimDest;                    // data+0x048
	Common::Rect _leverDest;                     // data+0x058
	Common::Rect _switchDest;                    // data+0x068
	Common::Array<Common::Rect> _grooveDest;     // data+0x078 (8 rects, one per groove slot)
	Common::Array<Common::Rect> _markerDest;     // data+0x0f8 (8 rects, blade position markers)
	Common::Array<Common::Rect> _bladeDest;      // data+0x178 (8 rects, blade assembly positions)
	Common::Rect _animFrameDest;                 // data+0x1f8

	// Source rects (within the sprite sheet)
	Common::Rect _noAnimSrc;                     // data+0x208  static overlay when lathe is off
	Common::Array<Common::Rect> _leverSrc;       // data+0x218  3 rects for lever depths 1..3
	Common::Rect _switchOnSrc;                   // data+0x248  switch sprite when lathe is on
	Common::Array<Common::Rect> _grooveTypeSrc;  // data+0x258  6 rects for groove visual types 0..5
	Common::Array<Common::Rect> _bladeSrc;       // data+0x2b8  4 rects for blade at lever depths 0..3
	Common::Rect _baseSrc;                       // data+0x2f8  base/knob sprite (opaque)

	uint16 _numAnimFrames = 0;                   // data+0x308
	Common::Array<Common::Rect> _animSrc;        // data+0x30a  12 animation frame rects
	uint16 _frameDelayMs  = 0;                   // data+0x3ca  delay between animation frames (ms)

	Common::Array<int16> _correctGrooves;        // data+0x3cc  8 target depths (one per groove slot)

	SoundDescription _latheSound;     // data+0x3dc  looping lathe running sound
	SoundDescription _moveSound;      // data+0x40d  move left & right sound
	SoundDescription _startStopSound; // data+0x43e  on/off switch sound
	SoundDescription _depthSound;     // data+0x46f  depth adjust sound
	SoundDescription _cutSound;       // data+0x4a0  groove-cutting sound

	SceneChangeWithFlag _puzzleSolvedScene;    // data+0x4d1 (25 bytes)
	uint16 _doneSoundDelaySecs = 0;        // data+0x4ea  wait before playing done sound (seconds)
	SoundDescription _doneSound;           // data+0x4ec

	byte  _itemCheckByte = 0;              // data+0x51d  0=no check, nonzero=require inventory item
	int16 _itemID        = -1;             // data+0x51e

	SceneChangeDescription _missingGogglesScene;    // data+0x520  (20 bytes + 2-byte skip = 22 bytes total)
	SceneChangeWithFlag    _cancelScene;   // data+0x536  (25 bytes)
	Common::Rect _exitHotspot;

	// ---- runtime state ----

	Graphics::ManagedSurface _image1;
	Graphics::ManagedSurface _image2;

	uint16 _currentMarkerPos   = 0; // blade position index (0..numGrooves-1)
	uint16 _currentLeverDepth  = 0; // lever depth (0=none, 1=shallow, 2=med, 3=deep)
	Common::Array<int16> _grooveDepths;  // recorded depth at each groove position

	bool _latheRunning  = false;
	bool _animRestore   = false; // true when lathe just stopped: restore animation area on next draw
	bool _leverReset    = false; // true when lever just snapped back to 0: show reset animation once
	uint16 _animFrame   = 0;     // current animation frame index (0..numAnimFrames-1)

	SubState _subState  = kIdle;
	bool _cancelled     = false;
	bool _solved        = false;
	bool _gogglesMissing      = false;
	uint32 _timerDeadline = 0;
	uint16 _macroCycleCount = 0; // counts lathe macro-cycles (0..14); at 14 the run ends
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_CUTTINGPUZZLE_H
