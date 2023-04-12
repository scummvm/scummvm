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

#ifndef NANCY_ACTION_OVERLAY_H
#define NANCY_ACTION_OVERLAY_H

#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// ActionRecord describing an overlay on top of the viewport.
// That overlay can be either a short animation, or a static bitmap
// that changes depending on the current viewport frame.
// This class covers three different ActionRecord types:
// - PlayStaticBitmapAnimation: nancy1 only, does not support static mode
// - PlayIntStaticBitmapAnimation: nancy1 only, same as above but supports being interrupted by an event flag
// - Overlay: nancy2 and above, supports static mode
class Overlay : public ActionRecord, public RenderObject {
public:
	static const byte kPlayOverlayPlain				= 1;
	static const byte kPlayOverlayTransparent		= 2;

	static const byte kPlayOverlaySceneChange		= 1;
	static const byte kPlayOverlayNoSceneChange 	= 2;

	static const byte kPlayOverlayStatic			= 1;
	static const byte kPlayOverlayAnimated			= 2;

	static const byte kPlayOverlayOnce				= 1;
	static const byte kPlayOverlayLoop				= 2;

	static const byte kPlayOverlayForward			= 1;
	static const byte kPlayOverlayReverse			= 2;

	static const byte kPlayOverlayWithHotspot		= 1;
	static const byte kPlayOverlayNoHotspot			= 2;

	Overlay(bool interruptible) : RenderObject(7), _isInterruptible(interruptible) {}
	virtual ~Overlay() { _fullSurface.free(); }

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void onPause(bool pause) override;

	Common::String _imageName;

	uint16 _transparency = kPlayOverlayPlain;
	uint16 _hasSceneChange = kPlayOverlaySceneChange;
	uint16 _enableHotspot = kPlayOverlayNoHotspot;
	uint16 _overlayType = kPlayOverlayAnimated;
	uint16 _playDirection = kPlayOverlayForward;
	uint16 _loop = kPlayOverlayOnce;
	uint16 _firstFrame = 0;
	uint16 _loopFirstFrame = 0;
	uint16 _loopLastFrame = 0;
	Time _frameTime;
	FlagDescription _interruptCondition;
	SceneChangeDescription _sceneChange;
	MultiEventFlagDescription _flagsOnTrigger; // 0x2A

	Nancy::SoundDescription _sound; // 0x52

	// Describes a single frame in this animation
	Common::Array<Common::Rect> _srcRects;
	// Describes how the animation will be displayed on a single
	// frame of the viewport
	Common::Array<BitmapDescription> _bitmaps;

	int16 _currentFrame = -1;
	int16 _currentViewportFrame = -1;
	Time _nextFrameTime;
	bool _isInterruptible;

protected:
	Common::String getRecordTypeName() const override;
	bool isViewportRelative() const override { return true; }

	void setFrame(uint frame);

	Graphics::ManagedSurface _fullSurface;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_OVERLAY_H
