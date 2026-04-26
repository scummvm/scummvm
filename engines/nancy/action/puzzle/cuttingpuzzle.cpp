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
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/cuttingpuzzle.h"

namespace Nancy {
namespace Action {

void CuttingPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName1);      // +0x000 (33 bytes)
	readFilename(stream, _imageName2);      // +0x021 (33 bytes)

	_numGrooves = stream.readUint16LE();    // +0x042
	stream.skip(2);						  // +0x044 (unknown)
	stream.skip(2);                         // +0x046 (unknown)

	// Destination rects (viewport-local screen positions)
	readRect(stream, _noAnimDest);          // +0x048
	readRect(stream, _leverDest);           // +0x058
	readRect(stream, _switchDest);          // +0x068
	readRectArray(stream, _grooveDest, 8);  // +0x078..0x0f7
	readRectArray(stream, _markerDest, 8);  // +0x0f8..0x177
	readRectArray(stream, _bladeDest, 8);   // +0x178..0x1f7
	readRect(stream, _animFrameDest);       // +0x1f8

	// Source rects (from sprite sheet)
	readRect(stream, _noAnimSrc);                    // +0x208
	readRectArray(stream, _leverSrc, 3);             // +0x218..0x247  depths 1..3
	readRect(stream, _switchOnSrc);                  // +0x248
	readRectArray(stream, _grooveTypeSrc, 6);        // +0x258..0x2b7  types 0..5
	readRectArray(stream, _bladeSrc, 4);             // +0x2b8..0x2f7  depths 0..3
	readRect(stream, _baseSrc);                      // +0x2f8

	_numAnimFrames = stream.readUint16LE();          // +0x308
	readRectArray(stream, _animSrc, 12);             // +0x30a..0x3c9  (always 12 slots)
	_frameDelayMs = stream.readUint16LE();           // +0x3ca

	// Correct groove depths (target answer).  Always 8 slots in the data file.
	_correctGrooves.resize(8, 0);
	for (int i = 0; i < 8; ++i)
		_correctGrooves[i] = stream.readSint16LE();  // +0x3cc..0x3db

	_latheSound.readNormal(stream);                  // +0x3dc (49 bytes)
	_moveSound.readNormal(stream);                   // +0x40d (49 bytes)
	_startStopSound.readNormal(stream);              // +0x43e (49 bytes)
	_depthSound.readNormal(stream);                  // +0x46f (49 bytes)
	_cutSound.readNormal(stream);                    // +0x4a0 (49 bytes)

	_puzzleSolvedScene.readData(stream);                 // +0x4d1 (25 bytes)
	_doneSoundDelaySecs = stream.readUint16LE();     // +0x4ea

	_doneSound.readNormal(stream);                   // +0x4ec (49 bytes)

	_itemCheckByte = stream.readByte();              // +0x51d
	_itemID        = stream.readSint16LE();          // +0x51e

	// Solve scene: plain SceneChangeDescription (20 bytes) + 2-byte shouldStopRendering skip
	_missingGogglesScene.readData(stream);                    // +0x520 (20 bytes)
	stream.skip(2);                                  // +0x534 skip

	_cancelScene.readData(stream);                   // +0x536 (25 bytes)
}

void CuttingPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(),
	                    g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName1, _image1);
	g_nancy->_resource->loadImage(_imageName2, _image2);
	_image1.setTransparentColor(_drawSurface.getTransparentColor());
	_image2.setTransparentColor(_drawSurface.getTransparentColor());

	_grooveDepths.resize(_numGrooves, 0);

	redrawSurface();
}

// Returns the groove-type sprite index (0..5) for groove slot i, based on depth and neighbors.
// Type 0: shallow (depth 1)
// Type 1: medium  (depth 2)
// Type 2: deep single   (depth 3, no deep neighbors)
// Type 3: deep right-edge (depth 3, deep neighbor on left)
// Type 4: deep left-edge  (depth 3, deep neighbor on right)
// Type 5: deep middle     (depth 3, deep neighbors on both sides)
int CuttingPuzzle::grooveTypeForIndex(int i) const {
	int depth = _grooveDepths[i];
	if (depth == 1)
		return 0;
	if (depth == 2)
		return 1;
	// depth == 3
	bool leftDeep  = (i > 0)                    && (_grooveDepths[i - 1] == 3);
	bool rightDeep = (i < (int)_numGrooves - 1) && (_grooveDepths[i + 1] == 3);
	if (i == 0)
		return rightDeep ? 4 : 2;
	if (i == (int)_numGrooves - 1)
		return leftDeep ? 3 : 2;
	if (leftDeep && rightDeep)
		return 5;
	if (leftDeep)
		return 3;
	if (rightDeep)
		return 4;
	return 2;
}

