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
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/button.h"

namespace Nancy {
namespace UI {

Button::Button(RenderObject &redrawFrom, uint16 zOrder, Graphics::ManagedSurface &surface, const Common::Rect &srcBounds, const Common::Rect &destBounds) :
		RenderObject(redrawFrom, zOrder, surface, srcBounds, destBounds),
		_isClicked(false) {
	setVisible(false);
	setTransparent(true);
}

void Button::handleInput(NancyInput &input) {
	if (!_isClicked && _screenPosition.contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_isClicked = true;
			setVisible(true);
		}
	}
}

} // End of namespace UI
} // End of namespace Nancy
