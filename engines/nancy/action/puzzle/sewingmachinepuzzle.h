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

#ifndef NANCY_ACTION_SEWINGMACHINEPUZZLE_H
#define NANCY_ACTION_SEWINGMACHINEPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/navigationrecords.h"
#include "engines/nancy/action/actionzone.h"

namespace Nancy {
namespace Action {

// Nancy12 sewing-machine puzzle (action record 162): the player drags a tall cloth
// image under a fixed needle - vertical feeds it, horizontal steers it onto the
// pre-drawn seam. Straying off the seam plays one of Nancy's mistake lines; reaching
// the end cross-dissolves to the trigger zone's win scene.
// TODO: the needle spot comes from the scene's needle-overlay rect, not from this
// record's own data.
class SewingMachinePuzzle : public RenderActionRecord {
public:
	SewingMachinePuzzle() : RenderActionRecord(7) {}
	virtual ~SewingMachinePuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "SewingMachinePuzzle"; }

	// Sorts the ActionZone array into its gameplay roles.
	void classifyZones();
	// Plays one entry of a random-sound block (needle/stitch cues).
	void playSoundBlock(const RandomSoundBlock &block);
	// Draws the visible part of the cloth strip plus the stitches sewn so far.
	void drawCloth();

	// Where the needle currently sits within the cloth strip.
	Common::Point needleInStrip() const;
	// Advances the sewing by a drag delta, marking the puzzle solved once the whole
	// seam has been fed through.
	void feedCloth(const Common::Point &delta);
	// Whether a bottom trigger fires: the needle is inside it and its mistake-flag gate
	// matches (narrow = clean run, wide = a mistake was made).
	bool triggerFires(const ActionZone &z, const Common::Point &needle) const;

	// Tests the needle against the seam mask; leaving the marked corridor plays a
	// mistake line and sets the zone's flag (edge-triggered, once per excursion).
	void checkSeam();

	// Background image ("BED_Sewing_OVL").
	Common::Path _imageName;

	// 87-byte PuzzleBase header blob: image name + two cloth regions + a direction
	// vector + an extent vector (its magnitude = the seam length) + three params
	// (_params[1] = stitch distance threshold, _params[2] = its hysteresis).
	Common::Rect _rects[2];
	Common::Point _directionVector;
	Common::Point _extentVector;
	int16 _params[3] = {};

	RandomSoundBlock _soundBlock;
	Common::Array<ActionZone> _zones;

	// ActionZone roles (indices into _zones; -1 == none).
	int _collisionZone = -1;			// type 0x0b: the needle line + its sounds/flag
	int _boundaryZone = -1;				// type 0x14
	Common::Array<uint> _triggerZones;	// type 0x0c: completion triggers

	// Runtime state. The cloth is drawn 1:1 at _offset and dragged freely under the
	// fixed needle (vertical = feed, horizontal = steer); the viewport clips it.
	Common::Point _offset;				// cloth's top-left on screen
	int _minOffsetX = 0;
	int _maxOffsetX = 0;
	int _minOffsetY = 0;
	int _maxOffsetY = 0;
	Common::Point _needleScreen;		// needle's fixed spot on screen
	bool _dragging = false;
	Common::Point _lastDragPos;
	bool _solved = false;

	// Stitches sewn so far, in cloth-image space, drawn as a dark dashed thread.
	Common::Array<Common::Point> _stitches;

	// The seam mask covers only the cloth's seam region (the collision zone's rect),
	// so cloth pixels map to mask pixels by that origin + scale, not the whole cloth.
	uint32 _offSeamColor = 0;			// mask background color (off the seam)
	Common::Point _maskOrigin;			// cloth-space top-left of the seam mask
	double _maskScaleX = 1.0;			// seam-mask pixels per cloth pixel (x)
	double _maskScaleY = 1.0;			// seam-mask pixels per cloth pixel (y)
	bool _hasSeamMask = false;
	bool _offSeam = false;				// the needle was off the seam last check

	Graphics::ManagedSurface _image;
	Graphics::ManagedSurface _seamMask;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SEWINGMACHINEPUZZLE_H
