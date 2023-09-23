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
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"

#include "engines/nancy/action/overlay.h"

#include "engines/nancy/state/scene.h"

#include "common/serializer.h"

namespace Nancy {
namespace Action {

void Overlay::init() {
	// Check for special autotext strings, and use the requested surface as source
	if (_imageName.hasPrefix("USE_AUTOTEXT")) {
		uint surfID = _imageName[12] - '1';
		Graphics::ManagedSurface &surf = g_nancy->_graphicsManager->getAutotextSurface(surfID);
		_fullSurface.create(surf, surf.getBounds());
	} else if (_imageName.hasPrefix("USE_AUTOJOURNAL")) {
		uint surfID = _imageName.substr(15).asUint64() - 3;
		Graphics::ManagedSurface &surf = g_nancy->_graphicsManager->getAutotextSurface(surfID);
		_fullSurface.create(surf, surf.getBounds());
	} else {
		// No autotext, load image source
		g_nancy->_resource->loadImage(_imageName, _fullSurface);
	}

	setFrame(_firstFrame);

	RenderObject::init();
}

void Overlay::handleInput(NancyInput &input) {
	// For no apparent reason, the original engine handles Overlay input as a special case,
	// rather than simply set the general hotspot inside the ActionRecord struct. Special cases
	// (a.k.a puzzle types) get handled before regular ActionRecords, which means an Overlay
	// must take precedence when handling the mouse. Thus, out ActionManager class first iterates
	// through all records and calls their handleInput() function just to make sure this special
	// case is handled. This fixes nancy3 scene 7081.
	if (_enableHotspot == kPlayOverlayWithHotspot) {
		if (NancySceneState.getViewport().convertViewportToScreen(_hotspot).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_state = kActionTrigger;
				input.eatMouseInput(); // Make sure nothing else gets triggered
			}
		}
	}
}

void Overlay::readData(Common::SeekableReadStream &stream) {
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	uint16 numSrcRects = 0;

	readFilename(ser, _imageName);
	ser.skip(2); // VIDEO_STOP_RENDERING or VIDEO_CONTINUE_RENDERING
	ser.syncAsUint16LE(_transparency);
	ser.syncAsUint16LE(_hasSceneChange);
	ser.syncAsUint16LE(_enableHotspot, kGameTypeNancy2, kGameTypeNancy2);
	ser.syncAsUint16LE(_z, kGameTypeNancy2);
	ser.syncAsUint16LE(_overlayType, kGameTypeNancy2);
	ser.syncAsUint16LE(numSrcRects, kGameTypeNancy2);

	ser.syncAsUint16LE(_playDirection);
	ser.syncAsUint16LE(_loop);
	ser.syncAsUint16LE(_firstFrame);
	ser.syncAsUint16LE(_loopFirstFrame);
	ser.syncAsUint16LE(_loopLastFrame);
	uint16 framesPerSec = stream.readUint16LE();

	// Avoid divide by 0
	if (framesPerSec) {
		_frameTime = Common::Rational(1000, framesPerSec).toInt();
	}

	ser.syncAsUint16LE(_z, kGameTypeNancy1, kGameTypeNancy1);

	if (ser.getVersion() > kGameTypeNancy1) {
		_isInterruptible = true;
		
		if (ser.getVersion() > kGameTypeNancy2) {
			if (_overlayType == kPlayOverlayStatic) {
				_enableHotspot = (_hasSceneChange == kPlayOverlaySceneChange) ? kPlayOverlayWithHotspot : kPlayOverlayNoHotspot;
			}
		}
	}

	if (_isInterruptible) {
			ser.syncAsSint16LE(_interruptCondition.label);
			ser.syncAsUint16LE(_interruptCondition.flag);
		} else {
			_interruptCondition.label = kEvNoEvent;
			_interruptCondition.flag = g_nancy->_false;
		}

	_sceneChange.readData(stream);
	_flagsOnTrigger.readData(stream);
	_sound.readNormal(stream);

	uint numViewportFrames = stream.readUint16LE();

	if (_overlayType == kPlayOverlayAnimated) {
		numSrcRects = _loopLastFrame - _firstFrame + 1;
	}

	readRectArray(ser, _srcRects, numSrcRects);

	_blitDescriptions.resize(numViewportFrames);
	for (auto &bm : _blitDescriptions) {
		bm.readData(stream, ser.getVersion() >= kGameTypeNancy2);
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
			bool shouldTrigger = false;

			// Check for interrupt flag
			if (NancySceneState.getEventFlag(_interruptCondition)) {
				shouldTrigger = true;
			}

			// Wait until sound stops (if present)
			if (!g_nancy->_sound->isSoundPlaying(_sound)) {
				// Check if we're at the last frame
				if ((_currentFrame == _loopLastFrame) && (_playDirection == kPlayOverlayForward) && (_loop == kPlayOverlayOnce)) {
					shouldTrigger = true;
				} else if ((_currentFrame == _loopFirstFrame) && (_playDirection == kPlayOverlayReverse) && (_loop == kPlayOverlayOnce)) {
					shouldTrigger = true;
				}
			}

			if (shouldTrigger) {
				_state = kActionTrigger;
			} else {
				// Check if we've moved the viewport
				uint16 newFrame = NancySceneState.getSceneInfo().frameID;

				if (_currentViewportFrame != newFrame) {
					_currentViewportFrame = newFrame;

					setVisible(false);
					_hasHotspot = false;

					for (uint i = 0; i < _blitDescriptions.size(); ++i) {
						if (_currentViewportFrame == _blitDescriptions[i].frameID) {
							moveTo(_blitDescriptions[i].dest);
							setVisible(true);

							if (_enableHotspot == kPlayOverlayWithHotspot) {
								_hotspot = _screenPosition;
							}

							break;
						}
					}
				}

				if (_nextFrameTime == 0) {
					_nextFrameTime = _currentFrameTime + _frameTime;
				} else {
					_nextFrameTime += _frameTime;
				}

				uint16 nextFrame = _currentFrame;

				if (_playDirection == kPlayOverlayReverse) {
					if (nextFrame - 1 < _loopFirstFrame) {
						// We keep looping if sound is present (nancy1 only)
						if (_loop == kPlayOverlayLoop || (_sound.name != "NO SOUND" && g_nancy->getGameType() == kGameTypeNancy1)) {
							nextFrame = _loopLastFrame;
						}
					} else {
						--nextFrame;
					}
				} else {
					if (nextFrame + 1 > _loopLastFrame) {
						if (_loop == kPlayOverlayLoop || (_sound.name != "NO SOUND" && g_nancy->getGameType() == kGameTypeNancy1)) {
							nextFrame = _loopFirstFrame;
						}
					} else {
						++nextFrame;
					}
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

				for (uint i = 0; i < _blitDescriptions.size(); ++i) {
					if (_currentViewportFrame == _blitDescriptions[i].frameID) {
						moveTo(_blitDescriptions[i].dest);
						setVisible(true);

						// In static mode every "animation" frame corresponds to a viewport frame
						if (_overlayType == kPlayOverlayStatic) {
							setFrame(i);

							if (_enableHotspot == kPlayOverlayWithHotspot) {
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
		setVisible(false);
		g_nancy->_sound->stopSound(_sound);

		_flagsOnTrigger.execute();
		if (_hasSceneChange == kPlayOverlaySceneChange) {
			NancySceneState.changeScene(_sceneChange);
		}
		
		finishExecution();

		break;
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

	Common::Rect srcRect;

	if (_overlayType == kPlayOverlayAnimated) {
		// Workaround for:
		// - the arcade machine in nancy1 scene 833
		// - the fireplace in nancy2 scene 2491, where one of the rects is invalid.
		// - the ball thing in nancy2 scene 1562, where one of the rects is twice as tall as it should be
		// Assumes all rects in a single animation have the same dimensions
		srcRect = _srcRects[frame];
		if (!srcRect.isValidRect() || srcRect.width() != _srcRects[0].width() || srcRect.height() != _srcRects[0].height()) {
			srcRect.setWidth(_srcRects[0].width());
			srcRect.setHeight(_srcRects[0].height());
		}
	} else {
		if (_currentViewportFrame == -1) {
			return;
		}

		// Static mode overlays are an absolute mess, and use both the general source rects (_srcRects),
		// and the ones inside the blit description struct corresponding to the current scene background.

		if (_srcRects.size() > 1) {
			// First, the order of the blit descriptions does not necessarily correspond to the order of
			// the general rects, as the general rects are ordered based on the scene's background frame id,
			// while the blit descriptions can be in arbitrary order. 
			// An example is nancy4 scene 3500, where the overlay appears on background frames 0, 1, 2, 18 and 19,
			// while the blit descriptions are in order 18, 19, 0, 1, 2. Thus, if we don't do the counting below
			// we get wildly inaccurate results.
			uint srcID = 0;

			for (int i = 0; i < _currentViewportFrame; ++i) {
				for (uint j = 0; j < _blitDescriptions.size(); ++j) {
					if (_blitDescriptions[j].frameID == i) {
						++srcID;
						continue;
					}
				}
			}

			srcRect = _srcRects[srcID];
		} else {
			// Second, the number of general source rects may also be just one, in which case it's valid
			// for every blit description (nancy4 scene 1300)
			srcRect = _srcRects[0];
		}

		// Lastly, the general source rect we just got may also be completely empty (nancy5 scenes 2056, 2057),
		// or have coordinates other than (0, 0) (nancy3 scene 3070, nancy5 scene 2000). Presumably,
		// the general source rect was used for blitting to an (optional) intermediate surface, while the ones
		// inside the blit description below were used for blitting from that intermediate surface to the screen.
		// We can achieve the same results by doung the calculations below
		Common::Rect staticSrc = _blitDescriptions[frame].src;
		srcRect.translate(staticSrc.left, staticSrc.top);
		srcRect.setWidth(staticSrc.width());
		srcRect.setHeight(staticSrc.height());
	}

	_drawSurface.create(_fullSurface, srcRect);
	setTransparent(_transparency == kPlayOverlayTransparent);

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
