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

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"
#include "engines/nancy/resource.h"

#include "engines/nancy/action/puzzle/peepholepuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void PeepholePuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_innerImageName, _innerImage);

	if (_buttonsImageName.empty()) {
		// Empty image name for buttons, use other image as source
		_buttonsImage.create(_innerImage, _innerImage.getBounds());
	} else {
		g_nancy->_resource->loadImage(_buttonsImageName, _buttonsImage);
	}

	_currentSrc = _startSrc;

	setTransparent(true);
	_drawSurface.clear(_drawSurface.getTransparentColor());
	setVisible(true);

	drawInner();
	checkButtons();
}

void PeepholePuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _innerImageName);
	readFilename(stream, _buttonsImageName);

	_transparency = stream.readUint16LE();

	readRect(stream, _innerBounds);
	readRect(stream, _startSrc);
	readRect(stream, _dest);

	readRectArray(stream, _buttonDests, 4);
	readRectArray(stream, _buttonSrcs, 4);
	readRectArray(stream, _buttonDisabledSrcs, 4);

	_pixelsToScroll = stream.readByte();

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void PeepholePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		break;
	case kActionTrigger:
		
		finishExecution();
	}
}

void PeepholePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	bool justReleased = false;

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(g_nancy->_cursorManager->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}
	}

	if (_pressedButton != -1) {
		if (input.input & NancyInput::kLeftMouseButtonHeld) {
			// Player is still holding the left button, check if mouse has moved outside bounds
			if (NancySceneState.getViewport().convertViewportToScreen(_buttonDests[_pressedButton]).contains(input.mousePos)) {
				if (!_disabledButtons[_pressedButton]) {
					// Do not show hover cursor on disabled button
					g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);
				}

				if (_pressStart == 0) {
					// Mouse was out of bounds but still held, and is now back in bounds, continue moving
					_pressStart = g_nancy->getTotalPlayTime();
				}
			} else {
				// Mouse is not in bounds, pause moving
				_pressStart = 0;
				justReleased = true;
			}
		} else {
			// Player released mouse button

			// Avoid single frame with non-highlighted cursor
			if (NancySceneState.getViewport().convertViewportToScreen(_buttonDests[_pressedButton]).contains(input.mousePos) && !_disabledButtons[_pressedButton]) {
				g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);
			}

			_pressedButton = -1;
			_pressStart = 0;
			justReleased = true;
		}
	} else {
		// Mouse is not currently pressing button, check all buttons
		for (uint i = 0; i < 4; ++i) {
			if (!_disabledButtons[i]) {
				if (NancySceneState.getViewport().convertViewportToScreen(_buttonDests[i]).contains(input.mousePos)) {
					g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

					if (input.input & NancyInput::kLeftMouseButtonDown) {
						if (_pressedButton == -1) {
							// Just pressed
							_pressedButton = i;
							_pressStart = g_nancy->getTotalPlayTime();
						}
					}
				}
			}
		}
	}

	// Perform movement
	if (_pressedButton != -1 && _pressStart != 0) {
		uint32 curTime = g_nancy->getTotalPlayTime();
		uint pixelsToMove = 0;
		if (curTime - _pressStart >= 1000 / _pixelsToScroll) {
			pixelsToMove = (curTime - _pressStart) / (1000 / _pixelsToScroll);
		}

		switch (_pressedButton) {
		case 0 :
			// Up
			_currentSrc.translate(0, -pixelsToMove);
			if (_currentSrc.top < _innerBounds.top) {
				_currentSrc.translate(0, _innerBounds.top - _currentSrc.top);
			}
			break;
		case 1 :
			// Down
			_currentSrc.translate(0, pixelsToMove);
			if (_currentSrc.bottom > _innerBounds.bottom) {
				_currentSrc.translate(0, _innerBounds.top - _currentSrc.top);
			}
			break;
		case 2 :
			// Left
			_currentSrc.translate(-pixelsToMove, 0);
			if (_currentSrc.left < _innerBounds.left) {
				_currentSrc.translate(_innerBounds.left - _currentSrc.left, 0);
			}
			break;
		case 3:
			// Right
			_currentSrc.translate(pixelsToMove, 0);
			if (_currentSrc.right > _innerBounds.right) {
				_currentSrc.translate(_innerBounds.right - _currentSrc.right, 0);
			}
			break;
		}

		_pressStart = curTime;

		checkButtons();
		drawInner();
	} else if (justReleased) {
		checkButtons();
	}
}

void PeepholePuzzle::drawInner() {
	_drawSurface.blitFrom(_innerImage, _currentSrc, _dest);
	_needsRedraw = true;
}

void PeepholePuzzle::checkButtons() {
	for (int i = 0; i < 4; ++i) {
		int16 *srcCoord = nullptr;
		int16 *innerCoord = nullptr;

		switch(i) {
		case 0 :
			srcCoord = &_currentSrc.top;
			innerCoord = &_innerBounds.top;
			break;
		case 1 :
			srcCoord = &_currentSrc.bottom;
			innerCoord = &_innerBounds.bottom;
			break;
		case 2 :
			srcCoord = &_currentSrc.left;
			innerCoord = &_innerBounds.left;
			break;
		case 3 :
			srcCoord = &_currentSrc.right;
			innerCoord = &_innerBounds.right;
			break;
		}

		if (!_buttonDests[i].isEmpty()) {
			if (*srcCoord == *innerCoord) {
				if (_disabledButtons[i] == false) {
					_disabledButtons[i] = true;
					if (!_buttonDisabledSrcs[i].isEmpty()) {
						_drawSurface.blitFrom(_buttonsImage, _buttonDisabledSrcs[i], _buttonDests[i]);
						_needsRedraw = true;
					}
				}
			} else {
				_disabledButtons[i] = false;
				if (i == _pressedButton && _pressStart) {
					_drawSurface.blitFrom(_buttonsImage, _buttonSrcs[i], _buttonDests[i]);
					_needsRedraw = true;
				} else {
					_drawSurface.fillRect(_buttonDests[i], _drawSurface.getTransparentColor());
					_needsRedraw = true;
				}
			}
		} else {
			_disabledButtons[i] = true;
		}
	}	
}

} // End of namespace Action
} // End of namespace Nancy
