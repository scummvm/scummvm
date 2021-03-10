/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/nancy/ui/scrollbar.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/commontypes.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"

#include "common/stream.h"
#include "common/random.h"
#include "common/events.h"

namespace Nancy {
namespace UI {

void Scrollbar::handleInput(NancyInput &input) {
    if (_screenPosition.contains(input.mousePos)) {
        NanEngine.cursorManager->setCursorType(CursorManager::kHotspotArrow);

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
                uint16 minY = _startPosition.y;
                uint16 maxY = minY + _maxDist;
                uint16 newTop = CLIP<uint16>((_screenPosition.top + newMousePos.y - _mousePosOnClick.y), minY, maxY);
                moveTo(Common::Point(_screenPosition.left, newTop));

                calculatePosition();
            }   
        }
    }

    if (input.input & NancyInput::kLeftMouseButtonUp) {
        _isClicked = false;
    }
}

void Scrollbar::calculatePosition() {
    uint16 scrollY = _screenPosition.top - _startPosition.y;

    _currentPosition = scrollY != 0 ? (float)scrollY / (float)_maxDist : 0;
}

void Scrollbar::resetPosition() {
    moveTo(Common::Point(_screenPosition.left, _startPosition.y));
    calculatePosition();
}

} // End of namespace UI
} // End of namespace Nancy
