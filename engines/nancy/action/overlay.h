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

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Places a static image or a looping animation on top of the background
// Can move along with the scene's background frame, however:
// - in animation mode, the animation is the same for every background frame
// - in static mode, every background frame gets its own static image
// Also supports:
// - playing a sound;
// - playing backwards;
// - looping (non-looping animated overlays are very rare);
// - getting interrupted by an event flag;
// - changing the scene/setting flags when clicked/interrupted
// Originally introduced in nancy1, where it was split into two different types:
// PlayStaticBitmapAnimation and PlayIntStaticBitmapAnimation (the latter was interruptible)
// In nancy2, the two got merged inside the newly-renamed Overlay;
// that was also when static mode got introduced.
class Overlay : public RenderActionRecord {
public:
	Overlay(bool interruptible) : RenderActionRecord(7), _isInterruptible(interruptible), _usesAutotext(false) {}
	virtual ~Overlay() { _fullSurface.free(); }

	void init() override;
	void handleInput(NancyInput &input) override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Path _imageName;

	uint16 _transparency = kPlayOverlayPlain;
	uint16 _hasSceneChange = kPlayOverlaySceneChange;
	uint16 _enableHotspot = kPlayOverlayNoHotspot;
	uint16 _overlayType = kPlayOverlayAnimated;
	uint16 _playDirection = kPlayOverlayForward;
	uint16 _loop = kPlayOverlayOnce;
	uint16 _firstFrame = 0;
	uint16 _loopFirstFrame = 0;
	uint16 _loopLastFrame = 0;
	uint32 _frameTime = 0;
	FlagDescription _interruptCondition;
	SceneChangeDescription _sceneChange;
	MultiEventFlagDescription _flagsOnTrigger;

	Nancy::SoundDescription _sound;

	// Describes a single frame in this animation
	Common::Array<Common::Rect> _srcRects;
	// Describes how the animation will be displayed on a single
	// frame of the viewport
	Common::Array<FrameBlitDescription> _blitDescriptions;

	int16 _currentFrame = -1;
	int16 _currentViewportFrame = -1;
	uint32 _nextFrameTime = 0;
	bool _isInterruptible;
	bool _usesAutotext;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override;
	bool isViewportRelative() const override { return true; }

	Graphics::ManagedSurface _fullSurface;
};

// Short version of a static overlay; assumes scene background doesn't move
class OverlayStaticTerse : public Overlay {
public:
	OverlayStaticTerse() : Overlay(true) {}
	virtual ~OverlayStaticTerse() {}

	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "OverlayStaticTerse"; }
};

// Short version of an animated overlay; assumes scene background doesn't move
class OverlayAnimTerse : public Overlay {
public:
	OverlayAnimTerse() : Overlay(true) {}
	virtual ~OverlayAnimTerse() {}

	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "OverlayAnimTerse"; }
};

class TableIndexOverlay : public Overlay {
public:
	TableIndexOverlay() : Overlay(true) {}
	virtual ~TableIndexOverlay() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "TableIndexOverlay"; }

	uint16 _tableIndex = 0;
	int16 _lastIndexVal = -1;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_OVERLAY_H
