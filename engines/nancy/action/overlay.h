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
	enum AnimationType { kStaticAnimation, kInterruptibleAnimation };

	Overlay(AnimationType animationType) : RenderActionRecord(7), _animationType(animationType), _usesAutotext(false) {}
	virtual ~Overlay() { _fullSurface.free(); }

	void init() override;
	void handleInput(NancyInput &input) override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Path _imageName;

	uint16 _transparency = kPlayOverlayPlain;
	uint16 _hasSceneChange = kPlayOverlaySceneChange;
	uint16 _enableHotspotNancy2 = kPlayOverlayNoHotspot;
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

	SoundDescription _sound;

	// Describes a single frame in this animation
	Common::Array<Common::Rect> _srcRects;
	// Describes how the animation will be displayed on a single
	// frame of the viewport
	Common::Array<FrameBlitDescription> _blitDescriptions;

	int16 _currentFrame = -1;
	int16 _currentViewportFrame = -1;
	uint32 _nextFrameTime = 0;
	AnimationType _animationType;
	bool _usesAutotext;

	bool canHaveHotspot() const override { return true; }
	bool isViewportRelative() const override { return true; }
	bool survivesSceneChange(bool nextSceneIsNoArt) const override { return nextSceneIsNoArt; }
	Common::String getRecordExtraInfo() const override {
		return Common::String::format("Scene %d, file %s", _sceneChange.sceneID, _imageName.baseName().c_str());
	}

protected:
	Common::String getRecordTypeName() const override;

	Graphics::ManagedSurface _fullSurface;
};

// Short version of a static overlay; assumes scene background doesn't move
class OverlayStaticTerse : public Overlay {
public:
	OverlayStaticTerse() : Overlay(kInterruptibleAnimation) {}
	virtual ~OverlayStaticTerse() {}

	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "OverlayStaticTerse"; }
};

// Short version of a static overlay for a moving scene background. Unlike
// OverlayStaticTerse, which carries a single source/destination pair, this one
// carries a blit description for every background frame the overlay appears on.
class OverlayMultiframeTerse : public Overlay {
public:
	OverlayMultiframeTerse() : Overlay(kInterruptibleAnimation) {}
	virtual ~OverlayMultiframeTerse() {}

	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "OverlayMultiframeTerse"; }
};

// Short version of an animated overlay; assumes scene background doesn't move
class OverlayAnimTerse : public Overlay {
public:
	OverlayAnimTerse() : Overlay(kInterruptibleAnimation) {}
	virtual ~OverlayAnimTerse() {}

	void readData(Common::SeekableReadStream &stream) override;

protected:
	Common::String getRecordTypeName() const override { return "OverlayAnimTerse"; }
};

class TableIndexOverlay : public Overlay {
public:
	TableIndexOverlay() : Overlay(kInterruptibleAnimation) {}
	virtual ~TableIndexOverlay() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "TableIndexOverlay"; }

	uint16 _tableIndex = 0;
	int16 _lastIndexVal = -1;
};

// Draws a single line of text on top of the scene background. The text is a
// value looked up from the player-data table (used by the nancy12 minigolf
// scorecard, where each hole's score is a separate record).
class TextLineOverlay : public RenderActionRecord {
public:
	TextLineOverlay() : RenderActionRecord(8) {}
	virtual ~TextLineOverlay() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "TextLineOverlay"; }

	uint16 _fontID = 0;
	uint16 _textColor = 0;
	Common::Point _position;
	Common::String _textKey;
	int16 _tableIndex = 0;
};

// Nancy14 AR 53. A rollover label: an image that is only drawn while the mouse
// is inside its hotspot. Entering the hotspot plays a sound and sets an event
// flag, and clicking it plays a second sound before changing the scene.
class RolloverOverlay : public RenderActionRecord {
public:
	RolloverOverlay() : RenderActionRecord(7) {}
	virtual ~RolloverOverlay() { _fullSurface.free(); }

	void init() override;
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }
	bool canHaveHotspot() const override { return true; }
	CursorManager::CursorType getHoverCursor() const override { return (CursorManager::CursorType)_hoverCursor; }
	bool cursorSetFromScript() const override { return true; }
	Common::String getRecordExtraInfo() const override {
		return Common::String::format("Scene %d, file %s", _sceneChange.sceneID, _imageName.baseName().c_str());
	}

protected:
	Common::String getRecordTypeName() const override { return "RolloverOverlay"; }

	void playSoundBlock(const RandomSoundBlock &block);

	Common::Path _imageName;
	uint16 _transparency = kPlayOverlayPlain;
	uint16 _hoverCursor = 0;
	Common::Rect _hotspotRect;
	Common::Rect _srcRect;
	Common::Rect _destRect;
	// Set every time the mouse enters the hotspot
	FlagDescription _flagOnHover;
	// When nonzero the hover sound is only played the first time; otherwise it
	// plays on every hover
	uint16 _hoverSoundOnce = 0;
	RandomSoundBlock _hoverSound;
	SceneChangeDescription _sceneChange;
	RandomSoundBlock _clickSound;

	bool _isHovered = false;
	bool _hoverSoundPlayed = false;
	bool _clickSoundStarted = false;

	Graphics::ManagedSurface _fullSurface;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_OVERLAY_H
