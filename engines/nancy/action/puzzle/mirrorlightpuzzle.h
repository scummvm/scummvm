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

#ifndef NANCY_ACTION_MIRRORLIGHTPUZZLE_H
#define NANCY_ACTION_MIRRORLIGHTPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionzone.h"

namespace Nancy {
namespace Action {

// Light-reflection puzzle introduced in Nancy12 (AR 163). The player rotates a
// set of mirrors within their angle limits to route a beam of light from a
// source to a target.
class MirrorLightPuzzle : public RenderActionRecord {
public:
	MirrorLightPuzzle() : RenderActionRecord(7) {}
	virtual ~MirrorLightPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

	static const uint16 kMaxMirrors = 20;

protected:
	Common::String getRecordTypeName() const override { return "MirrorLightPuzzle"; }

	struct Mirror {
		Common::Rect destRect;	// where the mirror is drawn / clicked
		double angle = 0.0;		// current angle (radians)
		double minAngle = 0.0;	// rotation limits (min == max == fixed mirror)
		double maxAngle = 0.0;
	};

	// File data
	Common::Path _imageName;

	int16 _beamAngle = 0;		// initial beam angle (degrees)
	int32 _beamOriginX = 0;
	int32 _beamOriginY = 0;
	int16 _glowRadius = 0;		// beam-glow half-width in pixels

	// Mirror sprite frames - the mirror appearance at each of kNumFrames angles
	// (full turn split evenly), indexed by angle.
	Common::Array<Common::Rect> _frameSrcRects;

	Common::Array<Mirror> _mirrors;

	Common::Array<ActionZone> _zones;

	// Derived from the detector zone (the SpecialEffect zone at the bulb): its
	// rect is the target the beam must reach, its specialEffectId is the win scene.
	Common::Rect _detectorRect;
	SceneChangeDescription _winScene;

	// Runtime state
	int16 _pickedUpMirror = -1;
	bool _solved = false;
	uint32 _solvedTime = 0;					// ms timestamp when solved, for the win hold
	Common::Array<Common::Point> _beamPath;	// traced beam polyline, in viewport coords

	Graphics::ManagedSurface _image;

	uint frameForAngle(double angle) const;
	void drawMirror(uint index);
	void rotateMirror(uint index, bool clockwise);
	void traceBeam();
	void drawBeamGlow();
	void redraw();
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MIRRORLIGHTPUZZLE_H