void CuttingPuzzle::redrawSurface() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	// image1 = main sprite sheet (grooves, lever, blade, base, switch, no-anim overlay)
	// image2 = animation frames only

	if (!_latheRunning || _subState != kIdle) {
		// Static overlay (lathe off)
		_drawSurface.blitFrom(_image1, _noAnimSrc, _noAnimDest);
		_animRestore = false;
	} else {
		// Lathe running: show the on-switch sprite and the current animation frame.
		_drawSurface.blitFrom(_image1, _switchOnSrc, _switchDest);
		if (_numAnimFrames > 0 && _animFrame < _animSrc.size())
			_drawSurface.blitFrom(_image2, _animSrc[_animFrame], _animFrameDest);
	}

	// Draw the blade/marker base at the current position (opaque).
	if (_currentMarkerPos < _markerDest.size())
		_drawSurface.blitFrom(_image1, _baseSrc, _markerDest[_currentMarkerPos]);

	// Draw the lever sprite (if depth > 0).
	if (_currentLeverDepth > 0 && _currentLeverDepth <= (uint16)_leverSrc.size())
		_drawSurface.blitFrom(_image1, _leverSrc[_currentLeverDepth - 1], _leverDest);

	// Draw each groove that has been cut.
	for (uint i = 0; i < _numGrooves; ++i) {
		if (_grooveDepths[i] > 0) {
			int type = grooveTypeForIndex(i);
			if (type < (int)_grooveTypeSrc.size() && i < _grooveDest.size())
				_drawSurface.blitFrom(_image1, _grooveTypeSrc[type], _grooveDest[i]);
		}
	}

	// Draw the blade assembly at the current position (opaque).
	if (_currentMarkerPos < _bladeDest.size() && _currentLeverDepth < _bladeSrc.size())
		_drawSurface.blitFrom(_image1, _bladeSrc[_currentLeverDepth], _bladeDest[_currentMarkerPos]);

	_needsRedraw = true;
}

void CuttingPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		g_nancy->_sound->loadSound(_latheSound);
		g_nancy->_sound->loadSound(_moveSound);
		g_nancy->_sound->loadSound(_startStopSound);
		g_nancy->_sound->loadSound(_depthSound);
		g_nancy->_sound->loadSound(_cutSound);

		_state = kRun;
		break;

	case kRun: {
		switch (_subState) {
		case kIdle:
			if (!_latheRunning) {
				// Check if the current groove configuration is the correct answer.
				bool allMatch = true;
				for (uint i = 0; i < _numGrooves; ++i) {
					if (_grooveDepths[i] != _correctGrooves[i]) {
						allMatch = false;
						break;
					}
				}
				if (allMatch) {
					_solved = true;
					_timerDeadline = g_system->getMillis() + (uint32)_doneSoundDelaySecs * 1000;
					_subState = kWaitTimer;
					break;
				}
			} else {
				// Advance animation frame.
				if (_animFrame + 1 < _numAnimFrames) {
					++_animFrame;
					redrawSurface();
				} else {
					// Completed one full animation loop → one macro-cycle.
					_animFrame = 0;

					// Play the lathe loop sound if it's not already playing.
					if (!g_nancy->_sound->isSoundPlaying(_latheSound))
						g_nancy->_sound->playSound(_latheSound);

					if (_macroCycleCount == 14) {
						// The lathe run is complete: record the groove depth.
						_latheRunning = false;
						_animRestore  = true;

						// The groove gets the maximum of the previous depth and the
						// current lever setting.
						if (_currentMarkerPos < _numGrooves) {
							if (_grooveDepths[_currentMarkerPos] < (int16)_currentLeverDepth)
								_grooveDepths[_currentMarkerPos] = (int16)_currentLeverDepth;
						}

						// Check whether the item requirement is satisfied.
						if (_currentMarkerPos < _numGrooves && _grooveDepths[_currentMarkerPos] >= 1) {
							if (_itemCheckByte != 0) {
								_gogglesMissing = NancySceneState.hasItem(_itemID) == g_nancy->_false;
							}
						}

						if (!_gogglesMissing) {
							// Lever snaps back to 0 after the run.
							_currentLeverDepth = 0;
							_leverReset        = true;
							_macroCycleCount   = 0;
						}
					} else {
						// During cycles 2..11, play the cutting sound (if lever is down).
						// During cycles 12..13, stop the cutting sound.
						if (_macroCycleCount < 12) {
							if (_macroCycleCount > 1 && _currentLeverDepth > 0) {
								if (!g_nancy->_sound->isSoundPlaying(_cutSound))
									g_nancy->_sound->playSound(_cutSound);
							}
						} else {
							if (_currentLeverDepth > 0)
								g_nancy->_sound->stopSound(_cutSound);
						}
						++_macroCycleCount;
					}

					redrawSurface();
				}
			}

			// Set the frame-advance timer and move to kWaitTimer.
			_timerDeadline = g_system->getMillis() + _frameDelayMs;
			_subState = kWaitTimer;
			break;

		case kLatheFinished:
			if (_solved) {
				// Load and play the completion sound, then wait for it to finish.
				g_nancy->_sound->loadSound(_doneSound);
				g_nancy->_sound->playSound(_doneSound);
				_subState = kWaitDoneSound;
			} else {
				// Not solved: finish this AR (will process outcome in kActionTrigger).
				_state = kActionTrigger;
			}
			break;

		case kWaitTimer:
			if (g_system->getMillis() >= _timerDeadline) {
				if (_solved || _gogglesMissing) {
					_subState = kLatheFinished;
				} else {
					_subState = kIdle;
				}
			}
			break;

		case kWaitDoneSound:
			if (!g_nancy->_sound->isSoundPlaying(_doneSound)) {
				g_nancy->_sound->stopSound(_doneSound);
				_state = kActionTrigger;
			}
			break;
		}
		break;
	}

	case kActionTrigger:
		// Stop all sounds.
		g_nancy->_sound->stopSound(_latheSound);
		g_nancy->_sound->stopSound(_cutSound);
		g_nancy->_sound->stopSound(_moveSound);
		g_nancy->_sound->stopSound(_startStopSound);
		g_nancy->_sound->stopSound(_depthSound);

		if (_cancelled) {
			// Player explicitly cancelled: go to the cancel scene and possibly set
			// a flag if any grooves were cut.
			bool anyGroove = false;
			for (uint i = 0; i < _numGrooves; ++i) {
				if (_grooveDepths[i] != 0) {
					anyGroove = true;
					break;
				}
			}
			if (anyGroove)
				NancySceneState.setEventFlag(_cancelScene._flag);
			if (_cancelScene._sceneChange.sceneID != kNoScene)
				NancySceneState.changeScene(_cancelScene._sceneChange);
		} else if (_solved) {
			_puzzleSolvedScene.execute();
		} else if (_gogglesMissing) {
			NancySceneState.changeScene(_missingGogglesScene);
		}
		// If none of the above conditions apply, the AR finishes without a
		// scene change (lathe ran without producing the correct answer and no
		// item was needed).

		finishExecution();
		break;
	}
}

void CuttingPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _latheRunning)
		return;

	// Right-click cancels the puzzle (no dedicated exit hotspot in the data).
	if (input.input & NancyInput::kRightMouseButtonUp) {
		_cancelled = true;
		_state = kActionTrigger;
		return;
	}

	// Convert mouse position to viewport-local coordinates.
	Common::Point localMouse = input.mousePos;
	Common::Rect vpPos = NancySceneState.getViewport().getScreenPosition();
	localMouse -= Common::Point(vpPos.left, vpPos.top);

	// Lever: left half of the rect rotates the knob left (decrement depth),
	// right half rotates right (increment depth).
	if (_leverDest.contains(localMouse)) {
		int midX = (_leverDest.left + _leverDest.right) / 2;
		bool rotateLeft = localMouse.x < midX;

		g_nancy->_cursor->setCursorType(rotateLeft ? CursorManager::kRotateCCW
		                                            : CursorManager::kRotateCW);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (rotateLeft)
				_currentLeverDepth = (_currentLeverDepth == 0) ? 3 : _currentLeverDepth - 1;
			else
				_currentLeverDepth = (_currentLeverDepth + 1) % 4;
			g_nancy->_sound->playSound(_depthSound);
			redrawSurface();
		}
		return;
	}

	// Start/stop switch: start the lathe (re-clicking while stopped is a no-op
	// since the lathe auto-stops after 14 cycles).
	if (_switchDest.contains(localMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_latheRunning    = true;
			_macroCycleCount = 0;
			_animFrame       = 0;
			g_nancy->_sound->playSound(_startStopSound);
			redrawSurface();
		}
		return;
	}

	// Blade-position needle: clicking the left half moves the blade left,
	// clicking the right half moves it right.
	if (_currentMarkerPos < _markerDest.size() &&
	        _markerDest[_currentMarkerPos].contains(localMouse)) {
		int midX = (_markerDest[_currentMarkerPos].left + _markerDest[_currentMarkerPos].right) / 2;
		bool goLeft = localMouse.x < midX;

		if (goLeft && _currentMarkerPos > 0) {
			g_nancy->_cursor->setCursorType(CursorManager::kMoveLeft);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				--_currentMarkerPos;
				g_nancy->_sound->playSound(_moveSound);
				redrawSurface();
			}
		} else if (!goLeft && _currentMarkerPos + 1 < _numGrooves) {
			g_nancy->_cursor->setCursorType(CursorManager::kMoveRight);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				++_currentMarkerPos;
				g_nancy->_sound->playSound(_moveSound);
				redrawSurface();
			}
		}
		return;
	}
}

} // End of namespace Action
} // End of namespace Nancy
