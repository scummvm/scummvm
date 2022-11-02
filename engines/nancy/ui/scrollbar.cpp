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

Scrollbar::Scrollbar(RenderObject &redrawFrom, uint16 zOrder, const Common::Rect &srcBounds, const Common::Point &topPosition, uint16 scrollDistance, bool isVertical) :
		RenderObject(redrawFrom, zOrder),
		_isVertical(isVertical),
		_isClicked(false),
		_currentPosition(0),
		_maxDist(scrollDistance) {
	_drawSurface.create(g_nancy->_graphicsManager->_object0, srcBounds);

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
	if (_screenPosition.contains(input.mousePos)) {
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

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		_isClicked = false;
	}
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
