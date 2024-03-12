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
	// Autotext overlays need special handling when blitting
	if (_imageName.baseName().hasPrefix("USE_")) {
		_usesAutotext = true;
	}

	g_nancy->_resource->loadImage(_imageName, _fullSurface);

	_currentFrame = _firstFrame;

	RenderObject::init();
}

void Overlay::handleInput(NancyInput &input) {
	// For no apparent reason, from nancy3 on the original engine handles Overlay input as a special case,
	// rather than simply set the general hotspot inside the ActionRecord struct. Special cases
	// (a.k.a puzzle types) get handled before regular ActionRecords, which means an Overlay
	// must take precedence when handling the mouse. Thus, out ActionManager class first iterates
	// through all records and calls their handleInput() function just to make sure this special
	// case is handled. This fixes nancy3 scene 7081.
	if (g_nancy->getGameType() >= kGameTypeNancy3) {
		if (_hasHotspot) {
			if (NancySceneState.getViewport().convertViewportToScreen(_hotspot).contains(input.mousePos)) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_state = kActionTrigger;

					// Make sure nothing else gets triggered
					// This is nancy3 and up, since we actually want to trigger other records in nancy2 (e.g. scene 2541)
					input.eatMouseInput();
				}
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

	if (ser.getVersion() > kGameTypeNancy2) {
		if (_overlayType == kPlayOverlayStatic) {
			_enableHotspot = (_hasSceneChange == kPlayOverlaySceneChange) ? kPlayOverlayWithHotspot : kPlayOverlayNoHotspot;
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
								_hasHotspot = true;
							}

							break;
						}
					}
				}

				uint16 frameDiff = 1;
				uint16 nextFrame = _currentFrame;

				if (_nextFrameTime == 0) {
					_nextFrameTime = _currentFrameTime + _frameTime;
				} else {
					uint32 timeDiff = _currentFrameTime - _nextFrameTime;
					frameDiff = timeDiff / MAX<uint32>(_frameTime, 1); // Fix for nancy2 scene 1090, where _frameTime is 0
					_nextFrameTime += _frameTime * frameDiff;
				}

				if (_playDirection == kPlayOverlayReverse) {
					if (nextFrame - frameDiff < _loopFirstFrame) {
						// We keep looping if sound is present (nancy1/2 only)
						if (_loop == kPlayOverlayLoop || (_sound.name != "NO SOUND" && g_nancy->getGameType() <= kGameTypeNancy2)) {
							nextFrame = _loopLastFrame - (frameDiff % (_loopLastFrame - _loopFirstFrame + 1));
						}
					} else {
						nextFrame -= frameDiff;
					}
				} else {
					if (nextFrame + frameDiff > _loopLastFrame) {
						if (_loop == kPlayOverlayLoop || (_sound.name != "NO SOUND" && g_nancy->getGameType() <= kGameTypeNancy2)) {
							nextFrame = _loopFirstFrame + (frameDiff % (_loopLastFrame - _loopFirstFrame + 1));
						}
					} else {
						nextFrame += frameDiff;
					}
				}

				// Workaround for:
				// - the arcade machine in nancy1 scene 833
				// - the fireplace in nancy2 scene 2491, where one of the rects is invalid.
				// - the ball thing in nancy2 scene 1562, where one of the rects is twice as tall as it should be
				// Assumes all rects in a single animation have the same dimensions
				Common::Rect srcRect = _srcRects[nextFrame];
				if (!srcRect.isValidRect() || srcRect.width() != _srcRects[0].width() || srcRect.height() != _srcRects[0].height()) {
					srcRect.setWidth(_srcRects[0].width());
					srcRect.setHeight(_srcRects[0].height());
				}

				_drawSurface.create(_fullSurface, srcRect);
				setTransparent(_transparency == kPlayOverlayTransparent);

				_currentFrame = nextFrame;
				_needsRedraw = true;
			}
		} else {
			// Check if we've moved the viewport
			uint16 newFrame = NancySceneState.getSceneInfo().frameID;

			if (_currentViewportFrame != newFrame) {
				_currentViewportFrame = newFrame;

				setVisible(false);
				_hasHotspot = false;

				// First, check if there's more than one blit description for the current viewport frame.
				// This happens in nancy7 scene 3600
				Common::Array<uint16> blitsForThisFrame;
				Common::Rect destRect;
				for (uint i = 0; i < _blitDescriptions.size(); ++i) {
					if (_currentViewportFrame == _blitDescriptions[i].frameID) {
						blitsForThisFrame.push_back(i);
						if (destRect.isEmpty()) {
							destRect = _blitDescriptions[i].dest;
						} else {
							destRect.extend(_blitDescriptions[i].dest);
						}
					}
				}

				if (_overlayType == kPlayOverlayStatic && blitsForThisFrame.size()) {
					moveTo(destRect);
					setVisible(true);

					if (blitsForThisFrame.size() != 1) {
						_drawSurface.create(destRect.width(), destRect.height(), _fullSurface.format);
						setTransparent(true); // Force transparency. This shouldn't break anything. Hopefully.
						_drawSurface.clear(_drawSurface.getTransparentColor());
					}

					for (uint i = 0; i < blitsForThisFrame.size(); ++i) {
						// In static mode every "animation" frame corresponds to a viewport frame
						// Static mode overlays use both the general source rects (_srcRects),
						// and the ones inside the blit description struct corresponding to the current scene background.

						// BlitDescriptions contain the id of the source rect to actually use
						Common::Rect srcRect = _srcRects[_blitDescriptions[blitsForThisFrame[i]].staticRectID];
						Common::Rect staticBounds = _blitDescriptions[blitsForThisFrame[i]].src;

						if (_usesAutotext) {
							// For autotext overlays, the srcRect is junk data
							srcRect = staticBounds;
						} else {
							// Lastly, the general source rect we just got may also be completely empty (nancy5 scenes 2056, 2057),
							// or have coordinates other than (0, 0) (nancy3 scene 3070, nancy5 scene 2000). Presumably,
							// the general source rect was used for blitting to an (optional) intermediate surface, while the ones
							// inside the blit description below were used for blitting from that intermediate surface to the screen.
							// We can achieve the same results by doung the calculations below
							srcRect.translate(staticBounds.left, staticBounds.top);

							if (srcRect.isEmpty()) {
								srcRect.setWidth(staticBounds.width());
								srcRect.setHeight(staticBounds.height());
							} else {
								// Grab whichever dimensions are smaller. Fixes the book in nancy5 scene 3000
								srcRect.setWidth(MIN<int>(staticBounds.width(), srcRect.width()));
								srcRect.setHeight(MIN<int>(staticBounds.height(), srcRect.height()));
							}
						}

						// Make sure the srcRect doesn't extend beyond the image.
						// This fixes nancy7 scene 4228
						srcRect.clip(_fullSurface.getBounds());

						if (blitsForThisFrame.size() == 1) {
							_drawSurface.create(_fullSurface, srcRect);
							setTransparent(_transparency == kPlayOverlayTransparent);
						} else {
							Common::Rect d = _blitDescriptions[blitsForThisFrame[i]].dest;
							d.translate(-destRect.left, -destRect.top);
							_drawSurface.blitFrom(_fullSurface, srcRect, d);
						}

						_needsRedraw = true;

						if (g_nancy->getGameType() <= kGameTypeNancy2) {
							// In nancy2, the presence of a hotspot relies on whether the Overlay has a scene change
							if (_enableHotspot == kPlayOverlayWithHotspot) {
								_hotspot = _screenPosition;
								_hasHotspot = true;
							}
						} else {
							// nancy3 added a per-frame flag for hotspots. This allows the overlay to be clickable
							// even without a scene change (useful for setting flags).
							if (_blitDescriptions[i].hasHotspot == kPlayOverlayWithHotspot) {
								_hotspot = _screenPosition;
								_hasHotspot = true;
							}
						}
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

void OverlayStaticTerse::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	_transparency = stream.readUint16LE();
	_z = stream.readUint16LE();

	Common::Rect dest, src;
	readRect(stream, dest);
	readRect(stream, src);

	_srcRects.push_back(src);
	_blitDescriptions.resize(1);
	_blitDescriptions[0].src = Common::Rect(src.width(), src.height());
	_blitDescriptions[0].dest = dest;

	_overlayType = kPlayOverlayStatic;
}

void OverlayAnimTerse::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	stream.skip(2); // VIDEO_STOP_RENDERING, VIDEO_CONTINUE_RENDERING
	_transparency = stream.readUint16LE();
	_hasSceneChange = stream.readUint16LE();
	_z = stream.readUint16LE();
	_playDirection = stream.readUint16LE();
	_loop = stream.readUint16LE();

	_sceneChange.sceneID = stream.readUint16LE();
	_sceneChange.continueSceneSound = kContinueSceneSound;
	_sceneChange.listenerFrontVector.set(0, 0, 1);
	_flagsOnTrigger.descs[0].label = stream.readSint16LE();
	_flagsOnTrigger.descs[0].flag = stream.readUint16LE();

	_firstFrame = _loopFirstFrame = stream.readUint16LE();
	_loopLastFrame = stream.readUint16LE();

	_blitDescriptions.resize(1);
	readRect(stream, _blitDescriptions[0].dest);

	readRectArray(stream, _srcRects, _loopLastFrame - _loopFirstFrame + 1);

	_overlayType = kPlayOverlayAnimated;
	_frameTime = Common::Rational(1000, 15).toInt(); // Always set to 15 fps
}

void TableIndexOverlay::readData(Common::SeekableReadStream &stream) {
	_tableIndex = stream.readUint16LE();
	Overlay::readData(stream);
}

void TableIndexOverlay::execute() {
	if (_state == kBegin) {
		Overlay::execute();
	}

	TableData *playerTable = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	assert(playerTable);
	auto *tabl = GetEngineData(TABL);
	assert(tabl);

	if (_lastIndexVal != playerTable->singleValues[_tableIndex - 1]) {
		_lastIndexVal = playerTable->singleValues[_tableIndex - 1];
		_srcRects.clear();
		_srcRects.push_back(tabl->srcRects[_lastIndexVal - 1]);
		_currentViewportFrame = -1; // Force redraw
	}

	if (_state != kBegin) {
		Overlay::execute();
	}
}

} // End of namespace Action
} // End of namespace Nancy
