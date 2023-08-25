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
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"

#include "engines/nancy/ui/scrollbar.h"

namespace Nancy {
namespace UI {

Scrollbar::Scrollbar(uint16 zOrder, const Common::Rect &srcBounds, const Common::Point &topPosition, uint16 scrollDistance, bool isVertical) :
    Scrollbar(zOrder, srcBounds, g_nancy->_graphicsManager->_object0, topPosition, scrollDistance, isVertical) {}

Scrollbar::Scrollbar(uint16 zOrder, const Common::Rect &srcBounds, Graphics::ManagedSurface &srcSurf, const Common::Point &topPosition, uint16 scrollDistance, bool isVertical) :
		RenderObject(zOrder),
		_isVertical(isVertical),
		_isClicked(false),
		_currentPosition(0),
		_maxDist(scrollDistance) {
	_drawSurface.create(srcSurf, srcBounds);

	_startPosition = topPosition;
	_startPosition.x -= srcBounds.width() / 2;

	_screenPosition = srcBounds;
	_screenPosition.moveTo(_startPosition);
}

void Scrollbar::init() {
	setTransparent(true);
	RenderObject::init();
}

void Scrollbar::handleInput(NancyInput &input) {
	// Note: the original engine's scrollbars only work if the cursor is inside
	// the hotspot (happens if we remove the _isClicked check below). This doesn't make
	// for great UX, however, so it has been fixed.
	if (_screenPosition.contains(input.mousePos) || _isClicked) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

		if (input.input & NancyInput::kLeftMouseButtonDown && !_isClicked) {
			// Begin click and hold
			_isClicked = true;
			_mousePosOnClick = input.mousePos - Common::Point(_screenPosition.left, _screenPosition.top);
		}

		if (input.input & NancyInput::kRightMouseButtonUp) {
			// Right click, reset position
			resetPosition();
		}

		if (_isClicked) {
			// Is currently clicked, handle movement
			Common::Point newMousePos = input.mousePos - Common::Point(_screenPosition.left, _screenPosition.top);

			if (newMousePos != _mousePosOnClick) {

				if (_isVertical) {
					uint16 minY = _startPosition.y;
					uint16 maxY = minY + _maxDist;
					uint16 newTop = CLIP<uint16>((_screenPosition.top + newMousePos.y - _mousePosOnClick.y), minY, maxY);
					moveTo(Common::Point(_screenPosition.left, newTop));
				} else {
					uint16 minX = _startPosition.x;
					uint16 maxX = minX + _maxDist;
					uint16 newLeft = CLIP<uint16>((_screenPosition.left + newMousePos.x - _mousePosOnClick.x), minX, maxX);
					moveTo(Common::Point(newLeft, _screenPosition.top));
				}

				calculatePosition();
			}
		}
	}

	bool wasClicked = _isClicked;
	if (input.input & NancyInput::kLeftMouseButtonUp) {
		_isClicked = false;
	}

	// If the mouse is clicked and moves outside the scrollbar's hotspot, we don't want it
	// to trigger other events. This only works if scrollbars are at the very top of the input priority.
	// As a result, this effect won't be applied to the scrollbars in SoundEqualizerPuzzle
	// In the future, this can be fixed by creating an input queue inside InputManager.
	if (wasClicked) {
		input.eatMouseInput();
	}
}

void Scrollbar::setPosition(float pos) {
    _currentPosition = pos;
    moveTo(Common::Point(_screenPosition.left, _startPosition.y + (_maxDist * pos)));
}

void Scrollbar::calculatePosition() {
	uint16 scroll = _isVertical ? _screenPosition.top - _startPosition.y : _screenPosition.left - _startPosition.x;

	_currentPosition = scroll != 0 ? (float)scroll / (float)_maxDist : 0;
}

void Scrollbar::resetPosition() {
	moveTo(Common::Point(_screenPosition.left, _startPosition.y));
	calculatePosition();
}

} // End of namespace UI
} // End of namespace Nancy
