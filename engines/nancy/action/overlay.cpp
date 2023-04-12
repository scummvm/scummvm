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

#include "engines/nancy/action/overlay.h"

#include "engines/nancy/state/scene.h"

#include "common/serializer.h"

namespace Nancy {
namespace Action {

void Overlay::init() {
	g_nancy->_resource->loadImage(_imageName, _fullSurface);

	setFrame(_firstFrame);

	RenderObject::init();
}

void Overlay::readData(Common::SeekableReadStream &stream) {
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	uint16 numSrcRects;

	readFilename(ser, _imageName);
	ser.skip(2);
	ser.syncAsUint16LE(_transparency);
	ser.syncAsUint16LE(_hasSceneChange);

	ser.syncAsUint16LE(_enableHotspot, kGameTypeNancy2);
	ser.syncAsUint16LE(_z, kGameTypeNancy2);
	ser.syncAsUint16LE(_overlayType, kGameTypeNancy2);
	ser.syncAsUint16LE(numSrcRects, kGameTypeNancy2);

	ser.syncAsUint16LE(_playDirection);
	ser.syncAsUint16LE(_loop);
	ser.syncAsUint16LE(_firstFrame);
	ser.syncAsUint16LE(_loopFirstFrame);
	ser.syncAsUint16LE(_loopLastFrame);
	_frameTime = Common::Rational(1000, stream.readUint16LE()).toInt();
	ser.syncAsUint16LE(_z, kGameTypeNancy1, kGameTypeNancy1);

	if (ser.getVersion() > kGameTypeNancy1) {
		_isInterruptible = true;
	}

	if (_isInterruptible) {
			ser.syncAsSint16LE(_interruptCondition.label);
			ser.syncAsUint16LE(_interruptCondition.flag);
		} else {
			_interruptCondition.label = kEvNoEvent;
			_interruptCondition.flag = kEvNotOccurred;
		}

	_sceneChange.readData(stream);
	_flagsOnTrigger.readData(stream);
	_sound.read(stream, SoundDescription::kNormal);
	uint numViewportFrames = stream.readUint16LE();

	if (_overlayType == kPlayOverlayAnimated) {
		numSrcRects = _loopLastFrame - _firstFrame + 1;
	}

	readRectArray(ser, _srcRects, numSrcRects);

	_bitmaps.resize(numViewportFrames);
	for (auto &bm : _bitmaps) {
		bm.readData(stream);
	}
}

void Overlay::execute() {
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
		if (_overlayType == kPlayOverlayAnimated && _nextFrameTime <= _currentFrameTime) {
			// World's worst if statement
			if (NancySceneState.getEventFlag(_interruptCondition) ||
				(	(((_currentFrame == _loopLastFrame) && (_playDirection == kPlayOverlayForward) && (_loop == kPlayOverlayOnce)) ||
					((_currentFrame == _loopFirstFrame) && (_playDirection == kPlayOverlayReverse) && (_loop == kPlayOverlayOnce))) &&
						!g_nancy->_sound->isSoundPlaying(_sound))	) {

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

					setVisible(false);

					for (uint i = 0; i < _bitmaps.size(); ++i) {
						if (_currentViewportFrame == _bitmaps[i].frameID) {
							moveTo(_bitmaps[i].dest);
							setVisible(true);
							break;
						}
					}
				}

				_nextFrameTime = _currentFrameTime + _frameTime;

				uint16 nextFrame = _currentFrame;

				if (_playDirection == kPlayOverlayReverse) {
					--nextFrame;
					nextFrame = nextFrame < _loopFirstFrame ? _loopLastFrame : nextFrame;
				} else {
					++nextFrame;
					nextFrame = nextFrame > _loopLastFrame ? _loopFirstFrame : nextFrame;
				}

				setFrame(nextFrame);
			}
		} else {
			// Check if we've moved the viewport
			uint16 newFrame = NancySceneState.getSceneInfo().frameID;

			if (_currentViewportFrame != newFrame) {
				_currentViewportFrame = newFrame;

				setVisible(false);
				_hasHotspot = false;

				for (uint i = 0; i < _bitmaps.size(); ++i) {
					if (_currentViewportFrame == _bitmaps[i].frameID) {
						moveTo(_bitmaps[i].dest);
						setVisible(true);

						// In static mode every "animation" frame corresponds to a viewport frame
						if (_overlayType == kPlayOverlayStatic) {
							setFrame(i);

							if (_enableHotspot) {
								_hotspot = _screenPosition;
								_hasHotspot = true;
							}
						}

						break;
					}
				}
			}
		}

		break;
	}
	case kActionTrigger:
		_flagsOnTrigger.execute();
		if (_hasSceneChange == kPlayOverlaySceneChange) {
			NancySceneState.changeScene(_sceneChange);
			finishExecution();
		}
		break;
	}
}

void Overlay::onPause(bool pause) {
	if (!pause) {
		registerGraphics();
	}
}

Common::String Overlay::getRecordTypeName() const {
	if (g_nancy->getGameType() <= kGameTypeNancy1) {
		if (_isInterruptible) {
			return "PlayIntStaticBitmapAnimation";
		} else {
			return "PlayStaticBitmapAnimation";
		}
	} else {
		return "Overlay";
	}
}

void Overlay::setFrame(uint frame) {
	_currentFrame = frame;
	
	// Workaround for the fireplace in nancy2 scene 2491,
	// where one of the rects is invalid. Assumes all
	// rects in a single animation have the same dimensions
	Common::Rect srcRect = _srcRects[frame];
	if (!srcRect.isValidRect()) {
		srcRect.setWidth(_srcRects[0].width());
		srcRect.setHeight(_srcRects[0].height());
	}

	_drawSurface.create(_fullSurface, srcRect);

	setTransparent(_transparency == kPlayOverlayPlain);

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
