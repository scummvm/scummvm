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

#ifndef NANCY_ACTION_STATICBITMAPANIM_H
#define NANCY_ACTION_STATICBITMAPANIM_H

#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// ActionRecord subclass describing a short "flipbook" animation from a single bitmap
// Also supports sound and getting interrupted by an event flag.
// This class covers both the PlayStaticBitmapAnimation and PlayIntStaticBitmapAnimation
// action record types, whose functionality is nearly identical
class PlayStaticBitmapAnimation : public ActionRecord, public RenderObject {
public:
	static const byte kPlayAnimationPlain			= 1;
	static const byte kPlayAnimationTransparent		= 2;

	static const byte kPlayAnimationSceneChange		= 1;
	static const byte kPlayAnimationNoSceneChange 	= 2;

	static const byte kPlayAnimationOnce			= 1;
	static const byte kPlayAnimationLoop			= 2;

	static const byte kPlayAnimationForward			= 1;
	static const byte kPlayAnimationReverse			= 2;

	PlayStaticBitmapAnimation(bool interruptible) : RenderObject(7), _isInterruptible(interruptible) {}
	virtual ~PlayStaticBitmapAnimation() { _fullSurface.free(); }

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void onPause(bool pause) override;

	Common::String _imageName;

	uint16 _transparency = kPlayAnimationPlain; // 0xC
	uint16 _animationSceneChange = kPlayAnimationSceneChange; // 0xE
	uint16 _playDirection = kPlayAnimationForward; // 0x10
	uint16 _loop = kPlayAnimationOnce; // 0x12
	uint16 _firstFrame = 0; // 0x14
	uint16 _loopFirstFrame = 0; // 0x16
	uint16 _loopLastFrame = 0; // 0x18
	Time _frameTime;
	FlagDescription _interruptCondition; // 0x1E
	SceneChangeDescription _sceneChange;
	MultiEventFlagDescription _triggerFlags; // 0x2A

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
	Common::String getRecordTypeName() const override { return _isInterruptible ? "PlayIntStaticBitmapAnimation" : "PlayStaticBitmapAnimation"; }
	bool isViewportRelative() const override { return true; }

	void setFrame(uint frame);

	Graphics::ManagedSurface _fullSurface;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_STATICBITMAPANIM_H
