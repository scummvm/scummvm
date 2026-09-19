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

// Light-reflection puzzle introduced in Nancy12 (AR 163). The player turns a set
// of mirrors within their angle limits to route a beam of light from a source to
// a target zone.
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
		double step = 0.0;		// signed turn per tick; flips when a turn would cross a limit

		bool isRotatable() const { return minAngle != maxAngle; }
	};

	// An overlay zone's looping animation, drawn while the beam ends inside the zone.
	struct ZoneOverlay {
		uint zoneIndex = 0;
		Graphics::ManagedSurface image;
		uint frame = 0;
		uint32 nextFrameTime = 0;
	};

	// File data
	Common::Path _imageName;

	int16 _beamAngle = 0;		// initial beam angle (degrees)
	int32 _beamOriginX = 0;
	int32 _beamOriginY = 0;
	byte _beamColor[3] = {};	// r, g, b
	int16 _beamHalfWidth = 0;	// the beam is drawn this many pixels to each side of its center line
	double _beamCenterOpacity = 0.0;
	double _beamEdgeOpacity = 0.0;

	// Mirror sprite frames - the mirror appearance at each of kNumFrames angles
	// (full turn split evenly), indexed by angle.
	Common::Array<Common::Rect> _frameSrcRects;

	Common::Array<Mirror> _mirrors;

	Common::Array<ActionZone> _zones;

	// Runtime state
	int16 _hoveredMirror = -1;
	int _rotateDir = 0;					// +1 while the left button turns a mirror, -1 for the right one
	uint32 _nextRotateTime = 0;
	Common::Array<bool> _zoneOccupied;	// per zone: the beam currently ends inside it
	int _winZone = -1;					// the scene-change zone the beam reached
	bool _solved = false;
	Common::Array<Common::Point> _beamPath;	// traced beam polyline, in viewport coords
	Common::Array<ZoneOverlay> _overlays;

	Graphics::ManagedSurface _image;

	bool isAngleWithinLimits(const Mirror &m, double angle) const;
	uint frameForAngle(double angle) const;
	void drawMirror(uint index);
	void rotateMirror(uint index, int dir);
	void saveMirrorAngles();
	void traceBeam();
	void updateZones();
	void blendBeamPixel(int x, int y, double opacity);
	void drawBeamLine(Common::Point p0, Common::Point p1, double opacity);
	void drawBeam();
	void drawOverlays();
	void redraw();
	void playSoundBlock(const RandomSoundBlock &block);
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_MIRRORLIGHTPUZZLE_H
