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

#include "engines/nancy/misc/mousefollow.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"

namespace Nancy {
namespace Misc {

MouseFollowObject::MouseFollowObject()
		: RenderObject(8) {
	_viewportData = GetEngineData(VIEW);
}

void MouseFollowObject::handleInput(NancyInput &input) {
	Common::Point mousePos = input.mousePos;
	Common::Rect viewport = _viewportData->screenPosition;

	if (!_isPickedUp || !viewport.contains(mousePos)) {
		return;
	}

	mousePos.x -= viewport.left;
	mousePos.y -= viewport.top;

	// Move the tile under the cursor
	Common::Rect newScreenPos = _drawSurface.getBounds();
	newScreenPos.moveTo(mousePos);
	newScreenPos.translate(-newScreenPos.width() / 2, -newScreenPos.height() / 2);

	// Clip movement so the ring stays entirely inside the viewport
	if (newScreenPos.left < 0) {
		newScreenPos.translate(-newScreenPos.left, 0);
	} else if (newScreenPos.right > viewport.width()) {
		newScreenPos.translate(viewport.width() - newScreenPos.right, 0);
	}

	if (newScreenPos.top < 0) {
		newScreenPos.translate(0, -newScreenPos.top);
	} else if (newScreenPos.bottom > viewport.height()) {
		newScreenPos.translate(0, viewport.height() - newScreenPos.bottom);
	}

	if (newScreenPos != _screenPosition) {
		moveTo(newScreenPos);
	}
}

} // End of namespace Misc
} // End of namespace Nancy
