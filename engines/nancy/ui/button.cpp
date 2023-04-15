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

Button::Button(uint16 zOrder, Graphics::ManagedSurface &surface, const Common::Rect &clickSrcBounds, const Common::Rect &destBounds, const Common::Rect &hoverSrcBounds) :
		RenderObject(zOrder, surface, clickSrcBounds, destBounds),
		surf(surface),
		_clickSrc(clickSrcBounds),
		_hoverSrc(hoverSrcBounds),
		_isClicked(false) {
	setVisible(false);
	setTransparent(true);
}

void Button::handleInput(NancyInput &input) {
	if (_screenPosition.contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

		if (!_hoverSrc.isEmpty() && !_isClicked) {
			_drawSurface.create(surf, _hoverSrc);
			setVisible(true);
		}

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_isClicked = true;
			if (_hoverSrc.isEmpty()) {
				setVisible(true);
			} else {
				_drawSurface.create(surf, _clickSrc);
			}
		}
	} else if (!_isClicked && _isVisible) {
		setVisible(false);
	}
}

} // End of namespace UI
} // End of namespace Nancy
