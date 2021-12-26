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

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/staticbitmapanim.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void PlayStaticBitmapAnimation::init() {
	g_nancy->_resource->loadImage(_imageName, _fullSurface);

	setFrame(0);

	RenderObject::init();
}

void PlayStaticBitmapAnimation::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	stream.skip(0x2);
	_isTransparent = (NancyFlag)(stream.readUint16LE());
	_doNotChangeScene = (NancyFlag)(stream.readUint16LE());
	_isReverse = (NancyFlag)(stream.readUint16LE());
	_isLooping = (NancyFlag)(stream.readUint16LE());
	_firstFrame = stream.readUint16LE();
	_loopFirstFrame = stream.readUint16LE();
	_loopLastFrame = stream.readUint16LE();
	_frameTime = Common::Rational(1000, stream.readUint16LE()).toInt();
	_z = stream.readUint16LE();

	if (_isInterruptible) {
		_interruptCondition.label = stream.readSint16LE();
		_interruptCondition.flag = (NancyFlag)stream.readUint16LE();
	} else {
		_interruptCondition.label = -1;
		_interruptCondition.flag = kFalse;
	}

	_sceneChange.readData(stream);
	_triggerFlags.readData(stream);
	_sound.read(stream, SoundDescription::kNormal);
	uint numViewportFrames = stream.readUint16LE();

	_srcRects.reserve(_loopLastFrame - _firstFrame);
	for (uint i = _firstFrame; i <= _loopLastFrame; ++i) {
		_srcRects.push_back(Common::Rect());
		readRect(stream, _srcRects[i]);
	}

	_bitmaps.reserve(numViewportFrames);
	for (uint i = 0; i < numViewportFrames; ++i) {
		_bitmaps.push_back(BitmapDescription());
		BitmapDescription &rects = _bitmaps.back();
		rects.frameID = stream.readUint16LE();
		readRect(stream, rects.src);
		readRect(stream, rects.dest);
	}
}

void PlayStaticBitmapAnimation::execute() {
	uint32 _currentFrameTime = g_nancy->getTotalPlayTime();
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_sound);
		g_nancy->_sound->playSound(_sound);
		_state = kRun;
		// fall through
	case kRun: {
		// Check the timer to see if we need to draw the next animation frame
		if (_nextFrameTime <= _currentFrameTime) {
			// World's worst if statement
			if (NancySceneState.getEventFlag(_interruptCondition) ||
				(   (((_currentFrame == _loopLastFrame) && (_isReverse == kFalse) && (_isLooping == kFalse)) ||
					((_currentFrame == _loopFirstFrame) && (_isReverse == kTrue) && (_isLooping == kFalse))) &&
						!g_nancy->_sound->isSoundPlaying(_sound))   ) {

				_state = kActionTrigger;

				// Not sure if hiding when triggered is a hack or the intended behavior, but it's here to fix
				// nancy1's safe lock light not turning off.
				setVisible(false);

				if (!g_nancy->_sound->isSoundPlaying(_sound)) {
					g_nancy->_sound->stopSound(_sound);
				}
			} else {
				// Check if we've moved the viewport
				uint16 newFrame = NancySceneState.getSceneInfo().frameID;

				if (_currentViewportFrame != newFrame) {
					_currentViewportFrame = newFrame;

					for (uint i = 0; i < _bitmaps.size(); ++i) {
						if (_currentViewportFrame == _bitmaps[i].frameID) {
							_screenPosition = _bitmaps[i].dest;
							break;
						}
					}
				}

				_nextFrameTime = _currentFrameTime + _frameTime;
				setFrame(_currentFrame);

				if (_isReverse == kTrue) {
					--_currentFrame;
					_currentFrame = _currentFrame < _loopFirstFrame ? _loopLastFrame : _currentFrame;
					return;
				} else {
					++_currentFrame;
					_currentFrame = _currentFrame > _loopLastFrame ? _loopFirstFrame : _currentFrame;
					return;
				}
			}
		} else {
			// Check if we've moved the viewport
			uint16 newFrame = NancySceneState.getSceneInfo().frameID;

			if (_currentViewportFrame != newFrame) {
				_currentViewportFrame = newFrame;

				for (uint i = 0; i < _bitmaps.size(); ++i) {
					if (_currentViewportFrame == _bitmaps[i].frameID) {
						_screenPosition = _bitmaps[i].dest;
						break;
					}
				}
			}
		}

		break;
	}
	case kActionTrigger:
		_triggerFlags.execute();
		if (_doNotChangeScene == kFalse) {
			NancySceneState.changeScene(_sceneChange);
			finishExecution();
		}
		break;
	}
}

void PlayStaticBitmapAnimation::onPause(bool pause) {
	if (!pause) {
		registerGraphics();
	}
}

void PlayStaticBitmapAnimation::setFrame(uint frame) {
	_currentFrame = frame;
	_drawSurface.create(_fullSurface, _srcRects[frame]);

	setTransparent(_isTransparent == kTrue);

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
